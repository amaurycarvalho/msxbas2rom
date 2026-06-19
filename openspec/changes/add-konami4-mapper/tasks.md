## 1. Domain Layer — CompileMode Enum and Output Naming

- [ ] 1.1 Add `Konami4` to `CompileMode` enum in `src/domain/options/build_options.h`
- [ ] 1.2 Add `Konami4` to `setInputFilename()` condition in `src/domain/options/build_options.cpp` (alongside ASCII8 and KonamiSCC for `[Konami4]` suffix)
- [ ] 1.3 Add `Konami4` case to `getCompileModeShortName()` in `src/domain/options/build_options.cpp`
- [ ] 1.4 Add `Konami4` case to `getCompileModeLongName()` in `src/domain/options/build_options.cpp`

## 2. CLI Layer — Flags and Status Messages

- [ ] 2.1 Add `-4`/`--konami` CLI flags in `src/cli/options/build_options_setup.cpp`
- [ ] 2.2 Add `-4`/`--konami` entry to help text in `src/cli/appinfo.h` (under compile options)
- [ ] 2.3 Update Konami-related status messages in `src/cli/main.cpp` to mention Konami4

## 3. Application Layer — ROM Builder Patch

- [ ] 3.1 Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` in `src/application/builder/rom.h`
- [ ] 3.2 Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` in `src/application/builder/rom.cpp` and extend condition to include `Konami4`
- [ ] 3.3 Update method call in `Rom::addKernel()` in `src/application/builder/rom.cpp`

## 4. Unit Tests

- [ ] 4.1 Add test cases for `-4` and `--konami` CLI flags in `tests/unit/src/test_options.cpp`
- [ ] 4.2 Add test case for Konami4 ROM build with patched kernel addresses in `tests/unit/src/test_rom.cpp`

## 5. Integration Tests

- [ ] 5.1 Update `tests/integration/Makefile` to compile MEGAROM test programs with `--konami` flag
- [ ] 5.2 Run integration tests and verify no regressions

## 6. Verification

- [ ] 6.1 Run `make test-unit` and verify all unit tests pass
- [ ] 6.2 Run `make test-integration` and verify all integration tests pass
- [ ] 6.3 Run `make release` and verify build succeeds without warnings
- [ ] 6.4 Manual verification: compile a test program with `--konami` and verify output ROM byte pattern at known segment-switch offsets
