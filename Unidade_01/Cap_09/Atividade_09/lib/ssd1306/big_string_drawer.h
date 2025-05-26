/**
 * @file big_string_drawer.h
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/ssd1306/big_string_drawer.h`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#ifndef BIG_STRING_DRAWER_H
#define BIG_STRING_DRAWER_H

#include <stdint.h>

void draw_big_string_aligned_right(uint8_t *ssd, int y, const char *str);

#endif