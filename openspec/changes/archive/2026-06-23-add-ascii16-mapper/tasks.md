## 1. Domain Layer — CompileMode Enum and Output Naming

- [x] 1.1 Add `ASCII16` to `CompileMode` enum after `ASCII8` (`src/domain/options/build_options.h`)
- [x] 1.2 Add `ASCII16` to `setInputFilename()` condition for `[ASCII16]` suffix and `megaROM = true` (`src/domain/options/build_options.cpp`)
- [x] 1.3 Add `ASCII16` case to `getCompileModeShortName()` returning `"ASCII16"` (`src/domain/options/build_options.cpp`)
- [x] 1.4 Add `ASCII16` case to `getCompileModeLongName()` returning `"ASCII16 MegaROM"` (`src/domain/options/build_options.cpp`)

## 2. Kernel Assembly — Dispatch Table Labels and Entries

- [x] 2.1 Add `ascii16_patch_*` labels at boot bugfix NOP points in `20_runtime.asm`:
  - `ascii16_patch_bugfix_inc1` before `inc a` after patched `ld (0x6800),a`: 1-byte NOP
  - `ascii16_patch_bugfix_nopseq` before `inc a` after `ld (0x7000),a`: 4-byte NOP sequence
- [x] 2.2 Add `dw ascii16_patch_*` entries in `wrapper_routines_map_table` (`20_runtime.asm`)
- [x] 2.3 Add `DISP_ASCII16_PATCH_BUGFIX_INC1` (221) and `DISP_ASCII16_PATCH_BUGFIX_NOPSEQ` (222) in `compiler_hooks.h`
- [x] 2.4 Update `DISP_ENTRIES` from 221 to 223
- [x] 2.5 Regenerate `header.h` by running `make header` (pasmo + xxd -i)

## 3. CLI Layer — Flags, Help Text, and Status Messages

- [x] 3.1 Add `-6`/`--ascii16` CLI flags setting `compileMode = CompileMode::ASCII16` (`src/cli/options/build_options_setup.cpp`)
- [x] 3.2 Add `-6 or --ascii16 = ASCII16 MegaROM compile mode` entry to help text in `src/cli/appinfo.h`
- [x] 3.3 Update compile-mode info message in `src/cli/main.cpp` to show `"ASCII16"` for ASCII16 mode
- [x] 3.4 Update summary message in `src/cli/main.cpp` to distinguish ASCII16 from other MegaROM modes

## 4. Application Layer — ROM Builder (fixAscii16Mapper)

- [x] 4.1 Declare `fixAscii16Mapper()` method in `src/application/builder/rom.h`
- [x] 4.2 Implement `fixAscii16Mapper()` in `src/application/builder/rom.cpp`:
  - SeqReplace (9 bytes): rewrites MR_CHANGE_SGM with `push af; srl a; ld (0x7000),a; pop af; ret` at `DISP_KONAMI_PATCH_SGM_8000` label
  - ByteReplace: `DISP_KONAMI_PATCH_BUGFIX_6800` byte[+2]=0x70 (redirect port 0x6800→0x7000)
  - NOP 3B: `DISP_KONAMI_PATCH_BUGFIX_8000` (redundant second write)
  - NOP 1B: `DISP_ASCII16_PATCH_BUGFIX_INC1`
  - NOP 4B: `DISP_ASCII16_PATCH_BUGFIX_NOPSEQ`
  - SeqReplace (3 bytes): `DISP_KONAMI_PATCH_OMSX_3` → `ld (0x77FF),a` (OpenMSX mapper detection)
  - SeqReplace (3 bytes): `DISP_KONAMI_PATCH_OMSX_4` → `ld (0x77FF),a` (OpenMSX mapper detection)
- [x] 4.3 `resourceSegment` stays at `pages.size() * 2` (8KB-pair convention unchanged; kernel handles conversion)
- [x] 4.4 Call `fixAscii16Mapper()` in `addKernel()` when `compileMode == ASCII16`

## 5. Application Layer — Compiler (unchanged)

- [x] 5.1 Compiler segment math is **unchanged** — zero diff in `compiler.cpp`
- [x] 5.2 8KB→16KB segment conversion handled entirely in kernel via `srl a` in patched `MR_CHANGE_SGM`

## 6. Unit Tests

- [x] 6.1 Add test cases for `-6` and `--ascii16` CLI flags in `tests/unit/src/test_options.cpp`
- [x] 6.2 Add test case for ASCII16 ROM build with patched kernel in `tests/unit/src/test_rom.cpp`

## 7. Integration Tests

- [x] 7.1 Update `tests/integration/Makefile` to compile MEGAROM test programs with `--ascii16` flag
- [x] 7.2 Run integration tests and verify no regressions

## 8. Verification

- [x] 8.1 Run `make test-unit` and verify all unit tests pass (175/175, 1458 assertions)
- [x] 8.2 Run `make test-integration` and verify all integration tests pass
- [x] 8.3 Run `make release` and verify build succeeds without warnings
- [x] 8.4 Manual verification: compile with `--ascii16`, verify `push af; srl a; ld (0x7000),a; pop af; ret` at MR_CHANGE_SGM, single `ld (0x7000),1` in boot bugfix
