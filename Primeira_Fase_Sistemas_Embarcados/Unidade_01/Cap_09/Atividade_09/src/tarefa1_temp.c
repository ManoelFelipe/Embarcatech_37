/**
 * @file tarefa1_temp.c
 * @brief Comentários detalhados sobre o arquivo.
 * @details Este arquivo faz parte do projeto Atividade_09. Contém implementações e definições
 *          relacionadas à funcionalidade do módulo representado pelo caminho `src/tarefa1_temp.c`.
 *          Todos os comentários seguem o padrão Doxygen em português (Brasil) para facilitar
 *          a geração de documentação automática. (Gerado em 25/05/2025).
 */

/**
 * ------------------------------------------------------------
 *  Arquivo: tarefa1_temp.c
 *  Projeto: TempCycleDMA
 * ------------------------------------------------------------
 *  Descrição:
 *      Este módulo implementa a Tarefa 1 do executor cíclico,
 *      responsável por realizar a leitura do sensor interno
 *      de temperatura utilizando ADC + DMA, durante um intervalo
 *      contínuo de 0,5 segundos.
 *
 *      A leitura é feita em blocos menores (10.000 amostras)
 *      para evitar o uso excessivo da SRAM do RP2040. Cada
 *      bloco é transferido via DMA e processado de forma
 *      incremental para obter a média da temperatura ao final
 *      do intervalo total.
 *
 *  Funcionalidades:
 *      - Converte valores brutos do ADC para graus Celsius.
 *      - Controla o tempo de aquisição com precisão usando
 *        o clock interno via 'get_absolute_time()'.
 *      - Utiliza DMA canal 0 e depende da flag 'dma_temp_done'
 *        sinalizada pelo handler definido em 'irq_handlers.c'.
 *
 *  Relacionamento:
 *      - Chamado pelo laço principal em 'main.c' como tarefa do ciclo.
 *      - Requer configuração do canal DMA e IRQ em 'setup.c'.
 *
 *  
 *  Data: 11/05/2025
 * ------------------------------------------------------------
 */

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/sync.h"
#include "tarefa1_temp.h"

#define BLOCO_AMOSTRAS 10000
#define DURACAO_AMOSTRAGEM_US 500000  // 0,5 segundos em microssegundos

static uint16_t buffer_temp[BLOCO_AMOSTRAS];
extern volatile bool dma_temp_done;

/**
 * @brief Converte valor do ADC para temperatura em °C.
 * 
 * @param raw Valor bruto de 12 bits lido do ADC.
 * @return float Temperatura em graus Celsius.
 */
/**
 * @brief Descrição da função convert_to_celsius.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param raw Descrição do parâmetro raw.
 * @return Valor de retorno descrevendo o significado.
 */

static float convert_to_celsius(uint16_t raw) {
    const float conv = 3.3f / (1 << 12);  // Conversão para tensão
    float voltage = raw * conv;
    return 27.0f - (voltage - 0.706f) / 0.001721f;
}

/**
 * @brief Inicia uma transferência via DMA de um bloco do sensor de temperatura.
 *
 * @param buffer Buffer de destino.
 * @param cfg Configuração do canal DMA.
 * @param dma_chan Canal DMA utilizado.
 */
/**
 * @brief Descrição da função iniciar_dma_temp.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param buffer Descrição do parâmetro buffer.
 * @param cfg Descrição do parâmetro cfg.
 * @param dma_chan Descrição do parâmetro dma_chan.
 * @return Valor de retorno descrevendo o significado.
 */

static void iniciar_dma_temp(uint16_t *buffer, dma_channel_config *cfg, int dma_chan) {
    adc_select_input(4);           // Canal 4 → sensor interno
    adc_fifo_drain();
    adc_run(false);
    adc_fifo_setup(true, true, 1, false, false);
    adc_run(true);

    dma_channel_configure(
        dma_chan,
        cfg,
        buffer, &adc_hw->fifo,
        BLOCO_AMOSTRAS,
        true
    );
}

/**
 * @brief Executa a Tarefa 1 do executor cíclico: coleta de temperatura por 0,5s.
 *
 * @param cfg_temp Configuração do canal DMA.
 * @param dma_chan Número do canal DMA utilizado.
 * @return float Temperatura média calculada ao final do intervalo.
 */
/**
 * @brief Descrição da função tarefa1_obter_media_temp.
 *
 * @details Explique aqui a lógica da função, parâmetros de entrada e o que ela retorna.
 * @param cfg_temp Descrição do parâmetro cfg_temp.
 * @param dma_chan Descrição do parâmetro dma_chan.
 * @return Valor de retorno descrevendo o significado.
 */

float tarefa1_obter_media_temp(dma_channel_config* cfg_temp, int dma_chan) {
    float soma = 0.0f;
    uint32_t total_amostras = 0;

    absolute_time_t inicio = get_absolute_time();

    while (absolute_time_diff_us(inicio, get_absolute_time()) < DURACAO_AMOSTRAGEM_US) {
        dma_temp_done = false;
        iniciar_dma_temp(buffer_temp, cfg_temp, dma_chan);
        while (!dma_temp_done) __wfi();  // Aguarda fim do DMA
        adc_run(false); // Desliga o ADC

        for (int i = 0; i < BLOCO_AMOSTRAS; i++) {
            soma += convert_to_celsius(buffer_temp[i]);
        }

        total_amostras += BLOCO_AMOSTRAS;
    }

    return soma / total_amostras;
}