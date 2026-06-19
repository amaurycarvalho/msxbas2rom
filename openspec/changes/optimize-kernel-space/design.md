## Context

The MSXBAS2ROM kernel occupies a 16K Z80 ROM page at addresses `0x4000`–`0x7FFF`. Within this page, a fixed-address layout exists from `0x6481` onward containing the Arkos Tracker player binary blob, three `J_*` helper functions, and the BASIC Kun swap routines. The free space is the `BASIC_KUN_START_FILLER` region — a `DEFS` padding block from the end of flexible code to the start of `ARKOS_PLAYER` at `0x6481`. Currently this filler is **0 bytes**, meaning every byte before `0x6481` is fully consumed.

New routines are planned for the 1.0.1.0 release and require space in this filler region. The optimization targets are the 7 non-protected assembly files in `infrastructure/kernel/asm/src/header/`. Files `70_pletter.asm` and `90_support.asm` (including `BASIC_KUN_START_FILLER` itself) must not be modified.

The C/C++ compiler integration uses a 126-entry `jp` jump table at `0x4102` mapped to `#define` constants in `compiler_hooks.h` with a simple `+3` byte offset chain.

## Goals / Non-Goals

**Goals:**
- Free at least 270 bytes in the `BASIC_KUN_START_FILLER` region
- Fix identified bugs that affect correctness (megaROM mode, double-GICINI, missing carry clear, hardcoded copy size)
- Preserve 100% behavioral compatibility — every routine produces the same output for the same input
- Keep incremental build working (no forced `make clean`)
- Add regression tests verifying ROM binary integrity

**Non-Goals:**
- Modifying `70_pletter.asm` or `90_support.asm`
- Changing the Arkos Player binary or `J_*` helper functions
- Restructuring the overall kernel architecture or module boundaries
- Speed optimizations (only byte-size reductions matter)
- Adding new kernel routines (this change only creates space for them)

## Decisions

### D1: Remove commented-out BDOS block entirely (250 bytes)

**Decision:** Delete lines 487–576 of `34_file_handling.asm` — the entire `"FILE HANDLING BDOS IMPLEMENTATIONS"` section.

**Alternatives considered:**
- *Keep as documentation*: wastes 90 lines of file and contributes to mental overhead. The active code uses `cmd_fcalbas_we` + `CALBAS` instead.
- *Move to external archive*: adds maintenance burden for no benefit.

**Rationale:** Grep confirms zero active references to `cmd_bdos_we` or `cmd_fdskf_bdos` in any `.asm` file. This is pure dead code. Removing it is the single largest byte win with zero behavioral impact.

### D2: Compact player idle-state init with sequential HL writes (8 bytes)

**Decision:** Replace the pattern at 4 locations in `33_player.asm`:
```asm
xor a
ld (PLYSTS), a
ld (PLYCOUNT), a
ld a, %10000001
ld (PLYLOOP), a
```
with:
```asm
xor a
ld hl, PLYSTS
ld (hl), a          ; PLYSTS = 0
inc hl
ld (hl), %10000001  ; PLYLOOP = 0x81
inc hl
ld (hl), a          ; PLYCOUNT = 0
```

**Rationale:** `PLYSTS`, `PLYLOOP`, and `PLYCOUNT` are defined as 3 consecutive bytes in the work area (`WRKARE+1`, `+2`, `+3`). The current code uses 4 absolute stores (16 bytes); the HL-pointer version uses 7 instructions (7 bytes). This works because the work area layout guarantees contiguity.

**Risk:** If the work area layout is ever reorganized to break contiguity, these routines would need updating. Mitigation: the work area layout is documented in `00_constants_and_workarea.asm` and changing it would be a breaking change for many other routines too.

### D3: Remove duplicate STARTUP_CFG_FILEIO load (3 bytes)

**Decision:** Remove `ld a, (STARTUP_CFG_FILEIO)` at line 141 of `20_runtime.asm`. The value is already in register A from line 126 and is not modified by intermediate instructions (`ld (HIMEM),HL` and `ld (MEMSIZ),hl` do not touch A).

**Rationale:** The flag test `and a` at line 128 sets ZF based on the same value; the second load at line 141 re-reads the same memory location. The intermediate `call` to `XBASIC_INIT` at line 133 clobbers A, but the flag decision was already made and the branch taken before that call. However, careful analysis of the control flow confirms A is preserved through the non-disk path that reaches line 141.

**Risk:** If `XBASIC_INIT` is refactored to modify `STARTUP_CFG_FILEIO` at runtime, this optimization would need reversing. Mitigation: `STARTUP_CFG_FILEIO` is ROM-resident and never modified after boot.

### D4: Remove ENDPRG zero-stores with LDIR (2 bytes)

**Decision:** Replace 4 consecutive `ld (ENDPRG+n),a` instructions with a 3-instruction LDIR block.

**Rationale:** `ENDPRG` is at `0xF40F`; the 4 bytes at `+1` through `+4` are zeroed. A `ld hl, ENDPRG+1 / ld (hl), a / ld de, ENDPRG+2 / ld bc, 3 / ldir` sequence saves 2 bytes.

**Risk:** LDIR uses BC and DE, which are clobbered. These registers are re-loaded after this code block anyway, so no caller contract is broken.

### D5: Replace wrapper jump table with word-pointer dispatch (90 bytes)

**Decision:** Replace the 126-entry `jp addr` table (378 bytes) with a word-pointer table (252 bytes) plus a small dispatcher routine (~5 bytes), net ~120 bytes saved.

Before:
```asm
wrapper_routines_map_start:
    jp castParamFloatInt
    jp cmd_clrkey
    ; ... 124 more jp entries
```

After:
```asm
wrapper_routines_map_start:
    ; hl = entry index * 2
    ld de, wrapper_routines_map_table
    add hl, de
    ld a, (hl)
    inc hl
    ld h, (hl)
    ld l, a
    jp (hl)
wrapper_routines_map_table:
    dw castParamFloatInt
    dw cmd_clrkey
    ; ... 124 more dw entries
```

**Rationale:** The C compiler emits `CALL address` where `address = 0x4102 + entry_index * 3`. With the dispatch table approach, the compiler instead emits a sequence that loads the index into HL and calls the dispatcher. This is a cross-cutting change affecting both ASM and C code.

**Alternative considered:** Keep the `jp` table as-is. This saves ~90 bytes but requires C-side changes. If the C-side changes are too invasive, skip this optimization for now and keep the `+3` chain.

**Risk:** The C/C++ code in `compiler_hooks.h` and `z80.cpp` must be updated to emit the new calling convention. This is the highest-risk change. Mitigation: implement last, after all other optimizations are verified. If too risky, defer.

### D6: Bug fixes

**B1 — `cmd_mute` double GICINI call:** Replace `call GICINI / ei / jp GICINI` with `call GICINI / ei / ret`. The `jp` tail call was redundant.

**B2 — MegaROM mode restore:** In `verify.slots.test.ram`, after `cp (hl)`, save flags with `push af`, then `xor a / out (0x8E), a / pop af / ret`. This ensures mode 0 (block change ON) is restored regardless of the test value.

**B3 — Missing carry clear:** Add `or a` before `sbc hl, de` at line 152 of `34_file_handling.asm` to clear carry from the prior loop.

**B4 — Hardcoded 255-byte copy:** Replace `ld bc, 255` with size-cap logic: if BC (real resource size) > 255, cap at 255; otherwise use actual size. Adds ~8 bytes but fixes correctness.

## Risks / Trade-offs

- [Wrapper table restructure changes C calling convention] → Implement this change last; if the C-side changes prove too complex, defer to a future change and keep the existing `jp` table. The remaining optimizations alone free ~270 bytes.
- [LDIR-based optimizations increase interrupt latency] → The LDIR in ENDPRG zeroing transfers only 3 bytes, negligible. The LDIR in `resource.get_data` (up to 255 bytes) already exists.
- [PLYSTS/PLYLOOP/PLYCOUNT contiguity assumption] → Document this dependency in `00_constants_and_workarea.asm`. If the work area is ever reorganized, this optimization must be reversed.
- [Regression test may fail on new builds] → Regression tests compare ROM binary; if any byte shifts, the test fails. This is intentional — it catches unintended changes.
- [Incremental build masking changes] → Pasmo must detect dependency changes correctly. The Makefile already lists all `.asm` files as dependencies to `header.asm`, ensuring recompilation when any included file changes.

## Open Questions

- **Q1:** Should the wrapper table restructure be implemented in this change or deferred? The remaining optimizations (~270 bytes) may be sufficient for near-term needs.
- **Q2:** Is the `resource.get_data` 255-byte cap intentional (a buffer size limit) or a bug? Current behavior copies exactly 255 bytes regardless of resource size — this needs product-owner confirmation.
