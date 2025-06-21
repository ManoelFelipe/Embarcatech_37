/**
 * @file efeito_curva_ar.h
 * @brief Arquivo de cabeçalho para o efeito de curva dinâmica na matriz NeoPixel.
 *
 * Define o protótipo da função `efeitoCurvaNeoPixel`, responsável por gerar
 * um efeito gráfico de uma curva que se move horizontalmente na matriz de LEDs.
 * O efeito é baseado em um modelo autorregressivo (AR).
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#ifndef EFEITO_CURVA_AR_H
#define EFEITO_CURVA_AR_H

#include <stdint.h>

/**
 * @brief Gera um quadro (frame) do efeito de curva na matriz de LEDs.
 *
 * Esta função desenha uma barra vertical cuja altura e posição são determinadas
 * por um modelo matemático (autorregressivo), e em seguida desloca toda a matriz
 * para a esquerda, criando uma animação de curva em movimento.
 *
 * @param r Componente Vermelho (0-255) da cor da curva.
 * @param g Componente Verde (0-255) da cor da curva.
 * @param b Componente Azul (0-255) da cor da curva.
 * @param delay_ms Tempo em milissegundos para aguardar após desenhar o quadro, controlando a velocidade da animação.
 */
void efeitoCurvaNeoPixel(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms);

#endif // EFEITO_CURVA_AR_H