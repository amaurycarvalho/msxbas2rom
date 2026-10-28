# Tasks

## 1. Temporary-string allocator

- [x] 1.1 Change `GET_NEXT_TEMP_STRING_ADDRESS` to return `TMPSTRADDR + index * 256` while preserving the 0..4 rotation and AF/BC; rebuild the kernel and verify a two-`CHR$` program places each result in a distinct slot.
- [x] 1.2 Audit every `GET_NEXT_TEMP_STRING_ADDRESS` caller (`MR_GET_DATA`, helper buffers, and string strategies) and verify each treats the returned address as a 256-byte string destination; record the audit result with the kernel build.

## 2. Compiler string strategies

- [x] 2.1 Change the `STRING$` strategy to copy the kernel `BUF` result into a temporary slot with `DISP_XBASIC_COPY_STRING`, following the `LEFT$` pattern; verify a unit test compiles the two-call concatenation and the STRING regression probe produces `AAAAABBBBBB`.
- [x] 2.2 Apply the same copy pattern to `SPACE$`; verify a unit or integration case concatenates `SPACE$` with another string function and preserves both operands.
- [x] 2.3 Apply the same copy pattern to `INKEY$`; verify a compile test emits the function call followed by the temporary copy and retains the string subtype.

## 3. Startup screen mode

- [x] 3.1 Replace the manual `LINLEN` setup plus mode-dependent `XBASIC_SCREEN` call with an explicit `SCREEN 0` + `WIDTH 40` startup sequence; verify the emitted start code selects mode 0 and applies width 40.
- [x] 3.2 Capture a simple `PRINT "HELLO"` / `PRINT "WORLD"` program on `Sharp_HB-8000_1.2`, `Panasonic_FS-A1WX`, and `Panasonic_FS-A1GT`; verify identical 40-column rows, name table `0x0000`, and no rightward drift.
- [x] 3.3 Verify the CR+LF routine is unchanged and that consecutive PRINTs start at column 0 of consecutive rows on each machine class.

## 4. STRING$ arity hardening

- [x] 4.1 Apply the shared function-arity bound from `harden-cpp-parameter-validation-tier1` task 4.4 to `STRING$`; add a unit test that `STRING$(3,65)` compiles and `STRING$(3,65,1,2,3)` fails with a syntax error, and verify no argument subtypes are evaluated for the failing call.

## 5. PRINT and STRING regression probes

- [x] 5.1 Add a `PRINT/test5` entry to `tests/integration/regression/manifest.json` with `width: 40`, `name_table: "0x0000"`, expected rows for trailing `;`/`,` and repeated separators, and VRAM byte probes for the exact tab/space layout; verify `regression-list` selects the MSX1 machine.
- [x] 5.2 Add a `STRING/test1` entry with `width: 40`, `name_table: "0x0000"`, expected `AAAAABBBBBB!!!`, `AAAAABBBBBB`, and `AAAAABBBBB` rows, and VRAM byte probes at rows 3, 4, 5, 6, 7, and 10; verify `regression-list` selects the MSX1 machine.
- [x] 5.3 Generate `PRINT/test5` and `STRING/test1` baselines from the fixed ROM with `make -C tests/integration regression-baseline`; verify each baseline records the 40-column layout and machine.
- [x] 5.4 Run `make -C tests/integration regression SCENARIO=PRINT PROGRAM=test5` and `... SCENARIO=STRING PROGRAM=test1`; verify every text and probe check passes, and confirm the pre-fix ROM fails the same checks.
- [x] 5.5 Document the explicit 40-column `name_table`/`width` requirement for text regression programs in `tests/integration/README.md` and verify the documented commands reproduce both runs.

## 6. PRINT and STRING$ baseline audit

- [x] 6.1 Enumerate all integration programs using `PRINT`, `?`, or `STRING$` and classify each as explicit-`SCREEN` or default-layout; record the inventory and the 94 existing `screen: 1` baselines in the change notes.
- [x] 6.2 Regenerate and review every `PRINT/*` and `STRING/*` baseline, including the new `PRINT/test5` and `STRING/test1`; verify no baseline still encodes the shifted 32-column rows.
- [x] 6.3 Regenerate and review the text-bearing baselines and manifest checks for `INPUT/test8`, `ON/test2`, `ON/test4`, `MISC/test1`, `MISC/test64`, `MISC/test65`, `MISC/rsc_offset`, `MISC/rscfunc`, `MISC/turbo_gate`, and `GRAPH/mouse2`; verify their `name_table`/`width` match the fixed layout.
- [x] 6.4 Re-run the `STRING$` programs that set their own screen (`GRAPH/mouse2`, `GRAPH/sprite3`, `GRAPH/test28`, `ON/test8`) and verify their VRAM probes or baselines still match after the string-result fix.

## 7. Integration checks

- [x] 7.1 Run the full local regression suite after all baseline updates and verify every program passes.
- [x] 7.2 Run `make test-unit`, `make test-integration`, and `make test-architecture`, and record the results with the change.

## 8. Documentation

- [x] 8.1 Create `doc/adr/ADR-002 Initial text screen normalization.md` with the per-machine boot-mode research, the `SCREEN 0` + `WIDTH 40` decision, alternatives, and baseline consequences; verify it follows the ADR-001 structure.
- [x] 8.2 Update `doc/misc/architecture.md` section 5.3 (`Start code`) to describe the explicit `SCREEN 0`/`WIDTH 40` startup and link to ADR-002; verify the architecture text matches the compiler spec delta.

## 9. Additional PRINT probes

- [x] 9.1 Add `PRINT/test6` (explicit `SCREEN 0`) and `PRINT/test8` (explicit `SCREEN 1`) manifest entries with 40-column `0x0000` and 32-column `0x1800` text captures and row-by-row VRAM probes.
- [x] 9.2 Add `PRINT/test9` (`SCREEN 2`), `PRINT/test10` (`SCREEN 3`), and `PRINT/test11` (`SCREEN 4`) manifest entries with pattern-generator VRAM probes for the rendered PRINT glyphs.
- [x] 9.3 Generate baselines for the five programs, run their regression captures, and record the results in `verification.md`.

## Workflow follow-up

- Review the new PRINT and STRING baselines before treating them as a gate.
- Mark `harden-cpp-parameter-validation-tier1` task 4.4 as covered for `STRING$` when that change is applied.
- Archive the change after review.
