# Tasks

## 1. Manifest and validation

- [x] 1.1 Add `vram.probes` to `manifest.schema.json` with `address` and
  `value`/`bytes` items; verify `openspec validate add-regression-vram-probes`
  and a JSON-schema check pass.
- [x] 1.2 Add `compare_vram_probes` (or equivalent) in `runner.py`, validating
  bounds and producing `vram_probe_0x...` checks; verify with a matching and a
  mismatching synthetic probe.

## 2. Runner integration

- [x] 2.1 Call the VRAM probe comparator from `compare_vram` before the volatile
  early return; verify a volatile program with a matching probe passes and one
  with a mismatching probe fails.
- [x] 2.2 Keep text and region checks unchanged; verify the existing
  `test_runner.py` VRAM tests still pass.

## 3. Unit tests and documentation

- [x] 3.1 Add unit tests for matching, mismatching, multi-byte, and
  out-of-range VRAM probes; verify `python3 tests/integration/regression/test_runner.py`
  passes.
- [x] 3.2 Document `vram.probes` in `tests/integration/README.md`; verify the
  example manifest snippet is valid against the schema.
- [x] 3.3 Run `openspec validate add-regression-vram-probes --strict`; verify it
  passes.
