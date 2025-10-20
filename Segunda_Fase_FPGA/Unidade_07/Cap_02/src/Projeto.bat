@echo off
:: =============================================================================
:: Data:        12/10/2025
:: Autor:       Manoel Furtado
:: Script de Compilacao e Programacao para FPGA Lattice ECP5
::
:: Ferramentas: OSS CAD Suite (Yosys, NextPNR, ecppack, openFPGALoader)
:: Projeto    : motores_alternados
:: Alvo       : Placa baseada no chip ECP5-45F (ex: Colorlight i9)
::
:: Workflow:
::   1. Sintese        : Converte o codigo SystemVerilog (.sv) em um netlist (.json).
::   2. Place & Route  : Posiciona e conecta os elementos logicos no chip da FPGA.
::   3. Empacotamento  : Gera o arquivo bitstream (.bit) final.
::   4. Programacao    : Carrega o bitstream na SRAM da FPGA (programacao volatil).
:: =============================================================================

:: --- Configuracao do Projeto ---
:: Variaveis para facilitar a mudanca de nome do projeto ou do caminho das ferramentas.

:: Define o caminho de instalacao do OSS CAD Suite.
set OSSCAD=C:\OSS-CAD-SUITE

:: Define o nome do modulo principal (top-level) do projeto.
:: Este nome deve corresponder ao arquivo .sv e ao nome do modulo principal.
set TOP=motores_alternados

:: Define o nome do arquivo de restricoes fisicas (pinagem, clocks, etc.).
set LPF=motores_alternados.lpf


:: --- Preparacao do Ambiente ---

:: Carrega as variaveis de ambiente do OSS CAD Suite, adicionando as ferramentas ao PATH.
call "%OSSCAD%\environment.bat"

:: Muda o diretorio de trabalho para o local onde este script esta.
:: Garante que todos os arquivos do projeto (como .sv e .lpf) sejam encontrados.
cd %~dp0


:: --- Etapa 1: Sintese com Yosys ---
echo [1/4] Sintetizando o projeto com Yosys...
:: 'read_verilog -sv': Le o arquivo fonte em SystemVerilog.
:: 'synth_ecp5': Executa o fluxo de sintese otimizado para a arquitetura ECP5.
:: '-top %TOP%': Especifica qual modulo e o principal do projeto.
:: '-json %TOP%.json': Gera o netlist de saida em formato JSON.
yosys -p "read_verilog -sv %TOP%.sv; synth_ecp5 -top %TOP% -json %TOP%.json"


:: --- Etapa 2: Place & Route (P&R) com NextPNR ---
echo [2/4] Realizando Place & Route com NextPNR...
:: Posiciona os componentes logicos (place) e desenha as conexoes entre eles (route).
:: '--json': Arquivo de entrada gerado pelo Yosys.
:: '--textcfg': Arquivo de configuracao de saida para a proxima etapa.
:: '--lpf': Arquivo de restricoes fisicas (pinagem, etc.).
:: '--45k --package CABGA381 --speed 6': Especifica o modelo exato do chip FPGA.
nextpnr-ecp5 --json "%TOP%.json" --textcfg "%TOP%.config" --lpf "%LPF%" --45k --package CABGA381 --speed 6


:: --- Etapa 3: Empacotamento do Bitstream com ecppack ---
echo [3/4] Gerando o bitstream com ecppack...
:: Converte a configuracao gerada pelo NextPNR no arquivo binario (.bit) que a FPGA entende.
:: '--compress': Habilita a compressao do bitstream, resultando em um arquivo menor.
ecppack --compress "%TOP%.config" "%TOP%.bit"


:: --- Etapa 4: Programacao da FPGA com openFPGALoader ---
echo [4/4] Programando a FPGA (destino: RAM)...
:: Carrega o bitstream na memoria volatil (SRAM) da FPGA.
:: A programacao sera perdida quando a placa for desligada.
:: '-b colorlight-i9': Especifica o modelo da placa/programador.
openFPGALoader -b colorlight-i9 "%TOP%.bit"

:: COMANDO ALTERNATIVO: Programacao da memoria Flash (nao-volatil)
:: Descomente a linha abaixo para gravar o projeto de forma permanente na placa.
:: '--unprotect-flash': Desbloqueia a memoria flash para gravacao.
:: '-f': Forca a gravacao (do ingles, "flash").
:: '--verify': Compara o conteudo da flash com o arquivo apos a gravacao para garantir a integridade.
REM openFPGALoader -b colorlight-i9 --unprotect-flash -f --verify "%TOP%.bit"


echo.
echo === FLUXO CONCLUIDO ===