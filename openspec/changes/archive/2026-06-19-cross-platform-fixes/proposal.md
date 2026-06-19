## Why

The project's architecture spec requires C++11 compatibility with Linux, Windows, and macOS, and the CI builds on all three via GitHub Actions. However, production use on Windows revealed a crashing bug (uninitialized `vscode` member), and code review found several non-standard patterns that would fail under MSVC or in edge-case shell environments.

These gaps erode the portability guarantee and will cause more user-facing failures as the user base grows beyond Linux.

## What Changes

- Fix uninitialized `vscode` bool in `BuildOptions` constructor (root cause of the "VSCode already initialized" error on Windows)
- Remove dead `#include <malloc.h>` from `main.cpp` (glibc-only, no symbols used)
- Normalize Windows detection from `#ifdef Win` to standard `#ifdef _WIN32` across all platform-guarded blocks
- Implement `pathExists()` for Windows (currently returns `false` — a known TODO)
- Remove `-?` help option (glob expansion risk in POSIX shells outweighs convenience)
- Add `--` (end-of-options) support to `CommandLineParser` so filenames starting with `-` can be passed
- Add Windows Unicode filename support via `__wargv` to UTF-8 conversion at startup

No new capabilities are introduced — this change fixes existing non-compliance with the cross-platform requirement in `openspec/specs/architecture/spec.md`.

This change targets **release 1.0.0.0** (current version in `appinfo.h`).

## Capabilities

### New Capabilities
None — this is a compliance-fixing change.

### Modified Capabilities
- `cli`: Command-line parser gains `--` support; `-?` removed; Windows builds get Unicode argv and proper path-existence checking

## Impact

- `src/cli/main.cpp` — remove `#include <malloc.h>`, remove `#include <math.h>`; add Windows Unicode argv conversion at entry
- `src/cli/parser/cliparser.cpp` — add `--` handling
- `src/cli/options/build_options_setup.cpp` — remove `-?` short-option registration; update help text accordingly
- `src/cli/appinfo.h` — remove `-?` from help text
- `src/domain/options/build_options.cpp` — add `vscode = false` to constructor
- `src/infrastructure/fs/fswrapper.h` — change `#ifdef Win` to `#ifdef _WIN32`, add `#include <windows.h>` for Windows path functions
- `src/infrastructure/fs/fswrapper.cpp` — implement `pathExists()` for `_WIN32` path
- `src/cli/parser/cliparser.h` — no changes needed (interface unchanged)
