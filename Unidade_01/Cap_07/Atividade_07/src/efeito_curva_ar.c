/**
 * @file efeito_curva_ar.c
 * @brief Implementação de um efeito de curva dinâmica com modelo autorregressivo (AR).
 *
 * Este arquivo gera um efeito visual de uma curva ou gráfico de barras que se
 * desloca horizontalmente na matriz de LEDs. A altura de cada nova barra
 * é calculada usando um modelo autorregressivo de ordem 5 (AR(5)), que gera
 * uma sequência de valores com dependência temporal e um componente de ruído,
 * resultando em um movimento suave e pseudoaleatório.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#include <stdlib.h>             // Para rand() e RAND_MAX
#include "pico/stdlib.h"
#include "libs/LabNeoPixel/neopixel_driver.h"
#include "efeito_curva_ar.h"

/// @brief Ordem do modelo autorregressivo (AR). Define quantos estados passados influenciam o presente.
#define TAM 5
/// @brief Coeficientes do modelo AR(5). Estes pesos determinam a influência de cada estado passado.
static float coef[TAM] = {0.4, -0.2, 0.15, 0.1, 0.05};
/// @brief Armazena os últimos `TAM` valores gerados pelo modelo. É a "memória" do sistema.
static float estados[TAM] = {0.0};
/// @brief Variável estática para rastrear a coluna atual (não utilizada na versão atual, mas poderia ser usada para outros efeitos).
static int coluna_atual = 0;

/// @brief Ponteiro externo para o array de LEDs definido no driver neopixel.
extern npLED_t leds[LED_COUNT];

/**
 * @brief Gera um valor de ruído aleatório em um intervalo simétrico.
 *
 * @param amp A amplitude do ruído. O valor retornado estará no intervalo [-amp, +amp].
 * @return float Um valor de ruído aleatório.
 */
static float ruido_aleatorio(float amp) {
    // ((float)rand() / RAND_MAX) gera um número entre 0.0 e 1.0.
    // Multiplicar por 2 * amp e subtrair amp mapeia o resultado para [-amp, +amp].
    return ((float)rand() / RAND_MAX) * 2 * amp - amp;
}

/**
 * @brief Calcula o próximo valor da série temporal usando o modelo AR(5).
 *
 * O valor futuro é uma soma ponderada dos valores passados (estados),
 * mais um componente de ruído aleatório. Após o cálculo, os estados são
 * atualizados (deslocados) para a próxima iteração.
 *
 * @return float O próximo valor calculado da série AR.
 */
static float proximo_valor_ar() {
    float valor = 0.0;
    // Calcula a soma ponderada dos estados anteriores.
    for (int i = 0; i < TAM; i++) {
        valor += coef[i] * estados[i];
    }
    // Adiciona ruído para tornar o movimento menos previsível.
    valor += ruido_aleatorio(1.0f);

    // Desloca os estados: o estado[i] se torna o estado[i-1].
    // O estado mais antigo é descartado.
    for (int i = TAM - 1; i > 0; i--) {
        estados[i] = estados[i - 1];
    }
    // O novo valor calculado se torna o estado mais recente.
    estados[0] = valor;

    return valor;
}

/**
 * @brief Gera e desenha um quadro do efeito de curva na matriz NeoPixel.
 *
 * A lógica da animação é dividida em duas etapas principais:
 * 1. Deslocamento: Todo o conteúdo da matriz é movido uma coluna para a esquerda.
 * 2. Desenho: Uma nova barra vertical é desenhada na última coluna da direita.
 * A altura e posição dessa barra são determinadas pelo valor gerado pela função `proximo_valor_ar`.
 *
 * @param r Componente Vermelho (0-255) da cor da barra.
 * @param g Componente Verde (0-255) da cor da barra.
 * @param b Componente Azul (0-255) da cor da barra.
 * @param delay_ms Atraso em milissegundos para controlar a velocidade da animação.
 */
void efeitoCurvaNeoPixel(uint8_t r, uint8_t g, uint8_t b, uint16_t delay_ms) {
    // Gera o próximo valor da série para determinar a altura da barra.
    float valor = proximo_valor_ar();

    // --- Cálculo da posição da barra ---
    int linha_ref = 2; // Linha central (eixo zero do gráfico).
    // O `deslocamento` é a altura da barra em relação à linha de referência.
    // O valor é multiplicado por 1.5f para amplificar o efeito visual.
    int deslocamento = (int)(valor * 1.5f);
    // A linha de destino é a extremidade da barra.
    int linha_destino = linha_ref - deslocamento;

    // Garante que a linha de destino permaneça dentro dos limites da matriz (0 a 4).
    if (linha_destino < 0) linha_destino = 0;
    if (linha_destino > NUM_LINHAS - 1) linha_destino = NUM_LINHAS - 1;

    // --- Etapa 1: Desloca toda a matriz uma coluna para a esquerda ---
    // Itera por todas as linhas da matriz.
    for (int linha = 0; linha < NUM_LINHAS; linha++) {
        // Itera pelas colunas, exceto a última.
        for (int coluna = 0; coluna < NUM_COLUNAS - 1; coluna++) {
            int idx_atual = linha * NUM_COLUNAS + coluna;
            int idx_prox  = linha * NUM_COLUNAS + coluna + 1;
            // O pixel atual recebe a cor do pixel à sua direita.
            leds[idx_atual] = leds[idx_prox];
        }
    }

    // --- Etapa 2: Escreve a nova barra na última coluna (coluna 4) ---
    int nova_coluna = NUM_COLUNAS - 1;
    // Determina os limites verticais da barra (do início ao fim).
    int inicio = linha_ref;
    int fim = linha_destino;
    // Garante que `inicio` seja sempre menor ou igual a `fim` para o loop `for`.
    if (inicio > fim) {
        int temp = inicio;
        inicio = fim;
        fim = temp;
    }

    // Itera pelas linhas da última coluna para desenhar ou apagar os pixels.
    for (int linha = 0; linha < NUM_LINHAS; linha++) {
        int index = linha * NUM_COLUNAS + nova_coluna;
        // Se a linha atual estiver dentro do intervalo da barra, acende o LED.
        if (linha >= inicio && linha <= fim) {
            npSetLED(index, r, g, b);
        } else {
            // Caso contrário, apaga o LED para garantir que o resto da coluna fique vazio.
            npSetLED(index, 0, 0, 0);
        }
    }

    // Atualiza a matriz de LEDs com os novos dados.
    npWrite();
    // Aguarda o tempo definido para controlar a velocidade da animação.
    sleep_ms(delay_ms);
}