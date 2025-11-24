# Tabela de Sinais e Parâmetros

## Entradas
- `clk` (clock): referência temporal do sistema.
- `rst_n` (reset síncrono, ativo em 0): reinicia FSM e contadores.
- `lvl_inf[2:0]`: nível do tanque inferior (0..4).
- `lvl_sup[2:0]`: nível do tanque superior (0..4).
- `en_auto`: 1=automático; 0=manual (bomba OFF).

## Saídas
- `pump_on`: 1=liga bomba.
- `solenoid_open`: 1=abre válvula (histerese: fecha em 100% do inferior; reabre <75%).
- `led_green`: segue `pump_on`.
- `led_red`: segue `solenoid_open` (ou padrão de segurança em FAULT).
- `fault`: 1=estado de falha por leitura inválida.
- `seg_inf[6:0]`, `seg_sup[6:0]`: mapeamento 7 segmentos (0..4, “E” p/ erro).

## Parâmetros
- `CLK_HZ`               (Hz): clock base.
- `FILTER_MS`            (ms): estabilidade mínima p/ aceitar mudança de nível.
- `FAULT_RECOVERY_MS`    (ms): janela de leituras válidas para sair de FAULT.
- `LVL_SUP_START`        (0..4): limiar de partida em sup (padrão 1=25%).
- `LVL_INF_START`        (0..4): limiar de partida em inf (padrão 3=75%).
- `LVL_SUP_STOP`         (0..4): limiar de parada em sup (padrão 3=75%).
- `LVL_INF_STOP`         (0..4): limiar de parada em inf (padrão 1=25%).
- `LVL_INF_REFILL`       (0..4): ponto de fechamento de válvula (padrão 4=100%).
- `INVERT_LEVEL_CODE`    (0/1): ajustar polaridade dos sensores discretos.
- `SEG_ACTIVE_HIGH`      (0/1): polaridade dos displays de 7 segmentos.
