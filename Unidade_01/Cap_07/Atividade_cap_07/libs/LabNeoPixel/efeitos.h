/**
 * @file efeitos.h
 * @brief Arquivo de cabeçalho para as funções de efeitos visuais na matriz NeoPixel.
 *
 * Declara as funções que implementam diversas animações, como espirais,
 * ondas e varreduras de linhas/colunas, tornando a criação de padrões visuais
 * mais modular e reutilizável.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#ifndef EFEITOS_H
#define EFEITOS_H

#include <stdint.h>
// #include "LabNeoPixel/efeitos.h" // Esta linha parece ser uma inclusão circular/redundante e pode ser removida.

/** @brief Acende todos os LEDs de uma fileira (linha) específica. */
void acenderFileira(uint8_t y, uint8_t r, uint8_t g, uint8_t b);

/** @brief Acende todos os LEDs de uma coluna específica. */
void acenderColuna(uint8_t x, uint8_t r, uint8_t g, uint8_t b);

/** @brief Cria um efeito de preenchimento em espiral, de fora para dentro. */
void efeitoEspiral(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

/** @brief Cria um efeito de onda vertical suave que percorre a matriz. */
void efeitoOndaVertical(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

/** @brief Cria um efeito de preenchimento em espiral reverso, de dentro para fora. */
void efeitoEspiralInversa(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

/** @brief Cria um efeito de onda vertical com brilho progressivo. */
void efeitoOndaVerticalBrilho(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

/** @brief Acende as fileiras de cima para baixo com brilho crescente. */
void efeitoFileirasColoridas(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

/** @brief Acende as fileiras de baixo para cima com brilho crescente. */
void efeitoFileirasColoridasReverso(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

/** @brief Acende as colunas da esquerda para a direita com brilho crescente. */
void efeitoColunasColoridas(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

/** @brief Acende as colunas da direita para a esquerda com brilho crescente. */
void efeitoColunasColoridasReverso(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

#endif // EFEITOS_H