## Context

O kernel do msxbas2rom define rotinas `resource.*` para localizar e carregar recursos embutidos na ROM (dados, sprites, fontes, música, etc.). Atualmente, o resource number — índice do recurso no resource map — trafega por três canais diferentes dependendo de quem chama:

| Caller | Canal | Como chega |
|--------|-------|------------|
| CMD handlers (BASIC) | `DAC` (`0xF7F6`) | `ld (DAC), hl` no C++ compiler |
| USR functions (BASIC) | `HL` → stack → `BC` | push/pop em `usr0`/`usr1` |
| resource.open_and_get_address | `DAC` → `BC` | `ld bc, (DAC)` interno |
| resource.get_data | `BC` → stack → `BC` | push/pop interno |
| resource.address | `BC` (consumido) | cálculo ×5 do offset |

Isso gera inconsistência e ineficiência:
- DAC é lido duas vezes no path `CMD → resource.open_and_get_address → resource.address`
- `resource.open` tem o contrato `in: DAC = resource number` mas nunca lê DAC — o resource number sobrevive em RAM desnecessariamente
- `cmd_screen_load` + `XBASIC_BLOAD` repurpoem `(DAC)` como ponteiro de bloco após consumir o resource number, gerando confusão semântica
- Alguns callers (`cmd_rsctoram`, `cmd_mtf`) escrevem HL→DAC só para satisfazer o contrato, gastando 3 bytes de opcode

O `DAC` (`0xF7F6`, 16 bytes) é o acumulador do MSX BASIC — usado pelo math pack, BCD, strings temporárias. Mantê-lo limpo reduz risco de interferência com rotinas BASIC nativas chamadas pelo kernel.

### Descoberta crítica: `resource.open` destrói H

```asm
; 60_bios_helpers.asm:13 — caminho não-MegaROM
select_rsc_on_page_0:
    ld a, (SLTAD2)
    ld h, 000h          ; ★ DESTRÓI H ★
    call SUB_ENASLT
```

No caminho MegaROM (`MR_CHANGE_SGM`), HL é preservado. Mas como o código precisa funcionar em ambos, o contrato deve assumir que **HL NÃO sobrevive a `resource.open`**. Isso significa que toda rotina que chama `resource.open` e precisa do resource number depois DEVE preservá-lo via stack.

## Goals / Non-Goals

**Goals:**
- Padronizar TODAS as rotinas `resource.*` para receber resource number via `HL`
- Eliminar leitura/escrita de `(DAC)` para resource number em callers e rotinas internas
- Migrar usos não-resource de `(DAC)` (BLOAD scratch, flags de turbo/player) para `(PARM1)`
- Manter ou reduzir o footprint binário do kernel
- Reduzir o footprint do código gerado na ROM do usuário

**Non-Goals:**
- Alterar a estrutura do resource map ou numeração de recursos
- Alterar índices da dispatch table (`DISP_*`)
- Alterar comportamento visível ao usuário (BASIC)
- Otimizar `resource.open` para usar o resource number (adiado para change futuro)

## Decisions

### Decision 1: HL como protocolo único de entrada

**Escolha**: `HL` como registrador canônico para resource number em `resource.open`, `resource.address`, `resource.get_data`, `resource.open_and_get_address`.

**Alternativas consideradas**:
- Manter BC: inconsistente com USR callers (HL→BC shuffle) e cmd_rsctoram/mtf (HL→DAC→BC). BC é usado como output (resource size) em `resource.address`, gerando conflito.
- Manter DAC: polui o acumulador BASIC, requer escrita em RAM no caminho feliz (3 bytes, ~16 ciclos).

**Rationale**: HL é o registrador natural de saída do avaliador de expressões BASIC. Usá-lo como entrada elimina qualquer escrita em RAM no trânsito do resource number.

### Decision 2: Preservação via push/pop em vez de registrador alternativo

**Escolha**: Rotinas que chamam `resource.open` e precisam do resource number depois preservam HL via `push hl / pop hl`.

**Alternativa considerada**: Usar um registrador shadow (ex: `exx` para BC'). Rejeitado porque:
- `resource.open` no caminho expandido (`SUB_ENASLT` → `ENASLT.EXPSLT` → `ENASLT.SUBSLT`) pode usar registradores shadow
- Push/pop é explícito, legível, e custa o mesmo (2 bytes, ~21 ciclos)

**Custo por rotina**:

| Rotina | Preservação atual | Preservação HL | Delta |
|--------|------------------|----------------|-------|
| `resource.get_data` | `push bc / pop bc` (2B) | `push hl / pop hl` (2B) | 0 |
| `resource.open_and_get_address` | `ld bc, (DAC)` (4B) | `push hl / pop hl` (2B) | −2B |
| `resource.address` (entrada) | BC sobrevive a `resource.count` | `push hl / pop bc` (2B) | +2B |
| `usr0` / `usr1` | `push hl / pop bc` (2B) → `call address` | `push hl / pop hl` (2B) → `call address` | 0 |

**Saldo líquido no kernel**: 0 bytes.

### Decision 3: `resource.address` preserva HL→BC internamente

**Escolha**: Adicionar `push hl; call resource.count; pop bc` no início de `resource.address`. O restante do código permanece idêntico (usa BC para validação e multiplicação ×5).

**Rationale**: `resource.count` destrói HL (retorna `hl = map start`). Sem preservação, o resource number se perde. A conversão HL→BC no início mantém compatibilidade com o corpo existente da rotina, que depende de BC para o cálculo `add hl, bc` (×5).

### Decision 4: `(DAC)` → `(PARM1)` para scratch do BLOAD

**Escolha**: `cmd_screen_load` e `XBASIC_BLOAD` usam `(PARM1)` e `(PARM1+2)` em vez de `(DAC)` e `(DAC+2)` como ponteiro de bloco e segmento.

**Rationale**: `PARM1` (`0xF6E8`, 100 bytes) está livre durante todo o loop de BLOAD (que roda com DI). Nenhuma rotina chamada pelo BLOAD (`resource.open`, `resource.close`, `MR_CHANGE_SGM`, `resource.ram.unpack`, `LDIRVM`) toca em PARM1. A constante `def_PARAM1` já existe em `compiler_hooks.h:440`.

### Decision 5: `(DAC)` → `(PARM1)` para parâmetros não-resource de CMD

**Escolha**: `cmd_turbo`, `cmd_plyloop`, `cmd_plysong`, `cmd_plysound` leem seus parâmetros de `(PARM1)` em vez de `(DAC)`.

**Rotinas afetadas**:

| Rotina | Parâmetro | Tamanho | Leitura atual | Leitura nova |
|--------|-----------|---------|---------------|--------------|
| `cmd_turbo` | flag on/off | 1 byte | `ld a, (DAC)` | `ld a, (PARM1)` |
| `cmd_plyloop` | flag on/off | 1 byte | `ld a, (DAC)` | `ld a, (PARM1)` |
| `cmd_plysong` | subsong nº | 1 byte | `ld a, (DAC)` | `ld a, (PARM1)` |
| `cmd_plysound` | effect nº | 1 byte | `ld a, (DAC)` | `ld a, (PARM1)` |

**Rationale**: Nenhuma dessas rotinas usa o valor como resource number. Mantê-las em DAC seria o único uso remanescente de `(DAC)` no kernel, anulando o propósito da limpeza. PARM1 é semanticamente apropriado (parâmetros de comando) e já é usado por `cmd_mtf` para o mesmo fim.

**Nota sobre `cmd_plysong`/`cmd_plysound`**: Estas rotinas também chamam `resource.open` para slot selection. Após a migração HL, passarão `HL = 0` (ou valor indefinido) para `resource.open`, já que o contexto do player (segmento) foi salvo em `PLYSGTM` por `cmd_plyload` e `resource.open` não consome HL. Custo: 0 bytes adicionais.

### Decision 6: Migração em duas fases (compiler + kernel)

**Fase 1 — Kernel ASM**: Alterar todas as rotinas `resource.*` e seus callers no kernel para o protocolo HL. Alterar BLOAD scratch e parâmetros não-resource para PARM1.

**Fase 2 — C++ Compiler**: Remover `cpu.addLdiiHL(def_DAC)` dos resource handlers. Trocar `def_DAC` por `def_PARAM1` nos non-resource handlers (turbo, plyloop, plysong, plysound).

**Rationale**: A Fase 1 é independente — o kernel pode ser alterado primeiro porque os callers C++ ainda escrevem em DAC, e o `resource.open_and_get_address` antigo lia de lá. Mas na prática a ordem não importa porque ambos os lados precisam ser alterados para o sistema funcionar. O importante é que a mudança seja atômica no commit (ambas as fases juntas).

## Risks / Trade-offs

**[R1] `resource.open` destrói H no caminho não-MegaROM** → Mitigação: todas as rotinas que precisam do resource number após `resource.open` usam `push hl / pop hl`. Isso já é documentado no contrato da nova spec.

**[R2] `resource.address` cresce +2 bytes** → Mitigação: compensado pelos −2 bytes em `resource.open_and_get_address` e pelas economias nos callers (`cmd_play` −4B, `cmd_rsctoram` −3B, etc.). Saldo líquido do kernel: ~0 bytes.

**[R3] `cmd_plysong`/`cmd_plysound` passam HL indefinido para `resource.open`** → Mitigação: `resource.open` não consome HL (só reserva o contrato). O segmento real é carregado explicitamente via `ld a, (PLYSGTM)` + `call MR_CHANGE_SGM`. Nenhum impacto funcional.

**[R4] PARM1 pode ser usado por rotinas BIOS chamadas indiretamente** → Mitigação: `PARM1`/`PARM2` são documentados como "parameter buffers" do MSX BASIC, livres após o dispatch do CMD handler. Nenhuma rotina chamada pelo BLOAD ou pelos player commands os referencia. `cmd_mtf` é o único outro usuário e não coexiste na stack.

**[R5] Testes de integração podem quebrar** → Mitigação: Reexecutar toda a suíte de testes de integração (`tests/integration/`) após a migração. Os testes geram ROMs e as executam, validando o código gerado pelo compiler + kernel.
