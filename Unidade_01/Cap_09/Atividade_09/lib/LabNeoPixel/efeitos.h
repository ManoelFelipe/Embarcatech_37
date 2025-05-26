/**
 * @file efeitos.h
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/LabNeoPixel/efeitos.h`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#ifndef EFEITOS_H
#define EFEITOS_H

#include <stdint.h>
#include "LabNeoPixel/efeitos.h"


void acenderFileira(uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void acenderColuna(uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void efeitoEspiral(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);
void efeitoOndaVertical(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);
void efeitoEspiralInversa(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);
void efeitoOndaVerticalBrilho(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);
void efeitoFileirasColoridas(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);
void efeitoFileirasColoridasReverso(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);
void efeitoColunasColoridas(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);
void efeitoColunasColoridasReverso(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

#endif