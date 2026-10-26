# Design

## Context

See `proposal.md` - Why. Baseline generation runs the selected programs in
openMSX and stores RAM/VRAM references. The dependent changes alter the selected
machine, the decoded screen mode, the terminal capture point, and the effective
fallback time. A baseline generated before those changes can encode the wrong
machine, screen layout, or capture state.

## Goals / Non-Goals

**Goals:**

- Regenerate every baseline affected by the dependent changes.
- Review machine, screen, and VRAM-region differences before committing.
- Leave the full regression suite passing with machine-consistent baselines.

**Non-Goals:**

- Changing runner or compiler behavior.
- Regenerating unrelated baselines without a reason.
- Hiding failures by marking programs volatile without a review.

## Decisions

### D1: Regenerate after all behavior changes land

Run the baseline refresh only after the dependent changes are implemented and
their unit/integration checks pass. Generate in dependency order and review each
program's `machine`, `screen`, `screen_stable`, `regions`, and `volatile` fields.

### D2: Affected programs

Machine changes:

- `IO/test5`, `MTF/mtf2`, `GRAPH/test96`, `GRAPH/base2`, `GRAPH/tree`:
  MSX1 -> MSX2+ (`Panasonic_FS-A1WX`).
- `GRAPH/sprite3`: new MSX2+ baseline.
- `MISC/test1`, `MISC/test64`, `MISC/test65`: turbo R (`Panasonic_FS-A1GT`).

Screen/region or capture changes:

- `GRAPH/cadbit`, `GRAPH/sprite1`, `GRAPH/sprite2`, `GRAPH/test38`,
  `GRAPH/test80`, `TS/ts7`.
- `GRAPH/line1` (verify the final capture point and screen mode).
- Volatile metadata: `BLOAD/test60`, `GRAPH/cadbit2`, `GRAPH/cadbit3`.
- Long-running implicit-end program: `MEGAROM/test51`. Its 500-line print loop
  finishes around 33.6 emulated seconds, so the default 10-second fallback
  captures mid-run. Set `fallback_seconds: 45` and regenerate the baseline at
  `END_STMT+2`.

Any additional program that reports a baseline machine mismatch after the
machine-selection change must be regenerated.

### D3: Review criteria

For each regenerated baseline, confirm:

- `machine` matches the intended class/machine.
- `screen` is the actual mode at the capture point (or the documented SCREEN
  9-12 fallback).
- Non-volatile `regions` use the layout implied by the screen mode; no text
  layout for a graphics program.
- Volatile programs rely on explicit probes or stable text, not a full snapshot.
- The capture trigger and effective fallback time are expected.

### D4: Full-suite verification

Run `make -C tests/integration regression` and the machine audit. Failures caused
by unexplained baseline diffs must be investigated, not auto-accepted.

## Risks / Trade-offs

- **[Risk] Reviewing many baselines is slow** -> Use the machine/audit output and
  review only the affected list first, then run the full suite.
- **[Risk] A baseline captures a flaky boot screen** -> Use the boot-timing and
  END-capture fixes; re-run a failing program before regenerating.
- **[Risk] An unrelated program changes** -> Investigate the behavior change
  instead of refreshing it silently.

## Migration Plan

- Regenerate the affected baselines, review, and commit them with the dependent
  changes.
- Roll back by restoring the previous baseline files; the runner's machine check
  will show which ones are inconsistent.

## Validation note

- MSX2+ machine changes: `IO/test5`, `MTF/mtf2`, `GRAPH/test96`,
  `GRAPH/base2`, `GRAPH/tree`, `GRAPH/mexhat`, and `MATH/cos` now record
  `Panasonic_FS-A1WX`. Turbo R feature baselines record `Panasonic_FS-A1GT`.
- Screen-mode changes from the V9958 decoder: `MTF/mtf2` moved to SCREEN 4;
  `GRAPH/cadbit`, `sprite1`, `sprite2`, `test38`, `line1`, and `TS/ts7` moved to
  SCREEN 5; `GRAPH/tree`, `test80`, `BLOAD/test60`, `cadbit2`, and `cadbit3`
  moved to SCREEN 8. `GRAPH/line1` now captures at the fallback point and uses a
  bitmap layout.
- `GRAPH/mexhat` and `MATH/cos` needed bitmap refreshes after the boot-timing and
  capture-point changes; three-run generation reported stable regions.
- `MEGAROM/test51` had a stale early `END_PGM` baseline captured at 9.15 s. Its
  500-line program actually ends at about 33.6 emulated seconds, so the manifest
  now sets `fallback_seconds: 45`; the regenerated baseline captures at
  `END_STMT+2` after the full print loop. The full suite passes 164/164.

