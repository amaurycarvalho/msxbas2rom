# Proposal

## Why

`STRING$` and related string functions return their result in the shared `BUF`, while the compiler allocates a distinct temporary string destination. Concatenating two such calls therefore loses the first result: `STRING$(5,65)+STRING$(6,66)` produces the second string twice instead of `AAAAABBBBBB`.

Separately, the generated startup leaves the VDP in `SCREEN 1` (32 columns) while configuring the BIOS for 40 columns. Every `PRINT` line drifts to the right on screen, which looks like `print_crlf` emitting LF without CR. The CR+LF routine itself is correct. The mismatch exists because the initial mode depends on the machine: MSX1 normally boots in `SCREEN 0` (40 columns), while Japanese MSX2, MSX2+, and turbo R machines boot in `SCREEN 1` (32 columns); non-Japanese MSX2 boots in `SCREEN 0` (80 columns by default).

## What Changes

- Fix `GET_NEXT_TEMP_STRING_ADDRESS` to advance by one 256-byte slot per temporary string instead of one byte.
- Align `STRING$`, `SPACE$`, and `INKEY$` with the existing `BUF` to temporary-string copy pattern used by `LEFT$`, `RIGHT$`, `MID$`, `HEX$`, and `USING$`, so each result survives until concatenation.
- Normalise startup display explicitly to `SCREEN 0` with `WIDTH 40`, matching `LINL40`/`LINLEN` and producing one deterministic 40-column layout on MSX1, MSX2, MSX2+, and turbo R.
- Add probes for both split programs:
  - `PRINT/test5` for trailing `;`/`,` and tab/separator layout.
  - `STRING/test1` for literal and variable `STRING$` concatenation.
- Review and regenerate the regression probes/baselines of every program that uses `PRINT`, `?`, or `STRING$`. Existing baselines encode the current 32-column mismatch and must be treated as stale.
- Document the initial-screen normalization decision in `doc/adr/ADR-002 Initial text screen normalization.md` and update the start-code section of `doc/misc/architecture.md`.
- Apply the tier-1 hardening for bounded function arity to `STRING$` (reject five or more arguments), reusing `harden-cpp-parameter-validation-tier1` task 4.4. The `harden-cpp-*` changes contain no PRINT/`?`-specific requirement beyond this shared function-arity cap.
- **BREAKING**: the default text layout changes from the inconsistent 32-column display to `SCREEN 0` 40-column text; programs that relied on the old shifted layout render differently.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `compiler`: define distinct temporary-string destinations for string functions and select a consistent 40-column text mode at startup.
- `emulator-regression-tests`: add PRINT/test5 and STRING/test1 probes and require a full review of PRINT/`?`/`STRING$` regression expectations.

## Impact

- Z80 kernel: `GET_NEXT_TEMP_STRING_ADDRESS` stride and string-function result handling.
- Compiler strategies: `STRING$`, `SPACE$`, `INKEY$`, and the startup code emitted by the start statement strategy.
- Regression assets: `tests/integration/PRINT/test5.bas`, `tests/integration/STRING/test1.bas`, `tests/integration/regression/manifest.json`, and baselines.
- 119 existing baselines belong to programs that use `PRINT`, `?`, or `STRING$`; 94 currently record `screen: 1` and must be reviewed or regenerated. Programs with explicit `SCREEN` statements must still be rechecked if they use `STRING$`.
- Documentation: a new ADR records the per-machine boot-mode research and the 40-column normalization decision; the architecture guide describes the explicit startup sequence.
- No PRINT CR+LF kernel change is planned; the apparent CR bug is the screen-width mismatch.
