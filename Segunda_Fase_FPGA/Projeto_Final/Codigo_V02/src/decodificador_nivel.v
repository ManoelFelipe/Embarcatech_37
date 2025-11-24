/*
 * ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 *
 * Módulo: decodificador_nivel
 * Descrição:
 * Converte o nível dos 5 sensores em um valor para o display de 7 segmentos.
 * A LÓGICA DO SENSOR É INVERTIDA: 0 = Água Presente, 1 = Sem Água.
 *
 * Implementa um codificador de prioridade que dá prioridade ao sensor
 * MAIS ALTO que está ativo (em '0').
 * Ex: Se [4] e [3] estiverem em '0', ele exibirá '4'.
 * ======================================================================
 */
module decodificador_nivel(
    // Entrada de 5 bits vinda dos sensores
    input  [4:0] sensores_in, // [4]=100%, [3]=75%, ..., [0]=0%
    
    // Saída de 7 bits para o display (a,b,c,d,e,f,g)
    output reg [6:0] display_out
);

    // Mapeamento dos segmentos para Cátodo Comum (1=Acende)
    localparam DISP_0 = 7'b0111111; // "0"
    localparam DISP_1 = 7'b0000110; // "1"
    localparam DISP_2 = 7'b1011011; // "2"
    localparam DISP_3 = 7'b1001111; // "3"
    localparam DISP_4 = 7'b1100110; // "4"

    // Bloco de lógica combinacional para decodificar a prioridade
    always @(*) begin
        
        // **CORREÇÃO**: Define um valor padrão para evitar a inferência de LATCH.
        // Se nenhum sensor estiver ativo (todos '1'), ele exibirá "0".
        display_out = DISP_0; 

        // Inicia a verificação de prioridade (do mais alto para o mais baixo)
        
        // Se o sensor de 100% (bit 4) está em '0' (com água)...
        if (!sensores_in[4])      
            display_out = DISP_4; // ...exibe "4"
        
        // Senão, se o sensor de 75% (bit 3) está em '0' (com água)...
        else if (!sensores_in[3]) 
            display_out = DISP_3; // ...exibe "3"
        
        // Senão, se o sensor de 50% (bit 2) está em '0' (com água)...
        else if (!sensores_in[2]) 
            display_out = DISP_2; // ...exibe "2"
        
        // Senão, se o sensor de 25% (bit 1) está em '0' (com água)...
        else if (!sensores_in[1]) 
            display_out = DISP_1; // ...exibe "1"
        
        // Senão, se o sensor de 0% (bit 0) está em '0' (com água)...
        else if (!sensores_in[0])                     
            display_out = DISP_0; // ...exibe "0"
            
        // Se todos os sensores estiverem em '1' (sem água),
        // o 'display_out' permanecerá com o valor padrão (DISP_0).
    end

endmodule