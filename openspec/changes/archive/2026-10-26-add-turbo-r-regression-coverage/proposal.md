# Proposal

## Why

Turbo R feature tests currently run on MSX1 or MSX2+, where they do not exercise
the R800 path or the turbo R return values. At the same time, tests that only use
`CMD TURBO` to speed up drawing or math should keep running on the MSX2+ machine
so MSX2+ compatibility coverage is not replaced by turbo R coverage.

## What Changes

- Mark the turbo R feature programs (`MISC/test1`, `MISC/test64`, `MISC/test65`)
  with `requires: turboR` so they run on `Panasonic_FS-A1GT`.
- Keep performance-only users of `CMD TURBO`/`CALL TURBO` (`GRAPH/tree`,
  `GRAPH/mexhat`, `GRAPH/test78`, `IO/test48`, `MATH/cos`) on the MSX2+ machine
  unless they declare a turbo R requirement.
- Add stable text expectations for `MSX()` and turbo output labels, and mark
  timing-dependent values volatile.
- Add a turbo R gate that verifies `MSX() = 3` and `CMD TURBO` continues, using
  `TURBO()` only after `fix-turbo-function-runtime` lands.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `emulator-regression-tests`: turbo R feature programs are explicitly assigned
  to the turbo R machine, while performance-only turbo usage stays on MSX2+.

## Impact

- `tests/integration/regression/manifest.json`
- `tests/integration/regression/baselines/MISC/test1.json`
- `tests/integration/regression/baselines/MISC/test64.json`
- `tests/integration/regression/baselines/MISC/test65.json`
- `tests/integration/README.md`
- `openspec/specs/emulator-regression-tests/spec.md`
- Depends on: `fix-regression-machine-selection`,
  `fix-turbo-function-runtime`, `fix-regression-boot-timing`.
- Affects: `refresh-regression-baselines`.
