// ======================================================================
// Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
// Data: 06/11/2025
// Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
//
// Descrição:
// Módulo Verilog 'transistor_sensor_test'.
//
// Este projeto testa um circuito de sensor de umidade baseado em transistor
// NPN, conectado a uma placa FPGA Colorlight i9.
//
// O sensor é conectado à base do transistor. Quando o sensor está
// molhado, ele conduz, satura o transistor, e o pino do coletor 
// (sensor_col_n) é drenado para 0V (GND).
//
// Lógica do Circuito:
// 1. Sensor SECO   -> Transistor em CORTE   -> Coletor = 3.3V (Lógica 1)
// 2. Sensor MOLHADO -> Transistor SATURADO -> Coletor = 0V   (Lógica 0)
//
// O módulo inverte esse sinal (ativo-baixo) e acende um LED (ativo-alto)
// quando o sensor está molhado.
// ======================================================================

module transistor_sensor_test (
    // ENTRADA: Leitura do coletor do transistor 
    // Esta entrada é ATIVO-BAIXO. 
    // 0 = Sensor Molhado (transistor saturado)
    // 1 = Sensor Seco (transistor em corte)
    input wire sensor_col_n, // Conectado ao pino C1 no .lpf

    // SAÍDA: LED indicador na placa 
    // Esta saída é ATIVO-ALTO.
    // 1 = LED Aceso
    // 0 = LED Apagado
    output wire led_d1 // Conectado ao pino D1 no .lpf
);

    // Sinal interno (fio) para representar o estado "molhado" em lógica
    // positiva (ativo-alto).
    wire sensor_wet;

    // LÓGICA COMBINACIONAL: Inversão da entrada [cite: 10]
    // A função 'assign' cria uma lógica combinacional contínua.
    // O til (~) é o operador NOT (inversão).
    // Se sensor_col_n = 0 (Molhado), sensor_wet se torna 1.
    // Se sensor_col_n = 1 (Seco),   sensor_wet se torna 0.
    assign sensor_wet = ~sensor_col_n;

    // LÓGICA COMBINACIONAL: Acionamento do LED [cite: 11]
    // O LED (led_d1) é diretamente conectado ao estado 'sensor_wet'.
    // O LED acenderá (led_d1 = 1) quando sensor_wet for 1 (ou seja, molhado).
    assign led_d1 = sensor_wet;

    // Comentário original do SystemVerilog mantido para referência futura: [cite: 12]
    // Se quiser também expor um comando de bomba depois:
    // output logic pump_on
    // assign pump_on = sensor_wet;

endmodule