// ===================================================================
// Atividade: Unidade 07, Capítulo 01
// Arquivo:   design.sv
// Autor:     Manoel Felipe Costa Furtado
// Data:      05/10/2025
// Versão:    1.0
//
// Módulo:    adder_subtractor
// Descrição: Implementa um somador/subtrator completo de 1 bit.
//            O módulo calcula a soma (S) e o transporte de saída (Ts)
//            com base nas entradas A, B, um transporte de entrada (Te)
//            e um seletor de modo (M).
//
// Modo de Operação:
// - M = 0: Soma (A + B + Te) -> Ts é o Carry-Out
// - M = 1: Subtração (A - B - Te) -> Ts é o Borrow-Out
// ===================================================================

module adder_subtractor (
    input  logic M,      // Seletor de operação: 0=Soma, 1=Subtração
    input  logic A,      // Entrada (operando) A
    input  logic B,      // Entrada (operando) B
    input  logic Te,     // Transporte de Entrada (Carry-in / Borrow-in)
    output logic S,      // Saída da Soma/Diferença
    output logic Ts      // Saída de Transporte (Carry-out / Borrow-out)
);
    // A lógica da saída S (Soma/Diferença) é um XOR de 3 entradas,
    // sendo idêntica para a soma e a subtração de 1 bit.
    assign S = A ^ B ^ Te;

    // Sinais intermediários para maior clareza da lógica de Ts.
    logic carry_out;  // Armazena o resultado do carry-out da soma.
    logic borrow_out; // Armazena o resultado do borrow-out da subtração.

    // Implementa a lógica do Carry-Out de um somador completo.
    // Ocorre transporte de saída se pelo menos duas das entradas (A, B, Te) forem '1'.
    assign carry_out = (A & B) | (A & Te) | (B & Te);

    // Implementa a lógica do Borrow-Out de um subtrator completo.
    // Ocorre "empréstimo" de um bit se ~A&B, ou ~A&Te, ou B&Te.
    assign borrow_out = (~A & B) | (~A & Te) | (B & Te);

    // Multiplexador 2x1: Seleciona a saída Ts com base no modo M.
    // Se M for 0 (soma), a saída é carry_out.
    // Se M for 1 (subtração), a saída é borrow_out.
    assign Ts = (M == 0) ? carry_out : borrow_out;

endmodule