-- Atividade: Unidade 04, Capítulo 01
-- Autor:     Manoel Felipe Costa Furtado
-- Data:      21/09/2025
-- Arquivo:   unid_04_cap_01.vhd
-- Versão:    1.0
-- Descrição: Implementa o circuito digital para o "Verificador de Calorias".
--            A saída é ativada quando a soma calórica dos ingredientes
--            ultrapassa 50% do produto original.

-------------------------------------------------------------------------------
-- BIBLIOTECAS
-------------------------------------------------------------------------------
LIBRARY ieee;
-- Usa o pacote std_logic_1164, que define os tipos STD_LOGIC e STD_LOGIC_VECTOR.
USE ieee.std_logic_1164.all;

-------------------------------------------------------------------------------
-- ENTIDADE (Interface Pública do Circuito)
-------------------------------------------------------------------------------
-- A entidade define as portas de entrada e saída do nosso circuito,
-- funcionando como uma "caixa preta" para o mundo exterior.
ENTITY unid_04_cap_01 IS
    PORT (
        -- Entradas que representam a presença de cada ingrediente.
        A, B, C, D : IN  STD_LOGIC;
        -- Saída que aciona a lâmpada (L).
        L          : OUT STD_LOGIC
    );
END ENTITY unid_04_cap_01;

-------------------------------------------------------------------------------
-- ARQUITETURA (Lógica Interna do Circuito)
-------------------------------------------------------------------------------
-- A arquitetura 'logica' descreve como as saídas são calculadas a partir
-- das entradas.
ARCHITECTURE logica OF unid_04_cap_01 IS
BEGIN
    -- Implementa a expressão booleana simplificada via Mapa de Karnaugh.
    -- A lâmpada (L) será '1' (acesa) se a combinação de ingredientes
    -- A(40%), B(30%), C(20%) e D(10%) ultrapassar 50% em valor calórico.
    L <= (A AND B) OR (A AND C) OR (B AND C AND D);

END ARCHITECTURE logica;