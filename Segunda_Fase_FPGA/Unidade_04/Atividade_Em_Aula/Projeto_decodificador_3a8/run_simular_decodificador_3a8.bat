@echo off

ghdl -a decodificador_3a8.vhd
ghdl -a tb_decodificador_3a8.vhd
ghdl -e tb_decodificador_3a8
ghdl -r tb_decodificador_3a8 --vcd=decodificador.vcd

gtkwave decodificador.vcd

del *.cf
pause