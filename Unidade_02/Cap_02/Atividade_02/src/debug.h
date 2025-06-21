/**
 * @file    debug.h
 * @brief   Interface pública para funções de depuração.
 * @details Declara funções úteis para imprimir informações de status no console.
 */

#ifndef DEBUG_H
#define DEBUG_H

/**
 * @brief Imprime uma linha de status formatada no console.
 * @details Exibe o estado atual do LED e a temperatura, prefixados por uma tag.
 * @param tag Uma string para identificar o contexto da mensagem de depuração (ex: "HTTP", "PERIODIC").
 */
void debug_status(const char *tag);

#endif // DEBUG_H