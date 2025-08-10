/**
 * @file Atividade_5.c
 * @brief Arquivo principal da aplicação para controle de LEDs NeoPixel e LEDs RGB externos
 * utilizando botões e comunicação inter-core no Raspberry Pi Pico.
 *
 * Este arquivo inicializa o sistema, configura os pinos de GPIO para LEDs e botões,
 * inicializa a fita de LEDs NeoPixel, e gerencia a comunicação entre os dois núcleos
 * do microcontrolador. O núcleo 0 é responsável por detectar interrupções dos botões,
 * enquanto o núcleo 1 trata a lógica de acendimento dos LEDs e a interação com a fila.
 *
 * @version 0.1
 * @author  Manoel Furtado
 * @date    01 junho 2025
 * @copyright Modificado por Manoel Furtado (MIT License) (veja LICENSE.md)
 */

#include "funcao_atividade_.h"
#include "funcoes_neopixel.h"

/**
 * @brief Estrutura para representar uma cor RGB.
 */
typedef struct {
    uint8_t r; /**< Componente vermelho da cor (0-255). */
    uint8_t g; /**< Componente verde da cor (0-255). */
    uint8_t b; /**< Componente azul da cor (0-255). */
} CorRGB;

/**
 * @brief Array de cores RGB pré-definidas.
 * Este array contém uma paleta de 16 cores que podem ser utilizadas na aplicação.
 */
CorRGB cores[] = {
    {255, 0, 0},    /**< Vermelho */
    {255, 64, 0},   /**< Laranja avermelhado */
    {255, 128, 0},  /**< Laranja */
    {255, 192, 0},  /**< Laranja amarelado */
    {255, 255, 0},  /**< Amarelo */
    {192, 255, 0},  /**< Verde amarelado */
    {128, 255, 0},  /**< Verde limão */
    {0, 255, 0},    /**< Verde */
    {0, 255, 128},  /**< Verde ciano */
    {0, 255, 255},  /**< Ciano */
    {0, 128, 255},  /**< Azul ciano */
    {0, 0, 255},    /**< Azul */
    {128, 0, 255},  /**< Roxo */
    {255, 0, 255},  /**< Magenta */
    {255, 0, 128},  /**< Rosa */
    {255, 255, 255} /**< Branco */
};

/**
 * @brief Constante que armazena o número total de cores pré-definidas no array `cores`.
 * Calculado dividindo o tamanho total do array `cores` pelo tamanho de um elemento `CorRGB`.
 */
const size_t TOTAL_CORES = sizeof(cores) / sizeof(CorRGB);

/**
 * @brief Variável volátil que armazena o índice do próximo LED NeoPixel a ser aceso ou apagado.
 * Esta variável é marcada como `volatile` porque pode ser modificada por rotinas de interrupção
 * ou por diferentes núcleos, e o compilador não deve otimizar o acesso a ela.
 */
volatile uint index_neo = 0; //

/**
 * @brief Função principal da aplicação.
 *
 * Esta função é o ponto de entrada do programa. Ela realiza as seguintes etapas:
 * 1. Inicializa a fita de LEDs NeoPixel.
 * 2. Apaga todos os LEDs NeoPixel e atualiza a fita.
 * 3. Inicializa todas as funções de entrada/saída padrão (stdio).
 * 4. Inicializa os pinos dos LEDs RGB externos como saída e os desliga.
 * 5. Inicializa os pinos dos botões como entrada com pull-up.
 * 6. Inicia o núcleo 1 (core1) para executar a função `tratar_eventos_leds`.
 * 7. Aguarda o núcleo 1 sinalizar que está pronto.
 * 8. Configura a função de callback para interrupções de GPIO no núcleo 0.
 * 9. Habilita as interrupções de GPIO para os botões (detecção de borda de descida).
 * 10. Coloca o núcleo 0 em modo de espera de baixa energia (`__wfi`), aguardando interrupções.
 *
 * @return int Retorna 0 se a execução for bem-sucedida (embora, em sistemas embarcados, main geralmente não retorna).
 */
int main() {
    // Inicializa a fita NeoPixel no pino especificado por LED_PIN
    npInit(LED_PIN); //
    // Apaga todos os LEDs da fita NeoPixel
    npClear(); //
    // Envia os dados para a fita NeoPixel para efetivar o comando de apagar
    npWrite(); //

    // Inicializa todas as funções de I/O padrão (necessário para printf, etc.)
    stdio_init_all(); //

    // Inicializa os pinos dos LEDs RGB externos
    for (int i = 0; i < NUM_BOTOES; i++) {
        // Configura o pino do LED como saída, sem pull-up/pull-down
        inicializar_pino(LEDS[i], GPIO_OUT, false, false); //
        // Desliga o LED
        gpio_put(LEDS[i], 0); //
        // Define o estado inicial do LED como desligado
        estado_leds[i] = false; //
    }

    // Inicializa os pinos dos botões
    for (int i = 0; i < NUM_BOTOES; i++) {
        // Configura o pino do botão como entrada, com resistor de pull-up habilitado
        inicializar_pino(BOTOES[i], GPIO_IN, true, false); //
    }

    // Inicia a execução da função tratar_eventos_leds no núcleo 1 (core1)
    multicore_launch_core1(tratar_eventos_leds); //

    // Aguarda até que o núcleo 1 sinalize que está pronto através da variável core1_pronto
    while (!core1_pronto); //

    // Configura a função de callback a ser chamada quando ocorrer uma interrupção de GPIO
    gpio_set_irq_callback(gpio_callback); //
    // Habilita as interrupções de GPIO no nível do processador (IRQ_BANK0)
    irq_set_enabled(IO_IRQ_BANK0, true); //

    // Habilita interrupções para cada botão individualmente
    for (int i = 0; i < NUM_BOTOES; i++) {
        // Configura a interrupção para o botão BOTOES[i] para ser acionada na borda de descida (quando o botão é pressionado)
        gpio_set_irq_enabled(BOTOES[i], GPIO_IRQ_EDGE_FALL, true); //
    }

    // Núcleo 0 entra em um loop infinito de espera de baixa energia
    while (true) {
        // __wfi (Wait For Interrupt) coloca o processador em modo de baixo consumo
        // até que uma interrupção ocorra. As interrupções dos botões "acordarão" o núcleo.
        __wfi(); //
    }
    // Em teoria, o programa nunca chega aqui em um sistema embarcado.
    return 0;
}