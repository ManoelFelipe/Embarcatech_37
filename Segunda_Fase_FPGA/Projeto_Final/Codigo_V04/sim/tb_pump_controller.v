/* ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 * ======================================================================
 */


// tb/tb_pump_controller.v — Testbench para FSM Robusta v4
`timescale 1ns/1ps

module tb_pump_controller;

    // Clock/reset
    reg clk = 0;
    reg rst_n = 0;

    // Entradas
    reg [2:0] lvl_inf_in;
    reg [2:0] lvl_sup_in;
    reg       en_auto;

    // Saídas
    wire pump_on;
    wire solenoid_open;
    wire led_green, led_red;
    wire fault;
    wire [6:0] seg_inf, seg_sup;

    // DUT
    pump_controller #(
        .CLK_HZ(1_000_000),        // clock reduzido p/ simulação
        .FILTER_MS(2),
        .FAULT_RECOVERY_MS(10),
        .INVERT_LEVEL_CODE(0)      // ajustar conforme cenário
    ) dut (
        .clk(clk), .rst_n(rst_n),
        .lvl_inf_in(lvl_inf_in),
        .lvl_sup_in(lvl_sup_in),
        .en_auto(en_auto),
        .pump_on(pump_on),
        .solenoid_open(solenoid_open),
        .led_green(led_green), .led_red(led_red),
        .fault(fault),
        .seg_inf(seg_inf), .seg_sup(seg_sup)
    );

    // Clock: 10ns -> 100MHz (mas param CLK_HZ=1MHz no DUT p/ facilitar tempos)
    always #5 clk = ~clk;

    // VCD
    initial begin
        $dumpfile("wave.vcd");
        $dumpvars(0, tb_pump_controller);
    end

    // Tarefas auxiliares
    task apply_levels(input [2:0] inf, input [2:0] sup, input integer us);
        begin
            lvl_inf_in = inf;
            lvl_sup_in = sup;
            #(us*1000); // microssegundos
        end
    endtask

    // Asserts simples
    task assert_eq(input string tag, input bit cond);
        if (!cond) begin
            $display("[FAIL] %s @%0t", tag, $time);
            $fatal;
        end else begin
            $display("[ OK ] %s @%0t", tag, $time);
        end
    endtask

    // Estímulos
    initial begin
        en_auto   = 0;
        lvl_inf_in= 3'd2;
        lvl_sup_in= 3'd2;

        // Reset
        #100; rst_n = 1;

        // --------- (a) Partida normal ---------
        en_auto = 1;
        apply_levels(3'd3, 3'd1, 50); // inf=75%, sup=25%  -> deve armar PUMPING
        #10000;
        assert_eq("PUMP ON após partida", pump_on==1);

        // Manter até sup≥75% -> parar
        apply_levels(3'd3, 3'd3, 50);
        #10000;
        assert_eq("PUMP OFF após sup≥75%", pump_on==0);

        // --------- (b) Parada por inf≤25% ---------
        en_auto = 1;
        apply_levels(3'd3, 3'd1, 50); #10000;
        assert_eq("PUMP ON (novamente)", pump_on==1);
        apply_levels(3'd1, 3'd2, 50); #10000;
        assert_eq("PUMP OFF por inf≤25%", pump_on==0);

        // --------- (c) Leituras inválidas -> FAULT ---------
        apply_levels(3'd5, 3'd2, 10); // inválido
        #1000;
        assert_eq("FAULT=1 em leitura inválida", fault==1);

        // Recuperação: leituras válidas por janela
        apply_levels(3'd2, 3'd2, 50);
        #50000; // aguarda RECOV_TICKS aproximado
        assert_eq("FAULT=0 após recuperação", fault==0);

        // --------- (d) Modo manual ---------
        en_auto = 1;
        apply_levels(3'd3, 3'd1, 50); #10000;
        assert_eq("PUMP ON antes do manual", pump_on==1);
        en_auto = 0; #1000;
        assert_eq("PUMP OFF forçado no manual", pump_on==0);

        // --------- (e) Debounce: ruído ---------
        en_auto = 1;
        // alterna leituras breves que não devem mudar estado por conta do filtro
        repeat (5) begin
            apply_levels(3'd3, 3'd1, 1); // curto
            apply_levels(3'd2, 3'd2, 1); // curto
        end
        // estabiliza em condição de partida
        apply_levels(3'd3, 3'd1, 50); #20000;
        assert_eq("PUMP ON após leituras estáveis", pump_on==1);

        $display("Testbench finalizado com sucesso.");
        $finish;
    end

endmodule
