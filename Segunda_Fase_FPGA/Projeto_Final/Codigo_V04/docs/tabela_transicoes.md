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