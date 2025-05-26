/**
 * @file app_config.h
 * @brief Arquivo de configuração central para definições e constantes do projeto.
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

// Definições do Wi-Fi Access Point
#define WIFI_SSID "PICO_ALARME_AP"      /**< Nome da rede Wi-Fi (SSID) a ser criada. */
#define WIFI_PASSWORD "picoalarme123"   /**< Senha da rede Wi-Fi. */
#define TCP_PORT 80                     /**< Porta TCP para o servidor HTTP. */

// Definições dos Pinos GPIO
#define LED_GREEN_GPIO 11   /**< Pino GPIO para o LED Verde (sistema em repouso). */
#define LED_BLUE_GPIO 12    /**< Pino GPIO para o LED Azul (status do Access Point). */
#define LED_RED_GPIO 13     /**< Pino GPIO para o LED Vermelho (alarme ativo). */
#define BUZZER_GPIO 10      /**< Pino GPIO para o Buzzer. */

// Definições do Display OLED I2C
#define I2C_SDA_PIN 14      /**< Pino GPIO para o SDA da comunicação I2C com o OLED. */
#define I2C_SCL_PIN 15      /**< Pino GPIO para o SCL da comunicação I2C com o OLED. */
#define OLED_I2C_CLOCK 400000 /**< Velocidade do clock I2C para o OLED (400 kHz). */

// Configurações do Alarme
#define ALARM_BLINK_INTERVAL_MS 500 /**< Intervalo em milissegundos para piscar o LED vermelho e o buzzer. */

// Mensagens para o Display OLED
#define MSG_EVACUAR "EVACUAR"           /**< Mensagem para estado de alarme ativo. */
#define MSG_REPOUSO_L1 "Sistema em"     /**< Linha 1 para estado de sistema em repouso. */
#define MSG_REPOUSO_L2 "repouso"        /**< Linha 2 para estado de sistema em repouso. */
#define MSG_AP_OFF "AP Desativado"      /**< Mensagem quando o Access Point é desativado. */

// Outras definições do servidor HTTP
#define DEBUG_printf printf /**< Define a função de impressão para debug. Pode ser redirecionada se necessário. */
#define POLL_TIME_S 5     /**< Tempo de poll para conexões TCP (define o intervalo do callback tcp_poll). */

#endif // APP_CONFIG_H