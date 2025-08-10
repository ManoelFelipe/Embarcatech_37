/**
 * @file    led_control.h
 * @brief   Interface pública para o módulo de controle de um LED.
 * @details Este header declara as funções para inicializar, ligar/desligar
 * e obter o estado de um LED conectado a um pino GPIO.
 */

#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include <stdbool.h> // Para usar o tipo `bool` (true/false)

/**
 * @brief Inicializa o pino GPIO para controlar o LED.
 * @param gpio O número do pino GPIO ao qual o LED está conectado.
 * @param initial_state O estado inicial do LED (true para ligado, false para desligado).
 */
void  led_init(int gpio, bool initial_state);

/**
 * @brief Define o estado do LED.
 * @param on `true` para ligar o LED, `false` para desligá-lo.
 */
void  led_set (bool on);

/**
 * @brief Obtém o estado atual do LED.
 * @return `true` se o LED estiver ligado, `false` caso contrário.
 */
bool  led_get (void);

#endif // LED_CONTROL_H