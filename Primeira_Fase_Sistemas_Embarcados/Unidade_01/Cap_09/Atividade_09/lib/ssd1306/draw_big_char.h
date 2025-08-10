/**
 * @file draw_big_char.h
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/ssd1306/draw_big_char.h`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#ifndef DRAW_BIG_CHAR_H
#define DRAW_BIG_CHAR_H

#include <stdbool.h>
#include <stdint.h>
#include "ssd1306.h"

// Desenha um caractere grande no buffer ssd[] a partir de bitmap de 64 bytes
/**
 * @brief Descrição da função draw_big_char.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param ssd Descrição do parâmetro ssd.
 * @param x Descrição do parâmetro x.
 * @param y Descrição do parâmetro y.
 * @param bitmap Descrição do parâmetro bitmap.
 */

void draw_big_char(uint8_t *ssd, int x, int y, const uint8_t *bitmap) {
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 16; col++) {
            int byte_index = (row * 2) + (col / 8);
            bool pixel = (bitmap[byte_index] >> (7 - (col % 8))) & 0x01;
            ssd1306_set_pixel(ssd, x + col, y + row, pixel);
        }
    }
}
#endif