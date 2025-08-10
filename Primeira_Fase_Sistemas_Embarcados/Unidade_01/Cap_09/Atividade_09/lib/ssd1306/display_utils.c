/**
 * @file display_utils.c
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/ssd1306/display_utils.c`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#include <stdio.h>
#include <stdint.h>
#include "display_utils.h"
#include "big_string_drawer.h"

/**
 * @brief Descrição da função mostrar_valor_grande.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param ssd Descrição do parâmetro ssd.
 * @param valor Descrição do parâmetro valor.
 * @param y Descrição do parâmetro y.
 */

void mostrar_valor_grande(uint8_t *ssd, float valor, int y) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%+.1foC", valor);
    draw_big_string_aligned_right(ssd, y, buffer);
}