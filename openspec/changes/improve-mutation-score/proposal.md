## Why

The mutation score is **70%** (1714 killed + 4 timeout = 1718 detected / 2445 mutants), still short of the 85% target mandated by the `mutation-testing` spec and enforced by `make mutation-check`. Round 1 of this change moved the score 66% → 70% (+~118 kills) but under-delivered because it sampled a *representative* case per branch (a few relocation opcodes, a few type pairs, a few reader boundaries) instead of exhausting the reachable cases. A fresh analysis of `mutation_report.json` shows **727 surviving mutants**, dominated by three areas Round 1 never fully reached: the cross-segment relocation switch in `compiler.cpp` (106 survivors, ~85 arithmetic), the AKM binary reader (75), and the type-dispatch equality checks in `compiler_expression_evaluator.cpp` (42, of which 30 `eq_to_ne`). Reaching 85% requires killing **~361** more survivors (about half of those remaining), which is achievable only by exhausting every reachable branch, with a scope-narrowing fallback for genuinely equivalent mutants.

## What Changes

- Add **exhaustive** targeted unit tests (doctest), ordered easiest → hardest and driven by the survivor locations in `mutation_report.json` (mutator + line, not "more subtype tests"):
  - **Phase 1 — easy (counters & boundary arithmetic on already-tested paths):** `CompilerFloatConverter::getUsingFormat` format-bit assertions; graphics-strategy byte/entity counts; `resource_manager` map-limit arithmetic; `rom.cpp` layout offsets; and the long tail of single inc/relational mutants across parser/lexer/domain/symbols.
  - **Phase 2 — medium (binary readers & strategies):** exhaustive boundary/malformed fixtures for AKM/AKX/MTF/SPR/CSV readers; both-sides sub-command tests for SET/COLOR/ON strategies; equality coverage for `compiler_symbol_resolver` and `compiler_variable_emitter`.
  - **Phase 3 — hard (type dispatch & relocation):** drive every type/subtype pair in `compiler_expression_evaluator` and the graphics copy/line/put/circle/get strategies; complete every peephole pattern in `compiler_code_optimizer`; exercise every reachable relocation opcode in `compiler.cpp`.
- **Fallback scope narrowing (only if equivalent mutants block 85%):** extend `mull.yml` `excludePaths` for defensive/unreachable code that no test can distinguish — documented as a last resort, not the primary lever.
- No production source changes; this change touches tests and, only if the fallback is needed, `mull.yml`. The `Makefile` `mutation-run` target is unchanged (Round 1 already added `--ld-search-path` and `--test-suite-exclude=Slow`).

## Capabilities

### New Capabilities

<!-- None. The mutation-testing capability already exists. -->

### Modified Capabilities

- `mutation-testing`: Strengthen the coverage requirements from *representative* to *exhaustive enumeration* (every reachable relocation opcode, every reader boundary, every peephole pattern, every type/subtype pair); add a requirement permitting further `excludePaths` narrowing when mutants are provably equivalent; record that mull-runner 0.34.0 exposes no per-mutant filter, so incremental verification is via the unit suite and the full `make mutation-run` is a user-managed step.

## Impact

- `tests/unit/src/*.cpp`: new exhaustive test cases, primarily in `test_compiler_expressions.cpp`, `test_compiler.cpp`, `test_resources.cpp`, `test_resources_extra.cpp`, and `test_code_optimizer.cpp`.
- `mull.yml`: only if the Phase-4 fallback is needed (add `excludePaths` for defensive/unreachable code).
- `Makefile`: no change.
- No production code, APIs, or dependencies change.
