# Tasks

## 1. Kernel bank normalization

- [x] 1.1 In `src/infrastructure/kernel/asm/src/header/31-cmd-tile-sprite.asm`, change the SET all-bank checks in `set_tile.paste`, `set_tile_pattern`, `set_tile_color_buf`, and `set_tile_pattern_buffer` from `cp 3` + `jr z` to `cp 3` + `jr nc`; verify the kernel builds with `make test-kernel`.
- [x] 1.2 Add the bank normalization guard to `get_tile_pattern` and `get_tile_color` so `B >= 3` becomes bank 0 before the bank loop; verify the kernel builds with `make test-kernel`.
- [x] 1.3 Force single-bank pattern behavior for `SCRMOD < 2` in `set_tile_pattern`, `set_tile_pattern_buffer`, `get_tile_pattern`, and `set_tile.paste`; verify the kernel builds with `make test-kernel`.
- [x] 1.4 Regenerate the kernel artifacts (`header.bin`, `header.h`, `start.h`, symbols, and manifests) and verify `make test-kernel` reports valid size, symbols, and filler.

## 2. Compiler buffer and GET destination classification

- [x] 2.1 Add a compiler helper that returns a buffer identifier only for a declared array (`isArray`) or an `ARRAY` action wrapping one declared array identifier, with a symbol-resolver fallback; verify with unit tests for scalar, `C%()`, and `(C%)` forms.
- [x] 2.2 Use the helper in the `SET TILE PATTERN` and `SET TILE COLOR` buffer paths so `SET TILE COLOR 5, F%, G%` and `SET TILE COLOR 5, F%` use the scalar color path; verify in `tests/unit/src/test_compiler_set.cpp` by checking the emitted `set_tile_color` call and absence of `set_tile_color_buf`.
- [x] 2.3 Use the helper in `GET TILE PATTERN` and `GET TILE COLOR`, reject literals, scalar variables, and undeclared identifiers, and accept declared arrays; verify with positive and negative unit tests in `tests/unit/src/test_compiler_set.cpp`.
- [x] 2.4 Support the parenthesized single-array form (`(C%)`) for GET and SET buffer paths; verify it compiles and uses the same buffer call as `C%()`.

## 3. Bank and parameter shape validation

- [x] 3.1 Reject an `ARRAY` or buffer in every tile bank position and honor a scalar variable bank, including `SET TILE COLOR 5, (1,2,3,4), BK%`; verify with unit tests that the generated code uses `BK%` and that an array bank produces a command-specific error.
- [x] 3.2 Normalize numeric literal bank values above 2 to bank 3 in the compiler; verify unit tests for banks 3, 9, 200, and 300 emit bank 3 and for banks 0, 1, and 2 keep their literal value.
- [x] 3.3 Reject inline `SET TILE PATTERN` parameter counts above 3 and cap pattern and color tuples/arrays at eight rows while preserving partial and `null` rows; verify with positive and negative unit tests in `tests/unit/src/test_compiler_set.cpp` and parser tests in `tests/unit/src/test_parser.cpp`.
- [x] 3.4 Fix `SET TILE COLOR` so the bank action never clobbers the FC/BC operand actions: `t=3` FC array + bank, `t=4` simple FC/BC + bank, and `t=4` FC/BC arrays + bank must use actions[1], actions[2], and the bank action independently. Verify with unit tests for `SET TILE COLOR 66,2,8,1`, `SET TILE COLOR 69,(8,7,6,5,4,3,2,1),2`, and the FC+BC arrays + bank form, and with the `test97`/`tile_bank_all` probes.

## 4. Color and direction validation

- [x] 4.1 Mask scalar and array background values to four bits and keep foreground values masked; verify unit tests inspect the emitted `and 0x0F` sequence and a regression probe for `SET TILE COLOR 5, 1, 16` writes `0x10`.
- [x] 4.2 Reject literal `SET TILE FLIP` and `SET TILE ROTATE` directions outside `0..2` while leaving non-literal expressions accepted; verify with unit tests in `tests/unit/src/test_compiler_set.cpp`.

## 5. Integration and regression coverage

- [x] 5.1 Add deterministic VRAM probes to `tests/integration/GRAPH/test87.bas` (SCREEN 1 single-bank pattern/color), `test88.bas` (SCREEN 2 default-all pattern/color), and `test97.bas` (all color bank forms); remove their blocking `INPUT$` waits and end each in a capture loop. Verify all three compile with the integration build or the release binary.
- [x] 5.2 Add `tests/integration/GRAPH/tile_bank_all.bas` covering banks 3, 9, and 200 for inline pattern and scalar color, bank 9 and 200 for FLIP/ROTATE, and GET with banks 3 and 200 followed by writes that prove the bank 0 fallback. Verify it compiles.
- [x] 5.3 Add manifest entries for `test87`, `test88`, `test97`, and `tile_bank_all` with `volatile: true` and the exact `vram.probes`; verify with `python3 tests/integration/regression/runner.py list --scenario GRAPH` and targeted runs for each program.
- [x] 5.4 Explicitly run `test87`, `test88`, `test94`, `test97`, `tile_flip_rotate`, and `tile_bank_all`; also confirm that `base2` and `test93` remain in the GRAPH run even though they only use `SET TILE ON`. Verify with `python3 tests/integration/regression/runner.py run --scenario GRAPH`.
- [x] 5.5 Confirm all affected programs are volatile and do not require baseline regeneration; record that `test94` and `tile_flip_rotate` remain volatile and are run without baselines.

## 6. Final verification

- [x] 6.1 Run `make test-unit` and confirm all compiler, parser, and generated-Z80 tests pass.
- [x] 6.2 Run `make test-kernel` and confirm the rebuilt kernel passes size, symbol, relocation, and filler checks.
- [x] 6.3 Run `make test-integration` and `make test-regression` (or `runner.py run --scenario GRAPH`) and confirm `test87`, `test88`, `test94`, `test97`, `tile_flip_rotate`, and `tile_bank_all` all pass their probes.
- [x] 6.4 Run `make test-architecture` and confirm no architecture or test-inventory violations were introduced.

## Workflow follow-up

- Archive the change after implementation and verification are complete.
