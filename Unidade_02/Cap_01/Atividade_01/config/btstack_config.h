/**
 * @file btstack_config.h
 * @brief Arquivo de configuração para a pilha BTstack no Pico.
 *
 * Define macros para habilitar funcionalidades do BTstack, configurar buffers,
 * tamanhos e outras opções específicas da plataforma Pico.
 */
#ifndef _PICO_BTSTACK_BTSTACK_CONFIG_H
#define _PICO_BTSTACK_BTSTACK_CONFIG_H

// Garante que o BLE está habilitado, caso contrário, gera um erro de compilação.
#ifndef ENABLE_BLE
#error Please link to pico_btstack_ble // Por favor, vincule com pico_btstack_ble
#endif

/**
 * @name Funcionalidades do BTstack que podem ser habilitadas
 * @{
 */
#define ENABLE_LE_PERIPHERAL            /**< Habilita o papel de Periférico LE (Low Energy). Permite que o dispositivo seja descoberto e conectado por outros dispositivos Centrais. */
#define ENABLE_LOG_INFO                 /**< Habilita mensagens de log de nível INFO. Útil para depuração e acompanhamento do fluxo normal. */
#define ENABLE_LOG_ERROR                /**< Habilita mensagens de log de nível ERROR. Importante para identificar problemas críticos. */
#define ENABLE_PRINTF_HEXDUMP           /**< Habilita a função printf_hexdump para imprimir dados em formato hexadecimal. Útil para inspecionar pacotes e buffers. */
/** @} */

// Configurações específicas para o papel de cliente (Central) ou servidor (Periférico)
#if RUNNING_AS_CLIENT
#define ENABLE_LE_CENTRAL               /**< Habilita o papel de Central LE. Permite que o dispositivo descubra e conecte-se a Periféricos. */
#define MAX_NR_GATT_CLIENTS 1           /**< Define o número máximo de clientes GATT que podem ser gerenciados. Se for um cliente, define como 1. */
#else
#define MAX_NR_GATT_CLIENTS 0           /**< Se não for um cliente (ou seja, um periférico), define o número máximo de clientes GATT como 0 (pois ele não atuará como cliente GATT para outros). */
#endif

/**
 * @name Configuração do BTstack: buffers, tamanhos, etc.
 * @{
 */
#define HCI_OUTGOING_PRE_BUFFER_SIZE 4  /**< Define o tamanho do pré-buffer para pacotes HCI de saída. Espaço reservado antes do payload do pacote. */
#define HCI_ACL_PAYLOAD_SIZE (255 + 4)  /**< Define o tamanho máximo do payload ACL (Asynchronous Connection-Less) HCI. (255 bytes de payload + 4 bytes de cabeçalho L2CAP). */
#define HCI_ACL_CHUNK_SIZE_ALIGNMENT 4  /**< Define o alinhamento para o tamanho dos pedaços (chunks) de ACL HCI. Garante que os pedaços sejam múltiplos de 4 bytes. */
#define MAX_NR_HCI_CONNECTIONS 1        /**< Define o número máximo de conexões HCI (Host Controller Interface) simultâneas. */
#define MAX_NR_SM_LOOKUP_ENTRIES 3      /**< Define o número máximo de entradas na tabela de lookup do Security Manager (SM). Usado para armazenar informações de pareamento. */
#define MAX_NR_WHITELIST_ENTRIES 16     /**< Define o número máximo de entradas na whitelist. A whitelist é usada para filtrar dispositivos que podem se conectar. */
#define MAX_NR_LE_DEVICE_DB_ENTRIES 16  /**< Define o número máximo de entradas no banco de dados de dispositivos LE. Armazena informações sobre dispositivos LE conhecidos. */
/** @} */

/**
 * @name Limitação de Buffers ACL/SCO
 * @brief Limita o número de Buffers ACL/SCO usados pela pilha para evitar estouro no barramento compartilhado do CYW43.
 * @{
 */
#define MAX_NR_CONTROLLER_ACL_BUFFERS 3 /**< Define o número máximo de buffers ACL que o controlador pode usar. */
#define MAX_NR_CONTROLLER_SCO_PACKETS 3 /**< Define o número máximo de pacotes SCO (Synchronous Connection-Oriented) que o controlador pode usar. */
/** @} */

/**
 * @name Controle de Fluxo HCI Controller-para-Host
 * @brief Habilita e configura o Controle de Fluxo HCI do Controlador para o Host para evitar estouro no barramento compartilhado do CYW43.
 * @{
 */
#define ENABLE_HCI_CONTROLLER_TO_HOST_FLOW_CONTROL /**< Habilita o mecanismo de controle de fluxo do controlador para o host. */
#define HCI_HOST_ACL_PACKET_LEN (255+4)            /**< Define o tamanho do pacote ACL que o host espera receber. */
#define HCI_HOST_ACL_PACKET_NUM 3                  /**< Define o número de pacotes ACL que o host pode buferizar. */
#define HCI_HOST_SCO_PACKET_LEN 120                /**< Define o tamanho do pacote SCO que o host espera receber. */
#define HCI_HOST_SCO_PACKET_NUM 3                  /**< Define o número de pacotes SCO que o host pode buferizar. */
/** @} */

/**
 * @name Configuração do Banco de Dados de Chaves de Link e Dispositivos LE
 * @brief Utiliza TLV (Tag-Length-Value) sobre a interface do Setor Flash para armazenamento.
 * @{
 */
#define NVM_NUM_DEVICE_DB_ENTRIES 16 /**< Define o número de entradas no banco de dados de dispositivos armazenadas na NVM (Non-Volatile Memory). */
#define NVM_NUM_LINK_KEYS 16         /**< Define o número de chaves de link armazenadas na NVM. */
/** @} */

/**
 * @brief Define o tamanho máximo do banco de dados ATT.
 * Como não fornecemos uma função `malloc` para o BTstack, usamos um banco de dados ATT de tamanho fixo.
 */
#define MAX_ATT_DB_SIZE 512

/**
 * @name Configuração do BTstack HAL (Hardware Abstraction Layer)
 * @{
 */
#define HAVE_EMBEDDED_TIME_MS       /**< Indica que o sistema possui uma função para obter o tempo em milissegundos (`hal_time_ms`). */
// Mapeia btstack_assert para a função assert() do Pico SDK.
#define HAVE_ASSERT                 /**< Indica que uma função de asserção (`btstack_assert`) está disponível. */
// Alguns dongles USB demoram mais para responder ao reset HCI (ex. BCM20702A).
#define HCI_RESET_RESEND_TIMEOUT_MS 1000 /**< Define o timeout em milissegundos para reenviar o comando de reset HCI se não houver resposta. */
#define ENABLE_SOFTWARE_AES128      /**< Habilita a implementação de software do AES128, caso o hardware não forneça. */
#define ENABLE_MICRO_ECC_FOR_LE_SECURE_CONNECTIONS /**< Habilita o uso da biblioteca micro-ecc para Conexões Seguras LE (LE Secure Connections). */
/** @} */

#endif // _PICO_BTSTACK_BTSTACK_CONFIG_H