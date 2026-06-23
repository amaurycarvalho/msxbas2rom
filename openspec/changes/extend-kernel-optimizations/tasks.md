## 1. Comparison Routine Optimizations (21_logic_pack.asm)

- [x] 1.1 Replace `intCompareGT` body with `ex de,hl` + `jp intCompareLT` (lines 45-54)
- [x] 1.2 Replace `intCompareGE` body with `ex de,hl` + `jp intCompareLE` (lines 56-67)
- [x] 1.3 Build kernel and verify `intCompareGT` and `intCompareGE` entry points resolve correctly in `header.symbols.asm`
- [x] 1.4 Run integration tests that exercise integer comparisons (MATH/*.bas)

## 2. FloatToBCD Optimizations (21_logic_pack.asm)

- [x] 2.1 Replace `ld a,h` / `res 7,a` / `ld h,a` with `res 7,h` in `FloatToBCD` (lines 175-177)
- [x] 2.2 Replace 4× `sla e` with `ld a,e` / 4× `add a,a` / `ld e,a` in `FloatToBCD.add.2` (lines 246-249)
- [x] 2.3 Run floating-point integration tests (MATH/floats/*.bas, any BCD tests)

## 3. intSHR Loop Optimization (21_logic_pack.asm)

- [x] 3.1 Remove dead `xor a` from `intSHR.1` loop (line 111)
- [x] 3.2 Verify shift behavior with edge-case inputs (negative values, multi-bit shifts)
- [x] 3.3 Run SHR-related integration tests

## 4. XBASIC Helper Routine Optimizations (30_basic_helpers.asm)

- [x] 4.1 Replace `cpl` + 2× `ld (CURLIN),a` with `ld hl,0xFFFF` / `ld (CURLIN),hl` in `XBASIC_END` (lines 157-159)
- [x] 4.2 Replace `bit 7,h` / `jr z` with `jp p` in `XBASIC_TAB` (lines 635-636)
- [x] 4.3 Remove dead commented-out source lines in `XBASIC_END` / `XBASIC_INIT` (lines 153-154, 160, 201-202)
- [x] 4.4 Run integration tests for END, TAB statements

## 5. XBASIC_COPY_TO Stack Optimization (30_basic_helpers.asm)

- [x] 5.1 Replace `ld (MV_DPTR),hl` with `push hl` (line 441)
- [x] 5.2 Replace `ld hl,(MV_DPTR)` with `pop hl` (line 458)
- [x] 5.3 Verify stack balance: audit intervening code (lines 442-457) for zero push/pop operations
- [x] 5.4 Run COPY TO integration tests (GRAPH/*.bas)

## 6. CMD Statement Optimizations (31_cmd.asm)

- [x] 6.1 Replace 2× `sla a` with 2× `add a,a` in `cmd_play` (lines 14-15)
- [x] 6.2 Replace 4× `sla a` with 4× `add a,a` in `cmd_setfnt.default_colors.1` (lines 498-502)
- [x] 6.3 Replace 7× `inc hl` with `ld de,7` / `add hl,de` in `cmd_wrtfnt` (lines 90-96)
- [x] 6.4 Replace 4× `sla a` with 4× `add a,a` in `cmd_page.delay` (lines 1284-1287)
- [x] 6.5 Replace 4× `sla a` with 4× `add a,a` in `cmd_mtf.palette.copy.to_vram.loop` (lines 1716-1719)
- [x] 6.6 Run PLAY, SETFNT, WRTFNT, SETPAGE, MTF integration tests

## 7. Dead Code Removal

- [x] 7.1 Remove commented-out Panasonic turbo check from `cmd_turbo` (`31_cmd.asm` lines 534-536)
- [x] 7.2 Remove commented-out Panasonic turbo check from `usr2_turbo` (`32_usr.asm` lines 108-110)
- [x] 7.3 Remove commented-out MSX2 VDP fallback from `gfxVDP.set` (`60_bios_helpers.asm` lines 913-917)
- [x] 7.4 Remove commented-out `MAXFIL`/`NLONLY`/`DSKDIS` lines from `XBASIC_END`/`XBASIC_INIT` (`30_basic_helpers.asm` lines 153-154, 160)
- [x] 7.5 Remove commented-out BIOS call lines from `XBASIC_INIT` (`30_basic_helpers.asm` lines 201-202)
- [x] 7.6 Build kernel and confirm no assembly errors for removed code

## 8. Final Verification

- [x] 8.1 Run `make test-unit` — all unit tests pass (168/168, 1429/1429 assertions)
- [x] 8.2 Run `make test-integration` — all `.bas` files compile without errors
- [x] 8.3 Compare `header.symbols.asm` before/after — verify only filler addresses and affected routine addresses changed
- [x] 8.4 Verify `intCompareGT` and `intCompareGE` entry points are at same addresses (label unchanged)
- [x] 8.5 Verify `intCompareLT` address is unchanged (delegation target must not move)
- [x] 8.6 Verify protected files (`70_pletter.asm`, `90_support.asm`) are unmodified
- [x] 8.7 Confirm incremental build works: modify one affected `.asm` file, run `make`, verify only `header.bin` rebuilds
- [x] 8.8 Verify byte savings: compare `BASIC_KUN_START_FILLER` size before/after — 48 bytes gained (expected ~47)
