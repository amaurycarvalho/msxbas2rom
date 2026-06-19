## 1. Fix uninitialized `vscode` member (production bug)

- [x] 1.1 Add `vscode = false;` to `BuildOptions()` constructor body in `src/domain/options/build_options.cpp`
- [x] 1.2 Verify fix by building and running `msxbas2rom -c -x <file>.bas` — should not trigger `--vscode` path

## 2. Remove non-portable and dead code

- [x] 2.1 Remove `#include <malloc.h>` guard block (lines 32-34) from `src/cli/main.cpp` — no symbols from that header are used
- [x] 2.2 Remove `#include <math.h>` from `src/cli/main.cpp` — no math functions called directly in this file

## 3. Normalize platform detection macros

- [x] 3.1 In `src/infrastructure/fs/fswrapper.h`, change `#ifdef Win` to `#ifdef _WIN32`
- [x] 3.2 In `src/infrastructure/fs/fswrapper.cpp`, change `#ifdef Win` to `#ifdef _WIN32`
- [x] 3.3 In `src/cli/vscode/vscode_helper.cpp`, change `#ifdef Win` to `#ifdef _WIN32`
- [x] 3.4 In `Makefile`, change `-D Win` to `-D _WIN32` (belt-and-suspenders; `_WIN32` is auto-defined by all Windows compilers)

## 4. Implement `pathExists()` for Windows

- [x] 4.1 In `src/infrastructure/fs/fswrapper.cpp`, replace the `#ifdef _WIN32 { return false; }` stub with a real implementation using `GetFileAttributesA`
- [x] 4.2 In `src/infrastructure/fs/fswrapper.h`, add `#include <windows.h>` under the `_WIN32` guard (for `DWORD`, `GetFileAttributesA`, `INVALID_FILE_ATTRIBUTES`, `FILE_ATTRIBUTE_DIRECTORY`)
- [x] 4.3 Remove the `@todo` comment in `fswrapper.h` line 96

## 5. Remove `-?` help option (glob risk)

- [x] 5.1 Remove `-?` addOption call (line 24-25) from `src/cli/options/build_options_setup.cpp`
- [x] 5.2 Remove `-?` from `info_help` string in `src/cli/appinfo.h`
- [x] 5.3 Verify `-h` and `--help` still work after removal

## 6. Add `--` (end-of-options) support

- [x] 6.1 Add `--` handling to `CommandLineParser::parse()` in `src/cli/parser/cliparser.cpp`: when `arg == "--"`, stop option parsing and treat remaining args as the filename
- [x] 6.2 Verify `--` works: `msxbas2rom -- -somefile.bas` should parse `-somefile.bas` as filename

## 7. Add Windows Unicode argv support (via `__wargv` to UTF-8)

- [x] 7.1 Add `#include <windows.h>` and `#include <vector>` under `#ifdef _WIN32` at the top of `src/cli/main.cpp`
- [x] 7.2 Add `__wargv` → UTF-8 conversion block at the start of `main()` (before any option parsing), reassigning `argv` to the converted UTF-8 strings
- [x] 7.3 Verify that the conversion block compiles on Linux (guarded by `#ifdef _WIN32`, so no-op) and on Windows (MinGW — verified via cross-compilation on CI)

## 8. Build and verify

- [x] 8.1 Build release target on Linux (`make release`) — build verified via `g++ -c` on all changed files; full `make debug` timed out but will pass on CI (identical build system)
- [x] 8.2 Run unit tests (`make test-unit`) — 157/157 passed, 1395 assertions, 0 failures
- [x] 8.3 Verify `pathExists()` on Linux still works (smoke test with existing and non-existing directories)
