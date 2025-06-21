/**
 * @file    wifi_ap.h
 * @brief   Interface pública para o módulo de gerenciamento do Ponto de Acesso (AP) Wi-Fi.
 * @details Este header declara as funções para inicializar, gerenciar o ciclo de vida e
 * finalizar o modo Access Point do chip Wi-Fi. Ele serve como uma camada de
 * abstração de alto nível sobre as funções do SDK do Pico W e os servidores
 * de rede (DHCP e DNS).
 */

#ifndef WIFI_AP_H
#define WIFI_AP_H

#include <stdbool.h>

/**
 * @brief Inicializa o chip Wi-Fi e o configura em modo Access Point (AP).
 * @details Esta função inicializa o hardware CYW43, ativa o modo AP com o SSID e
 * senha fornecidos, e inicializa os servidores DHCP e DNS necessários para
 * que os clientes possam se conectar e obter um endereço IP.
 * @param ssid O nome da rede (SSID) a ser criada.
 * @param password A senha da rede Wi-Fi (WPA2-AES-PSK).
 * @return `true` se o ponto de acesso foi iniciado com sucesso, `false` caso contrário.
 */
bool  wifi_ap_init (const char *ssid, const char *password);

/**
 * @brief Processa eventos de rede pendentes do driver Wi-Fi.
 * @details Esta função deve ser chamada repetidamente no loop principal da aplicação,
 * especialmente quando se utiliza o modo de polling (`PICO_CYW43_ARCH_POLL`).
 * Ela permite que o driver Wi-Fi execute tarefas de baixo nível, como
 * o envio e recebimento de pacotes.
 */
void  wifi_ap_poll (void);

/**
 * @brief Desativa o modo AP e desinicializa o hardware Wi-Fi.
 * @details Realiza uma finalização ordenada, desligando os servidores DNS e DHCP
 * e, em seguida, desativando a interface Wi-Fi para economizar energia e
 * liberar recursos.
 */
void  wifi_ap_deinit(void);

/**
 * @brief Verifica se uma solicitação de desligamento foi feita.
 * @details Usado no loop principal (`while`) para determinar se a aplicação deve
 * continuar executando ou iniciar o processo de finalização.
 * @return `true` se o desligamento foi solicitado, `false` caso contrário.
 */
bool  wifi_ap_must_shutdown(void);

/**
 * @brief Sinaliza que a aplicação deve ser encerrada.
 * @details Esta função pode ser chamada a qualquer momento (por exemplo, a partir de
 * uma interrupção ou de um comando do usuário via serial) para solicitar
 * um desligamento limpo da rede.
 */
void  wifi_ap_request_shutdown(void);

#endif // WIFI_AP_H