/**
 * @file Atividade_08.c
 * @brief Simulador Portátil de Alarme para Treinamentos de Brigadas e Evacuação
 *
 * @objectives
 * - Configurar o Raspberry Pi Pico W como um ponto de acesso (Access Point) Wi-Fi.
 * - Iniciar servidores DHCP e DNS locais para permitir a conexão de dispositivos clientes.
 * - Criar um servidor HTTP embarcado que disponibiliza uma página HTML de controle.
 * - Permitir o controle remoto de um sistema de alarme (LEDs, Buzzer, Display OLED).
 * - Indicar estado do alarme via LED vermelho piscante e buzzer intermitente.
 * - Exibir mensagens de status ("EVACUAR", "Sistema em repouso") em um display OLED SSD1306.
 * - Usar LED verde para indicar sistema em repouso e LED azul para status do Access Point.
 * - Finalização controlada do modo Access Point via tecla 'd' no terminal serial.
 *
 * @pinout
 * - OLED SSD1306:
 * - SDA: GPIO 14 (I2C1)
 * - SCL: GPIO 15 (I2C1)
 * - LEDs:
 * - Verde: GPIO 11
 * - Azul:  GPIO 12 (AP Status)
 * - Vermelho: GPIO 13 (Alarme Ativo)
 * - Buzzer: GPIO 10
 *
 * @materials_concepts
 * - Wi-Fi em Modo Access Point (Pico W como servidor local)
 * - LED de Sinalização (estados do sistema e alarme)
 * - Buzzer (alerta sonoro intermitente)
 * - Comunicação I2C (para display OLED)
 * - Display OLED SSD1306 (exibição de mensagens textuais)
 * - Interface Web HTTP (controle remoto via navegador)
 *
 * @technical_requirements
 * - Rede Wi-Fi: Nome (SSID) e senha definidos, IP fixo.
 * - Interface Web: Página HTML com botões "Ligar Alarme" / "Desligar Alarme".
 * - Lógica de Controle: Interpretação de comandos HTTP (e.g., /?alarm=on).
 * - Controle de Periféricos: LEDs e buzzer via GPIO.
 * - Atualização OLED: Mensagens contextuais ao estado do alarme.
 * - Temporização: Piscar do LED e som do buzzer em intervalos, sem bloqueios.
 *  * @author  Manoel Furtado
 *          
 * @date      25 maio 2025
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 */

#include <string.h>
#include <stdio.h> // Para printf e snprintf
#include <stdlib.h> // Para calloc

// Pico SDK headers
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h" // Para Wi-Fi e controle do LED onboard (se usado)
#include "hardware/gpio.h"   // Para controle direto de GPIOs
#include "hardware/i2c.h"    // Para comunicação I2C com o OLED

// LwIP headers (Lightweight IP stack)
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

// Servidores DHCP e DNS (fornecidos externamente ou parte do SDK/exemplo)
#include "dhcpserver.h"
#include "dnsserver.h"

// Biblioteca para o Display OLED SSD1306 (arquivos fornecidos pelo usuário)
#include "ssd1306.h" // Deve incluir ssd1306_i2c.h e usar ssd1306_font.h internamente

// --- Definições do Projeto ---

// Configurações de Rede
#define TCP_PORT 80         // Porta padrão para HTTP
#define WIFI_SSID "PICO_ALARME_AP" // Nome da rede Wi-Fi criada pelo Pico W
#define WIFI_PASSWORD "picoalarme123" // Senha da rede Wi-Fi

// Pinos GPIO
#define LED_GREEN_GPIO 11    // LED verde indica sistema em repouso
#define LED_BLUE_GPIO 12     // LED azul indica status do Access Point ativo
#define LED_RED_GPIO 13      // LED vermelho pisca quando o alarme está ativo
#define BUZZER_GPIO 10       // Buzzer para alerta sonoro

// Pinos I2C para o Display OLED SSD1306
// A instância I2C (i2c0 ou i2c1) será usada diretamente nas funções i2c_init.
#define I2C_SDA_PIN 14       // Pino SDA para I2C (corresponde ao i2c1)
#define I2C_SCL_PIN 15       // Pino SCL para I2C (corresponde ao i2c1)
#define OLED_I2C_CLOCK 400000 // Clock do I2C para o OLED (400 kHz)

// Configurações do Alarme e Display
#define ALARM_BLINK_INTERVAL_MS 500 // Intervalo em milissegundos para piscar LED/Buzzer

// Strings para o Display OLED
#define MSG_EVACUAR "EVACUAR"
#define MSG_REPOUSO_L1 "Sistema em"
#define MSG_REPOUSO_L2 "repouso"
#define MSG_AP_OFF "AP Desativado"

// Definições para o Servidor HTTP
#define DEBUG_printf printf // Usar printf para mensagens de debug
#define POLL_TIME_S 5     // Tempo de poll para conexões TCP (não usado diretamente para fechar neste mod)

#define HTTP_GET "GET"
// Cabeçalhos HTTP para uma resposta OK com conteúdo HTML
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
// Corpo da página HTML para controle do alarme
// %s (1): Classe do Status (on/off)
// %s (2): Texto do Estado atual (LIGADO/DESLIGADO)
// %s (3): Próxima ação do parâmetro (on/off)
// %s (4): Classe do Botão (off/on)
// %s (5): Texto do botão (Desligar/Ligar)
#define ALARM_PAGE_BODY "<html><head><title>Controle de Alarme</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>" \
                        "<body><style>body{font-family: Arial, sans-serif; text-align: center; margin-top: 50px;} " \
                        "h1{color: #333;} h2{color: #444; font-size: 1.2em; margin-top: 0px;} p{color: #555;} " \
                        ".button {display: inline-block; padding: 15px 25px; font-size: 20px; cursor: pointer; " \
                        "text-align: center; text-decoration: none; outline: none; color: #fff; " \
                        "border: none; border-radius: 15px; box-shadow: 0 9px #999;} " \
                        ".button-on {background-color: #4CAF50;} .button-on:hover {background-color: #3e8e41} " \
                        ".button-off {background-color: #f44336;} .button-off:hover {background-color: #da190b} " \
                        ".status {font-weight: bold; font-size: 22px;} " \
                        ".status-on {color: #f44336;} .status-off {color: #4CAF50;}</style>" \
                        "<h1>Simulador Portatil de Alarme</h1>" \
                        "<h2>Atividade 08 - Manoel</h2>" \
                        "<p>Estado do Alarme: <strong class=\"status status-%s\">%s</strong></p>" \
                        "<p><a href=\"/?alarm=%s\" class=\"button button-%s\">%s Alarme</a></p>" \
                        "</body></html>"
// Parâmetro HTTP para controlar o alarme (ex: /?alarm=on)
#define ALARM_PARAM_STR "alarm=%s"
// Resposta de redirecionamento para a raiz (caso uma URL inválida seja acessada)
#define HTTP_RESPONSE_REDIRECT_TO_ROOT "HTTP/1.1 302 Redirect\nLocation: http://%s/\n\n"

// --- Variáveis Globais ---

// Estado do Alarme: 'true' se ativo, 'false' se inativo.
// 'volatile' pois pode ser modificada por uma requisição HTTP (ISR/callback conceitual) e lida pelo loop principal.
static volatile bool alarm_active = false;
// Estado de toggle para o LED/Buzzer piscante (alterna entre true/false)
static bool alarm_output_toggle_state = false;
// Timestamp da última vez que o LED/Buzzer foi alternado (para controle de intervalo)
static uint64_t last_toggle_time_us = 0;

// Buffer para o conteúdo do display OLED
static uint8_t oled_buffer[ssd1306_buffer_length];
// Estrutura que define a área de renderização do OLED (tela inteira)
static struct render_area display_area;


// --- Estruturas de Estado do Servidor TCP ---

// Estado do servidor TCP principal
typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb; // Protocol Control Block (PCB) para o servidor
    bool complete;              // Flag para indicar se o servidor deve terminar
    ip_addr_t gw;               // Endereço IP do gateway (o próprio Pico W no modo AP)
} TCP_SERVER_T;

// Estado de uma conexão TCP individual com um cliente
typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;    // PCB para a conexão do cliente
    int sent_len;           // Bytes enviados ao cliente
    char headers[128];      // Buffer para os cabeçalhos HTTP
    //char result[512];       // Buffer para o corpo da página HTML (aumentado para a nova página)
    char result[1500];
    int header_len;         // Comprimento dos cabeçalhos
    int result_len;         // Comprimento do corpo HTML
    ip_addr_t *gw;          // Ponteiro para o endereço IP do gateway
} TCP_CONNECT_STATE_T;


// --- Funções Utilitárias ---

/**
 * @brief Atualiza o display OLED com a mensagem de status apropriada.
 * Chamada quando o estado do alarme muda ou para inicialização.
 */
void update_oled_display_status() {
    // Limpa o buffer do OLED preenchendo com zeros (pixels apagados)
    memset(oled_buffer, 0, ssd1306_buffer_length);

    if (alarm_active) {
        // Se o alarme estiver ativo, exibe "EVACUAR"
        // Ajuste as coordenadas (x, y) para centralizar ou posicionar conforme desejado.
        // ssd1306_draw_string(buffer, x, y, texto)
        // A altura do caractere é 8 pixels. O display tem 64 pixels de altura.
        // Para centralizar verticalmente (64/2 - 8/2 = 28)
        ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_EVACUAR) * 8)) / 2, 28, MSG_EVACUAR);
    } else {
        // Se o alarme estiver inativo, exibe "Sistema em repouso" em duas linhas
        ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_REPOUSO_L1) * 8)) / 2, 20, MSG_REPOUSO_L1);
        ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_REPOUSO_L2) * 8)) / 2, 36, MSG_REPOUSO_L2);
    }
    // Envia o conteúdo do buffer para ser renderizado no display OLED
    render_on_display(oled_buffer, &display_area);
}

// --- Funções do Servidor TCP ---

/**
 * @brief Fecha a conexão com um cliente TCP.
 * Libera recursos associados à conexão.
 * @param con_state Estado da conexão do cliente.
 * @param client_pcb PCB do cliente.
 * @param close_err Código de erro para o fechamento.
 * @return Código de erro resultante.
 */
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        // Verifica se o estado da conexão e o PCB são válidos e correspondem
        assert(con_state && con_state->pcb == client_pcb);
        // Remove os callbacks e argumentos associados ao PCB do cliente
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);

        // Fecha a conexão TCP
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            // Se o fechamento falhar, aborta a conexão
            DEBUG_printf("Falha ao fechar TCP, erro %d. Abortando.\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT; // Marca como abortado
        }
        // Libera a memória alocada para o estado da conexão do cliente
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

/**
 * @brief Fecha o servidor TCP principal.
 * Libera o PCB do servidor.
 * @param state Estado do servidor TCP.
 */
static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL); // Remove argumento do PCB do servidor
        tcp_close(state->server_pcb);     // Fecha o PCB do servidor
        state->server_pcb = NULL;
    }
}

/**
 * @brief Callback chamado quando dados enviados ao cliente foram acked (confirmados).
 * Verifica se todos os dados (cabeçalhos + corpo) foram enviados e, em caso afirmativo, fecha a conexão.
 * @param arg Argumento (estado da conexão do cliente).
 * @param pcb PCB da conexão.
 * @param len Número de bytes confirmados.
 * @return Código de erro LwIP.
 */
static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("TCP dados enviados (acked): %u bytes\n", len);
    con_state->sent_len += len; // Atualiza o total de bytes enviados e confirmados

    // Se o total enviado é maior ou igual ao tamanho dos cabeçalhos + corpo da página,
    // significa que toda a resposta foi enviada e confirmada.
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("Todos os dados enviados e acked. Fechando conexão.\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK); // Fecha a conexão
    }
    return ERR_OK;
}

/**
 * @brief Gera o conteúdo da página HTML de controle do alarme.
 * Processa parâmetros da URL (se houver) para ligar/desligar o alarme.
 * @param params String contendo os parâmetros da URL (ex: "alarm=on").
 * @param result Buffer para armazenar o HTML gerado.
 * @param max_result_len Tamanho máximo do buffer 'result'.
 * @return Comprimento do HTML gerado.
 */
static int alarm_server_generate_page_content(const char *params, char *result, size_t max_result_len) {
    char alarm_command_str[4]; // Buffer para "on" ou "off"

    // Se existem parâmetros na URL
    if (params) {
        // Tenta extrair o valor do parâmetro "alarm"
        if (sscanf(params, ALARM_PARAM_STR, alarm_command_str) == 1) {
            if (strcmp(alarm_command_str, "on") == 0) {
                if (!alarm_active) { // Muda o estado apenas se for diferente
                    alarm_active = true;
                    DEBUG_printf("Comando HTTP: LIGAR ALARME\n");
                    update_oled_display_status(); // Atualiza o OLED
                    gpio_put(LED_GREEN_GPIO, false); // Apaga LED verde imediatamente
                    // A lógica de piscar o LED vermelho e buzzer é tratada no loop principal
                }
            } else if (strcmp(alarm_command_str, "off") == 0) {
                if (alarm_active) { // Muda o estado apenas se for diferente
                    alarm_active = false;
                    DEBUG_printf("Comando HTTP: DESLIGAR ALARME\n");
                    update_oled_display_status(); // Atualiza o OLED
                    // Desliga imediatamente os componentes do alarme
                    gpio_put(LED_RED_GPIO, false);
                    gpio_put(BUZZER_GPIO, false);
                    gpio_put(LED_GREEN_GPIO, true); // Acende LED verde
                    alarm_output_toggle_state = false; // Reseta o estado do toggle
                }
            }
        }
    }

    // Gera a página HTML com base no estado atual do alarme
    const char *current_status_str = alarm_active ? "LIGADO" : "DESLIGADO";
    const char *status_class_suffix = alarm_active ? "on" : "off"; // para cor do status
    const char *next_action_param = alarm_active ? "off" : "on";
    const char *button_text = alarm_active ? "Desligar" : "Ligar";
    const char *button_class_suffix = alarm_active ? "off" : "on"; // para cor do botão

    return snprintf(result, max_result_len, ALARM_PAGE_BODY,
                    status_class_suffix, current_status_str,
                    next_action_param, button_class_suffix, button_text);
}

/**
 * @brief Callback chamado quando dados são recebidos de um cliente TCP.
 * Processa a requisição HTTP GET, gera e envia a página de controle.
 * @param arg Argumento (estado da conexão do cliente).
 * @param pcb PCB da conexão.
 * @param p Buffer LwIP contendo os dados recebidos.
 * @param err Código de erro LwIP.
 * @return Código de erro LwIP.
 */
err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;

    // Se não há buffer (p == NULL), a conexão foi fechada pelo cliente
    if (!p) {
        DEBUG_printf("Conexão fechada pelo cliente.\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }

    // Verifica se o estado da conexão e o PCB são válidos
    assert(con_state && con_state->pcb == pcb);

    // Se dados foram recebidos (p->tot_len > 0)
    if (p->tot_len > 0) {
        DEBUG_printf("TCP dados recebidos: %d bytes, erro: %d\n", p->tot_len, err);

        // Copia a requisição HTTP para o buffer 'headers' do estado da conexão
        // Limita a cópia ao tamanho do buffer menos 1 para garantir terminação nula.
        size_t copy_len = p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len;
        pbuf_copy_partial(p, con_state->headers, copy_len, 0);
        con_state->headers[copy_len] = '\0'; // Garante terminação nula

        // Processa apenas requisições GET
        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            // Isola o caminho da requisição e os parâmetros
            char *full_request_path_and_params = con_state->headers + sizeof(HTTP_GET) -1; // Avança para depois de "GET"
            while(*full_request_path_and_params == ' ') full_request_path_and_params++; // Pula espaços após "GET "

            char *http_version_start = strchr(full_request_path_and_params, ' '); // Encontra o espaço antes de "HTTP/1.1"
            if (http_version_start) {
                *http_version_start = '\0'; // Termina a string do caminho+parâmetros
            }

            char *params = strchr(full_request_path_and_params, '?');
            char *actual_path = full_request_path_and_params; // Por padrão, o caminho completo é o atual

            if (params) {
                *params++ = '\0'; // Separa o caminho dos parâmetros. 'actual_path' agora é só o caminho.
                                  // 'params' agora aponta para a string de parâmetros.
            }

            DEBUG_printf("Requisição HTTP: Caminho='%s', Parâmetros='%s'\n", actual_path, params ? params : "Nenhum");

            // Processa apenas requisições para o caminho raiz "/"
            if (strcmp(actual_path, "/") == 0) {
                // Gera o conteúdo HTML da página de controle
                con_state->result_len = alarm_server_generate_page_content(params, con_state->result, sizeof(con_state->result));

                // Verifica se o buffer de resultado foi suficiente
                if (con_state->result_len >= sizeof(con_state->result) -1) { // >= por causa do terminador nulo
                    DEBUG_printf("Buffer de resultado HTML muito pequeno: %d necessário, %u disponível.\n", con_state->result_len, (unsigned int)sizeof(con_state->result));
                    pbuf_free(p); // Libera o pbuf recebido
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD); // Fecha por erro
                }

                // Gera os cabeçalhos HTTP para a resposta
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                                                 200, con_state->result_len); // HTTP 200 OK
                if (con_state->header_len >= sizeof(con_state->headers) -1) { // >= por causa do terminador nulo
                    DEBUG_printf("Buffer de cabeçalhos HTTP muito pequeno.\n");
                    pbuf_free(p);
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
                }
            } else {
                // Se o caminho não for "/", envia um redirecionamento para a raiz
                DEBUG_printf("Caminho '%s' não encontrado. Redirecionando para '/'.\n", actual_path);
                con_state->result_len = 0; // Sem corpo para redirecionamento
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers),
                                                 HTTP_RESPONSE_REDIRECT_TO_ROOT, ipaddr_ntoa(con_state->gw));
            }

            // Envia os cabeçalhos HTTP para o cliente
            con_state->sent_len = 0; // Reseta contador de bytes enviados
            err_t write_err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (write_err != ERR_OK) {
                DEBUG_printf("Falha ao escrever cabeçalhos HTTP: %d\n", write_err);
                pbuf_free(p);
                return tcp_close_client_connection(con_state, pcb, write_err);
            }

            // Se houver corpo de página (não é um redirecionamento puro), envia o corpo
            if (con_state->result_len > 0) {
                write_err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (write_err != ERR_OK) {
                    DEBUG_printf("Falha ao escrever corpo HTML: %d\n", write_err);
                    pbuf_free(p);
                    return tcp_close_client_connection(con_state, pcb, write_err);
                }
            }
            // Se não houver corpo (redirect), os cabeçalhos já foram enviados.
            // O callback tcp_server_sent fechará a conexão após o ack dos cabeçalhos.
        }
        // Informa ao LwIP que os dados do pbuf foram processados
        tcp_recved(pcb, p->tot_len);
    }
    // Libera o pbuf recebido, pois seus dados foram copiados ou processados
    pbuf_free(p);
    return ERR_OK;
}

/**
 * @brief Callback chamado periodicamente por LwIP para polling da conexão.
 * Se a conexão estiver ociosa por muito tempo, pode ser fechada aqui.
 * Neste exemplo, optamos por fechar se não houver atividade (pode ser agressivo).
 * @param arg Argumento (estado da conexão do cliente).
 * @param pcb PCB da conexão.
 * @return Código de erro LwIP.
 */
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("TCP Poll callback. Fechando conexão inativa.\n");
    // Fecha a conexão. Isso pode ser muito agressivo dependendo do uso.
    // O exemplo original fechava aqui. Manteremos para consistência com ele,
    // mas em um cenário real, pode ser necessário um timeout mais inteligente.
    return tcp_close_client_connection(con_state, pcb, ERR_OK);
}

/**
 * @brief Callback chamado quando ocorre um erro na conexão TCP.
 * @param arg Argumento (estado da conexão do cliente).
 * @param err Código do erro.
 */
static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) { // ERR_ABRT significa que a conexão já foi abortada
        DEBUG_printf("Erro na conexão TCP: %d\n", err);
        if (con_state) { // Se o estado da conexão ainda existe
            tcp_close_client_connection(con_state, con_state->pcb, err);
        }
    }
}

/**
 * @brief Callback chamado quando uma nova conexão TCP é aceita pelo servidor.
 * Configura callbacks para a nova conexão e aloca estado para ela.
 * @param arg Argumento (estado do servidor TCP).
 * @param client_pcb PCB da nova conexão do cliente.
 * @param err Código de erro LwIP.
 * @return Código de erro LwIP.
 */
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("Falha ao aceitar conexão: %d\n", err);
        return ERR_VAL;
    }
    DEBUG_printf("Cliente conectado.\n");

    // Aloca memória para o estado da nova conexão
    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("Falha ao alocar estado para conexão do cliente.\n");
        return ERR_MEM; // Erro de memória
    }
    con_state->pcb = client_pcb; // Armazena o PCB do cliente no estado
    con_state->gw = &state->gw;   // Passa o ponteiro do IP do gateway

    // Configura os callbacks para esta conexão específica
    tcp_arg(client_pcb, con_state);          // Passa o estado da conexão como argumento para os callbacks
    tcp_sent(client_pcb, tcp_server_sent);   // Callback para dados enviados e confirmados
    tcp_recv(client_pcb, tcp_server_recv);   // Callback para dados recebidos
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2); // Callback de polling periódico
    tcp_err(client_pcb, tcp_server_err);     // Callback para erros

    return ERR_OK;
}

/**
 * @brief Inicializa e abre o servidor TCP na porta definida.
 * Configura o servidor para escutar por novas conexões.
 * @param arg Argumento (estado do servidor TCP).
 * @param ap_name Nome do Access Point (usado para mensagem de log).
 * @return 'true' se o servidor foi aberto com sucesso, 'false' caso contrário.
 */
static bool tcp_server_open(void *arg, const char *ap_name) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    DEBUG_printf("Iniciando servidor na porta %d\n", TCP_PORT);

    // Cria um novo PCB TCP para qualquer tipo de IP (IPv4 ou IPv6, se habilitado)
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("Falha ao criar PCB TCP.\n");
        return false;
    }

    // Associa (bind) o PCB ao endereço IP local (qualquer) e à porta TCP_PORT
    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err != ERR_OK) {
        DEBUG_printf("Falha ao fazer bind na porta %d: %d\n", TCP_PORT, err);
        tcp_close(pcb); // Libera o PCB se o bind falhar
        return false;
    }

    // Coloca o servidor em modo de escuta (LISTEN) com um backlog de 1 conexão pendente
    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("Falha ao colocar servidor em modo LISTEN.\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    // Configura o argumento e o callback para aceitar novas conexões
    tcp_arg(state->server_pcb, state);                      // Passa o estado do servidor para o callback de accept
    tcp_accept(state->server_pcb, tcp_server_accept);     // Define a função a ser chamada quando uma conexão é aceita

    printf("Servidor HTTP iniciado. Conecte-se a rede Wi-Fi '%s'.\n", ap_name);
    printf("Acesse http://%s (ou o IP atribuido ao seu dispositivo) no navegador.\n", ipaddr_ntoa(&state->gw));
    printf("Pressione 'd' no terminal serial para desabilitar o Access Point e encerrar.\n");
    return true;
}

/**
 * @brief Callback chamado quando um caractere está disponível na entrada serial (stdio).
 * Usado para detectar a tecla 'd' para desabilitar o Access Point.
 * @param param Argumento (estado do servidor TCP).
 */
void key_pressed_func(void *param) {
    assert(param); // Garante que o parâmetro não é nulo
    TCP_SERVER_T *state = (TCP_SERVER_T*)param;
    int key = getchar_timeout_us(0); // Lê um caractere sem bloquear

    // Se a tecla 'd' ou 'D' for pressionada
    if (key == 'd' || key == 'D') {
        printf("\nTecla 'd' pressionada. Desabilitando Access Point e encerrando...\n");

        // Desativa o alarme e todos os atuadores
        alarm_active = false;
        gpio_put(LED_RED_GPIO, false);
        gpio_put(BUZZER_GPIO, false);
        gpio_put(LED_GREEN_GPIO, false); // Todas as LEDs de status/alarme desligadas
        gpio_put(LED_BLUE_GPIO, false);  // LED de status do AP desligado

        // Atualiza o display OLED para indicar que o AP foi desativado
        memset(oled_buffer, 0, ssd1306_buffer_length);
        ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_AP_OFF) * 8)) / 2, 28, MSG_AP_OFF);
        render_on_display(oled_buffer, &display_area);

        // Desabilita o modo Access Point da interface Wi-Fi
        // cyw43_arch_lwip_begin/end são necessários para proteger o acesso ao stack LwIP
        cyw43_arch_lwip_begin();
        cyw43_arch_disable_ap_mode();
        cyw43_arch_lwip_end();

        state->complete = true; // Sinaliza para o loop principal que o programa deve terminar
    }
}


// --- Função Principal (main) ---
int main() {
    // Inicializa stdio para comunicação serial (printf, getchar)
    stdio_init_all();
    printf("Simulador Portatil de Alarme - Iniciando...\n");

    // Aloca memória para o estado do servidor TCP
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("Falha ao alocar estado do servidor TCP.\n");
        return 1;
    }

    // Inicializa a interface Wi-Fi (CYW43)
    if (cyw43_arch_init()) {
        DEBUG_printf("Falha ao inicializar cyw43_arch (Wi-Fi).\n");
        free(state);
        return 1;
    }
     printf("CYW43 Arch inicializado.\n");

    // --- Inicialização dos GPIOs ---
    gpio_init(LED_GREEN_GPIO);
    gpio_set_dir(LED_GREEN_GPIO, GPIO_OUT);
    gpio_put(LED_GREEN_GPIO, true); // LED Verde ON inicialmente (sistema em repouso)

    gpio_init(LED_BLUE_GPIO);
    gpio_set_dir(LED_BLUE_GPIO, GPIO_OUT);
    gpio_put(LED_BLUE_GPIO, false); // LED Azul OFF inicialmente (será ligado com AP)

    gpio_init(LED_RED_GPIO);
    gpio_set_dir(LED_RED_GPIO, GPIO_OUT);
    gpio_put(LED_RED_GPIO, false); // LED Vermelho OFF inicialmente

    gpio_init(BUZZER_GPIO);
    gpio_set_dir(BUZZER_GPIO, GPIO_OUT);
    gpio_put(BUZZER_GPIO, false); // Buzzer OFF inicialmente

    printf("GPIOs para LEDs e Buzzer inicializados.\n");

    // --- Inicialização do Display OLED (I2C) ---
    printf("Inicializando I2C para Display OLED...\n");
    // Usa diretamente i2c1, que corresponde aos pinos GP14 (SDA) e GP15 (SCL)
    // quando configurados para a função I2C.
    i2c_init(i2c1, OLED_I2C_CLOCK); // Inicializa I2C1 na velocidade definida
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // Configura pino SDA para função I2C
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // Configura pino SCL para função I2C
    // Pull-ups geralmente não são necessários nos pinos do Pico se o módulo OLED já os tiver.
    // Se houver problemas de comunicação, descomente as duas linhas abaixo:
    // gpio_pull_up(I2C_SDA_PIN);
    // gpio_pull_up(I2C_SCL_PIN);
    printf("Pinos I2C configurados (SDA: %d, SCL: %d para i2c1).\n", I2C_SDA_PIN, I2C_SCL_PIN);


    // Define a área de renderização para cobrir todo o display
    display_area.start_column = 0;
    display_area.end_column = ssd1306_width - 1;
    display_area.start_page = 0;
    display_area.end_page = ssd1306_n_pages - 1;
    // Calcula o tamanho do buffer necessário para esta área (deve ser feito pela lib ssd1306)
    // A função calculate_render_area_buffer_length é externa, vinda de ssd1306.h/ssd1306_i2c.c
    calculate_render_area_buffer_length(&display_area);

    // Inicializa o hardware do display OLED
    ssd1306_init(); // Esta função deve configurar o display SSD1306
    printf("Display OLED SSD1306 inicializado.\n");

    // Atualiza o display OLED com a mensagem inicial de "Sistema em repouso"
    update_oled_display_status();


    // Configura callback para ser notificado quando um caractere é pressionado no terminal serial
    stdio_set_chars_available_callback(key_pressed_func, state);

    // Habilita o modo Access Point (AP)
    // cyw43_arch_lwip_begin/end são para acesso seguro ao stack LwIP
    cyw43_arch_lwip_begin();
    cyw43_arch_enable_ap_mode(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    gpio_put(LED_BLUE_GPIO, true); // Liga LED Azul para indicar que AP está ativo

    // Configura o endereço IP e máscara de rede para o Pico W no modo AP
    // Estes são os endereços padrão da SDK para o modo AP.
    ip4_addr_t mask;
    IP4_ADDR(&state->gw, 192, 168, 4, 1); // IP do Pico W (Gateway para os clientes)
    IP4_ADDR(&mask, 255, 255, 255, 0);   // Máscara de sub-rede

    // Inicia o servidor DHCP para atribuir IPs aos clientes que se conectarem
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);
    printf("Servidor DHCP iniciado no IP %s\n", ipaddr_ntoa(&state->gw));

    // Inicia o servidor DNS (geralmente para captive portal, mas útil para AP)
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);
    printf("Servidor DNS iniciado.\n");
    cyw43_arch_lwip_end();


    // Abre o servidor TCP para escutar por conexões HTTP
    if (!tcp_server_open(state, WIFI_SSID)) {
        DEBUG_printf("Falha ao abrir servidor TCP.\n");
        cyw43_arch_deinit();
        free(state);
        return 1;
    }

    // Loop principal do programa
    state->complete = false;
    while(!state->complete) {
        // Se estiver usando PICO_CYW43_ARCH_POLL, é necessário chamar cyw43_arch_poll()
        // periodicamente para processar eventos Wi-Fi e LwIP.
        #if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        #endif

        uint64_t current_time_us_loop = time_us_64();

        // Lógica do Alarme (piscar LED vermelho e buzzer)
        if (alarm_active) {
            // Verifica se passou o intervalo para alternar o estado do LED/Buzzer
            if (current_time_us_loop - last_toggle_time_us >= (ALARM_BLINK_INTERVAL_MS * 1000)) {
                alarm_output_toggle_state = !alarm_output_toggle_state; // Alterna o estado
                gpio_put(LED_RED_GPIO, alarm_output_toggle_state);      // Aplica ao LED vermelho
                gpio_put(BUZZER_GPIO, alarm_output_toggle_state);       // Aplica ao buzzer
                last_toggle_time_us = current_time_us_loop;             // Atualiza o timestamp
            }
            // Garante que o LED verde está desligado quando o alarme está ativo
            // (já tratado na mudança de estado, mas redundância não prejudica aqui)
            // gpio_put(LED_GREEN_GPIO, false);
        } else {
            // Se o alarme não estiver ativo, garante que LED vermelho e buzzer estão desligados
            // e LED verde está ligado. (maior parte já tratada na mudança de estado)
            // gpio_put(LED_RED_GPIO, false);
            // gpio_put(BUZZER_GPIO, false);
            // gpio_put(LED_GREEN_GPIO, true);
            alarm_output_toggle_state = false; // Reseta o estado do toggle para evitar surpresas
        }

        // O PICO_CYW43_ARCH_POLL define se o driver Wi-Fi é baseado em polling ou interrupções.
        // Se baseado em polling, cyw43_arch_wait_for_work_until() pode ser usado para
        // economizar energia esperando por trabalho ou por um timeout.
        #if PICO_CYW43_ARCH_POLL
        // Espera por trabalho do Wi-Fi/LwIP ou até 10ms.
        // Um timeout menor permite que a lógica do alarme (piscar) seja mais responsiva
        // se não houver muita atividade de rede.
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10));
        #else
        // Se não estiver usando polling, o trabalho Wi-Fi/LwIP ocorre em background (interrupções).
        // Um pequeno sleep_ms pode ser usado para ceder tempo a outras tarefas, se houver.
        sleep_ms(10);
        #endif
    }

    // --- Encerramento ---
    printf("Encerrando servidor e serviços...\n");
    tcp_server_close(state);    // Fecha o servidor TCP
    dns_server_deinit(&dns_server); // Desinicializa o servidor DNS
    dhcp_server_deinit(&dhcp_server); // Desinicializa o servidor DHCP

    // Desinicializa a arquitetura CYW43 (Wi-Fi)
    cyw43_arch_deinit();

    free(state); // Libera a memória do estado do servidor
    printf("Simulador de Alarme encerrado.\n");
    return 0;
}