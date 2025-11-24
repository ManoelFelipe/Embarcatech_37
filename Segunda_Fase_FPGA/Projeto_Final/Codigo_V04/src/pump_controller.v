/* ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 * ======================================================================
 */

// rtl/pump_controller.v — Versão: FSM Robusta (v4)
// Sintetizável (Verilog 2001). Temporizações por contadores; sem delays # em RTL.

`timescale 1ns/1ps

module pump_controller #(
    parameter integer CLK_HZ             = 50_000_000,
    parameter integer FILTER_MS          = 20,
    parameter integer FAULT_RECOVERY_MS  = 200,
    parameter integer LVL_SUP_START      = 1, // 25%
    parameter integer LVL_INF_START      = 3, // 75%
    parameter integer LVL_SUP_STOP       = 3, // 75%
    parameter integer LVL_INF_STOP       = 1, // 25%
    parameter integer LVL_INF_REFILL     = 4, // 100%
    parameter         INVERT_LEVEL_CODE  = 1, // sensores ativos em 0
    parameter         SEG_ACTIVE_HIGH    = 1
)(
    input  wire        clk,
    input  wire        rst_n,
    input  wire [2:0]  lvl_inf_in,
    input  wire [2:0]  lvl_sup_in,
    input  wire        en_auto,
    output reg         pump_on,
    output reg         solenoid_open,
    output reg         led_green,
    output reg         led_red,
    output reg         fault,
    output reg  [6:0]  seg_inf,
    output reg  [6:0]  seg_sup
);

    // =============================
    // 1) Condicionamento de sinais
    // =============================

    // (a) Inversão opcional de códigos 0..4 (se sensores ativos em nível baixo)
    function [2:0] invert_level_0_to_4(input [2:0] v);
        case (v)
            3'd0: invert_level_0_to_4 = 3'd4;
            3'd1: invert_level_0_to_4 = 3'd3;
            3'd2: invert_level_0_to_4 = 3'd2;
            3'd3: invert_level_0_to_4 = 3'd1;
            3'd4: invert_level_0_to_4 = 3'd0;
            default: invert_level_0_to_4 = 3'd7; // inválido
        endcase
    endfunction

    wire [2:0] lvl_inf_raw = INVERT_LEVEL_CODE ? invert_level_0_to_4(lvl_inf_in) : lvl_inf_in;
    wire [2:0] lvl_sup_raw = INVERT_LEVEL_CODE ? invert_level_0_to_4(lvl_sup_in) : lvl_sup_in;

    // (b) Debounce/estabilização de leitura por FILTER_MS
    localparam integer FILTER_TICKS = (CLK_HZ/1000) * FILTER_MS;
    reg [31:0] cnt_inf, cnt_sup;
    reg [2:0]  lvl_inf_stable, lvl_sup_stable;
    reg [2:0]  lvl_inf_prev,   lvl_sup_prev;

    wire inf_invalid = (lvl_inf_raw > 3'd4);
    wire sup_invalid = (lvl_sup_raw > 3'd4);

    // filtro de nivel inferior
    always @(posedge clk) begin
        if (!rst_n) begin
            cnt_inf       <= 0;
            lvl_inf_prev  <= 3'd0;
            lvl_inf_stable<= 3'd0;
        end else begin
            if (lvl_inf_raw != lvl_inf_prev) begin
                lvl_inf_prev <= lvl_inf_raw;
                cnt_inf      <= 0;
            end else if (cnt_inf < FILTER_TICKS) begin
                cnt_inf <= cnt_inf + 1;
            end else begin
                lvl_inf_stable <= lvl_inf_raw;
            end
        end
    end

    // filtro de nivel superior
    always @(posedge clk) begin
        if (!rst_n) begin
            cnt_sup       <= 0;
            lvl_sup_prev  <= 3'd0;
            lvl_sup_stable<= 3'd0;
        end else begin
            if (lvl_sup_raw != lvl_sup_prev) begin
                lvl_sup_prev <= lvl_sup_raw;
                cnt_sup      <= 0;
            end else if (cnt_sup < FILTER_TICKS) begin
                cnt_sup <= cnt_sup + 1;
            end else begin
                lvl_sup_stable <= lvl_sup_raw;
            end
        end
    end

    // (c) Detecção de inválido (após inversão); invalidações assíncronas levam a FAULT
    wire any_invalid = inf_invalid || sup_invalid;

    // =============================
    // 2) FSM de controle
    // =============================
    typedef enum reg [2:0] {S_IDLE=3'd0, S_CK_START=3'd1, S_PUMP=3'd2, S_CK_STOP=3'd3, S_FAULT=3'd4} state_t;
    reg [2:0] state, nstate;

    // Recuperação de FAULT
    localparam integer RECOV_TICKS = (CLK_HZ/1000) * FAULT_RECOVERY_MS;
    reg [31:0] recov_cnt;

    // Histerese da válvula
    reg valve_latch; // 1=aberta, 0=fechada

    // Next-state logic
    always @* begin
        nstate = state;
        if (any_invalid) begin
            nstate = S_FAULT;
        end else begin
            case (state)
                S_IDLE: begin
                    if (en_auto &&
                        (lvl_sup_stable <= LVL_SUP_START) &&
                        (lvl_inf_stable >= LVL_INF_START)) nstate = S_CK_START;
                end
                S_CK_START: begin
                    if (!(en_auto &&
                          (lvl_sup_stable <= LVL_SUP_START) &&
                          (lvl_inf_stable >= LVL_INF_START))) nstate = S_IDLE;
                    else nstate = S_PUMP;
                end
                S_PUMP: begin
                    if ((lvl_sup_stable >= LVL_SUP_STOP) || (lvl_inf_stable <= LVL_INF_STOP)) nstate = S_CK_STOP;
                    else if (!en_auto) nstate = S_IDLE;
                end
                S_CK_STOP: begin
                    if ((lvl_sup_stable >= LVL_SUP_STOP) || (lvl_inf_stable <= LVL_INF_STOP)) nstate = S_IDLE;
                    else nstate = S_PUMP;
                end
                S_FAULT: begin
                    if (recov_cnt >= RECOV_TICKS) nstate = S_IDLE;
                end
                default: nstate = S_IDLE;
            endcase
        end
    end

    // State registers, counters, outputs
    always @(posedge clk) begin
        if (!rst_n) begin
            state       <= S_IDLE;
            recov_cnt   <= 0;
            fault       <= 1'b0;
            pump_on     <= 1'b0;
            valve_latch <= 1'b1; // começa aberta
        end else begin
            state <= nstate;

            // Recuperação de FAULT
            if (state==S_FAULT) begin
                if (!any_invalid) recov_cnt <= (recov_cnt < RECOV_TICKS) ? (recov_cnt + 1) : recov_cnt;
                else recov_cnt <= 0;
            end else begin
                recov_cnt <= 0;
            end

            // Fault flag
            fault <= (nstate==S_FAULT);

            // Bomba
            case (nstate)
                S_PUMP: pump_on <= en_auto; // ON em automático
                default: pump_on <= 1'b0;
            endcase

            // Válvula (histerese + estado seguro)
            // Fecha a 100% do inferior; reabre <75% quando não em FAULT
            if (state!=S_FAULT) begin
                if (lvl_inf_stable == LVL_INF_REFILL) valve_latch <= 1'b0; // fecha
                else if (lvl_inf_stable < 3)          valve_latch <= 1'b1; // reabre
            end
        end
    end

    // Saídas visuais
    always @* begin
        solenoid_open = valve_latch;
        led_green     = pump_on;
        led_red       = solenoid_open; // opcional: em FAULT, red piscar via TB
    end

    // 7 segmentos (exibir 0..4; 'E' em erro)
    function [6:0] seg_encode_active_high(input [3:0] v);
        case (v)
            4'd0: seg_encode_active_high = 7'b1111110;
            4'd1: seg_encode_active_high = 7'b0110000;
            4'd2: seg_encode_active_high = 7'b1101101;
            4'd3: seg_encode_active_high = 7'b1111001;
            4'd4: seg_encode_active_high = 7'b0110011;
            default: seg_encode_active_high = 7'b1001111; // 'E'
        endcase
    endfunction

    wire [3:0] inf_digit = any_invalid ? 4'hE : {1'b0, lvl_inf_stable};
    wire [3:0] sup_digit = any_invalid ? 4'hE : {1'b0, lvl_sup_stable};

    wire [6:0] seg_inf_ah = seg_encode_active_high(inf_digit);
    wire [6:0] seg_sup_ah = seg_encode_active_high(sup_digit);

    always @* begin
        if (SEG_ACTIVE_HIGH) begin
            seg_inf = seg_inf_ah;
            seg_sup = seg_sup_ah;
        end else begin
            seg_inf = ~seg_inf_ah;
            seg_sup = ~seg_sup_ah;
        end
    end

endmodule
