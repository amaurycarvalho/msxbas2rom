## Why

The Z80 kernel assembly routines under `infrastructure/kernel/asm/src/header/` currently consume all available space before the fixed-address `ARKOS_PLAYER` block at `0x6481`, leaving the `BASIC_KUN_START_FILLER` at zero bytes. New kernel routines (planned for the 1.0.1.0 release) require free space within the 16K kernel page. By removing dead code, eliminating redundant operations, and compacting repetitive patterns, we can free ~280–370 bytes without changing any external behavior.

## What Changes

- Remove the fully commented-out BDOS fallback block (90 lines, ~250 bytes) from `34_file_handling.asm`
- Remove dead `inc de` in `XBASIC_INIT` function key loop (`30_basic_helpers.asm`)
- Remove dead `ld a, 2` in `cmd_finput` state machine (`34_file_handling.asm`)
- Remove redundant `push de`/`pop de` guard in `cmd_preflight_disk` (`34_file_handling.asm`)
- Remove duplicate `ld a, (STARTUP_CFG_FILEIO)` load in `run_user_basic_code_on_rom` (`20_runtime.asm`)
- Remove redundant `xor a` in `resource.address` (`80_resources.asm`)
- Consolidate 4 consecutive `ld (ENDPRG+n),a` zero-stores into a loop/ldir (`20_runtime.asm`)
- Compact player idle-state initialization across 4 occurrences using sequential writes (`33_player.asm`)
- Fix **GICINI double-call bug** in `cmd_mute` — calls GICINI twice when only once is needed (`31_cmd.asm`)
- Fix **MegaROM mode restore bug** in `verify.slots.test.ram` — writes wrong value to mapper port (`60_bios_helpers.asm`)
- Fix **missing carry clear** before `sbc hl,de` in `cmd_fopen` heap calculation (`34_file_handling.asm`)
- Fix **hardcoded 255-byte copy** in `resource.get_data` that ignores real resource size (`80_resources.asm`)
- Replace 126-entry `jp` jump table with word-pointer dispatch table (`20_runtime.asm`)
- Update `compiler_hooks.h` C-side dispatch offsets to match new wrapper table layout
- Add kernel regression tests that verify ROM binary integrity after optimizations

## Capabilities

### New Capabilities
- `kernel-space-optimization`: Compact Z80 kernel routines to maximize free space in `BASIC_KUN_START_FILLER`, fix identified bugs, and add regression tests for ROM output integrity

### Modified Capabilities
<!-- No existing spec-level requirements are changing; behavioral contracts are preserved -->

## Impact

- **Affected files**: `20_runtime.asm`, `30_basic_helpers.asm`, `31_cmd.asm`, `33_player.asm`, `34_file_handling.asm`, `60_bios_helpers.asm`, `80_resources.asm`, `compiler_hooks.h`
- **Protected files** (not modified): `70_pletter.asm`, `90_support.asm` (including `BASIC_KUN_START_FILLER`)
- **C/C++ integration**: `compiler_hooks.h` requires dispatch offset updates if wrapper table is restructured
- **Build system**: Incremental compilation preserved; no clean-build requirement added
- **Testing**: New regression test targets added to `tests/` for ROM binary comparison
