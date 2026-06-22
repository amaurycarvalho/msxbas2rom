## Why

~90% das chamadas do compilador C++ para rotinas do kernel Z80 usam endereços hardcoded (`cpu.addCall()`) em vez de passar pela tabela de indireção `wrapper_routines_map_start`. Quando o kernel em assembly é recompilado e as rotinas mudam de endereço, essas chamadas diretas produzem ROMs quebrados. A tabela wrapper, que já existe, é o mecanismo correto de isolamento — faltam entradas para as rotinas XBASIC e falta disciplina no lado C++ para usar o `addKernelCall`.

## What Changes

- **Adicionar ~60 novas entradas `jp`** na tabela `wrapper_routines_map_start` em `20_runtime.asm`, cobrindo todas as rotinas XBASIC atualmente chamadas com endereço direto (math, string, graphics, traps, casts, I/O)
- **Atualizar a cadeia de `#define`** em `compiler_hooks.h` com as novas entradas, mantendo o padrão de offset `+3`
- **Substituir ~117 chamadas `cpu.addCall(def_XBASIC_*)`** por `optimizer.addKernelCall(def_XBASIC_*)` em todos os arquivos C++ que emitem código para essas rotinas (expression evaluator, math/string/graphics functions, statements)
- **Substituir ~74 chamadas `cpu.addCall(def_wrapper)`** (que já apontam para a wrapper table) por `optimizer.addKernelCall(def_wrapper)` para consistência e aceleração
- **Substituir ~12 magic numbers** (`0x009C`, `0x009F`, `0x00C0`, etc.) por constantes nomeadas `def_CHSNS`, `def_CHGET`, etc. em `compiler_hooks.h`

## Capabilities

### New Capabilities
- `kernel-call-routing`: Mecanismo de despacho indireto unificado — toda chamada do compilador para o kernel passa pela wrapper table, e o `addKernelCall` resolve o endereço real lendo `bin_header_bin`, garantindo que recompilações do kernel não quebrem o compilador

### Modified Capabilities
- `compiler`: O codegen do compilador SHALL emitir todas as chamadas a rotinas do kernel via `addKernelCall`, nunca via `cpu.addCall()` com endereço hardcoded de rotina interna do kernel, e SHALL usar constantes nomeadas para endereços da BIOS

## Impact

- **Assembly**: `src/infrastructure/kernel/asm/src/header/20_runtime.asm` — ~60 novas linhas `jp` na wrapper table
- **C++ headers**: `src/application/compiler/helpers/hooks/compiler_hooks.h` — ~60 novos `#define` seguindo a cadeia de `+3`, mais ~12 defines para BIOS magic numbers
- **C++ compiler**: ~60 arquivos em `src/application/compiler/` (expression evaluator, variable emitter, code helper, todas as strategies de functions/statements) — substituição de `cpu.addCall` por `optimizer.addKernelCall`
- **C++ optimizer**: `compiler_code_optimizer.cpp` — sem alterações (o `getKernelCallAddr` já funciona corretamente; as novas entradas `jp` serão resolvidas naturalmente)
- **Sem breaking changes**: o comportamento do código gerado é idêntico; a mudança é puramente no mecanismo de resolução de endereços em tempo de compilação
