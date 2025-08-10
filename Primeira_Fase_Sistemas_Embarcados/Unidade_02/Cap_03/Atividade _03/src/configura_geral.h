/**
 * @file configura_geral.h
 * @author Modificado Por Manoel Furtado
 * @version 1.1
 * @date 15 de junho de 2025
 * @brief Arquivo de configuração central para o projeto do Raspberry Pi Pico W.
 *
 * @details
 * Este header define todas as constantes e configurações globais utilizadas em
 * múltiplos arquivos do sistema. Centralizar essas definições aqui facilita a
 * manutenção e a reconfiguração do projeto, pois todas as principais
 * variáveis de hardware e de rede estão em um único local.
 *
 * Inclui:
 * - Pinos de hardware para periféricos (LED RGB, I2C para OLED).
 * - Parâmetros de temporização para tarefas e conexões.
 * - Credenciais de rede Wi-Fi (SSID e senha).
 * - Configurações do broker MQTT (endereço, porta, tópico).
 * - Declarações de funções e variáveis globais externas.
 *
 * @note **ATENÇÃO:** As credenciais de Wi-Fi (WIFI_SSID, WIFI_PASS) e o endereço
 * do broker MQTT (MQTT_BROKER_IP) são placeholders e **DEVEM** ser alterados
 * para os valores corretos do seu ambiente.
 */

#ifndef CONFIGURA_GERAL_H
#define CONFIGURA_GERAL_H

// =============================================================================
// Includes Padrão e de Hardware
// =============================================================================
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"      // Funções padrão do SDK do Pico (GPIO, etc.)
#include "hardware/pwm.h"       // Funções para controle de PWM (para o LED RGB)
#include "pico/multicore.h"   // Funções para gerenciamento dos dois núcleos (FIFO, etc.)
#include "pico/cyw43_arch.h"  // Funções de arquitetura para o chip Wi-Fi CYW43
#include "pico/time.h"        // Funções de temporização e delays

// =============================================================================
// Definições de Hardware
// =============================================================================

// --- Pinos do LED RGB ---
#define LED_R 12      /**< Pino GPIO conectado ao catodo/anodo vermelho do LED RGB. */
#define LED_G 11      /**< Pino GPIO conectado ao catodo/anodo verde do LED RGB. */
#define LED_B 13      /**< Pino GPIO conectado ao catodo/anodo azul do LED RGB. */
#define PWM_STEP 0xFFFF /**< Valor máximo para o ciclo de trabalho do PWM (100%).
                         * Corresponde à resolução de 16 bits do contador do PWM no RP2040. */

// --- Pinos da Interface I2C para o OLED ---
#define SDA_PIN 14    /**< Pino GPIO para a linha de dados (SDA) do I2C. */
#define SCL_PIN 15    /**< Pino GPIO para a linha de clock (SCL) do I2C. */

// =============================================================================
// Definições de Temporização e Buffers
// =============================================================================
#define TEMPO_CONEXAO 2000    /**< Tempo em milissegundos para exibir mensagens de status de conexão. */
#define TEMPO_MENSAGEM 2000   /**< Tempo em milissegundos para exibir mensagens gerais no OLED. */
#define TAM_FILA 16           /**< Tamanho máximo da fila circular que armazena mensagens do Wi-Fi. */

// =============================================================================
// Configurações de Rede (Wi-Fi e MQTT)
// =============================================================================
// ATENÇÃO: Substitua os valores abaixo pelos da sua rede e broker MQTT.
#define WIFI_SSID "RENASCENCA_Cozinha_multilaser_" /**< O SSID (nome) da sua rede Wi-Fi. */
#define WIFI_PASS "12345678"                 /**< A senha da sua rede Wi-Fi. */
#define MQTT_BROKER_IP "192.168.1.107" /**< O endereço IP do seu broker MQTT (ex: "192.168.1.10"). */
#define MQTT_BROKER_PORT 3004                 /**< A porta padrão do broker MQTT (1883 para não criptografado). */
#define TOPICO "pico/PING"                    /**< Tópico MQTT onde a mensagem "PING" será publicada. */
#define INTERVALO_PING_MS 5000                /**< Intervalo em milissegundos entre o envio de cada "PING" via MQTT. */

// =============================================================================
// Declarações Globais Externas
// =============================================================================

/**
 * @brief Buffer de vídeo global para o display OLED.
 * Este buffer armazena os dados dos pixels que serão desenhados na tela.
 * Ele é definido em `estado_mqtt.c`.
 */
extern uint8_t buffer_oled[];

/**
 * @brief Estrutura global que define a área de renderização do OLED.
 * Usada pelas funções de desenho para saber qual parte da tela atualizar.
 * É definida em `estado_mqtt.c`.
 */
extern struct render_area area;

/**
 * @brief Inicializa o hardware do display OLED (I2C e o controlador SSD1306).
 * Definida em `oled_utils.c` (não fornecido).
 */
void setup_init_oled(void);

/**
 * @brief Exibe uma mensagem em uma linha específica do OLED e aguarda um tempo.
 * Função utilitária para simplificar a exibição de status temporários.
 * Definida em `oled_utils.c` (não fornecido).
 * @param mensagem A string a ser exibida.
 * @param linha_y A coordenada Y (linha) onde a mensagem será exibida.
 */
void exibir_e_esperar(const char *mensagem, int linha_y);

#endif // CONFIGURA_GERAL_H