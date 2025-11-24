/* ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 * ======================================================================
 */
// Definição do módulo 'pump_controller'
module pump_controller #(
    // Parâmetro para inverter a lógica dos sensores (se 1, inverte; se 0, usa direto)
    parameter INVERT_LEVEL_CODE = 0
)(
    // --- Entradas ---
    input  wire        clk,            // Sinal de clock global
    input  wire        rst_n,          // Reset síncrono, ativo em nível baixo (0)
    input  wire [2:0]  lvl_inf_raw,  // Nível 'cru' do tanque inferior (codificado 0-4)
    input  wire [2:0]  lvl_sup_raw,  // Nível 'cru' do tanque superior (codificado 0-4)

    // --- Saídas ---
    output reg         pump_on,        // Comando da bomba (1=LIGADA). 'reg' pois é definido em um 'always'
    output reg 
        solenoid_open,  // Comando da válvula (1=ABERTA). 'reg' pois é definido em um 'always'
    output wire        led_green,      // LED verde (espelho da bomba)
    output wire        led_red,        // LED vermelho (espelho da válvula)
    output wire [6:0]  seg_inf,        // Saída para o display 7-seg do tanque inferior
    output wire [6:0]  seg_sup         // Saída para o display 7-seg do tanque superior
);
// --- Lógica Interna ---

    // Ajusta a polaridade dos sensores conforme o parâmetro 'INVERT_LEVEL_CODE'
    // Se INVERT_LEVEL_CODE=1, inverte (4 - valor). Se 0, usa o valor 'raw'.
    wire [2:0] lvl_inf = INVERT_LEVEL_CODE ? (3'd4 - lvl_inf_raw) : lvl_inf_raw;
wire [2:0] lvl_sup = INVERT_LEVEL_CODE ? (3'd4 - lvl_sup_raw) : lvl_sup_raw;
// --- Definição da FSM (Máquina de Estados Finitos) ---

    // Define os estados da FSM (apenas 2 estados)
    localparam IDLE    = 1'b0; // Estado: Bomba desligada
    localparam PUMPING = 1'b1; // Estado: Bomba ligada

    // Registradores para o estado atual (state) e próximo estado (next)
    reg state, next;
// Define constantes (parâmetros locais) para os limiares de nível (0% a 100%)
    localparam L0  = 3'd0; // 0%
    localparam L25 = 3'd1;
// 25%
    localparam L50 = 3'd2; // 50%
    localparam L75 = 3'd3;
// 75%
    localparam L100= 3'd4; // 100%

    // --- Bloco 1: Registro de Estado (Síncrono) ---
    // Este bloco atualiza o 'state' na borda de subida do clock.
    always @(posedge clk) begin
        if (!rst_n) state <= IDLE; // Se reset (rst_n=0) for ativado, vai para IDLE
else        state <= next; // Caso contrário, atualiza o estado atual com o próximo estado
end

    // --- Bloco 2: Lógica de Próximo Estado (Combinacional) ---
    // Este bloco define o valor de 'next' com base no estado atual e nas entradas.
    always @* begin
        next = state; // Valor padrão: permanecer no estado atual
case (state)
            IDLE: begin
                // CONDIÇÃO DE PARTIDA: Se sup ≤ 25% E inf ≥ 75%
                if ((lvl_sup <= L25) && (lvl_inf >= L75)) 
                    next = PUMPING; // Muda para o estado PUMPING
end
            PUMPING: begin
                // CONDIÇÃO DE PARADA: Se sup ≥ 75% OU inf ≤ 25%
                if ((lvl_sup >= L75) || (lvl_inf <= L25)) 
                    next = IDLE; // Muda para o estado IDLE
end
        endcase
    end

    // --- Bloco 3: Lógica de Saída (Combinacional) ---
    // Define as saídas com base no estado atual e/ou nas entradas.
    always @* begin
        // A bomba está ligada (pump_on=1) se, e somente se, o estado for PUMPING
        pump_on = (state == PUMPING);
// A válvula (solenoid_open) fica aberta (1) por padrão
        // Ela fecha (0) APENAS se o tanque inferior estiver 100% cheio (L100)
        solenoid_open = (lvl_inf == L100) ?
1'b0 : 1'b1;
    end

    // --- Saídas de LEDs (espelhos) ---
    // O LED verde é um espelho direto da saída 'pump_on'
    assign led_green = pump_on;
// O LED vermelho é um espelho direto da saída 'solenoid_open'
    assign led_red   = solenoid_open;

    // --- Instanciação dos Decodificadores 7-Segmentos ---
    // Instancia o decodificador para o tanque inferior
    sevenseg_decoder u_dec_inf (.val(lvl_inf), .seg(seg_inf));
// Instancia o decodificador para o tanque superior
    sevenseg_decoder u_dec_sup (.val(lvl_sup), .seg(seg_sup));

endmodule