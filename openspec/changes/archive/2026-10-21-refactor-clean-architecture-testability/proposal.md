## Why

The current compilation pipeline couples application logic to filesystem I/O,
the embedded kernel global, concrete diagnostics, and large mutable contexts.
This makes unit tests slow and environment-dependent: compiling a BASIC
program commonly requires creating a temporary `.bas` file and can pull in the
real kernel or output filesystem even when the behavior under test is local.

This change makes dependency direction conform to the accepted Clean
Architecture rule, so application and domain behavior can be exercised with
in-memory adapters while CLI and infrastructure remain the only owners of OS
and generated-kernel details.

The change is sequenced before the survivor-driven Round 6 of
`improve-mutation-score`. Its testability-first Phase A (architecture
boundary check, in-memory source, kernel image, and resource/symbol
boundaries) runs first; `improve-mutation-score` then consumes a fresh Mull
baseline and adds the mutation-killing tests; finally this change completes
Phase B (diagnostics, ROM output, CLI workflow, and context decomposition).
The boundary check and its approved exception list are established before
either phase so the refactor cannot silently introduce new layer violations.

## What Changes

- Establish an automated Clean Architecture boundary check with a
  version-controlled, pre-approved exception list; the check fails on new
  violations and on stale exceptions.
- Introduce application ports and infrastructure adapters for BASIC source
  input, `INCLUDE` resolution, ROM/resource/symbol output, and kernel-image
  access.
- Make lexer/parser/compiler workflows accept in-memory source content with a
  logical source name; file-backed behavior becomes an adapter concern.
- Keep `BuildOptions` as a domain value object by removing its dependency on
  filesystem utilities.
- Separate pure ROM image assembly from ROM-file persistence.
- Extract the CLI's compile/retry orchestration into an application use case
  with structured results and diagnostics, leaving the CLI as the composition
  and presentation boundary.
- Incrementally replace public, all-purpose parser/compiler contexts with
  narrower collaborators and encapsulated state.
- Preserve CLI behavior, ROM bytes, diagnostics, resource formats, and
  cross-platform C++11 support; no kernel-assembly behavior changes.

## Capabilities

### New Capabilities

- `testable-compilation-boundaries`: Compilation and ROM-building behavior can
  be invoked through application ports using in-memory source, output, and
  kernel adapters.

### Modified Capabilities

- `architecture`: Strengthen the Clean Architecture dependency rule with
  explicit ownership of I/O and generated-kernel dependencies, and require
  application-level compilation orchestration.

## Impact

- `src/domain/options`, `src/application/{lexer,parser,compiler,builder,symbols}`
  and `src/cli/main.cpp` will be refactored in behavior-preserving increments.
- New interfaces/adapters will be added under application and infrastructure;
  tests will use in-memory adapters rather than temporary BASIC files.
- Existing unit, integration, ROM, kernel and mutation-oriented tests gain
  characterization coverage before each migration. No external CLI/API
  contract is intentionally changed.
- New boundary-check tooling and its approved exception list are added under
  `scripts/`, wired into `make test`/`quality-gate` and CI.
- Target release: **1.3.1.0**.
