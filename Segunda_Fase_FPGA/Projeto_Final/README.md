# Controlador Digital de Nível de Líquido (FSM)

![Licença](https://img.shields.io/badge/licen%C3%A7a-MIT-blue.svg)
![Linguagem](https://img.shields.io/badge/linguagem-Verilog-green.svg)
![FPGA](https://img.shields.io/badge/FPGA-Lattice_ECP5-orange.svg)

Sistema automatizado de bombeamento entre dois reservatórios utilizando Máquina de Estados Finitos (FSM) em Verilog, com implementação direcionada à FPGA **Lattice ECP5 (placa Colorlight i9)**.

Repositório oficial: https://github.com/ManoelFelipe/Embarcatech_37_FPGA

👨‍💻 Autores
Projeto acadêmico — Instituto Federal do Maranhão (IFMA).

- Manoel Furtado - manoel.furtado.br@outlook.com
- Yuri Cândido - yuri.gcandido@gmail.com    
- Paulo Gomes - paulo.gabriel1019@gmail.com

📜 Licença
Distribuído sob a licença MIT. Você é livre para usar, modificar e distribuir, mantendo os créditos.

---

## 📄 Resumo do Projeto (Visão Geral)

Este projeto apresenta o desenvolvimento e implementação de um sistema automatizado de controle de nível de líquido utilizando máquina de estados finitos (FSM). O sistema monitora dois reservatórios com sensores discretos de nível (0% a 100% em cinco patamares) e controla uma bomba hidráulica e uma válvula solenóide para realizar a transferência de líquido de forma segura e eficiente.

A lógica de acionamento é baseada em condições de nível específicas, incluindo uma **histerese (25%/75%)** para evitar oscilações rápidas (*chattering*), enquanto indicadores visuais (LEDs e displays de 7 segmentos) permitem a leitura do status operacional em tempo real.

O protótipo demonstra a aplicação prática de lógica digital sequencial em automação, com foco em robustez, modularidade e expansão para funções avançadas, como supervisório ou integração com sensores analógicos.

> 📖 **Artigo Completo:** O documento acadêmico detalhado do projeto está disponível em: [`docs/nivel_liquido.pdf`](docs/nivel_liquido.pdf)

---

## 🎯 Objetivos do Projeto

### Objetivo Geral
Desenvolver um sistema automatizado de controle de nível de líquido baseado em FSM, capaz de acionar bomba hidráulica e válvula solenoide de forma segura, eficiente e previsível, evitando condições de risco como transbordamento ou cavitação.

### Objetivos Específicos
* Modelar a lógica de controle utilizando uma FSM com histerese explícita entre ligar/desligar.
* Implementar a solução em HDL (Verilog 2001) com arquitetura modular e parametrizável.
* Integrar sensores discretos de nível (0-100% em cinco patamares) e atuadores eletromecânicos.
* Validar o comportamento por simulação e por protótipo em bancada, com sinalização visual (LEDs/displays).
* Avaliar a estabilidade do controle frente a variações rápidas de nível e leituras de fronteira.

---

## 🧠 Arquitetura e Modelagem

### Arquitetura Funcional
A arquitetura do sistema segue um fluxo unidirecional: os sensores discretos enviam os níveis para a FSM, que processa a lógica de controle e envia comandos para os atuadores (bomba/válvula) e indicadores (LEDs/Displays).

![Figura 1 – Arquitetura funcional](/docs/figures/fig1_arch.png)

### Máquina de Estados (FSM)
A controladora é modelada como uma FSM de dois estados:
* **`IDLE`**: Estado de repouso com bomba desligada, aguardando condição de partida.
* **`PUMPING`**: Estado de operação com bomba ligada, até ocorrer a condição de parada.

| Estado | Condição de Transição | Ação |
| :--- | :--- | :--- |
| `IDLE` | Sup ≤ 25 % **e** Inf ≥ 75 % | Liga bomba / Válvula aberta |
| `PUMPING` | Sup ≥ 75 % **ou** Inf ≤ 25 % | Desliga bomba / Válvula aberta\* |


\* A válvula de reposição (reservatório inferior) só fecha quando o nível inferior atinge **100 %**.

---

## 💡 Resultados e Protótipo

Os cenários de simulação contemplaram: (i) partida normal (Sup=25%, Inf=75%); (ii) desligamento por nível alto no reservatório superior (75%); e (iii) desligamento por nível baixo no reservatório inferior (25%).

Observou-se um comportamento determinístico e livre de oscilações espúrias, com histerese funcional. A sinalização por LEDs e o mapeamento dos displays facilitaram a depuração em bancada.

<!-- 
### Fotos da Bancada

| Protótipo em Bancada (Visão Geral) | Indicadores Visuais (Displays e LEDs) |
| :---: | :---: |
| ![Protótipo em bancada](docs/figures/foto_prototipo_bancada.png) | ![Indicadores visuais](docs/figures/foto_displays_leds.png) |
| *Figura 4: Montagem geral com bomba, válvula e FPGA.* | *Figura 5: Displays indicando níveis e LEDs de status.* |
)-->
---


## 💻 Ferramentas (Toolchain Open Source)

O projeto utiliza um fluxo de ferramentas *open source* para simulação e síntese.

### 1. Simulação (Icarus Verilog + GTKWave)

```bash
# Instalar dependências (Linux/WSL)
sudo apt install iverilog gtkwave

# Navegar para o diretório de simulação
cd sim/

# Compilar os módulos Verilog e o testbench
iverilog -o fsm_tb tb_fsm.v ../src/hdl/*.v

# Executar a simulação
vvp fsm_tb

# Abrir o visualizador de formas de onda
gtkwave waves.vcd


2. Síntese e Upload (Yosys + NextPNR + OpenFPGALoader)
Fluxo para a placa Lattice ECP5 (Colorlight i9):

# Instalar toolchain completa
sudo apt install yosys nextpnr-ecp5 fpga-toolchain

# 1. Síntese (Yosys)
yosys -p "synth_ecp5 -top top -json top.json" src/hdl/*.v

# 2. Place and Route (NextPNR)
# (Assumindo que 'colorlight_i9.lpf' está no diretório 'hw/')
nextpnr-ecp5 --json top.json --lpf hw/colorlight_i9.lpf --textcfg top.cfg --um5g-85k

# 3. Geração do Bitstream (ecppack)
ecppack top.cfg top.bit

# 4. Upload para a FPGA
openFPGLoader -b colorlight_i9 top.bit
```

🚀 Trabalhos Futuros (Roadmap)
Como trabalhos futuros, propõe-se a evolução para uma FSM robusta, incluindo:

- debounce parametrizável dos sensores.
- Verificação de leituras inválidas e estado de FAULT com recuperação temporizada.
- Modo automático/manual (en_auto) com desligamento forçado.
- Exibição de códigos de erro nos displays.
- Integração com supervisório UART/MQTT.
- Versão com sensores analógicos (ADC).