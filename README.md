# Projeto de Residência em TIC 37 – IFMA – EmbarcaTech

## Sobre o Projeto

Este repositório documenta as atividades desenvolvidas por **Manoel Felipe Costa Furtado** (Matrícula: `20251RSE.MTC0086`) durante a **Segunda Fase do Programa EmbarcaTech - Projeto Residência em TIC 37**.
- **Instituição:** IFMA - MA
- **Período do Programa:** 22 de abril de 2025 a 22 de abril de 2026

O objetivo principal é apresentar as soluções e aprendizados adquiridos ao longo do programa.

## Tecnologias Abordadas

O programa de residência abrange uma variedade de tópicos em tecnologia, incluindo, mas não se limitando a:
- Sistemas Embarcados
- Programação de Microcontroladores
- FPGA (Field-Programmable Gate Array)
- IoT (Internet das Coisas)
- Periféricos Avançados (Temporizadores, DMA, USB, Ethernet/WiFi)
- Modelos de Multitarefas
- Tratamento de Interrupções

## Nota Importante sobre o Código-Fonte

É fundamental destacar que a instituição de ensino IFMA forneceu aulas e códigos base que serviram de ponto de partida para as atividades aqui apresentadas. Os códigos neste repositório são, em sua maioria, modificações e adaptações realizadas sobre esse material original, e não construídos integralmente do zero.

## Como Navegar neste Repositório

- As atividades estão organizadas em **Unidades** e **Capítulos**, seguindo a estrutura do programa.
- Cada capítulo de atividade contém uma descrição do problema (Enunciado) e um link direto para a pasta com o código-fonte da solução no GitHub.
- Para ter acesso ao conteúdo completo, incluindo pastas `build` e outros artefatos dos projetos, utilize o link do OneDrive abaixo.

## Acesso ao Conteúdo Completo (Incluindo Builds)

Todo o conteúdo deste repositório, incluindo as pastas `build` dos projetos, está disponível no seguinte link do OneDrive:
[Repositório Completo no OneDrive](https://1drv.ms/u/c/faa9e6024cd17b33/EVEstXOcZ8pPneu2MVdEiyoBXzcnumK1d4pn0zNSxvJZLQ?e=NcoEeq)

---

## Atividades Desenvolvidas

### Unidade 01: Programação Para Microcontroladores

Esta unidade foca nos fundamentos da programação para sistemas embarcados e microcontroladores. Para cada atividade listada abaixo, a solução e a lógica de implementação são geralmente detalhadas nos comentários do respectivo código-fonte.

#### Capítulo 02: Modelos de Multitarefas em Sistemas Embarcados
- **Enunciado:** Estação de Monitoramento Interativo.
  - Criar um sistema de dois núcleos que simula uma estação de monitoramento com sensores e atuadores. Os núcleos se comunicam entre si utilizando FIFO, compartilham estado com flags e usam alarmes para executar tarefas periódicas.
- **Link para o código:** [Unidade_01/Cap_02/Atividade_01](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_02/Atividade_01)

#### Capítulo 03: Tratamento de Interrupções
- **Enunciado:** Monitoramento de Som com Interrupção de Timer.
  - Configure um timer periódico para realizar leituras do microfone analógico. Sempre que o som captado ultrapassar um limiar definido (nível de ruído considerado “alto”), a matriz de LED WS2812 deverá ser ativada com uma animação ou padrão de cores.
- **Link para o código:** [Unidade_01/Cap_03/Atividade_03](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_03/Atividade_03)

#### Capítulo 04: Periféricos Avançados: Temporizadores
- **Enunciado:** Semáforo de Trânsito Interativo.
  - Criar um semáforo de trânsito, com acionamento de travessia para pedestres e indicação de tempo restante.
- **Link para o código:** [Unidade_01/Cap_04/Atividade_04](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_04/Atividade_04)

#### Capítulo 05: Periféricos Avançados: DMA
- **Enunciado:** Sistema de Aquisição de Temperatura com DMA e Interface I2C em Microcontrolador RP2040.
  - Desenvolver um sistema embarcado que utilize o controlador DMA do RP2040 para capturar automaticamente as amostras do sensor de temperatura interno (canal ADC4) e exibir os valores em um display OLED SSD1306, utilizando comunicação I2C.
- **Link para o código:** [Unidade_01/Cap_05/Atividade_05](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_05/Atividade_05)

#### Capítulo 06: Periféricos Avançados: USB
- **Enunciado:** Dispositivo CDC (Communication Device Class) responsivo e com videntificação visual: Criar um Dispositivo CDC (Communication Device Class), que 
responda  aos  comandos  realizados  no  computador  através  de  eco  e  indique visualmente o comando executado. O projeto deve ser implementado com a biblioteca 
TinyUSB.
- **Link para o código:** [Unidade_01/Cap_06/Atividade_06](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_06/Atividade_06)

#### Capítulo 07: Periféricos Avançados: PIO - Programmable Input/Output
- **Enunciado:** Complementação do Projeto NeoControlLab Utilizando uma técnica
de interrupção em conjunto com o botão A, refaça o código de modo que a geração de
números aleatórios ocorrerá quando pressionado o botão A.
- **Link para o código:** [Unidade_01/Cap_07/Atividade_07](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_07/Atividade_07)

#### Capítulo 08: Periféricos Avançados: Ethernet/Wifi
- **Enunciado:** Simulador  Portátil  de  Alarme  para  Treinamentos  de  Brigadas  e Evacuação 
- Este trabalho propõe o desenvolvimento de uma ferramenta portátil, baseada no 
microcontrolador  Raspberry  Pi  Pico  W,  destinada  a  simulações  de  emergência  e 
treinamentos de evacuação. O sistema opera em modo Access Point, permitindo que o 
instrutor  ative  remotamente  o  modo “ALARME”  por  meio  de  um  dispositivo  móvel 
conectado à rede Wi-Fi local. Uma vez acionado, o sistema emite sinais visuais e sonoros 
— com LED piscante, buzzer ativo — e exibe, no display OLED, a mensagem “EVACUAR”, 
simulando uma situação real de emergência, sem depender de infraestrutura de rede 
externa.
- **Link para o código:** [Unidade_01/Cap_08/Atividade_08](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_08/Atividade_08)

#### Capítulo 09: Executor Cíclico
- **Enunciado:** Complementação do Projeto TempCycleDMA. Não foi utilizado outra 
estratégia para gerenciar o tempo de execução das tarefas em função da Tarefa 1, 
considerara a principal. 
- Qual a melhoria que deve ser realizada no novo projeto: \
Sincronizar as tarefas em função da primeira utilizado add_repeating_timer_ms nas 
demais tarefas e repeating_timer_callback para a tarefa 1.
- **Link para o código:** [Unidade_01/Cap_09/Atividade_09](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_09/Atividade_09)

#### Capítulo 10: Foreground/Background
- **Enunciado:** Complementação da Atividade 5 Ao executar a atividade 5,
percebemos que o BOTÃO do Joystick não foi utilizado. Também observamos que o
contador de tarefa não é zerado quando chegamos no fim da fila.
- Qual a melhoria que deve ser realizada no novo projeto: \
Manter toda lógica já existente de controle da fila com
Os BOTÕES A e B e o uso da Matriz NeoPixel. Inserir uma nova lógica de Controle no BOTÃO do JOYSTICK:
AO PRESSIONAR O BOTÃO DO JOYSTICK, O SISTEMA ZERA O CONTADOR DE EVENTOS E APAGA TODA A MATRIZ DE NEOPIXEL.
Simulando uma fila vazia que está pronta para iniciar. Os contadores que monitoram a fila e de eventos deverão ser ajustados de modo que tenhamos a ideia de uma nova fila que está sendo executada pela primeira vez.
- **Link para o código:** [Unidade_01/Cap_10/Atividade_10](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_01/Cap_10/Atividade_10)

### Unidade 02: Internet das Coisas

#### Capítulo 01: IEEE 802.15.4, Bluetooth, Zigbee
- **Enunciado:** Analise o conjunto de arquivos fornecidos (server_common.c,
server.c, server_with_wifi.c, client.c, btstack_config.h, lwipopts.h, btstack_config.h) e reflita sobre os seguintes pontos. Não altere o código: apenas estude-o e responda às
questões de forma clara e objetiva.

- **Link para o código:** [Unidade_02/Cap_01/Atividade_01](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_02/Cap_01/Atividade_01)


#### Capítulo 02: IEEE 802.11, LoRaWAN, SigFox, 4G, 5G
- **Enunciado:** A atividade propõe o desenvolvimento de um servidor HTTP
embarcado no Raspberry Pi Pico W, capaz de atuar como Access Point, fornecer uma
página HTML para controle de LED e leitura da temperatura interna.

- **Link para o código:** [Unidade_02/Atividades_Em_Aula/27-05-25_WiFi](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_02/Atividades_Em_Aula/27-05-25_WiFi)
- **Link para o código:** [Unidade_02/Atividades_Em_Aula/28-05-25_LoRaWan](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_02/Atividades_Em_Aula/28-05-25_LoRaWan)

#### Atidvidades em Aula sobre Cap.02 e 01: WiFi, LoRaWAN
- **Enunciado:** Estudo de caso completo sobre implementação de rede Wi-Fi segura em
instituição educacional. Baseado no padrão IEEE 802.11ax (Wi-Fi 6) e LoRaWAN para monitoramento de lixeiras, identificando quando estiverem cheias, os níveis das lixeiras. Identifique quais sensores ou atuadores serão necessários.

- **Link para o código:** [Unidade_02/Cap_2/Atividade_02](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_02/Cap_02/Atividade_02)


#### Capítulo 03: MQTT, CoAP, AMQP, STOMP
- **Enunciado:** Complementação da Atividade_3_MQTT_2. Perceba que, ao ocorrer o
envio do PING, o LED RGB permanece aceso na cor verde. Logo após ocorrer o PING o LED RGB deve mudar para uma cor aleatória que não seja o VERDE, padrão já definido. Na função, percebemos que ela ativa o LED para VERDE. Que o LED RGB permaneça na cor aleatória por 1 segundo, após ocorrer o PING, sinalizando no LED que houve o PING.

- **Link para o código:** [Unidade_02/Cap_3/Atividade_03](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_02/Cap_03/Atividade_03)

#### Capítulo 04: Sensores e Atuadores
- **Enunciado:** Desenvolver um sistema embarcado com o Raspberry Pi Pico W que integre
sensores (temperatura, luminosidade e gás) com atuadores (relé, servo motor e LED), criando uma solução automatizada de monitoramento e resposta a variações ambientais.

- **Link para o código:** [Unidade_02/Cap_4/Atividade_04](https://github.com/ManoelFelipe/Embarcatech_37/tree/main/Unidade_02/Cap_04/Atividade_04)





---

*Este README será atualizado conforme o progresso no programa Residência em TIC 37.*