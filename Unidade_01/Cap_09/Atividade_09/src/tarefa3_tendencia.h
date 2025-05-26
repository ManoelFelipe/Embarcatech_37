/**
 * @file tarefa3_tendencia.h
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `src/tarefa3_tendencia.h`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

/**
 * ------------------------------------------------------------
 *  Arquivo: tarefa3_tendencia.h
 *  Projeto: TempCycleDMA
 * ------------------------------------------------------------
 *  Descrição:
 *      Este cabeçalho define a interface da Tarefa 3 do
 *      executor cíclico: análise da tendência da temperatura.
 *
 *      Fornece:
 *        - Enumeração `tendencia_t` com os três estados possíveis
 *        - Função para determinar a tendência com base na temperatura atual
 *        - Função para converter a tendência em texto
 *
 *  
 *  Data: 12/05/2025
 * ------------------------------------------------------------
 */

#ifndef TAREFA3_TENDENCIA_H
#define TAREFA3_TENDENCIA_H

#ifdef __cplusplus
extern "C" {
#endif

// Enumeração da tendência térmica
typedef enum {
    TENDENCIA_ESTAVEL,
    TENDENCIA_SUBINDO,
    TENDENCIA_CAINDO
} tendencia_t;

/**
 * @brief Analisa a tendência com base na temperatura atual e anterior.
 *
 * @param atual Temperatura atual (ºC)
 * @return tendência identificada
 */
tendencia_t tarefa3_analisa_tendencia(float atual);

/**
 * @brief Converte a tendência para texto ("SUBINDO", "CAINDO", "ESTÁVEL").
 *
 * @param t Valor do tipo tendencia_t
 * @return Ponteiro para string correspondente
 */
const char* tendencia_para_texto(tendencia_t t);

#ifdef __cplusplus
}
#endif

#endif  // TAREFA3_TENDENCIA_H