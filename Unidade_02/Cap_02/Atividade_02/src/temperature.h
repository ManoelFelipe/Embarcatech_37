/**
 * @file    temperature.h
 * @brief   Interface pública para o módulo de leitura do sensor de temperatura interno.
 * @details Declara as funções para inicializar o conversor analógico-digital (ADC)
 * e ler a temperatura em graus Celsius.
 */

#ifndef TEMPERATURE_H
#define TEMPERATURE_H

/**
 * @brief Inicializa o hardware necessário para a leitura da temperatura.
 * @details Configura o ADC e habilita o sensor de temperatura interno do RP2040.
 * @param num_samples O número de leituras do ADC a serem feitas para calcular uma média.
 * Isso ajuda a reduzir o ruído e obter um valor mais estável.
 */
void   temperature_init(int num_samples);

/**
 * @brief Lê a temperatura atual do sensor interno.
 * @return A temperatura medida em graus Celsius (°C).
 */
float  temperature_read_c(void);

#endif // TEMPERATURE_H