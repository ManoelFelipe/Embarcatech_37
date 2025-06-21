/**
 * @file lwipopts.h
 * @brief Arquivo de configuração da pilha de rede LwIP para o Raspberry Pi Pico W.
 * @version 1.0
 * @date 06 de Junho de 2025
 * * @details
 * Este arquivo é fundamental para o funcionamento do LwIP. Ele permite personalizar
 * a pilha TCP/IP para se adequar às restrições de memória e aos requisitos de
 * desempenho do hardware. Cada macro `#define` habilita, desabilita ou ajusta
 * uma funcionalidade específica.
 *
 * As configurações aqui presentes são um ponto de partida comum para os exemplos
 * do Pico W, buscando um equilíbrio entre funcionalidade e uso de recursos.
 *
 * Para uma lista completa e detalhada de todas as opções, consulte a documentação oficial do LwIP:
 * https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

//==================================================================================================
/** @brief Configurações do Sistema Operacional (OS Abstraction Layer) */
//==================================================================================================

// NO_SYS = 1: Indica que o LwIP está rodando em modo "bare-metal", ou seja, sem um
// sistema operacional multitarefa (como FreeRTOS). Neste modo, o processamento da
// pilha de rede é feito em um loop principal (super-loop) através de polling.
#ifndef NO_SYS
#define NO_SYS                      1
#endif

// LWIP_SOCKET = 0: Desabilita a API de Sockets compatível com Berkeley (POSIX).
// Em vez dela, estamos usando a API "RAW" nativa do LwIP, que é baseada em callbacks.
// A API RAW é mais leve e mais eficiente em termos de memória, ideal para sistemas embarcados.
#ifndef LWIP_SOCKET
#define LWIP_SOCKET                 0
#endif

// LWIP_NETCONN = 0: Desabilita a API Netconn, que é uma camada de abstração sequencial
// sobre a API RAW. Ela só é necessária se você estiver usando a API de Sockets ou
// um RTOS com certas funcionalidades. Como desabilitamos os Sockets e estamos em modo
// NO_SYS, ela não é necessária.
#define LWIP_NETCONN                0


//==================================================================================================
/** @brief Configurações de Gerenciamento de Memória (Heap e Pools) */
//==================================================================================================

#if PICO_CYW43_ARCH_POLL
// MEM_LIBC_MALLOC = 1: Quando em modo de polling, instrui o LwIP a usar as
// funções de alocação de memória padrão da biblioteca C (`malloc`, `free`).
#define MEM_LIBC_MALLOC             1
#else
// Em outros modos (não polling), o LwIP usa seu próprio gerenciador de heap,
// pois o `malloc` da libc pode não ser thread-safe.
#define MEM_LIBC_MALLOC             0
#endif

// MEM_ALIGNMENT = 4: Define o alinhamento de memória para 4 bytes. Essencial para
// arquiteturas de 32 bits como o ARM Cortex-M0+ do RP2040 para evitar acessos
// desalinhados que podem causar falhas ou degradação de performance.
#define MEM_ALIGNMENT               4

// MEM_SIZE = 4000: Define o tamanho total do heap que o LwIP pode usar, em bytes.
// Este valor é um balanço crítico entre a capacidade da rede (quantas conexões,
// quão grandes os buffers) e a RAM disponível para o resto da aplicação.
#define MEM_SIZE                    4000

// MEMP_NUM_TCP_SEG = 32: Número de segmentos TCP que podem ser enfileirados para
// transmissão ou retransmissão. Um valor maior permite mais dados "em trânsito",
// mas consome mais memória de pools estáticos.
#define MEMP_NUM_TCP_SEG            32


//==================================================================================================
/** @brief Configurações do Protocolo ARP */
//==================================================================================================

// LWIP_ARP = 1: Habilita o protocolo ARP (Address Resolution Protocol), que é
// fundamental para mapear endereços IP para endereços MAC em redes Ethernet.
#define LWIP_ARP                    1

// MEMP_NUM_ARP_QUEUE = 10: Número de pacotes que podem ser enfileirados
// aguardando a resolução de um endereço ARP. Se um destino IP não está na
// tabela ARP, o pacote é enfileirado aqui enquanto uma requisição ARP é enviada.
#define MEMP_NUM_ARP_QUEUE          10


//==================================================================================================
/** @brief Configurações de PBUF (Packet Buffer Management) */
//==================================================================================================

// PBUF_POOL_SIZE = 24: Número de buffers de pacotes (pbufs) disponíveis no pool
// principal. Pbufs são as estruturas de dados centrais do LwIP para manipular
// pacotes de rede. Este é um dos parâmetros mais críticos para o desempenho da rede.
// Se este valor for muito baixo, a rede pode parar de funcionar sob carga.
#define PBUF_POOL_SIZE              24

// LWIP_NETIF_TX_SINGLE_PBUF = 1: Otimização que permite que a pilha envie pacotes
// que estão contidos em um único pbuf sem precisar copiá-los para um novo pbuf
// alinhado, melhorando a performance de transmissão.
#define LWIP_NETIF_TX_SINGLE_PBUF   1


//==================================================================================================
/** @brief Configurações dos Protocolos de Rede (Camadas 2, 3 e 4) */
//==================================================================================================

// LWIP_ETHERNET = 1: Habilita o suporte para pacotes no formato Ethernet.
// Essencial, já que o Wi-Fi opera emulando uma camada Ethernet.
#define LWIP_ETHERNET               1

// LWIP_IPV4 = 1: Habilita o suporte para o Protocolo de Internet versão 4.
#define LWIP_IPV4                   1

// LWIP_ICMP = 1: Habilita o protocolo ICMP (Internet Control Message Protocol),
// usado principalmente pelo comando `ping`.
#define LWIP_ICMP                   1

// LWIP_RAW = 1: Habilita a API de baixo nível (RAW API), que é a que estamos usando
// para o servidor TCP. Ela é baseada em callbacks e é muito eficiente.
#define LWIP_RAW                    1

// LWIP_UDP = 1: Habilita o protocolo UDP (User Datagram Protocol). Necessário para
// o funcionamento do DHCP e do DNS.
#define LWIP_UDP                    1

// LWIP_TCP = 1: Habilita o protocolo TCP (Transmission Control Protocol), essencial
// para o nosso servidor HTTP.
#define LWIP_TCP                    1


//==================================================================================================
/** @brief Configurações de Desempenho do TCP */
//==================================================================================================

// TCP_MSS = 1460: Maximum Segment Size. O tamanho máximo de dados que um único
// segmento TCP pode conter. 1460 é o valor padrão para Ethernet (1500 MTU - 20 IP hdr - 20 TCP hdr).
#define TCP_MSS                     1460

// TCP_WND = (8 * TCP_MSS): Tamanho da "janela de recepção" TCP, em bytes.
// Informa ao outro lado da conexão quanto de dados ele pode nos enviar antes
// de receber uma confirmação (ACK). Uma janela maior pode melhorar a performance
// em redes com alta latência, mas consome mais RAM.
#define TCP_WND                     (8 * TCP_MSS)

// TCP_SND_BUF = (8 * TCP_MSS): Tamanho do buffer de envio por conexão TCP, em bytes.
// Define quantos dados podemos enfileirar para envio em uma única conexão.
#define TCP_SND_BUF                 (8 * TCP_MSS)

// TCP_SND_QUEUELEN: Calcula o número de pbufs que podem ser enfileirados para
// envio com base no tamanho do buffer de envio (TCP_SND_BUF).
#define TCP_SND_QUEUELEN            ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))

// LWIP_TCP_KEEPALIVE = 1: Habilita o envio de pacotes "keep-alive" em conexões TCP
// ociosas para verificar se o outro lado ainda está ativo. Ajuda a detectar
// conexões que foram interrompidas abruptamente.
#define LWIP_TCP_KEEPALIVE          1


//==================================================================================================
/** @brief Configurações de Interface de Rede (Netif) */
//==================================================================================================

// LWIP_NETIF_STATUS_CALLBACK = 1: Habilita um callback que é chamado quando o
// status da interface de rede muda (ex: quando um IP é atribuído).
#define LWIP_NETIF_STATUS_CALLBACK  1

// LWIP_NETIF_LINK_CALLBACK = 1: Habilita um callback que é chamado quando o
// status do link físico muda (ex: cabo de rede conectado/desconectado).
#define LWIP_NETIF_LINK_CALLBACK    1

// LWIP_NETIF_HOSTNAME = 1: Habilita o suporte para definir um nome de host (hostname)
// para o dispositivo na rede.
#define LWIP_NETIF_HOSTNAME         1


//==================================================================================================
/** @brief Configurações dos Protocolos de Aplicação (DHCP e DNS) */
//==================================================================================================

// LWIP_DHCP = 1: Habilita o cliente DHCP, que permite ao Pico obter um endereço
// IP automaticamente de um servidor DHCP na rede. Embora nosso projeto atue como
// um SERVIDOR DHCP, ter o código do cliente LwIP habilitado é comum.
#define LWIP_DHCP                   1

// DHCP_DOES_ARP_CHECK = 0: Desabilita a verificação via ARP que o cliente DHCP
// faz para ver se o endereço IP oferecido já está em uso. Desabilitar pode
// acelerar a obtenção de um IP.
#define DHCP_DOES_ARP_CHECK         0

#define LWIP_DHCP_DOES_ACD_CHECK    0

// LWIP_DNS = 1: Habilita o cliente DNS, permitindo que o Pico resolva nomes de
// domínio (ex: "google.com") para endereços IP.
#define LWIP_DNS                    1


//==================================================================================================
/** @brief Configurações de Checsum */
//==================================================================================================

// LWIP_CHKSUM_ALGORITHM = 3: Seleciona um algoritmo de cálculo de checksum otimizado.
// A pilha LwIP oferece diferentes implementações e esta é uma escolha comum
// para obter um bom desempenho.
#define LWIP_CHKSUM_ALGORITHM       3


//==================================================================================================
/** @brief Configurações de Depuração e Estatísticas */
//==================================================================================================

// Esta seção é controlada pela macro NDEBUG (No Debug). Se NDEBUG não estiver
// definida (o que é comum em builds de depuração), as estatísticas e os logs
// do LwIP são habilitados.
#ifndef NDEBUG
#define LWIP_DEBUG                  1 // Habilita a infraestrutura de depuração.
#define LWIP_STATS                  1 // Habilita a coleta de estatísticas de rede.
#define LWIP_STATS_DISPLAY          1 // Habilita funções para exibir as estatísticas.
#endif

// A seguir, está a configuração do nível de log para cada módulo do LwIP.
// LWIP_DBG_OFF desliga completamente os logs para aquele módulo, economizando
// espaço de código e tempo de processamento.
// Para depurar um problema específico (ex: com TCP), você pode mudar a linha
// correspondente para LWIP_DBG_ON.

#define ETHARP_DEBUG                LWIP_DBG_OFF
#define NETIF_DEBUG                 LWIP_DBG_OFF
#define PBUF_DEBUG                  LWIP_DBG_OFF
#define API_LIB_DEBUG               LWIP_DBG_OFF
#define API_MSG_DEBUG               LWIP_DBG_OFF
#define SOCKETS_DEBUG               LWIP_DBG_OFF
#define ICMP_DEBUG                  LWIP_DBG_OFF
#define INET_DEBUG                  LWIP_DBG_OFF
#define IP_DEBUG                    LWIP_DBG_OFF
#define IP_REASS_DEBUG              LWIP_DBG_OFF
#define RAW_DEBUG                   LWIP_DBG_OFF
#define MEM_DEBUG                   LWIP_DBG_OFF
#define MEMP_DEBUG                  LWIP_DBG_OFF
#define SYS_DEBUG                   LWIP_DBG_OFF
#define TCP_DEBUG                   LWIP_DBG_OFF
#define TCP_INPUT_DEBUG             LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG            LWIP_DBG_OFF
#define TCP_RTO_DEBUG               LWIP_DBG_OFF
#define TCP_CWND_DEBUG              LWIP_DBG_OFF
#define TCP_WND_DEBUG               LWIP_DBG_OFF
#define TCP_FR_DEBUG                LWIP_DBG_OFF
#define TCP_QLEN_DEBUG              LWIP_DBG_OFF
#define TCP_RST_DEBUG               LWIP_DBG_OFF
#define UDP_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                 LWIP_DBG_OFF
#define PPP_DEBUG                   LWIP_DBG_OFF
#define SLIP_DEBUG                  LWIP_DBG_OFF
#define DHCP_DEBUG                  LWIP_DBG_OFF

// =================================================================================================

#endif /* __LWIPOPTS_H__ */