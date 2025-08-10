/**
 * @file estado_mqtt.h
 * @author Modificado Por Manoel Furtado
 * @version 1.1
 * @date 15 de junho de 2025
 * @brief Declarações de variáveis globais para o estado compartilhado do sistema.
 *
 * @details
 * Este arquivo de cabeçalho (header) utiliza a palavra-chave `extern` para
 * declarar variáveis globais que são definidas em `estado_mqtt.c`. Ao incluir
 * este arquivo, outros módulos do projeto (`main.c`, `main_auxiliar.c`, etc.)
 * ganham acesso a essas variáveis compartilhadas sem precisar redefini-las.
 *
 * O uso de `extern` é uma prática padrão em C para gerenciar o estado global,
 * permitindo que diferentes partes do programa comuniquem-se e compartilhem
 * dados de forma centralizada e organizada.
 */

#ifndef ESTADO_MQTT_H
#define ESTADO_MQTT_H

#include <stdint.h>
#include <stdbool.h>

// =============================================================================
// Declarações de Variáveis de Estado Compartilhado
// =============================================================================

/**
 * @brief Endereço IP em formato binário (uint32_t) recebido do Núcleo 1.
 * O Núcleo 1, após conectar-se ao Wi-Fi, envia o IP para o Núcleo 0.
 * O Núcleo 0 utiliza esta variável para saber quando pode iniciar o cliente MQTT.
 * Um valor de 0 indica que nenhum IP foi recebido ainda.
 */
extern uint32_t ultimo_ip_bin;

/**
 * @brief Flag booleano para garantir que o cliente MQTT seja iniciado apenas uma vez.
 * Após o Núcleo 0 iniciar o cliente MQTT com sucesso, ele define este flag
 * para `true`, evitando tentativas repetidas de inicialização no loop principal.
 */
extern bool mqtt_iniciado;

// =============================================================================
// Declarações Globais para o Display OLED
// =============================================================================

/**
 * @brief Buffer de vídeo global para o display OLED.
 * Este array armazena os dados de pixel a serem renderizados na tela.
 * Seu tamanho é determinado pela resolução do display (ex: 1024 bytes para 128x64 pixels).
 * A definição real deste buffer está em `estado_mqtt.c`.
 */
extern uint8_t buffer_oled[];

/**
 * @brief Estrutura que define a área da tela a ser desenhada.
 * Utilizada pelas funções de renderização para aplicar atualizações no OLED.
 * A definição real desta estrutura está em `estado_mqtt.c`.
 */
extern struct render_area area;

#endif // ESTADO_MQTT_H