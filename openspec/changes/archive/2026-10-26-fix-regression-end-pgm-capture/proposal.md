# Proposal

## Why

Terminal captures can be taken after the compiled program's end routine has
already restored text mode. The runner then reports a text screen for a program
that ended in a graphical mode, and VRAM region selection can use the wrong
layout. `GRAPH/line1` exposed a text/text-like VDP state around `END_PGM` while
the program's screen was graphical, so the capture point and screen-mode policy
need to be deterministic.

## What Changes

- Capture terminal state at the pre-cleanup END entry (`END_STMT + 2`) for both
  explicit and implicit program termination.
- Use `END_PGM` only as a fallback when `END_STMT` is unavailable.
- Report a clear diagnostic when the captured VDP mode conflicts with the
  program's source-declared screen mode.
- Keep the source fallback only for SCREEN 9-12, where VDP registers do not
  distinguish the modes.
- Add regression checks that a graphics-mode program captured at END reports
  the graphics mode and uses the correct VRAM region layout.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `emulator-regression-tests`: terminal capture now happens before the end
  routine restores text mode, with a deterministic screen-mode policy.

## Impact

- `tests/integration/regression/runner.py`
- `tests/integration/regression/test_runner.py`
- `tests/integration/README.md`
- `openspec/specs/emulator-regression-tests/spec.md`
- `tests/integration/regression/baselines/GRAPH/line1.json` (refresh only)
- Depends on: `fix-regression-machine-selection` for the V9958 decoder.
- Affects: `refresh-regression-baselines`.
