/**
 * @file util.c
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `lib/LabNeoPixel/util.c`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#include "util.h"
#include <stdlib.h>
#include <time.h>

/**
 * @brief Descrição da função inicializar_aleatorio.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 */

void inicializar_aleatorio(void) {
    srand(time(NULL));
}

/**
 * @brief Descrição da função numero_aleatorio.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param min Descrição do parâmetro min.
 * @param max Descrição do parâmetro max.
 * @return Valor de retorno descrevendo o significado.
 */

int numero_aleatorio(int min, int max) {
    return rand() % (max - min + 1) + min;
}

/**
 * @brief Descrição da função numero_aleatorio_0a1.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @return Valor de retorno descrevendo o significado.
 */

float numero_aleatorio_0a1(void) {
    return (float)rand() / (float)RAND_MAX;
}