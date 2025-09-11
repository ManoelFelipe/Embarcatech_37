## Atividades Desenvolvidas
Segunda fase da residência em TIC 37: FPGA

### Unidade 01: Introdução às FPGAs

Esta unidade foca nos fundamentos da Introdução às FPGAs. Para cada atividade listada abaixo, a solução e a lógica de implementação são geralmente detalhadas nos comentários do respectivo código-fonte.

#### Capítulo 01: Introdução às FPGAs
- **Enunciado:** Objetivo: Projetar um circuito combinacional capaz de ativar um alarme de segurança em um cofre bancário, com base nas condições de horário de
funcionamento e autorização do gerente. A tarefa envolve a construção da tabela verdade, derivação da expressão booleana, e o desenho do circuito
lógico utilizando portas lógicas básicas. 
  - Entrada (Sensores)
    - Porta do cofre (C = 0 - porta fechada; C = 1 - porta aberta)
    - Relógio eletrônico (R = 0 - fora do expediente; R = 1 - horário de expediente)
    -  Interruptor na mesa do gerente (I = 0 - alarme desativado; I = 1 - alarme ativado)

   - Saída (Atuadores)
      - Alarme (A): 0 - silencioso, 1 - gerando sinal sonoro.
   - Nota: Utilizar a seguinte sequência para tabela a verdade: C R I | A
  
- **Link para o código:** [Unidade_01/Cap_01](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Segunda_Fase_FPGA/Unidade_01/Cap_01)

### Unidade 02: Ferramentas de Desenvolvimento

Esta unidade foca nas ferramentas a ser usadas nesta fase do programa. Para cada atividade listada abaixo, a solução e a lógica de implementação são geralmente detalhadas nos comentários do respectivo código-fonte.

#### Capítulo 01: Ferramentas de Desenvolvimento: Digital JS
- **Enunciado:** Sistema de Alarme Digital com Simulação Visual Você deverá criar um circuito lógico que funcione como um sistema de alarme de segurança, com as seguintes condições:
  - Três sensores de entrada:
    - Sensor de porta (entrada A)
    - Sensor de janela (entrada B)
    - Sensor de presença (entrada C)
  - Regras de ativação do alarme (saída Y):
    - O alarme deve ser ativado (Y = 1) se qualquer dois sensores forem acionados simultaneamente.
    - O alarme deve permanecer desativado (Y = 0) se menos de dois sensores forem acionados.
- **Link para o código:** [Unidade_02/Cap_01](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Segunda_Fase_FPGA/Unidade_02/Cap_01)
- 
#### Capítulo 02: Ferramentas de Desenvolvimento: Icarus Verilog e GTKWave
- **Enunciado:** E...
  - C...
- **Link para o código:** 