/**
 * @file alarm_control.c
 * @brief Implementação do controle do sistema de alarme (LEDs e Buzzer).
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 */

#include <stdio.h>
#include "alarm_control.h"
#include "app_config.h"
#include "oled_display.h"    // Para atualizar o display quando o estado do alarme muda
#include "pico/stdlib.h"     // Para time_us_64()
#include "hardware/gpio.h"

// Variáveis estáticas para encapsular o estado do alarme dentro deste módulo.
static volatile bool s_alarm_active = false;            /**< Estado atual do alarme (ativo/inativo). */
static bool s_alarm_output_toggle_state = false;    /**< Estado de toggle para piscar LED/Buzzer. */
static uint64_t s_last_toggle_time_us = 0;          /**< Timestamp do último toggle do LED/Buzzer. */

/**
 * @brief Inicializa os GPIOs para os LEDs e o buzzer.
 */
void alarm_control_init(void) {
    // LED Verde (sistema em repouso)
    gpio_init(LED_GREEN_GPIO);
    gpio_set_dir(LED_GREEN_GPIO, GPIO_OUT);
    gpio_put(LED_GREEN_GPIO, true); // Ligado inicialmente

    // LED Azul (status do AP)
    gpio_init(LED_BLUE_GPIO);
    gpio_set_dir(LED_BLUE_GPIO, GPIO_OUT);
    gpio_put(LED_BLUE_GPIO, false); // Desligado inicialmente

    // LED Vermelho (alarme ativo)
    gpio_init(LED_RED_GPIO);
    gpio_set_dir(LED_RED_GPIO, GPIO_OUT);
    gpio_put(LED_RED_GPIO, false); // Desligado inicialmente

    // Buzzer
    gpio_init(BUZZER_GPIO);
    gpio_set_dir(BUZZER_GPIO, GPIO_OUT);
    gpio_put(BUZZER_GPIO, false); // Desligado inicialmente

    printf("GPIOs para LEDs e Buzzer inicializados.\n");
}

/**
 * @brief Define o estado do alarme (ativo ou inativo).
 */
void alarm_control_set_active(bool active) {
    // Muda o estado apenas se for diferente do atual para evitar processamento desnecessário
    if (s_alarm_active != active) {
        s_alarm_active = active;
        oled_display_update_status(s_alarm_active); // Atualiza o display OLED

        if (s_alarm_active) {
            printf("Alarme ATIVADO.\n");
            gpio_put(LED_GREEN_GPIO, false); // Apaga LED verde
            // LED vermelho e buzzer serão controlados por alarm_control_process()
            s_last_toggle_time_us = time_us_64(); // Reseta o timer de blink para iniciar imediatamente
            s_alarm_output_toggle_state = false; // Garante que o primeiro blink acenda
        } else {
            printf("Alarme DESATIVADO.\n");
            gpio_put(LED_RED_GPIO, false);    // Apaga LED vermelho
            gpio_put(BUZZER_GPIO, false);  // Desliga buzzer
            gpio_put(LED_GREEN_GPIO, true);   // Acende LED verde
            s_alarm_output_toggle_state = false; // Reseta estado do toggle
        }
    }
}

/**
 * @brief Verifica se o alarme está atualmente ativo.
 */
bool alarm_control_is_active(void) {
    return s_alarm_active;
}

/**
 * @brief Processa a lógica de temporização do alarme.
 */
void alarm_control_process(void) {
    if (s_alarm_active) {
        uint64_t current_time_us = time_us_64();
        // Verifica se passou o intervalo para alternar o estado do LED/Buzzer
        if (current_time_us - s_last_toggle_time_us >= (ALARM_BLINK_INTERVAL_MS * 1000)) {
            s_alarm_output_toggle_state = !s_alarm_output_toggle_state; // Alterna o estado
            gpio_put(LED_RED_GPIO, s_alarm_output_toggle_state);      // Aplica ao LED vermelho
            gpio_put(BUZZER_GPIO, s_alarm_output_toggle_state);       // Aplica ao buzzer
            s_last_toggle_time_us = current_time_us;                  // Atualiza o timestamp
        }
    }
    // Se o alarme não estiver ativo, os LEDs e buzzer já foram definidos por alarm_control_set_active()
}

/**
 * @brief Controla o LED de status do Access Point (AP).
 */
void alarm_control_set_ap_led(bool on) {
    gpio_put(LED_BLUE_GPIO, on);
}

/**
 * @brief Desliga todas as saídas controladas pelo sistema de alarme.
 */
void alarm_control_shutdown_outputs(void) {
    gpio_put(LED_RED_GPIO, false);
    gpio_put(BUZZER_GPIO, false);
    gpio_put(LED_GREEN_GPIO, false);
    // O LED azul (AP_LED) é controlado separadamente pela lógica de rede/main.
}