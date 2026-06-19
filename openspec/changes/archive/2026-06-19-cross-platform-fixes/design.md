## Context

The project is built on Linux (GCC), macOS (GCC), and Windows (MinGW via GitHub Actions). The architecture spec requires C++11 cross-platform compatibility. This change targets **release 1.0.0.0** (current version in `appinfo.h`). Current code has:

1. An uninitialized member (`vscode`) that caused a production crash on Windows
2. Platform-specific headers (`<malloc.h>`) with no corresponding MSVC path
3. Non-standard Windows detection (`#ifdef Win` instead of `#ifdef _WIN32`)
4. A known stub (`pathExists()` returns `false` on Windows)
5. Shell-edges: `-?` glob expansion in POSIX shells, no `--` terminator, no Unicode path support
6. Include guard `#ifndef BUILD_OPTIONS_CPP` in `.cpp` files (harmless but noisy)

## Goals / Non-Goals

**Goals:**
- Fix the `vscode` uninitialized-member bug that blocks Windows users
- Make the code compilable with MSVC by removing non-portable includes and using standard platform macros
- Fill the `pathExists()` Windows gap
- Improve CLI robustness across shells (bash, zsh, PowerShell, cmd.exe)
- Clean up noise: dead includes, `.cpp` include guards

**Non-Goals:**
- Adding CMake or any new build system (Makefile stays)
- Full MSVC CI pipeline (MinGW CI is sufficient; MSVC compilation is a correctness gate, not a delivery target)
- Rewriting the CLI parser framework
- Adding `wmain()` / full Unicode rework (scope-limited to making `char* argv[]` paths work correctly on Windows via the ANSI codepage, which matches current MinGW behavior)

## Decisions

### D1 — Add `vscode = false` to `BuildOptions` constructor

**Why**: Root cause of the Windows crash. Minimal change — one line in the existing initialization block. No behavioral change on platforms where the memory happened to be zero.

**Alternatives considered**: C++11 in-class member initializer (`bool vscode = false;`). Rejected for consistency — the existing codebase initializes all bools in the constructor body, not inline.

### D2 — Remove `#include <malloc.h>` entirely

**Why**: Zero symbols from `<malloc.h>` are used in `main.cpp`. It was the only file including it. On MSVC, this header doesn't exist.

**Note**: Functions like `malloc_trim` are only relevant for memory-constrained embedded scenarios; this is a CLI tool on desktop OSes.

### D3 — Replace `#ifdef Win` with `#ifdef _WIN32` everywhere

**Files affected**: `fswrapper.h`, `fswrapper.cpp`, `vscode_helper.cpp`

**Why**: `_WIN32` is the standard macro defined by all Windows C/C++ compilers (MSVC, MinGW, Clang, ICC). `Win` was MinGW-specific and passed via `-D Win` in the Makefile.

**Makefile**: Change `-D Win` to `-D _WIN32` for backward compat, though `_WIN32` is auto-defined by MinGW too. This is a belt-and-suspenders measure during the transition.

### D4 — Implement `pathExists()` for Windows via `GetFileAttributesA`

**Current**: `#ifdef Win { return false; }` — a documented TODO.

**Why**: `GetFileAttributesA` is the simplest Win32 API to check path existence and directory-ness. It works with ANSI paths (matching the rest of the codebase's `char*` / `string` usage).

**Constraints**: Must include `<windows.h>` only under `_WIN32` to not pollute other platforms.

### D5 — Remove `-?` short option entirely

**Why**: `?` is a single-character glob wildcard in POSIX shells (bash, zsh, sh, dash). When a directory contains a single-character filename (e.g., `-x`), `msxbas2rom -?` expands to `msxbas2rom -x` before the program even runs — an unpredictable and confusing failure. PowerShell and cmd.exe are not affected, but removing `-?` is the only way to guarantee safety on POSIX shells.

**Backward compat**: `-?` was a minor convenience alias for `-h`. The main `-h` / `--help` options remain unchanged. Users of `-?` need only replace it with `-h` or `--help`. Help text in `appinfo.h` and `build_options_setup.cpp` must be updated to remove references.

**Files affected**:
- `src/cli/options/build_options_setup.cpp` — remove the `-?` `addOption` call (line 24-25)
- `src/cli/appinfo.h` — remove `-?` from `info_help` string

### D6 — Add `--` (end-of-options) support to `CommandLineParser::parse`

**Implementation**: In the argument loop, when `arg == "--"`, stop processing options and treat all remaining arguments as positional (filename). This matches POSIX convention.

**Why**: Enables users to pass filenames starting with `-` (rare but correct).

**Change scope**: One condition in `cliparser.cpp` — no interface changes.

### D7 — Convert `__wargv` to UTF-8 at entry on Windows

**Analysis**: Windows native encoding is UTF-16. MinGW (and MSVC) provide `__wargv` — the wide-character version of `argv` — alongside the ANSI `argv`. The ANSI version is encoded in the system's active code page (e.g., CP-1252 on Western European systems), which cannot represent characters outside that code page (CJK, Cyrillic, etc.). Converting `__wargv` to UTF-8 at startup preserves all Unicode filenames.

**How**:

```
main(argc, argv)
 │
 ├─ #ifdef _WIN32
 │    ┌──────────────────────────────────┐
 │    │  wchar_t** __wargv → UTF-8       │
 │    │  via WideCharToMultiByte(CP_UTF8) │
 │    │  stored in vector<string>,        │
 │    │  argv reassigned to point there   │
 │    └──────────────────────────────────┘
 │
 └─ rest of main() unchanged
      (all downstream code receives UTF-8 char*)
```

**Implementation sketch** (added at the top of `main()` in `main.cpp`):

```cpp
#ifdef _WIN32
  std::vector<std::string> utf8Storage;
  std::vector<char*> utf8Argv;
  utf8Storage.reserve(argc);
  utf8Argv.reserve(argc);
  for (int i = 0; i < argc; i++) {
    wchar_t* warg = __wargv[i];
    int len = WideCharToMultiByte(CP_UTF8, 0, warg, -1, NULL, 0, NULL, NULL);
    utf8Storage.emplace_back(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, warg, -1, &utf8Storage.back()[0], len, NULL, NULL);
    utf8Argv.push_back(&utf8Storage.back()[0]);
  }
  argv = utf8Argv.data();
#endif
```

**Why this works**: `vector<string>` owns the memory for the lifetime of `main()`. After `argv` is reassigned, every downstream function (`BuildOptionsSetup::parse`, `CommandLineParser::parse`, file I/O via `fopen`, etc.) receives UTF-8 paths. On Windows, `fopen` with UTF-8 strings works correctly when the system code page is UTF-8 (Windows 10 version 1903+ with beta UTF-8 support) or when using the MinGW runtime's UTF-8 awareness.

**Trade-off**: Very old Windows versions (pre-Vista) lack `WideCharToMultiByte`, but MinGW's minimum target is Windows XP+.

**Files affected**: `src/cli/main.cpp` only — adding the conversion block and the necessary `#include <windows.h>` under `_WIN32`.

## Risks / Trade-offs

- **[Low] `-?` removal breaks scripts**: If any user script passes `-?`, it will now be rejected as "Unexpected argument". Mitigation: `-?` was a minor alias for `-h`; the error message is clear, and the fix is trivial (`-?` → `-h`). Noted in changelog.
- **[Low] `_WIN32` transition**: `#ifdef Win` currently works because the Makefile passes `-D Win`. Switching to `_WIN32` is safe but one contributor's local build might still rely on `-D Win`. Mitigation: keep `-D Win` in Makefile for one release cycle, using `#if defined(_WIN32) || defined(Win)` as a transitional guard.
- **[Low] Unicode conversion on old Windows**: `WideCharToMultiByte(CP_UTF8)` requires Windows Vista+. MinGW's minimum target is XP, but XP is EOL since 2014. Acceptable trade-off for modern Unicode support.
