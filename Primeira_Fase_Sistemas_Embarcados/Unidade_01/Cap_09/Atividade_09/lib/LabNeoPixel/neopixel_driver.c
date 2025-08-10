/**
 * @file neopixel_driver.c
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/LabNeoPixel/neopixel_driver.c`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#include "neopixel_driver.h"
#include "ws2818b.pio.h"

npLED_t leds[LED_COUNT];
PIO np_pio;
int sm;

/**
 * @brief Descrição da função npInit.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param pin Descrição do parâmetro pin.
 */

void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = 0; // Usar SM 0 fixamente
    pio_sm_claim(np_pio, sm);
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    npClear();
}

/**
 * @brief Descrição da função npWrite.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 */

void npWrite(void) {
    for (uint i = 0; i < LED_COUNT; ++i) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

/**
 * @brief Descrição da função npWriteComBrilho.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param brilho Descrição do parâmetro brilho.
 */

void npWriteComBrilho(float brilho) {
    for (uint i = 0; i < LED_COUNT; ++i) {
        uint8_t r = leds[i].R * brilho;
        uint8_t g = leds[i].G * brilho;
        uint8_t b = leds[i].B * brilho;
        pio_sm_put_blocking(np_pio, sm, g);
        pio_sm_put_blocking(np_pio, sm, r);
        pio_sm_put_blocking(np_pio, sm, b);
    }
}

/**
 * @brief Descrição da função npSetLED.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param index Descrição do parâmetro index.
 * @param r Descrição do parâmetro r.
 * @param g Descrição do parâmetro g.
 * @param b Descrição do parâmetro b.
 */

void npSetLED(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < LED_COUNT) {
        leds[index].R = r;
        leds[index].G = g;
        leds[index].B = b;
    }
}

/**
 * @brief Descrição da função npSetAll.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param r Descrição do parâmetro r.
 * @param g Descrição do parâmetro g.
 * @param b Descrição do parâmetro b.
 */

void npSetAll(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < LED_COUNT; ++i) {
        npSetLED(i, r, g, b);
    }
}

/**
 * @brief Descrição da função npClear.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 */

void npClear(void) {
    npSetAll(0, 0, 0);
}

/**
 * @brief Descrição da função liberar_maquina_pio.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param pio Descrição do parâmetro pio.
 * @param sm_id Descrição do parâmetro sm_id.
 */

void liberar_maquina_pio(PIO pio, uint sm_id) {
    if (sm_id < 4) {
        pio_sm_set_enabled(pio, sm_id, false);
        pio_sm_unclaim(pio, sm_id);
    }
}

/**
 * @brief Descrição da função getLEDIndex.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param x Descrição do parâmetro x.
 * @param y Descrição do parâmetro y.
 * @return Valor de retorno descrevendo o significado.
 */

uint getLEDIndex(uint x, uint y) {
    if (x >= NUM_COLUNAS || y >= NUM_LINHAS) return 0;
    uint linha_fisica = NUM_LINHAS - 1 - y;
    uint base = linha_fisica * NUM_COLUNAS;
    return (linha_fisica % 2 == 0) ? base + (NUM_COLUNAS - 1 - x) : base + x;
}