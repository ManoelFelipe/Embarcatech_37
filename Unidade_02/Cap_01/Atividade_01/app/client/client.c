/**
 * @file client.c
 * @brief Implementação de um cliente GATT (Generic Attribute Profile) Bluetooth Low Energy (BLE).
 *
 * Este código demonstra como um dispositivo Pico configurado como cliente BLE
 * pode escanear por periféricos, conectar-se a eles, descobrir serviços e
 * características, e registrar-se para notificações de mudança de valor de
 * uma característica específica (neste caso, um sensor de temperatura).
 * O LED da placa pisca para indicar o estado da conexão e atividade.
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

// Macro para habilitar/desabilitar logs de depuração.
// Se definido como 1, usa printf para logs. Se 0, os logs são desabilitados.
#if 0
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

#define LED_QUICK_FLASH_DELAY_MS 100  /**< Define o intervalo em milissegundos para o piscar rápido do LED. */
#define LED_SLOW_FLASH_DELAY_MS 1000 /**< Define o intervalo em milissegundos para o piscar lento do LED. */

/**
 * @enum gc_state_t
 * @brief Enumeração dos estados da máquina de estados do cliente GATT.
 */
typedef enum {
    TC_OFF,                             /**< Estado inicial, Bluetooth desligado ou inativo. */
    TC_IDLE,                            /**< Estado ocioso, BTstack pronto, mas não fazendo nada específico. */
    TC_W4_SCAN_RESULT,                  /**< Esperando por resultados da varredura de dispositivos BLE. */
    TC_W4_CONNECT,                      /**< Tentando conectar a um periférico descoberto. */
    TC_W4_SERVICE_RESULT,               /**< Esperando pelo resultado da descoberta de serviços GATT. */
    TC_W4_CHARACTERISTIC_RESULT,        /**< Esperando pelo resultado da descoberta de características GATT. */
    TC_W4_ENABLE_NOTIFICATIONS_COMPLETE,/**< Esperando pela confirmação de que as notificações foram habilitadas. */
    TC_W4_READY                         /**< Conectado e pronto para receber notificações ou realizar outras operações GATT. */
} gc_state_t;

static btstack_packet_callback_registration_t hci_event_callback_registration; /**< Estrutura para registrar o callback de eventos HCI. */
static gc_state_t state = TC_OFF;                                              /**< Estado atual da máquina de estados do cliente. */
static bd_addr_t server_addr;                                                  /**< Endereço Bluetooth do servidor/periférico ao qual se conectar. */
static bd_addr_type_t server_addr_type;                                        /**< Tipo de endereço do servidor (público, aleatório). */
static hci_con_handle_t connection_handle;                                     /**< Handle da conexão HCI com o periférico. */
static gatt_client_service_t server_service;                                   /**< Estrutura para armazenar informações sobre o serviço descoberto no servidor. */
static gatt_client_characteristic_t server_characteristic;                     /**< Estrutura para armazenar informações sobre a característica descoberta no servidor. */
static bool listener_registered = false;                                       /**< Flag para indicar se um listener de notificação está registrado. */
static gatt_client_notification_t notification_listener;                       /**< Estrutura para o listener de notificações GATT. */
static btstack_timer_source_t heartbeat;                                       /**< Timer para o piscar do LED (heartbeat). */

/**
 * @brief Inicia o processo de varredura por dispositivos BLE.
 *
 * Configura os parâmetros de varredura e inicia a varredura.
 * Muda o estado para TC_W4_SCAN_RESULT.
 */
static void client_start(void){
    DEBUG_LOG("Start scanning!\n"); // "Iniciando varredura!"
    state = TC_W4_SCAN_RESULT;
    // Configura os parâmetros de varredura: tipo de varredura (0), intervalo e janela de varredura (0x0030 = 48 * 0.625ms = 30ms).
    gap_set_scan_parameters(0,0x0030, 0x0030);
    gap_start_scan();
}

/**
 * @brief Verifica se um relatório de anúncio (advertisement report) contém um UUID de serviço específico.
 * @param service O UUID do serviço de 16 bits a ser procurado.
 * @param advertisement_report Ponteiro para o buffer do relatório de anúncio.
 * @return true se o serviço for encontrado, false caso contrário.
 */
static bool advertisement_report_contains_service(uint16_t service, uint8_t *advertisement_report){
    // Obtém os dados do anúncio do evento de relatório.
    const uint8_t * adv_data = gap_event_advertising_report_get_data(advertisement_report);
    uint8_t adv_len  = gap_event_advertising_report_get_data_length(advertisement_report);

    // Itera sobre os dados do anúncio.
    ad_context_t context;
    for (ad_iterator_init(&context, adv_len, adv_data) ; ad_iterator_has_more(&context) ; ad_iterator_next(&context)){
        uint8_t data_type = ad_iterator_get_data_type(&context); // Tipo do dado (ex: Flags, Nome Local Completo, Lista de UUIDs de Serviço).
        uint8_t data_size = ad_iterator_get_data_len(&context);  // Tamanho do dado.
        const uint8_t * data = ad_iterator_get_data(&context);    // Ponteiro para o dado.
        switch (data_type){
            case BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS:
                // Se for uma lista completa de UUIDs de serviço de 16 bits.
                for (int i = 0; i < data_size; i += 2) { // UUIDs de 16 bits ocupam 2 bytes.
                    uint16_t type = little_endian_read_16(data, i); // Lê o UUID.
                    if (type == service) return true; // Serviço encontrado.
                }
            default:
                break;
        }
    }
    return false; // Serviço não encontrado.
}

/**
 * @brief Manipulador de eventos GATT do cliente.
 *
 * Processa eventos recebidos do servidor GATT, como resultados de descoberta
 * de serviços, características e notificações de mudança de valor.
 *
 * @param packet_type Tipo do pacote (geralmente GATT_EVENT_PACKET).
 * @param channel Canal (não utilizado aqui).
 * @param packet Ponteiro para o buffer do pacote.
 * @param size Tamanho do pacote.
 */
static void handle_gatt_client_event(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(packet_type); // Parâmetro não utilizado.
    UNUSED(channel);     // Parâmetro não utilizado.
    UNUSED(size);        // Parâmetro não utilizado.

    uint8_t att_status; // Status de uma operação ATT (Attribute Protocol).

    switch(state){
        case TC_W4_SERVICE_RESULT: // Aguardando resultado da descoberta de serviço.
            switch(hci_event_packet_get_type(packet)) {
                case GATT_EVENT_SERVICE_QUERY_RESULT:
                    // Serviço encontrado, armazena suas informações.
                    DEBUG_LOG("Storing service\n"); // "Armazenando serviço"
                    gatt_event_service_query_result_get_service(packet, &server_service);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    // Consulta de serviços concluída.
                    att_status = gatt_event_query_complete_get_att_status(packet);
                    if (att_status != ATT_ERROR_SUCCESS){
                        printf("SERVICE_QUERY_RESULT, ATT Error 0x%02x.\n", att_status);
                        gap_disconnect(connection_handle); // Desconecta se houver erro.
                        break;
                    }
                    // Consulta de serviço bem-sucedida, agora procura pela característica.
                    state = TC_W4_CHARACTERISTIC_RESULT;
                    DEBUG_LOG("Search for env sensing characteristic.\n"); // "Procurar por característica de sensoriamento ambiental."
                    // Descobre características para o serviço encontrado, filtrando pelo UUID da característica de Temperatura.
                    gatt_client_discover_characteristics_for_service_by_uuid16(handle_gatt_client_event, connection_handle, &server_service, ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE);
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_CHARACTERISTIC_RESULT: // Aguardando resultado da descoberta de característica.
            switch(hci_event_packet_get_type(packet)) {
                case GATT_EVENT_CHARACTERISTIC_QUERY_RESULT:
                    // Característica encontrada, armazena suas informações.
                    DEBUG_LOG("Storing characteristic\n"); // "Armazenando característica"
                    gatt_event_characteristic_query_result_get_characteristic(packet, &server_characteristic);
                    break;
                case GATT_EVENT_QUERY_COMPLETE:
                    // Consulta de características concluída.
                    att_status = gatt_event_query_complete_get_att_status(packet);
                    if (att_status != ATT_ERROR_SUCCESS){
                        printf("CHARACTERISTIC_QUERY_RESULT, ATT Error 0x%02x.\n", att_status);
                        gap_disconnect(connection_handle); // Desconecta se houver erro.
                        break;
                    }
                    // Característica encontrada, registra o handler para notificações.
                    listener_registered = true;
                    gatt_client_listen_for_characteristic_value_updates(&notification_listener, handle_gatt_client_event, connection_handle, &server_characteristic);
                    // Habilita as notificações para esta característica.
                    DEBUG_LOG("Enable notify on characteristic.\n"); // "Habilitar notificação na característica."
                    state = TC_W4_ENABLE_NOTIFICATIONS_COMPLETE;
                    // Escreve no Client Characteristic Configuration Descriptor (CCCD) para habilitar notificações.
                    gatt_client_write_client_characteristic_configuration(handle_gatt_client_event, connection_handle,
                        &server_characteristic, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_ENABLE_NOTIFICATIONS_COMPLETE: // Aguardando confirmação da habilitação de notificações.
            switch(hci_event_packet_get_type(packet)) {
                case GATT_EVENT_QUERY_COMPLETE:
                    // Confirmação recebida.
                    DEBUG_LOG("Notifications enabled, ATT status 0x%02x\n", gatt_event_query_complete_get_att_status(packet)); // "Notificações habilitadas"
                    if (gatt_event_query_complete_get_att_status(packet) != ATT_ERROR_SUCCESS) break; // Se erro, não avança.
                    state = TC_W4_READY; // Pronto para receber notificações.
                    break;
                default:
                    break;
            }
            break;
        case TC_W4_READY: // Conectado e pronto.
            switch(hci_event_packet_get_type(packet)) {
                case GATT_EVENT_NOTIFICATION: { // Notificação recebida.
                    uint16_t value_length = gatt_event_notification_get_value_length(packet);
                    const uint8_t *value = gatt_event_notification_get_value(packet);
                    DEBUG_LOG("Indication value len %d\n", value_length); // "Comprimento do valor da indicação"
                    if (value_length == 2) { // Espera-se que o valor da temperatura seja de 2 bytes.
                        // Lê o valor da temperatura (esperado em centésimos de grau Celsius).
                        float temp = little_endian_read_16(value, 0);
                        printf("read temp %.2f degc\n", temp / 100); // "temperatura lida %.2f graus C"
                    } else {
                        printf("Unexpected length %d\n", value_length); // "Comprimento inesperado"
                    }
                    break;
                }
                default:
                    printf("Unknown packet type 0x%02x\n", hci_event_packet_get_type(packet)); // "Tipo de pacote desconhecido"
                    break;
            }
            break;
        default:
            printf("error\n"); // "erro"
            break;
    }
}

/**
 * @brief Manipulador de eventos HCI (Host Controller Interface).
 *
 * Processa eventos gerais do BTstack, como o estado da pilha,
 * relatórios de anúncio de dispositivos próximos, eventos de conexão e desconexão.
 *
 * @param packet_type Tipo do pacote (geralmente HCI_EVENT_PACKET).
 * @param channel Canal (não utilizado aqui).
 * @param packet Ponteiro para o buffer do pacote.
 * @param size Tamanho do pacote.
 */
static void hci_event_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(size);    // Parâmetro não utilizado.
    UNUSED(channel); // Parâmetro não utilizado.
    bd_addr_t local_addr; // Endereço Bluetooth local.

    if (packet_type != HCI_EVENT_PACKET) return; // Somente processa pacotes de evento HCI.

    uint8_t event_type = hci_event_packet_get_type(packet);
    switch(event_type){
        case BTSTACK_EVENT_STATE: // Evento de mudança de estado do BTstack.
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                // BTstack está pronto e funcionando.
                gap_local_bd_addr(local_addr); // Obtém o endereço local.
                printf("BTstack up and running on %s.\n", bd_addr_to_str(local_addr)); // "BTstack funcionando em %s."
                client_start(); // Inicia o processo do cliente (varredura).
            } else {
                state = TC_OFF; // Se não estiver funcionando, volta ao estado OFF.
            }
            break;
        case GAP_EVENT_ADVERTISING_REPORT: // Relatório de anúncio de um dispositivo próximo.
            if (state != TC_W4_SCAN_RESULT) return; // Só processa se estiver esperando por resultados de varredura.
            // Verifica se o anúncio contém o serviço de Sensoriamento Ambiental.
            if (!advertisement_report_contains_service(ORG_BLUETOOTH_SERVICE_ENVIRONMENTAL_SENSING, packet)) return;
            // Serviço encontrado, armazena o endereço e tipo do dispositivo.
            gap_event_advertising_report_get_address(packet, server_addr);
            server_addr_type = gap_event_advertising_report_get_address_type(packet);
            // Para a varredura e tenta conectar ao dispositivo.
            state = TC_W4_CONNECT;
            gap_stop_scan();
            printf("Connecting to device with addr %s.\n", bd_addr_to_str(server_addr)); // "Conectando ao dispositivo com endereço %s."
            gap_connect(server_addr, server_addr_type);
            break;
        case HCI_EVENT_LE_META: // Meta evento LE.
            // Espera pelo evento de conexão completa.
            switch (hci_event_le_meta_get_subevent_code(packet)) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: // Conexão LE estabelecida.
                    if (state != TC_W4_CONNECT) return; // Só processa se estiver no estado de tentativa de conexão.
                    connection_handle = hci_subevent_le_connection_complete_get_connection_handle(packet);
                    // Conexão bem-sucedida, agora descobre os serviços primários do periférico.
                    DEBUG_LOG("Search for env sensing service.\n"); // "Procurar por serviço de sensoriamento ambiental."
                    state = TC_W4_SERVICE_RESULT;
                    // Descobre serviços primários filtrando pelo UUID do serviço de Sensoriamento Ambiental.
                    gatt_client_discover_primary_services_by_uuid16(handle_gatt_client_event, connection_handle, ORG_BLUETOOTH_SERVICE_ENVIRONMENTAL_SENSING);
                    break;
                default:
                    break;
            }
            break;
        case HCI_EVENT_DISCONNECTION_COMPLETE: // Evento de desconexão.
            connection_handle = HCI_CON_HANDLE_INVALID; // Invalida o handle da conexão.
            if (listener_registered){
                // Se um listener estava registrado, remove-o.
                listener_registered = false;
                gatt_client_stop_listening_for_characteristic_value_updates(&notification_listener);
            }
            printf("Disconnected %s\n", bd_addr_to_str(server_addr)); // "Desconectado de %s"
            if (state == TC_OFF) break; // Se já estava desligado, não faz nada.
            client_start(); // Reinicia o processo do cliente (nova varredura).
            break;
        default:
            break;
    }
}

/**
 * @brief Manipulador do timer de heartbeat.
 *
 * Inverte o estado do LED da placa em intervalos regulares para indicar atividade.
 * O intervalo de piscar muda se as notificações estiverem ativas.
 *
 * @param ts Ponteiro para a estrutura do timer.
 */
static void heartbeat_handler(struct btstack_timer_source *ts) {
    static bool quick_flash = false; // Controla o piscar rápido.
    static bool led_on = true;       // Estado atual do LED (ligado/desligado).

    led_on = !led_on; // Inverte o estado do LED.
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on); // Aplica o novo estado ao LED.

    // Lógica para piscar rápido quando notificações estão ativas e o LED está aceso.
    if (listener_registered && led_on) {
        quick_flash = !quick_flash;
    } else if (!listener_registered) {
        quick_flash = false;
    }

    // Reinicia o timer com o intervalo apropriado.
    // Se 'led_on' ou 'quick_flash' for verdadeiro, usa o atraso rápido, senão o lento.
    btstack_run_loop_set_timer(ts, (led_on || quick_flash) ? LED_QUICK_FLASH_DELAY_MS : LED_SLOW_FLASH_DELAY_MS);
    btstack_run_loop_add_timer(ts); // Adiciona o timer de volta ao run loop.
}

/**
 * @brief Função principal do programa cliente.
 *
 * Inicializa o hardware, a pilha Bluetooth, registra os manipuladores de eventos
 * e inicia o loop de execução do BTstack.
 *
 * @return 0 em caso de sucesso, -1 em caso de falha na inicialização.
 */
int main() {
    stdio_init_all(); // Inicializa todas as E/S padrão (para printf, etc.).

    // Inicializa a arquitetura do driver CYW43 (que habilita o BT se CYW43_ENABLE_BLUETOOTH == 1).
    if (cyw43_arch_init()) {
        printf("failed to initialise cyw43_arch\n"); // "falha ao inicializar cyw43_arch"
        return -1;
    }

    l2cap_init(); // Inicializa a camada L2CAP.
    sm_init();    // Inicializa o Security Manager (SM).
    // Define as capacidades de E/S do dispositivo (sem entrada, sem saída - para Just Works pairing).
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    // Configura um servidor ATT vazio. Necessário apenas se o Periférico LE fizer consultas ATT por conta própria (ex: Android, iOS).
    att_server_init(NULL, NULL, NULL);

    gatt_client_init(); // Inicializa o cliente GATT.

    // Registra o manipulador de eventos HCI.
    hci_event_callback_registration.callback = &hci_event_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Configura o timer de heartbeat.
    heartbeat.process = &heartbeat_handler;
    btstack_run_loop_set_timer(&heartbeat, LED_SLOW_FLASH_DELAY_MS);
    btstack_run_loop_add_timer(&heartbeat);

    // Liga o rádio Bluetooth!
    hci_power_control(HCI_POWER_ON);

    // btstack_run_loop_execute() é necessário apenas ao usar o método 'polling'.
    // Este exemplo usa o método 'threadsafe background', onde o trabalho BT é
    // tratado em uma IRQ de baixa prioridade.
    // É seguro chamar btstack_run_loop_execute(), mas também se pode continuar
    // executando código do usuário.

#if 1 // Necessário apenas com polling (que não estamos usando, mas mostrando que é seguro chamar).
    btstack_run_loop_execute();
#else
    // Este núcleo está livre para fazer suas próprias tarefas, exceto ao usar o método 'polling'
    // (nesse caso, você deve usar os métodos btstack_run_loop_ para adicionar trabalho ao run loop).

    // Loop infinito no lugar onde o código do usuário iria.
    while(true) {
        sleep_ms(1000);
    }
#endif
    return 0;
}