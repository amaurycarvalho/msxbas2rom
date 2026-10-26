# Proposal

## Why

MSX2+ and turbo R machines with real BIOS ROMs can take longer to boot than the
current fallback window. Runs at 8 seconds sometimes captured the boot screen,
and 10 seconds is borderline; 15 seconds was reliable in local probes. A
fallback capture taken before the program starts produces meaningless RAM/VRAM
baselines.

## What Changes

- Add a per-machine-class minimum boot delay for fallback captures.
- Default MSX2+, MSX2, and turbo R to at least 15 emulated seconds.
- Keep program-specific `fallback_seconds` as the capture delay, but use
  `max(fallback_seconds, machine_boot_seconds)` for the effective fallback.
- Allow the machine boot minimum to be overridden in the manifest.
- Document the timing rule and add runner tests for the effective value.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `emulator-regression-tests`: add a machine-specific minimum boot delay for
  fallback captures.

## Impact

- `tests/integration/regression/runner.py`
- `tests/integration/regression/manifest.schema.json`
- `tests/integration/regression/manifest.json`
- `tests/integration/regression/test_runner.py`
- `tests/integration/README.md`
- `openspec/specs/emulator-regression-tests/spec.md`
- Depends on: none.
- Affects: `refresh-regression-baselines` (capture timing).
