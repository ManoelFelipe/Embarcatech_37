-- =================================================================================
-- Testbench Rápido: tb_porta_automatica_fast.vhd
-- Autor:     Manoel Felipe Costa Furtado
-- Data:      05/10/2025
-- Arquivo:   tb_porta_automatica_fast.vhd
-- Versão:    1.0
-- Descrição: Testbench abrangente para a FSM `porta_automatica_fsm`. Este roteiro
--            valida todas as principais regras de transição em 7 cenários de teste
--            distintos. Cobre a operação normal (abrir/fechar), o reinício do
--            timer de porta aberta, o acionamento do fechamento manual e a ativação
--            dos timeouts de segurança. Utiliza tempos de simulação curtos para
--            uma execução rápida e eficiente.
-- =================================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity tb_porta_automatica_fast is
end entity;

architecture sim of tb_porta_automatica_fast is
  -- === Sinais para conectar ao componente sob teste (DUT) ===
  -- Entradas
  signal clk               : std_logic := '0';        -- Clock da simulação.
  signal rst_n             : std_logic := '0';        -- Reset ativo em baixo.
  signal sensor            : std_logic := '0';        -- Sensor de presença.
  signal fechar_manual     : std_logic := '0';        -- Botão de fechamento manual.
  signal fim_curso_aberta  : std_logic := '0';        -- Sensor de porta totalmente aberta.
  signal fim_curso_fechada : std_logic := '1';        -- Sensor de porta totalmente fechada (inicia em '1').
  -- Saídas
  signal motor_abrir       : std_logic;               -- Comando para motor de abrir.
  signal motor_fechar      : std_logic;               -- Comando para motor de fechar.
  -- Sinal de depuração para observar o estado interno da FSM.
  signal estado_dbg        : string(1 to 8);

  -- Constante para o período do clock da simulação (1ms = 1KHz).
  constant CLK_PERIOD : time := 1 ms;

begin
  -- Geração de Clock contínuo.
  clk <= not clk after CLK_PERIOD/2;

  -- === Instanciação do DUT (Design Under Test) ===
  -- Conecta a FSM ao testbench. Os genéricos são configurados com valores
  -- baixos para acelerar a simulação dos temporizadores.
  dut: entity work.porta_automatica_fsm
    generic map(
      G_CLK_FREQ     => 1000, -- Frequência de 1kHz, consistente com CLK_PERIOD de 1ms.
      G_T_ABERTA_MS  => 1000, -- Tempo de porta aberta configurado para 1s.
      G_T_TIMEOUT_S  => 2     -- Timeouts de segurança configurados para 2s.
    )
    port map(
      clk               => clk,
      rst_n             => rst_n,
      sensor            => sensor,
      fechar_manual     => fechar_manual,
      fim_curso_aberta  => fim_curso_aberta,
      fim_curso_fechada => fim_curso_fechada,
      motor_abrir       => motor_abrir,
      motor_fechar      => motor_fechar,
      estado_debug      => estado_dbg
    );

  -- === Processo de Estímulo (Roteiro de Teste) ===
  -- Este processo descreve a sequência de eventos para testar a FSM.
  stim: process
  begin
    -- ETAPA 1: Reset inicial do sistema.
    -- Garante que a FSM comece no estado FECHADA.
    report "INICIO: Aplicando reset...";
    rst_n <= '0';
    wait for 5 ms;
    rst_n <= '1';
    wait for 5 ms;

    -- CENÁRIO 1: Testa a transição FECHADA -> ABRINDO.
    -- Simula a detecção de presença quando a porta está fechada.
    report "C1: Presenca detectada -> ABRINDO";
    sensor <= '1';
    wait for 20 ms; -- Mantém o sensor ativo por 20 ciclos.
    sensor <= '0';
    wait for 50 ms; -- Aguarda tempo suficiente para a FSM processar e entrar em ABRINDO.
    assert motor_abrir = '1' report "C1: FALHA - Nao ativou motor para ABRIR" severity error;

    -- CENÁRIO 2: Testa a transição ABRINDO -> ABERTA.
    -- Simula a porta atingindo o fim de curso de abertura.
    report "C2: Fim de curso de abertura atingido -> ABERTA";
    fim_curso_fechada <= '0'; -- A porta não está mais fisicamente fechada.
    wait for 200 ms;          -- Simula o tempo de percurso da porta.
    fim_curso_aberta <= '1';  -- Ativa o sensor de porta totalmente aberta.
    wait for 5 ms;            -- Aguarda a FSM processar o fim de curso.
    assert (motor_abrir='0' and motor_fechar='0') report "C2: FALHA - Motores nao desligaram em ABERTA" severity error;

    -- CENÁRIO 3: Testa se a presença no estado ABERTA reinicia o timer.
    -- Verifica se a porta permanece aberta se alguém reaparecer.
    report "C3: Presenca em ABERTA deve reiniciar o timer de T_ABERTA";
    wait for 400 ms;  -- Espera 40% do tempo de porta aberta (1000ms).
    sensor <= '1';    -- Simula uma nova presença.
    wait for 200 ms;
    sensor <= '0';
    wait for 700 ms;  -- Espera mais 70% do tempo. Se o timer não reiniciou, a porta fecharia aqui (em 400+700=1100ms > 1000ms).
    assert motor_fechar='0' report "C3: FALHA - Porta fechou, timer nao reiniciou" severity error;

    -- CENÁRIO 4: Testa a transição ABERTA -> FECHANDO por tempo expirado.
    -- Verifica o fechamento automático após o tempo de espera sem presença.
    report "C4: Expiracao de T_ABERTA -> FECHANDO";
    wait for 350 ms; -- Completa o 1 segundo de espera (700ms + 350ms > 1000ms).
    assert motor_fechar='1' report "C4: FALHA - Nao ativou motor para FECHAR apos T_ABERTA" severity error;

    -- CENÁRIO 5: Testa a transição FECHANDO -> FECHADA.
    -- Simula a porta atingindo o fim de curso de fechamento.
    report "C5: Fim de curso de fechamento atingido -> FECHADA";
    wait for 300 ms;          -- Simula o tempo de percurso.
    fim_curso_aberta  <= '0'; -- A porta não está mais fisicamente aberta.
    fim_curso_fechada <= '1'; -- Ativa o sensor de porta fechada.
    wait for 5 ms;
    assert motor_fechar='0' report "C5: FALHA - Nao desligou motor em FECHADA" severity error;

    -- CENÁRIO 6: Testa a transição ABERTA -> FECHANDO por comando manual.
    report "C6: Acionamento do fechamento manual a partir de ABERTA";
    -- Primeiro, reabre a porta para preparar o cenário.
    sensor <= '1'; wait for 10 ms; sensor <= '0';
    wait for 50 ms;
    fim_curso_fechada <= '0';
    wait for 150 ms;
    fim_curso_aberta <= '1';
    wait for 10 ms; -- Confirma que está no estado ABERTA (motores desligados).
    -- Agora, aciona o comando manual.
    fechar_manual <= '1'; wait for 5 ms; -- Simula um pulso no botão.
    fechar_manual <= '0';
    wait for 10 ms;
    assert motor_fechar='1' report "C6: FALHA - Fechamento manual nao iniciou" severity error;
    -- Finaliza o ciclo de fechamento para os próximos testes.
    wait for 300 ms;
    fim_curso_aberta <= '0'; fim_curso_fechada <= '1';
    wait for 10 ms;

    -- CENÁRIO 7: Testa os timeouts de segurança.
    -- Parte A: Timeout durante a abertura.
    report "C7a: Timeout de seguranca em ABRINDO";
    -- Força um ciclo de abertura sem nunca ativar o fim de curso `fim_curso_aberta`.
    sensor <= '1'; wait for 10 ms; sensor <= '0';
    fim_curso_fechada <= '0';
    -- Espera 2100ms. O timeout está configurado para 2s (2000ms), então o motor deve desligar.
    wait for 2100 ms;
    assert (motor_abrir='0' and motor_fechar='0') report "C7a: FALHA - Timeout de ABRINDO nao desligou o motor" severity error;
    
    -- Parte B: Timeout durante o fechamento.
    report "C7b: Timeout de seguranca em FECHANDO";
    -- Força um ciclo de fechamento sem nunca ativar o fim de curso `fim_curso_fechada`.
    -- A porta está "aberta" pelo timeout anterior, então podemos mandar fechar.
    fechar_manual <= '1'; wait for 5 ms; fechar_manual <= '0';
    -- Espera 2100ms, novamente excedendo o timeout de 2s.
    wait for 2100 ms;
    assert motor_fechar='0' report "C7b: FALHA - Timeout de FECHANDO nao desligou o motor" severity error;
    fim_curso_fechada <= '1'; -- Restaura a condição inicial para o fim da simulação.

    report "=== FIM: Todos os cenarios foram executados com sucesso ===";
    wait; -- Pausa a simulação indefinidamente.
  end process;
end architecture;