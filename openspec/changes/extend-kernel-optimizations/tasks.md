## 1. Comparison Routine Optimizations

- [ ] 1.1 Replace `intCompareGT` body with `ex de,hl` + `jp intCompareLT` in `21_logic_pack.asm` (lines 45-54)
- [ ] 1.2 Replace `intCompareGE` body with `ex de,hl` + `jp intCompareLE` in `21_logic_pack.asm` (lines 56-67)
- [ ] 1.3 Build kernel and verify `intCompareGT` and `intCompareGE` entry points resolve correctly in `header.symbols.asm`
- [ ] 1.4 Run integration tests that exercise integer comparisons (MATH/*.bas)

## 2. Instruction-Level Optimizations

- [ ] 2.1 Replace four `sla a` with four `add a, a` in `cmd_setfnt.default_colors.1` (`31_cmd.asm` lines 495-499)
- [ ] 2.2 Replace `ld (MV_DPTR),hl` with `push hl` and `ld hl,(MV_DPTR)` with `pop hl` in `XBASIC_COPY_TO` (`30_basic_helpers.asm` lines 441, 458)
- [ ] 2.3 Build kernel and verify filler space increased; run integration tests (GRAPH/*.bas for COPY_TO, font/screen tests)

## 3. Dead Code Removal

- [ ] 3.1 Remove commented-out Panasonic turbo check from `cmd_turbo` (`31_cmd.asm` lines 531-533)
- [ ] 3.2 Remove commented-out Panasonic turbo check from `usr2_turbo` (`32_usr.asm` lines 108-110)
- [ ] 3.3 Remove commented-out MSX2 VDP fallback from `gfxVDP.set` (`60_bios_helpers.asm` lines 927-931)
- [ ] 3.4 Build kernel and confirm no assembly errors for removed code

## 4. Final Verification

- [ ] 4.1 Run `make test-unit` — all unit tests pass
- [ ] 4.2 Run `make test-integration` — all `.bas` files compile without errors
- [ ] 4.3 Compare `header.symbols.asm` before/after — verify only filler addresses and the affected routine addresses changed
- [ ] 4.4 Verify `intCompareGT` and `intCompareGE` entry points are at same addresses (label unchanged)
- [ ] 4.5 Verify `intCompareLT` address is unchanged (delegation target must not move)
- [ ] 4.6 Verify protected files (`70_pletter.asm`, `90_support.asm`) are unmodified
- [ ] 4.7 Confirm incremental build works: modify one affected `.asm` file, run `make`, verify only `header.bin` rebuilds
