// ============================================================================
// Testbench para o Módulo: motores_alternados
//
// Arquivo:     tb_motores_alternados.sv
// Autor:       Manoel Furtado
// Data:        12/10/2025
// Versão:      1.0
//
// OBJETIVO DE VERIFICAÇÃO:
// Este testbench foi projetado para simular e verificar o comportamento do
// módulo 'motores_alternados', focando nos seguintes cenários:
//
// 1.  Inicialização: Confirma que o sistema inicia no estado IDLE com as
//     saídas corretas.
// 2.  Comando START: Testa o início do ciclo de operação com I1.
// 3.  Alternância de Motores: Verifica se M1 e M2 alternam corretamente
//     após o tempo T expirar.
// 4.  Modo de Teste (I4): Valida a mudança do tempo de alternância (T)
//     em tempo real, ao ativar e desativar a entrada I4. 
// 5.  Comando STOP (I2): Verifica a interrupção imediata do ciclo e o
//     retorno ao estado IDLE. 
// 6.  Comando RESET (I3): Valida a funcionalidade de reset, que é
//     idêntica ao STOP. 
// 7.  Reinício do Ciclo: Garante que, após um STOP ou RESET, um novo
//     START sempre reinicia o ciclo a partir de M1. 
//
// ESTRATÉGIA DE SIMULAÇÃO:
// Para tornar a simulação rápida e a análise de formas de onda viável, os
// parâmetros de tempo foram drasticamente reduzidos:
//   - A frequência do clock é reduzida de 25 MHz para 100 kHz.
//   - O tempo do modo normal é reduzido de 30s para 5s. 
//   - O tempo do modo de teste é reduzido de 3s para 2s. 
//
// ============================================================================
`timescale 1ns/1ps

module tb_motores_alternados;

    // --- Parâmetros da Simulação ---
    // Clock mais lento para reduzir o tempo de simulação.
    localparam int unsigned CLK_HZ_SIM     = 100_000;      // 100 kHz
    // Tempos de ciclo reduzidos para verificação rápida.
    localparam int unsigned T_NORMAL_S_SIM = 5;            // 5 segundos no modo normal 
    localparam int unsigned T_TEST_S_SIM   = 2;            // 2 segundos no modo de teste 

    // --- Geração de Clock ---
    logic clk;
    initial begin
        clk = 1'b0; 
        // Gera um clock de 100 kHz (Período = 1/100e3 = 10 us).
        // Meio período = 5 us = 5000 ns.
        forever #(5_000) clk = ~clk; 
    end

    // --- Sinais de Interface para o DUT (Device Under Test) ---
    logic I1, I2, I3, I4, I5; // Entradas
    wire  O1, O2, O3, O4, O5; // Saídas 

    // --- Instanciação do DUT ---
    // O módulo 'motores_alternados' é instanciado com os parâmetros de simulação.
    motores_alternados #(
        .CLK_HZ     (CLK_HZ_SIM),     // Sobrescreve o clock padrão
        .T_NORMAL_S (T_NORMAL_S_SIM), // Sobrescreve o tempo normal
        .T_TEST_S   (T_TEST_S_SIM)    // Sobrescreve o tempo de teste
    ) dut (
        .clk(clk),
        .I1 (I1), .I2 (I2), .I3 (I3), .I4 (I4), .I5 (I5), 
        .O1 (O1), .O2 (O2), .O3 (O3), .O4 (O4), .O5 (O5) 
    );

    // --- Sequência de Estímulos e Verificação ---
    initial begin
        $dumpfile("tb_motores_alternados.vcd");
        $dumpvars(0, tb_motores_alternados);

        // --- 1. Inicialização ---
        // Garante que todas as entradas comecem em '0' e o sistema estabilize.
        I1=0; I2=0; I3=0; I4=0; I5=0; 
        // Aguarda alguns ciclos de clock.
        repeat (20) @(posedge clk); 

        // --- 2. Teste de START e Ciclo Normal (T = 5s) ---
        $display("SIM: Acionando START em modo normal (T=%0ds)", T_NORMAL_S_SIM);
        // Gera um pulso na entrada I1 para iniciar o ciclo.
        I1=1;
        repeat (3) @(posedge clk); I1=0; 

        // Aguarda 12 segundos de simulação para observar a alternância:
        // M1 (5s) -> M2 (5s) -> M1 (2s).
        repeat (CLK_HZ_SIM * 12) @(posedge clk); 

        // --- 3. Teste do Modo de Teste em Tempo Real (T = 2s) ---
        $display("SIM: Ativando Modo de Teste (I4=1, T=%0ds)", T_TEST_S_SIM);
        // Ativa I4 em nível. A troca de tempo deve ser imediata.
        I4=1; 
        // Aguarda 8 segundos para observar várias alternâncias rápidas (2s cada).
        repeat (CLK_HZ_SIM * 8) @(posedge clk); 
        
        // --- 4. Retorno ao Modo Normal e Teste de STOP ---
        $display("SIM: Desativando Modo de Teste e acionando STOP");
        // Desativa o modo de teste. O tempo de ciclo volta para 5s.
        I4=0; 
        // Aguarda 6 segundos.
        repeat (CLK_HZ_SIM * 6) @(posedge clk); 

        // Gera um pulso em I2 para parar o ciclo. O sistema deve ir para IDLE.
        I2=1; repeat (3) @(posedge clk); I2=0; 
        // Aguarda 2 segundos para confirmar que o sistema permanece em IDLE.
        repeat (CLK_HZ_SIM * 2) @(posedge clk); 
        
        // --- 5. Teste de Reinício após STOP ---
        $display("SIM: Novo START após STOP, deve começar com M1");
        // Gera um novo pulso de START.
        I1=1; repeat (3) @(posedge clk); I1=0; 
        // Aguarda 3 segundos para confirmar que M1 foi ativado.
        repeat (CLK_HZ_SIM * 3) @(posedge clk); 
        
        // --- 6. Teste de RESET ---
        $display("SIM: Acionando RESET");
        // Gera um pulso em I3. O sistema deve retornar para IDLE.
        I3=1; repeat (3) @(posedge clk); I3=0; 
        // Aguarda 2 segundos para confirmar que o sistema está em IDLE.
        repeat (CLK_HZ_SIM * 2) @(posedge clk); 

        $display("SIM: Simulação concluída com sucesso.");
        $finish; // Termina a simulação.
    end

endmodule