/**
 * @file server.c
 * @brief Implementação de um servidor GATT (Generic Attribute Profile) Bluetooth Low Energy (BLE).
 *
 * Este código demonstra como um dispositivo Pico configurado como servidor BLE
 * pode anunciar sua presença, aceitar conexões de clientes, e expor um serviço
 * de sensoriamento ambiental com uma característica de temperatura. A temperatura
 * é lida do sensor interno do RP2040 e pode ser notificada a um cliente conectado.
 * O LED da placa pisca para indicar atividade.
 * Este exemplo foca apenas na funcionalidade BLE.
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h" // Para integração do BTstack com cyw43
#include "hardware/adc.h"
#include "pico/stdlib.h"

#include "server_common.h" // Inclui definições e funções comuns ao servidor

#define HEARTBEAT_PERIOD_MS 1000 /**< Período em milissegundos para o timer de heartbeat. */

static btstack_timer_source_t heartbeat;                                       /**< Estrutura do timer para o heartbeat. */
static btstack_packet_callback_registration_t hci_event_callback_registration; /**< Estrutura para registrar o callback de eventos HCI. */

/**
 * @brief Manipulador do timer de heartbeat.
 *
 * Esta função é chamada periodicamente pelo timer.
 * A cada 10 segundos, ela aciona a leitura da temperatura (`poll_temp`).
 * Se as notificações LE estiverem habilitadas por um cliente, solicita o envio
 * de uma notificação da temperatura atual.
 * Também inverte o estado do LED da placa para indicar atividade.
 *
 * @param ts Ponteiro para a estrutura do timer (btstack_timer_source_t).
 */
static void heartbeat_handler(struct btstack_timer_source *ts) {
    static uint32_t counter = 0; // Contador para controlar a frequência da leitura de temperatura.
    counter++;

    // Atualiza a temperatura a cada 10 segundos (10 * HEARTBEAT_PERIOD_MS).
    if (counter % 10 == 0) {
        poll_temp(); // Lê e atualiza o valor da temperatura.
        if (le_notification_enabled) { // Se um cliente habilitou notificações...
            // Solicita ao servidor ATT para enviar um evento 'ATT_EVENT_CAN_SEND_NOW'
            // quando estiver pronto para enviar uma notificação.
            att_server_request_can_send_now_event(con_handle);
        }
    }

    // Inverte o estado do LED.
    static int led_on = true;
    led_on = !led_on;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on); // Controla o LED da placa.

    // Reinicia o timer para a próxima execução.
    btstack_run_loop_set_timer(ts, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(ts);
}

/**
 * @brief Função principal do programa servidor BLE.
 *
 * Inicializa o hardware (stdio, CYW43, ADC), a pilha Bluetooth (L2CAP, SM, ATT Server),
 * registra os manipuladores de eventos e timers, e liga o rádio Bluetooth.
 * O loop principal pode então ser usado para outras tarefas ou simplesmente dormir,
 * pois o BTstack opera em background ou via polling (neste caso, background).
 *
 * @return 0 em caso de sucesso, -1 em caso de falha na inicialização do CYW43.
 */
int main() {
    stdio_init_all(); // Inicializa todas as E/S padrão (para printf, etc.).

    // Inicializa a arquitetura do driver CYW43.
    // Habilita o Bluetooth se CYW43_ENABLE_BLUETOOTH estiver definido como 1.
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n"); // "falha ao inicializar cyw43_arch"
        return -1;
    }

    // Inicializa o ADC para o sensor de temperatura.
    adc_init();
    adc_select_input(ADC_CHANNEL_TEMPSENSOR); // Seleciona o canal do ADC conectado ao sensor de temperatura interno.
    adc_set_temp_sensor_enabled(true);      // Habilita o sensor de temperatura.

    l2cap_init(); // Inicializa a camada L2CAP.
    sm_init();    // Inicializa o Security Manager (SM).

    // Inicializa o servidor ATT (Attribute Protocol).
    // - `profile_data`: Contém a definição dos serviços e características do servidor.
    // - `att_read_callback`: Função chamada quando um cliente tenta ler um atributo.
    // - `att_write_callback`: Função chamada quando um cliente tenta escrever em um atributo.
    att_server_init(profile_data, att_read_callback, att_write_callback);

    // Registra o manipulador de pacotes para eventos HCI e outros.
    // `packet_handler` (definido em server_common.c) tratará esses eventos.
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Registra o mesmo `packet_handler` para eventos específicos do servidor ATT.
    att_server_register_packet_handler(packet_handler);

    // Configura e adiciona o timer de heartbeat ao run loop do BTstack.
    heartbeat.process = &heartbeat_handler;
    btstack_run_loop_set_timer(&heartbeat, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(&heartbeat);

    // Liga o rádio Bluetooth.
    hci_power_control(HCI_POWER_ON);

    // `btstack_run_loop_execute()` é necessário apenas ao usar o método 'polling'.
    // Este exemplo usa o método 'threadsafe background', onde o trabalho BT é
    // tratado em uma IRQ de baixa prioridade. Portanto, o loop principal pode
    // fazer outras coisas ou apenas existir.

#if 0 // btstack_run_loop_execute() não é estritamente necessário aqui, então não o usamos.
    btstack_run_loop_execute();
#else
    // Este núcleo está livre para fazer suas próprias tarefas, exceto ao usar o método 'polling'
    // (nesse caso, você deve usar os métodos btstack_run_loop_ para adicionar trabalho ao run loop).

    // Loop infinito no lugar onde o código do usuário iria, ou apenas para manter o programa rodando.
    while(true) {
        sleep_ms(1000); // Dorme por 1 segundo, economizando energia.
    }
#endif
    return 0; // Teoricamente, nunca alcançado no loop infinito.
}