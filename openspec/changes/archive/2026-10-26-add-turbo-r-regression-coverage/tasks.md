# Tasks

## 1. Classification

- [x] 1.1 Add `requires: "turboR"` to `MISC/test1`, `MISC/test64`, and
  `MISC/test65`; verify `regression-list` selects `Panasonic_FS-A1GT`.
- [x] 1.2 Verify `GRAPH/tree`, `GRAPH/mexhat`, `GRAPH/test78`, `IO/test48`, and
  `MATH/cos` still select `Panasonic_FS-A1WX`; add a runner/audit test for those
  assignments.

## 2. Expectations and gate

- [x] 2.1 Add stable text expectations for `MSX = 3` and the turbo timing labels;
  verify a turbo R run on `Panasonic_FS-A1GT` passes with volatile time values.
- [x] 2.2 Add a turbo R gate that checks `MSX() = 3` and `CMD TURBO` on/off, then
  prints a stable marker; verify it passes on FS-A1GT.
- [x] 2.3 After `fix-turbo-function-runtime` lands, enable `MISC/test1` and
  `MISC/test64` expectations for `TURBO()`/`VDP()` and verify they complete on
  FS-A1GT.

## 3. Documentation and verification

- [x] 3.1 Document the feature-vs-performance turbo classification in
  `tests/integration/README.md`; verify the documented examples match the
  manifest.
- [x] 3.2 Run the turbo R selection and
  `openspec validate add-turbo-r-regression-coverage --strict`; verify both pass.
