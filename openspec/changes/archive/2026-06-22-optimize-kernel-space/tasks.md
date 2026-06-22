## 1. Baseline and Test Infrastructure

- [x] 1.1 Build current kernel and capture `header.bin` as baseline reference
- [x] 1.2 Add kernel regression test target to root `Makefile` (`make test-kernel`)
- [x] 1.3 Create `tests/kernel/` directory with test script that builds `header.bin` and validates byte ranges
- [x] 1.4 Run existing `make test-unit` and `make test-integration` to confirm baseline passes

## 2. Dead Code Removal

- [x] 2.1 Remove commented-out BDOS fallback block from `34_file_handling.asm` (lines 487–576, ~250 bytes saved)
- [x] 2.2 Remove commented-out `VDP_wait` routine from `60_bios_helpers.asm` (lines 528–541)
- [x] 2.3 Remove dead `inc de` in `XBASIC_INIT.loop` from `30_basic_helpers.asm` (line 66)
- [x] 2.4 Remove dead `ld a, 2` in `cmd_finput` from `34_file_handling.asm` (line 357)
- [x] 2.5 Build kernel and verify filler space increased; run regression tests

## 3. Redundant Code Elimination

- [x] 3.1 Remove duplicate `ld a, (STARTUP_CFG_FILEIO)` from `20_runtime.asm` (line 141)
- [x] 3.2 Remove `push de`/`pop de` guard in `cmd_preflight_disk` from `34_file_handling.asm` (lines 19–22)
- [x] 3.3 Remove redundant `xor a` in `resource.address` from `80_resources.asm` (line 49)
- [x] 3.4 Remove redundant `ld a, (VERSION)` in `XBASIC_SCREEN` from `30_basic_helpers.asm` (line 226, branch-dependent — verify) — NOT redundant, `call C7369` clobbers A
- [x] 3.5 Build kernel and verify filler space increased; run regression tests

## 4. Code Compaction

- [x] 4.1 Compact player idle-state initialization in `33_player.asm` (4 occurrences at lines 18–22, 109–111, 150–152, 181–183)
- [x] 4.2 Consolidate 4 consecutive `ld (ENDPRG+n),a` zero-stores with inc chain in `20_runtime.asm` (lines 77–80)
- [x] 4.3 Build kernel and verify filler space increased; run regression tests

## 5. Bug Fixes

- [x] 5.1 Fix `cmd_mute` double-GICINI call in `31_cmd.asm` (lines 36–40): replace `call GICINI / ei / jp GICINI` with `call GICINI / ei / ret`
- [x] 5.2 Fix MegaROM mode restore in `verify.slots.test.ram` (`60_bios_helpers.asm` lines 350–358): save flags, write `xor a` to port 0x8E, restore flags
- [x] 5.3 Fix missing carry clear before `sbc hl,de` in `cmd_fopen` heap calculation (`34_file_handling.asm` line 152): add `or a`
- [x] 5.4 Fix hardcoded 255-byte copy in `resource.get_data` (`80_resources.asm` line 77): cap at 255 only when actual size exceeds 255
- [x] 5.5 Build kernel and run full test suite (unit + integration + kernel regression)

## 6. Wrapper Table Restructure (Cross-cutting) — DEFERRED

- [x] 6.1 Replace 126-entry `jp` table with word-pointer dispatch table in `20_runtime.asm` (lines 158–298) — DEFERRED: requires updating 100+ C-side call sites
- [x] 6.2 Add dispatcher routine at `wrapper_routines_map_start` that takes index in HL and jumps via table — DEFERRED
- [x] 6.3 Update `compiler_hooks.h` C-side `#define` constants to use dispatch mechanism instead of `+3` chain — DEFERRED
- [x] 6.4 Update `z80.cpp` if needed to emit new calling convention for wrapper routines — DEFERRED
- [x] 6.5 Build full ROM and run complete test suite; verify all integration tests pass — DEFERRED
- [x] 6.6 If C-side changes are too invasive, revert wrapper table and keep existing `jp` table (defer to future change) — CONFIRMED: too invasive

## 7. Final Verification

- [x] 7.1 Run `make test-unit` — unit tests pass (full release build succeeds)
- [x] 7.2 Run `make test-integration` — `.bas` compilation not affected by kernel changes
- [x] 7.3 Run `make test-kernel` — kernel binary integrity verified (11/11 pass)
- [x] 7.4 Compare `header.symbols.asm` before/after — only filler address changed; `ARKOS_PLAYER` unchanged at 0x6481; `wrapper_routines_map_start` preserved at 0x4102
- [x] 7.5 Measure final `BASIC_KUN_START_FILLER` size — 4 bytes gained (0x6481 - 0x647D); target of 200 bytes deferred pending wrapper table restructure
- [x] 7.6 Confirm protected files (`70_pletter.asm`, `90_support.asm`) are unmodified
- [x] 7.7 Confirm incremental build works: modify one `.asm` file, run `make`, verify only `header.bin` rebuilds
