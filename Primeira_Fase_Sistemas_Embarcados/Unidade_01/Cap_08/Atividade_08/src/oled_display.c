/**
 * @file oled_display.c
 * @brief Implementação para controle do display OLED SSD1306.
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 */

 #include <stdio.h>
#include "oled_display.h"
#include "app_config.h"
#include "ssd1306.h"         // Biblioteca principal do SSD1306
#include "hardware/i2c.h"
#include "pico/stdlib.h"     // Para memset, etc.
#include <string.h>          // Para strlen

// Buffer interno para o conteúdo do display OLED.
// O tamanho é definido pela biblioteca ssd1306 (ssd1306_buffer_length).
static uint8_t oled_buffer[ssd1306_buffer_length];

// Estrutura que define a área de renderização do OLED (tela inteira).
// É definida pela biblioteca ssd1306.
static struct render_area display_area;

/**
 * @brief Inicializa o hardware I2C e o display OLED.
 */
void oled_display_init(void) {
    printf("Inicializando I2C para Display OLED...\n");
    // Usa diretamente i2c1, que corresponde aos pinos GP14 (SDA) e GP15 (SCL)
    // quando configurados para a função I2C.
    i2c_init(i2c1, OLED_I2C_CLOCK); // Inicializa I2C1 na velocidade definida
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C); // Configura pino SDA para função I2C
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C); // Configura pino SCL para função I2C
    // Pull-ups geralmente não são necessários nos pinos do Pico se o módulo OLED já os tiver.
    printf("Pinos I2C configurados (SDA: %d, SCL: %d para i2c1).\n", I2C_SDA_PIN, I2C_SCL_PIN);

    // Define a área de renderização para cobrir todo o display
    display_area.start_column = 0;
    display_area.end_column = ssd1306_width - 1; // ssd1306_width da lib
    display_area.start_page = 0;
    display_area.end_page = ssd1306_n_pages - 1; // ssd1306_n_pages da lib

    // Calcula o tamanho do buffer necessário para esta área.
    // Esta função é externa, fornecida pela biblioteca ssd1306.
    calculate_render_area_buffer_length(&display_area);

    // Inicializa o hardware do display OLED.
    // Esta função é externa, fornecida pela biblioteca ssd1306.
    ssd1306_init();
    printf("Display OLED SSD1306 inicializado.\n");
}

/**
 * @brief Atualiza o display OLED com o status atual do alarme.
 */
void oled_display_update_status(bool is_alarm_active) {
    // Limpa o buffer do OLED preenchendo com zeros (pixels apagados)
    memset(oled_buffer, 0, ssd1306_buffer_length);

    if (is_alarm_active) {
        // Se o alarme estiver ativo, exibe "EVACUAR" centralizado.
        // A altura do caractere é 8 pixels. O display tem geralmente 64 pixels de altura.
        // Posição Y para centralizar verticalmente um texto de 1 linha: (altura_display / 2) - (altura_fonte / 2)
        // Ex: (64 / 2) - (8 / 2) = 32 - 4 = 28
        ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_EVACUAR) * 8)) / 2, 28, MSG_EVACUAR);
    } else {
        // Se o alarme estiver inativo, exibe "Sistema em repouso" em duas linhas, centralizado.
        // Posição Y para 2 linhas: L1 em ~20, L2 em ~36 (ajustar conforme gosto visual)
        ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_REPOUSO_L1) * 8)) / 2, 20, MSG_REPOUSO_L1);
        ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_REPOUSO_L2) * 8)) / 2, 36, MSG_REPOUSO_L2);
    }
    // Envia o conteúdo do buffer para ser renderizado no display OLED.
    // Esta função é externa, fornecida pela biblioteca ssd1306.
    render_on_display(oled_buffer, &display_area);
}

/**
 * @brief Exibe a mensagem "AP Desativado" no display OLED.
 */
void oled_display_show_ap_disabled(void) {
    memset(oled_buffer, 0, ssd1306_buffer_length);
    ssd1306_draw_string(oled_buffer, (ssd1306_width - (strlen(MSG_AP_OFF) * 8)) / 2, 28, MSG_AP_OFF);
    render_on_display(oled_buffer, &display_area);
}

/**
 * @brief Limpa o conteúdo do display OLED.
 */
void oled_display_clear(void) {
    memset(oled_buffer, 0, ssd1306_buffer_length);
    render_on_display(oled_buffer, &display_area);
}