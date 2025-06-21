/**
 * @file efeitos.c
 * @brief Implementação de vários efeitos visuais para a matriz NeoPixel.
 *
 * Este arquivo contém a lógica para gerar diversas animações na matriz de LEDs,
 * aproveitando as funções do driver NeoPixel.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#include "libs/LabNeoPixel/neopixel_driver.h"
#include "libs/LabNeoPixel/efeitos.h"
#include "pico/stdlib.h"
#include "testes_cores.h"
#include <stdlib.h>

/**
 * @brief Acende todos os LEDs de uma única fileira (linha) com uma cor específica.
 * @param y Índice da fileira (0 a NUM_LINHAS-1).
 * @param r Componente Vermelho da cor.
 * @param g Componente Verde da cor.
 * @param b Componente Azul da cor.
 */
void acenderFileira(uint8_t y, uint8_t r, uint8_t g, uint8_t b) {
    for (uint x = 0; x < NUM_COLUNAS; x++) {
        uint index = getLEDIndex(x, y);
        npSetLED(index, r, g, b);
    }
    npWrite();
}

/**
 * @brief Acende todos os LEDs de uma única coluna com uma cor específica.
 * @param x Índice da coluna (0 a NUM_COLUNAS-1).
 * @param r Componente Vermelho da cor.
 * @param g Componente Verde da cor.
 * @param b Componente Azul da cor.
 */
void acenderColuna(uint8_t x, uint8_t r, uint8_t g, uint8_t b) {
    for (uint y = 0; y < NUM_LINHAS; y++) {
        uint index = getLEDIndex(x, y);
        npSetLED(index, r, g, b);
    }
    npWrite();
}

/**
 * @brief Animação de preenchimento em espiral, começando de fora e indo para o centro.
 *
 * Utiliza um array pré-definido com as coordenadas (x, y) dos 25 LEDs na ordem
 * de uma espiral. A função percorre esse array, acendendo um LED de cada vez.
 *
 * @param r Componente Vermelho da cor.
 * @param g Componente Verde da cor.
 * @param b Componente Azul da cor.
 * @param delay_ms Atraso em milissegundos entre acender cada LED, controlando a velocidade.
 */
void efeitoEspiral(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    // Array com as coordenadas (x, y) na ordem da espiral de fora para dentro.
    const uint8_t ordem_espiral[25][2] = {
        {0,0},{1,0},{2,0},{3,0},{4,0},{4,1},{4,2},{4,3},{4,4},
        {3,4},{2,4},{1,4},{0,4},{0,3},{0,2},{0,1},{1,1},{2,1},
        {3,1},{3,2},{3,3},{2,3},{1,3},{1,2},{2,2}
    };

    npClear();
    for (uint i = 0; i < 25; ++i) {
        uint x = ordem_espiral[i][0];
        uint y = ordem_espiral[i][1];
        uint index = getLEDIndex(x, y);
        npSetLED(index, r, g, b);
        npWrite();
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Animação de uma onda de luz vertical com brilho suave.
 *
 * Cria uma "barra" de luz vertical que se move de cima para baixo. O brilho de cada
 * linha é calculado com base na sua distância ao centro da onda ("fase"), criando
 * um efeito de gradiente suave.
 *
 * @param r Componente Vermelho da cor base.
 * @param g Componente Verde da cor base.
 * @param b Componente Azul da cor base.
 * @param delay_ms Atraso entre cada passo da onda.
 */
void efeitoOndaVertical(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    // O loop de 'fase' move o centro da onda.
    for (int fase = 0; fase < NUM_LINHAS + 3; ++fase) {
        npClear();
        for (int y = 0; y < NUM_LINHAS; ++y) {
            // Calcula a intensidade da luz para a linha 'y' atual.
            // A intensidade é máxima quando 'y' é igual a 'fase' e diminui linearmente com a distância.
            float intensidade = 1.0f - 0.25f * abs(fase - y);
            if (intensidade < 0) intensidade = 0; // Garante que a intensidade não seja negativa.

            for (int x = 0; x < NUM_COLUNAS; ++x) {
                uint index = getLEDIndex(x, y);
                npSetLED(index, r * intensidade, g * intensidade, b * intensidade);
            }
        }
        npWrite();
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Animação de preenchimento em espiral inversa, do centro para fora.
 *
 * Funciona como o `efeitoEspiral`, mas utiliza um array com as coordenadas na
 * ordem inversa, começando pelo LED central.
 *
 * @param r Componente Vermelho da cor.
 * @param g Componente Verde da cor.
 * @param b Componente Azul da cor.
 * @param delay_ms Atraso entre acender cada LED.
 */
void efeitoEspiralInversa(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    // Array com as coordenadas (x, y) na ordem da espiral de dentro para fora.
    const uint8_t ordem_espiral[25][2] = {
        {2,2},{1,2},{1,3},{2,3},{3,3},{3,2},{3,1},{2,1},{1,1},
        {0,1},{0,2},{0,3},{0,4},{1,4},{2,4},{3,4},{4,4},{4,3},
        {4,2},{4,1},{4,0},{3,0},{2,0},{1,0},{0,0}
    };

    npClear();
    for (uint i = 0; i < 25; ++i) {
        uint x = ordem_espiral[i][0];
        uint y = ordem_espiral[i][1];
        uint index = getLEDIndex(x, y);
        npSetLED(index, r, g, b);
        npWrite();
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Efeito de preenchimento vertical (de cima para baixo) com gradiente de brilho.
 *
 * Preenche a matriz linha por linha. O brilho de cada linha acesa é proporcional
 * à sua posição, criando um efeito de gradiente.
 *
 * @param r, g, b Componentes da cor base.
 * @param delay_ms Atraso entre o preenchimento de cada linha.
 */
void efeitoOndaVerticalBrilho(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    for (uint8_t passo = 0; passo < NUM_LINHAS; ++passo) {
        npClear();
        for (uint8_t y = 0; y <= passo; ++y) {
            float brilho = ((float)(y + 1)) / NUM_LINHAS;
            for (uint8_t x = 0; x < NUM_COLUNAS; ++x) {
                uint index = getLEDIndex(x, y);
                npSetLED(index, r * brilho, g * brilho, b * brilho);
            }
        }
        npWrite();
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Animação de varredura de fileiras de cima para baixo com brilho progressivo.
 *
 * Acende uma fileira de cada vez, de y=0 a y=4. O brilho da fileira acesa aumenta
 * a cada passo.
 *
 * @param r, g, b Componentes da cor base.
 * @param delay_ms Atraso entre cada fileira.
 */
void efeitoFileirasColoridas(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    for (uint8_t y = 0; y < NUM_LINHAS; ++y) {
        npClear();
        float brilho = ((float)(y + 1)) / NUM_LINHAS; // Brilho de 0.2 a 1.0
        acenderFileira(y, r * brilho, g * brilho, b * brilho);
        // npWrite() é chamado dentro de acenderFileira
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Animação de varredura de fileiras de baixo para cima com brilho progressivo.
 *
 * O mesmo que `efeitoFileirasColoridas`, mas na direção oposta (y=4 a y=0).
 *
 * @param r, g, b Componentes da cor base.
 * @param delay_ms Atraso entre cada fileira.
 */
void efeitoFileirasColoridasReverso(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    for (int8_t y = NUM_LINHAS - 1; y >= 0; --y) {
        npClear();
        float brilho = ((float)(NUM_LINHAS - y)) / NUM_LINHAS;
        acenderFileira(y, r * brilho, g * brilho, b * brilho);
        // npWrite() é chamado dentro de acenderFileira
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Animação de varredura de colunas da esquerda para a direita com brilho progressivo.
 *
 * Acende uma coluna de cada vez, de x=0 a x=4. O brilho da coluna acesa aumenta
 * a cada passo.
 *
 * @param r, g, b Componentes da cor base.
 * @param delay_ms Atraso entre cada coluna.
 */
void efeitoColunasColoridas(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    for (uint8_t x = 0; x < NUM_COLUNAS; ++x) {
        npClear();
        float brilho = ((float)(x + 1)) / NUM_COLUNAS;
        acenderColuna(x, r * brilho, g * brilho, b * brilho);
        // npWrite() é chamado dentro de acenderColuna
        sleep_ms(delay_ms);
    }
}

/**
 * @brief Animação de varredura de colunas da direita para a esquerda com brilho progressivo.
 *
 * O mesmo que `efeitoColunasColoridas`, mas na direção oposta (x=4 a x=0).
 *
 * @param r, g, b Componentes da cor base.
 * @param delay_ms Atraso entre cada coluna.
 */
void efeitoColunasColoridasReverso(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    for (int8_t x = NUM_COLUNAS - 1; x >= 0; --x) {
        npClear();
        float brilho = ((float)(NUM_COLUNAS - x)) / NUM_COLUNAS;
        acenderColuna(x, r * brilho, g * brilho, b * brilho);
        // npWrite() é chamado dentro de acenderColuna
        sleep_ms(delay_ms);
    }
}