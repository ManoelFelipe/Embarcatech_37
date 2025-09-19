-- Este arquivo é um Testbench, um código VHDL usado para simular e verificar
-- o funcionamento do componente 'decodificador_3a8'.

-- Inclusão da biblioteca padrão IEEE.
library IEEE;
-- Uso do pacote std_logic_1164.
use IEEE.std_logic_1164.all;

-- A ENTIDADE de um testbench é tipicamente vazia, pois ele não se conecta
-- a nenhum componente de nível superior.
entity tb_decodificador_3a8 is
end tb_decodificador_3a8;

-- A ARQUITETURA define o ambiente de teste.
architecture test of tb_decodificador_3a8 is
    -- Declaração de SINAIS internos que serão usados para conectar ao componente
    -- sob teste. Eles simulam os fios em uma placa de circuito.
    signal sel_tb : std_logic_vector(2 downto 0); -- Sinal para estimular a entrada 'sel'.
    signal en_tb  : std_logic;                   -- Sinal para estimular a entrada 'en'.
    signal saida_tb : std_logic_vector(7 downto 0); -- Sinal para observar a saída 'saida'.

    -- Declaração do COMPONENTE que será testado. A declaração deve ser
    -- idêntica à entidade do componente original.
    component decodificador_3a8
        port (
            sel   : in std_logic_vector(2 downto 0);
            en    : in std_logic;
            saida : out std_logic_vector(7 downto 0)
        );
    end component;
begin
    -- INSTANCIAÇÃO do componente sob teste (Unit Under Test - UUT).
    -- 'port map' conecta os sinais do testbench às portas do componente.
    uut: decodificador_3a8 port map (
        sel => sel_tb,
        en => en_tb,
        saida => saida_tb
    );

    -- Este PROCESS gera os estímulos (sinais de entrada) ao longo do tempo.
    -- Como não tem lista de sensibilidade, ele executa apenas uma vez no início da simulação.
    stim_proc: process
    begin
        -- 1. Inicia com o decodificador desabilitado para testar essa condição.
        en_tb <= '0';
        sel_tb <= "000";
        wait for 10 ns; -- Aguarda 10 nanosegundos.

        -- 2. Habilita o decodificador.
        en_tb <= '1';

        -- 3. Percorre todas as combinações de entrada possíveis para 'sel',
        --    mudando a cada 10 ns para que a mudança na saída possa ser observada.
        sel_tb <= "000"; wait for 10 ns;
        sel_tb <= "001"; wait for 10 ns;
        sel_tb <= "010"; wait for 10 ns;
        sel_tb <= "011"; wait for 10 ns;
        sel_tb <= "100"; wait for 10 ns;
        sel_tb <= "101"; wait for 10 ns;
        sel_tb <= "110"; wait for 10 ns;
        sel_tb <= "111"; wait for 10 ns;

        -- 4. Desabilita o decodificador novamente no final do teste.
        en_tb <= '0';
        wait for 10 ns;

        -- 'wait;' finaliza o processo de estímulo e suspende a simulação indefinidamente.
        wait;
    end process;
end test;