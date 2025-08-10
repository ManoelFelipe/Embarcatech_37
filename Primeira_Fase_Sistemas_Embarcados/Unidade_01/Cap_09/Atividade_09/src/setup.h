/**
 * @file setup.h
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `src/setup.h`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

#ifndef SETUP_H
#define SETUP_H

#include "hardware/dma.h"

#define DMA_TEMP_CHANNEL 0

extern dma_channel_config cfg_temp;

void setup(void);

#endif