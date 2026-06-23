## 1. Domain Layer — CompileMode Enum and Output Naming

- [ ] 1.1 Add `ASCII16X` to `CompileMode` enum after `ASCII16` (`src/domain/options/build_options.h`)
- [ ] 1.2 Add `ASCII16X` to `setInputFilename()` condition for `[ASCII16X]` suffix and `megaROM = true` (`src/domain/options/build_options.cpp`)
- [ ] 1.3 Add `ASCII16X` case to `getCompileModeShortName()` returning `"ASCII16X"` (`src/domain/options/build_options.cpp`)
- [ ] 1.4 Add `ASCII16X` case to `getCompileModeLongName()` returning `"ASCII16X MegaROM"` (`src/domain/options/build_options.cpp`)

## 2. Kernel Assembly — Signature Space Reservation

- [ ] 2.1 Insert `ds 8, 0x00` between `db 'MSXB2R'` and `INIT1:` in `src/infrastructure/kernel/asm/src/header/20_runtime.asm` to reserve 8 bytes at ROM offset 0x0010; this shifts INIT1 from 0x4010 to 0x4018
- [ ] 2.2 Regenerate kernel binary and symbols by running `make header` (pasmo + xxd -i); `header.h` and `header.symbols.asm` auto-update with shifted addresses; `BIN_HEADER_BIN_LEN` remains at 32768 (KERNEL_END_FILLER absorbs the 8-byte delta)

## 3. CLI Layer — Flags, Help Text, and Status Messages

- [ ] 3.1 Add `-7`/`--ascii16x` CLI flags setting `compileMode = CompileMode::ASCII16X` (`src/cli/options/build_options_setup.cpp`)
- [ ] 3.2 Add `-7 or --ascii16x = ASCII16-X MegaROM compile mode` entry to help text in `src/cli/appinfo.h` (info_help, under compile options)
- [ ] 3.3 Add `ASCII16X` case to compile-mode info messages in `src/cli/main.cpp` (compile phase: `"Compiling for MegaROM format (ASCII16-X mapper)"`)
- [ ] 3.4 Add `ASCII16X` case to summary messages in `src/cli/main.cpp` (summary phase: `"MegaROM mode activated (ASCII16-X mapper)."`)

## 4. Application Layer — ROM Builder (fixAscii16Mapper guard + signature write)

- [ ] 4.1 Extend `fixAscii16Mapper()` condition in `src/application/builder/rom.cpp` from `compileMode == CompileMode::ASCII16` to `compileMode == CompileMode::ASCII16 || compileMode == CompileMode::ASCII16X`
- [ ] 4.2 Add signature write for ASCII16X mode in `fixAscii16Mapper()`: `if (compileMode == ASCII16X) memcpy(pages[0].data() + 0x0010, "ASCII16X", 8);` — writes the 8-byte signature at ROM offset 0x0010

## 5. Application Layer — Symbols (LOADER address update)

- [ ] 5.1 Update hardcoded `"LOADER"` address from `"4010"` to `"4018"` in `src/application/symbols/strategies/context/symbol_export_context.cpp` to match the shifted INIT1 entry point

## 6. Compiler — Code Size Overflow Check

- [ ] 6.1 Add bounds check in `ICpuOpcodeWriter::addCodeByte()` (`src/application/compiler/helpers/cpu/cpu_opcode_writer.cpp`): before incrementing `code_pointer`, verify `code_pointer < code_size_limit`; if exceeded, set `context->error = true` and log error `"Compiled code exceeds COMPILE_CODE_SIZE limit of X bytes"`
- [ ] 6.2 Add a `bool error` flag to `CpuWorkspaceContext` (`src/application/compiler/helpers/cpu/cpu_workspace_context.h`) and initialize to `false` in `clear()` (`cpu_workspace_context.cpp`)
- [ ] 6.3 In `Compiler::write()` (`src/application/compiler/compiler.cpp`), check `context->cpu->context->error` before the main copy loop; if true, return 0 and log error via `context->logger`

## 7. Unit Tests

- [ ] 7.1 Add test cases for `-7` and `--ascii16x` CLI flags in `tests/unit/src/test_options.cpp`: verify `compileMode == ASCII16X`, `megaROM == true`, output filename contains `[ASCII16X]`
- [ ] 7.2 Add test case for ASCII16X ROM build in `tests/unit/src/test_rom.cpp`: verify compilation + ROM building succeeds in ASCII16X mode and the 8-byte signature `"ASCII16X"` is present at ROM offset 0x0010
- [ ] 7.3 Verify non-ASCII16X modes do NOT have the signature at offset 0x0010 (bytes are 0x00)

## 8. Integration Tests

- [ ] 8.1 Add `--ascii16x` compilation target to `tests/integration/Makefile` (alongside existing `--ascii16` target)
- [ ] 8.2 Run integration tests and verify no regressions

## 9. Verification

- [ ] 9.1 Run `make test-unit` and verify all unit tests pass
- [ ] 9.2 Run `make release` and verify build succeeds without warnings
- [ ] 9.3 Manual verification: compile a test program with `--ascii16x` and verify output ROM has `[ASCII16X]` suffix, kernel has `srl a` in MR_CHANGE_SGM, 7 patch points applied, and `"ASCII16X"` string at offset 0x0010 (hex `41 53 43 49 49 31 36 58`)
- [ ] 9.4 Verify `make header` regenerated files: `INIT1 EQU 04018H` in `header.symbols.asm`, `BIN_HEADER_BIN_LEN = 32768` in `header.h`
