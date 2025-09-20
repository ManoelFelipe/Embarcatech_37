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

Esta unidade foca nas ferramentas a ser usadas nesta fase do programa. Sobre a linguagem Verilog. Para cada atividade listada abaixo, a solução e a lógica de implementação são geralmente detalhadas nos comentários do respectivo código-fonte.

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
- **Enunciado:** Desenvolva um código utilizando o Icarus Verilog que represente um subtrator completo (“subtratorcompleto”). A aplicação deverá ser composta por três entrada (a, b, cin) e duas saídas (s, cout). Faça a compilação, simulação e visualização das formas de onda do projeto “subtratorcompleto” no VSCode, utilizando o Icarus Verilog e verifique os resultados da simulação no GTKWave. A tabela 1 mostra os valores de entrada e saída do circuito subtrator completo.
  
- **Link para o código:** [Unidade_02/Cap_02](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Segunda_Fase_FPGA/Unidade_02/Cap_02)


### Unidade 04: Ferramentas de Desenvolvimento

Esta unidade foca nas ferramentas a ser usadas nesta fase do programa. Sobre a linguagem VHDL. Para cada atividade listada abaixo, a solução e a lógica de implementação são geralmente detalhadas nos comentários do respectivo código-fonte.

#### Atividade em Aula: Realisado em Dupla: Decodificador binário de 3 para 8 bits

- **Enunciado:** Responder as questões sobre o projeto. Está descrito no arquivo Apresentação_Final_Manoel_Furtado.pdf no link abaixo.
  
- **Link para o código:** [Unidade_04/Atividade_Em_Aula](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Segunda_Fase_FPGA/Unidade_04/Atividade_Em_Aula/)

#### Capítulo 01: Descrição de Hardware com VHDL

- **Enunciado:** Para que um alimento seja classificado como light, seu valor calórico deve corresponder, no máximo, a 50% das calorias presentes no produto original. Considera-se, neste contexto, a adição de ingredientes opcionais utilizados para realçar o sabor e a coloração do alimento, os quais contribuem com diferentes percentuais calóricos em relação ao produto
normal, conforme descrito a seguir:
• A contém 40%; B contém 30%; C contém 20%; D contém 10% A partir dessas definições, monte a tabela-verdade, o mapa de Karnaugh e implemente em linguagem VHDL a solução do problema. Projete um circuito para acender uma lâmpada cada vez que a combinação dos produtos misturados em um tanque ultrapassar 50% das calorias de um produto normal.
  
- **Link para o código:** [Unidade_04/Cap_01](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Segunda_Fase_FPGA/Unidade_04/Cap_01)


### Unidade 05: Aritimética Distribuída

#### Capítulo 01: Registradores

- **Enunciado:** Desenvolver e simular um registrador paralelo de 8 bits utilizando flip-flops tipo D, onde cada flip-flop possui apenas entrada D e entrada de clock. O registrador deve permitir o armazenamento de dados paralelos e a atualização simultânea de todos os bits a cada pulso ascendente de clock.
  
- **Link para o código:** [Unidade_05/Cap_01](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Segunda_Fase_FPGA/Unidade_05/Cap_01)

