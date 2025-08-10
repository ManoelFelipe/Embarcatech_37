/**
 * @file network_manager.c
 * @brief Implementação do gerenciamento da rede Wi-Fi e servidor HTTP.
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 */

#include "network_manager.h"
#include "app_config.h"
#include "alarm_control.h" // Para interagir com a lógica do alarme

#include <string.h>
#include <stdio.h>
#include <stdlib.h> // Para calloc/free

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "dhcpserver.h"
#include "dnsserver.h"

// --- Definições HTTP Internas ---
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
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
#define ALARM_PARAM_STR "alarm=%s"
#define HTTP_RESPONSE_REDIRECT_TO_ROOT "HTTP/1.1 302 Redirect\nLocation: http://%s/\n\n"


// Instâncias dos servidores DHCP e DNS, gerenciadas estaticamente por este módulo.
static dhcp_server_t s_dhcp_server;
static dns_server_t s_dns_server;

/**
 * @brief Estrutura de estado para uma conexão TCP individual com um cliente.
 * Esta estrutura é específica para a implementação do servidor HTTP.
 */
typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;    /**< PCB para a conexão do cliente. */
    int sent_len;           /**< Bytes enviados ao cliente. */
    char headers[128];      /**< Buffer para os cabeçalhos HTTP. */
    char result[1500];      /**< Buffer para o corpo da página HTML. */
    int header_len;         /**< Comprimento dos cabeçalhos. */
    int result_len;         /**< Comprimento do corpo HTML. */
    ip_addr_t *gw;          /**< Ponteiro para o endereço IP do gateway (IP do Pico). */
} TCP_CONNECT_STATE_T;

// --- Protótipos de Funções Estáticas (Callbacks TCP e Lógica Interna) ---
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err);
static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
static int http_generate_page_content(const char *params, char *result, size_t max_result_len);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb);
static void tcp_server_err(void *arg, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);
static bool tcp_server_open_internal(TCP_SERVER_T *state); // Renomeada para evitar conflito de nome

/**
 * @brief Fecha a conexão com um cliente TCP.
 */
static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("Falha ao fechar TCP, erro %d. Abortando.\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

/**
 * @brief Callback: Dados enviados e confirmados (acked) pelo cliente.
 */
static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("TCP dados enviados (acked): %u bytes\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("Todos os dados enviados e acked. Fechando conexão.\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

/**
 * @brief Gera o conteúdo da página HTML de controle do alarme.
 */
static int http_generate_page_content(const char *params, char *result, size_t max_result_len) {
    char alarm_command_str[4]; // "on" ou "off"

    if (params) {
        if (sscanf(params, ALARM_PARAM_STR, alarm_command_str) == 1) {
            if (strcmp(alarm_command_str, "on") == 0) {
                // Chama a função do módulo de controle de alarme para ativar
                if (!alarm_control_is_active()) {
                     DEBUG_printf("Comando HTTP: LIGAR ALARME\n");
                    alarm_control_set_active(true);
                }
            } else if (strcmp(alarm_command_str, "off") == 0) {
                // Chama a função do módulo de controle de alarme para desativar
                if (alarm_control_is_active()) {
                    DEBUG_printf("Comando HTTP: DESLIGAR ALARME\n");
                    alarm_control_set_active(false);
                }
            }
        }
    }

    // Obtém o estado atual do alarme do módulo de controle
    bool is_active = alarm_control_is_active();
    const char *current_status_str = is_active ? "LIGADO" : "DESLIGADO";
    const char *status_class_suffix = is_active ? "on" : "off";
    const char *next_action_param = is_active ? "off" : "on";
    const char *button_text = is_active ? "Desligar" : "Ligar";
    const char *button_class_suffix = is_active ? "off" : "on";

    return snprintf(result, max_result_len, ALARM_PAGE_BODY,
                    status_class_suffix, current_status_str,
                    next_action_param, button_class_suffix, button_text);
}

/**
 * @brief Callback: Dados recebidos de um cliente TCP.
 */
err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        DEBUG_printf("Conexão fechada pelo cliente.\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);

    if (p->tot_len > 0) {
        DEBUG_printf("TCP dados recebidos: %d bytes, erro: %d\n", p->tot_len, err);
        size_t copy_len = p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len;
        pbuf_copy_partial(p, con_state->headers, copy_len, 0);
        con_state->headers[copy_len] = '\0';

        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            char *full_request_path_and_params = con_state->headers + sizeof(HTTP_GET) - 1;
            while(*full_request_path_and_params == ' ') full_request_path_and_params++;
            char *http_version_start = strchr(full_request_path_and_params, ' ');
            if (http_version_start) {
                *http_version_start = '\0';
            }
            char *params = strchr(full_request_path_and_params, '?');
            char *actual_path = full_request_path_and_params;
            if (params) {
                *params++ = '\0';
            }

            DEBUG_printf("Requisição HTTP: Caminho='%s', Parâmetros='%s'\n", actual_path, params ? params : "Nenhum");

            if (strcmp(actual_path, "/") == 0) {
                con_state->result_len = http_generate_page_content(params, con_state->result, sizeof(con_state->result));
                if (con_state->result_len >= sizeof(con_state->result) - 1) {
                    DEBUG_printf("Buffer de resultado HTML muito pequeno: %d necessário, %u disponível.\n", con_state->result_len, (unsigned int)sizeof(con_state->result));
                    pbuf_free(p);
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
                }
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS, 200, con_state->result_len);
            } else {
                DEBUG_printf("Caminho '%s' não encontrado. Redirecionando para '/'.\n", actual_path);
                con_state->result_len = 0;
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT_TO_ROOT, ipaddr_ntoa(con_state->gw));
            }

            if (con_state->header_len >= sizeof(con_state->headers) - 1) {
                 DEBUG_printf("Buffer de cabeçalhos HTTP muito pequeno.\n");
                 pbuf_free(p);
                 return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            con_state->sent_len = 0;
            err_t write_err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (write_err != ERR_OK) {
                DEBUG_printf("Falha ao escrever cabeçalhos HTTP: %d\n", write_err);
                pbuf_free(p);
                return tcp_close_client_connection(con_state, pcb, write_err);
            }
            if (con_state->result_len > 0) {
                write_err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (write_err != ERR_OK) {
                    DEBUG_printf("Falha ao escrever corpo HTML: %d\n", write_err);
                    pbuf_free(p);
                    return tcp_close_client_connection(con_state, pcb, write_err);
                }
            }
        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

/**
 * @brief Callback: Polling periódico da conexão TCP.
 */
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("TCP Poll callback. Fechando conexão inativa.\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK);
}

/**
 * @brief Callback: Erro na conexão TCP.
 */
static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        DEBUG_printf("Erro na conexão TCP: %d\n", err);
        if (con_state) {
            tcp_close_client_connection(con_state, con_state->pcb, err);
        }
    }
}

/**
 * @brief Callback: Nova conexão TCP aceita.
 */
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg; // O 'arg' aqui é o estado global do servidor TCP
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("Falha ao aceitar conexão: %d\n", err);
        return ERR_VAL;
    }
    DEBUG_printf("Cliente conectado.\n");

    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("Falha ao alocar estado para conexão do cliente.\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb;
    con_state->gw = &state->gw; // Passa o endereço do gateway do servidor principal

    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

/**
 * @brief Abre o servidor TCP para escutar por conexões HTTP.
 * Função interna chamada por network_manager_init.
 */
static bool tcp_server_open_internal(TCP_SERVER_T *state) {
    DEBUG_printf("Iniciando servidor na porta %d\n", TCP_PORT);
    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("Falha ao criar PCB TCP.\n");
        return false;
    }
    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err != ERR_OK) {
        DEBUG_printf("Falha ao fazer bind na porta %d: %d\n", TCP_PORT, err);
        tcp_close(pcb);
        return false;
    }
    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("Falha ao colocar servidor em modo LISTEN.\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }
    tcp_arg(state->server_pcb, state); // Passa o estado principal do servidor como argumento para accept
    tcp_accept(state->server_pcb, tcp_server_accept);
    return true;
}

/**
 * @brief Inicializa o gerenciador de rede.
 */
bool network_manager_init(TCP_SERVER_T *state) {
    if (!state) {
        DEBUG_printf("Estado do servidor TCP nulo em network_manager_init.\n");
        return false;
    }

    if (cyw43_arch_init()) {
        DEBUG_printf("Falha ao inicializar cyw43_arch (Wi-Fi).\n");
        return false;
    }
    printf("CYW43 Arch inicializado.\n");

    cyw43_arch_lwip_begin(); // Protege acesso ao LwIP
    cyw43_arch_enable_ap_mode(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    alarm_control_set_ap_led(true); // Liga LED Azul para indicar AP ativo

    ip4_addr_t mask;
    IP4_ADDR(&state->gw, 192, 168, 4, 1); // IP do Pico W (Gateway)
    IP4_ADDR(&mask, 255, 255, 255, 0);   // Máscara de sub-rede

    dhcp_server_init(&s_dhcp_server, &state->gw, &mask);
    printf("Servidor DHCP iniciado no IP %s\n", ipaddr_ntoa(&state->gw));

    dns_server_init(&s_dns_server, &state->gw);
    printf("Servidor DNS iniciado.\n");
    cyw43_arch_lwip_end();

    if (!tcp_server_open_internal(state)) {
        DEBUG_printf("Falha ao abrir servidor TCP.\n");
        // Desfaz inicializações parciais
        cyw43_arch_lwip_begin();
        dns_server_deinit(&s_dns_server);
        dhcp_server_deinit(&s_dhcp_server);
        cyw43_arch_disable_ap_mode();
        cyw43_arch_lwip_end();
        alarm_control_set_ap_led(false);
        cyw43_arch_deinit();
        return false;
    }
    printf("Servidor HTTP iniciado. Conecte-se a rede Wi-Fi '%s'.\n", WIFI_SSID);
    printf("Acesse http://%s no navegador.\n", ipaddr_ntoa(&state->gw));
    return true;
}

/**
 * @brief Desinicializa o gerenciador de rede.
 */
void network_manager_deinit(TCP_SERVER_T *state) {
    if (state && state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
    // As conexões clientes ativas serão fechadas pelos seus timeouts ou callbacks de erro.

    cyw43_arch_lwip_begin();
    dns_server_deinit(&s_dns_server);
    dhcp_server_deinit(&s_dhcp_server);
    // A desabilitação do modo AP já foi feita na key_pressed_func,
    // mas podemos garantir aqui também se o state->complete for setado por outro motivo.
    // No entanto, a key_pressed_func já chama cyw43_arch_disable_ap_mode().
    // Se esta função for chamada após key_pressed_func, o AP já estará desabilitado.
    // cyw43_arch_disable_ap_mode(); // Pode ser redundante ou causar problema se chamado duas vezes sem reabilitar.
    cyw43_arch_lwip_end();

    alarm_control_set_ap_led(false);
    // cyw43_arch_deinit(); // A desinicialização final do CYW43 será feita no main.
    printf("Serviços de rede (DHCP, DNS, TCP) encerrados.\n");
}