/**
 * @file Atividade_cap_07.c
 * @brief Versão 3: Arquivo principal com interrupção e tratamento de debounce robusto.
 *
 * Este programa demonstra o uso da matriz de LEDs NeoPixel 5x5 (LabNeoPixel).
 * Ele aguarda o pressionar do Botão A (GPIO 5). Quando uma interrupção é detectada,
 * o programa utiliza uma técnica de debounce robusta (inspirada na Atividade 4)
 * para confirmar o pressionamento. Se confirmado, gera uma sequência de números
 * aleatórios e os exibe na matriz.
 *
 * @author Modificado por Manoel Furtado
 * @date 12 de Junho de 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h" // Necessário para interrupções de GPIO
#include "hardware/sync.h" // Para funções de sincronização como __wfi()

#include "libs/LabNeoPixel/neopixel_driver.h"
#include "src/numeros_neopixel.h"
#include "src/testes_cores.h"
#include "libs/LabNeoPixel/efeitos.h"
#include "src/efeito_curva_ar.h"


// === Definições de hardware e debounce ===
#define BOTAO_A 5 // Botão A conectado ao GPIO5
/**
 * @brief Tempo de debounce em milissegundos.
 * O programa aguardará este tempo após a detecção inicial da interrupção
 * para que o ruído elétrico do botão se estabilize.
 */
#define DEBOUNCE_MS 50

/**
 * @brief Flag volátil para comunicação entre a interrupção e o loop principal.
 */
volatile bool botao_a_pressionado = false;

/**
 * @brief Função de Callback para a interrupção do GPIO com lógica de debounce.
 *
 * Esta função é chamada pelo hardware na borda de descida (pressionar do botão).
 * Passo 1: Desabilita imediatamente a interrupção para o pino do botão para evitar "bounces".
 * Passo 2: Sinaliza ao loop principal (setando a flag) que um evento precisa ser verificado.
 *
 * @param gpio O número do pino que causou a interrupção.
 * @param events O tipo de evento que ocorreu.
 */
void botao_a_callback(uint gpio, uint32_t events) {
    // Desabilita a interrupção para este pino. O loop principal irá reabilitá-la
    // após o tratamento completo do evento.
    gpio_set_irq_enabled(BOTAO_A, GPIO_IRQ_EDGE_FALL, false);
    // Sinaliza ao loop principal que o botão foi pressionado.
    botao_a_pressionado = true;
}

/**
 * @brief Configura e inicializa o sistema, a matriz NeoPixel e a interrupção do botão.
 */
void setup() {
    // Inicializa a comunicação serial via USB.
    stdio_init_all();
    sleep_ms(1000);

    // Inicializa o driver da matriz NeoPixel.
    npInit(LED_PIN);
    // Define a semente para o gerador de números aleatórios.
    srand(time_us_32());

    // --- Configuração da Interrupção do Botão A ---
    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    // Habilita a interrupção para o pino BOTAO_A e registra a função de callback.
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &botao_a_callback);
}

/**
 * @brief Gera um número inteiro aleatório dentro de um intervalo inclusivo.
 */
int sorteia_entre(int min, int max) {
    return rand() % (max - min + 1) + min;
}

/**
 * @brief Exibe na matriz NeoPixel o número sorteado (de 1 a 6).
 */
void mostrar_numero_sorteado(int numero) {
    switch (numero) {
        case 1: mostrar_numero_1(); break;
        case 2: mostrar_numero_2(); break;
        case 3: mostrar_numero_3(); break;
        case 4: mostrar_numero_4(); break;
        case 5: mostrar_numero_5(); break;
        case 6: mostrar_numero_6(); break;
    }
}

/**
 * @brief Função principal (ponto de entrada) do programa.
 *
 * Orquestra a execução do projeto, implementando a lógica de debounce no loop principal.
 */
int main() {
    // Executa a rotina de configuração inicial.
    setup();

    printf("NeoControlLab pronto! Pressione o Botao A para sortear um numero.\n");

    // Loop infinito que constitui o programa principal.
    while (true) {
        // Verifica se a flag foi setada pela interrupção.
        if (botao_a_pressionado) {
            // Reseta a flag imediatamente.
            botao_a_pressionado = false;

            // Passo 3: Aguarda o tempo de debounce para o ruído mecânico do botão cessar.
            sleep_ms(DEBOUNCE_MS);

            // Passo 4: Re-lê o estado do pino. Se ainda estiver pressionado, confirma o clique.
            if (!gpio_get(BOTAO_A)) {
                
                // --- AÇÃO PRINCIPAL (Executada somente em um clique confirmado) ---
                
                // Sorteia quantas vezes o "dado" será lançado.
                int vezes = sorteia_entre(100, 500);
                printf("Botao A pressionado! Mostrando %d numeros aleatorios...\n", vezes);

                int sorteado = 0;
                // Loop que simula os lançamentos do dado.
                for (int i = 1; i <= vezes; i++) {
                    int n = sorteia_entre(1, 6);
                    // Imprime o progresso e o número sorteado em uma única linha
                    printf("Sorteio %d de %d: O numero sorteado foi: %d\n", i, vezes, n);
                    
                    mostrar_numero_sorteado(n);
                    sorteado = n;
                    sleep_ms(10);
                }
                
                printf("\n--- Fim da Sequencia ---\n");
                printf("Total de %d numeros sorteados. Ultimo numero sorteado: %d\n\n", vezes, sorteado);
            }
            
            // Passo 5: Reabilita a interrupção para o pino do botão,
            // independentemente de ter sido um clique válido ou apenas ruído.
            // O sistema está pronto para a próxima detecção.
            gpio_set_irq_enabled(BOTAO_A, GPIO_IRQ_EDGE_FALL, true);
        }
        
        // Coloca o processador em modo de baixo consumo até a próxima interrupção.
        __wfi();       
    }

    return 0; // Esta linha nunca será alcançada.
}