# Proposal

## Why

The machine-selection, screen-decoding, END-capture, boot-timing, OR-color, and
turbo R coverage changes invalidate existing regression baselines. Several
baselines were generated on the wrong machine or with a stale screen mode, and
the runner will reject machine mismatches after the machine-selection change.
The baselines must be regenerated from known-good, correctly classified runs and
reviewed before the suite can be trusted.

## What Changes

- Regenerate baselines for programs whose machine class changed to MSX2+ or
  turbo R.
- Regenerate baselines whose captured screen mode or VRAM regions changed
  because of the V9958 decoder and END-capture fixes.
- Add the new `GRAPH/sprite3` baseline after the OR-color fix.
- Raise `MEGAROM/test51` fallback time so its 500-line program reaches the
  pre-cleanup END entry before capture, then regenerate its baseline.
- Review every regenerated baseline for selected machine, screen mode, and
  region/layout changes; reject unexplained diffs.
- Run the full local regression and the machine audit.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- _(none; this is a baseline-maintenance change with no behavior delta)_

## Impact

- `tests/integration/regression/baselines/`
- `tests/integration/regression/manifest.json`
- `tests/integration/regression/artifacts/` (review only; gitignored)
- `tests/integration/README.md` (refresh procedure if changed)
- Depends on: `fix-regression-machine-selection`,
  `fix-regression-end-pgm-capture`, `fix-regression-boot-timing`,
  `fix-turbo-function-runtime`, `fix-put-sprite-or-color`,
  `add-turbo-r-regression-coverage`.
- No compiler/kernel behavior change.
