# Tasks

## 1. Parsing and machine classes

- [x] 1.1 Change `screen_modes` to accept `SCREEN8` and add unit tests for
  `SCREEN 4`, `SCREEN8`, `SCREEN 12`, and a non-literal screen expression;
  verify `python3 tests/integration/regression/test_runner.py` passes.
- [x] 1.2 Add `msx2plus_machine` and `turbo_r_machine` defaults and `requires`
  to `manifest.schema.json`; verify `openspec validate` and a schema check pass.
- [x] 1.3 Refactor `machine_for` to class precedence (`machine` > `requires` >
  inference) with MSX2+ for `SCREEN >= 4` / turbo commands; verify unit tests
  cover MSX1, MSX2, MSX2+, turbo R, and explicit override.

## 2. VDP mode decoding

- [x] 2.1 Implement the V9958 register table and source fallback for SCREEN
  9-12 in `screen_from_vdp`; add unit tests for all observed register pairs.
- [x] 2.2 Verify the existing `screen_from_vdp` unit cases still pass and that a
  SCREEN 5 capture reports 5 rather than 2.

## 3. Baseline and audit

- [x] 3.1 Compare `baseline["machine"]` with the selected machine and fail with
  a regeneration message; add a unit or runner test for mismatch and match.
- [x] 3.2 Include machine class/machine in `regression-list` output and in the
  run report; verify an audit shows `GRAPH/sprite3` and `GRAPH/tree` on
  `Panasonic_FS-A1WX`.
- [x] 3.3 Document the machine classes and precedence in
  `tests/integration/README.md`; verify the documented example matches the
  schema.

## 4. Integration

- [x] 4.1 Run `make -C tests/integration regression-list` and
  `make -C tests/integration regression SCENARIO=GRAPH PROGRAM=sprite3` after
  `refresh-regression-baselines`; verify the selected machine is
  `Panasonic_FS-A1WX`.
- [x] 4.2 Run `openspec validate fix-regression-machine-selection --strict`;
  verify it passes.
