## 1. Startup Conversion Safety

- [ ] 1.1 Inspect the current `_WIN32` argv conversion in `src/cli/main.cpp` and identify all paths that can read null pointers or allocate from invalid conversion lengths.
- [ ] 1.2 Extract or isolate Windows Unicode argv conversion into a small fail-safe helper or scoped routine whose lifetime keeps converted strings valid through CLI parsing.
- [ ] 1.3 Guard against unavailable `__wargv`, null wide arguments, `WideCharToMultiByte()` returning 0, and allocation failures.
- [ ] 1.4 Fall back to original `argv` when conversion fails, preserving normal CLI parsing for informational commands.
- [ ] 1.5 Ensure converted UTF-8 arguments remain null-terminated and pointer-stable for the parser when conversion succeeds.

## 2. CLI Behavior Validation

- [ ] 2.1 Verify Linux no-argument, `-h`, `--help`, `-v`, and `--version` behavior remains unchanged.
- [ ] 2.2 Add focused unit coverage for conversion sizing/fallback logic if the helper can be tested without direct Windows runtime dependency.
- [ ] 2.3 Add guarded Windows-specific coverage or smoke instrumentation for conversion failure safety where the test framework/toolchain allows it.
- [ ] 2.4 Confirm quiet mode and invalid-parameter behavior are not changed by startup conversion fallback.

## 3. Windows Build and Smoke Checks

- [ ] 3.1 Cross-compile Windows x64 and x86 release artifacts using the existing MinGW build path.
- [ ] 3.2 Run or document Windows smoke checks for no arguments, `-h`, `--help`, `-v`, and `--version`.
- [ ] 3.3 When a Windows runtime is available, smoke-check a BASIC filename containing non-ASCII characters to confirm Unicode conversion still works.
- [ ] 3.4 Record any Wine limitations separately from real Windows validation so release confidence is based on the correct runtime.

## 4. Regression Review

- [ ] 4.1 Compare startup behavior against v0.3.4.2 expectations: no arguments prints help and exits successfully.
- [ ] 4.2 Confirm no new external dependencies or non-C++11 language features were introduced.
- [ ] 4.3 Run `openspec status --change "fix-windows-startup-unicode-argv"` and verify artifacts remain complete before implementation review.
