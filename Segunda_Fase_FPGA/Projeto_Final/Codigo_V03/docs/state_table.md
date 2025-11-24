# Tabela de Transição de Estados (FSM mínima)

| Estado atual | Condição                                  | Próximo estado | Ações                                   |
|--------------|--------------------------------------------|----------------|-----------------------------------------|
| IDLE         | (sup ≤ 25%) ∧ (inf ≥ 75%)                 | PUMPING        | pump_on=1; solenoid_open=1*             |
| PUMPING      | (sup ≥ 75%) ∨ (inf ≤ 25%)                 | IDLE           | pump_on=0; solenoid_open=1*             |

\* A lógica da válvula implementa histerese: `solenoid_open=0` **somente** quando `inf=100%` e só reabre quando o nível for inferior a 75.
