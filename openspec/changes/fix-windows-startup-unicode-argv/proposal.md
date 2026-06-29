## Why

Windows release builds started returning silently before printing help/version output after the startup Unicode argv conversion introduced in v1.0.0.0. The CLI must start reliably on Windows and must not perform unsafe dynamic allocation before validating command-line conversion results.

## What Changes

- Make Windows startup argument handling fail-safe before CLI parsing begins.
- Preserve UTF-8 command-line support for Windows filenames when Unicode conversion succeeds.
- Avoid dangerous allocation from `WideCharToMultiByte()` failure paths, especially `len - 1` when `len == 0`.
- Ensure informational invocations such as no arguments, `-h`, `--help`, `-v`, and `--version` print normally on Windows.
- Add focused tests or build/run validation for Windows startup behavior where the project tooling allows it.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `cli`: Windows CLI startup must safely initialize argument handling and print informational output instead of crashing or returning silently.

## Impact

- `src/cli/main.cpp` - Windows `_WIN32` startup argv conversion and failure handling.
- `src/cli/main.h` - only if a small helper declaration is needed for testability or separation.
- `src/cli/parser/cliparser.cpp` and `src/cli/options/build_options_setup.cpp` - no expected behavior change, but may be touched by tests.
- Tests/build validation - add or extend coverage for no-argument/help/version startup paths and conversion failure safety.
- Release confidence - validates regression introduced after v0.3.4.2 and first observed in Windows `.exe` builds from v1.0.0.0 onward.
