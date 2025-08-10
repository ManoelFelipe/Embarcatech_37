/**
 * @file util.h
 * @brief Arquivo de cabeçalho para funções de utilidade geral.
 *
 * Declara funções auxiliares que podem ser usadas em várias partes do projeto,
 * como as relacionadas à geração de números aleatórios.
 *
 * @author Modoficado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#ifndef UTIL_H
#define UTIL_H

/**
 * @brief Inicializa o gerador de números pseudoaleatórios.
 */
void inicializar_aleatorio(void);

/**
 * @brief Gera um número inteiro aleatório dentro de um intervalo inclusivo.
 * @param min O valor mínimo do intervalo.
 * @param max O valor máximo do intervalo.
 * @return int Um número inteiro aleatório entre min e max.
 */
int numero_aleatorio(int min, int max);

/**
 * @brief Gera um número de ponto flutuante aleatório entre 0.0 e 1.0.
 * @return float Um número aleatório no intervalo [0.0, 1.0].
 */
float numero_aleatorio_0a1(void);

#endif // UTIL_H