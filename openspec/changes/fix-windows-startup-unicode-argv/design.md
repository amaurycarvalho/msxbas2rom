## Context

Windows builds are produced with MinGW through the existing GitHub Actions workflow. The v1.0.0.0 startup path added an `_WIN32` block in `src/cli/main.cpp` that reads `__wargv`, converts each wide argument to UTF-8 with `WideCharToMultiByte()`, and replaces `argv` before CLI parsing.

That code runs before any diagnostic output. If `__wargv[i]` is null/invalid or `WideCharToMultiByte()` returns 0, the current `std::string(len - 1, '\0')` allocation can request a huge size and abort before `printHeader()`, `printHelp()`, or `printVersion()` can run. The result matches the observed Windows behavior: the executable returns silently or appears to dump memory while Linux remains healthy.

## Goals / Non-Goals

**Goals:**

- Make Windows startup argument conversion safe before CLI parsing.
- Preserve UTF-8 handling for Windows filenames when conversion succeeds.
- Ensure no-argument, help, and version invocations print normally on Windows.
- Avoid unsafe dynamic allocation when Windows API conversion fails.
- Keep the fix small and localized to CLI startup code.

**Non-Goals:**

- Redesign the command-line parser.
- Introduce new CLI flags or user-visible options.
- Change Linux/macOS startup behavior.
- Replace the project's C++11 toolchain or add external dependencies.
- Solve every possible Windows path encoding issue beyond startup argument safety.

## Decisions

### Decision: Isolate Windows argv conversion behind a fail-safe helper

Move the conversion logic out of the first lines of `main()` into a small helper or tightly scoped block that validates every step before replacing `argv`.

Rationale:
- Keeps `main()` readable.
- Makes failure paths explicit.
- Allows focused tests or debug instrumentation around conversion behavior.

Alternative considered: remove Unicode conversion completely and use the original narrow `argv`. This would restore startup reliability but would regress Unicode filename support added in v1.0.0.0.

### Decision: Fall back to original `argv` on conversion failure

If any Windows conversion precondition fails (`__wargv` is unavailable, an entry is null, `WideCharToMultiByte()` returns 0, or allocation fails), the CLI should continue with the original `argv` rather than aborting before informational commands can run.

Rationale:
- The narrow `argv` path is the behavior that worked in v0.3.4.2.
- Help/version/no-argument invocations do not need Unicode filename conversion.
- A degraded filename-encoding path is preferable to a silent process failure.

Alternative considered: print an immediate Windows conversion error and exit. This is more explicit but risks failing informational commands and preserving the same user-visible class of startup failure.

### Decision: Allocate using the exact validated conversion size

Only allocate after `WideCharToMultiByte()` returns a positive size. The target buffer should account for the trailing null in the API result while storing a valid C string for parser use.

Rationale:
- Prevents `len - 1` underflow.
- Avoids relying on undefined or implementation-specific behavior for empty strings.
- Keeps `argv` pointers stable by storing converted strings in a vector whose lifetime covers all CLI parsing.

Alternative considered: allocate a fixed-size buffer. This would introduce truncation risk and would not improve correctness.

### Decision: Validate Windows startup behavior through build/run tests where possible

The implementation should include focused unit coverage for the conversion helper if it is extractable without platform friction, and should document/perform a Windows or MinGW smoke check for:
- no arguments
- `-h` / `--help`
- `-v` / `--version`
- a filename containing non-ASCII characters if a Windows runtime is available

Rationale:
- The regression appears before parser logic, so normal Linux tests are not enough.
- The build system already produces Windows artifacts; adding smoke validation reduces release risk.

Alternative considered: rely on manual testing only. Manual testing is useful but too easy to miss for a startup regression that has no compiler error.

## Risks / Trade-offs

- [Risk] Falling back to original `argv` can preserve legacy Windows codepage limitations for non-ASCII filenames when Unicode conversion fails. -> Mitigation: conversion remains the preferred path; fallback is only for failure safety.
- [Risk] Testing `__wargv` behavior under Wine or CI may differ from a real Windows console. -> Mitigation: treat Wine as a useful smoke check, but prefer GitHub Actions Windows execution or real Windows validation for release confidence.
- [Risk] A helper using Windows APIs can be awkward to unit test on non-Windows platforms. -> Mitigation: split pure buffer-size/allocation decisions from direct API calls if needed, and keep platform-specific tests guarded by `_WIN32`.
- [Risk] Catching allocation failure broadly could hide a serious memory pressure problem. -> Mitigation: fallback only before CLI parsing and only for argument conversion; normal compilation errors remain reported through existing diagnostics.

## Migration Plan

1. Implement the fail-safe Windows argv conversion.
2. Build Linux normally to verify unaffected platforms.
3. Cross-compile Windows x64/x86 artifacts with the existing MinGW workflow.
4. Run Windows smoke checks for no arguments, help, and version.
5. If a regression appears, rollback is localized: disable the conversion helper and use original `argv` while investigating.

## Open Questions

- Should GitHub Actions gain an explicit Windows smoke-run step after building `msxbas2rom.exe`?
- Is preserving Unicode filenames more important than always continuing with original `argv` after partial conversion failure, or should any conversion failure be made visible in debug mode?
