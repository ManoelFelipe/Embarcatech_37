/**
 * @file funcoes_neopixel.c
 * @brief Implementação das funções para controle de LEDs WS2812B (NeoPixel) utilizando PIO no Raspberry Pi Pico.
 *
 * Este arquivo contém as funções para inicializar a comunicação PIO com a fita NeoPixel,
 * definir cores de LEDs individuais ou de todos os LEDs, limpar (apagar) os LEDs,
 * e enviar os dados de cor para a fita. Também inclui funções auxiliares para acender
 * fileiras, colunas e gerar números aleatórios para cores.
 *
 * @version 0.1
 * @author  Manoel Furtado
 * @date    01 junho 2025
 * @copyright Modificado por Manoel Furtado (MIT License) (veja LICENSE.md)
 */

#include "funcoes_neopixel.h" //
#include "ws2818b.pio.h"      // Arquivo gerado pelo pioasm para o programa PIO WS2812B
#include "pico/stdlib.h"      // Biblioteca padrão do SDK do Pico
#include "hardware/clocks.h"  // Para funções relacionadas a clocks do sistema (não usado diretamente aqui, mas pio_sm_init pode usar)

/**
 * @brief Array que define a ordem física dos LEDs na matriz 5x5, se houver um mapeamento específico.
 * Este array parece ser para um layout de LEDs em serpentina ou alguma outra ordem não linear.
 * No código atual, não está sendo diretamente utilizado para remapear os índices dos LEDs
 * nas funções `npSetLED` ou `npAcendeLED`, que operam com um índice linear.
 * Se um mapeamento fosse necessário, as funções de configuração de LED precisariam consultar este array.
 * Exemplo de uso (não implementado): `leds[ordem[index]].R = r;`
 */
uint ordem[] = { //
    4, 5, 14, 15, 24,
    3, 6, 13, 16, 23,
    2, 7, 12, 17, 22,
    1, 8, 11, 18, 21,
    0, 9, 10, 19, 20
};

/**
 * @brief Array para armazenar o estado de cor (RGB) de cada LED NeoPixel.
 * O tamanho é definido por `LED_COUNT`. Cada elemento representa um LED.
 */
npLED_t leds[LED_COUNT]; //

/**
 * @brief Instância do Programmable I/O (PIO) utilizada para controlar os LEDs NeoPixel.
 * Pode ser pio0 ou pio1.
 */
PIO np_pio; //
/**
 * @brief State Machine (SM) dentro do PIO selecionado que executa o programa WS2812B.
 */
uint sm; //

/**
 * @brief Inicializa o hardware PIO para controlar a fita de LEDs NeoPixel.
 *
 * Esta função:
 * 1. Adiciona o programa PIO `ws2818b_program` a uma instância PIO (pio0 ou pio1).
 * 2. Reivindica uma máquina de estados (SM) não utilizada no PIO escolhido.
 * 3. Inicializa a máquina de estados com o programa PIO, configurando o pino de saída
 * e a frequência de operação (800kHz para WS2812B).
 * 4. Inicializa o buffer de LEDs (`leds`) com todas as cores apagadas (0,0,0).
 *
 * @param pin O número do pino GPIO ao qual o pino de dados (DIN) da fita NeoPixel está conectado.
 */
void npInit(uint pin) { //
    // Tenta adicionar o programa PIO (ws2818b_program) ao pio0.
    // O programa PIO é definido em ws2818b.pio e compilado pelo pioasm.
    uint offset = pio_add_program(pio0, &ws2818b_program); //
    np_pio = pio0; //

    // Tenta reivindicar uma máquina de estados (SM) não utilizada no pio0.
    // O 'false' indica que não deve travar se não houver SM livre.
    sm = pio_claim_unused_sm(np_pio, false); //
    if (sm < 0) { // Se não conseguiu SM no pio0 (sm < 0 indica falha)
        // Tenta usar o pio1.
        offset = pio_add_program(pio1, &ws2818b_program); // Adiciona o programa ao pio1
        np_pio = pio1; //
        // Reivindica uma SM no pio1. O 'true' aqui significa que deve travar (panic) se não houver SM livre.
        sm = pio_claim_unused_sm(np_pio, true); //
    }

    // Inicializa a máquina de estados (SM) reivindicada com o programa ws2818b.
    // Configura o pino de saída, a frequência (800kHz para WS2812B) e passa o offset do programa.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); //

    // Inicializa todos os LEDs no buffer local como desligados (0,0,0).
    for (uint i = 0; i < LED_COUNT; ++i) { //
        leds[i].R = 0; //
        leds[i].G = 0; //
        leds[i].B = 0; //
    }
}

/**
 * @brief Define a cor de um LED NeoPixel específico no buffer local.
 *
 * Esta função atualiza o estado de cor de um LED no array `leds`.
 * Para que a mudança seja visível na fita, `npWrite()` deve ser chamada posteriormente.
 *
 * @param index O índice do LED a ser alterado (0 a LED_COUNT-1).
 * @param r Componente vermelho da cor (0-255).
 * @param g Componente verde da cor (0-255).
 * @param b Componente azul da cor (0-255).
 */
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) { //
    // Verifica se o índice está dentro dos limites da fita de LEDs.
    if (index < LED_COUNT) { //
        // Define os componentes RGB do LED especificado no buffer.
        // Note a ordem G, R, B na struct npLED_t, que é comum para WS2812B.
        leds[index].R = r; //
        leds[index].G = g; //
        leds[index].B = b; //
    }
}

/**
 * @brief Define a cor de todos os LEDs NeoPixel no buffer local para a mesma cor.
 *
 * Atualiza o estado de cor de todos os LEDs no array `leds`.
 * Para que a mudança seja visível na fita, `npWrite()` deve ser chamada posteriormente.
 *
 * @param r Componente vermelho da cor (0-255).
 * @param g Componente verde da cor (0-255).
 * @param b Componente azul da cor (0-255).
 */
void npSetAll(uint8_t r, uint8_t g, uint8_t b) { //
    // Itera por todos os LEDs da fita.
    for (uint i = 0; i < LED_COUNT; ++i) { //
        // Define a cor de cada LED no buffer.
        leds[i].R = r; //
        leds[i].G = g; //
        leds[i].B = b; //
    }
}


/**
 * @brief Acende uma fileira (linha) de LEDs em uma matriz com uma cor específica.
 * Assume um arranjo linear de LEDs onde `colunas` LEDs formam uma linha.
 * Atualiza o buffer local. `npWrite()` é necessária para exibir na fita.
 *
 * @param linha O índice da linha a ser acesa (começando em 0).
 * @param r Componente vermelho da cor (0-255).
 * @param g Componente verde da cor (0-255).
 * @param b Componente azul da cor (0-255).
 * @param colunas O número de colunas (LEDs por linha) na matriz.
 */
void acenderFileira(uint linha, uint8_t r, uint8_t g, uint8_t b, uint colunas) { //
    // Calcula o índice do primeiro LED da linha especificada.
    uint inicio = linha * colunas; //
    // Itera por todos os LEDs da coluna (dentro da linha).
    for (uint i = 0; i < colunas; ++i) { //
        uint index = inicio + i; // // Calcula o índice linear do LED atual.
        // Verifica se o índice está dentro dos limites da fita.
        if (index < LED_COUNT) { //
            // Define a cor do LED no buffer.
            npSetLED(index, r, g, b); //
        }
    }
}

/**
 * @brief Acende uma coluna de LEDs em uma matriz com uma cor específica e atualiza a fita.
 * Assume um arranjo de LEDs em matriz definido por `NUM_LINHAS` e `NUM_COLUNAS`.
 * Esta função chama `npWrite()` internamente, atualizando a fita imediatamente.
 *
 * @param coluna O índice da coluna a ser acesa (começando em 0).
 * @param r Componente vermelho da cor (0-255).
 * @param g Componente verde da cor (0-255).
 * @param b Componente azul da cor (0-255).
 */
void acender_coluna(uint8_t coluna, uint8_t r, uint8_t g, uint8_t b) { //
    // Itera por todas as linhas da matriz.
    for (int linha = 0; linha < NUM_LINHAS; linha++) { //
        // Calcula o índice linear do LED na coluna e linha especificadas.
        uint index = linha * NUM_COLUNAS + coluna; //
        // Define a cor do LED diretamente no buffer 'leds'.
        // Note que esta função modifica 'leds' diretamente, em vez de chamar npSetLED.
        // Isso é seguro pois 'leds' é acessível globalmente neste arquivo.
        leds[index].R = r; //
        leds[index].G = g; //
        leds[index].B = b; //
    }
    // Envia os dados atualizados do buffer para a fita NeoPixel.
    npWrite(); //
}


/**
 * @brief Apaga todos os LEDs NeoPixel, definindo sua cor para (0,0,0) no buffer local.
 *
 * Para que a mudança seja visível na fita, `npWrite()` deve ser chamada posteriormente.
 */
void npClear() { //
    // Itera por todos os LEDs da fita.
    for (uint i = 0; i < LED_COUNT; ++i) //
        // Define a cor de cada LED como preto (apagado) no buffer.
        npSetLED(i, 0, 0, 0); //
}

/**
 * @brief Envia os dados de cor do buffer local (`leds`) para a fita NeoPixel.
 *
 * Esta função transmite os dados de cor para cada LED através da máquina de estados PIO.
 * A ordem de envio dos componentes de cor (G, R, B) é determinada pelo programa PIO
 * e pela estrutura `npLED_t`.
 */
void npWrite() { //
    // Itera por todos os LEDs no buffer.
    for (uint i = 0; i < LED_COUNT; ++i) { //
        // Envia os componentes de cor para a FIFO da máquina de estados PIO.
        // A ordem G, R, B é crucial e corresponde ao que o programa PIO espera.
        // pio_sm_put_blocking aguarda se a FIFO estiver cheia.
        pio_sm_put_blocking(np_pio, sm, leds[i].G); //
        pio_sm_put_blocking(np_pio, sm, leds[i].R); //
        pio_sm_put_blocking(np_pio, sm, leds[i].B); //
    }
    // Pequeno atraso após o envio de todos os dados.
    // Alguns controladores NeoPixel podem precisar de um tempo de reset/latch.
    // 50us é um valor comum para o sinal de reset do WS2812B.
    sleep_us(100); //
}  

/**
 * @brief Define a cor de um LED NeoPixel específico e atualiza a fita imediatamente.
 *
 * Combina as funcionalidades de `npSetLED` e `npWrite` para uma única chamada.
 *
 * @param index O índice do LED a ser alterado (0 a LED_COUNT-1).
 * @param r Componente vermelho da cor (0-255).
 * @param g Componente verde da cor (0-255).
 * @param b Componente azul da cor (0-255).
 */
void npAcendeLED(uint index, uint8_t r, uint8_t g, uint8_t b) { //
    // Verifica se o índice está dentro dos limites da fita.
    if (index < LED_COUNT) { //
        // Define a cor do LED especificado no buffer local.
        npSetLED(index, r, g, b); //
        // Envia os dados atualizados do buffer para a fita NeoPixel.
        npWrite(); //
    }
}

/**
 * @brief Inicializa o gerador de números aleatórios.
 *
 * Esta função deve ser chamada uma vez no início do programa para semear
 * o gerador de números aleatórios com o tempo atual, garantindo sequências
 * diferentes de números aleatórios a cada execução.
 * @note No RP2040, `time(NULL)` pode não ser a melhor fonte de entropia se o RTC
 * não estiver configurado ou se o tempo não for significativo.
 * Para aplicações mais críticas, pode-se usar o `ROSC` (Ring Oscillator)
 * para semear o gerador, através do `hardware_rand` do SDK, ou usar o ADC
 * em um pino flutuante. No entanto, para simples cores aleatórias, `time(NULL)`
 * após `stdio_init_all()` (que pode inicializar o RTC) ou `srand(get_absolute_time())`
 * costuma ser suficiente.
 */
void inicializar_aleatorio() { //
    // Semeia o gerador de números aleatórios com o valor do tempo atual.
    // Se o RTC não estiver configurado, time(NULL) pode retornar um valor constante.
    // Alternativa: srand(to_us_since_boot(get_absolute_time()));
    srand(time(NULL)); //
}

/**
 * @brief Gera um número aleatório dentro de um intervalo especificado (inclusivo).
 *
 * @param min O valor mínimo do intervalo.
 * @param max O valor máximo do intervalo.
 * @return int Um número aleatório entre `min` e `max`, inclusive.
 * Retorna `min` se `max < min`.
 */
int numero_aleatorio(int min, int max) { //
    // Garante que max não seja menor que min para evitar comportamento indefinido do módulo.
    if (max < min) {
        return min; // Ou algum outro tratamento de erro, como assert.
    }
    // rand() % (N) gera números de 0 a N-1.
    // Para o intervalo [min, max], o tamanho do intervalo é (max - min + 1).
    // Então, rand() % (max - min + 1) gera números de 0 a (max - min).
    // Somando min, o resultado fica no intervalo [min, max].
    return rand() % (max - min + 1) + min; //
}