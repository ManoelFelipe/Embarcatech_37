/**
 * @file server_common.h
 * @brief Declarações de funções e variáveis comuns para as implementações de servidor BLE.
 *
 * Este arquivo de cabeçalho define as interfaces para o código compartilhado
 * entre diferentes exemplos de servidor BLE, como manipuladores de pacotes,
 * callbacks ATT e a função de leitura de temperatura.
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SERVER_COMMON_H_
#define SERVER_COMMON_H_

#include "btstack_config.h" // Para definições da pilha BTstack, como hci_con_handle_t

/**
 * @def ADC_CHANNEL_TEMPSENSOR
 * @brief Define o canal do ADC usado para o sensor de temperatura interno do RP2040.
 * O RP2040 possui um sensor de temperatura conectado internamente ao canal 4 do ADC.
 */
#define ADC_CHANNEL_TEMPSENSOR 4

// ---- Variáveis Globais Externas ----
// Estas variáveis são definidas em server_common.c e usadas por outros módulos do servidor.

/**
 * @var le_notification_enabled
 * @brief Flag que indica se as notificações para uma característica LE (Low Energy)
 * foram habilitadas por um cliente conectado.
 * `1` se habilitadas, `0` caso contrário.
 */
extern int le_notification_enabled;

/**
 * @var con_handle
 * @brief Handle da conexão HCI (Host Controller Interface) atual com o cliente.
 * Este handle é necessário para direcionar operações GATT, como notificações,
 * para o cliente correto. É `HCI_CON_HANDLE_INVALID` se não houver conexão ativa.
 */
extern hci_con_handle_t con_handle;

/**
 * @var current_temp
 * @brief Armazena o valor atual da temperatura lido pelo sensor.
 * O valor é geralmente armazenado em centésimos de grau Celsius para evitar
 * o uso de ponto flutuante em transferências BLE e facilitar a representação
 * como um inteiro de 16 bits. Por exemplo, 25.50°C seria armazenado como 2550.
 */
extern uint16_t current_temp;

/**
 * @var profile_data
 * @brief Ponteiro para os dados do perfil GATT do servidor.
 * Esta é uma estrutura de dados (geralmente um array de bytes) que define
 * os serviços, características e descritores que o servidor BLE expõe.
 * É tipicamente gerada a partir de um arquivo .gatt usando o compilador GATT do BTstack.
 * A definição real desta variável está no arquivo .gatt compilado (ex: `profile.c` ou similar).
 */
extern uint8_t const profile_data[];


// ---- Declarações de Funções ----

/**
 * @brief Manipulador de pacotes genérico para eventos HCI e ATT.
 *
 * Esta função é registrada no BTstack para receber e processar vários tipos
 * de eventos, como mudanças de estado da pilha, eventos de conexão/desconexão
 * e eventos que indicam que o servidor ATT está pronto para enviar dados.
 *
 * @param packet_type O tipo do pacote recebido (ex: HCI_EVENT_PACKET).
 * @param channel O canal no qual o pacote foi recebido (geralmente não usado para eventos).
 * @param packet Ponteiro para o buffer contendo os dados do pacote.
 * @param size O tamanho do pacote em bytes.
 */
void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

/**
 * @brief Callback chamado quando um cliente GATT tenta ler um atributo do servidor.
 *
 * O servidor ATT invoca esta função quando recebe uma solicitação de leitura de um cliente.
 * A implementação desta função deve verificar o `att_handle` para determinar qual
 * atributo está sendo solicitado e, se for um atributo legível e válido, copiar
 * seu valor para o `buffer` fornecido, respeitando o `offset` e `buffer_size`.
 *
 * @param connection_handle O handle da conexão com o cliente solicitante.
 * @param att_handle O handle do atributo que o cliente deseja ler.
 * @param offset O offset dentro do valor do atributo a partir do qual a leitura deve começar
 * (útil para atributos longos).
 * @param buffer Ponteiro para o buffer onde os dados lidos devem ser colocados.
 * @param buffer_size O tamanho máximo do `buffer`.
 * @return O número de bytes copiados para o `buffer`. Se o atributo não for encontrado
 * ou não for legível, ou se o `offset` for inválido, deve retornar 0 ou um
 * código de erro apropriado se a assinatura da função permitisse (aqui, 0 indica falha).
 */
uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size);

/**
 * @brief Callback chamado quando um cliente GATT tenta escrever em um atributo do servidor.
 *
 * O servidor ATT invoca esta função quando recebe uma solicitação de escrita de um cliente.
 * A implementação deve verificar o `att_handle` para identificar o atributo alvo.
 * Se for um atributo gravável e válido (ex: Client Characteristic Configuration Descriptor - CCCD),
 * ela deve processar os dados do `buffer` e atualizar o estado do servidor conforme necessário.
 *
 * @param connection_handle O handle da conexão com o cliente solicitante.
 * @param att_handle O handle do atributo no qual o cliente deseja escrever.
 * @param transaction_mode Modo da transação de escrita (ex: ATT_TRANSACTION_MODE_NONE, ATT_TRANSACTION_MODE_VALIDATE).
 * @param offset O offset dentro do valor do atributo onde a escrita deve começar
 * (geralmente 0 para CCCDs e atributos curtos).
 * @param buffer Ponteiro para o buffer contendo os dados que o cliente deseja escrever.
 * @param buffer_size O tamanho dos dados no `buffer`.
 * @return 0 se a escrita for aceita. Caso contrário, um código de erro ATT
 * (definido em `att.h`, ex: `ATT_ERROR_ATTRIBUTE_NOT_FOUND`, `ATT_ERROR_WRITE_NOT_PERMITTED`).
 */
int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);

/**
 * @brief Realiza a leitura do sensor de temperatura e atualiza a variável global `current_temp`.
 *
 * Esta função encapsula a lógica para selecionar o canal ADC correto, ler o valor bruto,
 * convertê-lo para graus Celsius e armazenar o resultado formatado (geralmente
 * multiplicado por 100 para representar centésimos de grau) na variável `current_temp`.
 */
void poll_temp(void);

#endif /* SERVER_COMMON_H_ */