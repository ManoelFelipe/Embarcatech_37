/*
 * ======================================================================
 * Autor: Manoel Furtado, Yuri Cândido, Paulo Gomes
 * Data: 06/11/2025
 * Repositório: https://github.com/ManoelFelipe/Embarcatech_37_FPGA
 *
 * Módulo: controlador_caixa_dagua
 * Descrição:
 * Unidade de controle principal (FSM) para o sistema automático de
 * transferência de água.
 * Utiliza LÓGICA DE SENSOR INVERTIDA (0 = Água Presente).
 * ======================================================================
 */
module controlador_caixa_dagua(
    // --- Entradas Globais ---
    input wire clk,                // Clock principal (25MHz do .lpf)
    input wire reset,              // Reset assíncrono (Botão)

    // --- Entradas de Sensores (Lógica Invertida) ---
    input wire [4:0] sensores_inf, // Vetor de 5 sensores da caixa Inferior
    input wire [4:0] sensores_sup, // Vetor de 5 sensores da caixa Superior

    // --- Saídas de Atuadores e Status ---
    output wire bomba,             // Aciona o relé da bomba
    output reg  valvula,           // Aciona a válvula de entrada
    output wire led_vermelho,      // LED de status (Bomba Desligada)
    output wire led_verde,         // LED de status (Bomba Ligada)
    
    // --- Saídas para Displays ---
    output wire [6:0] display_inf, // Barramento para o display inferior
    output wire [6:0] display_sup  // Barramento para o display superior
);

    // Definição dos Estados da FSM (Máquina de Estados Finitos)
    localparam [1:0] S_VAZIA      = 2'b00; // Estado: Caixa inferior vazia, enchendo (Válvula ON)
    localparam [1:0] S_CHEIA      = 2'b01; // Estado: Caixa inferior cheia, aguardando
    localparam [1:0] S_ESVAZIANDO = 2'b10; // Estado: Transferindo água (Bomba ON)
    // Estado 2'b11 é não utilizado/inválido

    // Sinais Internos para a FSM
    reg [1:0] estado_atual, proximo_estado;
    reg bomba_internal; // Sinal interno 'reg' para controlar a bomba

    // === Instanciação dos Decodificadores de Display ===
    
    // Instância para a Caixa Inferior
    decodificador_nivel DECODER_INF (
        .sensores_in(sensores_inf),   // Conecta sensores inferiores
        .display_out(display_inf)     // Conecta ao display inferior
    );
    // Instância para a Caixa Superior
    decodificador_nivel DECODER_SUP (
        .sensores_in(sensores_sup),   // Conecta sensores superiores
        .display_out(display_sup)     // Conecta ao display superior
    );

    // ==================================================================
    // === Lógica da FSM (Separada em 3 blocos) ===
    // ==================================================================
    
    // Bloco 1: Registrador de Estado (Lógica Sequencial)
    // Atualiza o estado atual na borda de subida do clock ou no reset.
    always @(posedge clk or posedge reset) begin
        if (reset) // Reset assíncrono
            estado_atual <= S_VAZIA; // Estado inicial padrão
        else
            estado_atual <= proximo_estado; // Atualização normal no clock
    end

    // Bloco 2: Lógica de Próximo Estado (Lógica Combinacional)
    // Decide qual será o próximo estado com base no estado atual e nas entradas.
    always @(*) begin
        proximo_estado = estado_atual; // Padrão: permanecer no estado atual

        case (estado_atual)
            S_VAZIA: // Estado: Enchendo a caixa inferior
                // Condição: Se sensores 0, 1, 2, 3 estão em '0' (molhados)
                if ((!sensores_inf[3] && !sensores_inf[2] && !sensores_inf[1] && !sensores_inf[0]))
                    proximo_estado = S_CHEIA; // Transição: Caixa ficou cheia
            
            S_CHEIA: // Estado: Caixa inferior cheia, aguardando
                // Condição: Se Sup[0] '0' (molhado) E Sup[1] '1' (seco) -> (Superior está baixo)
                // E Se Inf[3] '0' (molhado) -> (Inferior tem água suficiente)
                if (!sensores_sup[0] && sensores_sup[1] && !sensores_inf[3])
                    proximo_estado = S_ESVAZIANDO; // Transição: Começar a bombear

            S_ESVAZIANDO: // Estado: Bombeando água para cima
                // Condição 1: Se Sup[3] '0' (molhado) -> (Superior está quase cheio)
                // OU
                // Condição 2: Se Inf[0] '0' (molhado) E Inf[1..4] '1' (secos) -> (Inferior está quase vazio)
                if (!sensores_sup[3] || (!sensores_inf[0] && sensores_inf[1] && sensores_inf[2] && sensores_inf[3] && sensores_inf[4]))
                    proximo_estado = S_VAZIA; // Transição: Parar de bombear

            default: // Caso o estado seja inválido (2'b11)
                proximo_estado = S_VAZIA; // Volta para o estado inicial
        endcase
    end

    // Bloco 3: Lógica de Saída (Lógica Combinacional - Estilo Moore)
    // Define as saídas (bomba, valvula) com base APENAS no estado atual.
    always @(*) begin
        // --- DEFINIÇÕES PADRÃO ---
        // **CORREÇÃO**: Garante que saídas estejam desligadas
        // por padrão para evitar a inferência de LATCH.
        bomba_internal = 1'b0; 
        valvula        = 1'b0; 

        case (estado_atual)
            S_VAZIA: begin
                bomba_internal = 1'b0; // Bomba desligada
                valvula        = 1'b0; // Válvula desligada (aguardando)
            end
            
            S_CHEIA: begin
                bomba_internal = 1'b0; // Bomba desligada
                valvula        = 1'b1; // VÁLVULA LIGADA (para encher)
            end
            
            S_ESVAZIANDO: begin
                bomba_internal = 1'b1; // BOMBA LIGADA
                valvula        = 1'b0; // Válvula desligada
            end
            
            // O 'default' (para 2'b11) usa os padrões definidos acima (tudo 0)
        endcase
    end

    // Atribuições contínuas para as saídas físicas
    assign bomba = bomba_internal;          // Saída da bomba espelha o sinal interno
    assign led_verde = bomba_internal;      // LED Verde acende se a bomba está ligada
    assign led_vermelho = !bomba_internal;  // LED Vermelho acende se a bomba está desligada

endmodule