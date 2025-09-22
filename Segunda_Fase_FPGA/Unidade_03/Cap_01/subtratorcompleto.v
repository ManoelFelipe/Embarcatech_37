/*
 * Atividade_Uni_02_Cap_02 
 * @file Atividade_Uni_02_Cap_02.sv
 * @version 1.0
 * @date 05/09/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Subtrator Completo
 --------------------------------------------------------------------
   Módulo: subtratorcompleto
   Descrição: Representa um subtrator completo de 1 bit.
   Entradas: a, b, cin (borrow-in)
   Saídas: s (diferença), cout (borrow-out)
--------------------------------------------------------------------
*/
module subtratorcompleto(
    input a,
    input b,
    input cin,
    output s,
    output cout
);

    // Lógica para a saída de diferença (S)
    assign s = a ^ b ^ cin;

    // Lógica para a saída de empréstimo (Cout)
    // Cout é '1' quando precisamos "emprestar" do próximo bit.
    // Isso ocorre se (não A e B) ou (não A e Cin) ou (B e Cin).
    assign cout = (~a & b) | (~a & cin) | (b & cin);

endmodule

