/**
 * @file    dhcpserver.h
 * @brief   Interface pública e definições para um servidor DHCP simples.
 * @details Este arquivo, originalmente do projeto MicroPython, define a configuração
 * (faixa de IPs), as estruturas de dados para gerenciar as concessões de IP
 * (leases) e o estado geral do servidor DHCP.
 *
 * @note    Este arquivo faz parte do projeto MicroPython, http://micropython.org/
 * Copyright (c) 2018-2019 Damien P. George (Licença MIT).
 */

#ifndef MICROPY_INCLUDED_LIB_NETUTILS_DHCPSERVER_H
#define MICROPY_INCLUDED_LIB_NETUTILS_DHCPSERVER_H

#include "lwip/ip_addr.h" // Inclui a definição do tipo `ip_addr_t` da LwIP.

/**
 * @def DHCPS_BASE_IP
 * @brief Define o octeto inicial da faixa de IPs a serem distribuídos.
 * @details O endereço IP final será o IP do gateway com o último octeto
 * substituído por este valor. Ex: se o gateway é 192.168.4.1,
 * o primeiro IP a ser concedido será 192.168.4.16.
 */
#define DHCPS_BASE_IP (16)

/**
 * @def DHCPS_MAX_IP
 * @brief Define o número máximo de concessões (e, portanto, de IPs) que o servidor pode gerenciar.
 * @details Com BASE_IP=16 e MAX_IP=8, a faixa de IPs será de 192.168.4.16 a 192.168.4.23.
 */
#define DHCPS_MAX_IP (8)

/**
 * @struct _dhcp_server_lease_t
 * @brief  Estrutura que representa a concessão (lease) de um endereço IP para um cliente.
 */
typedef struct _dhcp_server_lease_t {
    uint8_t mac[6];     ///< O endereço MAC (identificador único) do cliente que recebeu o lease.
    uint16_t expiry;    ///< Marcador de tempo de expiração do lease. Armazena os 16 bits mais significativos
                        ///< do contador de milissegundos (`cyw43_hal_ticks_ms() >> 16`).
} dhcp_server_lease_t;

/**
 * @struct _dhcp_server_t
 * @brief  Estrutura que armazena o estado completo do servidor DHCP.
 */
typedef struct _dhcp_server_t {
    ip_addr_t ip;       ///< Endereço IP do próprio servidor (que também é o gateway da rede).
    ip_addr_t nm;       ///< Máscara de sub-rede (netmask) da rede.
    dhcp_server_lease_t lease[DHCPS_MAX_IP]; ///< Array com as concessões de IP gerenciadas.
    struct udp_pcb *udp; ///< Ponteiro para o PCB (Protocol Control Block) UDP da LwIP para a comunicação DHCP.
} dhcp_server_t;

/**
 * @brief Inicializa o servidor DHCP.
 * @details Configura o estado do servidor, cria o socket UDP e o vincula à porta 67.
 * @param d Ponteiro para a estrutura de estado do servidor a ser inicializada.
 * @param ip Endereço IP do servidor/gateway.
 * @param nm Máscara de sub-rede.
 */
void dhcp_server_init(dhcp_server_t *d, ip_addr_t *ip, ip_addr_t *nm);

/**
 * @brief Desinicializa o servidor DHCP.
 * @details Para o servidor e libera os recursos de rede associados.
 * @param d Ponteiro para a estrutura de estado do servidor a ser finalizada.
 */
void dhcp_server_deinit(dhcp_server_t *d);

#endif // MICROPY_INCLUDED_LIB_NETUTILS_DHCPSERVER_H