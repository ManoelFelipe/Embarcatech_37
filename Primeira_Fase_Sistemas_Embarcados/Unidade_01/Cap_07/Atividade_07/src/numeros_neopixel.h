/**
 * @file numeros_neopixel.h
 * @brief Arquivo de cabeçalho para as funções de exibição de números na matriz NeoPixel.
 *
 * Este arquivo define os protótipos das funções responsáveis por desenhar os
 * números de 1 a 6 na matriz de LEDs. Ele também define macros para as cores
 * (RGB) associadas a cada número, facilitando a customização e a leitura do código.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#ifndef NUMEROS_NEOPIXEL_H
#define NUMEROS_NEOPIXEL_H

#include <stdint.h>

/**
 * @name Macros de Cores
 * @brief Define os componentes (R, G, B) das cores para cada número.
 *
 * Usar macros para as cores torna o código mais legível e fácil de manter.
 * Se for necessário alterar a cor de um número, a mudança é feita em um único lugar.
 * Os valores de brilho (0-255) foram escolhidos para serem visíveis, mas não excessivamente fortes.
 * @{
 */
#define COR_1_R 0   ///< Componente Vermelho para o número 1
#define COR_1_G 80  ///< Componente Verde para o número 1
#define COR_1_B 80  ///< Componente Azul para o número 1

#define COR_2_R 0   ///< Componente Vermelho para o número 2
#define COR_2_G 80  ///< Componente Verde para o número 2
#define COR_2_B 0   ///< Componente Azul para o número 2

#define COR_3_R 80  ///< Componente Vermelho para o número 3
#define COR_3_G 80  ///< Componente Verde para o número 3
#define COR_3_B 0   ///< Componente Azul para o número 3

#define COR_4_R 0   ///< Componente Vermelho para o número 4
#define COR_4_G 0   ///< Componente Verde para o número 4
#define COR_4_B 80  ///< Componente Azul para o número 4

#define COR_5_R 80  ///< Componente Vermelho para o número 5
#define COR_5_G 0   ///< Componente Verde para o número 5
#define COR_5_B 0   ///< Componente Azul para o número 5

#define COR_6_R 80  ///< Componente Vermelho para o número 6
#define COR_6_G 0   ///< Componente Verde para o número 6
#define COR_6_B 80  ///< Componente Azul para o número 6
/** @} */

/**
 * @name Protótipos das Funções de Desenho
 * @brief Funções que desenham cada número de 1 a 6 na matriz de LEDs.
 * @{
 */

/** @brief Desenha o número 1 na matriz de LEDs. */
void mostrar_numero_1();

/** @brief Desenha o número 2 na matriz de LEDs. */
void mostrar_numero_2();

/** @brief Desenha o número 3 na matriz de LEDs. */
void mostrar_numero_3();

/** @brief Desenha o número 4 na matriz de LEDs. */
void mostrar_numero_4();

/** @brief Desenha o número 5 na matriz de LEDs. */
void mostrar_numero_5();

/** @brief Desenha o número 6 na matriz de LEDs. */
void mostrar_numero_6();

/** @} */

#endif // NUMEROS_NEOPIXEL_H