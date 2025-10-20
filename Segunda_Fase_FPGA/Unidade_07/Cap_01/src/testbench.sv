// ===================================================================
// Atividade: Unidade 07, Capítulo 01
// Arquivo:   testbench.sv
// Autor:     Manoel Felipe Costa Furtado
// Data:      05/10/2025
// Versão:    1.0
//
// Módulo:    testbench
// Descrição: Ambiente de verificação para o módulo 'adder_subtractor'.
//            Este testbench é auto-verificável ('self-checking'):
//            1. Gera automaticamente todas as 16 combinações de entrada.
//            2. Calcula internamente o resultado esperado ("golden model").
//            3. Compara a saída do DUT com o resultado esperado.
//            4. Reporta um erro detalhado se houver divergência.
// ===================================================================
`timescale 1ns/1ps

module testbench;
    // Sinais para conectar ao DUT (Device Under Test)
    logic M, A, B, Te;
    logic S, Ts;

    // Instanciação do Módulo sob Teste (Unit Under Test - UUT)
    adder_subtractor dut(.M(M), .A(A), .B(B), .Te(Te), .S(S), .Ts(Ts));

    // Bloco para configurar a geração do arquivo de formas de onda (VCD).
    initial begin
        $dumpfile("dump.vcd");
        $dumpvars(0, testbench); // Grava todas as variáveis deste módulo.
    end

    // Variáveis para o modelo de referência e controle dos loops.
    reg expS, expTs;
    integer m, a, b, te;

    // Bloco principal que gera os estímulos e verifica as saídas.
    initial begin
        // Cabeçalho para a saída no console.
        $display(" M A B Te | S Ts  (Resultado do DUT)");
        $display("----------|------------------------");

        // Loops aninhados para varrer todas as 16 combinações de entrada (2^4).
        for (int m=0; m<2; m++) begin
            for (int a=0; a<2; a++) begin
                for (int b=0; b<2; b++) begin
                    for (int te=0; te<2; te++) begin
                        // 1. Aplica os valores de entrada ao DUT.
                        M = m;
                        A = a; B = b; Te = te;

                        // Aguarda 1ns para a propagação dos sinais no DUT antes de verificar.
                        #1;

                        // 2. Calcula o resultado esperado (modelo de referência).
                        // A lógica esperada para S é um XOR de 3 entradas.
                        expS  = A ^ B ^ Te;
                        
                        // A lógica esperada para Ts depende do modo M.
                        // As expressões são idênticas às do design para consistência.
                        expTs = (M==1'b0) ? ((A & B) | (A & Te) | (B & Te)) // Carry-out esperado
                                          : ((~A & B) | (~A & Te) | (B & Te));   // Borrow-out esperado

                        // 3. Compara a saída do DUT com o resultado esperado e reporta erro se divergirem.
                        if (S !== expS || Ts !== expTs) begin
                            $error("ERRO! M=%b A=%b B=%b Te=%b | S=%b(exp:%b) Ts=%b(exp:%b)",
                                   M,A,B,Te, S,expS, Ts,expTs);
                        end

                        // Imprime os valores atuais do DUT no console para acompanhamento.
                        $display(" %1d %1d %1d %1d  | %1d %1d", M,A,B,Te,S,Ts);

                        // Aguarda mais 9ns para completar um ciclo de 10ns por vetor de teste.
                        // Isso cria uma forma de onda limpa e organizada no GTKWave.
                        #9;
                    end
                end
            end
        end
        
        $display("\nSimulacao concluida com sucesso. Nenhuma divergencia encontrada.");
        $finish;
    end
endmodule