/**
 * @file funcao_atividade_.h
 * @brief Arquivo de cabeçalho para as funções da atividade, incluindo definições e protótipos.
 *
 * Este arquivo define constantes, pinos de GPIO, tamanhos de buffers e protótipos
 * de funções utilizadas na lógica principal da aplicação, como callbacks de interrupção,
 * inicialização de pinos e tratamento de eventos de LED. Também declara variáveis
 * globais externas que são compartilhadas entre diferentes arquivos de origem.
 *
 * @version 0.1
 * @author  Manoel Furtado
 * @date    01 junho 2025
 * @copyright Modificado por Manoel Furtado (MIT License) (veja LICENSE.md)
 */

#ifndef FUNCAO_ATIVIDADE_3_H // Diretiva de pré-processador para evitar inclusão múltipla
#define FUNCAO_ATIVIDADE_3_H

#include <stdio.h>          // Para funções de entrada/saída padrão (printf)
#include "pico/stdlib.h"     // Biblioteca padrão do SDK do Raspberry Pi Pico
#include "hardware/gpio.h"   // Para controle de pinos GPIO
#include "hardware/irq.h"    // Para manipulação de interrupções
#include "hardware/sync.h"   // Para funções de sincronização como __wfi() (Wait For Interrupt)
#include "hardware/adc.h"    // Biblioteca para manipulação do ADC (Conversor Analógico-Digital)
#include "pico/multicore.h"  // Para funcionalidades multicore (FIFO, lançamento de core1)


// ======= DEFINIÇÕES E CONSTANTES =======

/** @brief Tempo em milissegundos para o debouncing de botões. Evita múltiplas leituras de um único pressionamento. */
#define DEBOUNCE_MS 40

/** @brief Tempo de atraso genérico em milissegundos. Pode ser usado para pausas. */
#define DELAY_MS 500

/** @brief Pino GPIO conectado ao Botão A. */
#define BOTAO_A 5
/** @brief Pino GPIO conectado ao Botão B. */
#define BOTAO_B 6
/** @brief Pino GPIO conectado ao botão do Joystick. */
#define BOTAO_JOYSTICK 22

/** @brief Pino GPIO conectado ao LED Vermelho externo. */
#define LED_VERMELHO 13
/** @brief Pino GPIO conectado ao LED Azul externo. */
#define LED_AZUL 12
/** @brief Pino GPIO conectado ao LED Verde externo. */
#define LED_VERDE 11

/** @brief Número total de botões utilizados na aplicação. */
#define NUM_BOTOES 3

/** @brief Tamanho máximo da fila circular utilizada para armazenar dados. */
#define TAM_FILA 25 //

/** @brief Constante simbólica para representar a ação de inserir na fila (não utilizada diretamente no código atual, mas pode ser útil para futuras expansões). */
#define ACAO_INSERIR 1
/** @brief Constante simbólica para representar a ação de remover da fila (não utilizada diretamente no código atual). */
#define ACAO_REMOVER 2

// ======= PROTÓTIPOS DAS FUNÇÕES =======

/**
 * @brief Função de callback chamada quando ocorre uma interrupção de GPIO.
 * @param gpio O pino GPIO que gerou a interrupção.
 * @param events O tipo de evento de interrupção.
 */
void gpio_callback(uint gpio, uint32_t events); //

/**
 * @brief Inicializa um pino GPIO com uma direção e configurações de pull específicas.
 * @param pino O número do pino GPIO.
 * @param direcao Direção do pino (GPIO_IN ou GPIO_OUT).
 * @param pull_up true para habilitar pull-up, false caso contrário.
 * @param pull_down true para habilitar pull-down, false caso contrário.
 */
void inicializar_pino(uint pino, uint direcao, bool pull_up, bool pull_down); //

/**
 * @brief Função executada no núcleo 1 para tratar eventos relacionados aos LEDs.
 */
void tratar_eventos_leds(); //

/**
 * @brief Imprime o conteúdo atual da fila no console.
 */
void imprimir_fila(); //

// ======= VARIÁVEIS GLOBAIS EXTERNAS =======

/** @brief Array externo constante dos pinos dos botões. Definido em `funcao_atividade_.c`. */
extern const uint BOTOES[NUM_BOTOES]; //
/** @brief Array externo constante dos pinos dos LEDs. Definido em `funcao_atividade_.c`. */
extern const uint LEDS[NUM_BOTOES]; //
/** @brief Flag externa volátil que indica se o núcleo 1 está pronto. Definido em `funcao_atividade_.c`. */
extern volatile bool core1_pronto; //

/** @brief Array externo volátil para sinalizar eventos pendentes de botões. Definido em `funcao_atividade_.c`. */
extern volatile bool eventos_pendentes[NUM_BOTOES]; //
/** @brief Array externo volátil para armazenar o estado dos LEDs. Definido em `funcao_atividade_.c`. */
extern volatile bool estado_leds[NUM_BOTOES]; //

#endif // FUNCAO_ATIVIDADE_3_H