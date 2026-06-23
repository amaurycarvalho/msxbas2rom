## 1. Domain Layer — CompileMode Enum and Output Naming

- [ ] 1.1 Add `Konami4` before `KonamiSCC` in `CompileMode` enum (`src/domain/options/build_options.h:25`)
- [ ] 1.2 Add `Konami4` to `setInputFilename()` condition for `[Konami]` suffix and `megaROM = true` (`src/domain/options/build_options.cpp:50-51`)
- [ ] 1.3 Add `Konami4` case to `getCompileModeShortName()` returning `"Konami"` (`src/domain/options/build_options.cpp:60`)
- [ ] 1.4 Add `Konami4` case to `getCompileModeLongName()` returning `"Konami MegaROM"` (`src/domain/options/build_options.cpp:74`)

## 2. CLI Layer — Flags, Help Text, and Status Messages

- [ ] 2.1 Add `-4`/`--konami` CLI flags setting `compileMode = CompileMode::Konami4` (`src/cli/options/build_options_setup.cpp`, after KonamiSCC block)
- [ ] 2.2 Add `-4 or --konami = Konami MegaROM compile mode` entry to help text in `src/cli/appinfo.h` (info_help, under compile options)
- [ ] 2.3 Update compile-mode info message in `src/cli/main.cpp:250-256` to say `"Konami4 (without SCC)"` for Konami4 mode
- [ ] 2.4 Update summary message in `src/cli/main.cpp:311-316` to distinguish Konami4 (no SCC) from KonamiSCC (with SCC)

## 3. Application Layer — ROM Builder Patch

- [ ] 3.1 Rename method declaration and comment in `src/application/builder/rom.h:51-52`: `fixIfKonamiSCC` → `fixKonamiMapper`, update comment to `//! @brief Fix kernel if Konami format`
- [ ] 3.2 Rename method definition in `src/application/builder/rom.cpp:210-234`: `fixIfKonamiSCC` → `fixKonamiMapper`, extend condition from `KonamiSCC` to `KonamiSCC || Konami4`, change error message from `"Konami SCC ROM format adjust"` to `"Konami ROM format adjust"`
- [ ] 3.3 Update call site in `Rom::addKernel()` (`src/application/builder/rom.cpp:122`): `fixIfKonamiSCC()` → `fixKonamiMapper()`

## 4. Unit Tests

- [ ] 4.1 Add test cases for `-4` and `--konami` CLI flags in `tests/unit/src/test_options.cpp` (verify compileMode = Konami4, megaROM = true, output filename contains `[Konami]`)
- [ ] 4.2 Add test case for Konami4 ROM build with patched kernel addresses (verify `fixKonamiMapper` patches 11 locations, addresses at 0x9000 and 0xB000) in `tests/unit/src/test_rom.cpp`

## 5. Integration Tests

- [ ] 5.1 Update `tests/integration/Makefile` to compile MEGAROM test programs with `--konami` flag
- [ ] 5.2 Run integration tests and verify no regressions

## 6. Verification

- [ ] 6.1 Run `make test-unit` and verify all unit tests pass
- [ ] 6.2 Run `make test-integration` and verify all integration tests pass
- [ ] 6.3 Run `make release` and verify build succeeds without warnings
- [ ] 6.4 Manual verification: compile a test program with `--konami` and verify output ROM byte pattern at known segment-switch offsets
