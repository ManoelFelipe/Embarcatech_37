-- Atividade: Unidade 04, Capítulo 01
-- Autor:     Manoel Felipe Costa Furtado
-- Data:      21/09/2025
-- Arquivo:   unid_04_cap_01_tb.vhd
-- Versão:    1.0
-- Descrição: Testbench para o módulo "Verificador de Calorias".
--            Este código simula todas as 16 combinações de entrada possíveis
--            para verificar se a saída do circuito corresponde ao esperado.

-------------------------------------------------------------------------------
-- BIBLIOTECAS
-------------------------------------------------------------------------------
LIBRARY ieee;
-- Pacote padrão para os tipos STD_LOGIC.
USE ieee.std_logic_1164.all;
-- Pacote para funções de conversão entre tipos numéricos e STD_LOGIC_VECTOR.
USE ieee.numeric_std.all;

-------------------------------------------------------------------------------
-- ENTIDADE (Testbench)
-------------------------------------------------------------------------------
-- A entidade de um testbench é tradicionalmente vazia, pois ele não possui
-- portas de entrada ou saída para o mundo exterior.
ENTITY unid_04_cap_01_tb IS
END ENTITY unid_04_cap_01_tb;

-------------------------------------------------------------------------------
-- ARQUITETURA (Simulação)
-------------------------------------------------------------------------------
ARCHITECTURE teste OF unid_04_cap_01_tb IS
    -- Declaração do componente que será testado (DUT - Design Under Test).
    -- A estrutura deve ser idêntica à entidade do arquivo de design.
    COMPONENT unid_04_cap_01 IS
        PORT (
            A, B, C, D : IN  STD_LOGIC;
            L          : OUT STD_LOGIC
        );
    END COMPONENT unid_04_cap_01;

    -- Sinais internos do testbench que serão conectados às portas do DUT.
    -- 's_' é um prefixo comum para indicar sinais de simulação.
    SIGNAL s_A, s_B, s_C, s_D : STD_LOGIC := '0';
    SIGNAL s_L                : STD_LOGIC;

BEGIN
    -- Instanciação do Componente sob Teste (UUT).
    -- Conecta os sinais locais do testbench às portas do componente.
    uut : unid_04_cap_01
        PORT MAP(
            A => s_A, B => s_B, C => s_C, D => s_D, L => s_L
        );

    -- Processo de estímulo: responsável por gerar os sinais de entrada ao longo do tempo.
    stimulus_process : PROCESS
        -- Variável local usada para armazenar a representação binária do contador do laço.
        VARIABLE i_vec : STD_LOGIC_VECTOR(3 DOWNTO 0);
    BEGIN
        -- Este laço itera de 0 a 15, cobrindo todas as combinações de 4 bits.
        FOR i IN 0 TO 15 LOOP
            -- 1. Converte o inteiro 'i' para um vetor de 4 bits.
            i_vec := std_logic_vector(to_unsigned(i, 4));

            -- 2. Atribui cada bit do vetor ao sinal de entrada correspondente.
            s_A <= i_vec(3); -- Bit mais significativo
            s_B <= i_vec(2);
            s_C <= i_vec(1);
            s_D <= i_vec(0); -- Bit menos significativo

            -- 3. Aguarda um intervalo de tempo para que o circuito processe as novas
            --    entradas e a saída se estabilize antes da próxima mudança.
            WAIT FOR 10 ns;
        END LOOP;

        -- Após o laço, o processo é suspenso indefinidamente para finalizar a simulação.
        WAIT;
    END PROCESS stimulus_process;

END ARCHITECTURE teste;