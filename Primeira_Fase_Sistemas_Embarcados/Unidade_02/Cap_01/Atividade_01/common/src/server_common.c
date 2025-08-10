/**
 * @file server_common.c
 * @brief Código comum para as implementações de servidor BLE.
 *
 * Este arquivo contém definições de dados de anúncio, manipuladores de pacotes,
 * callbacks de leitura/escrita de atributos ATT e a função para ler o sensor
 * de temperatura. É compartilhado entre diferentes exemplos de servidor BLE.
 * Os handles de característica GATT (como ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE)
 * são esperados serem definidos no arquivo "temp_sensor.h" ou em um arquivo incluído por ele,
 * geralmente gerado a partir de uma definição de perfil GATT.
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "btstack.h"
#include "hardware/adc.h"

#include "temp_sensor.h"   // Inclui definições do sensor e, crucialmente, os handles GATT.
#include "server_common.h" // Inclui as declarações deste módulo.

/**
 * @brief Flags de anúncio para BLE.
 * 0x06 = LE General Discoverable Mode + BR/EDR Not Supported.
 */
#define APP_AD_FLAGS 0x06

/**
 * @var adv_data
 * @brief Dados de anúncio (Advertising Data) do servidor BLE.
 *
 * Esta estrutura define o conteúdo que será enviado nos pacotes de anúncio.
 * Inclui:
 * - Flags: Indicam o modo de descoberta e suporte a BR/EDR.
 * - Nome Local Completo: Nome do dispositivo como "Pico XX:XX:XX:XX:XX:XX".
 * O nome é preenchido dinamicamente com o endereço MAC.
 * - Lista Completa de UUIDs de Serviço de 16 bits: Anuncia o serviço de
 * Sensoriamento Ambiental (0x181A).
 */
static uint8_t adv_data[] = {
    // Flags general discoverable
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, APP_AD_FLAGS,
    // Name
    // O nome 'Pico 00:00:00:00:00:00' é um placeholder.
    // O BTstack irá preencher o endereço MAC real aqui se configurado para tal,
    // ou pode ser feito manualmente se necessário.
    // O código atual não preenche o MAC no nome dinamicamente neste array.
    // Para um nome dinâmico com MAC, seria necessário ajustar a inicialização.
    0x17, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'i', 'c', 'o', ' ', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0',
    // UUID do Serviço de Sensoriamento Ambiental (0x181A)
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x1a, 0x18,
};
static const uint8_t adv_data_len = sizeof(adv_data); /**< Comprimento dos dados de anúncio. */

// Variáveis globais definidas em server_common.h e usadas aqui.
int le_notification_enabled;       /**< Flag: 1 se notificações estão habilitadas pelo cliente, 0 caso contrário. */
hci_con_handle_t con_handle;       /**< Handle da conexão atual com um cliente. */
uint16_t current_temp;             /**< Valor atual da temperatura (em centésimos de grau Celsius). */

/**
 * @brief Manipulador de pacotes HCI e ATT.
 *
 * Esta função é chamada pela pilha BTstack quando eventos ocorrem.
 * Trata eventos como:
 * - BTSTACK_EVENT_STATE: Mudança de estado da pilha (ex: pronta para uso).
 * - HCI_EVENT_DISCONNECTION_COMPLETE: Um cliente desconectou.
 * - ATT_EVENT_CAN_SEND_NOW: O servidor ATT está pronto para enviar uma notificação.
 *
 * @param packet_type Tipo do pacote (HCI_EVENT_PACKET, ATT_EVENT_PACKET, etc.).
 * @param channel Canal (não utilizado aqui).
 * @param packet Ponteiro para os dados do pacote.
 * @param size Tamanho do pacote.
 */
void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    UNUSED(size);    // Parâmetro não utilizado.
    UNUSED(channel); // Parâmetro não utilizado.
    bd_addr_t local_addr; // Endereço Bluetooth local.

    if (packet_type != HCI_EVENT_PACKET) return; // Processa apenas pacotes de evento HCI por enquanto.
                                                // Eventos ATT são tratados pelo servidor ATT e geram callbacks (att_read/write).

    uint8_t event_type = hci_event_packet_get_type(packet);
    switch(event_type){
        case BTSTACK_EVENT_STATE: // Evento de mudança de estado do BTstack.
            if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return; // Se não estiver funcionando, ignora.
            gap_local_bd_addr(local_addr); // Obtém o endereço MAC local.
            printf("BTstack up and running on %s.\n", bd_addr_to_str(local_addr)); // "BTstack funcionando em %s."

            // Configura os parâmetros de anúncio.
            uint16_t adv_int_min = 800; // Intervalo mínimo de anúncio (800 * 0.625 ms = 500 ms).
            uint16_t adv_int_max = 800; // Intervalo máximo de anúncio (800 * 0.625 ms = 500 ms).
            uint8_t adv_type = 0;       // Tipo de anúncio: 0 (ADV_IND - conectável e escaneável não direcionado).
            bd_addr_t null_addr;        // Endereço do peer direto (não usado para ADV_IND).
            memset(null_addr, 0, 6);
            gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
            // Parâmetros: intervalo min/max, tipo, tipo de endereço próprio (0=público),
            // endereço do peer (ignorado), canais de anúncio (0x07 = todos os 3), política de filtro (0=nenhuma).

            assert(adv_data_len <= 31); // Garante que os dados de anúncio não excedam o limite BLE (31 bytes).
            gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data); // Define os dados de anúncio.
            gap_advertisements_enable(1); // Habilita os anúncios.

            poll_temp(); // Lê a temperatura inicial.
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE: // Cliente desconectou.
            le_notification_enabled = 0; // Desabilita o flag de notificações.
            con_handle = HCI_CON_HANDLE_INVALID; // Invalida o handle da conexão.
            printf("Client disconnected.\n"); // "Cliente desconectado."
            // Poderia reiniciar anúncios aqui se eles param na conexão,
            // mas o padrão é continuar anunciando ou o BTstack gerencia isso.
            break;

        case ATT_EVENT_CAN_SEND_NOW: // Evento indicando que o servidor ATT pode enviar uma notificação.
            // Envia a notificação com o valor atual da temperatura.
            // ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE é definido no profile_data (gatt)
            // ou, neste caso, esperado de "temp_sensor.h".
            att_server_notify(con_handle, ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE, (uint8_t*)&current_temp, sizeof(current_temp));
            break;

        default:
            break;
    }
}

/**
 * @brief Callback chamado quando um cliente GATT tenta ler um atributo.
 *
 * Esta função é registrada com o servidor ATT durante a inicialização.
 * Se o handle do atributo corresponder ao da característica de temperatura,
 * ela fornece o valor atual da temperatura.
 *
 * @param connection_handle Handle da conexão com o cliente.
 * @param att_handle Handle do atributo que está sendo lido.
 * @param offset Offset dentro do valor do atributo (para leituras longas).
 * @param buffer Ponteiro para o buffer onde os dados lidos devem ser copiados.
 * @param buffer_size Tamanho do buffer fornecido.
 * @return O número de bytes copiados para o buffer, ou 0 se o handle não for reconhecido.
 */
uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size) {
    UNUSED(connection_handle); // Parâmetro não utilizado nesta implementação simples.

    // Verifica se o cliente está tentando ler o valor da característica de temperatura.
    // ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE é o handle do valor da característica.
    if (att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE){
        // Usa uma função helper para lidar com a leitura de blobs (objetos binários grandes),
        // que também funciona para valores pequenos e lida com o offset.
        return att_read_callback_handle_blob((const uint8_t *)&current_temp, sizeof(current_temp), offset, buffer, buffer_size);
    }
    return 0; // Handle de atributo não reconhecido para leitura.
}

/**
 * @brief Callback chamado quando um cliente GATT tenta escrever em um atributo.
 *
 * Esta função é registrada com o servidor ATT. É usada aqui principalmente para
 * permitir que o cliente habilite ou desabilite as notificações para a
 * característica de temperatura, escrevendo no seu Client Characteristic
 * Configuration Descriptor (CCCD).
 *
 * @param connection_handle Handle da conexão com o cliente.
 * @param att_handle Handle do atributo que está sendo escrito.
 * @param transaction_mode Modo da transação de escrita (não utilizado aqui).
 * @param offset Offset dentro do valor do atributo (não utilizado aqui para CCCD).
 * @param buffer Ponteiro para os dados que o cliente está escrevendo.
 * @param buffer_size Tamanho dos dados escritos.
 * @return 0 se a escrita for bem-sucedida ou o handle for reconhecido, ou um código de erro ATT.
 */
int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    UNUSED(transaction_mode); // Parâmetro não utilizado.
    UNUSED(offset);           // Parâmetro não utilizado.
    UNUSED(buffer_size);      // Parâmetro não utilizado (para CCCD, o tamanho é fixo em 2 bytes).

    // Verifica se o cliente está escrevendo no CCCD da característica de temperatura.
    // ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_CLIENT_CONFIGURATION_HANDLE é o handle do CCCD,
    // esperado de "temp_sensor.h".
    if (att_handle != ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_CLIENT_CONFIGURATION_HANDLE) return 0;

    // Verifica se o valor escrito no CCCD é para habilitar notificações.
    le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
    con_handle = connection_handle; // Armazena o handle da conexão para futuras notificações.

    if (le_notification_enabled) {
        printf("Notifications enabled\n"); // "Notificações habilitadas"
        // Se as notificações foram habilitadas, solicita o envio da primeira notificação.
        att_server_request_can_send_now_event(con_handle);
    } else {
        printf("Notifications disabled\n"); // "Notificações desabilitadas"
    }
    return 0; // Escrita bem-sucedida.
}

/**
 * @brief Lê a temperatura do sensor interno do RP2040 e a converte.
 *
 * A função lê o valor bruto do ADC conectado ao sensor de temperatura,
 * realiza as conversões necessárias para obter a temperatura em graus Celsius,
 * e armazena o resultado (multiplicado por 100) na variável global `current_temp`.
 */
void poll_temp(void) {
    adc_select_input(ADC_CHANNEL_TEMPSENSOR); // Certifica-se de que o canal do sensor de temperatura está selecionado.
    uint32_t raw32 = adc_read();              // Lê o valor bruto do ADC (12 bits).
    const uint32_t bits = 12;                 // Resolução do ADC do RP2040.

    // Escala a leitura bruta para um valor de 16 bits usando uma expansão de Taylor (para 8 <= bits <= 16).
    // Isso é feito para normalizar ou ajustar a faixa do valor, embora a fórmula exata
    // e sua derivação possam depender de como o valor de 16 bits será usado posteriormente.
    // No contexto da fórmula de conversão abaixo, o `raw32` (ou `raw16` se ajustado) é usado.
    // Esta linha específica pode ser uma otimização ou ajuste específico para alguma biblioteca.
    // No contexto da fórmula de conversão abaixo, o `raw32` é mais direto.
    // uint16_t raw16 = raw32 << (16 - bits) | raw32 >> (2 * bits - 16); // Não usado diretamente na fórmula abaixo, mantido para referência se necessário.

    // Fator de conversão: tensão de referência do ADC (3.3V) dividida pelo valor máximo de um ADC de 12 bits (4095).
    // A tensão correspondente é `raw32 * (3.3 / (1 << bits))`.

    // Referência: https://github.com/raspberrypi/pico-micropython-examples/blob/master/adc/temperature.py
    // A fórmula de conversão de temperatura no SDK do Pico é:
    // `temp_c = 27.0f - (adc_voltage - 0.706f) / 0.001721f;`
    // onde `adc_voltage = adc_raw_value * (ADC_VREF / (1 << ADC_RESOLUTION_BITS))`.
    // Usando ADC_VREF = 3.3V e ADC_RESOLUTION_BITS = 12.

    // Calculando a tensão a partir do valor bruto de 12 bits:
    float voltage = raw32 * (3.3f / (1 << bits)); // (1 << 12) = 4096. (1<<bits) é 4096. 4095 é (1<<bits) - 1. Usar 4096 é mais comum para a escala completa.

    // O sensor de temperatura mede a tensão Vbe de um diodo bipolar polarizado,
    // conectado ao quinto canal ADC (ADC4).
    // Tipicamente, Vbe = 0.706V a 27 graus C, com uma inclinação (coeficiente de temperatura)
    // de -1.721mV (-0.001721V) por grau Celsius.
    float deg_c = 27.0f - (voltage - 0.706f) / 0.001721f;

    current_temp = (uint16_t)(deg_c * 100); // Armazena a temperatura como um inteiro (centésimos de grau C).
    printf("Write temp %.2f degc (raw: %lu, voltage: %.3fV, stored: %u)\n", deg_c, raw32, voltage, current_temp);
    // "Escreve temp %.2f graus C (bruto: %lu, tensão: %.3fV, armazenado: %u)"
 }