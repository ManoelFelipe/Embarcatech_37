/**
 * @file tarefa3_tendencia.c
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `src/tarefa3_tendencia.c`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

/**
 * ------------------------------------------------------------
 *  Arquivo: tarefa3_tendencia.c
 *  Projeto: TempCycleDMA
 * ------------------------------------------------------------
 *  Descrição:
 *      Este módulo implementa a Tarefa 3 do executor cíclico:
 *      a análise de tendência da temperatura.
 *      
 *      A função compara a temperatura atual com a anterior
 *      e classifica como:
 *          - TENDÊNCIA_SUBINDO
 *          - TENDÊNCIA_CAINDO
 *          - TENDÊNCIA_ESTÁVEL
 *
 *      A tendência é detectada com base em um limiar mínimo
 *      de variação (±0.05°C) e pode ser exibida no terminal
 *      ou no display OLED.
 * 
 *  Funcionalidades:
 *      - Armazena a última temperatura registrada
 *      - Retorna enum `tendencia_t` representando o estado
 *      - Oferece função auxiliar para converter enum em string
 *
 *  Relacionamento:
 *      - Usado no `main.c` para complementar a exibição
 *      - Opcionalmente integrado à Tarefa 2 para mostrar no OLED
 *
 *  
 *  Data: 12/05/2025
 * ------------------------------------------------------------
 */

#include "tarefa3_tendencia.h"

static float temperatura_anterior = 0.0f;
static int primeiro_ciclo = 1;

/**
 * @brief Descrição da função tarefa3_analisa_tendencia.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param atual Descrição do parâmetro atual.
 * @return Valor de retorno descrevendo o significado.
 */

tendencia_t tarefa3_analisa_tendencia(float atual) {
    tendencia_t resultado;

    if (primeiro_ciclo) {
        resultado = TENDENCIA_ESTAVEL;
        primeiro_ciclo = 0;
    } else {
        float delta = atual - temperatura_anterior;

        if (delta > 0.01f)
            resultado = TENDENCIA_SUBINDO;
        else if (delta < -0.01f)
            resultado = TENDENCIA_CAINDO;
        else
            resultado = TENDENCIA_ESTAVEL;
    }

    temperatura_anterior = atual;
    return resultado;
}

/**
 * @brief Descrição da função tendencia_para_texto.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param t Descrição do parâmetro t.
 * @return Valor de retorno descrevendo o significado.
 */

const char* tendencia_para_texto(tendencia_t t) {
    switch (t) {
        case TENDENCIA_SUBINDO:  return "SUBINDO";
        case TENDENCIA_CAINDO:   return "CAINDO";
        default:                 return "ESTAVEL";
    }
}