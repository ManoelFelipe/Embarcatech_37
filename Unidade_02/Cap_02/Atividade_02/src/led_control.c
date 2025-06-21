/**
 * @file    led_control.c
 * @brief   Implementação do módulo de controle de LED.
 * @details Contém a lógica para manipular um pino GPIO como uma saída digital
 * para acender ou apagar um LED.
 */

#include "led_control.h"
#include "pico/stdlib.h"

/**
 * @brief Variável estática para armazenar o número do pino GPIO do LED.
 * @details Sendo `static`, esta variável só é visível dentro deste arquivo.
 * O valor 25 é o padrão para o LED on-board do Pico (não o Pico W).
 * A função led_init() irá atualizar este valor.
 */
static int led_gpio = 25; 

/**
 * @brief Inicializa o pino GPIO para controlar o LED.
 * @param gpio O número do pino GPIO ao qual o LED está conectado.
 * @param initial_state O estado inicial do LED (true para ligado, false para desligado).
 */
void led_init(int gpio, bool initial_state)
{
    // Armazena o pino GPIO fornecido para uso por outras funções do módulo.
    led_gpio = gpio;
    // Inicializa o pino GPIO.
    gpio_init(led_gpio);
    // Configura a direção do pino como saída (OUTPUT).
    gpio_set_dir(led_gpio, GPIO_OUT);
    // Define o estado inicial do LED.
    led_set(initial_state);
}

/**
 * @brief Define o estado do LED.
 * @param on `true` para ligar o LED, `false` para desligá-lo.
 */
void led_set(bool on) { 
    // `gpio_put` define o nível lógico do pino. `on` (true) corresponde ao nível alto (3.3V).
    gpio_put(led_gpio, on); 
}

/**
 * @brief Obtém o estado atual do LED.
 * @return `true` se o LED estiver ligado, `false` caso contrário.
 */
bool led_get(void) { 
    // `gpio_get` lê o último valor que foi escrito no pino de saída.
    return gpio_get(led_gpio); 
}