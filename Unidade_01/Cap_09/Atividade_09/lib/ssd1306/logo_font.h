/**
 * @file logo_font.h
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/ssd1306/logo_font.h`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#ifndef LOGO_FONT_H
#define LOGO_FONT_H

static const uint8_t logo_digits[][8] = {
  {0x3E, 0x41, 0x41, 0x41, 0x41, 0x41, 0x3E, 0x00}, // 0
  {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00}, // 1
  {0x62, 0x51, 0x49, 0x49, 0x49, 0x49, 0x46, 0x00}, // 2
  {0x22, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00}, // 3
  {0x18, 0x14, 0x12, 0x7F, 0x10, 0x10, 0x10, 0x00}, // 4
  {0x4F, 0x49, 0x49, 0x49, 0x49, 0x49, 0x31, 0x00}, // 5
  {0x3E, 0x49, 0x49, 0x49, 0x49, 0x49, 0x32, 0x00}, // 6
  {0x01, 0x01, 0x01, 0x7F, 0x09, 0x05, 0x03, 0x00}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
  {0x26, 0x49, 0x49, 0x49, 0x49, 0x49, 0x3E, 0x00}, // 9
};

static const uint8_t logo_char_plus[8] = {0x08, 0x08, 0x08, 0x7F, 0x08, 0x08, 0x08, 0x00};
static const uint8_t logo_char_dot[8]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00};
static const uint8_t logo_char_degree[8] = {0x06, 0x09, 0x09, 0x06, 0x00, 0x00, 0x00, 0x00};
static const uint8_t logo_char_C[8] = {0x3E, 0x41, 0x40, 0x40, 0x40, 0x41, 0x22, 0x00};

#endif