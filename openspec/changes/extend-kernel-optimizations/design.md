## Context

The MSXBAS2ROM kernel assembly in `infrastructure/kernel/asm/src/header/` is a Z80 codebase with established conventions: comparison routines return HL = 0xFFFF for truthy, 0x0000 for falsy; caller-save semantics (routine may clobber AF, BC, DE, HL unless documented); and shared work area at `WRKARE = 0xC010`. The `optimize-kernel-space` change (in-progress) already targets dead code removal, redundant elimination, code compaction, and bug fixes across 7 assembly files. This change targets micro-optimizations discovered during codebase exploration that were missed in the original scope.

The six comparison routines in `21_logic_pack.asm` (`EQ`, `NE`, `LT`, `LE`, `GT`, `GE`) all use the same xor-bias technique for signed integer comparison:
```
; Transform signed int to unsigned by flipping sign bit
xor 0x80  ; on both H and D
; Then unsigned compare via sbc hl,de / sbc hl,hl
```
`GT` and `LT` are structurally identical except for an operand swap (`ex de,hl`). Same for `GE` and `LE`.

## Goals / Non-Goals

**Goals:**
- Reduce `intCompareGT` to a 1-instruction delegation to `intCompareLT` via `ex de,hl`
- Reduce `intCompareGE` to a 1-instruction delegation to `intCompareLE` via `ex de,hl`
- Replace 4× `sla a` with 4× `add a, a` in `cmd_setfnt.default_colors.1`
- Replace `ld (MV_DPTR),hl` / `ld hl,(MV_DPTR)` with `push hl` / `pop hl` in `XBASIC_COPY_TO`
- Remove 3 blocks of dead commented-out code
- Preserve 100% behavioral compatibility — every routine returns identical results for identical input
- Keep incremental build working

**Non-Goals:**
- Modifying `70_pletter.asm` or `90_support.asm`
- Changing the comparison function calling convention or return semantics
- Adding new capabilities — purely size/speed tightening of existing code
- Restructuring the 3-bank write pattern (left as architectural note for future)
- Speed optimizations beyond what comes naturally from instruction substitution

## Decisions

### D1: Collapse intCompareGT into delegation to intCompareLT (9 bytes saved)

**Decision:** Replace the entire `intCompareGT` body with `ex de,hl` + `jp intCompareLT`.

**Rationale:** `GT(a,b)` ≡ `LT(b,a)`. Both routines apply the same xor-bias transform to both operands and differ only by the operand swap. Delegating eliminates 9 bytes of duplicated logic.

Before:
```asm
intCompareGT:
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  sbc hl,hl
  ret
```

After:
```asm
intCompareGT:
  ex de, hl          ; GT(a,b) = LT(b,a)
  jp intCompareLT
```

**Risk:** The `jp intCompareLT` is a tail call — `intCompareLT`'s `ret` returns directly to `intCompareGT`'s caller. This is correct Z80 behavior. `ex de,hl` swaps the two operands, making `intCompareLT(b,a)` compute the original `GT(a,b)`.

### D2: Collapse intCompareGE into delegation to intCompareLE (9 bytes saved)

**Decision:** Same pattern as D1 — replace body with `ex de,hl` + `jp intCompareLE`.

**Rationale:** `GE(a,b)` ≡ `LE(b,a)`. The xor-bias logic is identical between the two; only operand order differs.

Before:
```asm
intCompareGE:
  ex de,hl
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  ccf
  sbc hl,hl
  ret
```

After:
```asm
intCompareGE:
  ex de, hl          ; GE(a,b) = LE(b,a)
  jp intCompareLE
```

Note: The current `intCompareGE` already has `ex de,hl` as its first instruction, so the body after that is identical to `intCompareLE`'s body. This confirms the symmetry.

### D3: Replace sla a ×4 with add a, a ×4 (4 bytes, 16 cycles)

**Decision:** In `cmd_setfnt.default_colors.1` (line 495–499 of `31_cmd.asm`), replace four consecutive `sla a` with four `add a, a`.

**Rationale:** On Z80, `add a, a` = 1 byte / 4 cycles, while `sla a` = 2 bytes / 8 cycles. When the operand is A itself (not a memory location), both instructions produce identical results — bit 7 goes to carry, bit 0 becomes 0, and the result is in A. The flag differences (H, P/V) are inconsequential here since no following instruction depends on them.

**Alternatives considered:**
- *`rrca / rrca / rrca / and 0xF0`*: 5 bytes, but rearranges bits differently — wrong result.
- *`ld b,4 / .loop: add a,a / djnz .loop`*: 4 bytes, but slower and clobbers B.
- *Keep `sla a`*: No size win, but also no risk. Rejected because `add a,a` is a pure win.

### D4: Replace MV_DPTR load/store with push/pop (4 bytes, 11 cycles)

**Decision:** In `XBASIC_COPY_TO` (lines 441, 458 of `30_basic_helpers.asm`), replace:
```asm
  ld (MV_DPTR), hl     ; save HL
  ; ... intervening code that clobbers HL and DE ...
  ld hl, (MV_DPTR)     ; restore HL
```
with:
```asm
  push hl              ; save HL
  ; ... intervening code ...
  pop hl               ; restore HL
```

**Rationale:** `push hl` = 1 byte / 11 cycles; `pop hl` = 1 byte / 10 cycles. `ld (addr),hl` = 3 bytes / 16 cycles; `ld hl,(addr)` = 3 bytes / 16 cycles. The intervening code (lines 442–457) performs coordinate math with `HL`, `DE`, and `A` — it has no matching push/pop, so the stack depth is unchanged. The save/restore via stack is strictly better.

**Risk:** If future code changes insert unbalanced stack operations between the `push` and `pop`, the restored HL would be wrong. Mitigation: the code block is self-contained coordinate math with well-defined register usage.

### D5: Remove dead commented-out code (3 blocks)

**Decision:** Remove the following fully-commented blocks:
1. `31_cmd.asm` lines 531–533: Panasonic turbo check in `cmd_turbo`
2. `32_usr.asm` lines 108–110: Same Panasonic turbo check in `usr2_turbo`
3. `60_bios_helpers.asm` lines 927–931: MSX2 VDP fallback in `gfxVDP.set`

**Rationale:** These have zero active references. They are fully commented out (not conditional assembly). No behavioral impact.

## Risks / Trade-offs

- [intCompareGT/GE delegation adds 1 cycle of `ex de,hl` overhead per GT/GE call] → `ex de,hl` is 4 cycles; the original routine started with `ld a,h` (4 cycles). Net cycle change is negligible. The `jp` tail call adds no extra cycles vs the original `ret`.
- [`add a,a` changes H and P/V flags differently than `sla a`] → Verified: no subsequent code in `cmd_setfnt.default_colors.1` reads H or P/V flags. The result in A and the carry chain for `or (hl)` are identical.
- [push/pop reorder could break if stack depth changes] → The code block between push and pop has been audited: zero stack operations.
- [Comparison delegation breaks if `intCompareLT` or `intCompareLE` are modified] → This is a standard refactoring — all 4 functions share the same xor-bias spec. Tests will catch any regression.

## Open Questions

- **Q1:** Should the 3-bank repeated write pattern (`set_tile.paste.all`, `set_tile_color_buf.all`, `set_tile_pattern_buffer.all`) be factored into a shared subroutine? This would save ~20–30 bytes but adds call overhead and increases code complexity. Deferred to a future change.
- **Q2:** Could `intCompareNOT` be optimized to use `ld hl,1 / or l / dec hl / sbc hl,hl` for flag-based negation? Current implementation is already minimal at 7 bytes — unlikely to improve.
