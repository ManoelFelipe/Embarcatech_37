/**
 * @file network_manager.h
 * @brief Interface para gerenciamento da rede Wi-Fi e servidor HTTP.
 *
 * Este módulo configura o Pico W como Access Point, gerencia os servidores
 * DHCP e DNS, e lida com as conexões e requisições do servidor HTTP.
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 */

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "pico/cyw43_arch.h" // Para ip_addr_t e outras dependências de rede
#include <stdbool.h>

/**
 * @brief Estrutura de estado para o servidor TCP/AP.
 * Usada internamente pelo network_manager e potencialmente pela função main
 * para sinalizar o encerramento.
 */
typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb; /**< Protocol Control Block (PCB) para o servidor TCP. */
    bool complete;              /**< Flag para indicar se o servidor/aplicação deve terminar. */
    ip_addr_t gw;               /**< Endereço IP do gateway (o próprio Pico W no modo AP). */
    // Adicione outros campos de estado global da rede aqui se necessário.
    // Instâncias dos servidores DHCP e DNS serão gerenciadas internamente no .c
} TCP_SERVER_T;


/**
 * @brief Inicializa o gerenciador de rede.
 *
 * Configura o Wi-Fi em modo Access Point, inicia os servidores DHCP e DNS,
 * e abre o servidor TCP para escutar por conexões HTTP.
 *
 * @param state Ponteiro para a estrutura de estado do servidor TCP, que será gerenciada.
 * @return Verdadeiro se a inicialização for bem-sucedida, falso caso contrário.
 */
bool network_manager_init(TCP_SERVER_T *state);

/**
 * @brief Desinicializa o gerenciador de rede.
 *
 * Fecha o servidor TCP, desabilita o modo AP, e desinicializa os servidores
 * DHCP, DNS e a arquitetura CYW43.
 *
 * @param state Ponteiro para a estrutura de estado do servidor TCP.
 */
void network_manager_deinit(TCP_SERVER_T *state);

#endif // NETWORK_MANAGER_H