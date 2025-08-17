module tb_Atividade_Uni_02_Cap_01;
logic a, b, c, y;
alarme uut(
    .A(a),
    .B(b),
    .C(c),
    .Y(y)
);

initial 
    begin
        $dumpfile("waveform.vcd");
        $dumpvars(0, tb_Atividade_Uni_02_Cap_01);
        //2^3 = 8
        a = 0; b = 0; c = 0;
        #5
        a = 0; b = 0; c = 1;
        #5
        a = 0; b = 1; c = 0;
        #5
        a = 0; b = 1; c = 1;
        #5
        a = 1; b = 0; c = 0;
        #5
        a = 1; b = 0; c = 1;
        #5
        a = 1; b = 1; c = 0;
        #5
        a = 1; b = 1; c = 1;
        #5;
        $finish;
    end
endmodule

// iverilog -g2012 -o tb_Atividade_Uni_02_Cap_01 Atividade_Uni_02_Cap_01.sv tb_Atividade_Uni_02_Cap_01.sv
// vvp tb_Atividade_Uni_02_Cap_01
// gtkwave waveform.vcd

