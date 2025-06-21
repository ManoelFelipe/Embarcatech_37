/**
 * @file testes_cores.c
 * @brief Implementação das rotinas de teste para a matriz NeoPixel.
 *
 * Contém a lógica das funções de teste, que são úteis para depuração
 * e verificação do hardware e do driver da matriz de LEDs.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#include "libs/LabNeoPixel/neopixel_driver.h"
#include "pico/stdlib.h"
#include "testes_cores.h"
#include "libs/LabNeoPixel/efeitos.h"

/// @brief Estrutura simples para representar uma cor RGB.
typedef struct {
    uint8_t r, g, b;
} CorRGB;

/**
 * @brief Preenche toda a matriz com uma sequência de cores básicas.
 *
 * A função define um array de cores RGB e itera sobre ele. A cada iteração,
 * ela preenche a matriz inteira com a cor da vez usando `npSetAll`,
 * atualiza a exibição com `npWrite` e aguarda um tempo antes de passar
 * para a próxima cor.
 */
void preencher_matriz_com_cores(void) {
    // Array constante com as cores a serem testadas.
    const CorRGB cores[] = {
        {COR_MIN, COR_APAGA, COR_APAGA},  // Vermelho
        {COR_APAGA, COR_MIN, COR_APAGA},  // Verde
        {COR_APAGA, COR_APAGA, COR_MIN},  // Azul
        {COR_MIN, COR_MIN, COR_APAGA},    // Amarelo
        {COR_APAGA, COR_MIN, COR_MIN},    // Ciano
        {COR_MIN, COR_APAGA, COR_MIN},    // Magenta
        {COR_MIN, COR_MIN, COR_MIN}       // Branco suave
    };

    // Calcula o número de cores no array.
    const uint8_t total_cores = sizeof(cores) / sizeof(cores[0]);

    // Loop para percorrer cada cor.
    for (uint8_t i = 0; i < total_cores; ++i) {
        // Define todos os LEDs para a cor atual.
        npSetAll(cores[i].r, cores[i].g, cores[i].b);
        // Envia os dados para a matriz.
        npWrite();
        // Aguarda para que a cor seja visível.
        sleep_ms(700);
    }
}

/**
 * @brief Testa o acendimento individual de fileiras e colunas.
 *
 * Esta rotina demonstra o controle granular da matriz.
 * 1. Um loop `for` varre as linhas de 0 a 4. A cada passo, ele limpa a matriz
 * e acende apenas a linha atual em vermelho.
 * 2. Após uma pausa, um segundo loop `for` varre as colunas de 0 a 4,
 * acendendo cada uma em azul.
 */
void testar_fileiras_colunas(void) {
    // --- Teste das Fileiras (Linhas) ---
    // Itera de y=0 (linha de cima) até y=4 (linha de baixo).
    for (uint8_t y = 0; y < NUM_LINHAS; y++) {
        npClear(); // Limpa a matriz antes de acender a próxima linha.
        acenderFileira(y, COR_MIN, COR_APAGA, COR_APAGA); // Acende a linha 'y' em vermelho.
        npWrite(); // Atualiza a matriz.
        sleep_ms(250); // Pausa para visualização.
    }

    sleep_ms(500); // Pausa maior entre o teste de linhas e colunas.

    // --- Teste das Colunas ---
    // Itera de x=0 (coluna da esquerda) até x=4 (coluna da direita).
    for (uint8_t x = 0; x < NUM_COLUNAS; x++) {
        npClear(); // Limpa a matriz antes de acender a próxima coluna.
        acenderColuna(x, COR_APAGA, COR_APAGA, COR_MIN); // Acende a coluna 'x' em azul.
        npWrite(); // Atualiza a matriz.
        sleep_ms(250); // Pausa para visualização.
    }

    sleep_ms(500); // Pausa ao final do teste.
}