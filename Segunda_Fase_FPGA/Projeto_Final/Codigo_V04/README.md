# Medição de Nível de Líquido — FSM Robusta (v4)

Este repositório contém a evolução do projeto para a **Versão Completa (FSM robusta)**: debounce, detecção de falhas, modo manual, temporizações e parametrização.

## Estrutura
```
v4_fsm_robusta/
├─ src/
│  └─ pump_controller.v
├─ sim/
│  └─ tb_pump_controller.v
├─ docs/
│  ├─ README.md
│  ├─ fsm_diagram.mmd
│  ├─ tabela_transicoes.md
│  └─ tabela_sinais_parametros.md
└─ prompts/
   ├─ Prompt_Completa_v4_refinado.txt
   └─ Prompt_Simples_v1.txt
```

## Simulação (Icarus Verilog + GTKWave)
```bash
# 1) Compilar
iverilog -g2005 -o build/tb_pump tb/tb_pump_controller.v rtl/pump_controller.v

# 2) Executar
vvp build/tb_pump

# 3) Abrir waveform
gtkwave wave.vcd
```

> Ajuste parâmetros (ex. `CLK_HZ`, `FILTER_MS`, etc.) diretamente no `pump_controller.v` ou via `defparam` no testbench.

## Parâmetros principais (sugeridos)
- `CLK_HZ               = 50_000_000`
- `FILTER_MS            = 20`
- `FAULT_RECOVERY_MS    = 200`
- `LVL_SUP_START        = 1` (25%)
- `LVL_INF_START        = 3` (75%)
- `LVL_SUP_STOP         = 3` (75%)
- `LVL_INF_STOP         = 1` (25%)
- `LVL_INF_REFILL       = 4` (100%)
- `INVERT_LEVEL_CODE    = 1`
- `SEG_ACTIVE_HIGH      = 1`

## Estados
- `IDLE → CHECK_START → PUMPING → CHECK_STOP → IDLE`
- `FAULT` (assíncrono a partir de leituras inválidas; recuperação temporizada)

## Regras de Válvula
- Fecha quando o inferior atinge 100%
- Reabre quando inferior < 75%

## Critérios de Teste (TB)
- Partida e parada por histerese (25/75%)
- Parada por nível baixo no inferior
- Falha por leitura inválida (0..4 fora do intervalo), com recuperação
- Modo manual (en_auto=0) desligando bomba sem FAULT
- Debounce impedindo comutação por ruído


# Tabela de Transições (resumo)

| Estado       | Condição                                                                 | Próximo       | Ações/Observações                          |
|--------------|---------------------------------------------------------------------------|---------------|--------------------------------------------|
| IDLE         | en_auto==1 ∧ válidos ∧ (sup≤LVL_SUP_START ∧ inf≥LVL_INF_START)           | CHECK_START   | bomba=0                                    |
| CHECK_START  | condição mantida após filtro/estabilidade                                 | PUMPING       | bomba=1                                    |
| CHECK_START  | condição caiu                                                             | IDLE          | bomba=0                                    |
| PUMPING      | (sup≥LVL_SUP_STOP) ∨ (inf≤LVL_INF_STOP)                                   | CHECK_STOP    | bomba=1                                    |
| CHECK_STOP   | condição mantida após filtro/estabilidade                                 | IDLE          | bomba=0                                    |
| CHECK_STOP   | condição caiu                                                             | PUMPING       | bomba=1                                    |
| Qualquer     | leitura inválida (0..4 violado)                                           | FAULT         | bomba=0; fault=1                           |
| FAULT        | leituras válidas por RECOV_TICKS                                          | IDLE          | fault=0; bomba=0                           |

**Válvula (histerese):** fecha quando `lvl_inf==100%`; reabre quando `lvl_inf<75%` (e não em FAULT).

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
