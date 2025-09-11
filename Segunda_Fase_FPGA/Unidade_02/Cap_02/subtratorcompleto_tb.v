/*
 * Atividade_Uni_02_Cap_02 
 * @file subtratorcompleto_tb.v
 * @version 1.0
 * @date 05/09/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Subtrator Completo
--------------------------------------------------------------------
  Módulo: tb_subtratorcompleto
  Descrição: Testbench para o módulo subtratorcompleto.
--------------------------------------------------------------------
*/

`include "subtratorcompleto.v"
`timescale 1ns / 1ps // Define a unidade de tempo da simulação

module tb_subtratorcompleto;

    // 1. Declaração de sinais
    // Entradas para o DUT (Device Under Test) são do tipo 'reg'
    reg a_tb;
    reg b_tb;
    reg cin_tb;

    // Saídas do DUT são do tipo 'wire'
    wire s_tb;
    wire cout_tb;

    // 2. Instanciação do Módulo a ser testado (DUT)
    // Conecta os sinais do testbench às portas do módulo
    subtratorcompleto DUT (
        .a(a_tb),
        .b(b_tb),
        .cin(cin_tb),
        .s(s_tb),
        .cout(cout_tb)
    );

    // 3. Geração dos estímulos e controle da simulação
    initial begin
        // Configuração para gerar o arquivo de forma de onda (VCD)
        $dumpfile("subtratorcompleto.vcd");
        $dumpvars(0, tb_subtratorcompleto);

        // Mensagem inicial no console
        $display("Iniciando a simulacoo do Subtrator Completo...");
        $display("A B Cin | S Cout");
        $display("-------------------");

        // Casos de teste baseados na tabela verdade
        {a_tb, b_tb, cin_tb} = 3'b000; #10;
        {a_tb, b_tb, cin_tb} = 3'b001; #10;
        {a_tb, b_tb, cin_tb} = 3'b010; #10;
        {a_tb, b_tb, cin_tb} = 3'b011; #10;
        {a_tb, b_tb, cin_tb} = 3'b100; #10;
        {a_tb, b_tb, cin_tb} = 3'b101; #10;
        {a_tb, b_tb, cin_tb} = 3'b110; #10;
        {a_tb, b_tb, cin_tb} = 3'b111; #10;

        // Finaliza a simulação após o último caso de teste
        $display("Simulacao concluida.");
        $finish;
    end

    // Monitora as mudanças nos sinais e imprime no console
    // a cada mudança, formatando para se parecer com a tabela verdade.
    always @(a_tb, b_tb, cin_tb, s_tb, cout_tb) begin
        $monitor("%b %b %b   | %b %b", a_tb, b_tb, cin_tb, s_tb, cout_tb);
    end

endmodule