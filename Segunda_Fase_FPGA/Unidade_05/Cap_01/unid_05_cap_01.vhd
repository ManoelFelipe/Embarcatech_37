-- Atividade: Unidade 05, Capítulo 01
-- Autor:     Manoel Felipe Costa Furtado
-- Data:      21/09/2025
-- Arquivo:   unid_05_cap_01.vhd
-- Versão:    1.0
-- Descrição: Implementa um registrador paralelo de 8 bits utilizando o conceito
--            de flip-flops tipo D. A operação é síncrona, baseada na borda
--            de subida do sinal de clock.
--------------------------------------------------------------------------------

-- Biblioteca padrão IEEE.
-- A cláusula USE importa o pacote std_logic_1164, que define os tipos
-- STD_LOGIC e STD_LOGIC_VECTOR, essenciais para a modelagem de hardware.
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

-- ENTIDADE (ENTITY)
-- Define a interface pública do nosso componente, ou seja, suas portas de
-- entrada e saída. Funciona como um "contrato" que descreve como outros
-- circuitos podem se conectar a este.
ENTITY unid_05_cap_01 IS
    PORT (
        -- Porta de entrada do clock. É o sinal que dita o ritmo das operações.
        CLK : IN  STD_LOGIC;
        -- Barramento de entrada de dados com 8 bits.
        D   : IN  STD_LOGIC_VECTOR(7 DOWNTO 0);
        -- Barramento de saída de dados com 8 bits, onde o valor armazenado é lido.
        Q   : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
    );
END ENTITY unid_05_cap_01;

-- ARQUITETURA (ARCHITECTURE)
-- Descreve o comportamento interno e a estrutura lógica do componente
-- definido na entidade 'unid_05_cap_01'.
ARCHITECTURE behavioral OF unid_05_cap_01 IS
BEGIN
    -- PROCESSO (PROCESS) SÍNCRONO
    -- Um processo é um bloco de código sequencial que é sensível a mudanças
    -- nos sinais de sua lista de sensibilidade (neste caso, apenas 'CLK').
    -- Por ser sensível apenas ao clock, ele modela um comportamento síncrono.
    process(CLK)
    BEGIN
        -- Detecção de borda de subida do clock.
        -- A função 'rising_edge()' retorna verdadeiro apenas no instante em
        -- que o sinal CLK transita de '0' para '1'.
        -- Esta é a condição que dispara o armazenamento de dados.
        IF rising_edge(CLK) THEN
            -- Atribuição síncrona.
            -- O valor presente no barramento de entrada 'D' é amostrado e
            -- atribuído ao barramento de saída 'Q'. Esta operação simula
            -- o comportamento de 8 flip-flops tipo D operando em paralelo.
            Q <= D;
        END IF;
    END PROCESS;

END ARCHITECTURE behavioral;