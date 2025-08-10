/**
 * @file numeros_neopixel.c
 * @brief Implementação das funções para desenhar números na matriz NeoPixel.
 *
 * Este arquivo contém a lógica para acender os LEDs corretos na matriz 5x5
 * para formar os dígitos de 1 a 6. Ele utiliza uma função auxiliar estática (`mostrar_numero`)
 * para evitar repetição de código. Cada função pública (`mostrar_numero_X`) define
 * um array com os índices dos LEDs que devem ser ligados para formar o número
 * correspondente e, em seguida, chama a função auxiliar.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#include "numeros_neopixel.h"
#include "libs/LabNeoPixel/neopixel_driver.h" // Ajuste o caminho se necessário

/**
 * @brief Função auxiliar estática para exibir um padrão na matriz de LEDs.
 *
 * Esta função é o núcleo do desenho. Ela recebe um array de índices de LEDs,
 * o tamanho desse array e a cor desejada (RGB).
 * A função primeiro limpa toda a matriz (apaga todos os LEDs), depois percorre
 * o array de índices e acende cada LED especificado com a cor fornecida.
 * Finalmente, ela envia os dados para a matriz de LEDs para que a mudança
 * seja efetivada.
 *
 * @param indices Um ponteiro para um array de `uint8_t` contendo os índices dos LEDs a serem acesos.
 * @param tamanho O número de elementos no array `indices`.
 * @param r O componente Vermelho (0-255) da cor.
 * @param g O componente Verde (0-255) da cor.
 * @param b O componente Azul (0-255) da cor.
 */
static void mostrar_numero(const uint8_t indices[], uint tamanho, uint8_t r, uint8_t g, uint8_t b) {
    // Apaga todos os LEDs da matriz para começar com uma tela limpa.
    npClear();
    // Itera sobre o array de índices.
    for (uint i = 0; i < tamanho; i++) {
        // Define a cor do LED no índice especificado.
        npSetLED(indices[i], r, g, b);
    }
    // Envia o buffer de cores atualizado para a fita de LEDs, fazendo com que
    // as alterações se tornem visíveis.
    npWrite();
}

/**
 * @brief Exibe o número 1 na matriz de LEDs.
 *
 * Define os índices dos LEDs para formar o dígito '1' e chama a função auxiliar
 * `mostrar_numero` com a cor predefinida para o número 1.
 */
void mostrar_numero_1() {
    // Array de índices dos LEDs que formam o número 1.
    uint8_t indices[] = {22, 16, 17, 12, 7, 1, 2, 3};
    // Chama a função auxiliar para desenhar o número.
    mostrar_numero(indices, sizeof(indices)/sizeof(indices[0]), COR_1_R, COR_1_G, COR_1_B);
}

/**
 * @brief Exibe o número 2 na matriz de LEDs.
 *
 * Define os índices dos LEDs para formar o dígito '2' e chama a função auxiliar
 * `mostrar_numero` com a cor predefinida para o número 2.
 */
void mostrar_numero_2() {
    uint8_t indices[] = { 21, 22, 23,  18, 13, 12, 11, 6, 1, 2, 3};
    mostrar_numero(indices, sizeof(indices)/sizeof(indices[0]), COR_2_R, COR_2_G, COR_2_B);
}

/**
 * @brief Exibe o número 3 na matriz de LEDs.
 *
 * Define os índices dos LEDs para formar o dígito '3' e chama a função auxiliar
 * `mostrar_numero` com a cor predefinida para o número 3.
 */
void mostrar_numero_3() {
    uint8_t indices[] = { 21, 22, 23, 18, 8,   13, 12, 11,  1, 2, 3};
    mostrar_numero(indices, sizeof(indices)/sizeof(indices[0]), COR_3_R, COR_3_G, COR_3_B);
}

/**
 * @brief Exibe o número 4 na matriz de LEDs.
 *
 * Define os índices dos LEDs para formar o dígito '4' e chama a função auxiliar
 * `mostrar_numero` com a cor predefinida para o número 4.
 */
void mostrar_numero_4() {
    uint8_t indices[] = {23, 21, 16, 18, 13, 12, 11, 10,8, 1 };
    mostrar_numero(indices, sizeof(indices)/sizeof(indices[0]), COR_4_R, COR_4_G, COR_4_B);
}

/**
 * @brief Exibe o número 5 na matriz de LEDs.
 *
 * Define os índices dos LEDs para formar o dígito '5' e chama a função auxiliar
 * `mostrar_numero` com a cor predefinida para o número 5.
 */
void mostrar_numero_5() {
    uint8_t indices[] = { 21, 22, 23, 16, 13, 12, 11, 8, 1, 2, 3};
    mostrar_numero(indices, sizeof(indices)/sizeof(indices[0]), COR_5_R, COR_5_G, COR_5_B);
}

/**
 * @brief Exibe o número 6 na matriz de LEDs.
 *
 * Define os índices dos LEDs para formar o dígito '6' e chama a função auxiliar
 * `mostrar_numero` com a cor predefinida para o número 6.
 */
void mostrar_numero_6() {
    uint8_t indices[] = { 21, 22, 23,  16, 13, 12, 11, 8,6, 1, 2, 3};
    mostrar_numero(indices, sizeof(indices)/sizeof(indices[0]), COR_6_R, COR_6_G, COR_6_B);
}