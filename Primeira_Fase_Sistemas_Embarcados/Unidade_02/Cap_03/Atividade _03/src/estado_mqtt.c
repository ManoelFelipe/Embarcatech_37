/**
 * @file estado_mqtt.c
 * @author Modificado Por Manoel Furtado
 * @version 1.1
 * @date 15 de junho de 2025
 * @brief Definição do estado compartilhado do cliente MQTT e estruturas globais do OLED.
 *
 * @details
 * Este arquivo é o ponto central de armazenamento para **variáveis globais**
 * utilizadas em múltiplos arquivos do sistema. Ele implementa as variáveis
 * declaradas como `extern` em `estado_mqtt.h`.
 *
 * A principal vantagem desta abordagem é centralizar informações que precisam
 * ser acessadas e modificadas por diferentes partes do código, como:
 * - O código do Núcleo 0 (`main.c` e `main_auxiliar.c`).
 * - O código da camada de rede (`mqtt_lwip.c`, não fornecido).
 *
 * Ao fazer isso, evitamos a duplicação de variáveis e facilitamos a
 * manutenção, a legibilidade e a consistência do estado em todo o projeto.
 *
 * Variáveis definidas:
 * - `ultimo_ip_bin`: Armazena o endereço IP recebido para que o Núcleo 0 possa iniciar o MQTT.
 * - `mqtt_iniciado`: Um flag que previne a reinicialização do cliente MQTT.
 * - `buffer_oled`: O buffer de vídeo para o display.
 * - `area`: A estrutura que define a região da tela a ser atualizada.
 */

#include "estado_mqtt.h"        // Inclui as declarações `extern` das variáveis que este arquivo irá definir.
#include "ssd1306_i2c.h"        // Incluído para ter acesso a `ssd1306_buffer_length`, que define o tamanho do buffer do OLED.

// =============================================================================
// Definições Globais Únicas (Implementação das variáveis `extern`)
// =============================================================================

/**
 * @brief Endereço IP mais recente recebido via FIFO do Núcleo 1.
 *
 * Esta variável armazena o endereço IP em formato binário.
 * É inicializada com 0. O Núcleo 1 a atualiza (indiretamente, via FIFO)
 * quando obtém um IP. O Núcleo 0 a lê para verificar se a rede está pronta
 * e, então, prosseguir com a inicialização do cliente MQTT.
 */
uint32_t ultimo_ip_bin = 0;

/**
 * @brief Flag de controle que indica se o cliente MQTT já foi iniciado.
 *
 * Este flag é essencial para a lógica de estado no loop principal do Núcleo 0.
 * Ele é verificado a cada iteração para garantir que a função `iniciar_mqtt_cliente()`
 * seja chamada apenas uma vez, logo após a obtenção do endereço IP.
 * Inicializado como `false`.
 */
bool mqtt_iniciado = false;

/**
 * @brief Buffer de vídeo para o display OLED.
 *
 * Este array de bytes é o "canvas" onde todo o conteúdo visual é desenhado
 * antes de ser enviado para a tela do OLED. Seu tamanho é definido pela
 * constante `ssd1306_buffer_length` da biblioteca do display, que é calculada
 * com base na resolução (geralmente 128x64 pixels).
 */
uint8_t buffer_oled[ssd1306_buffer_length];

/**
 * @brief Estrutura que define a área da tela a ser desenhada.
 *
 * Esta estrutura é passada para as funções de renderização (como `render_on_display()`)
 * para indicar qual porção do `buffer_oled` deve ser transferida para o display.
 * Isso permite otimizações, como a atualização de apenas uma parte da tela.
 */
struct render_area area;