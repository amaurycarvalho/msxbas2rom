## Context

A `wrapper_routines_map_table` em `20_runtime.asm` é uma tabela de 228 word-pointers (`dw`) que mapeia índices de dispatch para endereços de rotinas kernel. Das 228 entradas:

- **147 apontam diretamente para labels** definidos como código real em arquivos `.asm` (ex: `cmd_clrscr`, `intCompareAND`, `gfxTileAddress`). Sem indireção.
- **80 apontam para labels XBASIC_*** definidos como EQUs em `00_constants_and_workarea.asm` com endereços hardcoded (ex: `XBASIC_SGN_INT: equ 0x6BD5`).
- **1 (WriteParamBCD)** é um caso híbrido: existe como label em `90_support.asm`, mas também tem um EQU redundante que conflita.

Cada endereço hardcoded nos EQUs XBASIC_* já possui um label nativo no mesmo endereço, seguindo a convenção `CXXXX`/`IXXXX`/`JXXXX` (ex: `0x6BD5` → `J_SGN_INT`, `0x74BC` → `C74BC`). A única exceção é `XBASIC_COPY_STRING` (0x7F44), que cai 4 bytes dentro da rotina `C7F40`, sem label próprio.

Os labels `XBASIC_*` são referenciados por:
1. `dw XBASIC_*` na `wrapper_routines_map_table` (20_runtime.asm)
2. `compiler_hooks.h` via constantes `DISP_*` que indexam a tabela, resolvendo o endereço através de `bin_header_bin[DISP_xxx * 2]`
3. Código C++ via `addKernelCall(DISP_*)` que chama `getKernelCallAddr()`

O Pasmo suporta `equ <label>` onde `<label>` é um label definido em qualquer arquivo incluído. Labels são resolvidos na segunda passagem do assembler, então forward references funcionam.

## Goals / Non-Goals

**Goals:**
- Alterar 80 EQUs `XBASIC_*: equ 0xXXXX` para `XBASIC_*: equ <label-nativo>` (label `CXXXX`/`IXXXX`/`JXXXX` correspondente)
- Remover EQU redundante `WriteParamBCD: equ 0x6C0B` que conflita com label homônimo
- Adicionar label `C7F44:` em `90_support.asm` no entry point do `XBASIC_COPY_STRING`
- Comprovar que a compilação produz binário byte-idêntico ao estado pós-fix do `WriteParamBCD`

**Non-Goals:**
- Não alterar a `wrapper_routines_map_table` — entradas `dw XBASIC_*` continuam exatamente iguais
- Não alterar `compiler_hooks.h` ou código C++
- Não alterar nenhum outro arquivo `.asm` além dos dois especificados
- Não criar labels intermediários redundantes — usar os labels nativos já existentes

## Decisions

### Decisão 1: `XBASIC_*: equ <label-nativo>` em vez de trocar `dw XBASIC_*` → `dw CXXXX`

**Escolha**: Manter `dw XBASIC_*` na map table e mudar apenas a definição dos EQUs.

**Alternativa considerada**: Substituir `dw XBASIC_SGN_INT` por `dw J_SGN_INT` diretamente na map table. Rejeitada porque:
- Exigiria alterar 80 linhas na map table + documentação associada
- Os nomes `XBASIC_*` são semanticamente significativos (indicam funções expostas ao compilador BASIC)
- A indireção EQU→label é transparente para o Pasmo (resolve na segunda passagem)
- Menor superfície de mudança = menor risco

### Decisão 2: Labels `CXXXX`/`IXXXX`/`JXXXX` como alvo dos EQUs

**Escolha**: Usar os labels nativos existentes que já estão no mesmo endereço físico.

**Justificativa**: Todos os 79 endereços (exceto 0x7F44) já possuem label definido em `90_support.asm` ou arquivos relacionados. A convenção `C`/`I`/`J` é consistente no código (C = callable subroutine, I = internal entry, J = jump target). Não há necessidade de criar novos nomes.

### Decisão 3: Label `C7F44` para XBASIC_COPY_STRING

**Escolha**: Adicionar `C7F44:` logo antes de `LD B,0` (linha após `LD C,(HL)`) em `90_support.asm`, dentro da rotina `C7F40`.

**Contexto**: `XBASIC_COPY_STRING` é um entry point alternativo da rotina de cópia de string que espera DE pré-carregado pelo caller (enquanto `C7F40`/`XBASIC_COPY_STRING_TO_NULBUF` carrega DE de NULBUF). A 4 bytes de diferença entre os dois entry points é típica em rotinas Z80 que compartilham corpo.

```
C7F40:  LD DE,(NULBUF)   ← XBASIC_COPY_STRING_TO_NULBUF
        LD C,(HL)
C7F44:  LD B,0            ← XBASIC_COPY_STRING (entry point)
        INC BC
        LDIR
        RET
```

**Alternativa considerada**: Manter `XBASIC_COPY_STRING: equ 0x7F44` como único label. Rejeitada porque inconsistente com o resto do código (todos os outros endereços têm label `CXXXX`).

### Decisão 4: Remover EQU `WriteParamBCD: equ 0x6C0B`

**Escolha**: Remover a linha `WriteParamBCD: equ 0x6C0B` de `00_constants_and_workarea.asm`.

**Justificativa**: O label `WriteParamBCD:` já existe em `90_support.asm:190` no endereço correto. O EQU é redundante e causa erro de compilação "Invalid definition, previously defined as EQU or label". A map table já referencia `dw WriteParamBCD` diretamente (não é um XBASIC_*), então o EQU nunca foi necessário.

## Risks / Trade-offs

- **Risco**: Erro de digitação em algum dos 80 labels de destino.
  - **Mitigação**: Compilar com `pasmo` e verificar que `header.symbols.asm` tem os mesmos endereços para todos os símbolos `XBASIC_*`. Um label incorreto geraria endereço diferente ou erro de símbolo indefinido.

- **Risco**: Labels `J_*` com underscore (ex: `J_SGN_INT`) podem ter problemas de parsing no Pasmo.
  - **Mitigação**: O Pasmo suporta underscore em labels. Os labels `J_SGN_INT`, `J_SGN_FLOAT`, `J_ABS_INT` já são definidos e usados em `90_support.asm` — comprovadamente funcionais.

- **Trade-off**: Cria dependência de forward reference (labels definidos em `90_support.asm` sendo referenciados em `00_constants_and_workarea.asm`, incluído antes). O Pasmo resolve na segunda passagem, então funciona. Mas se alguém tentar usar um assembler single-pass estrito, quebraria. Aceitável dado que o projeto é específico para Pasmo.

## Open Questions

Nenhuma — todos os mapeamentos foram verificados na fase de exploração e o escopo está totalmente definido.

