/**
 * @file testes_cores.c
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `src/testes_cores.c`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#include "lib/LabNeoPixel/neopixel_driver.h"
#include "pico/stdlib.h"
#include "testes_cores.h"
#include "lib/LabNeoPixel/efeitos.h"

typedef struct {
    uint8_t r, g, b;
} CorRGB;

/**
 * @brief Descrição da função preencher_matriz_com_cores.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 */

void preencher_matriz_com_cores(void) {
/**
 * @brief Descrição da função preencher_matriz_com_cores.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 */

    const CorRGB cores[] = {
        {COR_MIN, COR_APAGA, COR_APAGA},  // Vermelho
        {COR_APAGA, COR_MIN, COR_APAGA},  // Verde
        {COR_APAGA, COR_APAGA, COR_MIN},  // Azul
        {COR_MIN, COR_MIN, COR_APAGA},    // Amarelo
        {COR_APAGA, COR_MIN, COR_MIN},    // Ciano
        {COR_MIN, COR_APAGA, COR_MIN},    // Magenta
        {COR_MIN, COR_MIN, COR_MIN}       // Branco suave
    };

    const uint8_t total_cores = sizeof(cores) / sizeof(cores[0]);

    for (uint8_t i = 0; i < total_cores; ++i) {
        npSetAll(cores[i].r, cores[i].g, cores[i].b);
        npWrite();
        sleep_ms(700);
    }
}

/**
 * @brief Descrição da função testar_fileiras_colunas.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 */

void testar_fileiras_colunas(void) {
    // Fileiras de cima para baixo em vermelho suave
    for (uint8_t y = 0; y < NUM_LINHAS; y++) {
        npClear();
        acenderFileira(y, COR_MIN, COR_APAGA, COR_APAGA); // vermelho
        npWrite();
        sleep_ms(250);
    }

    sleep_ms(500);

    // Colunas da esquerda para a direita em azul suave
    for (uint8_t x = 0; x < NUM_COLUNAS; x++) {
        npClear();
        acenderColuna(x, COR_APAGA, COR_APAGA, COR_MIN); // azul
        npWrite();
        sleep_ms(250);
    }

    sleep_ms(500);
}