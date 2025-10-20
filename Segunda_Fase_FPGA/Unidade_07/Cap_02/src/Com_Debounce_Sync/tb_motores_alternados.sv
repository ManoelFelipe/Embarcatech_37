// ============================================================================
// Testbench — motores_alternados (com debounce em I1/I2/I3 e I4 por nível)
// Arquivo: tb_motores_alternados.sv
// Objetivo: Exercitar START/STOP/RESET (pulsos >= debounce), alternância com I4=0/1.
// ============================================================================
`timescale 1ns/1ps

module tb_motores_alternados;

    // Clock de simulação: 100 kHz (período = 10 us)
    localparam int unsigned CLK_HZ_SIM       = 100_000;
    localparam int unsigned T_NORMAL_S_SIM   = 5; // 5 s
    localparam int unsigned T_TEST_S_SIM     = 2; // 2 s
    localparam int unsigned DB_MS_BTN_SIM    = 2; // 2 ms (I1/I2/I3)
    localparam int unsigned DB_MS_TESTLV_SIM = 2; // 2 ms (I4 nível)

    logic clk;
    initial begin
        clk = 1'b0;
        forever #(5_000) clk = ~clk; // 5_000 ns => 10_000 ns por período => 100 kHz
    end

    // I/O
    logic I1, I2, I3, I4, I5;
    wire  O1, O2, O3, O4, O5;

    // DUT com debounce em botões e em I4 (nível)
    motores_alternados #(
        .CLK_HZ       (CLK_HZ_SIM),
        .T_NORMAL_S   (T_NORMAL_S_SIM),
        .T_TEST_S     (T_TEST_S_SIM),
        .DB_MS_BTN    (DB_MS_BTN_SIM),
        .DB_MS_TESTLV (DB_MS_TESTLV_SIM)
    ) dut (
        .clk(clk),
        .I1 (I1),
        .I2 (I2),
        .I3 (I3),
        .I4 (I4),
        .I5 (I5),
        .O1 (O1),
        .O2 (O2),
        .O3 (O3),
        .O4 (O4),
        .O5 (O5)
    );

    // Auxiliar: pressiona botão por 'ms' (>= janela de debounce)
    task automatic press_for_ms(output logic sig, int ms);
        int cycles = (CLK_HZ_SIM/1000) * ms;
        sig = 1'b1;
        repeat (cycles) @(posedge clk);
        sig = 1'b0;
        repeat (CLK_HZ_SIM/1000) @(posedge clk);
    endtask

    // Estímulos
    initial begin
        I1=0; I2=0; I3=0; I4=0; I5=0;
        repeat (20) @(posedge clk);

        // START
        press_for_ms(I1, 5);

        // Normal (5 s)
        repeat (CLK_HZ_SIM*12) @(posedge clk);

        // Entra modo teste (I4=1) — por nível (com debounce)
        I4 = 1;
        repeat (CLK_HZ_SIM*8) @(posedge clk);

        // Volta ao normal (I4=0)
        I4 = 0;
        repeat (CLK_HZ_SIM*6) @(posedge clk);

        // STOP
        press_for_ms(I2, 5);
        repeat (CLK_HZ_SIM*2) @(posedge clk);

        // Novo START
        press_for_ms(I1, 5);
        repeat (CLK_HZ_SIM*3) @(posedge clk);

        // RESET
        press_for_ms(I3, 5);

        repeat (CLK_HZ_SIM*2) @(posedge clk);
        $finish;
    end

endmodule
