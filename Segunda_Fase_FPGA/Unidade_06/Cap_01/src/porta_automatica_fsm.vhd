-- =================================================================================
-- Atividade: Unidade 06, Capítulo 01
-- Autor:     Manoel Felipe Costa Furtado
-- Data:      05/10/2025
-- Arquivo:   porta_automatica_fsm.vhd
-- Versão:    1.0
-- Descrição: Máquina de estados finitos (FSM) estilo Moore que implementa a lógica
--            de controle para uma porta automática. A FSM gerencia os motores de
--            abertura e fechamento com base em entradas como sensor de presença,
--            sensores de fim de curso (porta totalmente aberta/fechada) e um
--            comando para fechamento manual. Inclui temporizadores de segurança.
-- =================================================================================

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity porta_automatica_fsm is
    generic (
        -- Frequência do clock do sistema em Hertz (Hz).
        G_CLK_FREQ      : integer := 1000; -- Valor para simulação (1 KHz)
        -- Tempo que a porta deve permanecer aberta após a saída do usuário, em milissegundos.
        G_T_ABERTA_MS   : integer := 5000; -- Valor padrão de 5000 ms (5s)
        -- Timeout de segurança em segundos. Se a porta não atingir um fim de curso
        -- (aberta ou fechada) neste tempo, o motor é desligado para evitar danos.
        G_T_TIMEOUT_S   : integer := 10
    );
    port (
        -- Entradas de controle
        clk               : in  std_logic; -- Clock do sistema.
        rst_n             : in  std_logic; -- Reset assíncrono ativo em baixo.
        sensor            : in  std_logic; -- '1' se há presença, '0' caso contrário.
        fechar_manual     : in  std_logic; -- Pulso em '1' para forçar o fechamento.
        fim_curso_aberta  : in  std_logic; -- '1' quando a porta está totalmente aberta.
        fim_curso_fechada : in  std_logic; -- '1' quando a porta está totalmente fechada.

        -- Saídas de acionamento
        motor_abrir       : out std_logic; -- '1' para acionar o motor de abertura.
        motor_fechar      : out std_logic; -- '1' para acionar o motor de fechamento.

        -- Saída de depuração (para simulação)
        -- Permite visualizar o nome do estado atual em um simulador de formas de onda.
        estado_debug      : out string(1 to 8)
    );
end entity porta_automatica_fsm;

architecture rtl of porta_automatica_fsm is

    -- Define os quatro estados possíveis da máquina de estados.
    type t_estado is (FECHADA, ABRINDO, ABERTA, FECHANDO);

    -- Sinais para armazenar o estado atual e o próximo estado calculado.
    signal estado_atual, proximo_estado : t_estado;
    -- Sinal para registrar o estado do ciclo de clock anterior. Usado para detectar
    -- transições (ex: `estado_prev /= estado_atual`), o que é crucial para
    -- inicializar lógicas (como contadores) na entrada de um novo estado.
    signal estado_prev   : t_estado;

    -- Constantes para os limites dos contadores, calculados a partir dos genéricos.
    -- Isso torna o design parametrizável e a lógica mais legível.
    constant C_COUNT_ABERTA  : integer := (G_CLK_FREQ / 1000) * G_T_ABERTA_MS; -- Limite para o tempo em ABERTA.
    constant C_COUNT_TIMEOUT : integer := G_CLK_FREQ * G_T_TIMEOUT_S;       -- Limite para os timeouts de segurança.

    -- Contadores dedicados para cada estado que depende de tempo. Usar contadores
    -- separados evita que a contagem de um estado "vaze" para o outro (carry-over),
    -- tornando o design mais robusto e fácil de depurar.
    signal cnt_abrindo   : integer range 0 to C_COUNT_TIMEOUT := 0; -- Temporizador de segurança para o estado ABRINDO.
    signal cnt_aberta    : integer range 0 to C_COUNT_ABERTA  := 0; -- Temporizador para manter a porta aberta.
    signal cnt_fechando  : integer range 0 to C_COUNT_TIMEOUT := 0; -- Temporizador de segurança para o estado FECHANDO.

    -- Flags que sinalizam quando um temporizador atingiu seu limite.
    -- Simplificam a lógica de transição de estado.
    signal to_abrindo    : std_logic; -- Flag de timeout para ABRINDO.
    signal to_aberta     : std_logic; -- Flag de tempo esgotado para ABERTA.
    signal to_fechando   : std_logic; -- Flag de timeout para FECHANDO.

begin

    -- Processo combinacional para formatar o nome do estado atual para depuração.
    -- Preenche com espaços para manter um tamanho fixo, facilitando a visualização.
    process(estado_atual)
    begin
        case estado_atual is
            when FECHADA  => estado_debug <= "FECHADA ";
            when ABRINDO  => estado_debug <= "ABRINDO ";
            when ABERTA   => estado_debug <= "ABERTA  ";
            when FECHANDO => estado_debug <= "FECHANDO";
        end case;
    end process;

    -- ==============================================================================
    -- PROCESSO 1: REGISTRADOR DE ESTADO (Lógica Síncrona)
    -- ==============================================================================
    -- Este processo representa a "memória" da FSM. Na borda de subida do clock,
    -- ele atualiza o estado atual com o valor calculado pela lógica de próximo
    -- estado. Também armazena o estado antigo em `estado_prev` para detecção de transição.
    process(clk, rst_n)
    begin
        if rst_n = '0' then
            estado_atual <= FECHADA;
            estado_prev  <= FECHADA;
        elsif rising_edge(clk) then
            estado_prev  <= estado_atual;
            estado_atual <= proximo_estado;
        end if;
    end process;

    -- ==============================================================================
    -- PROCESSO 2: LÓGICA DE PRÓXIMO ESTADO (Lógica Combinacional)
    -- ==============================================================================
    -- Este bloco puramente combinacional calcula qual será o próximo estado da
    -- máquina com base no estado atual e nas entradas do sistema.
    process(estado_atual, sensor, fechar_manual, fim_curso_aberta, fim_curso_fechada,
            to_abrindo, to_aberta, to_fechando)
    begin
        proximo_estado <= estado_atual; -- Comportamento padrão: permanecer no estado atual.
        case estado_atual is
            when FECHADA =>
                -- Se a porta está fechada e o sensor detecta presença, o próximo estado é ABRINDO.
                if sensor = '1' then
                    proximo_estado <= ABRINDO;
                end if;
            when ABRINDO =>
                -- Se a porta está abrindo e atinge o fim de curso de abertura OU se o tempo
                -- de segurança estourar, o próximo estado é ABERTA (para parar o motor).
                if fim_curso_aberta = '1' or to_abrindo = '1' then
                    proximo_estado <= ABERTA;
                end if;
            when ABERTA =>
                -- Se o botão de fechamento manual for pressionado, a transição para FECHANDO tem prioridade.
                if fechar_manual = '1' then
                    proximo_estado <= FECHANDO;
                -- Caso contrário, se não houver mais ninguém (sensor='0') e o tempo de espera terminar,
                -- a porta começa a fechar automaticamente.
                elsif sensor = '0' and to_aberta = '1' then
                    proximo_estado <= FECHANDO;
                end if;
            when FECHANDO =>
                -- Se a porta está fechando e atinge o fim de curso de fechamento OU se o tempo
                -- de segurança estourar, ela retorna ao estado inicial FECHADA.
                if fim_curso_fechada = '1' or to_fechando = '1' then
                    proximo_estado <= FECHADA;
                end if;
        end case;
    end process;

    -- ==============================================================================
    -- PROCESSO 3: LÓGICA DE SAÍDA (Estilo Moore)
    -- ==============================================================================
    -- As saídas dependem *apenas* do estado atual, caracterizando uma FSM de Moore.
    -- Isso evita glitches nas saídas que podem ocorrer em FSMs de Mealy.
    -- Atribuições concorrentes são uma forma compacta e clara de descrever essa lógica.
    motor_abrir  <= '1' when estado_atual = ABRINDO  else '0';
    motor_fechar <= '1' when estado_atual = FECHANDO else '0';

    -- ==============================================================================
    -- PROCESSO 4: LÓGICA DOS TEMPORIZADORES (Síncrona)
    -- ==============================================================================
    -- Este processo gerencia a contagem de tempo para os estados que necessitam.
    process(clk, rst_n)
    begin
        if rst_n = '0' then
            cnt_abrindo  <= 0;
            cnt_aberta   <= 0;
            cnt_fechando <= 0;
        elsif rising_edge(clk) then
            -- Lógica de Reset dos Contadores: Zera um contador específico no exato ciclo de
            -- clock em que a FSM entra no estado correspondente. Isso é feito detectando a
            -- transição de `estado_prev` para `estado_atual`. Garante que a contagem sempre comece do zero.
            if estado_prev /= estado_atual then
                case estado_atual is
                    when ABRINDO  => cnt_abrindo  <= 0;
                    when ABERTA   => cnt_aberta   <= 0;
                    when FECHANDO => cnt_fechando <= 0;
                    when others   => null; -- Nos outros estados, nenhuma ação de reset é necessária na entrada.
                end case;
            end if;

            -- Lógica de Incremento dos Contadores: A contagem só progride se a FSM
            -- estiver no estado correspondente e a condição de contagem for atendida.
            case estado_atual is
                when ABRINDO =>
                    -- Enquanto estiver abrindo, incrementa o contador de timeout de segurança.
                    if cnt_abrindo < C_COUNT_TIMEOUT then
                        cnt_abrindo <= cnt_abrindo + 1;
                    end if;
                when ABERTA =>
                    -- Se uma nova presença for detectada, o contador é zerado para manter a porta aberta.
                    if sensor = '1' then
                        cnt_aberta <= 0;
                    -- Se não houver presença, o contador progride até o limite.
                    elsif cnt_aberta < C_COUNT_ABERTA then
                        cnt_aberta <= cnt_aberta + 1;
                    end if;
                when FECHANDO =>
                    -- Enquanto estiver fechando, incrementa o contador de timeout de segurança.
                    if cnt_fechando < C_COUNT_TIMEOUT then
                        cnt_fechando <= cnt_fechando + 1;
                    end if;
                when others => -- Inclui o estado FECHADA.
                    -- Para segurança, garante que os contadores sejam zerados quando não estão em uso.
                    cnt_abrindo  <= 0;
                    cnt_aberta   <= 0;
                    cnt_fechando <= 0;
            end case;
        end if;
    end process;

    -- Atribuições concorrentes para gerar os flags de timeout.
    -- Um flag é ativado ('1') quando o contador correspondente atinge ou excede seu limite.
    to_abrindo  <= '1' when cnt_abrindo  >= C_COUNT_TIMEOUT else '0';
    to_aberta   <= '1' when cnt_aberta   >= C_COUNT_ABERTA  else '0';
    to_fechando <= '1' when cnt_fechando >= C_COUNT_TIMEOUT else '0';

end architecture rtl;