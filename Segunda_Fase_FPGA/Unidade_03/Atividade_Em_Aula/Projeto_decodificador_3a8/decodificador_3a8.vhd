-- Este arquivo descreve um decodificador binário de 3 para 8.
-- Ele converte uma entrada binária de 3 bits em uma saída de 8 bits,
-- onde apenas um bit de saída é ativado por vez.
-- Possui também uma porta de habilitação (enable).

-- Inclusão da biblioteca padrão IEEE.
library IEEE;
-- Uso do pacote std_logic_1164, que define os tipos de dados padrão como std_logic.
use IEEE.std_logic_1164.all;

-- A ENTIDADE define a interface (as portas de entrada e saída) do componente.
entity decodificador_3a8 is
    port (
        -- 'sel' (select) é a entrada de 3 bits que determina qual saída será ativada.
        sel   : in std_logic_vector(2 downto 0);
        -- 'en' (enable) é a entrada de 1 bit que habilita ou desabilita o decodificador.
        en    : in std_logic;
        -- 'saida' é o barramento de 8 bits onde o resultado da decodificação aparece.
        saida : out std_logic_vector(7 downto 0)
    );
end decodificador_3a8;

-- A ARQUITETURA descreve o funcionamento interno do componente.
-- O estilo 'behavioral' ou comportamental descreve o comportamento através de processos e lógica sequencial.
architecture behavioral of decodificador_3a8 is
begin
    -- Um PROCESS é um bloco de código que é executado quando os sinais em sua
    -- lista de sensibilidade (neste caso, 'sel' e 'en') mudam de valor.
    process(sel, en)
    begin
        -- Verifica se o pino de habilitação 'en' está em nível lógico alto ('1').
        if en = '1' then
            -- Se estiver habilitado, um CASE é usado para avaliar o valor da entrada 'sel'.
            case sel is
                -- Para cada valor possível de 'sel', uma única saída correspondente é ativada.
                when "000" => saida <= "00000001"; -- Ativa a saída 0
                when "001" => saida <= "00000010"; -- Ativa a saída 1
                when "010" => saida <= "00000100"; -- Ativa a saída 2
                when "011" => saida <= "00001000"; -- Ativa a saída 3
                when "100" => saida <= "00010000"; -- Ativa a saída 4
                when "101" => saida <= "00100000"; -- Ativa a saída 5
                when "110" => saida <= "01000000"; -- Ativa a saída 6
                when "111" => saida <= "10000000"; -- Ativa a saída 7
                -- 'when others' é uma cláusula de segurança para cobrir quaisquer outros
                -- casos possíveis (útil para tipos de dados com mais estados, como std_ulogic).
                when others => saida <= (others => '0');
            end case;
        else
            -- Se o decodificador não estiver habilitado (en = '0'), todas as saídas
            -- são forçadas para o nível lógico baixo ('0').
            saida <= (others => '0');
        end if;
    end process;
end behavioral;