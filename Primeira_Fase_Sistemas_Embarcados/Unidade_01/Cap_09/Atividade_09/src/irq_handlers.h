/**
 * @file irq_handlers.h
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `src/irq_handlers.h`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#ifndef IRQ_HANDLERS_H
#define IRQ_HANDLERS_H

#include <stdbool.h>

extern volatile bool dma_temp_done;
void dma_handler_temp(void);

#endif