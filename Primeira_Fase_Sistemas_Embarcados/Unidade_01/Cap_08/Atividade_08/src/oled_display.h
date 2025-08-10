/**
 * @file oled_display.h
 * @brief Interface para controle do display OLED SSD1306.
 *
 * Este módulo encapsula a inicialização e as funções de atualização
 * para o display OLED.
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 */

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <stdbool.h> // Para o tipo bool

/**
 * @brief Inicializa o hardware I2C e o display OLED.
 *
 * Configura os pinos I2C, inicializa a comunicação I2C e envia os comandos
 * de inicialização para o display SSD1306.
 * Deve ser chamada uma vez no início do programa.
 */
void oled_display_init(void);

/**
 * @brief Atualiza o display OLED com o status atual do alarme.
 *
 * Exibe "EVACUAR" se o alarme estiver ativo, ou "Sistema em repouso" caso contrário.
 * @param is_alarm_active Verdadeiro se o alarme estiver ativo, falso caso contrário.
 */
void oled_display_update_status(bool is_alarm_active);

/**
 * @brief Exibe a mensagem "AP Desativado" no display OLED.
 *
 * Usada quando o modo Access Point é finalizado pelo usuário.
 */
void oled_display_show_ap_disabled(void);

/**
 * @brief Limpa o conteúdo do display OLED.
 */
void oled_display_clear(void);

#endif // OLED_DISPLAY_H