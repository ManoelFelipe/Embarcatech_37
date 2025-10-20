// ============================================================================
// Testbench — VARIANTE "TROCA INSTANTÂNEA" (I4 alterna M1/M2 imediatamente)
// Arquivo: tb_motores_alternados_instant.sv
// ============================================================================
`timescale 1ns/1ps

module tb_motores_alternados_instant;

    localparam int unsigned CLK_HZ_SIM       = 100_000; // 100 kHz
    localparam int unsigned T_NORMAL_S_SIM   = 5;
    localparam int unsigned T_TEST_S_SIM     = 2;
    localparam int unsigned DB_MS_BTN_SIM    = 2;
    localparam int unsigned DB_MS_TESTLV_SIM = 2;

    logic clk;
    initial begin
        clk = 1'b0;
        forever #(5_000) clk = ~clk;
    end

    logic I1, I2, I3, I4, I5;
    wire  O1, O2, O3, O4, O5;

    motores_alternados_instant #(
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

    task automatic press_for_ms(output logic sig, int ms);
        int cycles = (CLK_HZ_SIM/1000) * ms;
        sig = 1'b1;
        repeat (cycles) @(posedge clk);
        sig = 1'b0;
        repeat (CLK_HZ_SIM/1000) @(posedge clk);
    endtask

    initial begin
        I1=0; I2=0; I3=0; I4=0; I5=0;
        repeat (20) @(posedge clk);

        // START
        press_for_ms(I1, 5);
        // anda alguns segundos no normal
        repeat (CLK_HZ_SIM*3) @(posedge clk);

        // Alterna INSTANTE ao mudar I4 (0->1)
        I4 = 1;
        repeat (CLK_HZ_SIM*1) @(posedge clk);

        // Alterna INSTANTE ao mudar I4 (1->0)
        I4 = 0;
        repeat (CLK_HZ_SIM*1) @(posedge clk);

        // Deixa rodar e alternar por tempo
        repeat (CLK_HZ_SIM*8) @(posedge clk);

        // STOP
        press_for_ms(I2, 5);
        repeat (CLK_HZ_SIM*2) @(posedge clk);

        // START novamente
        press_for_ms(I1, 5);
        repeat (CLK_HZ_SIM*2) @(posedge clk);

        // RESET
        press_for_ms(I3, 5);

        repeat (CLK_HZ_SIM*2) @(posedge clk);
        $finish;
    end

endmodule
