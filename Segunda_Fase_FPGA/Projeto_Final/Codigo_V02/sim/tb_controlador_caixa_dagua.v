/*
 * ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 *
 * Módulo: tb_controlador_caixa_dagua (Testbench)
 * Descrição:
 * Testbench sintético para o 'controlador_caixa_dagua' (FSM principal).
 * Observa bomba/valvula ao simular enchimento e esvaziamento das caixas.
 * ======================================================================
 */
`timescale 1ns/1ps // Define a unidade de tempo (1ns) e a precisão (1ps)

// Define o módulo do testbench
module tb_controlador_caixa_dagua;
  // --- Sinais de Estímulo (reg) ---
  reg clk, reset;           // Clock e Reset
  reg [4:0] sensores_inf; // Sensores inferiores (lógica invertida 0=água)
  reg [4:0] sensores_sup; // Sensores superiores (lógica invertida 0=água)

  // --- Sinais de Observação (wire) ---
  wire bomba;             // Saída da bomba
  wire led_vermelho;      // Saída LED vermelho
  wire led_verde;         // Saída LED verde
  wire [6:0] display_inf; // Saída Display inferior
  wire valvula;           // Saída da válvula (declarada como 'reg' no módulo, mas é 'wire' aqui)

  // === Geração de Clock ===
  // Bloco 'initial' para o clock (executa concorrentemente)
  initial begin 
    clk = 0; // Começa em 0
    forever #5 clk = ~clk; // A cada 5ns, inverte o clock (Período total = 10ns = 100MHz)
                           [cite_start]// Nota: O LPF [cite: 2] define 25MHz (40ns), mas para simulação rápida 10ns é comum.
  end

  // === Instanciação do UUT (Unit Under Test) ===
  // Conecta os 'regs' e 'wires' locais às portas do módulo controlador
  controlador_caixa_dagua UUT (
    .clk(clk),
    .reset(reset),
    .sensores_inf(sensores_inf),
    .sensores_sup(sensores_sup),
    .bomba(bomba),
    .valvula(valvula),
    .led_vermelho(led_vermelho),
    .led_verde(led_verde),
    .display_inf(display_inf)
    // Se o UUT tivesse a porta .display_sup, ela poderia ser conectada ou deixada desconectada
  );

  // === Função "Helper" ===
  // Gera padrão de sensores até nível N (0..4) como água presente (0)
  function [4:0] lvl(input integer n);
    integer i; // Variável de loop
    begin
      lvl = 5'b11111; // Padrão: sem água
      for (i=0;i<=n;i=i+1) 
        lvl[i]=1'b0; // Define bits de 0 a 'n' como '0' (com água)
    end
  endfunction

  // === Cenário de Teste Principal ===
  // Bloco 'initial' que define a sequência de estímulos
  initial begin
    $display("=== TB controlador_caixa_dagua ===");
    // 1. Aplica Reset
    reset = 1'b1; // Ativa o reset (nível alto)
    sensores_inf = 5'b11111; // Estado inicial: sem água
    sensores_sup = 5'b11111; // Estado inicial: sem água
    
    // Espera 3 ciclos de clock com o reset ativo
    repeat(3) @(posedge clk);
    
    // 2. Libera o Reset
    reset = 1'b0;

    // 3. Aguarda 10 ciclos para FSM estabilizar em S_VAZIA
    repeat(10) @(posedge clk);

    // 4. Simula o enchimento gradual da Caixa Inferior
    integer n; // Variável de loop
    for (n=0; n<=4; n=n+1) begin // Loop de n=0 até n=4
      sensores_inf = lvl(n); // Aplica o nível 'n'
      repeat(8) @(posedge clk); // Espera 8 ciclos em cada nível
    end
    // (Neste ponto, INF = 100%)

    // 5. Mantém INF em 100% por 10 ciclos
    repeat(10) @(posedge clk);

    // 6. Simula queda de nível em INF para 50%
    sensores_inf = lvl(2); // Nível 2 (0, 1, 2 = '0')
    repeat(10) @(posedge clk); // Espera 10 ciclos

    // 7. Simula enchimento da Caixa Superior (para testar lógica de transferência)
    sensores_sup = lvl(4); // SUP = 100%
    repeat(10) @(posedge clk);

    // 8. Simula mais um ciclo de esvaziamento/enchimento em INF
    sensores_inf = lvl(1); repeat(6) @(posedge clk); // INF = 25%
    sensores_inf = lvl(3); repeat(6) @(posedge clk); // INF = 75%
    sensores_inf = lvl(4); repeat(6) @(posedge clk); // INF = 100%

    // 9. Termina a simulação
    $display("=== FIM TB controlador_caixa_dagua ===");
    $finish;
  end

  // === Monitoramento ===
  // Bloco 'initial' concorrente apenas para observar as saídas
  initial begin
    // Imprime o cabeçalho da tabela uma vez
    $display("   t    | INF SUP | bomba valv ledV ledR dispINF");
    
    // $monitor é chamado sempre que qualquer sinal da lista muda
    $monitor("%8t | %b %b |   %b    %b    %b    %b   %b",
             $time,          // Tempo atual da simulação
             sensores_inf,   // Sensores INF
             sensores_sup,   // Sensores SUP
             bomba,          // Saída bomba
             valvula,        // Saída valvula
             led_verde,      // Saída led_verde
             led_vermelho,   // Saída led_vermelho
             display_inf);   // Saída display_inf
  end
  
endmodule