// ============================================================================
// Projeto: Motores Alternados (FSM + Timer) — VERSÃO COM DEBOUNCE/SYNC EM I1/I2/I3 E I4
// Arquivo: motores_alternados.sv (TOP-LEVEL)
// Autor: Manoel Furtado
//
// Objetivo funcional (conforme enunciado):
//   • Alternar dois motores (O1=M1 e O2=M2) por tempo T: 30 s (normal) ou 3 s (teste).
//   • O modo teste é selecionado por NÍVEL contínuo em I4: 0 => 30 s | 1 => 3 s.
//     → Agora I4 também passa por sincronização + debounce, MAS continua sendo usado
//       como NÍVEL (sem pulsos). Usamos d4.st (estado estável) como "test_level".
//   • START (I1) inicia o ciclo sempre em M1_ON; STOP (I2) e RESET (I3) levam a IDLE.
//   • O3: pisca 1 Hz somente quando o temporizador está ativo (em M1_ON/M2_ON).
//   • O4: “em ciclo” (alto quando em M1_ON/M2_ON).
//   • O5: heartbeat contínuo (~2 Hz), inclusive em IDLE.
//
// Robustez de bancada adicionada:
//   • Debounce + sincronização em I1/I2/I3 (botões) => pulsos únicos I1_p/I2_p/I3_p.
//   • Debounce + sincronização em I4, mantendo COMPORTAMENTO POR NÍVEL:
//     → nenhuma "troca instantânea" forçada; apenas estabiliza o nível de I4 (test_level).
//
// Parametrização:
//   • CLK_HZ       : frequência do clock em Hz (ajuste para sua placa).
//   • T_NORMAL_S   : tempo (s) por motor no modo normal (I4=0).
//   • T_TEST_S     : tempo (s) por motor no modo teste (I4=1).
//   • DB_MS_BTN    : janela de debounce (ms) para I1/I2/I3 (pulsos).
//   • DB_MS_TESTLV : janela de debounce (ms) para I4 (nível).
//
// Notas:
//   • “Imediato” em STOP/RESET significa no próximo clock — típico de FSM síncrona.
//   • Mudar I4 durante a contagem altera o alvo T; a troca entre motores ocorre
//     somente quando o temporizador “vence” (sem troca instantânea).
//   • Nomes de portas I1..I5 / O1..O5 casam com o .lpf fornecido.
// ============================================================================

`timescale 1ns/1ps

module motores_alternados
#(
    parameter int unsigned CLK_HZ       = 25_000_000, // clock padrão 25 MHz
    parameter int unsigned T_NORMAL_S   = 30,         // 30 s
    parameter int unsigned T_TEST_S     = 3,          // 3 s (modo teste)
    parameter int unsigned DB_MS_BTN    = 10,         // debounce p/ I1/I2/I3 (ms)
    parameter int unsigned DB_MS_TESTLV = 10          // debounce p/ I4 (ms) — nível
)(
    input  logic clk,   // clock
    // Entradas (ativo-alto)
    input  logic I1,    // START (debounced/sync -> I1_p)
    input  logic I2,    // STOP  (debounced/sync -> I2_p)
    input  logic I3,    // RESET (debounced/sync -> I3_p)
    input  logic I4,    // MODO TESTE (NÍVEL debounced/sync -> test_level)
    input  logic I5,    // Reservado
    // Saídas (ativo-alto; LED cátodo comum)
    output logic O1,    // Motor 1
    output logic O2,    // Motor 2
    output logic O3,    // Cronômetro ativo: pisca 1 Hz (somente em M1_ON/M2_ON)
    output logic O4,    // Em ciclo (alto em M1_ON/M2_ON)
    output logic O5     // Heartbeat ~2 Hz (sempre)
);

    // =====================================================================
    // 1) Debounce + sincronização
    // =====================================================================
    function automatic int unsigned ms_to_cycles(input int unsigned ms);
        return (CLK_HZ/1000) * ms;
    endfunction
    localparam int unsigned DB_MAX_BTN    = (ms_to_cycles(DB_MS_BTN)    == 0) ? 1 : ms_to_cycles(DB_MS_BTN);
    localparam int unsigned DB_MAX_TESTLV = (ms_to_cycles(DB_MS_TESTLV) == 0) ? 1 : ms_to_cycles(DB_MS_TESTLV);

    typedef struct packed {
        logic s0, s1;                       // sincronização 2FF
        logic st;                           // estado estável
        logic prev;                         // estado anterior estável
        logic [$clog2(DB_MAX_BTN):0] cnt;   // contador de estabilidade
        logic pulse;                        // pulso (borda ↑)
    } db_btn_t;

    typedef struct packed {
        logic s0, s1;                             // sincronização 2FF
        logic st;                                 // estado estável (nível debounced)
        logic [$clog2(DB_MAX_TESTLV):0] cnt;      // contador de estabilidade
    } db_lvl_t;

    db_btn_t d1, d2, d3; // I1/I2/I3 => pulsos
    db_lvl_t d4;         // I4       => nível

    // Macro de passo para botões (gera pulso de borda ↑)
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

    // Macro de passo para nível (NÃO gera pulso; apenas estabiliza st)
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
        end

    always_ff @(posedge clk) begin
        `DB_BTN_STEP(I1, d1, DB_MAX_BTN)    // START
        `DB_BTN_STEP(I2, d2, DB_MAX_BTN)    // STOP
        `DB_BTN_STEP(I3, d3, DB_MAX_BTN)    // RESET
        `DB_LVL_STEP(I4, d4, DB_MAX_TESTLV) // TESTE (nível debounced)
    end

    wire I1_p      = d1.pulse;
    wire I2_p      = d2.pulse;
    wire I3_p      = d3.pulse;
    wire test_level= d4.st;  // nível estabilizado para seleção de T

    // =====================================================================
    // 2) Divisores de frequência: tick de 1 Hz, blink 1 Hz, heartbeat ~2 Hz
    // =====================================================================
    logic [$clog2(CLK_HZ):0] tick_cnt /* synthesis preserve */;
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
            heartbeat <= 1'b0; // reset explícito para evitar 'X'
        end
        if (cnt_hb == TOGGLE_2HZ-1) begin
            cnt_hb    <= '0;
            heartbeat <= ~heartbeat;
        end else begin
            cnt_hb    <= cnt_hb + 1'b1;
        end
    end

    // =====================================================================
    // 3) FSM + temporizador em segundos
    // =====================================================================
    typedef enum logic [1:0] {IDLE, M1_ON, M2_ON} state_t;
    state_t state, state_n;

    logic [15:0] sec_count;

    // Seleção de alvo por NÍVEL debounced em I4 (conforme enunciado)
    wire [15:0] target_s = test_level ? T_TEST_S[15:0] : T_NORMAL_S[15:0];

    // Próximo estado (combinacional)
    always_comb begin
        state_n = state;
        unique case (state)
            IDLE: begin
                if (I1_p)                         state_n = M1_ON;  // START (pulso debounced)
            end
            M1_ON: begin
                if (I2_p || I3_p)                 state_n = IDLE;   // STOP/RESET (pulso debounced)
                else if (sec_count >= target_s)   state_n = M2_ON;  // alterna ao "vencer" T
            end
            M2_ON: begin
                if (I2_p || I3_p)                 state_n = IDLE;   // STOP/RESET (pulso debounced)
                else if (sec_count >= target_s)   state_n = M1_ON;  // alterna ao "vencer" T
            end
            default: state_n = IDLE;
        endcase
    end

    // Registro de estado (sequencial)
    always_ff @(posedge clk) begin
        if (I2_p || I3_p) state <= IDLE; // STOP/RESET imediatos (no próximo clock)
        else              state <= state_n;
    end

    // Contador de segundos
    always_ff @(posedge clk) begin
        if (state_n != state) begin
            sec_count <= 16'd0; // toda entrada em novo estado zera contagem
        end else if (state == M1_ON || state == M2_ON) begin
            if (tick_1hz) begin
                if (sec_count != 16'hFFFF) sec_count <= sec_count + 16'd1;
            end
        end else begin
            sec_count <= 16'd0; // em IDLE, mantém 0
        end
    end

    // =====================================================================
    // 4) Saídas
    // =====================================================================
    always_comb begin
        // Motores
        O1 = (state == M1_ON);
        O2 = (state == M2_ON);

        // Em ciclo
        O4 = (state == M1_ON) || (state == M2_ON);

        // Cronômetro ativo: pisca 1 Hz somente quando em ciclo
        O3 = O4 ? blink_1hz : 1'b0;

        // Heartbeat sempre ativo (~2 Hz)
        O5 = heartbeat;
    end

endmodule
