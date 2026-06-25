## Why

O resource number trafega por três canais diferentes (DAC, BC, HL) dependendo do caller, causando dupla leitura de DAC, escrita desnecessária em RAM, repurposing confuso de DAC como scratch, e inconsistência de contrato entre as rotinas `resource.*` e seus callers. Padronizar em HL como protocolo único elimina poluição do DAC — liberando-o exclusivamente para o MSX BASIC math pack — e reduz o footprint do kernel e do código gerado.

## What Changes

- **resource.open**: contrato documentado muda de `in: DAC = resource number` para `in: HL = resource number` (reservado, não consumido)
- **resource.address**: entrada muda de `BC` para `HL`; preservação interna via `push hl / pop bc` (+2 bytes)
- **resource.get_data**: entrada muda de `BC` para `HL`; preservação via `push hl / pop hl` (custo igual)
- **resource.open_and_get_address**: entrada muda de `DAC` para `HL`; preservação via `push hl / pop hl` (−2 bytes vs `ld bc,(DAC)`)
- **resource.copy_to_ram_on_page_2/3**: herdam o novo contrato HL sem alterações internas
- **Todos os CMD handlers no kernel**: removem `ld bc, (DAC)` e passam HL diretamente (−4 bytes por caller)
- **Todos os CMD handlers no C++ compiler**: removem `cpu.addLdiiHL(def_DAC)` (−3 bytes por handler no código gerado)
- **usr0/usr1**: trocam `pop bc` por `pop hl` (custo igual)
- **cmd_screen_load + XBASIC_BLOAD**: trocam scratch de `(DAC)/(DAC+2)` para `(PARM1)/(PARM1+2)` — **R1 resolvido**
- **cmd_turbo, cmd_plyloop, cmd_plysong, cmd_plysound**: trocam leitura de parâmetro de `(DAC)` para `(PARM1)` — **R3 resolvido**
- C++ compiler: adiciona constante `def_PARAM1` e handlers não-resource passam a usar `def_PARAM1` em vez de `def_DAC`

## Capabilities

### New Capabilities
- `kernel-resource-protocol`: Define o contrato canônico HL para passagem de resource number entre C++ compiler, kernel CMD/USR handlers, e rotinas `resource.*`. Documenta os mecanismos de preservação (push/pop) necessários porque `resource.open` destrói H no caminho não-MegaROM.

### Modified Capabilities
- `kernel-call-routing`: Atualiza a documentação dos registradores de entrada/saída das rotinas `resource.open`, `resource.address`, `resource.get_data`, `resource.open_and_get_address` no dispatch table. (Mudança de documentação, sem alteração de índices DISP_*.)
- `compiler`: Atualiza a documentação do contrato de passagem de parâmetros: resource-related calls não usam mais DAC; non-resource CMD calls usam PARM1 em vez de DAC.

## Impact

- **Kernel ASM** (~15 pontos de edição em 6 arquivos): `60_bios_helpers.asm`, `80_resources.asm`, `31_cmd.asm`, `32_usr.asm`, `33_player.asm`, `30_basic_helpers.asm`
- **C++ Compiler** (~20 handlers em `src/application/compiler/`): remover `addLdiiHL(def_DAC)` dos resource handlers; trocar `def_DAC` por `def_PARAM1` nos non-resource handlers
- **C++ Compiler hooks** (`compiler_hooks.h`): `def_PARAM1` já existe (linha 440), sem alterações necessárias
- **Builder**: sem impacto (resource map e numeração não mudam)
- **Testes de integração**: requer recompilação e re-execução de todos os `.bas` de teste para validar código gerado
- **Saldo de bytes no kernel**: ~0 bytes (ganhos nos callers compensam custo de preservação em `resource.address`)
- **Saldo de bytes no código gerado (ROM do usuário)**: −60 bytes (~20 handlers × 3 bytes)
- **Breaking**: Nenhum — é refatoração interna, sem API pública exposta
