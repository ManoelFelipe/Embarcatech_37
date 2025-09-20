-- Atividade: Unidade 05, Capítulo 01
-- Autor:     Manoel Felipe Costa Furtado
-- Data:      21/09/2025
-- Arquivo:   unid_05_cap_01_tb.vhd
-- Versão:    1.0
-- Descrição: Testbench para o registrador paralelo de 8 bits (unid_05_cap_01).
--            Este código gera os sinais de clock e de dados para estimular
--            o componente sob teste e permitir a verificação de seu
--            funcionamento em um simulador.
--------------------------------------------------------------------------------

-- Biblioteca padrão IEEE.
-- A cláusula USE importa o pacote std_logic_1164, que define os tipos
-- STD_LOGIC e STD_LOGIC_VECTOR, essenciais para a modelagem de hardware.
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

-- A entidade de um testbench é tipicamente vazia, pois ele não possui
-- portas de entrada ou saída. Ele é o ambiente de simulação de mais alto nível.
ENTITY unid_05_cap_01_tb IS
END ENTITY unid_05_cap_01_tb;

-- Arquitetura de simulação para o testbench.
ARCHITECTURE simulation OF unid_05_cap_01_tb IS
    -- 1. Declaração do Componente Sob Teste (DUT - Device Under Test)
    -- A assinatura do componente deve ser idêntica à da sua entidade.
    COMPONENT unid_05_cap_01
        PORT (
            CLK : IN  STD_LOGIC;
            D   : IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
            Q   : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
        );
    END COMPONENT;

    -- 2. Sinais (Signals) do Testbench
    -- Sinais internos usados para conectar ao DUT, gerar estímulos e observar saídas.
    SIGNAL s_clk : STD_LOGIC := '0'; -- Sinal para gerar o clock
    SIGNAL s_d   : STD_LOGIC_VECTOR(7 DOWNTO 0) := (OTHERS => '0'); -- Sinal para os dados de entrada (DUT)
    SIGNAL s_q   : STD_LOGIC_VECTOR(7 DOWNTO 0); -- Sinal para observar a saída (DUT)

    -- 3. Constantes (Constants)
    -- Usado para definir parâmetros fixos, facilitando a manutenção do código.
    CONSTANT CLK_PERIOD : TIME := 10 ns; -- Define o período do clock em 10 ns (Frequência = 100 MHz)

BEGIN
    -- 4. Instanciação do Componente Sob Teste (DUT)
    -- Aqui, criamos uma instância do nosso registrador e conectamos suas portas
    -- (CLK, D, Q) aos sinais do testbench (s_clk, s_d, s_q) usando o mapeamento de portas.
    dut_instance : unid_05_cap_01
        PORT MAP (
            CLK => s_clk,
            D   => s_d,
            Q   => s_q
        );

    -- 5. Processo de Geração de Clock
    -- Este processo executa de forma independente e contínua para gerar um sinal
    -- de clock com o período definido em CLK_PERIOD.
    clk_process : PROCESS
    BEGIN
        s_clk <= '0';
        WAIT FOR CLK_PERIOD / 2; -- Clock fica em nível baixo por 5 ns
        s_clk <= '1';
        WAIT FOR CLK_PERIOD / 2; -- Clock fica em nível alto por 5 ns
    END PROCESS clk_process;

    -- 6. Processo de Geração de Estímulos
    -- Este processo gera a sequência de valores de entrada para testar
    -- o comportamento do registrador em diferentes cenários.
    stimulus_process : PROCESS
    BEGIN
        -- Aguarda 1 ns no início para não coincidir com o tempo 0.
        WAIT FOR 1 ns;

        -- Teste 1: Carrega o valor "todos em 1".
        s_d <= "11111111";
        WAIT UNTIL rising_edge(s_clk);
        -- Aguarda a próxima borda de subida do clock (em t=10ns).
        -- Após esta linha, espera-se que s_q seja atualizado para "11111111".

        -- Teste 2: Carrega o valor "todos em 0".
        s_d <= "00000000";
        WAIT UNTIL rising_edge(s_clk);
        -- Aguarda a próxima borda de subida do clock (em t=20ns).
        -- Após esta linha, espera-se que s_q seja atualizado para "00000000".

        -- Teste 3: Carrega um padrão alternado (par).
        s_d <= "10101010";
        WAIT UNTIL rising_edge(s_clk);
        -- Aguarda a próxima borda de subida do clock (em t=30ns).
        -- Após esta linha, espera-se que s_q seja atualizado para "10101010".

        -- Teste 4: Carrega um padrão alternado (ímpar).
        s_d <= "01010101";
        WAIT UNTIL rising_edge(s_clk);
        -- Aguarda a próxima borda de subida do clock (em t=40ns).
        -- Após esta linha, espera-se que s_q seja atualizado para "01010101".

        -- Teste 5: Carrega a metade alta dos bits.
        s_d <= "11110000";
        WAIT UNTIL rising_edge(s_clk);
        -- Aguarda a próxima borda de subida do clock (em t=50ns).
        -- Após esta linha, espera-se que s_q seja atualizado para "11110000".

        -- Teste 6: Carrega um valor assimétrico para teste.
        s_d <= "11001001";
        WAIT UNTIL rising_edge(s_clk);
        -- Aguarda a próxima borda de subida do clock (em t=60ns).
        -- Após esta linha, espera-se que s_q seja atualizado para "11001001".

        -- Fim dos estímulos.
        -- A instrução 'WAIT;' suspende este processo indefinidamente. [cite: 31]
        -- Isso impede que o processo de estímulos reinicie. A simulação
        -- irá parar quando atingir o '--stop-time' definido no comando de execução. [cite: 31]
        WAIT;
    END PROCESS stimulus_process;

END ARCHITECTURE simulation;