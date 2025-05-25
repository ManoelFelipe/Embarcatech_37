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
[Repositório Completo no OneDrive](https://1drv.ms/u/c/faa9e6024cd17b33/EfkBXkayImFAhTRsaxkK9zQB1-wfoC3P_vviyxPyBG-SoQ?e=3oVdD7)

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
- **Enunciado:** *[A ser definido/adicionado pelo aluno]*
- **Link para o código:** *[A ser adicionado pelo aluno]*

#### Capítulo 08: Periféricos Avançados: Ethernet/Wifi
- **Enunciado:** *[A ser definido/adicionado pelo aluno]*
- **Link para o código:** *[A ser adicionado pelo aluno]*

#### Capítulo 09: Executor Cíclico
- **Enunciado:** *[A ser definido/adicionado pelo aluno]*
- **Link para o código:** *[A ser adicionado pelo aluno]*

---

*Este README será atualizado conforme o progresso no programa Residência em TIC 37.*