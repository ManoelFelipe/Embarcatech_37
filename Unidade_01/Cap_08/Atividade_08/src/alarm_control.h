/**
 * @file alarm_control.h
 * @brief Interface para controle do sistema de alarme (LEDs e Buzzer).
 *
 * Este módulo gerencia o estado do alarme, controla os LEDs de status
 * (verde, vermelho, azul) e o buzzer.
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 */

#ifndef ALARM_CONTROL_H
#define ALARM_CONTROL_H

#include <stdbool.h> // Para o tipo bool

/**
 * @brief Inicializa os GPIOs para os LEDs e o buzzer.
 *
 * Configura os pinos GPIO como saídas e define seus estados iniciais.
 * O LED verde é ligado (sistema em repouso), os outros ficam desligados.
 */
void alarm_control_init(void);

/**
 * @brief Define o estado do alarme (ativo ou inativo).
 *
 * Atualiza o estado interno do alarme, controla os LEDs verde/vermelho
 * e solicita a atualização do display OLED.
 * @param active Verdadeiro para ativar o alarme, falso para desativar.
 */
void alarm_control_set_active(bool active);

/**
 * @brief Verifica se o alarme está atualmente ativo.
 *
 * @return Verdadeiro se o alarme estiver ativo, falso caso contrário.
 */
bool alarm_control_is_active(void);

/**
 * @brief Processa a lógica de temporização do alarme.
 *
 * Esta função deve ser chamada repetidamente no loop principal do programa.
 * Ela é responsável por piscar o LED vermelho e o buzzer se o alarme estiver ativo.
 */
void alarm_control_process(void);

/**
 * @brief Controla o LED de status do Access Point (AP).
 *
 * @param on Verdadeiro para ligar o LED azul, falso para desligar.
 */
void alarm_control_set_ap_led(bool on);

/**
 * @brief Desliga todas as saídas controladas pelo sistema de alarme.
 *
 * Usado durante o processo de encerramento do programa.
 */
void alarm_control_shutdown_outputs(void);

#endif // ALARM_CONTROL_H