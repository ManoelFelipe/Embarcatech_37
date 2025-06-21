/**
 * @file    dnsserver.h
 * @brief   Interface pública para um servidor DNS "catch-all".
 * @details Este arquivo de cabeçalho define a estrutura de dados e as funções públicas
 * para inicializar e desinicializar um servidor DNS simples. O propósito
 * deste servidor é responder a todas as consultas de nomes de domínio com
 * um único endereço IP, uma técnica comumente usada para criar um
 * "captive portal" (portal cativo).
 *
 * @note    Copyright (c) 2022 Raspberry Pi (Trading) Ltd. (Licença BSD-3-Clause).
 */

#ifndef _DNSSERVER_H_
#define _DNSSERVER_H_

#include "lwip/ip_addr.h" // Inclui a definição do tipo `ip_addr_t` da pilha de rede LwIP.

/**
 * @struct dns_server_t_
 * @brief  Estrutura que armazena o estado de uma instância do servidor DNS.
 */
typedef struct dns_server_t_ {
    struct udp_pcb *udp; ///< Ponteiro para o PCB (Protocol Control Block) UDP da LwIP. Representa a conexão UDP.
     ip_addr_t ip;       ///< O endereço IP que será retornado em todas as respostas DNS.
} dns_server_t;

/**
 * @brief Inicializa o servidor DNS.
 * @details Configura e inicia o servidor DNS, vinculando-o à porta padrão 53
 * e preparando-o para receber consultas.
 * @param d Ponteiro para a estrutura de estado do servidor (`dns_server_t`) a ser inicializada.
 * @param ip O endereço IP que o servidor deve usar como resposta para todas as consultas.
 */
void dns_server_init(dns_server_t *d, ip_addr_t *ip);

/**
 * @brief Desinicializa o servidor DNS.
 * @details Para o servidor e libera todos os recursos de rede associados,
 * como o PCB UDP.
 * @param d Ponteiro para a estrutura de estado do servidor a ser finalizada.
 */
void dns_server_deinit(dns_server_t *d);

#endif // _DNSSERVER_H_