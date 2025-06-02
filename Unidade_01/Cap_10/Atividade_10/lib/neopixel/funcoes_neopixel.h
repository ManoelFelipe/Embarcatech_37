/**
 * @file funcoes_neopixel.h
 * @brief Arquivo de cabeçalho para as funções de controle de LEDs WS2812B (NeoPixel).
 *
 * Este arquivo define a estrutura de dados para um LED NeoPixel, constantes
 * relacionadas à fita de LED (como contagem de LEDs e pino de controle),
 * e protótipos das funções para interagir com a fita.
 * Também declara variáveis globais externas usadas pela implementação PIO.
 * 
 * A nova rotina de reset no botão do joystick dentro de funcao_atividade_.c:
 * Joystick (id 2): chama reiniciar_sistema(), que
 * apaga toda a matriz NeoPixel (npClear + npWrite);
 * zera index_neo, fila (inicio, fim, quantidade) e contador;
 * atualiza os LEDs externos (Azul ligado indicando fila vazia);
 * registra a ação no console.
 * Toda a lógica anterior dos botões A e B foi preservada.
 *
 * @version 0.1
 * @author  Manoel Furtado
 * @date    01 junho 2025
 * @copyright Modificado por Manoel Furtado (MIT License) (veja LICENSE.md)
 */

#ifndef FUNCOES_NEOPIXEL_H // Diretiva de pré-processador para evitar inclusão múltipla
#define FUNCOES_NEOPIXEL_H

#include <stdint.h>      // Para tipos inteiros de tamanho fixo como uint8_t, uint
#include "hardware/pio.h" // Para tipos e funções relacionadas ao PIO (Programmable I/O)
#include <time.h>        // Para time() usado em inicializar_aleatorio()
#include <stdlib.h>      // Para rand() e srand()
#include "hardware/adc.h" // Incluído, mas não usado diretamente neste header. Pode ser para futuras expansões ou dependência indireta.
#include "pico/types.h"   // Para tipos básicos do Pico SDK como absolute_time_t (embora não usado diretamente aqui)

/** @brief Número total de LEDs NeoPixel na fita/matriz. */
#define LED_COUNT 25
/** @brief Pino GPIO do Raspberry Pi Pico conectado ao pino de dados (DIN) da fita NeoPixel. */
#define LED_PIN 7
/** @brief Número de colunas na matriz de LEDs (se aplicável, para funções como acender_coluna). */
#define NUM_COLUNAS 5
/** @brief Número de linhas na matriz de LEDs (se aplicável, para funções como acender_coluna). */
#define NUM_LINHAS 5

/**
 * @brief Estrutura para representar a cor de um LED NeoPixel.
 * Os componentes são armazenados na ordem G, R, B, pois é a ordem
 * esperada por muitos controladores WS2812B e pelo programa PIO ws2818b.pio.
 */
typedef struct {
    uint8_t G; /**< Componente Verde (0-255). */
    uint8_t R; /**< Componente Vermelho (0-255). */
    uint8_t B; /**< Componente Azul (0-255). */
} npLED_t;

// ======= VARIÁVEIS GLOBAIS EXTERNAS =======

/**
 * @brief Array externo que pode definir uma ordem de mapeamento físico dos LEDs.
 * Definido em `funcoes_neopixel.c`.
 */
extern uint ordem[]; //
/**
 * @brief Array externo que armazena o estado de cor de cada LED NeoPixel.
 * Definido em `funcoes_neopixel.c`.
 */
extern npLED_t leds[]; //

/**
 * @brief Instância PIO externa usada para controlar os LEDs.
 * Definido em `funcoes_neopixel.c`.
 */
extern PIO np_pio; //
/**
 * @brief Máquina de estados (SM) PIO externa usada para controlar os LEDs.
 * Definido em `funcoes_neopixel.c`.
 */
extern uint sm; //

/**
 * @brief Índice externo volátil do próximo LED NeoPixel a ser manipulado.
 * Definido em `Atividade_5.c` e usado também em `funcao_atividade_.c`.
 * É declarado aqui para que `funcoes_neopixel.c` (se precisasse) pudesse ter conhecimento dele,
 * embora a lógica principal de `index_neo` esteja fora de `funcoes_neopixel.c`.
 * Em geral, é melhor passar tais variáveis como parâmetros para manter o acoplamento baixo.
 */
extern volatile uint index_neo;

// ======= PROTÓTIPOS DAS FUNÇÕES =======

/**
 * @brief Inicializa o hardware PIO para controlar a fita NeoPixel.
 * @param pin O pino GPIO conectado à fita NeoPixel.
 */
void npInit(uint pin); //

/**
 * @brief Define a cor de um LED NeoPixel específico no buffer local.
 * @param index Índice do LED (0 a LED_COUNT-1).
 * @param r Componente Vermelho (0-255).
 * @param g Componente Verde (0-255).
 * @param b Componente Azul (0-255).
 */
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b); //

/**
 * @brief Define a cor de todos os LEDs NeoPixel no buffer local para a mesma cor.
 * @param r Componente Vermelho (0-255).
 * @param g Componente Verde (0-255).
 * @param b Componente Azul (0-255).
 */
void npSetAll(uint8_t r, uint8_t g, uint8_t b); //

/**
 * @brief Acende uma fileira (linha) de LEDs em uma matriz com uma cor específica.
 * @param linha Índice da linha.
 * @param r Componente Vermelho.
 * @param g Componente Verde.
 * @param b Componente Azul.
 * @param colunas Número de colunas na matriz.
 */
void acenderFileira(uint linha, uint8_t r, uint8_t g, uint8_t b, uint colunas); //

/**
 * @brief Acende uma coluna de LEDs em uma matriz com uma cor específica e atualiza a fita.
 * @param coluna Índice da coluna.
 * @param r Componente Vermelho.
 * @param g Componente Verde.
 * @param b Componente Azul.
 */
void acender_coluna(uint8_t coluna, uint8_t r, uint8_t g, uint8_t b); //

/**
 * @brief Apaga todos os LEDs NeoPixel (define a cor para preto no buffer).
 */
void npClear(); //

/**
 * @brief Envia os dados de cor do buffer local para a fita NeoPixel.
 */
void npWrite(); //

/**
 * @brief Define a cor de um LED NeoPixel específico e atualiza a fita imediatamente.
 * @param index Índice do LED.
 * @param r Componente Vermelho.
 * @param g Componente Verde.
 * @param b Componente Azul.
 */
void npAcendeLED(uint index, uint8_t r, uint8_t g, uint8_t b); //

/**
 * @brief Inicializa o gerador de números aleatórios.
 */
void inicializar_aleatorio(); //

/**
 * @brief Gera um número aleatório dentro de um intervalo especificado (inclusivo).
 * @param min Valor mínimo do intervalo.
 * @param max Valor máximo do intervalo.
 * @return int Número aleatório gerado.
 */
int numero_aleatorio(int min, int max); //

#endif // FUNCOES_NEOPIXEL_H