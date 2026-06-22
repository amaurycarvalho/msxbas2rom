## Context

The MSXBAS2ROM kernel assembly in `infrastructure/kernel/asm/src/header/` is a Z80 codebase with established conventions: comparison routines return HL = 0xFFFF for truthy, 0x0000 for falsy; caller-save semantics (routine may clobber AF, BC, DE, HL unless documented); and shared work area at `WRKARE = 0xC010`. The `optimize-kernel-space` change (in-progress) already targets dead code removal, redundant elimination, code compaction, and bug fixes across 7 assembly files. This change targets micro-optimizations discovered during codebase exploration that were missed in the original scope, plus additional findings from a full re-analysis of all 5 affected files.

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
- Replace 27 instances of `sla r` across 5 routines with `add a,a` equivalents (where A is the operand or can be routed through A)
- Replace `ld (MV_DPTR),hl` / `ld hl,(MV_DPTR)` with `push hl` / `pop hl` in `XBASIC_COPY_TO`
- Remove 1 dead `xor a` in `intSHR.1` loop
- Replace multi-instruction sequences with shorter equivalents (e.g., 7× `inc hl` → `ld de,7 / add hl,de`)
- Remove 3 blocks of dead commented-out operational code + 4 dead commented-out source lines
- Apply 2 register-level Z80 optimizations in `FloatToBCD` and `XBASIC_END`
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

### D3: Replace sla a ×4 with add a, a ×4 in cmd_setfnt.default_colors.1 (4 bytes, 16 cycles)

**Decision:** In `cmd_setfnt.default_colors.1` (lines 498-502 of `31_cmd.asm`), replace four consecutive `sla a` with four `add a, a`.

**Rationale:** On Z80, `add a, a` = 1 byte / 4 cycles, while `sla a` = 2 bytes / 8 cycles. When the operand is A itself (not a memory location), both instructions produce identical results — bit 7 goes to carry, bit 0 becomes 0, and the result is in A. The flag differences (H, P/V) are inconsequential here since no following instruction depends on them.

### D4: Replace MV_DPTR load/store with push/pop in XBASIC_COPY_TO (4 bytes, 11 cycles)

**Decision:** In `XBASIC_COPY_TO` (lines 441, 458 of `30_basic_helpers.asm`), replace `ld (MV_DPTR),hl` and `ld hl,(MV_DPTR)` with `push hl` and `pop hl`.

**Rationale:** `push hl` = 1 byte / 11 cycles; `pop hl` = 1 byte / 10 cycles. The intervening code (lines 442-457) performs coordinate math with `HL`, `DE`, and `A` — it has no matching push/pop, so the stack depth is unchanged.

**Risk:** If future code changes insert unbalanced stack operations between the `push` and `pop`, the restored HL would be wrong. Mitigation: the code block is self-contained coordinate math with well-defined register usage.

### D5: Remove dead commented-out code blocks (3 blocks)

**Decision:** Remove the following fully-commented blocks:
1. `31_cmd.asm` lines 534-536: Panasonic turbo check in `cmd_turbo`
2. `32_usr.asm` lines 108-110: Same Panasonic turbo check in `usr2_turbo`
3. `60_bios_helpers.asm` lines 913-917: MSX2 VDP fallback in `gfxVDP.set`

Also remove 4 dead commented-out source lines in `30_basic_helpers.asm` (lines 153-154, 160, 201-202).

**Rationale:** These have zero active references. They are fully commented out (not conditional assembly). No behavioral impact.

---

### D6: Replace sla e ×4 with add a,a chain in FloatToBCD.add.2 (2 bytes, 16 cycles)

**Decision:** Replace four `sla e` with `ld a,e` / 4× `add a,a` / `ld e,a` in `FloatToBCD.add.2` (`21_logic_pack.asm` lines 246-249).

**Rationale:** `sla e` (CB 23) = 2 bytes / 8 cycles each = 8 bytes / 32 cycles. The accumulator route uses `ld a,e` (1 byte/4T) + 4× `add a,a` (1 byte/4T each = 4 bytes/16T) + `ld e,a` (1 byte/4T) = 6 bytes / 24 cycles. Result in E is identical: bit 7→carry, bit 0←0, same shift count.

Before:
```asm
FloatToBCD.add.2:
    call FloatToBCD.add.0
    sla e
    sla e
    sla e
    sla e
    ret
```

After:
```asm
FloatToBCD.add.2:
    call FloatToBCD.add.0
    ld a, e
    add a, a
    add a, a
    add a, a
    add a, a
    ld e, a
    ret
```

**Risk:** `FloatToBCD.add.2` is called only from `FloatToBCD.add` (lines 219, 225), where A is dead on return. Verified: A is either consumed by `or e` or discarded by the caller.

### D7: Replace ld a,h / res 7,a / ld h,a with res 7,h in FloatToBCD (2 bytes)

**Decision:** Replace `ld a,h` / `res 7,a` / `ld h,a` with `res 7,h` in `FloatToBCD` (`21_logic_pack.asm` lines 175-177).

**Rationale:** The Z80 `RES 7,H` (CB BC) = 2 bytes, vs `ld a,h` (1) + `res 7,a` (2) + `ld h,a` (1) = 4 bytes. The sign bit extracted into A at line 172 is preserved by `and 0x80` earlier — A's value at line 175 is the original H, but A is immediately overwritten at line 178 by `ld de,DAC+1`. No consumer reads A between lines 177-178.

### D8: Remove dead xor a from intSHR.1 loop (1 byte)

**Decision:** Remove `xor a` at line 111 in `intSHR.1` (`21_logic_pack.asm`).

**Rationale:** `SRA H` (CB 2C) does NOT consume the incoming carry flag — it shifts H right arithmetically with bit 7 preserved and bit 0→carry. The subsequent `RR L` consumes the carry produced by `SRA H`, not any pre-existing carry. On first loop entry, `xor a` may reset carry but `SRA H` ignores it. On subsequent iterations, carry is already correct from the previous `RR L`'s bit 0. The `xor a` is dead — it costs 1 byte and 4 cycles per iteration with no effect on correctness.

### D9: Replace cpl + 2× ld (CURLIN),a with ld hl,0xFFFF / ld (CURLIN),hl (1 byte)

**Decision:** Replace `cpl` + `ld (CURLIN),a` + `ld (CURLIN+1),a` with `ld hl,0xFFFF` / `ld (CURLIN),hl` in `XBASIC_END` (`30_basic_helpers.asm` lines 157-159).

**Rationale:** After `xor a`, A=0 → `cpl` → A=0xFF. Writing 0xFF to two consecutive addresses. `ld hl,0xFFFF` (3 bytes) + `ld (CURLIN),hl` (3 bytes) = 6 bytes vs `cpl` (1) + 2× `ld (nn),a` (3 each) = 7 bytes. HL is reloaded at line 161 from `(HEAPSTR)` — the old HL value is dead.

### D10: Replace bit 7,h / jr z with jp p in XBASIC_TAB (1 byte)

**Decision:** Replace `bit 7,h` / `jr z` with `jp p` in `XBASIC_TAB` (`30_basic_helpers.asm` lines 635-636).

**Rationale:** After `sbc hl,de`, the Z80 sets S = bit 15 of the result = bit 7 of H. `JP P` jumps when S=0 (positive result), which is semantically identical to `BIT 7,H / JR Z`. `JP P` = 3 bytes vs `bit 7,h` (2) + `jr z` (2) = 4 bytes.

### D11: Replace sla a ×2 with add a,a ×2 in cmd_play (2 bytes)

**Decision:** Replace two `sla a` with two `add a,a` in `cmd_play` (`31_cmd.asm` lines 14-15).

**Rationale:** After `and 1`, A is 0 or 1. Both `sla a` and `add a,a` produce identical results: 0→0→0, 1→2→4. Carry matches (carry=0 at each level). 4 bytes → 2 bytes.

### D12: Replace sla a ×4 with add a,a ×4 in cmd_page.delay (4 bytes)

**Decision:** Replace four `sla a` with four `add a,a` in `cmd_page.delay` (`31_cmd.asm` lines 1284-1287).

**Rationale:** Shifts D's value into the high nibble of A. `add a,a` and `sla a` produce identical A results and carry for any 8-bit input. The subsequent `or e` discards carry. 8 bytes → 4 bytes.

### D13: Replace sla a ×4 with add a,a ×4 in cmd_mtf.palette.copy.to_vram.loop (4 bytes)

**Decision:** Replace four `sla a` with four `add a,a` in `cmd_mtf.palette.copy.to_vram.loop` (`31_cmd.asm` lines 1716-1719).

**Rationale:** Shifts red palette component into high nibble. Palette red values are 0-7 (3-bit GRB). Both instructions produce identical results for any 8-bit value. 8 bytes → 4 bytes.

### D14: Replace 7× inc hl with ld de,7 / add hl,de in cmd_wrtfnt (3 bytes)

**Decision:** Replace seven consecutive `inc hl` with `ld de,7` / `add hl,de` in `cmd_wrtfnt` (`31_cmd.asm` lines 90-96).

**Rationale:** 7× `inc hl` = 7 bytes / 42 cycles. `ld de,7` (3 bytes/10T) + `add hl,de` (1 byte/11T) = 4 bytes / 21 cycles. DE is clobbered by `resource.copy_to_ram_on_page_3` at line 88 but is not used between lines 90 and 111-112 where it's freshly loaded. Safe.

### D15: Remove dead commented-out source lines in XBASIC_END / XBASIC_INIT (zero bytes)

**Decision:** Remove commented-out lines at 30_basic_helpers.asm lines 153-154 (`MAXFIL`, `NLONLY`), 160 (`DSKDIS`), and 201-202 (call to BIOS routine).

**Rationale:** Zero active references. Purely cosmetic cleanup.

## Risks / Trade-offs

- [intCompareGT/GE delegation adds 4 cycles of `ex de,hl` overhead per call → negligible vs original `ld a,h` (4 cycles)]
- [`add a,a` changes H and P/V flags differently than `sla a`] → Verified: no subsequent code in any affected routine reads H or P/V flags. Results in A and carry chain are identical.
- [push/pop reorder could break if stack depth changes] → The code block between push and pop has been audited: zero stack operations.
- [Comparison delegation breaks if `intCompareLT` or `intCompareLE` are modified] → Standard refactoring — tests catch regression.
- [`res 7,h` changes flags vs the original sequence] → No code between lines 177-178 reads flags. Safe.
- [Removing `xor a` in intSHR.1] → Verified against Z80 documentation: `SRA H` does not consume incoming carry.
- [`ld de,7` clobbers DE in cmd_wrtfnt] → DE is reloaded before any downstream use.
- [`jp p` semantics after sbc] → After 16-bit SBC, S flag = bit 15 of result = bit 7 of H. Matches `bit 7,h`.

## Coordination with In-Flight Changes

- **`mtf-enhancements`** and **`set-page-screen4`**: Both modify `cmd_mtf` in `31_cmd.asm` (~lines 1654-1927). D14 (`cmd_mtf.palette`) at lines 1716-1719 is a simple instruction substitution (sla→add) that changes structure minimally. D13 (`cmd_page.delay`) at lines 1284-1287 is far from the mtf area with no known conflict.
- **`kernel-call-indirection`**: Modifies `20_runtime.asm` and C++ — zero overlap.
- **`disable-unused-90-support-code`**: Modifies `90_support.asm` only — zero overlap.

## Open Questions

- **Q1:** Should the 3-bank repeated write pattern (`set_tile.paste.all`, `set_tile_color_buf.all`, `set_tile_pattern_buffer.all`) be factored into a shared subroutine? ~20-30 bytes. Deferred.
- **Q2:** Could `intCompareNOT` be optimized? Current 7 bytes — already minimal.
- **Q3:** Should duplicate scroll-left/right loops in `cmd_screen_copy.3`/`cmd_screen_copy.7` be merged? ~8-10 bytes but adds branch overhead for 24 rows. Deferred.
- **Q4:** Should `usr0`/`usr1` near-duplicates be merged with a flag parameter? ~10 bytes but requires stack-based flag preservation across calls. Deferred.
