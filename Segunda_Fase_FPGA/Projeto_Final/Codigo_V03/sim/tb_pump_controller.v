/* ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 * ======================================================================
 */
// Define a escala de tempo para a simulação: 1ns (unidade) / 1ps (precisão)
`timescale 1ns/1ps

// Define o módulo de testbench
module tb_pump_controller;
    
    // --- Geração de Clock ---
    reg clk = 0; // Declara o 'clk' como registrador
    always #5 clk = ~clk; // Gera um clock com período de 10ns (5ns HIGH, 5ns LOW) -> 100MHz

    // --- Sinais do Testbench ---
    reg rst_n;             // Registrador para controlar o reset
    reg [2:0] lvl_inf, lvl_sup; // Registradores para simular as entradas de nível
wire pump_on, solenoid_open; // Wires para capturar as saídas do DUT
    wire led_green, led_red;   // Wires para capturar as saídas de LED
    wire [6:0] seg_inf, seg_sup; // Wires para capturar as saídas 7-seg

    // --- Instanciação do DUT (Device Under Test) ---
pump_controller #(.INVERT_LEVEL_CODE(0)) DUT ( // Instancia o 'pump_controller'
        .clk(clk),                 // Conecta o clock do TB ao DUT
        .rst_n(rst_n),             // Conecta o reset do TB ao DUT
        .lvl_inf_raw(lvl_inf),     // Conecta o nível 'inf' do TB ao DUT
        .lvl_sup_raw(lvl_sup),     // Conecta o nível 'sup' do TB ao DUT
        .pump_on(pump_on),         // Conecta a saída 'pump_on' do DUT ao wire do TB
        .solenoid_open(solenoid_open), // Conecta a saída 'solenoid_open' do DUT
        .led_green(led_green),     // Conecta a saída 'led_green' do DUT
        .led_red(led_red),         // Conecta a saída 'led_red' do DUT
        .seg_inf(seg_inf),         // Conecta a saída 'seg_inf' do DUT
        .seg_sup(seg_sup)          // Conecta a saída 'seg_sup' do DUT
    );
// --- Geração de Dump (Waveform VCD) ---
    initial begin
        $dumpfile("sim/waves.vcd"); // Define o nome do arquivo VCD
        $dumpvars(0, tb_pump_controller); // Informa para dumpar todas as variáveis do TB
end

    // --- Constantes locais para Níveis (legibilidade) ---
    localparam L0=3'd0, L25=3'd1, L50=3'd2, L75=3'd3, L100=3'd4;

    // --- Tarefa (Task) auxiliar para avançar N ciclos de clock ---
    task step(input integer cycles);
repeat(cycles) @(posedge clk); // Espera 'cycles' bordas de subida do clock
    endtask

    // --- Sequência Principal de Teste ---
    initial begin
        // --- 1. Reset ---
        rst_n = 0; // Ativa o reset (nível baixo)
lvl_inf = L50; // Define níveis arbitrários durante o reset
        lvl_sup = L50;
        step(5); // Aguarda 5 ciclos com o reset ativo
        rst_n = 1; // Libera o reset
        step(2); // Aguarda 2 ciclos para estabilizar

        // --- 2. Teste de Partida Normal ---
        // Condição: sup ≤ 25% E inf ≥ 75%
        lvl_sup = L25; // Define sup = 25%
lvl_inf = L75; // Define inf = 75%
        step(2); // Aguarda 2 ciclos
        // Mostra o status no console
        $display("[1] Partida: sup=%0d inf=%0d pump_on=%0b", lvl_sup, lvl_inf, pump_on);
// VERIFICAÇÃO: A bomba DEVE ligar (ser 1)
        if (pump_on !== 1) $fatal(1, "Falha: bomba não ligou na partida!");

        // --- 3. Teste de Desligamento por Nível Superior Alto ---
        // Condição: sup ≥ 75%
        lvl_sup = L75; // Define sup = 75%
step(2); // Aguarda 2 ciclos
        $display("[2] Desligamento por SUP alto: pump_on=%0b", pump_on);
        // VERIFICAÇÃO: A bomba DEVE desligar (ser 0)
        if (pump_on !== 0) $fatal(1, "Falha: bomba não desligou com SUP >= 75%%!");

        // --- 4. Teste de Desligamento por Nível Inferior Baixo ---
        // 4a. Religa a bomba
        lvl_sup = L25; // Condição de partida
lvl_inf = L75; // Condição de partida
        step(2);
        if (pump_on !== 1) $fatal(1, "Falha: bomba não religou na condição de partida!"); // Checa se ligou
        
        // 4b. Desliga pelo nível inferior
        lvl_inf = L25; // Define inf = 25% (Condição de parada)
step(2); // Aguarda
        $display("[3] Desligamento por INF baixo: pump_on=%0b", pump_on);
        // VERIFICAÇÃO: A bomba DEVE desligar (ser 0)
        if (pump_on !== 0) $fatal(1, "Falha: bomba não desligou com INF <= 25%%!");

        // --- 5. Teste da Válvula ---
        // 5a. Válvula deve fechar em 100%
        lvl_inf = L100; // Define inf = 100%
        step(2);
// VERIFICAÇÃO: A válvula DEVE fechar (ser 0)
        if (solenoid_open !== 0) $fatal(1, "Falha: válvula não fechou em INF=100%%!");

        // 5b. Válvula deve reabrir abaixo de 100%
        lvl_inf = L75; // Define inf = 75%
        step(2);
// VERIFICAÇÃO: A válvula DEVE reabrir (ser 1)
        if (solenoid_open !== 1) $fatal(1, "Falha: válvula não reabriu quando INF<100%%!");

        // --- Fim dos Testes ---
        $display(">> Testes mínimos concluídos com sucesso!");
        $finish; // Encerra a simulação
    end
endmodule