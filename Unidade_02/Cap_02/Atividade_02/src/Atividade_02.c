/**
 * @file    Atividade_02.c
 * @brief   Aplicação principal que cria um ponto-de-acesso Wi-Fi, publica uma
 * página HTTP para controlar um LED e exibir a temperatura interna do RP2040.
 * 
 * @details Este projeto demonstra uma arquitetura de software modular para sistemas
 * embarcados usando o Raspberry Pi Pico W. As funcionalidades de hardware,
 * rede e depuração são encapsuladas em módulos distintos para manter a 
 * função main() limpa e focada na lógica da aplicação.
 * 
 * @author  Manoel Furtado
 * @date    08 de Junho de 2025
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 */

#include <stdio.h>           // Inclui a biblioteca padrão de Entrada/Saída (para usar printf).
#include "pico/stdlib.h"     // Inclui a biblioteca principal do SDK do Pico (funções de hardware).
#include "wifi_ap.h"         // Inclui o módulo que gerencia o ponto de acesso Wi-Fi.
#include "led_control.h"     // Inclui o módulo para controlar o estado do LED.
#include "temperature.h"     // Inclui o módulo para ler o sensor de temperatura interno.
#include "web_server.h"      // Inclui o módulo do servidor HTTP que serve a página web.
#include "debug.h"           // Inclui o módulo com funções de ajuda para depuração.

/* ====== Configurações gerais da aplicação ============================== */

/** @brief Define o pino GPIO ao qual o LED está conectado. */
#define APP_LED_GPIO          13

/** @brief Define o número de amostras a serem lidas do ADC para calcular a média da temperatura. 
 * @details Um número maior de amostras resulta em uma leitura mais estável, mas leva mais tempo. */
#define APP_TEMP_SAMPLES      64

/** @brief Define o intervalo, em milissegundos, para a exibição de mensagens de depuração no console. */
#define APP_DEBUG_PERIOD_MS   1000

/** @brief Define o SSID (nome da rede) para o ponto de acesso Wi-Fi que será criado. */
#define APP_WIFI_SSID         "picow_test"

/** @brief Define a senha para o ponto de acesso Wi-Fi. */
#define APP_WIFI_PASSWORD     "password"

/** @brief Define a porta na qual o servidor HTTP irá escutar por conexões. A porta 80 é a padrão para HTTP. */
#define APP_HTTP_PORT         80

/* ====== main() ========================================================== */

/**
 * @brief Ponto de entrada principal da aplicação.
 * @details A função realiza três etapas principais:
 * 1. Inicializa todos os subsistemas (LED, temperatura, Wi-Fi, servidor web).
 * 2. Entra em um loop infinito para processar eventos de rede e comandos do usuário.
 * 3. Realiza uma finalização ordenada quando solicitado.
 * @return int Retorna 0 em caso de sucesso e 1 em caso de falha na inicialização.
 */
int main(void)
{
    // Inicializa a E/S padrão (para que `printf` funcione, geralmente via USB-serial).
    stdio_init_all();
    printf("\n=== Atividade 02 - aplicação modular ===\n");

    /* 1. Inicialização dos módulos -------------------------------------- */
    
    // Inicializa o módulo de controle do LED no GPIO definido, com o estado inicial desligado.
    led_init(APP_LED_GPIO, /*initial_state=*/false);

    // Inicializa o módulo de temperatura, especificando o número de amostras para a média.
    temperature_init(/*num_samples=*/APP_TEMP_SAMPLES);

    // Inicializa o modo Access Point (AP) do Wi-Fi. Se falhar, encerra a aplicação.
    if (!wifi_ap_init(APP_WIFI_SSID, APP_WIFI_PASSWORD))  {
        return 1;
    }

    // Inicia o servidor web na porta configurada. Se falhar, encerra a aplicação.
    if (!web_server_start(APP_HTTP_PORT)) {
        return 1;
    }

    // Configura um temporizador para a próxima mensagem de depuração periódica.
    absolute_time_t next_dbg = make_timeout_time_ms(APP_DEBUG_PERIOD_MS);

    /* 2. Loop principal -------------------------------------------------- */
    
    // O loop continua enquanto não houver uma solicitação de desligamento.
    while (!wifi_ap_must_shutdown()) {
        
        // Funções de "polling" que devem ser chamadas repetidamente para que a pilha de 
        // rede (lwIP) e o driver do Wi-Fi (cyw43) processem eventos pendentes.
        // Essencial para o funcionamento da rede em modo não-bloqueante.
        wifi_ap_poll();
        web_server_poll();

        // Bloco para exibir mensagens de depuração em intervalos regulares.
        // `absolute_time_diff_us` verifica se o tempo `next_dbg` já foi alcançado.
        if (absolute_time_diff_us(get_absolute_time(), next_dbg) < 0) {
            debug_status("PERIODIC");
            // Agenda a próxima exibição para o futuro, mantendo o intervalo preciso.
            next_dbg = delayed_by_ms(next_dbg, APP_DEBUG_PERIOD_MS);
        }

        // Verifica se o caractere 'q' ou 'Q' foi digitado no console serial.
        // `getchar_timeout_us(0)` é uma forma não-bloqueante de ler um caractere.
        int c = getchar_timeout_us(0);
        if (c == 'q' || c == 'Q') {
            // Se 'q' foi pressionado, solicita o desligamento do ponto de acesso,
            // o que fará com que o loop principal termine.
            wifi_ap_request_shutdown();
        }

        // Uma pequena pausa para evitar que o loop consuma 100% da CPU.
        // Isso permite que o processador economize energia e lide com outras tarefas de baixa prioridade.
        sleep_ms(10);
    }

    /* 3. Finalização ordenada ------------------------------------------- */
    
    // Para o servidor web, deixando de aceitar novas conexões.
    web_server_stop();
    // Desativa a interface Wi-Fi e libera os recursos.
    wifi_ap_deinit();
    
    printf("Encerrado.\n");
    return 0;
}