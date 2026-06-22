## 1. Assembly — split header.bin into virtual table + kernel

- [x] 1.1 Restructure `20_runtime.asm`: prepend `org 0000h` + `wrapper_routines_map_table` + `DEFS 0x4000 - $` before the existing ROM header
- [x] 1.2 Remove the 5 `nop` alignment bytes (lines 157-161) since the table is no longer in the kernel section
- [x] 1.3 Remove the `org 4000h` from line 5 (the DEFS padding naturally reaches logical 0x4000)
- [x] 1.4 Verify forward references work: `dw` entries in the first half reference labels defined in later includes (`21_logic_pack.asm` through `90_support.asm`)

## 2. Build system — update size check

- [x] 2.1 Update `kernel/asm/Makefile`: change `LIMIT=16384` to `LIMIT=32768`
- [x] 2.2 Update the excess calculation message to reflect the new limit context (total binary 0x8000)
- [x] 2.3 Rebuild kernel: `make` in `src/infrastructure/kernel/asm/` and verify `header.bin` is ~32768 bytes and `header.h` contains `bin_header_bin_len = 32768`

## 3. C++ constants — update dispatch table address

- [x] 3.1 Change `def_wrapper_routines_map_table` from `0x4102` to `0x0000` in `compiler_hooks.h`
- [x] 3.2 Update the warning comment about checking `header.symbols.asm` to reference the new virtual table location

## 4. C++ address resolution — adapt getKernelCallAddr

- [x] 4.1 Add 0x0000-0x3FFF range handling: when address is within table bounds, read `bin_header_bin[address] | (bin_header_bin[address + 1] << 8)` with no `- 0x4000` offset
- [x] 4.2 Change 0x4000-0x7FFF range: use `bin_header_bin[address]` directly instead of `bin_header_bin[address - 0x4000]`
- [x] 4.3 Verify `addKernelCall` and `addKernelCallNZ` still resolve correctly through the updated logic

## 5. C++ MegaROM — simplify MR target resolution

- [x] 5.1 Replace `bin_header_bin[def_wrapper_routines_map_table - 0x4000 + DISP_MR_CALL * 2]` with `bin_header_bin[DISP_MR_CALL * 2]`
- [x] 5.2 Apply same simplification to `DISP_MR_JUMP` and `DISP_MR_GET_DATA` target resolutions
- [x] 5.3 Verify `extern unsigned char bin_header_bin[]` declaration is needed (not included via any header in compiler.cpp)

## 6. ROM builder — copy only second half

- [x] 6.1 Change `memcpy(pages[0].data(), bin_header_bin, bin_header_bin_len)` to `memcpy(pages[0].data(), bin_header_bin + 0x4000, 0x4000)` in `rom.cpp`
- [x] 6.2 Verify `fixIfKonamiSCC` still works — the pattern-based scan (offset 0xDB-0x3FFF) operates on the kernel portion bytes which are shifted but pattern-identical

## 7. Rebuild and verify

- [x] 7.1 Full project rebuild: `make release` — compiles successfully, binary at `bin/Release/msxbas2rom`
- [x] 7.2 Run kernel tests: `tests/kernel/check_kernel.sh` — 11 passed, 0 failed (test updated for 32768-byte binary)
- [x] 7.3 Run integration tests: 82 plain ROMs generated, 1 MTF syntax error pre-existing, MegaROM test compiles successfully
- [x] 7.4 Verify ROM output: kernel page starts with `AB` signature and `MSXB2R` marker in both plain and MegaROM formats
- [x] 7.5 Verify `header.symbols.asm` shows `wrapper_routines_map_table` at address `0x0000`
