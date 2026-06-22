## Why

The `optimize-kernel-space` change identified ~280-370 bytes of savings in the Z80 kernel, but codebase exploration revealed additional micro-optimizations in comparison routines, bit-shift patterns, register save/restore idioms, and further dead code that were missed. These gains are pure wins (size AND speed) with zero behavioral impact and should be captured before `optimize-kernel-space` ships, to avoid re-testing the kernel binary twice.

## What Changes

- Collapse `intCompareGT` into a delegation to `intCompareLT` via operand swap (`ex de,hl` + `jp`), eliminating 9 bytes of duplicated xor-bias logic in `21_logic_pack.asm`
- Collapse `intCompareGE` into a delegation to `intCompareLE` via operand swap, eliminating another 9 bytes in `21_logic_pack.asm`
- Replace four consecutive `sla a` instructions with `add a, a` in `cmd_setfnt.default_colors.1` (`31_cmd.asm`), saving 4 bytes and 16 cycles
- Replace `ld (MV_DPTR),hl` / `ld hl,(MV_DPTR)` with `push hl` / `pop hl` in `XBASIC_COPY_TO` (`30_basic_helpers.asm`), saving 4 bytes and 11 cycles
- Remove commented-out Panasonic turbo check dead code in `cmd_turbo` (`31_cmd.asm`) and `usr2_turbo` (`32_usr.asm`)
- Remove commented-out MSX2 VDP fallback dead code in `gfxVDP.set` (`60_bios_helpers.asm`)

## Capabilities

### New Capabilities
- `kernel-micro-optimizations`: Compact Z80 comparison routines, replace suboptimal instruction sequences, and remove additional dead code in the kernel assembly source, preserving 100% behavioral compatibility

### Modified Capabilities
<!-- No existing spec-level requirements are changing; these are purely implementation-level micro-optimizations -->

## Impact

- **Affected files**: `21_logic_pack.asm`, `30_basic_helpers.asm`, `31_cmd.asm`, `32_usr.asm`, `60_bios_helpers.asm`
- **Protected files** (not modified): `70_pletter.asm`, `90_support.asm`
- **C/C++ integration**: No changes — the comparison functions (`intCompareGT`, `intCompareGE`) must retain the same entry points, calling conventions, and return semantics
- **Build system**: No changes required
- **Testing**: Must pass all existing tests (unit + integration), plus any kernel regression tests introduced by `optimize-kernel-space`
- **Byte savings**: ~26 bytes from instruction-level optimizations + removal of ~11 lines of dead code comments
