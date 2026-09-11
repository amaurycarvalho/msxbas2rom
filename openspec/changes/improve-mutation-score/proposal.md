## Why

The mutation score is now **73%** after Round 3 (`mutation_report.json` of set/08: 1791 killed + 10 timeout / 2445 mutants = 644 survivors), still short of the 85% target mandated by the `mutation-testing` spec and enforced by `make mutation-check`. Reaching 85% requires **2079 killed** (2445 × 0.85 = 2078.25), i.e. **~288 more kills**. The history: Round 1 moved 66% → 70% by sampling a representative case per branch; Round 2 attempted exhaustive enumeration but reported 70% because tests asserted high-level properties ("it compiled", `written > 0`); Round 3 switched to exact-output assertions and moved 70% → 73%. The residual 644 survivors are dominated by **index/offset mutants** (`dest[address - N]` → `address + N`, `data[i + 1]` → `i - 1`) whose Round 3 fixtures use **zeros/equal values**, so reading or writing the wrong offset yields the same byte the test asserts — the mutant is equivalent *for that fixture*. The fix is **distinct sentinel bytes per offset** plus both-sides assertions for the remaining equality/relational gaps, so that any wrong index changes an observable byte.

## What Changes

- **Round 3 (implemented, 70% → 73%)** rewrote the targeted unit tests (doctest) so they assert the **exact emitted bytes/values** each mutant alters, ordered highest-kill-first and driven by the survivor locations (mutator + line) in the latest `mutation_report.json`:
  - **Phase 5 — relocation byte-exact (`compiler.cpp`):** white-box the MegaROM relocation tests via `Compiler::write` to seed each reachable opcode (conditional call/jp `nc`/`nz`/`c`/`z`, unconditional call/jp, `0xFF` LOAD) and assert the exact `out[]` bytes (`0x18 0x07` same-segment `jr $+8`, `F5 08 D9 F1` push/exx, `0x38/0x30/0x28/0x20 0x0A` + `0xC3/0xCD` conditionals, `jr $+6` + nops), plus exact layout arithmetic (`segm_from/to`, `new_address % 0x4000`, `segm_total`, `addr_within_segm >= 0xA000`). ~90 kills.
  - **Phase 6 — reader boundary fixtures with exact-value assertions:** SPR/AKM/AKX/MTF/CSV/blob/data readers assert the exact decoded tables/bytes at every size boundary (`==`, `±1`); the AKM linker state machine (~43) is recorded as out-of-scope, not tested. ~90 kills.
  - **Phase 7 — type-dispatch both-sides assertions:** `compiler_expression_evaluator`, the graphics copy/line/put/circle/get/pset strategies, `compiler_symbol_resolver`, `compiler_variable_emitter`, and `compiler_float_converter::getUsingFormat` (exact format bits) assert the emitted result on both sides of every `==`. ~105 kills.
  - **Phase 8 — graphics strategy sub-commands both-sides:** SET/COLOR/SCREEN/COPY/ON/LET/IF assert exact byte/entity counts and both branches of `t>=2`/`t>=3`/`i>1`/`t<=3`. ~70 kills.
  - **Phase 9 — optimizer + long tail:** complete every peephole pattern with exact emitted-byte assertions; one exact-value assertion per surviving inc/relational location across parser/lexer/domain/symbols/cli and `rom.cpp`/`graphics_statement_strategy`. ~65 kills.
- **Round 4 (73% → 85%)** adds new test cases driven by the 644 remaining survivors, using **distinct sentinel bytes per offset** so index/offset mutants are no longer equivalent for the fixture:
  - **Phase 11 — relocation + layout sentinel byte-exact (`compiler.cpp`):** seed the white-box relocation harness with a sentinel per offset and assert every preamble/payload byte for **every** switch opcode — including the never-emitted `call p/po/m/pe` and `jp m/pe/p/po`, now exercised because `Compiler::write` reads `dest[address-1]` from a caller buffer — plus layout boundaries at `0x9FFF`/`0xA000`/`0xA001` and `0x3FF0`/`0x4000`. ~35 kills.
  - **Phase 12 — reader sentinel fixtures:** SPR/AKX/AKM-non-linker/MTF/CSV/blob/data readers seeded with a unique byte per position, asserting exact decoded tables and every size boundary. The AKM linker state machine (~43) stays out of scope/recorded. ~55 kills.
  - **Phase 13 — compiler semantic byte-exact:** `compiler_variable_emitter` (x/y factor branches and power-of-two loop), `compiler_code_optimizer` (every peephole + `getKernelCallAddr`), `compiler_expression_evaluator` (both sides of every type/subtype `==`), `compiler_symbol_resolver`, `compiler_float_converter`. ~60 kills.
  - **Phase 14 — compiler statement strategies both-sides:** SET/ON/FOR/IF/COLOR/SCREEN/COPY/LINE/GET/PUT/CIRCLE/PAINT/PSET/SPRITE/KEY, IO/CMD handlers and functions assert exact byte/entity counts and both branches of every `==`/`>=`/`<=`. ~65 kills.
  - **Phase 15 — parser counters & branches:** `graphics_statement_strategy` `sepCount`/`parmCount` plus the file/data/def/put/print/set/if/color/generic/next/dim/on strategies and parser core. ~50 kills.
  - **Phase 16 — long tail kernel-safe:** `rom.cpp` mapper patches with kernel addresses **derived at runtime**, `resource_manager`, symbol exports, lexer boundaries, cli/domain/infra. ~40 kills.
  - **Phase 17 — fallback + final verification:** record the accepted equivalents/unreachable mutants (AKM linker, dead optimizer branch, timeouts); `mull.yml` unchanged; user runs the full mull.
- **Fallback scope narrowing:** kept only for provably equivalent/unreachable mutants, but note that `excludePaths` matches whole files (Decision 8), so it cannot target individual switch cases; `mull.yml` remains unchanged and the accepted-equivalent mutants are recorded instead.
- **Anti-fragility constraint (Decisions 11/13/14):** exact-output tests use **distinct sentinel bytes per offset** and never hardcode kernel-derived values. Z80 opcodes, compiler-owned layout arithmetic, and the documented compiler↔kernel ABI are asserted literally; kernel-resolved addresses (`mr_call_target`/`mr_jump_target`/`mr_get_data_target`, `getKernelCallAddr`, `rom.cpp` mapper-patch `kernelAddr`/`offset`) are derived at test runtime from `bin_header_bin`/`DISP_*`/`def_wrapper_routines_map_table`, so the suite does not break when the embedded kernel assembly is reassembled.
- No production source changes; this change touches tests only (the sole production change, `Compiler::getContext()`, was added in Round 3). The `Makefile` `mutation-run` target is unchanged (Round 1 already added `--ld-search-path` and `--test-suite-exclude=Slow`).

## Capabilities

### New Capabilities

<!-- None. The mutation-testing capability already exists. -->

### Modified Capabilities

- `mutation-testing`: Strengthen the coverage requirement from *exercising each branch* to *asserting the exact emitted bytes/values* **with sentinel-diverse fixtures**, so tests fail when an arithmetic, relational, equality, or index mutant changes the output rather than when a branch is merely reached; require that kernel-derived expected values are computed at test runtime; retain the requirement permitting further `excludePaths` narrowing when mutants are provably equivalent; record that mull-runner 0.34.0 exposes no per-mutant filter, so incremental verification is via the unit suite and the full `make mutation-run` is a user-managed step.

## Impact

- `tests/unit/src/*.cpp`: new exact-output test cases (existing cases preserved) in `test_compiler.cpp` (relocation + layout sentinel white-box), `test_resources.cpp`/`test_resources_extra.cpp` (reader sentinel fixtures), `test_compiler_expressions.cpp` (type dispatch + variable emitter), `test_code_optimizer.cpp` (peephole), `test_compiler_graphics.cpp`/`test_compiler_set.cpp`/`test_compiler_on.cpp` (statement strategies), `test_parser.cpp`/`test_lexer.cpp` (counters/boundaries), `test_rom.cpp` (kernel-derived mapper patches), and `test_symbols.cpp` (exports).
- `mull.yml`: no change — the fallback is not viable at sub-file granularity (Decision 8).
- `Makefile`: no change.
- No production code, APIs, or dependencies change.
