/**
 * @file neopixel_driver.c
 * @brief Implementação do driver NeoPixel (WS2812B) usando PIO.
 *
 * Este arquivo implementa as funções para controlar a matriz de LEDs. Ele utiliza
 * uma Máquina de Estado (State Machine - SM) do periférico de I/O Programável (PIO)
 * do RP2040 para gerar os sinais com a temporização precisa exigida pelo protocolo WS2812B.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#include "neopixel_driver.h"
#include "ws2818b.pio.h" // Arquivo gerado pelo pioasm com o programa da fita de LED.

/// @brief Buffer global que armazena o estado de cor de todos os LEDs.
npLED_t leds[LED_COUNT];
/// @brief Instância do hardware PIO utilizada.
PIO np_pio;
/// @brief Índice da Máquina de Estado (SM) utilizada.
int sm;

/**
 * @brief Inicializa o driver NeoPixel e a Máquina de Estado do PIO.
 *
 * Esta função realiza 3 passos cruciais:
 * 1. Carrega o programa PIO (`ws2818b_program`) na memória de instruções do PIO0.
 * 2. Reivindica (claim) uma máquina de estado (SM 0) para uso exclusivo deste driver.
 * 3. Inicializa a SM com as configurações definidas pelo programa PIO (frequência, pino de saída, etc.).
 * Por fim, limpa a matriz para garantir que todos os LEDs comecem apagados.
 *
 * @param pin O número do pino GPIO ao qual a matriz está conectada.
 */
void npInit(uint pin) {
    // Carrega o programa PIO na memória do PIO0 e obtém o deslocamento (offset) onde ele foi armazenado.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0; // Define qual hardware PIO será usado.
    sm = 0; // Define que usaremos a SM 0.
    pio_sm_claim(np_pio, sm); // Reivindica a SM para que não seja usada por outro código.

    // Inicializa a SM com a função de ajuda gerada pelo pioasm.
    // Esta função configura o clock, o mapeamento de pinos e outras configurações
    // para gerar o sinal de 800kHz necessário para o WS2812B.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
    // Garante que a matriz comece apagada.
    npClear();
    npWrite(); // Envia o estado limpo para a matriz
}

/**
 * @brief Envia os dados do buffer de software `leds` para a matriz de LEDs física.
 *
 * Itera sobre cada LED no buffer `leds` e envia seus componentes de cor
 * (Verde, Vermelho, Azul, nesta ordem) para a FIFO de transmissão (TX) da Máquina de Estado do PIO.
 * A SM então consome esses dados e os serializa no pino de saída com a temporização correta.
 */
void npWrite(void) {
    for (uint i = 0; i < LED_COUNT; ++i) {
        // Envia os 24 bits de cor (8 por componente) para a SM.
        // A função é bloqueante, esperando a FIFO ter espaço.
        // A ordem G, R, B é crucial para o protocolo WS2812B.
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
}

/**
 * @brief Envia os dados do buffer, mas aplicando um fator de brilho global.
 *
 * Similar a `npWrite`, mas antes de enviar os dados para a FIFO do PIO,
 * multiplica cada componente de cor (R, G, B) por um fator de brilho.
 * Útil para efeitos de fade ou para limitar o consumo de energia.
 *
 * @param brilho Fator de brilho a ser aplicado, de 0.0 (apagado) a 1.0 (brilho total).
 */
void npWriteComBrilho(float brilho) {
    for (uint i = 0; i < LED_COUNT; ++i) {
        // Calcula a nova cor com brilho aplicado.
        uint8_t r = leds[i].R * brilho;
        uint8_t g = leds[i].G * brilho;
        uint8_t b = leds[i].B * brilho;
        // Envia os dados com brilho ajustado para a SM.
        pio_sm_put_blocking(np_pio, sm, g);
        pio_sm_put_blocking(np_pio, sm, r);
        pio_sm_put_blocking(np_pio, sm, b);
    }
}

/**
 * @brief Define a cor de um único LED no buffer `leds`.
 *
 * Esta função apenas atualiza o estado do LED no buffer de software.
 * Para que a mudança seja visível na matriz física, `npWrite()` deve ser chamada posteriormente.
 *
 * @param index O índice do LED (0 a LED_COUNT-1).
 * @param r Componente de cor Vermelha (0-255).
 * @param g Componente de cor Verde (0-255).
 * @param b Componente de cor Azul (0-255).
 */
void npSetLED(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < LED_COUNT) {
        leds[index].R = r;
        leds[index].G = g;
        leds[index].B = b;
    }
}

/**
 * @brief Define a mesma cor para todos os LEDs no buffer.
 *
 * Função de conveniência que itera por todos os LEDs e chama `npSetLED` para cada um.
 *
 * @param r Componente de cor Vermelha (0-255).
 * @param g Componente de cor Verde (0-255).
 * @param b Componente de cor Azul (0-255).
 */
void npSetAll(uint8_t r, uint8_t g, uint8_t b) {
    for (uint i = 0; i < LED_COUNT; ++i) {
        npSetLED(i, r, g, b);
    }
}

/**
 * @brief Apaga todos os LEDs, definindo suas cores como (0, 0, 0) no buffer.
 *
 * Função de conveniência que chama `npSetAll` com a cor preta.
 */
void npClear(void) {
    npSetAll(0, 0, 0);
}

/**
 * @brief Desabilita e libera a Máquina de Estado do PIO para que possa ser usada por outro recurso.
 *
 * @param pio A instância do PIO (pio0 ou pio1).
 * @param sm_id O índice da Máquina de Estado a ser liberada.
 */
void liberar_maquina_pio(PIO pio, uint sm_id) {
    if (sm_id < 4) { // Valida se o índice da SM é válido (0-3)
        pio_sm_set_enabled(pio, sm_id, false); // Para a execução da SM.
        pio_sm_unclaim(pio, sm_id); // Libera a SM.
    }
}

/**
 * @brief Converte coordenadas cartesianas (x, y) para o índice 1D da fita de LED.
 *
 * A matriz 5x5 utilizada tem uma fiação em "serpentina" ou "zigzag". Esta função
 * faz a tradução matemática entre o sistema de coordenadas lógico (x, y) com
 * origem no canto superior esquerdo e o índice físico linear da fita de LED.
 *
 * A lógica é:
 * 1. Inverte a coordenada `y` para que y=0 seja a linha de cima.
 * 2. Nas linhas físicas pares (0, 2, 4), os LEDs correm da direita para a esquerda.
 * 3. Nas linhas físicas ímpares (1, 3), os LEDs correm da esquerda para a direita.
 *
 * @param x Coordenada da coluna (0 a 4).
 * @param y Coordenada da linha (0 a 4).
 * @return uint O índice linear do LED (0 a 24).
 */
uint getLEDIndex(uint x, uint y) {
    // Validação para evitar acesso fora dos limites.
    if (x >= NUM_COLUNAS || y >= NUM_LINHAS) return 0;

    // Inverte a coordenada Y, pois a linha 0 lógica é a linha 4 física na matriz.
    uint linha_fisica = NUM_LINHAS - 1 - y;
    // Calcula o índice base para o início da linha física.
    uint base = linha_fisica * NUM_COLUNAS;

    // Verifica se a linha física é par ou ímpar para determinar a direção do mapeamento.
    if (linha_fisica % 2 == 0) {
        // Em linhas pares (0, 2, 4), a contagem é da direita para a esquerda.
        return base + (NUM_COLUNAS - 1 - x);
    } else {
        // Em linhas ímpares (1, 3), a contagem é da esquerda para a direita.
        return base + x;
    }
}