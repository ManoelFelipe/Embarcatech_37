/**
 * @file    wifi_ap.c
 * @brief   Implementação do gerenciador do Ponto de Acesso (AP) Wi-Fi.
 * @details Este módulo coordena a inicialização do hardware Wi-Fi (CYW43),
 * a configuração do modo AP, e a inicialização dos servidores DHCP e DNS
 * que são essenciais para uma rede funcional.
 */

#include "wifi_ap.h"
#include "pico/cyw43_arch.h" // Funções de arquitetura para o chip Wi-Fi CYW43
#include "dhcpserver.h"      // Interface do servidor DHCP
#include "dnsserver.h"       // Interface do servidor DNS
#include <stdio.h>

/**
 * @brief Estrutura estática para manter o estado dos serviços de rede.
 * @details Sendo `static`, esta estrutura é privada a este arquivo. Ela contém
 * as instâncias dos servidores DHCP e DNS, e um flag booleano para
 * controlar o desligamento (shutdown) da aplicação.
 */
static struct {
    dhcp_server_t dhcp;     ///< Instância do estado do servidor DHCP.
    dns_server_t  dns;      ///< Instância do estado do servidor DNS.
    bool          shutdown; ///< Flag que indica se o desligamento foi solicitado.
} net = {0};

/**
 * @brief Inicializa e configura o modo Access Point (AP).
 * @param ssid O nome da rede a ser criada.
 * @param password A senha da rede.
 * @return `true` em sucesso, `false` em falha.
 */
bool wifi_ap_init(const char *ssid, const char *password)
{
    // Inicializa o chip CYW43. Se falhar, é um erro crítico.
    if (cyw43_arch_init()) {
        puts("[WiFi] ERRO init");
        return false;
    }

    // `cyw43_arch_lwip_begin()` e `..._end()` funcionam como um lock/mutex.
    // Garantem que as operações na pilha de rede lwIP sejam atômicas e seguras
    // em relação a interrupções ou outros contextos de execução.
    cyw43_arch_lwip_begin();

    // Habilita o modo Access Point com o SSID, senha e tipo de autenticação especificados.
    cyw43_arch_enable_ap_mode(ssid, password, CYW43_AUTH_WPA2_AES_PSK);

    // Define os endereços de rede para o nosso Ponto de Acesso.
    // O Pico W terá o IP estático 192.168.4.1 e atuará como gateway.
    ip4_addr_t gw, mask;
    IP4_ADDR(&gw,   192, 168, 4, 1);
    IP4_ADDR(&mask, 255, 255, 255, 0);

    // Inicializa o servidor DHCP. Ele será responsável por distribuir IPs
    // na rede 192.168.4.0/24.
    dhcp_server_init(&net.dhcp, &gw, &mask);

    // Inicializa o servidor DNS. Ele responderá a todas as consultas DNS
    // com o endereço do gateway (nosso próprio IP).
    dns_server_init (&net.dns,  &gw);

    cyw43_arch_lwip_end(); // Libera o "lock" da pilha de rede.

    printf("[WiFi] AP \"%s\" ativo em %s\n", ssid, ipaddr_ntoa(&gw));
    return true;
}

/**
 * @brief Processa eventos de rede pendentes.
 */
void wifi_ap_poll(void)
{
// A diretiva #if PICO_CYW43_ARCH_POLL verifica se o modo de polling está
// habilitado no build. Se sim, a função cyw43_arch_poll() deve ser
// chamada periodicamente para que o driver Wi-Fi funcione corretamente.
#if PICO_CYW43_ARCH_POLL
    cyw43_arch_poll();
#endif
}

/**
 * @brief Solicita o desligamento do Ponto de Acesso.
 */
void wifi_ap_request_shutdown(void) { net.shutdown = true; }

/**
 * @brief Verifica se o desligamento foi solicitado.
 * @return true se a flag de desligamento estiver ativa.
 */
bool wifi_ap_must_shutdown(void)    { return net.shutdown; }

/**
 * @brief Desinicializa todos os serviços de rede e o hardware Wi-Fi.
 */
void wifi_ap_deinit(void)
{
    cyw43_arch_lwip_begin(); // Adquire o "lock" para operações de rede.

    // Desliga os servidores em ordem: primeiro os de aplicação (DNS, DHCP),
    // depois o modo de rede do chip.
    dns_server_deinit (&net.dns);
    dhcp_server_deinit(&net.dhcp);
    cyw43_arch_disable_ap_mode();

    cyw43_arch_lwip_end(); // Libera o "lock".

    // Desinicializa completamente o chip CYW43.
    cyw43_arch_deinit();
}