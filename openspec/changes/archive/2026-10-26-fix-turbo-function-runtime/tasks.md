# Tasks

## 1. Compiler fix

- [x] 1.1 Change the TURBO strategy to load `HL=5` and use
  `addKernelCall(DISP_usr2)`; verify the emitted code contains the dispatch call
  and not a direct absolute call.
- [x] 1.2 Apply the same change to VDP (`HL=4`) and MAKER (`HL=6`); verify their
  existing compiler tests still pass.
- [x] 1.3 Add/adjust unit tests that assert the dispatch index and selector;
  verify `make test-unit` or the project's unit target passes.

## 2. Runtime regression

- [x] 2.1 Add a small BASIC probe that prints `TURBO()`, `VDP()`, and `MAKER()`
  values and continues to an input wait; verify it compiles.
- [x] 2.2 Run the probe on MSX1 and verify `TURBO()` returns 0 without hanging.
- [x] 2.3 Run the probe on `Panasonic_FS-A1WX` and `Panasonic_FS-A1GT` and
  verify the values are printed and execution continues.

## 3. Documentation and verification

- [x] 3.1 Document the USR2 dispatch rule in the compiler/unit test notes;
  verify the note matches the emitted code.
- [x] 3.2 Run `openspec validate fix-turbo-function-runtime --strict`; verify it
  passes.
