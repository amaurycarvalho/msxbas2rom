## 0. Kernel-space gate

- [x] 0.1 Measure the current free space: `BASIC_KUN_START_FILLER` is `0x647B` and `ARKOS_PLAYER` is `0x6481`, so only 6 bytes are free before the fixed player block. Confirm that adding the new `cmd_rsctoram` code cannot fit without reclaiming or relocating bytes
- [x] 0.2 Produce the space-reclamation proposal: `kernel-space-proposal.md` selects a layout-preserving **fixed/resident** relocation of `cmd_rsctoram` into the former `31-cmd-input` slot (`0x5492-0x5537`). The routine is 86 bytes plus a 79-byte filler (165 bytes total); `31-cmd-input` moves to the extra relocatable modules. Overflow placement was rejected because page-2 resource mapping would unmap the routine on MegaROM
- [x] 0.3 Apply the space plan: remove the 26-byte body from `31-cmd-game-graphics.asm`, insert `DEFS 26, 0x00`, create `31-cmd-rsctoram.asm` (86 bytes code + `DEFS 79`), swap the `31-cmd-input` include in `31_cmd.asm`, add `cmd_pad equ 0` in `20_runtime.asm`, move `31-cmd-input` to `extra_modules` in `relocatable-modules.json`, and add `31-cmd-rsctoram` as fixed in `module-groups.json`
- [x] 0.4 Verify `header.bin` is exactly 32768 bytes, `ARKOS_PLAYER` remains `0x6481`, `player.int.control` remains `0x5659`, and `BASIC_KUN_START_FILLER` remains `0x647B`. Run `make test-kernel` and verify `check_kernel.sh`, `test_kernel_manifest.py`, and `check_relocation.py` all pass before continuing

## 1. Kernel — cmd_rsctoram

- [x] 1.1 Implement the extended `cmd_rsctoram` in the new **fixed** `src/infrastructure/kernel/asm/src/header/31-cmd-rsctoram.asm` to read `resource`, `dest`, `pletter`, `offset`, and `size` from the `PARM1` layout, compute `effective = min(size, resource_size - offset)` (with `size = 0` meaning full remaining), copy, and return `HL = dest`. The module MUST stay resident in page 1; do not move it to page-2 overflow. Use unique local label names. Verify with `make -C src/infrastructure/kernel/asm` that the kernel assembles, `header.bin` stays 32768 bytes, and the fixed symbols do not move
- [x] 1.2 Check the carry returned by `resource.open_and_get_address` before any copy. On an invalid resource, skip the copy, call `resource.close`, and return `HL = 0`. Verify that RAM is untouched for a missing resource
- [x] 1.3 Preserve `dest` across the uncompressed `ldir` and `resource.close`, and return it in `HL`. Verify that a successful copy returns the destination address, not the source end
- [x] 1.4 Keep setting `(DAC)` from the `PARM1` resource number before calling `resource.open_and_get_address`. Do not treat `refactor-resource-number-hl-protocol` as an active dependency
- [x] 1.5 Verify the pletter path (`pletter = 1`) still decompresses the whole resource and ignores `offset`/`size`. Verify by disassembling the routine or a targeted runtime probe

## 2. Compiler — CMD RSCTORAM handler

- [x] 2.1 Update `src/application/compiler/statements/strategies/io/cmd/handlers/io/rsctoram/compiler_cmd_rsctoram_handler.cpp` to accept 2-5 arguments, define/shared the `PARM1` offsets, and store `resource`, `dest`, `pletter`, `offset`, and `size` (defaults `pletter=0`, `offset=0`, `size=0`). Verify the generated code for the 2-, 3-, 4-, and 5-argument forms stores the expected `PARM1` values
- [x] 2.2 Reject more than five arguments with a `CMD RSCTORAM syntax error`. Verify compilation fails for a 6-argument call
- [x] 2.3 Add or update a unit test under `tests/unit/` that asserts the handler emits the `PARM1` stores for the 2-, 3-, and 5-argument forms and rejects 6 arguments
- [x] 2.4 Rebuild the compiler (`make release`) and verify the build succeeds

## 3. Tests and regression

- [x] 3.1 Run `make -C tests/integration regression PROGRAM=rsc1` and verify the existing `tests/integration/regression/baselines/MISC/rsc1.json` is unchanged. This baseline covers the 2-argument uncompressed path and the 3-argument pletter path with full RAM and VRAM text
- [x] 3.2 Run `make test-emulator` and verify `tests/dispatcher/check_page_safe_modules.py` still passes the 3-argument `CMD RSCTORAM` cases in openMSX
- [x] 3.3 Add a deterministic integration/regression program (for example `tests/integration/MISC/rsc_offset.bas`) covering: exact range, `size` larger than remaining, `offset` at/beyond the end, missing resource (no RAM write), and `size=0` (full remaining). Use `PEEK`/`PRINT` or fixed RAM probes so the result is observable
- [x] 3.4 Add the new program to `tests/integration/regression/manifest.json` with `vram.text.expected` and/or `ram_probes`; generate its baseline once with `make -C tests/integration regression-baseline PROGRAM=<name>`
- [x] 3.5 Run `make test-integration` and verify all `.bas` files still compile for ASCII8, ASCII16, ASCII16-X, and Konami
- [x] 3.6 Run `make test-kernel` and `make test-emulator` after the final kernel rewrite
- [x] 3.7 Update `tests/dispatcher/check_page_safe_modules.py`: the `31-cmd-game-graphics` fixed case currently uses `CMD RSCTORAM`, which no longer belongs to that module after the move. Switch it to a program that exercises a remaining `31-cmd-game-graphics` routine (for example `CMD PLAY`/`CMD WRTSCR`), add explicit resident runtime coverage for `31-cmd-rsctoram` on Plain ROM and MegaROM, and verify the `31-cmd-input` SAFE_CASE (now an extra module) still works with `--force-overflow-module 31-cmd-input`

## 4. Documentation

- [x] 4.1 Update the project wiki `Extended Commands` page (external) to document `CMD RSCTORAM <resource>, <dest>[, <pletter>[, <offset>[, <size>]]]` and the defaults, including `size=0` and invalid-resource behavior
- [x] 4.2 Update `doc/misc/z80-binary-code.md` (section 10.1 `CMD RSCTORAM` and the syntax summary table) to document the new optional `offset` and `size` parameters, their defaults (`0` and full remaining resource), the `size` clamp, and the pletter interaction
