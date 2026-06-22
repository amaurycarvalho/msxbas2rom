## Why

Das 228 entradas da `wrapper_routines_map_table`, 80 usam EQUs com endereço hardcoded (`XBASIC_*: equ 0xXXXX`) para indireção, apesar de cada endereço físico já possuir um label nativo (`CXXXX`, `IXXXX`, `JXXXX`). Isso cria dupla manutenção: qualquer realocação de código exige atualizar manualmente tanto o label nativo quanto o EQU. Além disso, o EQU `WriteParamBCD: equ 0x6C0B` conflita com o label `WriteParamBCD:` já existente em `90_support.asm`, quebrando a compilação. Por fim, `XBASIC_COPY_STRING` cai no meio da rotina `C7F40`, sem label próprio, dificultando navegação.

## What Changes

- Alterar 80 EQUs `XBASIC_*: equ 0xXXXX` em `00_constants_and_workarea.asm` para `XBASIC_*: equ <label-nativo>`, onde `<label-nativo>` é o label `CXXXX`/`IXXXX`/`JXXXX` já existente no mesmo endereço
- Remover o EQU redundante `WriteParamBCD: equ 0x6C0B` (conflito com label homônimo em `90_support.asm:190`)
- Adicionar label `C7F44:` em `90_support.asm` no entry point mid-routine do `XBASIC_COPY_STRING` (offset +4 de `C7F40`)
- **Nenhuma alteração** na `wrapper_routines_map_table` (as entradas `dw XBASIC_*` continuam válidas por transitividade)
- **Nenhuma alteração** em código C++ ou `compiler_hooks.h` (os `DISP_*` resolvem via `dw` table, que continua apontando para os mesmos endereços)
- **Nenhuma alteração** em `header.symbols.asm` gerado: símbolos `XBASIC_*` mantêm os mesmos endereços

## Capabilities

### New Capabilities

Nenhuma — esta é uma mudança puramente interna de refatoração, sem alteração de comportamento.

### Modified Capabilities

Nenhuma — os requisitos de comportamento não mudam. Os endereços resolvidos pela dispatch table permanecem idênticos.

## Impact

- **Afetado**: `src/infrastructure/kernel/asm/src/header/00_constants_and_workarea.asm` (~80 linhas alteradas)
- **Afetado**: `src/infrastructure/kernel/asm/src/header/90_support.asm` (+1 label `C7F44`)
- **Afetado**: `src/infrastructure/kernel/asm/src/header/20_runtime.asm` (0 alterações — map table inalterada)
- **Binário**: Idêntico byte a byte (endereços não mudam)
- **Símbolos exportados**: Inalterados
- **Compilação**: `make` no kernel deve continuar funcionando
- **Runtime**: Zero impacto funcional
- Release 1.0.1.0

