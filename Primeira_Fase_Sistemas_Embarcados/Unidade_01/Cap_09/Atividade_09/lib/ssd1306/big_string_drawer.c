/**
 * @file big_string_drawer.c
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/ssd1306/big_string_drawer.c`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#include "font_big_logo.h"
#include "draw_big_char.h"
#include <stddef.h>

/**
 * @brief Descrição da função get_big_bitmap.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param c Descrição do parâmetro c.
 * @return Valor de retorno descrevendo o significado.
 */

const uint8_t* get_big_bitmap(char c) {
    switch (c) {
        case '0': return big_digit_0;
        case '1': return big_digit_1;
        case '2': return big_digit_2;
        case '3': return big_digit_3;
        case '4': return big_digit_4;
        case '5': return big_digit_5;
        case '6': return big_digit_6;
        case '7': return big_digit_7;
        case '8': return big_digit_8;
        case '9': return big_digit_9;
        case '+': return big_char_plus;
        case '-': return big_char_minus;
        case '.': return big_char_dot;
        case 'o': return big_char_degree;
        case 'C': return big_char_C;
        default: return NULL;
    }
}

/**
 * @brief Descrição da função get_char_width.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param c Descrição do parâmetro c.
 * @return Valor de retorno descrevendo o significado.
 */

int get_char_width(char c) {
    switch (c) {
        case '.':
        case 'o':
            return 8;
        default:
            return 16;
    }
}

/**
 * @brief Descrição da função calc_string_width.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param str Descrição do parâmetro str.
 * @return Valor de retorno descrevendo o significado.
 */

int calc_string_width(const char *str) {
    int width = 0;
    while (*str) {
        width += get_char_width(*str);
        str++;
    }
    return width;
}

/**
 * @brief Descrição da função draw_big_string_aligned_right.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param ssd Descrição do parâmetro ssd.
 * @param y Descrição do parâmetro y.
 * @param str Descrição do parâmetro str.
 */

void draw_big_string_aligned_right(uint8_t *ssd, int y, const char *str) {
    int width = calc_string_width(str);
    int x = 128 - width;
    while (*str) {
        const uint8_t *bitmap = get_big_bitmap(*str);
        if (bitmap) {
            draw_big_char(ssd, x, y, bitmap);
        }
        x += get_char_width(*str);
        str++;
    }
}