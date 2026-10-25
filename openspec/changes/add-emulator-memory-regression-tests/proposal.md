# Proposal

## Why

The current `make test-integration` flow only compiles the 161 BASIC programs in
`tests/integration/`; it never executes them, so runtime regressions in generated
ROMs are invisible until a developer manually opens an emulator. Existing manual
debugging is screenshot-oriented and tied to a fixed delay, which misses programs
that finish before the capture point and does not expose VRAM contents.

This change adds a local, memory-centric regression workflow that compiles and runs
selected programs in openMSX, validates RAM and VRAM deterministically, and can be
driven by either a developer or an agent outside CI/CD. It also extends the
`msx-memory-debug` skill so the regression workflow can reuse its machine isolation
and capture conventions instead of duplicating them.

## What Changes

- Add a `regression` Make target in `tests/integration/Makefile`, independent from
  the existing `run`/`test-integration` flow.
- Add a regression runner and manifest under `tests/integration/regression/`.
- Group programs into scenarios by their existing feature folders and allow running
  a whole scenario or a single program.
- Compile selected programs in ASCII8 mode by default, with `--noi` symbols and a
  dedicated output directory, without touching the existing compile matrix.
- Execute ROMs in openMSX with deterministic capture:
  - breakpoints at `END_PGM` and `END_STMT + 2` for programs that terminate;
  - configurable emulated-time fallback for programs that loop or wait for input;
  - automated keyboard and joystick input scripts for interactive programs.
- Validate memory before visuals:
  - RAM probes and stable RAM snapshots;
  - VRAM text/name-table checks for text modes and VRAM region hashes for
    graphical modes;
  - screenshots only as diagnostic artifacts when a memory check fails.
- Add baseline generation/refresh with stable-byte masking and volatile-program
  handling.
- Extend the `msx-memory-debug` skill with VRAM capture, pre-boot breakpoints,
  input injection, and fallback capture while preserving its current behavior.
- Do **not** add the regression workflow to CI/CD.

## Capabilities

### New Capabilities

- `emulator-regression-tests`: scenario-based runtime regression of compiled BASIC
  programs with memory-centric validation, program/scenario selection, baselines,
  and local-only execution.

### Modified Capabilities

- `msx-memory-debug`: the skill gains VRAM capture, configurable breakpoints set
  before boot, input injection, and fallback capture semantics while retaining its
  existing RAM and screenshot behavior.

## Impact

- Target release: `1.4.0.0`.
- Affected files:
  - `tests/integration/Makefile`
  - `tests/integration/regression/` (runner, manifest, capture script, baselines)
  - `.opencode/skills/msx-memory-debug-skill/` scripts and documentation
  - `openspec/specs/msx-memory-debug/spec.md` via the change delta
- New runtime dependency: openMSX must be installed locally (already required by
  the skill).
- No compiler or kernel behavior changes.
- No CI/CD changes.
