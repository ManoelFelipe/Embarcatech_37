/**
 * @file util.c
 * @brief Implementação de funções de utilidade geral.
 *
 * Contém a lógica para as funções auxiliares, como as de geração de
 * números aleatórios.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#include "util.h"
#include <stdlib.h> // Para rand(), srand(), RAND_MAX
#include <time.h>   // Para time()
#include "pico/time.h" // Para time_us_32() no RP2040

/**
 * @brief Inicializa o gerador de números pseudoaleatórios (RNG).
 *
 * Define a semente (seed) para o RNG.
 * @note A função `time(NULL)` é padrão em sistemas com um relógio de tempo real.
 * Em um microcontrolador como o RP2040, é preferível usar `srand(time_us_32())`
 * (incluindo "pico/time.h") para obter uma semente mais variável a cada inicialização,
 * pois `time_us_32()` retorna o tempo em microssegundos desde a inicialização do chip.
 */
void inicializar_aleatorio(void) {
    // Usar time(NULL) pode não ser ideal no Pico se o RTC não estiver configurado.
    // A abordagem usada no arquivo main `NeoControlLab.c` com `time_us_32()` é mais robusta.
    srand(time(NULL));
}

/**
 * @brief Gera um número inteiro aleatório dentro de um intervalo [min, max].
 *
 * @param min O valor mínimo (inclusivo) do intervalo.
 * @param max O valor máximo (inclusivo) do intervalo.
 * @return int Um número inteiro aleatório no intervalo especificado.
 */
int numero_aleatorio(int min, int max) {
    // A expressão `rand() % (max - min + 1)` gera um número no intervalo [0, max-min].
    // Somar `min` desloca o intervalo para [min, max].
    return rand() % (max - min + 1) + min;
}

/**
 * @brief Gera um número de ponto flutuante aleatório entre 0.0 e 1.0.
 *
 * Divide o valor de `rand()` pelo valor máximo possível (`RAND_MAX`)
 * para normalizar o resultado para o intervalo [0.0, 1.0].
 *
 * @return float Um número aleatório no intervalo [0.0, 1.0].
 */
float numero_aleatorio_0a1(void) {
    return (float)rand() / (float)RAND_MAX;
}