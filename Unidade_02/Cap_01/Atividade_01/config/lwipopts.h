/**
 * @file lwipopts.h
 * @brief Arquivo de opções de configuração para a pilha TCP/IP LwIP (Lightweight IP).
 *
 * Este arquivo define várias macros que controlam o comportamento e os recursos
 * da pilha LwIP, como gerenciamento de memória, protocolos habilitados (TCP, UDP, DHCP, etc.),
 * tamanhos de buffer e opções de depuração. Estas são configurações comuns usadas
 * na maioria dos exemplos do Pico W.
 *
 * Para mais detalhes sobre as opções, consulte:
 * https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

// Configurações comuns usadas na maioria dos exemplos do pico_w

// Permite sobrescrever em alguns exemplos. NO_SYS=1 significa que LwIP roda sem um sistema operacional (bare metal).
#ifndef NO_SYS
#define NO_SYS                      1  /**< Define se LwIP opera com ou sem um SO. 1 = sem SO (bare metal). */
#endif
// Permite sobrescrever em alguns exemplos. LWIP_SOCKET=0 desabilita a API de sockets tipo BSD.
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0  /**< Habilita (1) ou desabilita (0) a API de Sockets (compatível com BSD). */
#endif
#if PICO_CYW43_ARCH_POLL
#define MEM_LIBC_MALLOC             1  /**< Se PICO_CYW43_ARCH_POLL estiver definido, usa malloc/free da libc para alocação de memória. */
#else
// MEM_LIBC_MALLOC é incompatível com versões não-polling.
#define MEM_LIBC_MALLOC             0  /**< Se não estiver usando polling, LwIP usa seu próprio gerenciador de memória de heap. */
#endif
#define MEM_ALIGNMENT               4  /**< Alinhamento de memória em bytes. Geralmente 4 para arquiteturas de 32 bits. */
#define MEM_SIZE                    4000 /**< Tamanho total da heap gerenciada por LwIP (se MEM_LIBC_MALLOC for 0). */
#define MEMP_NUM_TCP_SEG            32 /**< Número de segmentos TCP que podem ser enfileirados para transmissão. */
#define MEMP_NUM_ARP_QUEUE          10 /**< Número de pacotes IP que podem ser enfileirados para resolução ARP. */
#define PBUF_POOL_SIZE              24 /**< Número de pbufs no pool de pbufs. Pbufs são usados para armazenar dados de pacotes. */
#define LWIP_ARP                    1  /**< Habilita o protocolo ARP (Address Resolution Protocol). */
#define LWIP_ETHERNET               1  /**< Indica que LwIP está sendo usado com uma interface Ethernet (ou similar, como Wi-Fi). */
#define LWIP_ICMP                   1  /**< Habilita o protocolo ICMP (Internet Control Message Protocol), usado por ex. pelo ping. */
#define LWIP_RAW                    1  /**< Habilita a API de sockets crus (raw sockets). */
#define TCP_WND                     (8 * TCP_MSS) /**< Tamanho da janela de recepção TCP (TCP window size). Múltiplo do MSS. */
#define TCP_MSS                     1460 /**< Tamanho Máximo de Segmento TCP (Maximum Segment Size). */
#define TCP_SND_BUF                 (8 * TCP_MSS) /**< Tamanho do buffer de envio TCP. */
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS)) /**< Número máximo de pbufs na fila de envio TCP. */
#define LWIP_NETIF_STATUS_CALLBACK  1  /**< Habilita callbacks para mudança de status da interface de rede (ex: link up/down). */
#define LWIP_NETIF_LINK_CALLBACK    1  /**< Habilita callbacks para mudança de status do link da interface de rede. */
#define LWIP_NETIF_HOSTNAME         1  /**< Habilita o suporte a hostname para a interface de rede. */
#define LWIP_NETCONN                0  /**< Desabilita a API Netconn (API sequencial de alto nível). Socket API é geralmente preferida ou NO_SYS. */
#define MEM_STATS                   0  /**< Desabilita estatísticas de memória do heap principal. */
#define SYS_STATS                   0  /**< Desabilita estatísticas do sistema operacional (se NO_SYS=0). */
#define MEMP_STATS                  0  /**< Desabilita estatísticas dos pools de memória LwIP. */
#define LINK_STATS                  0  /**< Desabilita estatísticas da camada de enlace. */
// #define ETH_PAD_SIZE                2 /**< Se definido, adiciona um padding de 2 bytes ao frame Ethernet para alinhar o payload IP. Descomente se necessário. */
#define LWIP_CHKSUM_ALGORITHM       3  /**< Algoritmo de checksum. 3 geralmente significa usar um otimizado. */
#define LWIP_DHCP                   1  /**< Habilita o cliente DHCP (Dynamic Host Configuration Protocol) para obter IP automaticamente. */
#define LWIP_IPV4                   1  /**< Habilita o protocolo IPv4. */
#define LWIP_TCP                    1  /**< Habilita o protocolo TCP (Transmission Control Protocol). */
#define LWIP_UDP                    1  /**< Habilita o protocolo UDP (User Datagram Protocol). */
#define LWIP_DNS                    1  /**< Habilita o cliente DNS (Domain Name System). */
#define LWIP_TCP_KEEPALIVE          1  /**< Habilita pacotes TCP keepalive para manter conexões ativas. */
#define LWIP_NETIF_TX_SINGLE_PBUF   1  /**< Otimização para interfaces que enviam um único pbuf por vez. */
#define DHCP_DOES_ARP_CHECK         0  /**< Desabilita a verificação ARP pelo DHCP antes de usar um endereço IP oferecido. */
#define LWIP_DHCP_DOES_ACD_CHECK    0  /**< Desabilita a verificação de conflito de endereço (Address Conflict Detection) pelo DHCP. */

#ifndef NDEBUG // Se não estiver em modo NDEBUG (ou seja, DEBUG está ativo)
#define LWIP_DEBUG                  1  /**< Habilita a infraestrutura de depuração global da LwIP. */
#define LWIP_STATS                  1  /**< Habilita a coleta de estatísticas da LwIP. */
#define LWIP_STATS_DISPLAY          1  /**< Habilita funções para exibir as estatísticas coletadas. */
#endif

/**
 * @name Opções de Depuração por Módulo
 * @brief Controla o nível de mensagens de depuração para cada módulo da LwIP.
 * LWIP_DBG_OFF desabilita as mensagens de depuração para o módulo específico.
 * Outros níveis incluem LWIP_DBG_ON, LWIP_DBG_MIN_LEVEL, etc.
 * @{
 */
#define ETHARP_DEBUG                LWIP_DBG_OFF /**< Depuração para o módulo ARP e Ethernet. */
#define NETIF_DEBUG                 LWIP_DBG_OFF /**< Depuração para o módulo de interfaces de rede. */
#define PBUF_DEBUG                  LWIP_DBG_OFF /**< Depuração para o módulo de gerenciamento de pbuf. */
#define API_LIB_DEBUG               LWIP_DBG_OFF /**< Depuração para a API de sockets. */
#define API_MSG_DEBUG               LWIP_DBG_OFF /**< Depuração para a API de mensagens (Netconn). */
#define SOCKETS_DEBUG               LWIP_DBG_OFF /**< Depuração específica para sockets. */
#define ICMP_DEBUG                  LWIP_DBG_OFF /**< Depuração para o módulo ICMP. */
#define INET_DEBUG                  LWIP_DBG_OFF /**< Depuração para funções inet (ex: (ntoh, hton). */
#define IP_DEBUG                    LWIP_DBG_OFF /**< Depuração para o módulo IP. */
#define IP_REASS_DEBUG              LWIP_DBG_OFF /**< Depuração para a remontagem de fragmentos IP. */
#define RAW_DEBUG                   LWIP_DBG_OFF /**< Depuração para sockets crus. */
#define MEM_DEBUG                   LWIP_DBG_OFF /**< Depuração para o gerenciador de heap (mem.c). */
#define MEMP_DEBUG                  LWIP_DBG_OFF /**< Depuração para os pools de memória (memp.c). */
#define SYS_DEBUG                   LWIP_DBG_OFF /**< Depuração para a camada de abstração do SO (sys.c). */
#define TCP_DEBUG                   LWIP_DBG_OFF /**< Depuração geral para o módulo TCP. */
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF /**< Depuração para a entrada de pacotes TCP. */
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF /**< Depuração para a saída de pacotes TCP. */
#define TCP_RTO_DEBUG               LWIP_DBG_OFF /**< Depuração para o cálculo do RTO (Retransmission Timeout) TCP. */
#define TCP_CWND_DEBUG              LWIP_DBG_OFF /**< Depuração para o controle de congestionamento TCP (Congestion Window). */
#define TCP_WND_DEBUG               LWIP_DBG_OFF /**< Depuração para o controle de janela TCP (Window). */
#define TCP_FR_DEBUG                LWIP_DBG_OFF /**< Depuração para a recuperação rápida TCP (Fast Retransmit/Fast Recovery). */
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF /**< Depuração para o tamanho das filas TCP. */
#define TCP_RST_DEBUG               LWIP_DBG_OFF /**< Depuração para o tratamento de RST (Reset) TCP. */
#define UDP_DEBUG                   LWIP_DBG_OFF /**< Depuração para o módulo UDP. */
#define TCPIP_DEBUG                 LWIP_DBG_OFF /**< Depuração para o thread principal tcpip_thread. */
#define PPP_DEBUG                   LWIP_DBG_OFF /**< Depuração para o protocolo PPP. */
#define SLIP_DEBUG                  LWIP_DBG_OFF /**< Depuração para o protocolo SLIP. */
#define DHCP_DEBUG                  LWIP_DBG_OFF /**< Depuração para o módulo DHCP. */
/** @} */

#endif /* __LWIPOPTS_H__ */