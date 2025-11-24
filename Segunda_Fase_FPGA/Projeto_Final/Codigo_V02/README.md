
# Controlador de Caixa d'Água – Pacote de Simulação

Este pacote inclui:
- `tb_decodificador_nivel.v` – Testbench **auto-verificável** do *decodificador_nivel*.
- `tb_controlador_caixa_dagua.v` – Testbench **sintético** do *controlador_caixa_dagua* (ajuste portas se necessário).
- `maquina_de_estado.mmd` – Diagrama Mermaid da FSM principal.
- `tabela_de_transicao.md` – Tabela de estados/saídas/transições.
- **(esperado no diretório)**: `decodificador_nivel.v` e `controlador_caixa_dagua.v` (seus módulos).

## Premissas
- **Lógica de sensores INVERTIDA**: `0 = água presente`, `1 = sem água`.
- Mapa de bits: `[0]=0%`, `[1]=25%`, `[2]=50%`, `[3]=75%`, `[4]=100%`.
- **Regras da válvula (globais):**
  - Inicia **aberta**;
  - **Fecha** quando a **caixa inferior** atinge **100%**;
  - **Reabre** quando o nível da **caixa inferior** cair **abaixo de 75%**.

> Estas regras já estão refletidas no testbench sintético e na tabela/diagrama.
> Se seu `controlador_caixa_dagua.v` implementar estados com outros nomes ou thresholds, ajuste os rótulos/condições.

## Como simular (Icarus Verilog + GTKWave)

### 1) Teste do decodificador
```bash
iverilog -g2012 -o sim_dec tb_decodificador_nivel.v decodificador_nivel.v
vvp sim_dec
```

### 2) Teste do controlador
> **Atenção**: o instanciamento é por **nome**; se o seu arquivo tiver mais saídas (ex.: `display_sup`), a simulação ainda compila. Se os nomes diferirem, edite `tb_controlador_caixa_dagua.v`.

```bash
iverilog -g2012 -o sim_ctrl tb_controlador_caixa_dagua.v controlador_caixa_dagua.v decodificador_nivel.v
vvp sim_ctrl
```

Para inspecionar sinais com GTKWave, adicione `$dumpfile/$dumpvars` ao TB, por exemplo:
```verilog
initial begin
  $dumpfile("wave_ctrl.vcd");
  $dumpvars(0, tb_controlador_caixa_dagua);
end
```
E rode:
```bash
vvp sim_ctrl
gtkwave wave_ctrl.vcd
```

## Mermaid
Você pode visualizar `maquina_de_estado.mmd` com extensões do VS Code (ex.: *Markdown Preview Mermaid Support*) ou renderizadores Mermaid.

## Estrutura sugerida de pastas
```
/sim
  tb_decodificador_nivel.v
  tb_controlador_caixa_dagua.v
/src
  decodificador_nivel.v
  controlador_caixa_dagua.v
/docs
  maquina_de_estado.mmd
  tabela_de_transicao.md
```

## Próximos passos
- Ajustar thresholds/condições no `tabela_de_transicao.md` caso sua FSM real use critérios diferentes.
- Incluir asserts formais (SystemVerilog `assert`) quando migrar para SV.
- Adicionar *scoreboard* simples comparando `display_inf` com função de referência (similar ao TB do decodificador).
