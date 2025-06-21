/**
 * @file    web_server.c
 * @brief   Implementação de um servidor HTTP simples usando a pilha lwIP.
 * @details Este módulo gerencia conexões TCP, analisa requisições HTTP GET básicas,
 * e serve uma página HTML dinâmica que permite ao usuário interagir com
 * o dispositivo (controlar LED e ver temperatura).
 */

#include "web_server.h"
#include "led_control.h"
#include "temperature.h"
#include "debug.h"
#include "lwip/tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- página HTML -------------------------------------------------*/

/**
 * @brief Gera dinamicamente o conteúdo da página HTML.
 * @param buf Ponteiro para o buffer onde a página HTML será escrita.
 * @param len Tamanho máximo do buffer.
 * @return O número de caracteres escritos no buffer (excluindo o terminador nulo).
 */
static int make_page(char *buf, size_t len)
{
    // Obtém o estado atual do LED e da temperatura.
    bool  on  = led_get();
    float t   = temperature_read_c();

    // Define strings que mudarão com base no estado do LED.
    const char *state_cls  = on ? "state-on"  : "state-off";    // Classe CSS para o status.
    const char *button_txt = on ? "Desligar LED" : "Ligar LED"; // Texto do botão.
    const char *param      = on ? "off" : "on";                 // Parâmetro no link do botão.

    // `snprintf` é usado para construir a string HTML de forma segura, evitando buffer overflow.
    // A página contém CSS embutido para estilização e placeholders (%s, %.2f) que são
    // substituídos pelos valores dinâmicos.
    return snprintf(buf, len,
      "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
      "<title>Pico W</title>"
      "<style>"
        "body{font-family:sans-serif;text-align:center;margin-top:40px;background:#f2f2f2;}"
        "span.temp{color:#0D47A1;font-weight:bold;}"
        ".state-on{background:#8BC34A;color:#000;}"
        ".state-off{background:#EF5350;color:#000;}"
        "button{padding:14px 24px;font-size:18px;border:0;border-radius:10px;cursor:pointer;}"
      "</style></head><body>"
      "<h1>Manoel_Atividade 02_Und. 02</h1>"
      "<h2>Servidor HTTP Pico W</h2>"
      "<p>Temperatura interna: <span class=\"temp\">%.2f °C</span></p>"
      "<p>Status do LED: <span class=\"%s\">%s</span></p>"
      "<p><a href=\"/?led=%s\"><button class=\"%s\">%s</button></a></p>"
      "</body></html>",
      t, state_cls, on ? "ON" : "OFF", param, state_cls, button_txt);
}

/* ---------- estado do cliente ------------------------------------------*/

/**
 * @struct client_t
 * @brief  Estrutura para armazenar o estado de uma conexão de cliente individual.
 */
typedef struct {
    struct tcp_pcb *pcb;    ///< Ponteiro para o Bloco de Controle de Protocolo (PCB) do lwIP.
    char            hdr[128]; ///< Buffer para os cabeçalhos HTTP.
    char            body[1024];///< Buffer para o corpo da página HTML.
    int             hdr_len, body_len, sent; ///< Comprimentos e contagem de bytes enviados.
} client_t;

/* ---------- forward declarations ---------------------------------------*/
// Declarações antecipadas das funções de callback para que possam ser usadas antes de suas definições.
static err_t  on_accept(void *arg, struct tcp_pcb *new_pcb, err_t err);
static err_t  on_recv  (void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t  on_sent  (void *arg, struct tcp_pcb *pcb, u16_t len);
static err_t  on_poll  (void *arg, struct tcp_pcb *pcb);
static void   close_cli(struct tcp_pcb *pcb, client_t *st);

/* ---------- servidor ----------------------------------------------------*/

/// @brief PCB global para o servidor de escuta (listening).
static struct tcp_pcb *srv_pcb = NULL;

/**
 * @brief Inicia o servidor HTTP.
 * @param port Porta TCP para escutar.
 * @return `true` em sucesso, `false` em falha.
 */
bool web_server_start(int port)
{
    // Cria um novo PCB para TCP em qualquer tipo de endereço IP (IPv4 ou IPv6).
    srv_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!srv_pcb) return false;

    // Associa (bind) o PCB a qualquer endereço IP local e à porta especificada.
    if (tcp_bind(srv_pcb, IP_ANY_TYPE, port) != ERR_OK) { 
        tcp_close(srv_pcb); 
        return false; 
    }

    // Coloca o servidor no estado de escuta (LISTEN), com um backlog de 4 conexões pendentes.
    srv_pcb = tcp_listen_with_backlog(srv_pcb, 4);

    // Registra a função `on_accept` para ser chamada quando um novo cliente se conectar.
    // Este é o coração do modelo de programação assíncrono do lwIP.
    tcp_accept(srv_pcb, on_accept);
    printf("[HTTP] Escutando na porta %d\n", port);
    return true;
}

/**
 * @brief Para o servidor HTTP.
 */
void web_server_stop(void)
{
    if (srv_pcb) { 
        tcp_close(srv_pcb); 
        srv_pcb = NULL; 
    }
}

/**
 * @brief Função de poll para o servidor.
 */
void web_server_poll(void) { /* Vazio, pois tudo é gerenciado por callbacks. */ }

/* ---------- callbacks ---------------------------------------------------*/

/**
 * @brief Callback chamado quando uma nova conexão TCP é aceita.
 * @param arg Argumento opcional (não usado aqui).
 * @param pcb O PCB da nova conexão.
 * @param err Código de erro.
 * @return ERR_OK em sucesso.
 */
static err_t on_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
    if (err != ERR_OK || !pcb) return ERR_VAL;
    
    // Aloca memória para a estrutura de estado do novo cliente.
    client_t *st = calloc(1, sizeof(client_t));
    if (!st) return ERR_MEM;

    // Registra os callbacks para esta conexão específica.
    tcp_arg (pcb, st);       // Associa o estado `st` a esta conexão.
    tcp_recv(pcb, on_recv);  // Função a ser chamada quando dados forem recebidos.
    tcp_sent(pcb, on_sent);  // Função a ser chamada quando dados forem enviados com sucesso.
    tcp_poll(pcb, on_poll, 10); // Função a ser chamada periodicamente.
    printf("[HTTP] Cliente %s conectado\n", ipaddr_ntoa(&pcb->remote_ip));
    return ERR_OK;
}

/**
 * @brief Callback chamado quando dados são recebidos de um cliente.
 * @param arg Estado do cliente (`client_t`).
 * @param pcb PCB da conexão.
 * @param p  Buffer (`pbuf`) com os dados recebidos. Se NULL, o cliente fechou a conexão.
 * @param err Código de erro.
 * @return ERR_OK em sucesso.
 */
static err_t on_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    client_t *st = arg;
    // Se p for NULL, o cliente fechou a conexão.
    if (!p) { 
        close_cli(pcb, st); 
        return ERR_OK; 
    }

    /* Análise super simples da requisição GET */
    char req[64] = {0};
    pbuf_copy_partial(p, req, sizeof req - 1, 0); // Copia o início da requisição para um buffer local.
    pbuf_free(p); // Libera o buffer de recepção.

    // Procura pelo parâmetro "?led=" na requisição.
    char *q = strstr(req, "?led=");
    if (q) { 
        q += 5; // Avança o ponteiro para depois de "?led=".
        if (!strncmp(q, "on", 2)) {
            led_set(true); // Se o valor for "on", liga o LED.
        } else if (!strncmp(q, "off", 3)) {
            led_set(false); // Se for "off", desliga o LED.
        }
    }

    // Prepara a resposta HTTP.
    st->body_len = make_page(st->body, sizeof st->body);
    st->hdr_len  = snprintf(st->hdr, sizeof st->hdr,
        "HTTP/1.1 200 OK\r\nContent-Length: %d\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n",
        st->body_len);

    // Envia o cabeçalho e o corpo da resposta.
    st->sent = 0;
    tcp_write(pcb, st->hdr,  st->hdr_len, 0);
    tcp_write(pcb, st->body, st->body_len, TCP_WRITE_FLAG_COPY);
    
    // Imprime uma mensagem de depuração indicando que uma requisição foi tratada.
    debug_status("HTTP");
    return ERR_OK;
}

/**
 * @brief Callback chamado quando o envio de dados é confirmado pelo cliente (ACK).
 * @param arg Estado do cliente.
 * @param pcb PCB da conexão.
 * @param len Número de bytes confirmados.
 * @return ERR_OK em sucesso.
 */
static err_t on_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    client_t *st = arg;
    st->sent += len;
    // Se todos os dados (cabeçalho + corpo) foram enviados, fecha a conexão.
    if (st->sent >= st->hdr_len + st->body_len) {
        close_cli(pcb, st);
    }
    return ERR_OK;
}

/**
 * @brief Callback de poll, chamado periodicamente. Usado como um timeout.
 * @details Se esta função for chamada, significa que a conexão está ociosa.
 * Neste caso, optamos por fechá-la para liberar recursos.
 * @param arg Estado do cliente.
 * @param pcb PCB da conexão.
 * @return ERR_OK.
 */
static err_t on_poll(void *arg, struct tcp_pcb *pcb) { 
    close_cli(pcb, arg); 
    return ERR_OK; 
}

/**
 * @brief Fecha uma conexão de cliente e libera os recursos associados.
 * @param pcb PCB da conexão a ser fechada.
 * @param st Estado do cliente a ser liberado.
 */
static void close_cli(struct tcp_pcb *pcb, client_t *st)
{
    if (pcb) {
        // Desregistra todos os callbacks para evitar chamadas futuras em um PCB inválido.
        tcp_arg (pcb, NULL); 
        tcp_err(pcb, NULL);
        tcp_recv(pcb, NULL); 
        tcp_sent(pcb, NULL); 
        tcp_poll(pcb, NULL, 0);
        // Fecha a conexão TCP.
        tcp_close(pcb);
    }
    // Libera a memória da estrutura de estado do cliente.
    free(st);
}