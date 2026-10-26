# Proposal

## Why

The regression workflow can assert exact RAM bytes but can only compare VRAM
regions by hash. The PUT SPRITE OR-color fix needs a semantic assertion on the
sprite color table; a baseline hash generated from buggy output would freeze the
bug instead of detecting it.

## What Changes

- Add `vram.probes` to the regression manifest schema and runner.
- Each probe declares an absolute VRAM offset and expected `value` or `bytes`.
- Evaluate VRAM probes before the volatile short-circuit so volatile programs can
  still assert exact VRAM state.
- Report probe failures with the VRAM offset and expected/actual bytes.
- Add unit tests and document the manifest field.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `emulator-regression-tests`: add explicit VRAM byte probes alongside the
  existing text and region-hash checks.

## Impact

- `tests/integration/regression/runner.py`
- `tests/integration/regression/manifest.schema.json`
- `tests/integration/regression/test_runner.py`
- `tests/integration/README.md`
- `openspec/specs/emulator-regression-tests/spec.md`
- Depends on: none.
- Enables: `fix-put-sprite-or-color`.
