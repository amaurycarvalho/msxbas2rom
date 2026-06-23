## Context

O msxbas2rom suporta cinco mappers MegaROM (ASCII8, ASCII16, ASCII16-X, Konami4, KonamiSCC) via kernel binário único (`61_megarom.asm`) com patching em tempo de build usando dispatch table. O compilador é mapper-agnóstico — só verifica `opts->megaROM` (bool), nunca o `compileMode` específico.

O sistema atual armazena números de segmento como valores de 8 bits nos seguintes pontos críticos:
1. ROM resource map: 1 byte por entrada (`resource_manager.cpp:178`)
2. ROM header (`start.asm`): `resource.map.segment` = 1 byte (0x800D)
3. Kernel `MR_CHANGE_SGM`: recebe segmento no registrador A (8-bit)
4. Compiler bridges: `LD A, <segm8>` (2 bytes)
5. Cross-segment fixups: `dest[address-5] = segm_to & 0xFF`
6. Kernel RAM: `RSCMAPSG` (0xC033) e `RSCMAPT1` (0xC034) = 1 byte cada
7. Resource limit: `resourceBlockSegment > 255` → erro

Para 4MB ASCII16 (512 segmentos de 8KB = 256 páginas de 16KB), o número máximo de segmento é 511 (0x1FF) — excede 8 bits. Para 8MB ASCII16-X (1024 segmentos), o máximo é 1023 (0x3FF).

A estratégia atual de patching (ASCII16 → `SRL A` no kernel) não funciona para segmentos > 255 porque o registrador A (8-bit) não consegue armazenar o valor completo, e `SRL A` produziria resultados incorretos.

## Goals / Non-Goals

**Goals:**
- Migrar armazenamento interno de segmentos para 16 bits em todos os pontos do pipeline
- Suportar ROMs de até 4MB (256 páginas) no mapper ASCII16
- Suportar ROMs de até 8MB (512 páginas) no mapper ASCII16-X com address encoding
- Manter a estratégia de convivência ASCII8 ↔ ASCII16: compilador calcula segmentos no estilo ASCII8 (dois por página), kernel converte via patching
- Inserir NOPs onde necessário para espaço de código 16-bit no kernel
- Minimizar alterações de código — usar helper methods condicionais
- Manter compatibilidade binária total com ASCII8, Konami4 e KonamiSCC
- Criar testes unitários para validação

**Non-Goals:**
- Sem alteração na lógica de cálculo de segmentos do compilador (continua ASCII8-style)
- Sem kernel binário separado por mapper
- Sem suporte a ROMs > 8MB
- Sem alteração nos mappers Konami4/KonamiSCC
- Sem alteração no formato de arquivo `.rom` (continua sequência de páginas de 16KB)
- Sem suporte ao banco inferior (0x6000) do ASCII16 — kernel permanece fixo em 0x4000-0x7FFF

## Decisions

### Decision 1: Helper method `BuildOptions::uses16BitSegments()` para código condicional

Em vez de espalhar verificações `if (compileMode == ASCII16 || compileMode == ASCII16X)` pelo código, um método helper centraliza a decisão:

```cpp
bool BuildOptions::uses16BitSegments() const {
    return compileMode == CompileMode::ASCII16 ||
           compileMode == CompileMode::ASCII16X;
}
```

Isto mantém o compilador mapper-agnóstico para ASCII8/Konami (8-bit) e ativa 16-bit apenas para ASCII16/ASCII16X.

**Alternativas consideradas:**
- *Verificações inline em cada local*: Rejeitado — frágil, fácil de esquecer um local
- *Virtual dispatch por mapper*: Rejeitado — over-engineering para 2 valores diferentes

### Decision 2: Compiler bridges — 9 bytes para 16-bit, 8 bytes para 8-bit

O bridge de salto entre segmentos no compilador (`compiler.cpp:386-407`) atualmente usa 8 bytes fixos. Para 16-bit, cresce para 9 bytes:

```
ASCII8/Konami (8 bytes):              ASCII16/ASCII16X (9 bytes):
  LD A, <segm8>     ; 0x3E, nn         LD HL, <segm16>   ; 0x21, lo, hi
  LD HL, 0x8000     ; 0x21, 00, 80     LD DE, 0x8000     ; 0x11, 00, 80
  JP MR_JUMP         ; 0xC3, lo, hi     JP MR_JUMP        ; 0xC3, lo, hi
```

O `step` do compilador (`compiler.cpp:375-377`) passa a ser condicional: `step = 8` para ASCII8/Konami, `step = 9` para ASCII16/ASCII16X. O cálculo de overflow (`tt >= 0x4000`) e os skips de fixup continuam funcionando porque `step` já é variável.

**Alternativas consideradas:**
- *Manter 8 bytes com encoding*: Rejeitado — não cabe segmento 16-bit em 1 byte
- *Padding com NOP nos bridges*: Rejeitado — desperdiça espaço em todos os modos

### Decision 3: Cross-segment fixups — 2 bytes de segmento

Atualmente o fixup escreve 1 byte de segmento em `dest[address-5]`. Para 16-bit, escreve 2 bytes:

```
ANTES:
  dest[address-5] = segm_to & 0xFF;

DEPOIS (condicional):
  dest[address-5] = segm_to & 0xFF;
  dest[address-4] = (segm_to >> 8) & 0xFF;
```

O preamble de fixup (`compiler_fixup_resolver.cpp:27-39`) também muda: para 16-bit emite `LD HL, 0` ao invés de `LD A, 0` + `LD HL, 0`, mantendo o mesmo tamanho total de 8 bytes.

### Decision 4: Resource map — 6 bytes por entrada (era 5)

O resource map no ROM passa de 5 para 6 bytes por entrada:

```
ANTES (5 bytes):                     DEPOIS (6 bytes):
  WORD offset                          WORD offset
  BYTE segment    →                    WORD segment  (+1 byte)
  WORD size                            WORD size
```

O mapa base começa em 0x0010 com 2 bytes de count. Com o campo extra, cada entrada ocupa 6 bytes. O limite do mapa continua sendo 0x4000 (16KB).

No kernel (`80_resources.asm`), a rotina `resource.address` muda de `ld a, (hl)` para `ld e, (hl); inc hl; ld d, (hl)` para ler 2 bytes de segmento.

### Decision 5: ROM header (`start.asm`) — resource.map.segment de 2 bytes

O header do kernel em `start.asm` é modificado:

```
ANTES:                               DEPOIS:
0x800A: db 0x00    ; fileio          0x800A: db 0x00    ; fileio
0x800B: dw 0x0000  ; map addr        0x800B: dw 0x0000  ; map addr
0x800D: db 0x00    ; map segm        0x800D: dw 0x0000  ; map segm  (2 bytes!)
0x800E: dw ...     ; BASTEXT         0x800F: dw ...     ; BASTEXT   (shifted +1)
```

O `resource.map.segment` em `00_constants_and_workarea.asm` passa de `equ 0x800D` para continuar 0x800D (agora word). `BASTEXT` (0x800E → 0x800F) e a referência em `run_user_basic_code_on_rom` são ajustadas.

### Decision 6: Kernel workarea — RSCMAPSG e RSCMAPT1 de 1→2 bytes

Na workarea do kernel (RAM em 0xC010+), as variáveis de segmento de resource crescem:

```
ANTES:                               DEPOIS:
RSCMAPAD:  equ 0xC031  ; 2 bytes     RSCMAPAD:  equ 0xC031  ; 2 bytes
RSCMAPSG:  equ 0xC033  ; 1 byte      RSCMAPSG:  equ 0xC033  ; 2 bytes  (+1)
RSCMAPT1:  equ 0xC034  ; 1 byte      RSCMAPT1:  equ 0xC035  ; 1 byte   (moved)
PLYSGTM:   equ 0xC035  ; 1 byte      PLYSGTM:   equ 0xC036  ; 1 byte   (moved)
BASMEM:     equ 0xC036  ; start      BASMEM:     equ 0xC037  ; start    (moved)

def_BASMEM muda de 0xC036 para 0xC037 em compiler_hooks.h
```

O deslocamento total é de 1 byte (`RSCMAPSG` ganhou 1 byte). `RSCMAPT1` permanece 1 byte (uso temporário para segmento de 8 bits). O impacto em `BASMEM` é mínimo (1 byte a menos de RAM para variáveis BASIC).

### Decision 7: Kernel MR_CHANGE_SGM — rotinas separadas 8-bit e 16-bit

Em vez de modificar a rotina `MR_CHANGE_SGM` existente (quebrando compatibilidade com ASCII8/Konami), duas novas rotinas são adicionadas:

**MR_CHANGE_SGM_16** (ASCII16, segmento em HL):
```
; HL = 16-bit segment number (ASCII8 style, e.g., 2, 4, 6... = 2 per 16KB page)
; Converts to 16KB page number via 16-bit right shift
MR_CHANGE_SGM_16:
    ld (SGMADR), hl      ; save 16-bit segm (SGMADR+0 = L, SGMADR+1 = H)
    srl h                 ; HL /= 2 → 16-bit page number
    rr l
    ld a, l               ; L = page number (0-255 for ≤4MB)
    ld (0x7000), a        ; select page at 0x8000-0xBFFF
    ret
```

**MR_CHANGE_SGM_16X** (ASCII16-X, segmento em HL, address encoding):
```
; HL = 16-bit segment number (ASCII8 style)
; Converts to 16KB page number, uses address bits A8-A11 for upper bits
MR_CHANGE_SGM_16X:
    ld (SGMADR), hl
    srl h                 ; HL /= 2
    rr l
    ld a, h               ; check high byte of page number
    or a
    jr z, .base           ; page 0-255 → write to 0x7000
    ; page >= 256 → use address encoding
    ; shift H bits into A8-A11 position and write to appropriate port
    ; For 8MB: H can be 0 or 1 (512 pages = 9 bits)
    ld a, l
    ld (0x7100), a        ; A8=1 encodes upper bit
    ret
.base:
    ld a, l
    ld (0x7000), a
    ret
```

As rotinas originais `MR_CHANGE_SGM`, `MR_JUMP`, `MR_CALL`, `MR_GET_DATA`, `MR_GET_BYTE` permanecem inalteradas para ASCII8/Konami.

Os bridges do compilador chamam `MR_JUMP` (8-bit) ou `MR_JUMP_16` (16-bit) conforme o mapper. `MR_JUMP_16` é um wrapper que faz `push hl; call MR_CHANGE_SGM_16; ret` (ou equivalente).

**Alternativas consideradas:**
- *Modificar MR_CHANGE_SGM para aceitar ambos*: Rejeitado — adiciona overhead de branch em todo switch; rotinas separadas são mais limpas
- *Usar sempre HL mesmo para 8-bit*: Rejeitado — quebraria compatibilidade binária com código já emitido para ASCII8

### Decision 8: NOPs e espaço no kernel

O `KERNEL_END_FILLER` (`DEFS 08000H-$,000H` em `90_support.asm:3867`) absorve aumentos de tamanho do código do kernel. As novas rotinas `MR_CHANGE_SGM_16` e `MR_CHANGE_SGM_16X` são adicionadas ao final de `61_megarom.asm`, e o filler ajusta automaticamente.

Se necessário espaço extra em locais específicos (como `resource.address`), NOPs existentes ou bytes de padding são reutilizados. O tamanho total do kernel binário permanece 32768 bytes (0x8000) — o filler apenas diminui.

### Decision 9: Resource manager limit — condicional por mapper

O limite de segmentos no resource manager muda de fixo `> 255` para condicional:

```cpp
int maxSegment = opts->uses16BitSegments() ? 65535 : 255;
if (resourceBlockSegment > maxSegment) {
    logger->error("MegaROM size limit exceeded (" +
                  to_string(opts->uses16BitSegments() ? 8192 : 2048) + "K)");
    return false;
}
```

### Decision 10: DISP entries e dispatch table

Novos labels e DISP entries para as rotinas 16-bit:

- `MR_CHANGE_SGM_16` → `DISP_MR_CHANGE_SGM_16` (223)
- `MR_CHANGE_SGM_16X` → `DISP_MR_CHANGE_SGM_16X` (224)
- `MR_JUMP_16` → `DISP_MR_JUMP_16` (225)
- `MR_CALL_16` → `DISP_MR_CALL_16` (226)
- `MR_GET_DATA_16` → `DISP_MR_GET_DATA_16` (227)

`DISP_ENTRIES` atualizado de 223 para 228.

## Risks / Trade-offs

- **[Risk] Bridge de 9 bytes vs 8 bytes afeta cálculo de alinhamento** → Mitigation: `step` já é variável por code/data; o compilador já lida com steps variáveis via skips; basta atualizar `step = opts->uses16BitSegments() ? 9 : 8`
- **[Risk] Resource map com +1 byte/entry reduz capacidade máxima de resources** → Mitigation: Para 100 resources, o overhead extra é de apenas 100 bytes (menos de 0.6% de um bloco de 16KB). Impacto desprezível.
- **[Risk] Deslocamento de BASMEM em 1 byte reduz RAM disponível para variáveis BASIC** → Mitigation: 1 byte de 10534 bytes livres (<0.01%). Desprezível.
- **[Risk] MR_CHANGE_SGM_16X com address encoding é complexo de testar** → Mitigation: Testes unitários verificam o bytecode do kernel patchado; integração testa ROM gerada em emulador
- **[Risk] Regeneração do kernel (`make header`) pode quebrar offsets hardcoded** → Mitigation: Todos os offsets críticos usam dispatch table ou labels simbólicos; `def_BASMEM` e `DISP_*` são atualizados manualmente após regeneração
- **[Risk] ASCII16-X address encoding pode não funcionar em todos os emuladores** → Mitigation: A especificação ASCII16-X é suportada por openMSX e BlueMSX; o encoding de A8-A11 é padrão documentado

## Open Questions

1. **O `MR_JUMP_16` deve ser um wrapper separado ou o compilador deve chamar `MR_CHANGE_SGM_16` diretamente?** — Preferência por wrapper separado para manter o padrão existente (MR_JUMP → push hl + MR_CHANGE_SGM + ret)
2. **Para ASCII16-X 8MB, o limite superior é exatamente 8MB ou deve ser arredondado para baixo?** — 512 páginas × 16KB = 8MB exato; manter exato
3. **As rotinas 16-bit substituem completamente as 8-bit para ASCII16/ASCII16X ou coexistem?** — Coexistem: o patching do kernel redireciona as chamadas para as versões 16-bit nos modos ASCII16/ASCII16X, mas as rotinas 8-bit permanecem no binário para ASCII8/Konami
4. **O `resource.address` do kernel precisa de versão 16-bit separada ou basta modificar a existente?** — Modificar a existente: sempre lê 2 bytes de segmento do resource map (o campo WORD é incondicional), mas só usa o byte alto se `MAPPER` indicar modo 16-bit
