## Why

The `optimize-kernel-space` change identified ~280-370 bytes of savings in the Z80 kernel, but codebase exploration revealed additional micro-optimizations in comparison routines, bit-shift patterns, register save/restore idioms, and further dead code that were missed. A re-analysis of the entire kernel (5 files, ~4600 lines) uncovered 10 additional low-risk micro-optimizations beyond the original 5, bringing total byte savings to ~46 bytes of pure wins (size AND speed) with zero behavioral impact. These should be captured before other in-flight changes (mtf-enhancements, set-page-screen4) modify overlapping files to avoid merge conflicts.

## What Changes

### Original optimizations (still applicable)
- Collapse `intCompareGT` into a delegation to `intCompareLT` via operand swap (`ex de,hl` + `jp`), eliminating 9 bytes of duplicated xor-bias logic in `21_logic_pack.asm`
- Collapse `intCompareGE` into a delegation to `intCompareLE` via operand swap, eliminating another 9 bytes in `21_logic_pack.asm`
- Replace four consecutive `sla a` instructions with `add a, a` in `cmd_setfnt.default_colors.1` (`31_cmd.asm` lines 498-502), saving 4 bytes and 16 cycles
- Replace `ld (MV_DPTR),hl` / `ld hl,(MV_DPTR)` with `push hl` / `pop hl` in `XBASIC_COPY_TO` (`30_basic_helpers.asm` lines 441, 458), saving 4 bytes and 11 cycles
- Remove commented-out Panasonic turbo check dead code in `cmd_turbo` (`31_cmd.asm` lines 534-536) and `usr2_turbo` (`32_usr.asm` lines 108-110)
- Remove commented-out MSX2 VDP fallback dead code in `gfxVDP.set` (`60_bios_helpers.asm` lines 913-917)

### Newly discovered optimizations (re-analysis)
- Replace 4× `sla e` with `ld a,e` / 4× `add a,a` / `ld e,a` in `FloatToBCD.add.2` (`21_logic_pack.asm` lines 246-249), saving 2 bytes
- Replace `ld a,h` / `res 7,a` / `ld h,a` with `res 7,h` in `FloatToBCD` (`21_logic_pack.asm` lines 175-177), saving 2 bytes
- Remove dead `xor a` from `intSHR.1` shift loop (`21_logic_pack.asm` line 111), saving 1 byte and 4 cycles/iteration
- Replace `cpl` + 2× `ld (CURLIN),a` with `ld hl,0xFFFF` / `ld (CURLIN),hl` in `XBASIC_END` (`30_basic_helpers.asm` lines 157-159), saving 1 byte
- Replace `bit 7,h` / `jr z` with `jp p` in `XBASIC_TAB` (`30_basic_helpers.asm` lines 635-636), saving 1 byte
- Remove dead commented-out source lines in `XBASIC_END` / `XBASIC_INIT` (`30_basic_helpers.asm` lines 153-154, 160, 201-202), zero bytes (cleanup)
- Replace 2× `sla a` with 2× `add a,a` in `cmd_play` (`31_cmd.asm` lines 14-15), saving 2 bytes
- Replace 4× `sla a` with 4× `add a,a` in `cmd_page.delay` (`31_cmd.asm` lines 1284-1287), saving 4 bytes
- Replace 4× `sla a` with 4× `add a,a` in `cmd_mtf.palette.copy.to_vram.loop` (`31_cmd.asm` lines 1716-1719), saving 4 bytes
- Replace 7× `inc hl` with `ld de,7` / `add hl,de` in `cmd_wrtfnt` (`31_cmd.asm` lines 90-96), saving 3 bytes

## Capabilities

### New Capabilities
- `kernel-micro-optimizations`: Compact Z80 comparison routines, replace suboptimal instruction sequences (`sla`→`add a,a`, redundant shifts, inefficient memory access patterns), and remove dead code in the kernel assembly source, preserving 100% behavioral compatibility

### Modified Capabilities
<!-- No existing spec-level requirements are changing; these are purely implementation-level micro-optimizations -->

## Impact

- **Affected files**: `21_logic_pack.asm` (3 new + 2 original optimizations), `30_basic_helpers.asm` (3 new + 1 original optimization), `31_cmd.asm` (4 new + 2 original optimizations), `32_usr.asm` (1 original), `60_bios_helpers.asm` (1 original)
- **Protected files** (not modified): `70_pletter.asm`, `90_support.asm`
- **C/C++ integration**: No changes — the comparison functions (`intCompareGT`, `intCompareGE`) must retain the same entry points, calling conventions, and return semantics
- **Build system**: No changes required
- **Testing**: Must pass all existing tests (unit + integration), plus any kernel regression tests introduced by `optimize-kernel-space`
- **Byte savings**: ~26 bytes from original optimizations + ~20 bytes from new optimizations = ~46 bytes total + removal of ~17 lines of dead code comments
- **Coordination**: This change touches `31_cmd.asm` lines far from `cmd_mtf` (~1500+ lines apart), minimizing conflict with in-flight changes `mtf-enhancements` and `set-page-screen4`. The `cmd_mtf.palette` optimization (lines 1716-1719) is a simple instruction substitution that merges cleanly.
