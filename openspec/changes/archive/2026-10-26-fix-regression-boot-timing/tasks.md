# Tasks

## 1. Configuration

- [x] 1.1 Add `machine_boot_seconds` defaults and schema support; verify the
  manifest loads and the schema accepts MSX1, MSX2, MSX2+, and turbo R values.
- [x] 1.2 Implement `effective_fallback_seconds` using the maximum of the
  configured fallback and the selected machine's boot delay; add unit tests for
  shorter and longer program fallbacks.

## 2. Runtime and diagnostics

- [x] 2.1 Pass the effective fallback to the skill and record the effective
  value and machine class in capture metadata/run report; verify a sample
  capture shows the expected value.
- [x] 2.2 Run `GRAPH/sprite3` on `Panasonic_FS-A1WX` with an 8-second configured
  fallback and verify the effective capture occurs at 15 seconds and the program
  has executed.

## 3. Documentation and verification

- [x] 3.1 Document the boot-delay rule and override path in
  `tests/integration/README.md`; verify the documented example is valid.
- [x] 3.2 Run `openspec validate fix-regression-boot-timing --strict`; verify it
  passes.
