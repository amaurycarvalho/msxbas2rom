## Why

~90% das chamadas do compilador C++ para rotinas do kernel Z80 usam `cpu.addCall(def_XBASIC_*)` com endereços hardcoded, em vez de passar pela tabela de despacho `wrapper_routines_map_table`, que já existe como mecanismo de indireção via word-pointers (`dw`). Quando o kernel em assembly é recompilado e as rotinas mudam de endereço, essas chamadas diretas produzem ROMs quebrados. A tabela wrapper já cobre 126 rotinas do kernel, mas faltam ~55 entradas para rotinas XBASIC (math, string, graphics, traps, casts, I/O) que o compilador ainda chama com endereço absoluto.

## What Changes

- **Adicionar ~55 novas entradas `dw`** na tabela `wrapper_routines_map_table` em `20_runtime.asm`, cobrindo todas as rotinas XBASIC atualmente chamadas com endereço direto (math, string, graphics, traps, casts, I/O)
- **Adicionar ~55 novas constantes `DISP_XBASIC_*`** em `compiler_hooks.h`, estendendo a cadeia de índices até `DISP_ENTRIES = ~181`
- **Substituir ~117 chamadas `cpu.addCall(def_XBASIC_*)`** por `optimizer.addKernelCall(DISP_XBASIC_*)` em todos os arquivos C++ que emitem código para essas rotinas (expression evaluator, math/string/graphics functions, statements)
- **Substituir ~12 magic numbers** (`0x009C`, `0x009F`, `0x00C0`, etc.) por constantes nomeadas `def_CHSNS`, `def_CHGET`, etc. em `compiler_hooks.h`
- **Substituir todas as chamadas `cpu.addCall(def_XBASIC_*)` remanescentes** (já há entradas na tabela para algumas rotinas XBASIC como `XBASIC_CLS`, `XBASIC_PLAY`, `XBASIC_PRINT_*`, etc.) por `optimizer.addKernelCall(DISP_XBASIC_*)`

## Capabilities

### New Capabilities
- `kernel-call-routing`: Mecanismo de despacho indireto unificado — toda chamada do compilador para o kernel passa pela wrapper table via `addKernelCall(DISP_*)`, que resolve o endereço real lendo o word-pointer de `bin_header_bin`, garantindo que recompilações do kernel não quebrem o compilador

### Modified Capabilities
- `compiler`: O codegen do compilador SHALL emitir todas as chamadas a rotinas do kernel via `optimizer.addKernelCall(DISP_*)`, nunca via `cpu.addCall()` com endereço hardcoded de rotina interna do kernel, e SHALL usar constantes nomeadas para endereços da BIOS

## Impact

- **Assembly**: `src/infrastructure/kernel/asm/src/header/20_runtime.asm` — ~55 novas linhas `dw` na wrapper table
- **C++ headers**: `src/application/compiler/helpers/hooks/compiler_hooks.h` — ~55 novos `#define DISP_XBASIC_*` com índices sequenciais, atualização de `DISP_ENTRIES` de 126 para ~181, mais ~10 defines nomeados para BIOS magic numbers
- **C++ compiler**: ~60 arquivos em `src/application/compiler/` — substituição de `cpu.addCall(def_XBASIC_*)` por `optimizer.addKernelCall(DISP_XBASIC_*)`, e substituição de magic numbers por named constants
- **C++ optimizer**: `compiler_code_optimizer.cpp` — sem alterações (o `addKernelCall` e `getKernelCallAddr` já funcionam com word-pointer dispatch; as novas entradas serão resolvidas naturalmente)
- **Sem breaking changes**: o comportamento do código gerado é idêntico; a mudança é puramente no mecanismo de resolução de endereços em tempo de compilação
