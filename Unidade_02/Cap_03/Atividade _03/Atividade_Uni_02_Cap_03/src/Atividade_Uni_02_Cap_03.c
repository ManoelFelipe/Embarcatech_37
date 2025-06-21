/**
 * @file Atividade_Uni_02_Cap_03.c
 * @author Modificado Por Manoel Furtado
 * @version 1.2
 * @date 15 de junho de 2025
 * @brief Ponto de entrada principal do Núcleo 0 (Core 0) para o projeto com MQTT.
 *
 * @details
 * Este arquivo contém a lógica principal que roda no primeiro núcleo do microcontrolador
 * RP2040. Ele atua como o orquestrador geral do sistema, gerenciando a interface com
 * o usuário, a inicialização de hardware e a comunicação com o Núcleo 1.
 *
 * Suas responsabilidades são:
 * 1.  Inicialização do Sistema: Configura periféricos como o display OLED, o LED RGB (via PWM),
 *                               a comunicação serial e o gerador de números aleatórios.
 * 
 * 2.  Lançamento do Núcleo 1: Inicia o segundo núcleo, que é dedicado às tarefas de rede
 *                             (Wi-Fi e operações de baixo nível do MQTT).
 * 
 * 3.  Comunicação Inter-Core: Entra em um loop de eventos infinito onde monitora a FIFO (um buffer de
 *                             comunicação entre os núcleos) para receber mensagens do Núcleo 1.
 * 
 * As mensagens podem ser status da conexão Wi-Fi, um endereço IP ou confirmações de PING.
 * 4.  Gerenciamento de Estado MQTT: Implementa uma máquina de estados simples que, após receber um
 *                                   endereço IP válido do Núcleo 1, inicia o cliente MQTT.
 * 
 * 5.  Lógica de Aplicação: Utiliza um temporizador não-bloqueante para enviar periodicamente uma
 *                          mensagem "PING" via MQTT, verificando a conectividade de ponta a ponta.
 * 
 * 6.  Tratamento de Mensagens: Utiliza uma fila circular local para armazenar mensagens recebidas
 *                              do Núcleo 1, desacoplando o recebimento do processamento e
 *                              mantendo o loop principal responsivo.
 */

// =============================================================================
// Includes de Bibliotecas
// =============================================================================
#include "fila_circular.h"      // Estrutura de dados para a fila de mensagens do Wi-Fi.
#include "rgb_pwm_control.h"    // Funções para controlar o LED RGB com PWM.
#include "configura_geral.h"    // Arquivo de configuração central com pinos, senhas, etc.
#include "oled_utils.h"         // Funções utilitárias para o display OLED.
#include "ssd1306_i2c.h"        // Driver de baixo nível para o display OLED SSD1306.
#include "mqtt_lwip.h"          // Funções relacionadas ao cliente MQTT.
#include "lwip/ip_addr.h"       // Para manipulação de endereços IP da stack lwIP.
#include "pico/multicore.h"     // Funções da SDK para gerenciamento dos dois núcleos.
#include <stdio.h>              // Biblioteca padrão de entrada/saída para printf.
#include "estado_mqtt.h"        // Variáveis globais compartilhadas (estado do sistema).

// =============================================================================
// Protótipos de Funções Externas
// =============================================================================
// A palavra-chave 'extern' indica que estas funções estão definidas em outro arquivo.
extern void funcao_wifi_nucleo1(void);      // Ponto de entrada do código que rodará no Núcleo 1.
extern void espera_usb();                   // Função de espera definida em Atividade_Uni_02_Cap_03_auxiliar.c.
extern void tratar_ip_binario(uint32_t ip_bin); // Função para processar o IP, definida no arquivo auxiliar.
extern void tratar_mensagem(MensagemWiFi msg);  // Função principal de tratamento de mensagens, no arquivo auxiliar.
extern void inicializar_aleatorio(void);    // Função para inicializar o gerador de números aleatórios, no arquivo auxiliar.

// =============================================================================
// Protótipos de Funções Locais
// =============================================================================
// Funções definidas e utilizadas apenas dentro deste arquivo.
void inicia_hardware();
void inicia_core1();
void verificar_fifo(void);
void tratar_fila(void);
void inicializar_mqtt_se_preciso(void);
void enviar_ping_periodico(void);

// =============================================================================
// Variáveis Globais do Arquivo
// =============================================================================
FilaCircular fila_wifi;         /**< Fila circular local para armazenar mensagens recebidas do Núcleo 1 antes do processamento. */
absolute_time_t proximo_envio;  /**< Armazena o timestamp do próximo envio de PING MQTT para controle de tempo não-bloqueante. */
char mensagem_str[50];          /**< Buffer de string reutilizável para formatar mensagens de depuração via printf. */
bool ip_recebido = false;       /**< Flag local que se torna verdadeiro após o recebimento do IP, usado como condição. */

/**
 * @brief Função principal, ponto de entrada do programa no Núcleo 0.
 */
int main() {
    // 1. Inicializa o hardware básico (USB, OLED, PWM, gerador aleatório).
    inicia_hardware();
    
    // 2. Inicializa os componentes lógicos e lança o código de rede no Núcleo 1.
    inicia_core1();

    // 3. Loop de eventos principal e infinito. Este é o coração do programa no Núcleo 0.
    while (true) {
        // Tarefa 1: Verificar se há novas mensagens do Núcleo 1 na FIFO.
        verificar_fifo();
        
        // Tarefa 2: Processar uma mensagem da fila local (se houver).
        tratar_fila();
        
        // Tarefa 3: Verificar se as condições para iniciar o MQTT foram atendidas.
        inicializar_mqtt_se_preciso();
        
        // Tarefa 4: Enviar a mensagem "PING" se o tempo programado tiver chegado.
        enviar_ping_periodico();
        
        // Pausa muito curta para ceder tempo de processamento e evitar que o loop consuma 100% da CPU.
        sleep_ms(50);
    }

    // Este ponto do código nunca deve ser alcançado em um sistema embarcado.
    return 0;
}

/**
 * @brief Verifica a FIFO de comunicação inter-core por novas mensagens do Núcleo 1.
 * @details Esta função implementa o protocolo de comunicação customizado entre os núcleos.
 * Ela lê um "pacote" de 32 bits e o interpreta.
 */
void verificar_fifo(void) {
    // Verifica de forma não-bloqueante se há dados na FIFO para serem lidos. Se não houver, retorna imediatamente.
    if (!multicore_fifo_rvalid()) return;

    // Lê um pacote de 32 bits da FIFO. Esta chamada é bloqueante, mas só é executada se houver dados.
    uint32_t pacote = multicore_fifo_pop_blocking();
    
    // Extrai os 16 bits mais significativos, que são usados como um identificador da mensagem (ID).
    uint16_t id_msg = pacote >> 16;

    // O ID 0xFFFE é um "código mágico" que sinaliza que o próximo dado na FIFO é o endereço IP.
    if (id_msg == 0xFFFE) {
        uint32_t ip_bin = multicore_fifo_pop_blocking(); // Lê o endereço IP de 32 bits da FIFO.
        tratar_ip_binario(ip_bin); // Chama a função auxiliar para processar e exibir o IP.
        ip_recebido = true; // Define o flag para sinalizar que o IP foi recebido.
        return; // Finaliza a função, pois a mensagem de IP foi tratada.
    }

    // Se não for um IP, os 16 bits inferiores contêm o status real do Wi-Fi.
    uint16_t status = pacote & 0xFFFF;
    // E o `id_msg` representa o número da tentativa de conexão ou o ID do PING.
    uint16_t tentativa = id_msg;

    // Tratamento de erro básico para um status desconhecido. O ID 0x9999 é reservado para o PING.
    if (status > 2 && tentativa != 0x9999) {
        snprintf(mensagem_str, sizeof(mensagem_str), "Status inválido: %u (tentativa %u)", status, tentativa);
        exibir_e_esperar("Status inválido.", 0);
        printf("%s\n", mensagem_str);
        return;
    }

    // Cria a estrutura da mensagem com os dados recebidos.
    MensagemWiFi msg = {.tentativa = tentativa, .status = status};
    // Tenta inserir a mensagem na fila circular local.
    if (!fila_inserir(&fila_wifi, msg)) {
        // Se a fila estiver cheia, exibe um aviso e descarta a mensagem.
        exibir_e_esperar("Fila cheia.", 0);
        printf("Fila cheia. Mensagem descartada.\n");
    }
}

/**
 * @brief Remove e processa uma única mensagem da fila circular local.
 * @details Ao processar uma mensagem por vez, o loop principal se mantém responsivo
 * e não fica preso tratando múltiplas mensagens de uma só vez.
 */
void tratar_fila(void) {
    MensagemWiFi msg_recebida;
    // Tenta remover um item da fila. Se a operação for bem-sucedida (fila não estava vazia)...
    if (fila_remover(&fila_wifi, &msg_recebida)) {
        // ...chama a função de tratamento principal no arquivo auxiliar.
        tratar_mensagem(msg_recebida);
    }
}

/**
 * @brief Inicia o cliente MQTT se as condições necessárias forem satisfeitas.
 * @details Esta função atua como uma máquina de estados simples. Ela só executa sua ação
 * uma única vez, quando o cliente MQTT ainda não foi iniciado E um endereço IP
 * válido já foi recebido do Núcleo 1.
 */
void inicializar_mqtt_se_preciso(void) {
    // Verifica as duas condições para iniciar o MQTT.
    if (!mqtt_iniciado && ultimo_ip_bin != 0) {
        printf("[MQTT] Condições atendidas. Iniciando cliente MQTT...\n");
        iniciar_mqtt_cliente(); // Chama a função que configura e conecta o cliente.
        mqtt_iniciado = true;   // Define o flag para `true` para que esta lógica não seja executada novamente.
        // Agenda o primeiro envio de PING para dali a `INTERVALO_PING_MS`.
        proximo_envio = make_timeout_time_ms(INTERVALO_PING_MS);
    }
}

/**
 * @brief Envia a mensagem "PING" para o tópico MQTT em intervalos regulares.
 * @details Usa a função `absolute_time_diff_us` para uma verificação de tempo
 * não-bloqueante. A mensagem só é enviada se o cliente MQTT já estiver iniciado
 * e se o tempo definido em `INTERVALO_PING_MS` tiver passado.
 */
void enviar_ping_periodico(void) {
    // Verifica se o MQTT está ativo e se o tempo para o próximo envio já chegou.
    // `absolute_time_diff_us` retorna um valor negativo ou zero se o tempo de `proximo_envio` foi atingido.
    if (mqtt_iniciado && absolute_time_diff_us(get_absolute_time(), proximo_envio) <= 0) {
        publicar_mensagem_mqtt("PING"); // Publica a mensagem "PING" no tópico padrão.
        ssd1306_draw_utf8_multiline(buffer_oled, 0, 48, "PING enviado...");
        render_on_display(buffer_oled, &area);
        // Agenda o próximo envio, renovando o temporizador.
        proximo_envio = make_timeout_time_ms(INTERVALO_PING_MS);
    }
}

/**
 * @brief Inicializa o hardware básico do sistema no Núcleo 0.
 */
void inicia_hardware() {
    stdio_init_all();      // Inicializa a E/S padrão (necessário para printf via USB).
    setup_init_oled();     // Inicializa a comunicação I2C e o controlador do display OLED.
    espera_usb();          // Aguarda a conexão serial ser estabelecida para não perder mensagens de debug.

    // MELHORIA: Inicializa o gerador de números aleatórios com uma semente baseada no tempo.
    // Isto é crucial para que as cores do LED sejam diferentes a cada reinicialização.
    inicializar_aleatorio();

    oled_clear(buffer_oled, &area); // Limpa o buffer de vídeo do OLED.
    render_on_display(buffer_oled, &area); // Envia o buffer limpo para a tela, efetivamente limpando-a.
}

/**
 * @brief Inicializa os componentes lógicos e lança o Núcleo 1.
 */
void inicia_core1() {
    // Exibe mensagens de inicialização no display OLED para feedback visual.
    exibir_e_esperar("Nucleo 0 OK", 0);
    exibir_e_esperar("Iniciando Core 1", 16);
    
    printf(">> Núcleo 0 iniciado. Aguardando mensagens do núcleo 1...\n");

    init_rgb_pwm();        // Inicializa os pinos e o PWM para o controle do LED RGB.
    fila_inicializar(&fila_wifi); // Inicializa a estrutura da fila circular de mensagens.
    
    // Lança a função `funcao_wifi_nucleo1` no segundo núcleo.
    // A partir deste ponto, os dois núcleos do RP2040 estarão executando código em paralelo.
    multicore_launch_core1(funcao_wifi_nucleo1);
}