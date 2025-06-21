/**
 * @file neopixel_driver.h
 * @brief Arquivo de cabeçalho para o driver da matriz de LEDs NeoPixel (WS2812B) usando PIO do RP2040.
 *
 * Define a interface pública para controlar a matriz de LEDs. Inclui constantes
 * como o número de LEDs e o pino de controle, a estrutura de dados para um único
 * LED, variáveis globais externas e os protótipos de todas as funções essenciais
 * para inicializar o driver, definir cores e atualizar a matriz.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#ifndef NEOPIXEL_DRIVER_H
#define NEOPIXEL_DRIVER_H

#include <stdint.h>
#include "hardware/pio.h"

/**
 * @name Constantes de Configuração da Matriz
 * @brief Define os parâmetros físicos e de controle da matriz de LEDs.
 * @{
 */
#define LED_COUNT   25      ///< Número total de LEDs na matriz (5x5).
#define LED_PIN     7       ///< Pino GPIO do RP2040 conectado à entrada de dados (DIN) da matriz.
#define NUM_COLUNAS 5       ///< Número de colunas da matriz.
#define NUM_LINHAS  5       ///< Número de linhas da matriz.
/** @} */

/**
 * @name Níveis de Brilho Pré-definidos
 * @brief Macros para valores de brilho comuns (0-255).
 * @{
 */
#define COR_APAGA   0       ///< Nível de brilho 0 (LED apagado).
#define COR_MIN     64      ///< Nível de brilho baixo.
#define COR_INTER   128     ///< Nível de brilho intermediário.
#define COR_ALTA    192     ///< Nível de brilho alto.
#define COR_MAX     255     ///< Nível de brilho máximo.
/** @} */

/**
 * @struct npLED_t
 * @brief Estrutura de dados para representar a cor de um único LED NeoPixel.
 *
 * Armazena os componentes de cor de 8 bits para um LED.
 * @note A ordem dos membros (G, R, B) é intencional e corresponde à ordem
 * em que o controlador WS2812B espera receber os dados de cor (Verde, depois Vermelho, depois Azul).
 */
typedef struct {
    uint8_t G, R, B;
} npLED_t;

/**
 * @name Variáveis Globais do Driver
 * @brief Variáveis que mantêm o estado do driver NeoPixel.
 *
 * `leds`: Atua como um "frame buffer", armazenando o estado de cor de cada LED
 * antes de serem enviados para a matriz física.
 * `np_pio` e `sm`: Armazenam a instância do PIO e o índice da Máquina de Estado
 * utilizados para controlar os LEDs.
 * @{
 */
extern npLED_t leds[LED_COUNT]; ///< Buffer que armazena o estado de cor de todos os LEDs.
extern PIO np_pio;              ///< Ponteiro para a instância do hardware PIO utilizada (pio0 ou pio1).
extern int sm;                  ///< Índice da Máquina de Estado (State Machine) do PIO utilizada.
/** @} */

/**
 * @brief Inicializa o driver NeoPixel e a Máquina de Estado do PIO.
 * @param pin O número do pino GPIO ao qual a matriz está conectada.
 */
void npInit(uint pin);

/**
 * @brief Envia os dados do buffer `leds` para a matriz de LEDs.
 *
 * Esta função efetivamente atualiza a matriz física com as cores
 * que foram definidas no buffer de software.
 */
void npWrite(void);

/**
 * @brief Envia os dados do buffer `leds` aplicando um fator de brilho global.
 * @param brilho Fator de brilho a ser aplicado (0.0f a 1.0f).
 */
void npWriteComBrilho(float brilho);

/**
 * @brief Define a cor de um único LED no buffer.
 * @param index O índice do LED (0 a LED_COUNT-1) a ser alterado.
 * @param r Componente de cor Vermelha (0-255).
 * @param g Componente de cor Verde (0-255).
 * @param b Componente de cor Azul (0-255).
 */
void npSetLED(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Define a cor de todos os LEDs no buffer para o mesmo valor.
 * @param r Componente de cor Vermelha (0-255).
 * @param g Componente de cor Verde (0-255).
 * @param b Componente de cor Azul (0-255).
 */
void npSetAll(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Apaga todos os LEDs, definindo suas cores como (0, 0, 0) no buffer.
 */
void npClear(void);

/**
 * @brief Desabilita e libera a Máquina de Estado do PIO.
 * @param pio A instância do PIO (pio0 ou pio1).
 * @param sm O índice da Máquina de Estado a ser liberada.
 */
void liberar_maquina_pio(PIO pio, uint sm);

/**
 * @brief Converte coordenadas (x, y) da matriz para um índice de LED 1D.
 *
 * Leva em conta a montagem em "serpentina" (zigzag) da matriz 5x5.
 * @param x Coordenada da coluna (0 a NUM_COLUNAS-1).
 * @param y Coordenada da linha (0 a NUM_LINHAS-1), com (0,0) no canto superior esquerdo.
 * @return uint O índice linear do LED correspondente (0 a LED_COUNT-1).
 */
uint getLEDIndex(uint x, uint y);

#endif // NEOPIXEL_DRIVER_H