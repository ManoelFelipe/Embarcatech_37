REM @echo off: Oculta a exibição dos próprios comandos no console.
@echo off

REM --- Seção de Configuração de Variáveis ---

REM Define a variável 'OSSCAD' apontando para o local de instalação do OSS CAD Suite.
set OSSCAD=C:\OSS-CAD-SUITE

REM Define a variável 'TOP' com o nome do módulo principal (top-level) do seu projeto.
REM Isso facilita a reutilização do script, mudando apenas esta linha.
set TOP=controlador_caixa_dagua
REM Define a variável 'LPF' com o nome do arquivo de restrições de pinos.
set LPF=pins.lpf

REM --- Preparação do Ambiente ---

REM Executa o script 'environment.bat' da suíte OSS CAD.
REM Isso adiciona todas as ferramentas (Yosys, nextpnr, etc.) ao PATH do console.
call "%OSSCAD%\environment.bat"

REM Muda o diretório de trabalho atual ('cd') para o diretório onde o script .bat está (%~dp0).
REM Isso garante que os arquivos %TOP%.sv e %LPF% sejam encontrados.
cd %~dp0

REM --- Etapa 1: Síntese (Yosys) ---

REM Exibe uma mensagem de progresso no console.
echo [1/4] Synth

REM Executa o Yosys (ferramenta de síntese).
REM -p "...": Passa uma string de comandos para o Yosys.
REM "read_verilog -sv %TOP%.v": Lê o arquivo Verilog (%TOP% vira 'transistor_sensor_test').
REM "synth_ecp5 -top %TOP%": Sintetiza o código (transforma em lógica) para a arquitetura ECP5.
REM "-json %TOP%.json": Salva o resultado (netlist) em um arquivo .json.
yosys -p "read_verilog -sv decodificador_nivel.v controlador_caixa_dagua.v; synth_ecp5 -top %TOP% -json %TOP%.json"

REM --- Etapa 2: Place & Route (nextpnr) ---

REM Exibe uma mensagem de progresso (o nome da etapa está faltando, seria P&R).
echo [2/4] 

REM Executa o nextpnr (ferramenta de Place & Route).
REM --json "%TOP%.json": Lê o netlist gerado pelo Yosys.
REM --textcfg "%TOP%.config": Salva a configuração de roteamento em um arquivo de texto.
REM --lpf "%LPF%": Usa o arquivo de restrições de pinos para saber onde colocar as E/S.
REM --45k: Especifica o dispositivo (ECP5 de 45k LUTs).
REM --package CABGA381: Especifica o encapsulamento do chip (para o Colorlight i9).
REM --speed 6: Especifica o "speed grade" do chip.
nextpnr-ecp5 --json "%TOP%.json" --textcfg "%TOP%.config" --lpf "%LPF%" --45k --package CABGA381 --speed 6

REM --- Etapa 3: Empacotamento (ecppack) ---

REM Exibe uma mensagem de progresso.
echo [3/4] Pack

REM Executa o ecppack para criar o arquivo de bitstream final.
REM --compress: Compacta o bitstream.
REM "%TOP%.config": Arquivo de entrada (da etapa 2).
REM "%TOP%.bit": Arquivo de saída (o bitstream que vai para o FPGA).
ecppack --compress "%TOP%.config" "%TOP%.bit"

REM --- Etapa 4: Programação (openFPGALoader) ---

REM Exibe uma mensagem de progresso.
echo [4/4] Program (RAM)

REM Esta linha está comentada. Ela gravaria o bitstream na RAM do FPGA (volátil).
REM openFPGALoader -b colorlight-i9 "%TOP%.bit"

REM Esta é a linha ativa de programação.
REM openFPGALoader: Executa a ferramenta de gravação.
REM -b colorlight-i9: Especifica o modelo da placa/programador.
REM --unprotect-flash: Desbloqueia a memória flash para gravação.
REM -f: Indica para gravar na memória FLASH (permanente, não volátil).
REM --verify: Verifica se o conteúdo gravado na flash bate com o arquivo .bit.
REM "%TOP%.bit": O arquivo a ser gravado.
openFPGALoader -b colorlight-i9 --unprotect-flash -f --verify "%TOP%.bit"

REM Exibe uma mensagem de conclusão.
echo === DONE ===