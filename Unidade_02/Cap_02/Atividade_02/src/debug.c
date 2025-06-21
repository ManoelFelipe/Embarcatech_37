/**
 * @file    debug.c
 * @brief   Implementação das funções de depuração.
 * @details Centraliza a lógica de formatação de mensagens de status para
 * facilitar a depuração da aplicação.
 */

#include "debug.h"
#include "led_control.h"
#include "temperature.h"
#include <stdio.h>

/**
 * @brief Imprime uma linha de status formatada no console.
 * @param tag Uma string para identificar o contexto da mensagem de depuração.
 */
void debug_status(const char *tag)
{
    // `printf` para formatar e enviar a string para a saída padrão (serial-USB).
    // `led_get() ? "ON" : "OFF"` é um operador ternário que escolhe a string "ON" ou "OFF"
    // com base no retorno da função `led_get()`.
    printf("[%s] LED=%s | Temp=%.2f °C\n",
           tag, 
           led_get() ? "ON" : "OFF", 
           temperature_read_c());
}