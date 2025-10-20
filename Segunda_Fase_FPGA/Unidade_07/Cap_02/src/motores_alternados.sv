
// Projeto:     Controle de Acionamento Alternado de Motores com FSM e Temporizador
// Arquivo:     motores_alternados.sv
// Autor:       Manoel Felipe Costa Furtado
// Data:        12/10/2025
// Versão:      1.0
//
// DESCRIÇÃO DO FUNCIONAMENTO:
// Este módulo implementa uma máquina de estados finitos (FSM) para controlar dois
// motores (representados pelas saídas O1 e O2).
// O sistema opera da seguinte forma:
//
// 1. ESTADO INICIAL (IDLE):
//    - O sistema aguarda um comando de START (entrada I1).
//    - A saída O5 (Heartbeat) pisca continuamente para indicar que o FPGA está ativo.
//
// 2. CICLO de OPERAÇÃO (M1_ON -> M2_ON):
//    - Ao receber o comando START (I1), o sistema ativa o Motor 1 (O1=1) por um
//      período T e entra no estado M1_ON. 
//    - Ao final do tempo T, desliga o Motor 1 (O1=0) e ativa o Motor 2 (O2=1),
//      transitando para o estado M2_ON. O temporizador é reiniciado.
//    - Ao final do tempo T, desliga o Motor 2 (O2=0) e reativa o Motor 1 (O1=1),
//      retornando ao estado M1_ON. O ciclo se repete indefinidamente.
//
// 3. COMANDOS DE PARADA:
//    - Os comandos STOP (I2) ou RESET (I3) interrompem o ciclo imediatamente,
//      desligando ambos os motores e retornando o sistema ao estado IDLE.
//
// 4. MODO DE TESTE:
//    - A entrada I4, lida em nível, seleciona o tempo de acionamento T:
//      - I4 = 0: Modo Normal (T = 30 segundos).
//      - I4 = 1: Modo de Teste (T = 3 segundos).
//    - Esta seleção pode ser alterada a qualquer momento durante a operação.
//
// 5. SAÍDAS DE SINALIZAÇÃO:
//    - O3: Pisca a 1 Hz para indicar que o temporizador está ativo (somente nos
//          estados M1_ON ou M2_ON).
//    - O4: Fica em nível alto durante todo o ciclo de operação (M1_ON ou M2_ON).
//    - O5: Pisca continuamente a ~2 Hz (Heartbeat) para indicar que o sistema
//          está energizado e funcional.
//
// OBSERVAÇÕES DE IMPLEMENTAÇÃO:
//   - A FSM segue o modelo clássico de duas lógicas (próximo estado combinacional e
//     registro de estado sequencial) para robustez.
//   - As entradas não possuem debounce, conforme premissa do enunciado.
//   - A troca de T (30s/3s) via I4 é instantânea, afetando o alvo do temporizador
//     que está em curso.
// =======================================================================================


// motores_alternados.sv — implementação robusta (compatível com iverilog -g2012)
`timescale 1us/1us

module motores_alternados #(
  parameter int CLK_HZ     = 100_000,
  parameter int T_NORMAL_S = 5,
  parameter int T_TEST_S   = 2
)(
  input  logic clk,
  input  logic I1,   // START
  input  logic I2,   // STOP
  input  logic I3,   // RESET assíncrono
  input  logic I4,   // Toggle modo teste (aplicado só no PRÓXIMO ciclo)
  input  logic I5,   // (não usado aqui)
  output logic O1,   // Motor 1
  output logic O2,   // Motor 2
  output logic O3,   // Crono pisca 1 Hz quando em ciclo
  output logic O4,   // EmCiclo (RUN)
  output logic O5    // Heartbeat (~2 Hz em IDLE, ~1 Hz em RUN)
);

  // -------------------- Estado --------------------
  typedef enum logic [1:0] {IDLE=2'd0, M1_ON=2'd1, M2_ON=2'd2} state_t;
  state_t state, state_n;

  // -------------------- Divisores --------------------
  // 1 Hz
  localparam int CNT1W = $clog2(CLK_HZ);
  logic [CNT1W-1:0] cnt_1hz;
  wire tick_1hz = (cnt_1hz == CLK_HZ-1);

  always @(posedge clk or posedge I3) begin
    if (I3) cnt_1hz <= '0;
    else if (tick_1hz) cnt_1hz <= '0;
    else cnt_1hz <= cnt_1hz + 1'b1;
  end

  // ~2 Hz para heartbeat em IDLE (toggle a cada 0,25s * 2 bordas = ~2Hz)
  // Vamos gerar 2 frequências a partir de CLK_HZ para O5
  localparam int CNT2W = $clog2(CLK_HZ/2);
  logic [CNT2W-1:0] cnt_hb;
  always @(posedge clk or posedge I3) begin
    if (I3) cnt_hb <= '0;
    else if (cnt_hb == (CLK_HZ/2)-1) cnt_hb <= '0;
    else cnt_hb <= cnt_hb + 1'b1;
  end

  // -------------------- Contador de segundos + alvo --------------------
  logic [15:0] sec_count;
  logic [15:0] target_s;

  // Modo de teste "latched" para PRÓXIMO ciclo. I4 alterna o modo quando pulsado.
  logic test_mode;
  logic I4_q;
  always @(posedge clk or posedge I3) begin
    if (I3) begin
      test_mode <= 1'b0;
      I4_q      <= 1'b0;
    end else begin
      I4_q <= I4;
      if (I4 && !I4_q) begin
        test_mode <= ~test_mode; // toggle no pulso
      end
    end
  end

  // Latche do target_s ao ENTRAR em cada ciclo (M1_ON/M2_ON)
  wire enter_M1 = (state_n==M1_ON) && (state!=M1_ON);
  wire enter_M2 = (state_n==M2_ON) && (state!=M2_ON);

  always @(posedge clk or posedge I3) begin
    if (I3) begin
      target_s <= T_NORMAL_S;
    end else if (enter_M1 || enter_M2) begin
      target_s <= (test_mode) ? T_TEST_S : T_NORMAL_S;
    end
  end

  // Contador de segundos enquanto RUN
  always @(posedge clk or posedge I3) begin
    if (I3) begin
      sec_count <= 16'd0;
    end else if (state_n==IDLE) begin
      sec_count <= 16'd0;
    end else if (tick_1hz) begin
      sec_count <= sec_count + 16'd1;
    end
  end

  // -------------------- Próximo estado --------------------
  always @* begin
    state_n = state;
    case (state)
      IDLE: begin
        if (I1) state_n = M1_ON; // START
      end
      M1_ON: begin
        if (I2) state_n = IDLE;         // STOP
        else if (I3) state_n = IDLE;    // RESET
        else if (sec_count >= target_s) state_n = M2_ON; // troca
      end
      M2_ON: begin
        if (I2) state_n = IDLE;
        else if (I3) state_n = IDLE;
        else if (sec_count >= target_s) state_n = M1_ON;
      end
      default: state_n = IDLE;
    endcase
  end

  // -------------------- Registro de estado --------------------
  always @(posedge clk or posedge I3) begin
    if (I3) state <= IDLE;
    else state <= state_n;
  end

  // -------------------- Saídas --------------------
  assign O1 = (state == M1_ON);
  assign O2 = (state == M2_ON);
  assign O4 = (state == M1_ON) || (state == M2_ON);

  // O3 pisca 1Hz quando em ciclo (use o tick_1hz para toggle controlado)
  logic o3_ff;
  always @(posedge clk or posedge I3) begin
    if (I3) o3_ff <= 1'b0;
    else if (O4 && tick_1hz) o3_ff <= ~o3_ff;
    else if (!O4) o3_ff <= 1'b0;
  end
  assign O3 = o3_ff;

  // O5 Heartbeat: ~2Hz em IDLE, ~1Hz em RUN
  logic hb_run, hb_idle;
  // 1Hz: use tick_1hz para alternar
  always @(posedge clk or posedge I3) begin
    if (I3) hb_run <= 1'b0;
    else if (tick_1hz) hb_run <= ~hb_run;
  end
  // ~2Hz: derive de cnt_hb MSB
  assign hb_idle = cnt_hb[$bits(cnt_hb)-1];
  assign O5 = (O4) ? hb_run : hb_idle;

  // Exporte sinais internos (usados pelo TB com -DTB_PEEK)
  // (são wires/regs já definidos com estes nomes)
endmodule
