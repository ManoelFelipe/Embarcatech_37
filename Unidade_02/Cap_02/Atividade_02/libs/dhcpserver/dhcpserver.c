/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2019 Damien P. George
 *
 */

/**
 * @file dhcpserver.c
 * @brief Implementação de um servidor DHCP simples para LwIP.
 *
 * @details
 * Este arquivo contém a lógica para um servidor DHCP básico, capaz de responder
 * a requisições DHCPDISCOVER e DHCPREQUEST de clientes na rede. Ele gerencia
 * um pequeno conjunto de endereços IP e os concede aos clientes por um tempo limitado.
 *
 * O servidor escuta na porta UDP 67 e envia respostas para a porta UDP 68 do cliente.
 *
 * Principais funcionalidades:
 * - Resposta a DHCPDISCOVER com DHCPOFFER.
 * - Resposta a DHCPREQUEST com DHCPACK (ou NACK, implicitamente, ao ignorar).
 * - Gerenciamento de leases de IP com base no endereço MAC do cliente.
 * - Reutilização de IPs expirados.
 *
 * Limitações:
 * - Não implementa todas as opções DHCP.
 * - Não suporta DHCPINFORM, DHCPDECLINE, DHCPRELEASE, etc.
 * - Gerenciamento de leases simplificado.
 *
 * Este código é uma adaptação/simplificação baseada em implementações encontradas
 * no projeto MicroPython e exemplos LwIP.
 *
 * @see RFC 2131 (DHCP)
 * @see RFC 2132 (DHCP Options and BOOTP Vendor Extensions)
 */

 /**
 * Este módulo implementa apenas o necessário para que o Pico W opere como
 * ponto-de-acesso (AP) entregando endereços IP a alguns clientes (16 - 23)
 * na rede 192.168.4.0/24.  
 *
 * Principais características
 * --------------------------
 * DHCPDISCOVER  ➜  DHCPOFFER
 * DHCPREQUEST   ➜  DHCPACK
 *  - ignora Option 50 (*requested IP*) e força os leases à pool 192.168.4.[16-23]  
 * Renova o mesmo IP a cada MAC conhecido.  
 * Libera automaticamente leases vencidos (timeout = 24 h).  
 * Envia o ACK em **unicast** quando o cliente *não* pediu broadcast
 * (exigência do Windows 10/11).  
 *
 * Limitações
 * ----------
 * Não trata DHCPDECLINE, DHCPRELEASE nem NACK explícito.  
 * Tabela de leases cabe em RAM (máx. 8 IPs).  
 * Sem persistência em flash. Se reiniciar, “esquece” leases.  
 * @author  Modificado por Manoel Furtado.
 */

#include <stdio.h>      // Para printf (usado em depuração)
#include <string.h>     // Para memcpy, memset, memcmp
#include <errno.h>      // Para definições de erro como ENOMEM (embora não usado diretamente nas conversões de erro LwIP)

#include "pico/cyw43_arch.h" // Para cyw43_hal_ticks_ms(), usado para gerenciamento de tempo de lease.
                             // No código original era `cyw43_config.h` que provavelmente trazia essa dependência.
#include "dhcpserver.h"   // Definições do próprio servidor DHCP
#include "lwip/udp.h"     // API UDP da LwIP
#include "lwip/prot/dhcp.h" // Para definições padrão DHCP da LwIP (ex: DHCP_MIN_SIZE)
                           // Embora o código defina suas próprias constantes, algumas podem ser úteis
                           // ou o `DHCP_MIN_SIZE` pode ser referenciado.

/* === Constantes e Definições DHCP === */

/** @name Tipos de Mensagem DHCP (RFC 2131, Seção 3.1) */
/** @{ */
#define DHCPDISCOVER    (1) /**< Mensagem do cliente para localizar servidores DHCP disponíveis. */
#define DHCPOFFER       (2) /**< Resposta do servidor a um DHCPDISCOVER, oferecendo parâmetros de configuração. */
#define DHCPREQUEST     (3) /**< Mensagem do cliente para solicitar parâmetros de um servidor específico ou confirmar um lease. */
#define DHCPDECLINE     (4) /**< Mensagem do cliente para indicar que o endereço de rede é já está em uso. */
#define DHCPACK         (5) /**< Resposta do servidor a um DHCPREQUEST, confirmando o lease e os parâmetros. */
#define DHCPNACK        (6) /**< Resposta do servidor a um DHCPREQUEST, indicando que o lease não é válido ou expirou. */
#define DHCPRELEASE     (7) /**< Mensagem do cliente para o servidor, relinquindo o endereço de rede e cancelando o lease. */
#define DHCPINFORM      (8) /**< Mensagem do cliente para obter parâmetros de configuração local (sem alocação de IP). */
/** @} */

/** @name Códigos de Opção DHCP (RFC 2132) */
/** @{ */
#define DHCP_OPT_PAD                (0)   /**< Padding, usado para alinhar opções. */
#define DHCP_OPT_SUBNET_MASK        (1)   /**< Máscara de sub-rede. */
#define DHCP_OPT_ROUTER             (3)   /**< Endereço do roteador (gateway). */
#define DHCP_OPT_DNS                (6)   /**< Endereços dos servidores DNS. */
#define DHCP_OPT_HOST_NAME          (12)  /**< Nome do host do cliente. */
#define DHCP_OPT_REQUESTED_IP       (50)  /**< Endereço IP solicitado pelo cliente. */
#define DHCP_OPT_IP_LEASE_TIME      (51)  /**< Tempo de duração do lease do IP (em segundos). */
#define DHCP_OPT_MSG_TYPE           (53)  /**< Tipo da mensagem DHCP (DHCPDISCOVER, DHCPOFFER, etc.). */
#define DHCP_OPT_SERVER_ID          (54)  /**< Identificador do servidor DHCP (seu endereço IP). */
#define DHCP_OPT_PARAM_REQUEST_LIST (55)  /**< Lista de parâmetros solicitados pelo cliente. */
#define DHCP_OPT_MAX_MSG_SIZE       (57)  /**< Tamanho máximo da mensagem DHCP que o cliente pode aceitar. */
#define DHCP_OPT_VENDOR_CLASS_ID    (60)  /**< Identificador da classe do vendor do cliente. */
#define DHCP_OPT_CLIENT_ID          (61)  /**< Identificador único do cliente. */
#define DHCP_OPT_END                (255) /**< Marca o fim da lista de opções. */
/** @} */

/** @brief Porta UDP padrão do servidor DHCP. */
#define PORT_DHCP_SERVER (67)
/** @brief Porta UDP padrão do cliente DHCP. */
#define PORT_DHCP_CLIENT (68)

/** @brief Tempo de lease padrão em segundos (24 horas). */
#define DEFAULT_LEASE_TIME_S (24 * 60 * 60)

/** @brief Comprimento do endereço MAC em bytes. */
#define MAC_LEN (6)

// Comentário original: MAKE_IP4 está obsoleto, usar IP4_ADDR da LwIP.
// #define MAKE_IP4(a, b, c, d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))

/**
 * @brief Estrutura de uma mensagem DHCP.
 * @details Define os campos de uma mensagem DHCP conforme RFC 2131.
 * O tamanho dos campos `sname`, `file` e `options` pode variar, mas
 * aqui são fixados para simplificar. A parte de `options` começa
 * com um "magic cookie" (99, 130, 83, 99).
 */
typedef struct {
    uint8_t op;         /**< Código da operação: 1=BOOTREQUEST (cliente), 2=BOOTREPLY (servidor). */
    uint8_t htype;      /**< Tipo de endereço de hardware (ex: 1 para Ethernet). */
    uint8_t hlen;       /**< Comprimento do endereço de hardware (ex: 6 para Ethernet). */
    uint8_t hops;       /**< Contagem de saltos (hops) do relay agent. 0 para cliente direto. */
    uint32_t xid;       /**< ID da transação, um número aleatório escolhido pelo cliente. */
    uint16_t secs;      /**< Segundos decorridos desde que o cliente começou o processo de aquisição/renovação. */
    uint16_t flags;     /**< Flags (ex: bit BROADCAST). */
    uint8_t ciaddr[4];  /**< Endereço IP do cliente (Client IP address). Preenchido pelo cliente se ele já tem um IP. */
    uint8_t yiaddr[4];  /**< 'Seu' (do cliente) endereço IP (Your IP address). Preenchido pelo servidor. */
    uint8_t siaddr[4];  /**< Endereço IP do próximo servidor a ser usado no bootstrap (Server IP address). */
    uint8_t giaddr[4];  /**< Endereço IP do relay agent (Gateway IP address). */
    uint8_t chaddr[16]; /**< Endereço de hardware do cliente (Client Hardware Address). Apenas `hlen` bytes são usados. */
    uint8_t sname[64];  /**< Nome do host do servidor (opcional). */
    uint8_t file[128];  /**< Nome do arquivo de boot (opcional). */
    uint8_t options[312];/**< Campo de opções DHCP variáveis. Começa com o "magic cookie". */
} dhcp_msg_t;


/* === Funções Utilitárias para Sockets (Adaptadas para LwIP) === */

/**
 * @brief Cria um novo socket UDP (PCB na LwIP).
 * @param udp Ponteiro para o ponteiro do PCB UDP a ser criado.
 * @param cb_data Dados do usuário a serem passados para o callback de recebimento.
 * @param cb_udp_recv Função de callback a ser chamada quando dados UDP são recebidos.
 * @return int 0 em caso de sucesso, ou um código de erro negativo (ex: -ENOMEM).
 */
static int dhcp_socket_new_dgram(struct udp_pcb **udp, void *cb_data, udp_recv_fn cb_udp_recv) {
    *udp = udp_new(); // Cria um novo PCB UDP
    if (*udp == NULL) {
        printf("[DHCPS] Falha ao criar PCB UDP (udp_new): sem memória.\n");
        return -ENOMEM; // Erro: Sem memória
    }

    // Registra a função de callback para quando dados forem recebidos neste PCB.
    udp_recv(*udp, cb_udp_recv, cb_data);

    return 0; // Sucesso
}

/**
 * @brief Libera um socket UDP (remove o PCB na LwIP).
 * @param udp Ponteiro para o ponteiro do PCB UDP a ser liberado.
 */
static void dhcp_socket_free(struct udp_pcb **udp) {
    if (*udp != NULL) {
        udp_remove(*udp); // Remove e libera o PCB UDP
        *udp = NULL;
    }
}

/**
 * @brief Vincula (bind) um socket UDP a um endereço IP local e porta.
 * @param udp Ponteiro para o ponteiro do PCB UDP a ser vinculado.
 * @param port Porta UDP local à qual se vincular.
 * @return err_t Código de erro LwIP (ERR_OK em sucesso).
 */
static int dhcp_socket_bind(struct udp_pcb **udp, uint16_t port) {
    // Vincula o PCB a qualquer endereço IP local (IP_ANY_TYPE) na porta especificada.
    err_t err = udp_bind(*udp, IP_ANY_TYPE, port);
    if (err != ERR_OK) {
        printf("[DHCPS] Falha ao vincular PCB UDP à porta %u (udp_bind): %d\n", port, err);
    }
    return err;
}

/**
 * @brief Envia dados UDP para um destino específico através de uma interface de rede específica.
 * @param udp Ponteiro para o ponteiro do PCB UDP a ser usado para envio.
 * @param nif Ponteiro para a interface de rede (`netif`) a ser usada para enviar o pacote.
 * Se NULL, LwIP tentará encontrar a interface apropriada.
 * @param buf Ponteiro para os dados a serem enviados.
 * @param len Comprimento dos dados a serem enviados.
 * @param ip Endereço IP de destino (formato uint32_t, host byte order).
 * @param port Porta UDP de destino.
 * @return int Número de bytes enviados em sucesso, ou um código de erro LwIP.
 */
static int dhcp_socket_sendto(struct udp_pcb **udp, struct netif *nif, const void *buf, size_t len, uint32_t ip_dest_val, uint16_t port) {
    if (len > 0xffff) { // Limita o tamanho para caber em u16_t (pbuf_alloc)
        len = 0xffff;
    }

    // Aloca um pbuf (buffer de pacotes da LwIP) para enviar os dados.
    // PBUF_TRANSPORT indica que o buffer é para a camada de transporte.
    // PBUF_RAM significa que o buffer é alocado na RAM e os dados serão copiados para ele.
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, (u16_t)len, PBUF_RAM);
    if (p == NULL) {
        printf("[DHCPS] Falha ao alocar pbuf para envio (pbuf_alloc): sem memória.\n");
        return -ENOMEM; // Erro: Sem memória
    }

    // Copia os dados do buffer `buf` para o payload do `pbuf`.
    memcpy(p->payload, buf, len);

    ip_addr_t dest_addr;
    // Converte o uint32_t para o tipo ip_addr_t da LwIP.
    // A ordem dos bytes em ip_dest_val deve ser considerada (network vs host).
    // Assumindo que ip_dest_val é network byte order se vindo de funções de rede,
    // ou precisa de htonl se for host byte order.
    // A função IP4_ADDR espera (A,B,C,D) em host byte order.
    // Se ip_dest_val for 0xffffffff (broadcast), isso se traduzirá corretamente.
    IP4_ADDR(ip_2_ip4(&dest_addr), (uint8_t)((ip_dest_val >> 24) & 0xff),
                                   (uint8_t)((ip_dest_val >> 16) & 0xff),
                                   (uint8_t)((ip_dest_val >>  8) & 0xff),
                                   (uint8_t)((ip_dest_val >>  0) & 0xff));

    err_t err;
    if (nif != NULL) {
        // Envia o pacote UDP através da interface de rede especificada.
        err = udp_sendto_if(*udp, p, &dest_addr, port, nif);
    } else {
        // Envia o pacote UDP, LwIP escolherá a interface de rede.
        err = udp_sendto(*udp, p, &dest_addr, port);
    }

    pbuf_free(p); // Libera o pbuf após o envio (ou tentativa de envio).

    if (err != ERR_OK) {
        printf("[DHCPS] Falha ao enviar dados UDP (udp_sendto/udp_sendto_if): %d\n", err);
        return err; // Retorna o código de erro LwIP
    }

    return (int)len; // Sucesso, retorna o número de bytes enviados
}


/* === Funções Utilitárias para Opções DHCP === */

/**
 * @brief Encontra uma opção DHCP específica no campo de opções de uma mensagem DHCP.
 * @param opt Ponteiro para o início do campo de opções (após o magic cookie).
 * @param cmd Código da opção DHCP a ser procurada.
 * @return uint8_t* Ponteiro para o início da opção encontrada (código da opção),
 * ou NULL se a opção não for encontrada.
 */
static uint8_t *opt_find(uint8_t *opt, uint8_t cmd) {
    // O campo de opções tem um máximo de 308 bytes (312 - 4 do magic cookie).
    // Itera pelas opções: cada opção é [código (1 byte)] [comprimento (1 byte)] [dados (N bytes)].
    // A opção DHCP_OPT_PAD (0) é uma exceção, tem apenas o código.
    // A opção DHCP_OPT_END (255) marca o fim das opções.
    for (int i = 0; i < (312 - 4) && opt[i] != DHCP_OPT_END;) {
        if (opt[i] == cmd) {
            return &opt[i]; // Opção encontrada
        }
        if (opt[i] == DHCP_OPT_PAD) {
            i++; // Avança 1 byte para PAD
        } else {
            // Avança para a próxima opção: 1 (código) + 1 (comprimento) + opt[i+1] (dados)
            i += 2 + opt[i + 1];
        }
    }
    return NULL; // Opção não encontrada
}

/**
 * @brief Escreve uma opção DHCP com múltiplos bytes de dados.
 * @param opt Duplo ponteiro para a posição atual no buffer de opções. Será atualizado.
 * @param cmd Código da opção DHCP.
 * @param n Comprimento dos dados da opção.
 * @param data Ponteiro para os dados da opção.
 */
static void opt_write_n(uint8_t **opt, uint8_t cmd, size_t n, const void *data) {
    uint8_t *o = *opt;
    *o++ = cmd;         // Código da opção
    *o++ = (uint8_t)n;  // Comprimento dos dados
    memcpy(o, data, n); // Dados
    *opt = o + n;       // Atualiza o ponteiro para a próxima posição livre
}

/**
 * @brief Escreve uma opção DHCP com um único byte de dados.
 * @param opt Duplo ponteiro para a posição atual no buffer de opções. Será atualizado.
 * @param cmd Código da opção DHCP.
 * @param val Valor (1 byte) dos dados da opção.
 */
static void opt_write_u8(uint8_t **opt, uint8_t cmd, uint8_t val) {
    uint8_t *o = *opt;
    *o++ = cmd;     // Código
    *o++ = 1;       // Comprimento (1)
    *o++ = val;     // Dado
    *opt = o;       // Atualiza ponteiro
}

/**
 * @brief Escreve uma opção DHCP com dados de 4 bytes (uint32_t).
 * @param opt Duplo ponteiro para a posição atual no buffer de opções. Será atualizado.
 * @param cmd Código da opção DHCP.
 * @param val Valor (4 bytes) dos dados da opção, em network byte order.
 */
static void opt_write_u32(uint8_t **opt, uint8_t cmd, uint32_t val) {
    uint8_t *o = *opt;
    *o++ = cmd;         // Código
    *o++ = 4;           // Comprimento (4)
    // Escreve o valor em network byte order (big-endian)
    *o++ = (uint8_t)(val >> 24);
    *o++ = (uint8_t)(val >> 16);
    *o++ = (uint8_t)(val >> 8);
    *o++ = (uint8_t)(val);
    *opt = o;           // Atualiza ponteiro
}


/* === Processamento Principal do Servidor DHCP === */

/**
 * @brief Função de callback chamada pela LwIP quando um pacote UDP é recebido na porta do servidor DHCP.
 * @details Processa a mensagem DHCP recebida, determina o tipo (DISCOVER, REQUEST, etc.)
 * e formula uma resposta apropriada (OFFER, ACK).
 *
 * @param arg Argumento fornecido ao registrar o callback (ponteiro para `dhcp_server_t`).
 * @param upcb PCB UDP no qual o pacote foi recebido.
 * @param p pbuf (cadeia de buffers) contendo o pacote DHCP recebido.
 * @param src_addr Endereço IP de origem do pacote.
 * @param src_port Porta de origem do pacote.
 */
static void dhcp_server_process(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *src_addr, u16_t src_port) {
    dhcp_server_t *d = (dhcp_server_t*)arg; // Estado do servidor DHCP
    (void)upcb;     // Não usado diretamente, o PCB do servidor é d->udp
    (void)src_addr; // A resposta DHCP é geralmente broadcast ou para um IP específico na mensagem.
    (void)src_port; // Respostas são para PORT_DHCP_CLIENT (68)

    // Estrutura para armazenar a mensagem DHCP recebida/a ser enviada.
    // Seu tamanho é ~548 bytes, dependendo das definições.
    dhcp_msg_t dhcp_msg;

    // Tamanho mínimo de uma mensagem DHCP (RFC 2131, BOOTP msg + magic cookie).
    // A LwIP também define DHCP_MIN_LEN em lwip/prot/dhcp.h que é 236 + 4.
    // Este valor DHCP_MIN_SIZE (240 + 3) parece ser um arredondamento ou um valor específico.
    // Vamos usar uma verificação conservadora baseada no tamanho da struct até o início das opções.
    #define DHCP_FIXED_PART_SIZE (offsetof(dhcp_msg_t, options))
    #define DHCP_MIN_REQUIRED_SIZE (DHCP_FIXED_PART_SIZE + 4) // 4 para o magic cookie

    if (p->tot_len < DHCP_MIN_REQUIRED_SIZE) {
        printf("[DHCPS] Pacote muito pequeno (%u bytes), ignorando.\n", p->tot_len);
        goto ignore_request; // Libera pbuf e retorna
    }

    // Copia o conteúdo do pbuf para a estrutura dhcp_msg.
    size_t len_copied = pbuf_copy_partial(p, &dhcp_msg, sizeof(dhcp_msg), 0);
    if (len_copied < DHCP_MIN_REQUIRED_SIZE) {
        // Se nem o mínimo foi copiado, algo está errado.
        printf("[DHCPS] Falha ao copiar o mínimo do pacote DHCP (%u bytes copiados).\n", len_copied);
        goto ignore_request;
    }

    // Valida o "magic cookie" DHCP (99, 130, 83, 99)
    if (dhcp_msg.options[0] != 0x63 || dhcp_msg.options[1] != 0x82 ||
        dhcp_msg.options[2] != 0x53 || dhcp_msg.options[3] != 0x63) {
        printf("[DHCPS] Magic cookie DHCP inválido. Ignorando pacote.\n");
        goto ignore_request;
    }

    // Ponteiro para o início das opções DHCP (após o magic cookie).
    uint8_t *opt_ptr = (uint8_t *)&dhcp_msg.options[4];

    // Encontra a opção DHCP_OPT_MSG_TYPE (código 53) para determinar o tipo de mensagem.
    uint8_t *msg_type_opt = opt_find(opt_ptr, DHCP_OPT_MSG_TYPE);
    if (msg_type_opt == NULL || msg_type_opt[1] != 1) { // Opção deve existir e ter tamanho 1
        printf("[DHCPS] Opção DHCP Message Type (53) não encontrada ou inválida. Ignorando.\n");
        goto ignore_request;
    }

    uint8_t extracted_msg_type = msg_type_opt[2]; // O tipo da mensagem (ex: DHCPDISCOVER)

    // Prepara a mensagem de resposta (alguns campos são comuns)
    dhcp_msg.op = 2; // BOOTREPLY (resposta do servidor)
    // yiaddr (Your IP Address) será preenchido abaixo.
    // siaddr (Server IP Address, next server) pode ser o IP do servidor DHCP.
    // giaddr (Gateway IP Address) não é preenchido por este servidor simples.
    memcpy(&dhcp_msg.siaddr, &ip_2_ip4(&d->ip)->addr, 4); // IP do servidor (nós mesmos)

    int yi_idx = -1; // Índice do lease na tabela d->lease[] para o IP a ser oferecido/confirmado.

    switch (extracted_msg_type) {
        case DHCPDISCOVER: {
            printf("[DHCPS] Recebido DHCPDISCOVER de MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                   dhcp_msg.chaddr[0], dhcp_msg.chaddr[1], dhcp_msg.chaddr[2],
                   dhcp_msg.chaddr[3], dhcp_msg.chaddr[4], dhcp_msg.chaddr[5]);

            // Tenta encontrar um lease existente para este MAC ou um IP livre.
            int best_free_idx = -1;
            uint32_t current_time_marker = cyw43_hal_ticks_ms() >> 16; // Para comparação de expiração

            for (int i = 0; i < DHCPS_MAX_IP; ++i) {
                if (memcmp(d->lease[i].mac, dhcp_msg.chaddr, MAC_LEN) == 0) {
                    // MAC já conhecido, oferecer o mesmo IP.
                    yi_idx = i;
                    break;
                }
                // Se o MAC é zero ou o lease expirou, considera como livre.
                // Um MAC nulo (todos zeros) indica um slot de lease não utilizado.
                bool mac_is_null = true;
                for(int k=0; k<MAC_LEN; ++k) if(d->lease[i].mac[k] != 0) mac_is_null = false;

                // `expiry` armazena (ticks_ms_expiracao >> 16).
                // Se (lease_expiry_time_marker - current_time_marker) < 0, então expirou.
                // Nota: Esta comparação de tempo com rollover deve ser feita com cuidado.
                // (int32_t)(d->lease[i].expiry - current_time_marker) < 0 é uma forma de checar.
                // Mas, como `expiry` é uint16_t, a diferença direta pode ser enganosa se houver rollover em ticks_ms.
                // A intenção original era: if ((int32_t)( (d->lease[i].expiry << 16 | 0xffff) - cyw43_hal_ticks_ms()) < 0)
                // Simplificando: se a expiração (em unidades de ~65s) é menor que a atual, está expirado.
                // Ou, se `d->lease[i].expiry == 0` e MAC não é nulo, pode ser um lease que nunca foi usado/expirou muito tempo atrás.
                // Se o MAC for nulo, o slot está livre.
                if (mac_is_null) {
                    if (best_free_idx == -1) best_free_idx = i; // Primeiro slot totalmente livre.
                } else if ((int16_t)(current_time_marker - d->lease[i].expiry) > 0 && d->lease[i].expiry != 0) {
                    // Lease expirado (current_time > expiry_time)
                    printf("[DHCPS] Lease para MAC %02x:%02x... no IP ...%u expirou. Reutilizando.\n", d->lease[i].mac[0],d->lease[i].mac[1], DHCPS_BASE_IP + i);
                    memset(d->lease[i].mac, 0, MAC_LEN); // Limpa o MAC, tornando o slot livre.
                    d->lease[i].expiry = 0;
                    if (best_free_idx == -1) best_free_idx = i;
                }
            }

            if (yi_idx == -1 && best_free_idx != -1) { // Não achou MAC, mas achou slot livre/expirado
                yi_idx = best_free_idx;
            }

            if (yi_idx == -1) { // Nenhum IP disponível
                printf("[DHCPS] Nenhum IP disponível para oferecer.\n");
                goto ignore_request;
            }

            // Configura o IP oferecido (yiaddr)
            // O IP é d->ip com o último octeto modificado.
            /* monta yiaddr = 192.168.4.(16+yi_idx) ─ endianness à prova */
            {
                uint32_t ip_u32 = ip4_addr_get_u32(ip_2_ip4(&d->ip)); /* 192.168.4.1 (network-order) */
                memcpy(dhcp_msg.yiaddr, &ip_u32, 4);                  /* copia 192.168.4.1          */
                dhcp_msg.yiaddr[3] = (uint8_t)(DHCPS_BASE_IP + yi_idx); /* troca só o último octeto */
            }
            opt_ptr = (uint8_t *)&dhcp_msg.options[4]; /* reinicia lista de opções */
            opt_write_u8(&opt_ptr, DHCP_OPT_MSG_TYPE, DHCPOFFER);

            // Prepara as opções para DHCPOFFER
            opt_ptr = (uint8_t *)&dhcp_msg.options[4]; // Reinicia ponteiro de opções
            opt_write_u8(&opt_ptr, DHCP_OPT_MSG_TYPE, DHCPOFFER);
            // (Outras opções serão adicionadas depois do switch)
            break;
        }

        case DHCPREQUEST: {
            /* === ignora Option 50 e garante IP na pool === */
            int yi = DHCPS_MAX_IP;

            /* 1. Já existe lease para esta MAC? */
            for (int i = 0; i < DHCPS_MAX_IP; ++i) {
                if (memcmp(d->lease[i].mac, dhcp_msg.chaddr, MAC_LEN) == 0) {
                    yi = i;
                    break;
                }
            }

            /* 2. Senão, acha slot livre ou expirado */
            if (yi == DHCPS_MAX_IP) {
                for (int i = 0; i < DHCPS_MAX_IP; ++i) {
                    uint32_t expiry = (d->lease[i].expiry << 16) | 0xFFFF;
                    if (memcmp(d->lease[i].mac, "\0\0\0\0\0\0", MAC_LEN) == 0 ||
                        (int32_t)(expiry - cyw43_hal_ticks_ms()) < 0) {
                        yi = i;
                        memcpy(d->lease[i].mac, dhcp_msg.chaddr, MAC_LEN);
                        break;
                    }
                }
            }
            if (yi == DHCPS_MAX_IP)   /* pool lotada */ goto ignore_request;

            /* 3. Atualiza lease e monta ACK */
            d->lease[yi].expiry = (cyw43_hal_ticks_ms() + DEFAULT_LEASE_TIME_S * 1000) >> 16;

            /* monta yiaddr = 192.168.4.(16+yi) */
            uint32_t ip_u32 = ip4_addr_get_u32(ip_2_ip4(&d->ip));   // 192.168.4.1 em network-order
            memcpy(dhcp_msg.yiaddr, &ip_u32, 4);                    // copia 192.168.4.1
            dhcp_msg.yiaddr[3] = (uint8_t)(DHCPS_BASE_IP + yi);     // substitui o último octeto

            opt_write_u8(&opt_ptr, DHCP_OPT_MSG_TYPE, DHCPACK);

            printf("DHCPS: MAC=%02x:%02x:%02x:%02x:%02x:%02x  ->  IP=%u.%u.%u.%u\n",
                   dhcp_msg.chaddr[0], dhcp_msg.chaddr[1], dhcp_msg.chaddr[2],
                   dhcp_msg.chaddr[3], dhcp_msg.chaddr[4], dhcp_msg.chaddr[5],
                   dhcp_msg.yiaddr[0], dhcp_msg.yiaddr[1],
                   dhcp_msg.yiaddr[2], dhcp_msg.yiaddr[3]);
            break;
        }


        default:
            printf("[DHCPS] Tipo de mensagem DHCP não suportado: %d. Ignorando.\n", extracted_msg_type);
            goto ignore_request;
    }

    // Adiciona opções comuns para DHCPOFFER e DHCPACK
    // Assegure-se que opt_ptr está correto (após DHCP_OPT_MSG_TYPE)
    uint32_t temp_u32;

    temp_u32 = ip4_addr_get_u32(ip_2_ip4(&d->ip)); // IP do servidor DHCP
    //opt_write_u32(&opt_ptr, DHCP_OPT_SERVER_ID, lwip_htonl(temp_u32));
    opt_write_u32(&opt_ptr, DHCP_OPT_SERVER_ID, temp_u32); 
    //opt_write_u32(&opt_ptr, DHCP_OPT_SERVER_ID, lwip_htonl(temp_u32));


    temp_u32 = ip4_addr_get_u32(ip_2_ip4(&d->nm)); // Máscara de sub-rede
    opt_write_n(&opt_ptr, DHCP_OPT_SUBNET_MASK, 4, &temp_u32); // Já está em network order se d->nm é ip_addr_t

    temp_u32 = ip4_addr_get_u32(ip_2_ip4(&d->ip)); // Gateway (nosso próprio IP)
    opt_write_n(&opt_ptr, DHCP_OPT_ROUTER, 4, &temp_u32);

    temp_u32 = ip4_addr_get_u32(ip_2_ip4(&d->ip)); // Servidor DNS (nosso próprio IP)
    opt_write_n(&opt_ptr, DHCP_OPT_DNS, 4, &temp_u32);

    opt_write_u32(&opt_ptr, DHCP_OPT_IP_LEASE_TIME, lwip_htonl(DEFAULT_LEASE_TIME_S));

    *opt_ptr++ = DHCP_OPT_END; // Marca o fim das opções.

    // Envia a resposta DHCP.
    // Respostas DHCP (OFFER, ACK) são geralmente enviadas para o endereço MAC do cliente
    // em broadcast (255.255.255.255) ou unicast se o cliente já tem um IP e o flag BROADCAST não está setado.
    // Para simplificar, este servidor frequentemente envia em broadcast.
    // A porta de destino é sempre PORT_DHCP_CLIENT (68).
    // A interface de envio é a interface na qual a requisição foi recebida.
    struct netif *nif = ip_current_input_netif(); // Obtém a interface de entrada atual
    uint32_t dest_ip_val = 0xFFFFFFFF; // Broadcast IP

    // Se o cliente já tem um IP (ciaddr não é zero) e o flag BROADCAST não está setado na requisição,
    // o servidor PODE enviar a resposta em unicast para ciaddr.
    // Mas broadcast para 255.255.255.255 é mais seguro, especialmente para DHCPOFFER.
    // Para DHCPACK, se giaddr for 0 e ciaddr for 0, broadcast. Se giaddr for 0 e ciaddr não for 0, unicast para ciaddr.
    // Se giaddr não for 0, unicast para giaddr. (RFC 2131, Seção 4.1)
    // Este servidor não lida com giaddr, então simplifica.
    /*if (extracted_msg_type == DHCPACK && (dhcp_msg.flags & PP_HTONS(0x8000)) == 0 &&  // Se flag BROADCAST não está set
        (dhcp_msg.ciaddr[0] != 0 || dhcp_msg.ciaddr[1] != 0 || dhcp_msg.ciaddr[2] != 0 || dhcp_msg.ciaddr[3] != 0) ) {
        // Tenta unicast para ciaddr se não for broadcast explicitamente pedido e ciaddr existe.
        // No entanto, yiaddr é o IP que estamos ACKNOWLEDGING.
        // A especificação diz para enviar para 'yiaddr' se ciaddr for zero, ou para ciaddr.
        // Mas o mais comum para ACK é enviar para o IP que está sendo concedido (yiaddr) se já conhecido pelo cliente,
        // ou broadcast.
        // Por simplicidade e robustez, o broadcast ainda é uma boa opção.
        // dest_ip_val = *((uint32_t*)dhcp_msg.yiaddr); // Unicast para o IP concedido
        // Ou, manter broadcast.
    }*/

    /*  RFC 2131 – regra de unicast para DHCPACK  */
    if (extracted_msg_type == DHCPACK) {
        bool broadcast_flag = (dhcp_msg.flags & PP_HTONS(0x8000)) != 0;
        bool ciaddr_is_zero = (dhcp_msg.ciaddr[0] | dhcp_msg.ciaddr[1] |
                            dhcp_msg.ciaddr[2] | dhcp_msg.ciaddr[3]) == 0;

        if (!broadcast_flag && ciaddr_is_zero) {
            /* cliente ainda não tem IP e NÃO pediu broadcast → unicast em yiaddr */
            dest_ip_val = ((uint32_t)dhcp_msg.yiaddr[0] << 24) |
                        ((uint32_t)dhcp_msg.yiaddr[1] << 16) |
                        ((uint32_t)dhcp_msg.yiaddr[2] <<  8) |
                        (uint32_t) dhcp_msg.yiaddr[3];
        }
    }


    printf("[DHCPS] Enviando %s para MAC %02x:%02x... IP %u.%u.%u.%u via broadcast (255.255.255.255)\n",
           (extracted_msg_type == DHCPDISCOVER) ? "DHCPOFFER" : "DHCPACK",
           dhcp_msg.chaddr[0], dhcp_msg.chaddr[1],
           dhcp_msg.yiaddr[0], dhcp_msg.yiaddr[1], dhcp_msg.yiaddr[2], dhcp_msg.yiaddr[3]);

    dhcp_socket_sendto(&d->udp, nif, &dhcp_msg, (size_t)(opt_ptr - (uint8_t *)&dhcp_msg),
                       dest_ip_val, PORT_DHCP_CLIENT);

ignore_request:
    pbuf_free(p); // Libera o pbuf recebido, pois já foi processado ou ignorado.
}


/* === Funções de Inicialização e Desinicialização do Servidor DHCP === */

/**
 * @brief Inicializa o servidor DHCP.
 * @param d Ponteiro para a estrutura `dhcp_server_t` a ser inicializada.
 * @param ip Endereço IP do servidor (e gateway para os clientes).
 * @param nm Máscara de sub-rede da rede.
 */
void dhcp_server_init(dhcp_server_t *d, ip_addr_t *ip, ip_addr_t *nm) {
    printf("[DHCPS] Inicializando servidor DHCP...\n");
    printf("[DHCPS] IP do Servidor/Gateway: %s\n", ipaddr_ntoa(ip));
    printf("[DHCPS] Máscara de Sub-rede: %s\n", ipaddr_ntoa(nm));
    printf("[DHCPS] Faixa de IPs para clientes: %s.%u - %s.%u\n",
        ip4addr_ntoa_r((const ip4_addr_t *)ip_2_ip4(ip), (char[16]){0}, 16) , DHCPS_BASE_IP, // Início da faixa
        ip4addr_ntoa_r((const ip4_addr_t *)ip_2_ip4(ip), (char[16]){0}, 16) , DHCPS_BASE_IP + DHCPS_MAX_IP -1 // Fim da faixa
    );


    // Copia os endereços IP e máscara para a estrutura do servidor.
    ip_addr_copy(d->ip, *ip);
    ip_addr_copy(d->nm, *nm);

    // Limpa a tabela de leases (MACs e expirações).
    memset(d->lease, 0, sizeof(d->lease));

    // Cria e configura o socket UDP.
    if (dhcp_socket_new_dgram(&d->udp, d, dhcp_server_process) != 0) {
        printf("[DHCPS] Falha crítica: não foi possível criar o socket UDP para o servidor DHCP.\n");
        // Em um sistema real, isso exigiria tratamento de erro mais robusto.
        return;
    }
    if (dhcp_socket_bind(&d->udp, PORT_DHCP_SERVER) != ERR_OK) {
        printf("[DHCPS] Falha crítica: não foi possível vincular o socket UDP à porta %d.\n", PORT_DHCP_SERVER);
        dhcp_socket_free(&d->udp); // Libera o socket se o bind falhar
        return;
    }
    printf("[DHCPS] Servidor DHCP escutando na porta UDP %d.\n", PORT_DHCP_SERVER);
}

/**
 * @brief Desinicializa o servidor DHCP.
 * @param d Ponteiro para a estrutura `dhcp_server_t` a ser desinicializada.
 */
void dhcp_server_deinit(dhcp_server_t *d) {
    printf("[DHCPS] Desinicializando servidor DHCP...\n");
    dhcp_socket_free(&d->udp); // Libera o socket UDP.
    // Limpar d->ip, d->nm, d->lease não é estritamente necessário se 'd' for sair de escopo
    // ou for ser reutilizado após uma nova chamada a dhcp_server_init.
    printf("[DHCPS] Servidor DHCP desinicializado.\n");
}