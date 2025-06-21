/**
 * @file testes_cores.h
 * @brief Arquivo de cabeçalho para as rotinas de teste da matriz NeoPixel.
 *
 * Define os protótipos das funções utilizadas para verificar o funcionamento
 * da matriz de LEDs, como testar cores básicas e o endereçamento de
 * linhas e colunas.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#ifndef TESTE_CORES_H
#define TESTE_CORES_H

/**
 * @brief Preenche toda a matriz com uma sequência de cores básicas.
 *
 * Esta função de teste acende todos os LEDs da matriz com a mesma cor,
 * ciclando através de um conjunto predefinido de cores (vermelho, verde, azul, etc.),
 * com uma pausa entre cada cor. É útil para verificar se todas as cores
 * estão funcionando corretamente em todos os LEDs.
 */
void preencher_matriz_com_cores(void);

/**
 * @brief Testa o acendimento individual de fileiras e colunas.
 *
 * Esta função primeiro acende cada uma das 5 fileiras (linhas), de cima para baixo,
 * em vermelho. Depois, acende cada uma das 5 colunas, da esquerda para a direita,
 * em azul. É um teste excelente para verificar se o mapeamento de índices
 * (endereçamento) dos LEDs está correto.
 */
void testar_fileiras_colunas(void);

#endif // TESTE_CORES_H