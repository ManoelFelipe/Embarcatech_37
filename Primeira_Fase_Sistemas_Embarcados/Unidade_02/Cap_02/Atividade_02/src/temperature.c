/**
 * @file    temperature.c
 * @brief   Implementação do módulo de leitura do sensor de temperatura.
 * @details Este módulo utiliza o ADC do RP2040 para ler a voltagem do sensor
 * de temperatura interno e a converte para graus Celsius usando uma
 * fórmula fornecida no datasheet do microcontrolador.
 */

#include "temperature.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"

/// @brief Armazena o número de amostras a serem lidas para o cálculo da média.
static int   samples = 1;
/// @brief Um valor de offset que pode ser usado para calibrar o sensor, se necessário.
static float user_offset = 0.0f;

/**
 * @brief Inicializa o hardware necessário para a leitura da temperatura.
 * @param num_samples O número de leituras do ADC para calcular a média.
 */
void temperature_init(int num_samples)
{
    // Garante que o número de amostras seja pelo menos 1.
    samples = (num_samples > 0) ? num_samples : 1;
    
    // Inicializa o subsistema ADC.
    adc_init();
    // Habilita o sensor de temperatura, que está multiplexado com os pinos do ADC.
    adc_set_temp_sensor_enabled(true);
    // Seleciona o canal 4 do ADC, que é o canal conectado ao sensor de temperatura.
    adc_select_input(4);
}

/**
 * @brief Lê a temperatura atual do sensor interno.
 * @return A temperatura medida em graus Celsius (°C).
 */
float temperature_read_c(void)
{
    // Constantes para a conversão, baseadas no datasheet do RP2040.
    const float VREF = 3.3f;        // Tensão de referência do ADC é 3.3V.
    const float conv = VREF / (1 << 12); // Fator de conversão de valor ADC (12-bit) para voltagem.

    // Acumula a soma das leituras do ADC.
    uint32_t sum = 0;
    for (int i = 0; i < samples; ++i) { 
        sum += adc_read(); 
        sleep_us(5); // Pequena pausa entre leituras.
    }
    
    // Calcula a voltagem média.
    float voltage = (sum / (float)samples) * conv;
    
    // Fórmula de conversão de voltagem para temperatura em Celsius, conforme datasheet do RP2040.
    // A fórmula é: T = 27 - (V_be - 0.706) / 0.001721
    // Onde 27°C é a temperatura de referência, e 0.706V é a voltagem medida a 27°C.
    return 27.0f - (voltage - 0.706f) / 0.001721f + user_offset;
}