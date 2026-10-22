## Why

> **Round 6 update (approved plan, release 1.3.1.0):** the fresh post-Phase-A
> `mutation_report.json` baseline is **70%**: 1,783 killed, 733 survived, and
> 12 timeout mutants out of 2,528. Achieving 85% on that report requires at
> least **2,149 killed** mutants (`ceil(2,528 × 0.85)`), or **366 additional
> kills**. No ASM/Z80 survivor is reported; all survivor-driven work targets
> C++ code.
>
> Round 6 is sequenced **after Phase A of
> `refactor-clean-architecture-testability`**: the boundary check, in-memory
> source, kernel image, and resource/symbol boundaries run first so tests are
> faster and deterministic. Phase A invalidates the old line targets; the
> fresh report has been generated and the survivor inventory recomputed before
> Round 6 starts. The **~415-kill budget** below (minimum 366) replaces the
> former ~345-kill provisional estimate.

Before Phase A, the Round 3 baseline was **73%** (`mutation_report.json` of set/08: 1,791 killed + 10 timeout / 2,445 mutants = 644 survivors), still short of the 85% target mandated by the `mutation-testing` spec and enforced by `make mutation-check`. The history: Round 1 moved 66% → 70% by sampling a representative case per branch; Round 2 attempted exhaustive enumeration but reported 70% because tests asserted high-level properties ("it compiled", `written > 0`); Round 3 switched to exact-output assertions and moved 70% → 73%. Those residuals were dominated by **index/offset mutants** (`dest[address - N]` → `address + N`, `data[i + 1]` → `i - 1`) whose Round 3 fixtures use **zeros/equal values**, so reading or writing the wrong offset yields the same byte the test asserts — the mutant is equivalent *for that fixture*. The fix remains **distinct sentinel bytes per offset** plus both-sides assertions for the remaining equality/relational gaps, so that any wrong index changes an observable byte. The post-Phase-A report shows the same pattern at a larger scale: 733 survivors, dominated by `eq_to_ne` (195), `post_inc` (110), relational (254), and arithmetic (147) mutants.

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

- **Round 6 (after CleanArch Phase A, target >85%)** replaces the Round 5
  plan and does not reopen completed Rounds 1-4:
  - consumes the fresh post-Phase-A `mutation_report.json` (70%, 1,783 killed,
    733 survived, 12 timeout out of 2,528) and regenerates the survivor
    inventory; the old file/line targets are not reused;
  - prioritises C++ statement strategies (229), binary resource readers (145),
    semantic helpers/optimizer (98), parser (98), `compiler.cpp`
    relocation/layout (46), and the builder/symbols/lexer/application tail
    (117); no ASM/Z80 survivor is targeted;
  - uses direct, data-driven unit harnesses and byte-exact or normalised
    fingerprint assertions for generated code, instead of many costly BASIC
    end-to-end compilations;
  - preserves kernel-reassembly safety: kernel addresses are runtime-derived;
    generated-code fingerprints mask only derived operands while retaining
    opcodes, compiler-owned immediates, and layout bytes;
  - does not add global exact-distance tests for kernel routines. Dispatch
    integrity is checked structurally; an exact local distance is allowed only
    where a spec explicitly declares that layout span a stable ABI contract;
  - uses the shared `compileSource()` and `kernelImage()` test seams from
    Phase A so the cleanarch refactor can replace their internals without
    rewriting the mutation tests;
  - profiles the suite and moves only redundant aggregate tests to `Slow`
    after focused replacements cover their mutation-relevant assertions;
    normal-suite coverage is preserved;
  - prohibits temporary `.bas` files in existing and future tests: BASIC
    scenarios are supplied through the Phase A source boundary.
- **Kill budget (~415, minimum 366 on the post-Phase-A report):**
  graphics statements 75, control/IO/basic/sound 60, semantic/optimizer 60,
  compiler core 30, builder readers 85, parser 55, long tail 50. The ~415
  target leaves a ~49-kill margin over the 366 needed to reach 2,149 killed.
- **Target release: 1.3.1.0.**

## Capabilities

### New Capabilities

<!-- None. The mutation-testing capability already exists. -->

### Modified Capabilities

- `mutation-testing`: Strengthen the coverage requirement from *exercising each branch* to *asserting the exact emitted bytes/values* **with sentinel-diverse fixtures**, so tests fail when an arithmetic, relational, equality, or index mutant changes the output rather than when a branch is merely reached; require that kernel-derived expected values are computed at test runtime; require survivor-driven work to use a fresh report after a production architecture refactor; require C++-only mutation targets with Z80 regression tests excluded from the score; retain the requirement permitting further `excludePaths` narrowing when mutants are provably equivalent; record that mull-runner 0.34.0 exposes no per-mutant filter, so incremental verification is via the unit suite and the full `make mutation-run` is a user-managed step.

## Impact

- `tests/unit/src/*.cpp`: new exact-output test cases (existing cases preserved) in `test_compiler.cpp` (relocation + layout sentinel white-box), `test_resources.cpp`/`test_resources_extra.cpp` (reader sentinel fixtures), `test_compiler_expressions.cpp` (type dispatch + variable emitter), `test_code_optimizer.cpp` (peephole), `test_compiler_graphics.cpp`/`test_compiler_set.cpp`/`test_compiler_on.cpp` (statement strategies), `test_parser.cpp`/`test_lexer.cpp` (counters/boundaries), `test_rom.cpp` (kernel-derived mapper patches), and `test_symbols.cpp` (exports).
- New test-only files: `tests/unit/src/test_support.h` (shared `compileSource()`/`kernelImage()` seams), `tests/unit/src/test_generated_z80.cpp` (normalised fragment fingerprint), and `tests/unit/src/test_kernel_layout.cpp` (dispatch integrity and limited declared ABI spans).
- Depends on Phase A of `refactor-clean-architecture-testability`; the old `mutation_report.json` line targets are invalidated by that phase and must be regenerated.
- `mull.yml`: no change — the fallback is not viable at sub-file granularity (Decision 8).
- `Makefile`: no change.
- No production code, APIs, or dependencies change in this change; the Phase A production refactor belongs to `refactor-clean-architecture-testability`.
