## Why

The mutation score remains **70%** (1729 killed + 5 timeout / 2445 mutants = 711 survivors), still short of the 85% target mandated by the `mutation-testing` spec and enforced by `make mutation-check`. Round 1 moved 66% → 70% by sampling a *representative* case per branch; Round 2 then attempted *exhaustive* enumeration (every relocation opcode, every reader boundary, every type/subtype pair) but the user-managed run still reported **70%** — only ~16 of the 727 targeted survivors were actually killed. The root cause is now clear from the fresh `mutation_report.json`: the Round 2 tests assert high-level properties ("it compiled", `written > 0`, `codeSize > 0x4000`, trampoline signature) rather than the **exact byte/value each mutant alters** — e.g. `dest[address - 10] = 0x18` mutated to `address + 10` writes to a different offset and no test reads `out[address-10]` back. Reaching 85% requires killing **~350** more survivors (2079 − 1729), which is achievable only by switching the assertion strategy from *"exercises the branch"* to *"asserts the exact emitted bytes/values"*, rewriting the relocation, reader, and optimizer tests as white-box where needed.

## What Changes

- Rewrite the targeted unit tests (doctest) added in Round 2 so they assert the **exact emitted bytes/values** each mutant alters, ordered highest-kill-first and driven by the survivor locations (mutator + line) in the latest `mutation_report.json`:
  - **Phase 5 — relocation byte-exact (`compiler.cpp`):** white-box the MegaROM relocation tests via `Compiler::write` to seed each reachable opcode (conditional call/jp `nc`/`nz`/`c`/`z`, unconditional call/jp, `0xFF` LOAD) and assert the exact `out[]` bytes (`0x18 0x07` same-segment `jr $+8`, `F5 08 D9 F1` push/exx, `0x38/0x30/0x28/0x20 0x0A` + `0xC3/0xCD` conditionals, `jr $+6` + nops), plus exact layout arithmetic (`segm_from/to`, `new_address % 0x4000`, `segm_total`, `addr_within_segm >= 0xA000`). ~90 kills.
  - **Phase 6 — reader boundary fixtures with exact-value assertions:** SPR/AKM/AKX/MTF/CSV/blob/data readers assert the exact decoded tables/bytes at every size boundary (`==`, `±1`); the AKM linker state machine (~43) is recorded as out-of-scope, not tested. ~90 kills.
  - **Phase 7 — type-dispatch both-sides assertions:** `compiler_expression_evaluator`, the graphics copy/line/put/circle/get/pset strategies, `compiler_symbol_resolver`, `compiler_variable_emitter`, and `compiler_float_converter::getUsingFormat` (exact format bits) assert the emitted result on both sides of every `==`. ~105 kills.
  - **Phase 8 — graphics strategy sub-commands both-sides:** SET/COLOR/SCREEN/COPY/ON/LET/IF assert exact byte/entity counts and both branches of `t>=2`/`t>=3`/`i>1`/`t<=3`. ~70 kills.
  - **Phase 9 — optimizer + long tail:** complete every peephole pattern with exact emitted-byte assertions; one exact-value assertion per surviving inc/relational location across parser/lexer/domain/symbols/cli and `rom.cpp`/`graphics_statement_strategy`. ~65 kills.
- **Fallback scope narrowing:** kept only for provably equivalent/unreachable mutants, but note that `excludePaths` matches whole files (Decision 8), so it cannot target individual switch cases; `mull.yml` remains unchanged and the accepted-equivalent mutants are recorded instead.
- **Anti-fragility constraint (Decision 11):** exact-output tests classify each emitted byte and never hardcode kernel-derived values. Z80 opcodes and compiler-owned layout arithmetic are asserted literally; kernel-resolved addresses (`mr_call_target`/`mr_jump_target`/`mr_get_data_target`, `rom.cpp` mapper-patch offsets) are derived at test runtime from `bin_header_bin`/`DISP_*` so the suite does not break when the embedded kernel assembly is reassembled.
- No production source changes; this change touches tests only. The `Makefile` `mutation-run` target is unchanged (Round 1 already added `--ld-search-path` and `--test-suite-exclude=Slow`).

## Capabilities

### New Capabilities

<!-- None. The mutation-testing capability already exists. -->

### Modified Capabilities

- `mutation-testing`: Strengthen the coverage requirement from *exercising each branch* to *asserting the exact emitted bytes/values*, so tests fail when an arithmetic, relational, or equality mutant changes the output rather than when a branch is merely reached; retain the requirement permitting further `excludePaths` narrowing when mutants are provably equivalent; record that mull-runner 0.34.0 exposes no per-mutant filter, so incremental verification is via the unit suite and the full `make mutation-run` is a user-managed step.

## Impact

- `tests/unit/src/*.cpp`: rewritten exact-output test cases, primarily in `test_compiler.cpp` (relocation white-box), `test_resources.cpp`/`test_resources_extra.cpp` (reader boundary fixtures), `test_compiler_expressions.cpp` (type dispatch), `test_compiler_graphics.cpp`/`test_compiler_set.cpp`/`test_compiler_on.cpp` (strategy sub-commands), and `test_code_optimizer.cpp` (peephole).
- `mull.yml`: no change — the fallback is not viable at sub-file granularity (Decision 8).
- `Makefile`: no change.
- No production code, APIs, or dependencies change.
