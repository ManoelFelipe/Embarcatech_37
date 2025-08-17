/*
 * @file Atividade_Uni_02_Cap_01.sv
 * @version 1.0
 * @date 17/08/2025
 * @author Manoel Felipe Costa Furtado
 * @copyright 2025 Manoel Furtado (MIT License) (veja LICENSE.md)
 * @brief Psistema de alarme simplificado
 *
 * @details
 * Módulo: alarme
 * Descrição: Implementa um sistema de alarme simplificado que é ativado
 * se pelo menos dois de três sensores estiverem ativos.
 * Entradas: A, B, C (sinais dos sensores)
 * Saída: Y (sinal de ativação do alarme)
 */
 
module alarme (
    input  wire A, // Entrada do sensor de porta
    input  wire B, // Entrada do sensor de janela
    input  wire C, // Entrada do sensor de presença
    output wire Y  // Saída para o LED do alarme
);

    // Fios (wires) intermediários para armazenar os resultados das
    // verificações de pares de sensores.
    wire AB, AC, BC;

    // Lógica Combinacional usando atribuições contínuas (assign)
    
    // A primeira porta AND verifica se os sensores A e B estão ativos
    assign AB = A & B;
    
    // A segunda porta AND verifica se os sensores A e C estão ativos
    assign AC = A & C;
    
    // A terceira porta AND verifica se os sensores B e C estão ativos
    assign BC = B & C;
    
    // A porta OR final combina os resultados. O alarme (Y) será ativado
    // se QUALQUER uma das condições anteriores (AB, AC ou BC) for verdadeira.
    assign Y = AB | AC | BC;
    
endmodule
