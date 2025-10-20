// ============================================================================
// Projeto: Motores Alternados — VARIANTE "TROCA INSTANTÂNEA" ao mudar I4
// Arquivo: motores_alternados_instant.sv (TOP-LEVEL)
// Autor: Manoel Furtado
//
// Diferença principal desta variante:
//   • Ao mudar o nível de I4 (após debounce/sync), a FSM ALTERNARÁ IMEDIATAMENTE
//     entre M1_ON <-> M2_ON, em qualquer momento do ciclo.
//   • STOP (I2) e RESET (I3) continuam com prioridade e levam a IDLE.
//   • START (I1) inicia sempre por M1_ON.
//   • I4 segue sendo por NÍVEL: 0 => T_NORMAL_S; 1 => T_TEST_S.
//     → Nesta variante, além de alterar o alvo T, a mudança de nível também aciona
//       troca instantânea (útil para bancada/demonstração).
//
// Demais comportamentos iguais à versão “conforme enunciado + robusta”:
//   • O3: pisca 1 Hz somente quando temporizando (M1_ON/M2_ON).
//   • O4: em ciclo (alto em M1_ON/M2_ON).
//   • O5: heartbeat ~2 Hz sempre.
//   • Debounce/sync em I1/I2/I3 (pulsos) e I4 (nível).
//
// Prioridades na FSM (por estado M1_ON/M2_ON):
//   1) STOP/RESET (pulsos) -> IDLE
//   2) MUDANÇA NÍVEL I4 (pulso de alternância) -> alterna imediato
//   3) TEMPO VENCIDO (sec_count >= target_s) -> alterna
//
// ============================================================================

`timescale 1ns/1ps

module motores_alternados_instant
#(
    parameter int unsigned CLK_HZ       = 25_000_000,
    parameter int unsigned T_NORMAL_S   = 30,
    parameter int unsigned T_TEST_S     = 3,
    parameter int unsigned DB_MS_BTN    = 10,
    parameter int unsigned DB_MS_TESTLV = 10
)(
    input  logic clk,
    // Entradas (ativo-alto)
    input  logic I1,    // START (debounced/sync -> I1_p)
    input  logic I2,    // STOP  (debounced/sync -> I2_p)
    input  logic I3,    // RESET (debounced/sync -> I3_p)
    input  logic I4,    // TESTE (NÍVEL debounced/sync -> test_level + toggle_p)
    input  logic I5,    // Reservado
    // Saídas
    output logic O1,    // Motor 1
    output logic O2,    // Motor 2
    output logic O3,    // Cronômetro ativo (pisca 1 Hz em M1_ON/M2_ON)
    output logic O4,    // Em ciclo
    output logic O5     // Heartbeat ~2 Hz
);

    // -----------------------------
    // Debounce/sync
    // -----------------------------
    function automatic int unsigned ms_to_cycles(input int unsigned ms);
        return (CLK_HZ/1000)*ms;
    endfunction
    localparam int unsigned DB_MAX_BTN    = (ms_to_cycles(DB_MS_BTN)    == 0) ? 1 : ms_to_cycles(DB_MS_BTN);
    localparam int unsigned DB_MAX_TESTLV = (ms_to_cycles(DB_MS_TESTLV) == 0) ? 1 : ms_to_cycles(DB_MS_TESTLV);

    typedef struct packed {
        logic s0, s1;
        logic st, prev;
        logic [$clog2(DB_MAX_BTN):0] cnt;
        logic pulse;
    } db_btn_t;

    typedef struct packed {
        logic s0, s1;
        logic st, prev;                           // mantém prev p/ detectar mudança de nível
        logic [$clog2(DB_MAX_TESTLV):0] cnt;
        logic toggle_p;                           // pulso quando nível muda (↑ ou ↓)
    } db_lvl_t;

    db_btn_t d1, d2, d3;
    db_lvl_t d4;

    `define DB_BTN_STEP(raw, drec, DBMAX)                                 \
        drec.s0 <= (raw);                                                 \
        drec.s1 <= drec.s0;                                               \
        if (drec.s1 == drec.st) begin                                     \
            drec.cnt <= '0;                                               \
        end else if (drec.cnt == (DBMAX)[$bits(drec.cnt)-1:0]) begin      \
            drec.st  <= drec.s1;                                          \
            drec.cnt <= '0;                                               \
        end else begin                                                    \
            drec.cnt <= drec.cnt + 1'b1;                                  \
        end                                                               \
        drec.pulse <= (drec.st && !drec.prev);                            \
        drec.prev  <= drec.st;

    `define DB_LVL_STEP(raw, drec, DBMAX)                                 \
        drec.s0 <= (raw);                                                 \
        drec.s1 <= drec.s0;                                               \
        if (drec.s1 == drec.st) begin                                     \
            drec.cnt <= '0;                                               \
        end else if (drec.cnt == (DBMAX)[$bits(drec.cnt)-1:0]) begin      \
            drec.st  <= drec.s1;                                          \
            drec.cnt <= '0;                                               \
        end else begin                                                    \
            drec.cnt <= drec.cnt + 1'b1;                                  \
        end                                                               \
        drec.toggle_p <= (drec.st ^ drec.prev);                           \
        drec.prev     <= drec.st;

    always_ff @(posedge clk) begin
        `DB_BTN_STEP(I1, d1, DB_MAX_BTN)
        `DB_BTN_STEP(I2, d2, DB_MAX_BTN)
        `DB_BTN_STEP(I3, d3, DB_MAX_BTN)
        `DB_LVL_STEP(I4, d4, DB_MAX_TESTLV)
    end

    wire I1_p       = d1.pulse;
    wire I2_p       = d2.pulse;
    wire I3_p       = d3.pulse;
    wire test_level = d4.st;       // nível estabilizado
    wire test_toggle= d4.toggle_p; // pulso quando nível muda (↑ ou ↓)

    // -----------------------------
    // Divisores: tick 1 Hz, blink 1 Hz, heartbeat ~2 Hz
    // -----------------------------
    logic [$clog2(CLK_HZ):0] tick_cnt;
    logic tick_1hz;
    always_ff @(posedge clk) begin
        if (tick_cnt == CLK_HZ-1) begin
            tick_cnt <= '0;
            tick_1hz <= 1'b1;
        end else begin
            tick_cnt <= tick_cnt + 1'b1;
            tick_1hz <= 1'b0;
        end
    end

    localparam int unsigned TOGGLE_1HZ = CLK_HZ/2;
    logic [$clog2(TOGGLE_1HZ):0] cnt_blink1;
    logic blink_1hz;
    always_ff @(posedge clk) begin
        if (cnt_blink1 == TOGGLE_1HZ-1) begin
            cnt_blink1 <= '0;
            blink_1hz  <= ~blink_1hz;
        end else begin
            cnt_blink1 <= cnt_blink1 + 1'b1;
        end
    end

    localparam int unsigned TOGGLE_2HZ = CLK_HZ/4;
    logic [$clog2(TOGGLE_2HZ):0] cnt_hb;
    logic heartbeat;
    always_ff @(posedge clk) begin
        if (cnt_hb == '0) begin
            heartbeat <= 1'b0;
        end
        if (cnt_hb == TOGGLE_2HZ-1) begin
            cnt_hb    <= '0;
            heartbeat <= ~heartbeat;
        end else begin
            cnt_hb    <= cnt_hb + 1'b1;
        end
    end

    // -----------------------------
    // FSM + temporizador
    // -----------------------------
    typedef enum logic [1:0] {IDLE, M1_ON, M2_ON} state_t;
    state_t state, state_n;

    logic [15:0] sec_count;
    wire  [15:0] target_s = test_level ? T_TEST_S[15:0] : T_NORMAL_S[15:0];

    always_comb begin
        state_n = state;
        unique case (state)
            IDLE: begin
                if (I1_p) state_n = M1_ON; // START
            end
            M1_ON: begin
                if (I2_p || I3_p)       state_n = IDLE;    // STOP/RESET têm prioridade
                else if (test_toggle)   state_n = M2_ON;    // TROCA INSTANTÂNEA ao mudar I4
                else if (sec_count >= target_s) state_n = M2_ON; // alternância por tempo
            end
            M2_ON: begin
                if (I2_p || I3_p)       state_n = IDLE;
                else if (test_toggle)   state_n = M1_ON;    // TROCA INSTANTÂNEA ao mudar I4
                else if (sec_count >= target_s) state_n = M1_ON;
            end
            default: state_n = IDLE;
        endcase
    end

    // Registro de estado
    always_ff @(posedge clk) begin
        if (I2_p || I3_p) state <= IDLE;
        else              state <= state_n;
    end

    // Contador de segundos
    always_ff @(posedge clk) begin
        if (state_n != state) begin
            sec_count <= 16'd0; // zera a cada troca (inclui troca instantânea)
        end else if (state == M1_ON || state == M2_ON) begin
            if (tick_1hz) begin
                if (sec_count != 16'hFFFF) sec_count <= sec_count + 16'd1;
            end
        end else begin
            sec_count <= 16'd0;
        end
    end

    // Saídas
    always_comb begin
        O1 = (state == M1_ON);
        O2 = (state == M2_ON);
        O4 = (state == M1_ON) || (state == M2_ON);
        O3 = O4 ? blink_1hz : 1'b0;
        O5 = heartbeat;
    end

endmodule
