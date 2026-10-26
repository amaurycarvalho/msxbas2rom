# Tasks

## 1. Capture point

- [x] 1.1 Inspect the generated END stub and confirm `END_STMT + 2` is the
  pre-`XBASIC_END` instruction for both explicit and implicit ends; document the
  addresses in `design.md` or the code comment.
- [x] 1.2 Change `breakpoints_for` to prefer `END_STMT + 2` and use `END_PGM`
  only as a fallback; add unit tests for symbol combinations (`END_STMT` only,
  `END_PGM` only, both).

## 2. Screen-mode consistency

- [x] 2.1 Add source/VDP mode conflict reporting to the run result and report;
  verify a synthetic conflict is visible without changing pass/fail semantics.
- [x] 2.2 Verify `GRAPH/sprite1` (non-terminating fallback) still reports
  SCREEN 5 and uses bitmap regions after `fix-regression-machine-selection`.
- [x] 2.3 Re-run `GRAPH/line1` with the new capture point, inspect the
  screenshot/VDP/VRAM artifacts, and either confirm SCREEN 5 with bitmap regions
  or record the remaining source-level issue.

## 3. Documentation and verification

- [x] 3.1 Document the pre-cleanup capture rule in
  `tests/integration/README.md`; verify the described breakpoint matches the
  runner behavior.
- [x] 3.2 Run the affected GRAPH regression selection and
  `openspec validate fix-regression-end-pgm-capture --strict`; verify both pass.
