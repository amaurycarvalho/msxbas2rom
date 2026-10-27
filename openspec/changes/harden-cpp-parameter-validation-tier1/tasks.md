# Tasks

## 1. Parser contract

- [ ] 1.1 Replace the parameterless no-op parser path with a strategy that rejects trailing tokens for `BEEP`, `CLS`, `END`, `PLYMUTE`, `PLYPLAY`, and `PLYREPLAY`; verify `tests/unit/src/test_parser.cpp` accepts valid forms and rejects `BEEP 1`, `CLS 1`, and trailing non-comment tokens.
- [ ] 1.2 Verify a trailing comment or `:` separator remains accepted by the parser tests.

## 2. Arkos command defaults and arity

- [ ] 2.1 Fix `CMD PLYSOUND` to read the third argument from `actions[2]`, emit channel `2` when omitted, emit inverted volume `0` when omitted, and reject more than three arguments; verify exact bytes in `tests/unit/src/test_mutation_round6.cpp`.
- [ ] 2.2 Add `CMD PLYLOAD` coverage proving one argument emits the same value for both resource slots and more than two arguments fail; verify in `tests/unit/src/test_compiler.cpp`.
- [ ] 2.3 Implement optional `CMD PLYSONG [subsong]` with default `0` and reject more than one argument; verify zero-, one-, and two-argument cases in unit tests.
- [ ] 2.4 Validate `CMD PLYLOOP` literal `0`/`1` and invalid literals; verify pass/fail cases.

## 3. CLOSE, FRE, and RANDOMIZE

- [ ] 3.1 Emit `0xFF` for naked `CLOSE` and assert the emitted bytes in `tests/unit/src/test_mutation_round6.cpp`.
- [ ] 3.2 Require exactly one argument for `FRE`, reject `FRE()` and `FRE(0,1)`, and update `tests/integration/{IO,VAR,GRAPH,MISC}` programs that currently use bare `FRE()` to `FRE(0)`; verify `make -C tests/integration build` and the unit function tests.
- [ ] 3.3 Reject `RANDOMIZE` with an argument and verify parameterless `RANDOMIZE` still compiles in unit tests.

## 4. Types and literal ranges

- [ ] 4.1 Require a string value for `COLOR SPRITE$`, reject numeric values and an empty sprite index, and update the existing `r6_color_sprites` test to use a string value plus a negative numeric case.
- [ ] 4.2 Add signed-literal parsing for range checks and apply it to `SET TILE FLIP/ROTATE` and `SET SPRITE FLIP/ROTATE`; verify `-1` and `3` are rejected while `0..2` compile.
- [ ] 4.3 Validate documented literal ranges for `CMD TURBO`, `CMD PAGE`, `CMD CLIP`, `CMD PLAY`, `SET FONT`, `CMD PLYLOOP`, and non-negative tile banks; add pass/fail table-driven unit tests.
- [ ] 4.4 Cap function calls at four arguments with a syntax error before evaluating/storing subtypes; verify `MID$`, `INSTR`, and `VARPTR` with five arguments fail without crashing.

## 5. Integration and documentation

- [ ] 5.1 Add CLI characterization cases for the new syntax errors and verify the expected exit code and message in `tests/integration/cli/run_cli_characterization.sh`.
- [ ] 5.2 Update affected specs/docs for the one-argument `FRE` form and Arkos defaults; verify `openspec validate --change harden-cpp-parameter-validation-tier1`.
- [ ] 5.3 Run `make test-unit` and `make test-integration` and record the result.

## Workflow follow-up

- Archive the change after review.
