/*
 * ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 *
 * Módulo: tb_decodificador_nivel (Testbench)
 * Descrição:
 * Testbench auto-verificável para o módulo 'decodificador_nivel'.
 * Verifica o mapeamento do display (lógica de sensor INVERTIDA: 0 = água presente).
 * ======================================================================
 */
`timescale 1ns/1ps // Define a unidade de tempo (1ns) e a precisão (1ps) da simulação

// Define o módulo do testbench (não possui portas de entrada/saída)
module tb_decodificador_nivel;

  // Sinais 'reg' são usados para aplicar estímulos (entradas) ao UUT
  reg  [4:0] sensores_in;   // [4]=100%, [3]=75%, [2]=50%, [1]=25%, [0]=0%
  // Sinais 'wire' são usados para observar as saídas do UUT
  wire [6:0] display_out;

  // UUT = Unit Under Test (Unidade Sob Teste)
  // Instanciação do módulo 'decodificador_nivel' que queremos testar
  decodificador_nivel uut (
    .sensores_in(sensores_in), // Conecta o 'reg' local à entrada do UUT
    .display_out(display_out)  // Conecta o 'wire' local à saída do UUT
  );

  // === Definição dos valores esperados ("Golden Values") ===
  // Códigos esperados (espelhando o módulo) para comparação
  localparam DISP_0 = 7'b0111111; // "0"
  localparam DISP_1 = 7'b0000110; // "1"
  localparam DISP_2 = 7'b1011011; // "2"
  localparam DISP_3 = 7'b1001111; // "3"
  localparam DISP_4 = 7'b1100110; // "4"

  // === Funções "Helper" para facilitar o teste ===

  // Função: level_to_sensors
  // Gera o padrão de sensores (lógica invertida) para um dado nível 'n'
  // Ex: n=2 -> retorna 5'b11000 (0%, 25%, 50% = '0'; 75%, 100% = '1')
  function [4:0] level_to_sensors(input integer n);
    integer i; // Variável de loop
    begin
      level_to_sensors = 5'b11111; // Valor padrão (tudo '1', sem água)
      // Loop: para i de 0 até n...
      for (i = 0; i <= n; i = i + 1) 
        level_to_sensors[i] = 1'b0; // ...define o bit 'i' como '0' (água presente)
    end
  endfunction

  // Função: expected_disp
  // Retorna o código de display esperado para um dado nível 'n'
  function [6:0] expected_disp(input integer n);
    begin
      case (n) // Seleciona com base no nível 'n'
        0: expected_disp = DISP_0; // Se n=0, espera "0"
        1: expected_disp = DISP_1; // Se n=1, espera "1"
        2: expected_disp = DISP_2; // Se n=2, espera "2"
        3: expected_disp = DISP_3; // Se n=3, espera "3"
        4: expected_disp = DISP_4; // Se n=4, espera "4"
        default: expected_disp = 7'b0000000; // Padrão (não deve ocorrer)
      endcase
    end
  endfunction

  // Tarefa (Task): check_level
  // Executa a verificação completa para um nível 'n'
  task check_level(input integer n);
    reg [6:0] exp; // Variável local para armazenar o valor esperado
    begin
      // 1. Aplica o estímulo (converte nível 'n' para padrão de sensor)
      sensores_in = level_to_sensors(n);
      // 2. Aguarda um pequeno delta-time para a lógica combinacional propagar
      #1; 
      // 3. Calcula o resultado esperado
      exp = expected_disp(n);
      
      // 4. Compara a saída real (display_out) com a esperada (exp)
      //    Usa '!==" (comparação "case-equality" que checa X e Z)
      if (display_out !== exp) begin
        // 5. Reporta ERRO se forem diferentes
        $display("[ERRO] Nível=%0d sensores=%b => display=%b (esperado=%b)", n, sensores_in, display_out, exp);
      end else begin
        // 6. Reporta OK se forem iguais
        $display("[OK]   Nível=%0d sensores=%b => display=%b", n, sensores_in, display_out);
      end
    end
  endtask

  // === Sequência Principal de Teste ===
  // O bloco 'initial' é executado uma vez no início da simulação
  initial begin
    $display("=== TB decodificador_nivel ===");
    
    // Testa todos os níveis individuais (0 a 4)
    check_level(0); // 0%
    check_level(1); // 25%
    check_level(2); // 50%
    check_level(3); // 75%
    check_level(4); // 100%

    // --- Teste de Prioridade ---
    // Verifica se o decodificador prioriza o nível mais alto
    // Define [4] (100%) e [0] (0%) como ativos ('0')
    sensores_in = 5'b01110; 
    #1; // Espera a propagação

    // A saída DEVE ser DISP_4 (nível "4")
    if (display_out !== DISP_4) 
      $display("[ERRO] Prioridade falhou (esperado '4'), display=%b", display_out);
    else 
      $display("[OK] Prioridade (100%% > 0%%)");

    $display("=== FIM TB decodificador_nivel ===");
    $finish; // Termina a simulação
  end
endmodule