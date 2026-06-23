## 1. Domain Layer — CompileMode Enum and Output Naming

- [x] 1.1 Add `ASCII16X` to `CompileMode` enum after `ASCII16` (`src/domain/options/build_options.h`)
- [x] 1.2 Add `ASCII16X` to `setInputFilename()` condition for `[ASCII16X]` suffix and `megaROM = true` (`src/domain/options/build_options.cpp`)
- [x] 1.3 Add `ASCII16X` case to `getCompileModeShortName()` returning `"ASCII16X"` (`src/domain/options/build_options.cpp`)
- [x] 1.4 Add `ASCII16X` case to `getCompileModeLongName()` returning `"ASCII16X MegaROM"` (`src/domain/options/build_options.cpp`)

## 2. Kernel Assembly — Signature Space Reservation

- [x] 2.1 Insert `ds 8, 0x00` between `db 'MSXB2R'` and `INIT1:` in `src/infrastructure/kernel/asm/src/header/20_runtime.asm` to reserve 8 bytes at ROM offset 0x0010; this shifts INIT1 from 0x4010 to 0x4018
- [x] 2.2 Add `ascii16x_patch_bugfix_ab_check:` label at `megarom_ascii8_bug_fix` in `20_runtime.asm` and add `dw ascii16x_patch_bugfix_ab_check` entry to the dispatch table
- [x] 2.3 Add `#define DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK 223` and bump `DISP_ENTRIES` to 224 in `src/application/compiler/helpers/hooks/compiler_hooks.h`
- [x] 2.4 Regenerate kernel binary and symbols by running `make header` (pasmo + xxd -i); `header.h` and `header.symbols.asm` auto-update with shifted addresses; `BIN_HEADER_BIN_LEN` remains at 32768 (KERNEL_END_FILLER absorbs the 8-byte delta)

## 3. CLI Layer — Flags, Help Text, and Status Messages

- [x] 3.1 Add `-7`/`--ascii16x` CLI flags setting `compileMode = CompileMode::ASCII16X` (`src/cli/options/build_options_setup.cpp`)
- [x] 3.2 Add `-7 or --ascii16x = ASCII16-X MegaROM compile mode` entry to help text in `src/cli/appinfo.h` (info_help, under compile options)
- [x] 3.3 Add `ASCII16X` case to compile-mode info messages in `src/cli/main.cpp` (compile phase: `"Compiling for MegaROM format (ASCII16-X mapper)"`)
- [x] 3.4 Add `ASCII16X` case to summary messages in `src/cli/main.cpp` (summary phase: `"MegaROM mode activated (ASCII16-X mapper)."`)

## 4. Application Layer — ROM Builder (fixAscii16Mapper guard + signature write)

- [x] 4.1 Extend `fixAscii16Mapper()` condition in `src/application/builder/rom.cpp` from `compileMode == CompileMode::ASCII16` to `compileMode == CompileMode::ASCII16 || compileMode == CompileMode::ASCII16X`
- [x] 4.2 Add signature write + bugfix AB-check NOP for ASCII16X mode in `fixAscii16Mapper()`: `memcpy(pages[0]+0x0010, "ASCII16X", 8)` + 14-byte NOP at `DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK` (NOPs out the `ld a,(0x8000); cp 0x41; jr nz; ld a,(0x8001); cp 0x42; jr nz` comparison block so the ASCII8 bugfix always runs)

## 5. Application Layer — Symbols (LOADER address update)

- [x] 5.1 Update hardcoded `"LOADER"` address from `"4010"` to `"4018"` in `src/application/symbols/strategies/context/symbol_export_context.cpp` to match the shifted INIT1 entry point

## 6. Compiler — Code Size Overflow Check

- [x] 6.1 Add bounds check in `ICpuOpcodeWriter::addCodeByte()` (`src/application/compiler/helpers/cpu/cpu_opcode_writer.cpp`): before incrementing `code_pointer`, verify `code_pointer < code_size_limit`; if exceeded, set `context->error = true` and skip the write
- [x] 6.2 Add a `bool error` flag to `CpuWorkspaceContext` (`src/application/compiler/helpers/cpu/cpu_workspace_context.h`) and initialize to `false` in `clear()` (`cpu_workspace_context.cpp`)
- [x] 6.3 In `Compiler::write()` (`src/application/compiler/compiler.cpp`), check `context->cpu->context->error` before the main copy loop; if true, return 0 and log error via `context->logger`

## 7. Unit Tests

- [x] 7.1 Add test cases for `-7` and `--ascii16x` CLI flags in `tests/unit/src/test_options.cpp`: verify `compileMode == ASCII16X`, `megaROM == true`, output filename contains `[ASCII16X]`
- [x] 7.2 Add test case for ASCII16X ROM build in `tests/unit/src/test_rom.cpp`: verify compilation + ROM building succeeds in ASCII16X mode and the 8-byte signature `"ASCII16X"` is present at ROM offset 0x0010
- [x] 7.3 Verify non-ASCII16X modes do NOT have the signature at offset 0x0010 (bytes are 0x00)

## 8. Integration Tests

- [x] 8.1 Add `--ascii16x` compilation target to `tests/integration/Makefile` (alongside existing `--ascii16` target)
- [x] 8.2 Run integration tests and verify no regressions

## 9. Verification

- [x] 9.1 Run `make test-unit` and verify all unit tests pass
- [x] 9.2 Run `make release` and verify build succeeds without warnings
- [x] 9.3 Manual verification: compile a test program with `--ascii16x` and verify output ROM has `[ASCII16X]` suffix, kernel has `srl a` in MR_CHANGE_SGM, 7 patch points applied, and `"ASCII16X"` string at offset 0x0010 (hex `41 53 43 49 49 31 36 58`)
- [x] 9.4 Verify `make header` regenerated files: `INIT1 EQU 04018H` in `header.symbols.asm`, `BIN_HEADER_BIN_LEN = 32768` in `header.h`
