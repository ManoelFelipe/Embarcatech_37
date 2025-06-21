/**
 * @file    dnsserver.c
 * @brief   Implementação de um servidor DNS "catch-all" para captive portal.
 * @details Este servidor DNS escuta na porta 53 e responde a qualquer consulta
 * DNS padrão (query tipo A) com um único endereço IP pré-configurado.
 * Isso força qualquer dispositivo cliente que tente acessar um site
 * (ex: google.com) a ser redirecionado para o IP do servidor web
 * hospedado no Pico, criando assim um portal cativo.
 *
 * @note    Copyright (c) 2022 Raspberry Pi (Trading) Ltd. (Licença BSD-3-Clause).
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>

#include "dnsserver.h"      // Definições da interface deste módulo.
#include "lwip/udp.h"       // API da camada UDP da pilha de rede LwIP.

#define PORT_DNS_SERVER 53  ///< Porta padrão para o serviço DNS.
#define DUMP_DATA 0         ///< Flag para depuração. Se 1, imprime o conteúdo dos pacotes.

// Macros para controle de mensagens de depuração.
#define DEBUG_printf(...)   ///< Desativado para não poluir o console.
#define ERROR_printf printf ///< Ativado para imprimir mensagens de erro.

/**
 * @struct dns_header_t_
 * @brief Representação do cabeçalho de uma mensagem DNS, conforme RFC 1035.
 */
typedef struct dns_header_t_ {
    uint16_t id;                    ///< Identificador da transação. A resposta deve ter o mesmo ID da consulta.
    uint16_t flags;                 ///< Flags que controlam a operação (se é query/response, se é autoritativo, etc.).
    uint16_t question_count;        ///< Número de perguntas na seção "Question". Geralmente 1.
    uint16_t answer_record_count;   ///< Número de registros de resposta na seção "Answer".
    uint16_t authority_record_count;///< Número de registros na seção "Authority".
    uint16_t additional_record_count;///< Número de registros na seção "Additional".
} dns_header_t;

#define MAX_DNS_MSG_SIZE 300 ///< Tamanho máximo do buffer para uma mensagem DNS.

/**
 * @brief Função utilitária para criar um novo socket (PCB) UDP.
 */
static int dns_socket_new_dgram(struct udp_pcb **udp, void *cb_data, udp_recv_fn cb_udp_recv) {
    *udp = udp_new(); // Aloca um novo Protocol Control Block (PCB) para UDP.
    if (*udp == NULL) {
        return -ENOMEM; // Erro: Sem memória.
    }
    // Registra a função `cb_udp_recv` para ser chamada quando um pacote for recebido neste PCB.
    udp_recv(*udp, cb_udp_recv, (void *)cb_data);
    return ERR_OK;
}

/**
 * @brief Função utilitária para liberar um socket (PCB) UDP.
 */
static void dns_socket_free(struct udp_pcb **udp) {
    if (*udp != NULL) {
        udp_remove(*udp); // Libera o PCB.
        *udp = NULL;
    }
}

/**
 * @brief Função utilitária para vincular (bind) um socket UDP a um IP e porta.
 * @note  Esta implementação específica está vinculando a um IP numérico, o que é incomum.
 * A versão mais comum usa `udp_bind(*udp, IP_ANY_TYPE, port);` para escutar em todas as interfaces.
 */
static int dns_socket_bind(struct udp_pcb **udp, uint32_t ip, uint16_t port) {
    ip_addr_t addr;
    // Converte o endereço IP de um formato `uint32_t` para `ip_addr_t`.
    IP4_ADDR(&addr, ip >> 24 & 0xff, ip >> 16 & 0xff, ip >> 8 & 0xff, ip & 0xff);
    // Vincula o PCB ao endereço e porta.
    err_t err = udp_bind(*udp, &addr, port);
    if (err != ERR_OK) {
        ERROR_printf("dns failed to bind to port %u: %d", port, err);
        assert(false); // Em caso de falha, para a execução (requer build de debug).
    }
    return err;
}

#if DUMP_DATA // Bloco de código compilado apenas se DUMP_DATA for 1.
/**
 * @brief Função de depuração para imprimir o conteúdo de um buffer em hexadecimal.
 */
static void dump_bytes(const uint8_t *bptr, uint32_t len) {
    unsigned int i = 0;

    for (i = 0; i < len;) {
        if ((i & 0x0f) == 0) {
            printf("\n");
        } else if ((i & 0x07) == 0) {
            printf(" ");
        }
        printf("%02x ", bptr[i++]);
    }
    printf("\n");
}
#endif

/**
 * @brief Função utilitária para enviar um pacote UDP.
 */
static int dns_socket_sendto(struct udp_pcb **udp, const void *buf, size_t len, const ip_addr_t *dest, uint16_t port) {
    if (len > 0xffff) {
        len = 0xffff; // Garante que o tamanho cabe em 16 bits.
    }

    // Aloca um buffer de pacote (pbuf) da LwIP.
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (p == NULL) {
        ERROR_printf("DNS: Failed to send message out of memory\n");
        return -ENOMEM;
    }

    memcpy(p->payload, buf, len); // Copia os dados para o pbuf.
    err_t err = udp_sendto(*udp, p, dest, port); // Envia o pacote.

    pbuf_free(p); // Libera o pbuf.

    if (err != ERR_OK) {
        ERROR_printf("DNS: Failed to send message %d\n", err);
        return err;
    }

#if DUMP_DATA // Se habilitado, imprime os dados enviados.
    dump_bytes(buf, len);
#endif
    return len;
}

/**
 * @brief Função principal de processamento, chamada pela LwIP quando um pacote DNS chega.
 */
static void dns_server_process(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *src_addr, u16_t src_port) {
    dns_server_t *d = arg; // Recupera o estado do servidor.
    DEBUG_printf("dns_server_process %u\n", p->tot_len);

    uint8_t dns_msg[MAX_DNS_MSG_SIZE]; // Buffer local para a mensagem DNS.
    dns_header_t *dns_hdr = (dns_header_t*)dns_msg;

    // Copia o pacote recebido (do pbuf) para o buffer local.
    size_t msg_len = pbuf_copy_partial(p, dns_msg, sizeof(dns_msg), 0);
    if (msg_len < sizeof(dns_header_t)) {
        goto ignore_request; // Pacote muito pequeno para ser válido.
    }

#if DUMP_DATA
    dump_bytes(dns_msg, msg_len); // Imprime o pacote recebido se a depuração estiver ativa.
#endif

    // Converte os campos do cabeçalho de Network Byte Order para Host Byte Order.
    uint16_t flags = lwip_ntohs(dns_hdr->flags);
    uint16_t question_count = lwip_ntohs(dns_hdr->question_count);

    DEBUG_printf("len %d\n", msg_len);
    DEBUG_printf("dns flags 0x%x\n", flags);
    DEBUG_printf("dns question count 0x%x\n", question_count);

    // --- Validação da Consulta DNS ---
    // RFC 1035 - Formato do Cabeçalho
    // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    // |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    // +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    // Verifica o bit QR (Query/Response). Deve ser 0 para uma consulta.
    if (((flags >> 15) & 0x1) != 0) {
        DEBUG_printf("Ignoring non-query\n");
        goto ignore_request;
    }

    // Verifica o Opcode. Deve ser 0 para uma consulta padrão (QUERY).
    if (((flags >> 11) & 0xf) != 0) {
        DEBUG_printf("Ignoring non-standard query\n");
        goto ignore_request;
    }

    // A consulta deve ter pelo menos uma pergunta.
    if (question_count < 1) {
        DEBUG_printf("Invalid question count\n");
        goto ignore_request;
    }

    // --- Processamento da Seção "Question" ---
    // O objetivo aqui não é entender a pergunta, mas sim pular por ela
    // para encontrar onde a seção de resposta ("Answer") deve começar.
    const uint8_t *question_ptr_start = dns_msg + sizeof(dns_header_t);
    const uint8_t *question_ptr_end = dns_msg + msg_len;
    const uint8_t *question_ptr = question_ptr_start;
    while(question_ptr < question_ptr_end) {
        if (*question_ptr == 0) { // O fim do nome de domínio é marcado por um byte de comprimento 0.
            question_ptr++;
            break;
        } else {
            // Um nome de domínio é uma sequência de "labels", cada um precedido por seu comprimento.
            // Ex: "www.google.com" é [3]www[6]google[3]com[0]
            if (question_ptr > question_ptr_start) { DEBUG_printf("."); }
            int label_len = *question_ptr++;
            if (label_len > 63) { goto ignore_request; } // Label muito longo.
            DEBUG_printf("%.*s", label_len, question_ptr);
            question_ptr += label_len; // Pula o label.
        }
    }

    // Valida o comprimento total da pergunta.
    if (question_ptr - question_ptr_start > 255) {
        goto ignore_request;
    }

    // Pula os campos QTYPE (2 bytes) e QCLASS (2 bytes) que vêm após o nome.
    question_ptr += 4;

    // --- Geração da Resposta ---
    // `answer_ptr` agora aponta para o local onde a seção "Answer" começa.
    uint8_t *answer_ptr = dns_msg + (question_ptr - dns_msg);
    // Escreve a resposta (Resource Record - RR).

    // NOME: Usa compressão de mensagem DNS. 0xc0 seguido de um offset.
    // 0xc00c significa "ponteiro para o offset 12", que é o início da seção
    // de pergunta, reutilizando o nome de domínio da consulta.
    *answer_ptr++ = 0xc0;
    *answer_ptr++ = question_ptr_start - dns_msg;
    
    // TIPO: A (1) - um endereço de host IPv4.
    *answer_ptr++ = 0;
    *answer_ptr++ = 1;

    // CLASSE: IN (1) - para Internet.
    *answer_ptr++ = 0;
    *answer_ptr++ = 1;

    // TTL (Time To Live): 60 segundos.
    *answer_ptr++ = 0;
    *answer_ptr++ = 0;
    *answer_ptr++ = 0;
    *answer_ptr++ = 60;

    // RDLENGTH (Comprimento dos Dados): 4 bytes para um IPv4.
    *answer_ptr++ = 0;
    *answer_ptr++ = 4;
    // RDATA (Dados): O endereço IP do nosso servidor. É aqui que a "mágica" acontece.
    memcpy(answer_ptr, &d->ip.addr, 4);
    answer_ptr += 4;

    // --- Modificação do Cabeçalho para ser uma Resposta ---
    dns_hdr->flags = lwip_htons(
                0x1 << 15 | // QR = 1 (Response)
                0x1 << 10 | // AA = 1 (Authoritative Answer)
                0x1 << 7);  // RA = 1 (Recursion Available) - pode ser 0, mas não importa muito aqui.
    dns_hdr->question_count = lwip_htons(1);
    dns_hdr->answer_record_count = lwip_htons(1); // Temos uma resposta.
    dns_hdr->authority_record_count = 0;
    dns_hdr->additional_record_count = 0;

    // Envia a resposta de volta para o cliente que fez a consulta.
    dns_socket_sendto(&d->udp, &dns_msg, answer_ptr - dns_msg, src_addr, src_port);

// Rótulo para pular o processamento e apenas liberar o buffer.
ignore_request:
    pbuf_free(p); // É crucial liberar o pbuf recebido para evitar vazamento de memória.
}

/**
 * @brief Função pública para inicializar o servidor DNS.
 */
void dns_server_init(dns_server_t *d, ip_addr_t *ip) {
    if (dns_socket_new_dgram(&d->udp, d, dns_server_process) != ERR_OK) {
        return; // Falha ao criar o socket.
    }
    // Vincula o socket à porta 53 em qualquer endereço (0).
    // A implementação de `dns_socket_bind` usa o IP 0, que na LwIP se traduz para
    // IP_ADDR_ANY, ou seja, escutar em todas as interfaces de rede disponíveis.
    if (dns_socket_bind(&d->udp, 0, PORT_DNS_SERVER) != ERR_OK) {
        return; // Falha ao vincular.
    }
    ip_addr_copy(d->ip, *ip); // Armazena o endereço IP que será usado nas respostas.
}

/**
 * @brief Função pública para desinicializar o servidor DNS.
 */
void dns_server_deinit(dns_server_t *d) {
    dns_socket_free(&d->udp);
}