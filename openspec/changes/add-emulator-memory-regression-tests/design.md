# Design

## Context

See `proposal.md` - Why.

Current constraints that shape this design:

- `tests/integration/Makefile` builds every `.bas` four times and deletes the ROMs;
  there is no runtime execution.
- The existing `msx-memory-debug` skill captures RAM `0xC000-0xFFFF` plus one
  screenshot after a fixed emulated-time delay.
- openMSX can read `Main RAM`, `VRAM`, `physical VRAM`, and `VDP regs` through the
  debugger. RAM is deterministic between runs; screenshots are not a reliable
  pass/fail signal.
- A fixed delay is insufficient for programs that reach `END` early: after `END`,
  control returns to BASIC and the captured state no longer represents the program.
- In openMSX, `-script` runs too late to install breakpoints for fast programs;
  `-command` runs early enough.
- `SCREEN 0/1` text is written to the name table as character codes. `SCREEN 2`
  printing uses `GRPPRT` and draws characters as graphics, so its VRAM cannot be
  validated as ASCII name-table text. `SCREEN 5+` modes are bitmap-based.

## Goals / Non-Goals

**Goals:**

- A local regression workflow that compiles and runs selected programs in openMSX.
- Deterministic capture at program end, with a fallback for non-terminating programs.
- Memory-first validation: RAM probes/snapshots and VRAM text/region checks.
- Scenario-level and program-level selection using the existing folders.
- A reusable skill extension for VRAM capture, pre-boot breakpoints, and input.
- Baselines that can be regenerated with stable-byte masking.

**Non-Goals:**

- Replacing the existing `make test-integration` compile matrix.
- Running the regression workflow in CI/CD.
- Guaranteeing pixel-identical screenshots across environments.
- Reimplementing the Z80 kernel or compiler to add test hooks.
- Migrating existing `.bas` programs to a new format.

## Decisions

### D1: Extend the existing `msx-memory-debug` skill instead of duplicating capture logic

**Choice:** Add VRAM capture, pre-boot breakpoint setup, input injection, and
fallback capture to the skill, and have the regression runner invoke the skill.

**Why:** The skill already owns machine selection, isolated settings, artifact naming,
and capture semantics. Duplicating that logic in `tests/integration/regression/`
would create two divergent emulator launchers.

**Alternative considered:** A dedicated regression Tcl script that ignores the skill.
Rejected because it duplicates settings isolation and makes the skill expansion
requested by the user impossible to reuse.

### D2: Use a JSON manifest for scenario and per-program configuration

**Choice:** Store scenario membership and per-program overrides in
`tests/integration/regression/manifest.json`.

**Why:** The project's test scripts are Python 3 with standard-library dependencies
only. JSON avoids introducing PyYAML or another parser. Scenario membership can be
derived from the folder layout, while the manifest carries only overrides.

**Alternative considered:** YAML for readability. Rejected to avoid a new runtime
dependency and to match the existing stdlib-only Python tooling.

### D3: Capture at the program's terminal state using `.noi` symbols

**Choice:** Compile with `--noi` and install breakpoints before boot at:

- `END_PGM` for programs that fall off the end;
- `END_STMT + 2` for the explicit END routine (the `END_STMT` symbol points at the
  `JR` skip instruction, not the routine entry).

Breakpoints are installed through openMSX `-command` arguments, because `-script`
runs after the ROM may already have started.

**Why:** It produces the actual final state for short-lived programs and avoids
guessing a delay. Programs that never terminate fall back to a configured
emulated-time capture.

**Alternative considered:** Large fixed delays. Rejected because they either miss
fast programs or waste runtime, and still race programs that finish early.

### D4: Use RAM probes first, stable snapshots second, VRAM next, screenshots last

**Choice:** Each program's checks are evaluated in this order:

1. explicit RAM probes from the manifest;
2. stable RAM snapshot comparison with masked volatile ranges;
3. VRAM text or region hashes;
4. screenshot comparison only when no memory check exists or as diagnostics.

**Why:** It matches the user's priority on memory, produces actionable diffs, and
avoids false failures from cursor blink, timing, or rendering differences.

**Alternative considered:** Screenshot baselines as the primary check. Rejected as
fragile and non-diagnostic.

### D5: Derive VRAM regions from the active screen mode

**Choice:** The validator selects the VRAM strategy from the captured mode:

- `SCREEN 0/1`: extract text from the name table and compare strings;
- `SCREEN 2`: compare configured pattern/name/color regions as hashes, since text
  is drawn through `GRPPRT`;
- `SCREEN 5+`: compare bitmap and palette regions as hashes.

Region addresses come from VDP registers or BIOS work-area variables rather than
hardcoded constants.

**Why:** A single "dump VRAM and compare everything" check is noisy and fails on
unused or timing-dependent regions. Mode-aware regions keep checks meaningful.

**Alternative considered:** Compare the full 64 KiB VRAM blob. Rejected because it
amplifies unrelated differences and makes failures hard to interpret.

### D6: Generate baselines with stable-byte masking

**Choice:** `regression-baseline` runs each selected program multiple times, keeps
only bytes that are identical across runs, and stores per-region hashes plus the
stable mask. Programs dominated by `RND`, `TIME`, or clock values are explicitly
marked volatile and are checked only against their configured stable probes.

**Why:** It makes baseline comparison robust without requiring per-byte expected
values for all 161 programs, while still detecting broad runtime regressions.

**Alternative considered:** Manually authored expectations for every program.
Rejected as too costly for the first iteration; explicit probes can be added
incrementally where baselines are insufficient.

### D7: Input scripts are manifest data

**Choice:** Interactive programs receive a timed event list in the manifest, for
example keyboard keys and joystick direction/button states.

**Why:** It keeps program-specific behavior declarative and lets the runner stay
generic. Programs without a script fall back to capturing the waiting state.

**Alternative considered:** Hardcoded input sequences in the runner. Rejected
because it couples unrelated programs and complicates scenario selection.

### D8: Separate output directory and no CI wiring

**Choice:** Compile and capture into a dedicated regression output directory
(gitignored), keep baselines under `tests/integration/regression/baselines/`, and
add no CI workflow changes.

**Why:** It avoids interfering with existing integration artifacts and keeps the
workflow local-only as requested.

## Risks / Trade-offs

- **[Risk] openMSX VRAM debuggable semantics differ by VDP/machine** → Verify
  `physical VRAM`/`VRAM` sizes and banking on both test machines; derive addresses
  from VDP registers and BIOS variables; add a VRAM probe test before relying on it.
- **[Risk] A breakpoint may fire before the capture callback is ready** → Install
  breakpoints through `-command` and keep the breakpoint command self-contained;
  fall back to time capture if no breakpoint fires.
- **[Risk] Interactive input coverage grows without bound** → Start with the input
  scripts needed by the selected scenarios; programs without scripts still validate
  their waiting state.
- **[Risk] Baselines are sensitive to openMSX or machine changes** → Pin machine
  names and isolated settings; provide `regression-baseline` to refresh selected
  programs; document that baseline refresh requires review.
- **[Risk] Volatile programs mask too much and hide regressions** → Require explicit
  stable probes for volatile programs where functionality matters.
- **[Risk] Serial emulator runs are slow** → Default to ASCII8 only and support
  scenario/program selection; parallelism is left as an optional optimization.
- **[Risk] Skill expansion breaks existing callers** → Keep all new options opt-in
  and add a legacy-invocation scenario to the skill spec.

## Migration Plan

- Additive only: new target, new regression directory, opt-in skill options.
- Generate and review baselines before using them as a gate.
- Roll back by removing the new target and regression directory; existing
  `make test-integration` behavior is untouched.

## Open Questions

- Which exact openMSX Tcl commands to use for joystick injection on the selected
  machines? This is an implementation-time verification and does not change the
  spec or task breakdown.
- Whether to store compact baseline hashes only or also raw baseline captures for
  byte-level diffs. The first iteration can store hashes/masks and keep raw run
  artifacts, adding raw baselines only if diff quality requires it.
