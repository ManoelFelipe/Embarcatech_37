
# Tabela de Transições da FSM

**Convenção dos sensores (lógica invertida):** `0 = água presente`, `1 = sem água`.  
Mapa de bits (LSB→MSB): `[0]=0%`, `[1]=25%`, `[2]=50%`, `[3]=75%`, `[4]=100%`.

## Estados
- `S_VAZIA` – Caixa inferior abaixo de 25%.
- `S_CHEIA` – Caixa inferior cheia/estável (≥75%).
- `S_ESVAZIANDO` – Transferência ativa (bomba ligada), escoando para a superior.

## Saídas por estado (típicas)
| Estado         | bomba | válvula | Observações |
|----------------|:-----:|:-------:|-------------|
| `S_VAZIA`      |   0   |    1    | Válvula inicia aberta. |
| `S_CHEIA`      |   0   |  1* / 0 | **Regra**: fecha quando INF==100%; reabre quando INF<75%. |
| `S_ESVAZIANDO` |   1   |    0    | Bomba ativa para esvaziar INF / transferir. |

\* Em `S_CHEIA`, se a caixa inferior ainda não atingiu 100%, válvula pode permanecer **aberta**.
Quando INF==100%, **fechar** a válvula. Ao cair abaixo de 75%, **reabrir**.

## Transições (condições simplificadas)

| De → Para         | Condição (sensores)                                                      | Intuição |
|-------------------|--------------------------------------------------------------------------|----------|
| `S_VAZIA` → `S_CHEIA` | `INF==100%` (`sensores_inf[4]==0`)                                       | Inferior completou 100%. |
| `S_CHEIA` → `S_ESVAZIANDO` | `SUP<25%` **e** `INF≥75%` (`sensores_sup[0]==1` **e** `sensores_inf[3]==0`) | Prioriza transferência para repor superior. |
| `S_ESVAZIANDO` → `S_CHEIA` | `SUP==100%` (`sensores_sup[4]==0`)                                  | Superior atingiu 100%. |
| `S_ESVAZIANDO` → `S_VAZIA` | `INF<25%` (`sensores_inf[1]==1`)                                     | Inferior esvaziou demais. |

> Observação: os índices seguem a convenção do `decodificador_nivel.v`. Ajuste as condições exatas conforme sua codificação no `controlador_caixa_dagua.v`.
