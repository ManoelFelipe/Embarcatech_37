/**
 * @file server_with_wifi.c
 * @brief Implementação de um servidor BLE com sensor de temperatura e funcionalidade Wi-Fi (iperf).
 *
 * Este exemplo combina a funcionalidade de um servidor GATT BLE (expondo um sensor
 * de temperatura) com conectividade Wi-Fi. Após conectar-se a uma rede Wi-Fi,
 * ele inicia um servidor iperf, permitindo testes de largura de banda da rede.
 * A parte BLE continua funcionando em paralelo, anunciando e permitindo conexões
 * para leitura de temperatura.
 * Utiliza o `async_context` do CYW43 para gerenciar tarefas como o piscar do LED
 * e a atualização da temperatura, de forma a integrar-se bem com as operações
 * de Wi-Fi e Bluetooth.
 *
 * As credenciais de Wi-Fi (WIFI_SSID e WIFI_PASSWORD) devem ser definidas
 * externamente (por exemplo, em um arquivo `pico_sdk_import.cmake` ou via flags de compilação).
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "btstack.h"
#include "pico/cyw43_arch.h"  // Arquitetura CYW43 para Wi-Fi e Bluetooth
#include "pico/stdlib.h"
#include "hardware/adc.h"

// Inclusões específicas para LwIP (pilha TCP/IP)
#include "lwip/netif.h"      // Para gerenciamento de interfaces de rede LwIP
#include "lwip/ip4_addr.h"   // Para manipulação de endereços IPv4
#include "lwip/apps/lwiperf.h" // Para a aplicação iperf do LwIP

#include "server_common.h"   // Inclui definições e funções comuns ao servidor BLE

// As credenciais Wi-Fi devem ser definidas em algum lugar, por exemplo, no CMakeLists.txt
// ou em um arquivo de configuração incluído.
// Exemplo de como definir no CMakeLists.txt:
// target_compile_definitions(your_target_name PRIVATE
//     WIFI_SSID="YourSSID"
//     WIFI_PASSWORD="YourPassword"
// )
#ifndef WIFI_SSID
#error "WIFI_SSID não definido!"
#endif
#ifndef WIFI_PASSWORD
#error "WIFI_PASSWORD não definido!"
#endif

#define HEARTBEAT_PERIOD_MS 1000 /**< Período em milissegundos para o timer de heartbeat/worker. */

/**
 * @brief Protótipo da função do worker de heartbeat.
 *
 * Esta função será chamada pelo `async_context` do CYW43.
 * @param context O contexto assíncrono.
 * @param worker O worker que está sendo executado.
 */
static void heartbeat_handler(async_context_t *context, async_at_time_worker_t *worker);

/**
 * @var heartbeat_worker
 * @brief Estrutura do worker assíncrono para o heartbeat.
 *
 * Define o worker que será adicionado ao `async_context` do CYW43 para
 * executar a função `heartbeat_handler` periodicamente.
 */
static async_at_time_worker_t heartbeat_worker = { .do_work = heartbeat_handler };

/**
 * @var hci_event_callback_registration
 * @brief Estrutura para registrar o callback de eventos HCI.
 */
static btstack_packet_callback_registration_t hci_event_callback_registration;

/**
 * @brief Manipulador do worker de heartbeat (executado no contexto assíncrono).
 *
 * Esta função é chamada periodicamente pelo `async_context` do CYW43.
 * A cada 10 segundos (10 * HEARTBEAT_PERIOD_MS), ela aciona a leitura da temperatura (`poll_temp`).
 * Se as notificações LE estiverem habilitadas por um cliente BLE, solicita o envio
 * de uma notificação da temperatura atual.
 * Também inverte o estado do LED da placa para indicar atividade.
 *
 * @param context O contexto assíncrono ao qual este worker pertence.
 * @param worker Ponteiro para a estrutura do worker (este próprio worker).
 */
static void heartbeat_handler(async_context_t *context, async_at_time_worker_t *worker) {
    static uint32_t counter = 0; // Contador para controlar a frequência da leitura de temperatura.
    counter++;

    // Atualiza a temperatura a cada 10 segundos.
    if (counter % 10 == 0) {
        poll_temp(); // Lê e atualiza o valor da temperatura.
        if (le_notification_enabled) { // Se um cliente BLE habilitou notificações...
            // Solicita ao servidor ATT para enviar um evento 'ATT_EVENT_CAN_SEND_NOW'
            // quando estiver pronto para enviar uma notificação.
            att_server_request_can_send_now_event(con_handle);
        }
    }

    // Inverte o estado do LED.
    static int led_on = true;
    led_on = !led_on;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on); // Controla o LED da placa.

    // Reagenda o worker para a próxima execução.
    async_context_add_at_time_worker_in_ms(context, &heartbeat_worker, HEARTBEAT_PERIOD_MS);
}

/**
 * @brief Função de callback para reportar os resultados do iperf.
 *
 * Esta função é chamada pela biblioteca lwiperf quando um teste iperf é concluído
 * ou para reportar estatísticas intermediárias.
 *
 * @param arg Argumento customizado passado para `lwiperf_start_tcp_server_default` (não usado aqui).
 * @param report_type O tipo de relatório (ex: final, intermediário).
 * @param local_addr Endereço IP local da conexão iperf.
 * @param local_port Porta local da conexão iperf.
 * @param remote_addr Endereço IP remoto da conexão iperf.
 * @param remote_port Porta remota da conexão iperf.
 * @param bytes_transferred Número de bytes transferidos durante o teste/intervalo.
 * @param ms_duration Duração do teste/intervalo em milissegundos.
 * @param bandwidth_kbitpsec Largura de banda medida em kilobits por segundo.
 */
static void iperf_report(void *arg, enum lwiperf_report_type report_type,
                         const ip_addr_t *local_addr, u16_t local_port, const ip_addr_t *remote_addr, u16_t remote_port,
                         u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec) {
    UNUSED(arg);
    UNUSED(report_type);
    UNUSED(local_addr);
    UNUSED(local_port);
    UNUSED(remote_addr);
    UNUSED(remote_port);

    static uint32_t total_iperf_megabytes = 0; // Acumula o total de MBytes transferidos.
    uint32_t mbytes = bytes_transferred / (1024 * 1024); // Converte bytes para Megabytes.
    float mbits = bandwidth_kbitpsec / 1000.0f;         // Converte Kbit/s para Mbit/s.

    total_iperf_megabytes += mbytes;

    printf("Completed iperf transfer of %lu MBytes @ %.1f Mbits/sec\n", mbytes, mbits);
    // "Transferência iperf concluída de %lu MBytes @ %.1f Mbits/seg"
    printf("Total iperf megabytes since start %lu Mbytes\n", total_iperf_megabytes);
    // "Total de megabytes iperf desde o início %lu Mbytes"
}

/**
 * @brief Função principal do programa servidor BLE com Wi-Fi e iperf.
 *
 * Inicializa o hardware (stdio, CYW43, ADC), a pilha Bluetooth (L2CAP, SM, ATT Server),
 * conecta-se à rede Wi-Fi, inicia um servidor iperf, e liga o rádio Bluetooth.
 * Usa o `async_context` do CYW43 para tarefas periódicas como o heartbeat.
 *
 * @return 0 em caso de sucesso, 1 em caso de falha na conexão Wi-Fi, -1 em falha do CYW43.
 */
int main() {
    stdio_init_all(); // Inicializa todas as E/S padrão.
    sleep_ms(2000);   // Pequena pausa inicial, pode ser útil para estabilização ou depuração.

    // Inicializa a arquitetura CYW43.
    // - Habilita BT se CYW43_ENABLE_BLUETOOTH == 1.
    // - Habilita lwIP se CYW43_LWIP == 1.
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n"); // "falha ao inicializar cyw43_arch"
        return -1;
    }

    // Inicializa o ADC para o sensor de temperatura.
    adc_init();
    adc_select_input(ADC_CHANNEL_TEMPSENSOR);
    adc_set_temp_sensor_enabled(true);

    // Inicializa as camadas da pilha Bluetooth.
    l2cap_init();
    sm_init();
    att_server_init(profile_data, att_read_callback, att_write_callback);

    // Registra o manipulador de pacotes para eventos HCI e outros.
    // `packet_handler` (definido em server_common.c) tratará esses eventos.
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Registra o mesmo `packet_handler` para eventos específicos do servidor ATT.
    att_server_register_packet_handler(packet_handler);

    // Adiciona o worker de heartbeat ao contexto assíncrono do CYW43.
    // Isso permite que o heartbeat_handler seja executado cooperativamente
    // com as operações de Wi-Fi e Bluetooth gerenciadas pelo CYW43.
    async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(), &heartbeat_worker, HEARTBEAT_PERIOD_MS);

    // Conecta-se à rede Wi-Fi.
    cyw43_arch_enable_sta_mode(); // Habilita o modo estação (cliente Wi-Fi).
    printf("Connecting to Wi-Fi...\n"); // "Conectando ao Wi-Fi..."
    // Tenta conectar com timeout de 30 segundos.
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n"); // "falha ao conectar."
        return 1;
    } else {
        printf("Connected to Wi-Fi: %s\n", WIFI_SSID); // "Conectado ao Wi-Fi."
        printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default))); // Exibe o endereço IP obtido.
    }

    // Configura e inicia o servidor iperf.
    // `cyw43_arch_lwip_begin()` e `cyw43_arch_lwip_end()` são usados para
    // garantir que as operações LwIP sejam seguras em termos de concorrência
    // com o contexto do CYW43.
    cyw43_arch_lwip_begin();
    printf("\nReady, running iperf server at %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
    // "Pronto, executando servidor iperf em %s" (netif_list é o netif padrão)
    lwiperf_start_tcp_server_default(&iperf_report, NULL); // Inicia o servidor iperf na porta padrão (5001).
    cyw43_arch_lwip_end();

    // Liga o rádio Bluetooth.
    hci_power_control(HCI_POWER_ON);

    // Para o modo 'threadsafe background' do BTstack e com o `async_context` do CYW43,
    // o loop principal pode simplesmente existir ou realizar outras tarefas não bloqueantes.
    // As tarefas de rede e Bluetooth são tratadas em seus respectivos contextos/threads/IRQs.
    while(true) {
        // O trabalho principal é feito por callbacks e workers assíncronos.
        // `tight_loop_contents()` ou `sleep_ms()` podem ser usados aqui.
        // `sleep_ms` é melhor para economizar energia se não houver nada para o loop fazer.
        sleep_ms(1000);
    }

    // Desinicializa a arquitetura CYW43 (raramente alcançado no loop infinito).
    cyw43_arch_deinit();
    return 0;
}