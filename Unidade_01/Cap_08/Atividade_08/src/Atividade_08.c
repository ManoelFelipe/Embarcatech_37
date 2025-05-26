/**
 * @file Atividade_08.c
 * @brief Simulador Portátil de Alarme para Treinamentos de Brigadas e Evacuação (Versão Modularizada)
 *
 * @objectives
 * - Configurar o Raspberry Pi Pico W como um ponto de acesso (Access Point) Wi-Fi.
 * - Iniciar servidores DHCP e DNS locais para permitir a conexão de dispositivos clientes.
 * - Criar um servidor HTTP embarcado que disponibiliza uma página HTML de controle.
 * - Permitir o controle remoto de um sistema de alarme (LEDs, Buzzer, Display OLED).
 * - Indicar estado do alarme via LED vermelho piscante e buzzer intermitente.
 * - Exibir mensagens de status ("EVACUAR", "Sistema em repouso") em um display OLED SSD1306.
 * - Usar LED verde para indicar sistema em repouso e LED azul para status do Access Point.
 * - Finalização controlada do modo Access Point via tecla 'd' no terminal serial.
 *
 * @pinout
 * Definido em app_config.h
 *
 * @author  Manoel Furtado
 * @date    25 maio 2025
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 */

#include "pico/stdlib.h"     // Para stdio_init_all, etc.
#include "pico/cyw43_arch.h" // Para cyw43_arch_poll, etc.
#include <stdio.h>           // Para printf
#include <stdlib.h>          // Para calloc, free
#include <assert.h>          // Para assert

#include "app_config.h"      // Configurações do projeto
#include "oled_display.h"    // Gerenciamento do display OLED
#include "alarm_control.h"   // Lógica do alarme e controle de LEDs/Buzzer
#include "network_manager.h" // Gerenciamento de rede (Wi-Fi AP, DHCP, DNS, HTTP Server)

// --- Variável Global para Estado do Servidor/Aplicação ---
// A estrutura TCP_SERVER_T é definida em network_manager.h
// Usamos um ponteiro para ela, alocado dinamicamente no main.
static TCP_SERVER_T *g_server_state = NULL;

/**
 * @brief Callback chamado quando um caractere está disponível na entrada serial (stdio).
 * Usado para detectar a tecla 'd' para desabilitar o Access Point e encerrar a aplicação.
 * @param param Ponteiro para a estrutura de estado do servidor TCP (TCP_SERVER_T).
 */
void key_pressed_func(void *param) {
    if (!param) {
        return;
    }
    TCP_SERVER_T *state = (TCP_SERVER_T*)param;
    int key = getchar_timeout_us(0); // Lê um caractere sem bloquear

    // Se a tecla 'd' ou 'D' for pressionada e o estado de complete ainda não foi setado
    if ((key == 'd' || key == 'D') && state && !state->complete) {
        printf("\nTecla 'd' pressionada. Preparando para encerrar...\n");
        // Apenas sinaliza para o loop principal que o programa deve terminar.
        // A desinicialização dos módulos ocorrerá no main, após o loop.
        state->complete = true;
    }
}

/**
 * @brief Função principal da aplicação.
 *
 * Inicializa todos os módulos, entra no loop principal para processar
 * eventos de rede e lógica do alarme, e depois desinicializa os módulos
 * ao encerrar.
 *
 * @return 0 se sucesso, 1 em caso de erro na inicialização.
 */
int main() {
    // Inicializa stdio para comunicação serial (printf, getchar)
    stdio_init_all();
    printf("Simulador Portatil de Alarme - Iniciando (Versão Modularizada)...\n");

    // Aloca memória para o estado do servidor/aplicação.
    // Este estado é compartilhado e modificado pela função key_pressed_func.
    g_server_state = calloc(1, sizeof(TCP_SERVER_T));
    if (!g_server_state) {
        DEBUG_printf("Falha ao alocar estado do servidor TCP.\n");
        return 1;
    }
    g_server_state->complete = false; // Garante estado inicial

    // Inicializa o módulo de controle do alarme (LEDs, Buzzer)
    alarm_control_init();

    // Inicializa o módulo do display OLED
    oled_display_init();

    // Atualiza o display OLED com o status inicial do alarme (que é inativo)
    oled_display_update_status(alarm_control_is_active());

    // Inicializa o gerenciador de rede (Wi-Fi AP, DHCP, DNS, Servidor HTTP)
    if (!network_manager_init(g_server_state)) {
        DEBUG_printf("Falha ao inicializar o gerenciador de rede.\n");
        // Se a rede falhar, desliga saídas de alarme e limpa OLED antes de sair.
        alarm_control_shutdown_outputs();
        oled_display_clear(); // Ou uma mensagem de erro no OLED
        free(g_server_state);
        g_server_state = NULL;
        return 1;
    }

    // Configura callback para ser notificado quando um caractere é pressionado no terminal serial
    stdio_set_chars_available_callback(key_pressed_func, g_server_state);
    printf("Pressione 'd' no terminal serial para desabilitar o Access Point e encerrar.\n");


    // Loop principal do programa
    while(g_server_state && !g_server_state->complete) {
        // Se estiver usando PICO_CYW43_ARCH_POLL, é necessário chamar cyw43_arch_poll()
        // periodicamente para processar eventos Wi-Fi e LwIP.
        #if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        #endif

        // Processa a lógica do alarme (piscar LEDs/Buzzer se ativo)
        alarm_control_process();

        // O PICO_CYW43_ARCH_POLL define se o driver Wi-Fi é baseado em polling ou interrupções.
        // Se baseado em polling, cyw43_arch_wait_for_work_until() pode ser usado para
        // economizar energia esperando por trabalho ou por um timeout.
        #if PICO_CYW43_ARCH_POLL
        // Espera por trabalho do Wi-Fi/LwIP ou até 10ms.
        // Um timeout menor permite que a lógica do alarme (piscar) seja mais responsiva
        // se não houver muita atividade de rede.
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10));
        #else
        // Se não estiver usando polling, o trabalho Wi-Fi/LwIP ocorre em background (interrupções).
        // Um pequeno sleep_ms pode ser usado para ceder tempo a outras tarefas, se houver.
        sleep_ms(10);
        #endif
    }

    // --- Encerramento da Aplicação ---
    printf("Encerrando aplicação...\n");

    // Desabilita o callback do stdio para evitar chamadas durante o shutdown
    stdio_set_chars_available_callback(NULL, NULL);

    // Desliga as saídas do alarme (LEDs, buzzer)
    alarm_control_shutdown_outputs();

    // Mostra mensagem de AP desligado no OLED
    oled_display_show_ap_disabled();

    // Desinicializa o gerenciador de rede (fecha servidor TCP, desabilita AP, DHCP, DNS)
    // A função key_pressed_func já chama cyw43_arch_disable_ap_mode(),
    // então network_manager_deinit pode focar em fechar TCP e desiniciar DHCP/DNS.
    // A ordem de cyw43_arch_disable_ap_mode() e cyw43_arch_deinit() é importante.
    // Primeiro desabilitamos o modo AP (se ainda não foi feito pela key_pressed_func).
    // Esta lógica está agora encapsulada em network_manager_deinit.
    // A key_pressed_func APENAS seta state->complete.
    // O AP é desabilitado em network_manager_deinit.

    // A desabilitação do AP e o fechamento dos servidores de rede precisam do LwIP rodando.
    // O cyw43_arch_deinit final desliga tudo.
    // A key_pressed_func apenas sinaliza. O main faz o shutdown ordenado.
    if (g_server_state) { // Verifica se o estado ainda é válido
         // Primeiro, desabilitar o AP explicitamente antes de fechar os servidores LwIP
        printf("Desabilitando modo Access Point...\n");
        cyw43_arch_lwip_begin();
        cyw43_arch_disable_ap_mode();
        cyw43_arch_lwip_end();
        alarm_control_set_ap_led(false); // Garante que o LED do AP está desligado

        network_manager_deinit(g_server_state); // Fecha TCP, desliga DHCP/DNS
    }


    // Desinicializa a arquitetura CYW43 (Wi-Fi) como último passo da rede.
    cyw43_arch_deinit();
    printf("Arquitetura CYW43 desinicializada.\n");

    // Libera a memória alocada para o estado do servidor
    if (g_server_state) {
        free(g_server_state);
        g_server_state = NULL;
    }

    printf("Simulador de Alarme encerrado.\n");
    return 0;
}