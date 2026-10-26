# Proposal

## Why

The regression runner selects only MSX1 or MSX2 and treats `SCREEN 4` as MSX1,
so MSX2-only programs run on the wrong machine. It also misses `SCREEN8` without
a space and decodes V9938/V9958 modes as if they were MSX1 modes. Baselines do
not record whether the selected machine actually matches the baseline.

## What Changes

- Parse `SCREEN` literals with or without whitespace.
- Introduce machine classes `msx1`, `msx2`, `msx2plus`, and `turboR` with
  configurable defaults; MSX2/MSX2+ use `Panasonic_FS-A1WX`.
- Add an explicit `requires` manifest field and keep `machine` as the highest
  precedence override.
- Infer MSX2+ for `SCREEN >= 4` or turbo-capable commands; require an explicit
  `requires: turboR` for turbo R programs.
- Decode V9938/V9958 screen modes from VDP registers, using the source's last
  SCREEN for SCREEN 9-12 when the registers cannot distinguish them.
- Fail a regression run when the baseline machine differs from the selected
  machine.
- Expose each program's selected machine and class in the audit/list output and
  run report.
- Add unit tests for parsing, machine selection, and VDP mode decoding.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `emulator-regression-tests`: machine selection now supports MSX2+, turbo R,
  explicit requirements, accurate V9958 mode decoding, and machine-consistency
  checks.

## Impact

- `tests/integration/regression/runner.py`
- `tests/integration/regression/manifest.schema.json`
- `tests/integration/regression/manifest.json`
- `tests/integration/regression/test_runner.py`
- `tests/integration/README.md`
- `openspec/specs/emulator-regression-tests/spec.md`
- Depends on: none.
- Affects: `fix-regression-end-pgm-capture`, `fix-regression-boot-timing`,
  `add-turbo-r-regression-coverage`, `refresh-regression-baselines`.
