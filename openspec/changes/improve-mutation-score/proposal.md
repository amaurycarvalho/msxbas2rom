## Why

The mutation score is stuck at **66%** (1704 detected / 2549 mutants: 1695 killed + 9 timeout), still short of the 85% target already mandated by the `mutation-testing` spec and enforced by `make mutation-check`. The previous attempt (`raise-mutation-score-to-85`) moved the score only 65% → 66% (+29 kills) because it focused on easy paths and left the highest-survivor modules untouched. A fresh analysis of `mutation_report.json` shows **845 surviving mutants**, dominated by three areas the prior plan missed: the peephole code optimizer (6.7% score, zero dedicated tests), the cross-segment relocation switch in `compiler.cpp` (84 arithmetic survivors from untested conditional opcodes), and the complex binary readers (AKM/AKX/MTF). The remaining graphics/semantic survivors are largely equivalent or byte-exact-required, so this change also narrows the mutation scope (exclude third-party/embedded code) and reduces the ~4.8 h per-run cost (a ~27 s baseline run once per mutant) to make the 85% target reachable and faster to verify.

## What Changes

- Add targeted unit tests (doctest) to kill survivors, grouped by module and complexity:
  - **Peephole code optimizer** (`compiler_code_optimizer.cpp`): dedicated tests for each peephole pattern and immediate-value branch (the largest single gap, ~56 survivors at 6.7% score).
  - **Cross-segment relocation** (`compiler.cpp` `write()` fixup): tests that exercise the reachable conditional `jp z`/`jp nz`/`jp`/`0xFF` opcode variants (the conditional `call` and `jp m/pe/p/po` variants are never emitted and are left unreachable).
  - **Complex binary readers** (`resource_akm_reader.cpp`, `resource_akx_reader.cpp`, `resource_mtf_map_reader.cpp`, `resource_spr_reader.cpp`): boundary and error-path fixtures.
  - **Scattered semantic helpers**: both-sides coverage of `==`/`!=` guards in `compiler_expression_evaluator.cpp`, `compiler_symbol_resolver.cpp`, `compiler_variable_emitter.cpp`, `compiler_float_converter.cpp`.
- Optimize the Mull configuration and runtime (replaces the original "graphics strategies" phase, whose survivors were dominated by equivalent and byte-exact-required mutants):
  - **Narrow the mutation scope** via `mull.yml` `excludePaths` (third-party `pletter.cpp`), so the 85% score covers project-owned code.
  - **Silence the JIT library warnings** via `--ld-search-path` in the `mutation-run` target (cosmetic).
  - **Confirm doctest/test code is already excluded** from mutation (test objects compile without `-fpass-plugin`).
  - **Reduce the mull baseline** by excluding the slow 8 KB-boundary blob test from mutation runs (tag it `Slow` and pass `--test-suite-exclude=Slow`), while it still runs in the normal unit-test suite.
- Reconcile the stale Mull timeout assumption in the spec (the suite baseline is ~27 s in mull, not the assumed 10–15 s).
- No production source changes; this change touches tests, `mull.yml`, and the `Makefile` `mutation-run` target.

## Capabilities

### New Capabilities

<!-- None. The mutation-testing capability already exists. -->

### Modified Capabilities

- `mutation-testing`: Add requirements for peephole-optimizer coverage, cross-segment relocation opcode coverage, complex binary-reader coverage, a project-owned mutation scope (`excludePaths`), and exclusion of slow tests from mutation runs; correct the stale timeout baseline; amend the "existing tests remain unchanged" requirement to permit tagging the slowest tests with a slow suite and excluding them from mutation runs.

## Impact

- `tests/unit/src/*.cpp`: new test cases (primarily `test_compiler_expressions.cpp`, `test_compiler.cpp`, `test_resources.cpp`, `test_resources_extra.cpp`), plus a new `test_code_optimizer.cpp`.
- `mull.yml`: add `excludePaths` for `src/infrastructure/compression/pletter.cpp`.
- `Makefile`: add `--ld-search-path` to the `mutation-run` target; possible `mutation-run` timeout adjustment only if the baseline-verification task confirms the suite exceeds the current 32 s cap (to be decided during implementation, not pre-applied).
- No production code, APIs, or dependencies change.
