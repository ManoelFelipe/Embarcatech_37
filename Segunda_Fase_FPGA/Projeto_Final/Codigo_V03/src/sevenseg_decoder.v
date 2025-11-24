/* ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 * ======================================================================
 */
// Define o módulo 'sevenseg_decoder'
module sevenseg_decoder(
    input  wire [2:0] val,   // Entrada de valor (espera-se 0 a 4)
    output reg  [6:0] seg    // Saída para os 7 segmentos (a,b,c,d,e,f,g)
                             // 'reg' pois é definido em um 'always'
);
// Bloco puramente combinacional
    always @* begin
        case (val) // Verifica o valor da entrada 'val'
            3'd0: seg = 7'b1111110;  // Padrão para '0'
            3'd1: seg = 7'b0110000;  // Padrão para '1'
            3'd2: seg = 7'b1101101;  // Padrão para '2'
            3'd3: seg = 7'b1111001;  // Padrão para '3'
            3'd4: seg = 7'b0110011;  // Padrão para '4'
            // Se 'val' for 5, 6 ou 7 (ou indefinido)
            default: seg = 7'b0000001; // Mostra um '-' (traço)
        endcase
    end
endmodule