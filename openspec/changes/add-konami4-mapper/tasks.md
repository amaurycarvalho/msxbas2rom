## 1. Domain Layer — CompileMode Enum and Output Naming

- [x] 1.1 Add `Konami4` before `KonamiSCC` in `CompileMode` enum (`src/domain/options/build_options.h:25`)
- [x] 1.2 Add `Konami4` to `setInputFilename()` condition for `[Konami]` suffix and `megaROM = true` (`src/domain/options/build_options.cpp:50-51`)
- [x] 1.3 Add `Konami4` case to `getCompileModeShortName()` returning `"Konami"` (`src/domain/options/build_options.cpp:60`)
- [x] 1.4 Add `Konami4` case to `getCompileModeLongName()` returning `"Konami MegaROM"` (`src/domain/options/build_options.cpp:74`)

## 2. CLI Layer — Flags, Help Text, and Status Messages

- [x] 2.1 Add `-4`/`--konami` CLI flags setting `compileMode = CompileMode::Konami4` (`src/cli/options/build_options_setup.cpp`, after KonamiSCC block)
- [x] 2.2 Add `-4 or --konami = Konami MegaROM compile mode` entry to help text in `src/cli/appinfo.h` (info_help, under compile options)
- [x] 2.3 Update compile-mode info message in `src/cli/main.cpp:250-256` to say `"Konami4 (without SCC)"` for Konami4 mode
- [x] 2.4 Update summary message in `src/cli/main.cpp:311-316` to distinguish Konami4 (no SCC) from KonamiSCC (with SCC)

## 3. Application Layer — ROM Builder Patch

- [x] 3.1 Rename method declaration and comment in `src/application/builder/rom.h:51-52`: `fixIfKonamiSCC` → `fixKonamiMapper`, update comment to `//! @brief Fix kernel if Konami format`
- [x] 3.2 Rename method definition in `src/application/builder/rom.cpp:210-250`: `fixIfKonamiSCC` → `fixKonamiMapper`, extend condition from `KonamiSCC` to `KonamiSCC || Konami4`, change error message from `"Konami SCC ROM format adjust"` to `"Konami ROM format adjust"`, replace byte-scanning with dispatch-table-based patching using DISP*KONAMI_PATCH*\* entries
- [x] 3.3 Update call site in `Rom::addKernel()` (`src/application/builder/rom.cpp:122`): `fixIfKonamiSCC()` → `fixKonamiMapper()`
- [x] 3.4 Add `konami_patch_*` labels at exact segment-switch write points in kernel ASM files (`61_megarom.asm`, `20_runtime.asm`, `60_bios_helpers.asm`)
- [x] 3.5 Add `dw konami_patch_*` entries in `wrapper_routines_map_table` (`20_runtime.asm`) and corresponding `DISP_KONAMI_PATCH_*` constants in `compiler_hooks.h`
- [x] 3.6 Update `DISP_ENTRIES` from 207 to 224 (14 Konami entries + 3 ASCII16/ASCII16X entries from concurrent changes)

## 4. Unit Tests

- [x] 4.1 Add test cases for `-4` and `--konami` CLI flags in `tests/unit/src/test_options.cpp` (verify compileMode = Konami4, megaROM = true, output filename contains `[Konami]`)
- [x] 4.2 Add test case for Konami4 ROM build with patched kernel addresses (verify `fixKonamiMapper` patches all 14 locations via dispatch table, addresses at 0x8000 and 0xA000) in `tests/unit/src/test_rom.cpp`

## 5. Integration Tests

- [x] 5.1 Update `tests/integration/Makefile` to compile MEGAROM test programs with `--konami` flag
- [x] 5.2 Run integration tests and verify no regressions

## 6. Verification

- [x] 6.1 Run `make test-unit` and verify all unit tests pass
- [x] 6.2 Run `make test-integration` and verify all integration tests pass
- [x] 6.3 Run `make release` and verify build succeeds without warnings
- [x] 6.4 Manual verification: compile a test program with `--konami` and verify output ROM byte pattern at known segment-switch offsets
