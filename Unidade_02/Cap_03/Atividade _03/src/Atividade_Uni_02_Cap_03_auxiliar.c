/**
 * @file Atividade_Uni_02_Cap_03_auxiliar.c
 * @author Modificado Por Manoel Furtado
 * @version 1.2
 * @date 15 de junho de 2025
 * @brief Funções auxiliares do Núcleo 0 para interação com o usuário e tratamento de dados.
 *
 * @details
 * Este arquivo contém funções de suporte que são chamadas a partir do arquivo principal.
 * O objetivo desta separação é manter o arquivo principal mais limpo e focado na
 * lógica do loop de eventos, enquanto este arquivo lida com as "tarefas de apoio".
 *
 * As funções aqui presentes lidam com:
 * - Utilitários: Funções para gerar números aleatórios.
 * - Interface com o Usuário: Exibição de mensagens e status no display OLED.
 * - Feedback Visual: Controle do LED RGB para indicar o estado da rede, incluindo
 *                    a nova lógica de piscar com cor aleatória após um PING.
 * - Processamento de Dados: Interpretação das mensagens recebidas do Núcleo 1 via FIFO.
 */

// =============================================================================
// Includes de Bibliotecas
// =============================================================================
#include "fila_circular.h"      // Para o tipo de dado `MensagemWiFi`.
#include "rgb_pwm_control.h"    // Para a função `set_rgb_pwm`.
#include "configura_geral.h"    // Para constantes como `PWM_STEP`.
#include "oled_utils.h"         // Para funções como `exibir_e_esperar`.
#include "ssd1306_i2c.h"        // Para funções de desenho no OLED.
#include "estado_mqtt.h"        // Para acesso a variáveis globais como `buffer_oled`.
#include <stdio.h>              // Para `printf`.
#include <stdlib.h>             // Necessário para as funções `rand()` e `srand()`.
#include "pico/time.h"          // Necessário para `get_absolute_time()` e `sleep_ms()`.

// =============================================================================
// Protótipos de Funções Locais
// =============================================================================
// Embora estas funções sejam globais neste arquivo, elas não são expostas
// a outros arquivos através de um .h, sendo "privadas" a este módulo.
void inicializar_aleatorio(void);
int numero_aleatorio(int min, int max);

/**
 * @brief Inicializa o gerador de números aleatórios usando o tempo do sistema como semente.
 * @details Esta função deve ser chamada uma única vez no início do programa.
 * A "semente" (seed) garante que a sequência de números gerada por `rand()`
 * seja diferente a cada vez que o dispositivo é ligado.
 */
void inicializar_aleatorio() {
    // Usa o tempo em microssegundos desde o boot como "semente" para o gerador.
    // `get_absolute_time()` retorna um timestamp, e `to_us_since_boot` o converte para um número.
    srand(to_us_since_boot(get_absolute_time()));
}

/**
 * @brief Gera um número inteiro aleatório dentro de um intervalo especificado.
 * @param min O valor mínimo do intervalo (inclusivo).
 * @param max O valor máximo do intervalo (inclusivo).
 * @return Um número inteiro aleatório entre min e max.
 */
int numero_aleatorio(int min, int max) {
    // `rand()` gera um número pseudoaleatório. O operador '%' (módulo) ajusta este número para o intervalo desejado.
    return rand() % (max - min + 1) + min;
}

/**
 * @brief Aguarda em um loop bloqueante até que a conexão USB esteja pronta.
 * @details Útil para depuração, para garantir que as mensagens `printf` do início
 * não sejam perdidas se o monitor serial for aberto com atraso.
 */
void espera_usb() {
    // Loop bloqueante que só termina quando a conexão USB é detectada.
    while (!stdio_usb_connected()) {
        sleep_ms(200); // Pequena pausa para não sobrecarregar a CPU.
    }
    printf("Conexão USB estabelecida!\n");
}

/**
 * @brief Processa e exibe uma mensagem de status do Wi-Fi ou de confirmação de PING.
 * @details Esta função é o principal dispatcher de mensagens. Ela verifica o tipo de mensagem
 * (PING ou status de Wi-Fi) e age de acordo.
 * @param msg A estrutura `MensagemWiFi` contendo o status e o tipo de tentativa.
 */
void tratar_mensagem(MensagemWiFi msg) {
    // O identificador 0x9999 é um "código mágico" para indicar que esta é uma resposta de PING MQTT.
    if (msg.tentativa == 0x9999) {
        if (msg.status == 0) { // Status 0 para ACK de PING significa sucesso.
            ssd1306_draw_utf8_multiline(buffer_oled, 0, 32, "ACK do PING OK");
            
            // --- INÍCIO DA MELHORIA IMPLEMENTADA ---
            uint16_t r, g, b; // Variáveis para armazenar os componentes de cor (Vermelho, Verde, Azul).

            // 1. Gera uma cor aleatória, garantindo que não seja um verde forte.
            do {
                r = numero_aleatorio(0, 65535); // Gera valor para vermelho (0 a 100% do PWM).
                g = numero_aleatorio(0, 65535); // Gera valor para verde.
                b = numero_aleatorio(0, 65535); // Gera valor para azul.
            } while (g > r && g > b && g > 32768); // A condição impede cores onde o verde é o componente mais forte e brilhante.

            // 2. Define a cor aleatória gerada no LED para sinalizar visualmente o PING.
            set_rgb_pwm(r, g, b);
            render_on_display(buffer_oled, &area); // Atualiza o OLED junto com a mudança de cor do LED.

            // 3. Mantém a cor aleatória visível por 1 segundo.
            sleep_ms(1000);

            // 4. Retorna o LED para a cor verde padrão, indicando que a conexão continua OK.
            set_rgb_pwm(0, 65535, 0); // Verde sólido.
            // --- FIM DA MELHORIA IMPLEMENTADA ---

        } else { // Se o status do PING não for 0, significa falha.
            ssd1306_draw_utf8_multiline(buffer_oled, 0, 32, "ACK do PING FALHOU");
            set_rgb_pwm(65535, 0, 0); // LED Vermelho para indicar falha.
        }
        render_on_display(buffer_oled, &area); // Atualiza o display com o status final do PING.
        return; // Finaliza a função, pois a mensagem de PING já foi tratada.
    }

    // Se a mensagem não for de PING, executa a lógica antiga para status de conexão Wi-Fi.
    const char *descricao = "";
    switch (msg.status) {
        case 0:
            descricao = "INICIALIZANDO";
            set_rgb_pwm(PWM_STEP, 0, 0); // Vermelho.
            break;
        case 1:
            descricao = "CONECTADO";
            set_rgb_pwm(0, PWM_STEP, 0); // Verde.
            break;
        case 2:
            descricao = "FALHA";
            set_rgb_pwm(0, 0, PWM_STEP); // Azul.
            break;
        default:
            descricao = "DESCONHECIDO";
            set_rgb_pwm(PWM_STEP, PWM_STEP, PWM_STEP); // Branco.
            break;
    }
    // Formata e exibe a mensagem de status no OLED.
    char linha_status[32];
    snprintf(linha_status, sizeof(linha_status), "Status Wi-Fi: %s", descricao);
    oled_clear(buffer_oled, &area);
    ssd1306_draw_utf8_multiline(buffer_oled, 0, 0, linha_status);
    render_on_display(buffer_oled, &area);
    sleep_ms(2000); // Exibe a mensagem por 2 segundos.
    oled_clear(buffer_oled, &area);
    render_on_display(buffer_oled, &area);
    printf("[NÚCLEO 0] Status: %s\n", descricao);
}

/**
 * @brief Converte um endereço IP de formato binário (uint32_t) para string.
 * @details Após a conversão, a função exibe o IP no OLED, imprime no console, e
 * armazena o valor na variável global `ultimo_ip_bin`, que serve de gatilho
 * para o Núcleo 0 iniciar o cliente MQTT.
 * @param ip_bin O endereço IP de 32 bits recebido do Núcleo 1.
 */
void tratar_ip_binario(uint32_t ip_bin) {
    char ip_str[20]; // Buffer para armazenar o IP formatado como string.
    // Função da stack lwIP que converte um endereço IP binário para string no formato "A.B.C.D".
    ip4addr_ntoa_r((const ip4_addr_t*)&ip_bin, ip_str, sizeof(ip_str));
    // Exibe o IP no display.
    oled_clear(buffer_oled, &area);
    ssd1306_draw_utf8_string(buffer_oled, 0, 0, "IP Recebido:");
    ssd1306_draw_utf8_string(buffer_oled, 0, 16, ip_str);
    render_on_display(buffer_oled, &area);
    // Imprime no console para depuração.
    printf("[NÚCLEO 0] Endereço IP: %s\n", ip_str);
    // Atualiza a variável global de estado.
    ultimo_ip_bin = ip_bin;
}

/**
 * @brief Exibe uma mensagem de status do MQTT no display OLED e no terminal.
 * @details Função utilitária para centralizar e padronizar a forma como os
 * status do MQTT são reportados ao usuário.
 * @param texto A string de status a ser exibida.
 */
void exibir_status_mqtt(const char *texto) {
    // Escreve "MQTT: " seguido pelo texto de status no display.
    ssd1306_draw_utf8_string(buffer_oled, 0, 16, "MQTT: ");
    ssd1306_draw_utf8_string(buffer_oled, 40, 16, texto);
    render_on_display(buffer_oled, &area);
    printf("[MQTT] %s\n", texto);
}