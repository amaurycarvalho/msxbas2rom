## ADDED Requirements

### Requirement: Peephole code optimizer has exhaustive dedicated coverage

The peephole code optimizer SHALL have dedicated tests exercising **every** recognized optimization pattern (for example, `pop de` / `ld hl,x` / `push hl` / `ld hl,x` / `pop de` sequences, and `add hl,de` with immediate values 1, 2, and 3), and **every** boundary immediate value of each pattern, so that equality and arithmetic mutants on opcode and immediate-value comparisons do not survive because a branch or value is never exercised. Representative sampling of a subset of patterns is not sufficient.

#### Scenario: Every peephole pattern is exercised
- **WHEN** the optimizer is fed every documented byte-sequence pattern and all of its boundary immediate values
- **THEN** every opcode-equality and immediate-value branch SHALL be executed
- **THEN** the corresponding equality and arithmetic mutants SHALL be killed

### Requirement: Every cross-segment relocation switch opcode is covered

The cross-segment symbol fixup performed during code output SHALL have tests exercising **every** opcode variant its relocation switch handles — the conditional `call` (`nc`/`nz`/`c`/`z`/`p`/`po`/`m`/`pe`), the conditional `jp` (`nc`/`nz`/`c`/`z`/`m`/`pe`/`p`/`po`), the unconditional `call` and `jp`, and the segment-relative LOAD (`0xFF`) case — so that the arithmetic and boundary mutants inside those switch branches are killed. The `p`/`po`/`m`/`pe` variants are never emitted by the current compiler, but because `Compiler::write` reads the opcode from a caller-provided buffer it SHALL still be exercised white-box rather than excluded.

#### Scenario: Every fixup opcode is exercised
- **WHEN** the fixup is driven with each `call`/`jp`/LOAD opcode variant the switch handles, including the never-emitted `p`/`po`/`m`/`pe` forms
- **THEN** the corresponding rewrite branch SHALL be executed
- **THEN** the emitted byte sequence SHALL differ from the unrewritten sequence
- **THEN** the arithmetic and boundary mutants in that branch SHALL be killed
- **AND** the expected byte assertions SHALL assert Z80 opcode constants and compiler-owned layout arithmetic literally, while kernel-resolved addresses (`mr_call_target`/`mr_jump_target`/`mr_get_data_target`) SHALL be derived at test runtime from `bin_header_bin[DISP_MR_* *2]` rather than hardcoded, so the test does not break when the kernel assembly is reassembled

### Requirement: Exact-output fixtures use sentinel-diverse data

Tests that assert exact emitted bytes or decoded tables SHALL seed their input buffers with a **distinct sentinel value per offset**, so that an arithmetic or index mutant that reads or writes the wrong offset produces a byte that differs from the asserted one. Fixtures that fill buffers with zeros or repeated values are insufficient, because a wrong-offset access can then be indistinguishable from the intended one.

#### Scenario: Index mutants are observable
- **WHEN** a test seeds a buffer with a distinct sentinel at every position and asserts the exact bytes read/written by the code under test
- **THEN** a mutation that changes an index or offset expression SHALL change at least one asserted byte
- **THEN** the corresponding arithmetic/index mutant SHALL be killed

### Requirement: Kernel-derived expected values are computed at test runtime

Expected values that depend on the embedded kernel assembly (`bin_header_bin`, `DISP_*` dispatch-table entries, `def_wrapper_routines_map_table`, `getKernelCallAddr` results, and `rom.cpp` mapper-patch `kernelAddr`/`offset`) SHALL be derived at test runtime from those same sources, mirroring the production expression, and SHALL NOT be hardcoded. Only Z80 ISA opcodes, compiler-owned geometry/arithmetic constants, and the documented compiler↔kernel ABI byte sequences MAY be asserted literally.

#### Scenario: Kernel reassembly does not break the tests
- **WHEN** the embedded kernel assembly is reassembled and the kernel-derived addresses change
- **THEN** the tests SHALL recompute the expected values from `bin_header_bin`/`DISP_*`/`def_wrapper_routines_map_table`
- **THEN** the tests SHALL still pass without editing their expected values
- **THEN** the arithmetic mutants in the compiler's own index/offset math SHALL remain killed

### Requirement: Complex binary readers cover every boundary and error path

The AKM, AKX, MTF, SPR, and CSV resource readers SHALL have tests covering **every** exact-size boundary (`==` and `±1` on each limit) and **every** malformed or truncated input that reaches a distinct error path, so that boundary-comparison and arithmetic mutants in these parsers do not survive because only valid files or a single boundary is exercised.

#### Scenario: Every reader boundary and error path is exercised
- **WHEN** a reader is given inputs at every size boundary (exact, one over, one under) and every malformed or truncated shape
- **THEN** every boundary comparison SHALL be evaluated on both sides
- **THEN** every distinct error path SHALL be executed
- **THEN** the corresponding boundary and arithmetic mutants SHALL be killed

### Requirement: Type-dispatch equality is exhaustively covered

The type/subtype dispatch equality checks in `compiler_expression_evaluator.cpp` and the graphics copy/line/put/circle/get statement strategies, plus the equality guards in `compiler_symbol_resolver.cpp` and `compiler_variable_emitter.cpp`, SHALL have tests that drive **every** reachable type/subtype pair through the dispatch and assert the emitted result, so that `eq_to_ne` mutants on those branches do not survive because a specific type or subtype is never exercised.

#### Scenario: Every type/subtype pair is exercised
- **WHEN** the evaluator and statement strategies are invoked with every reachable type/subtype combination
- **THEN** every `==`/`!=` dispatch branch SHALL be evaluated on both sides
- **THEN** the corresponding `eq_to_ne`/`ne_to_eq` mutants SHALL be killed

### Requirement: Equivalent mutants may be excluded via scope narrowing

When a surviving mutant is provably equivalent (no test can distinguish the mutant's output) or resides in code the compiler never emits, the mutation scope SHALL MAY be narrowed by adding the specific region to `excludePaths` in `mull.yml`, as a fallback applied only after test-led coverage has been exhausted. This SHALL NOT be used to exclude mutants that are merely hard to reach with tests.

#### Scenario: Unreachable or equivalent code is excluded
- **WHEN** a mutant region is demonstrated to be equivalent or never emitted
- **THEN** that region MAY be added to `mull.yml` `excludePaths`
- **THEN** the mutation score SHALL be computed over the remaining project-owned, reachable code

### Requirement: Slow tests are excluded from mutation testing

Tests that are prohibitively slow to run once per mutant (identified by measured per-test duration, e.g. the 8 KB-boundary blob test) SHALL be tagged with a dedicated doctest suite (e.g. `Slow`) and SHALL be excluded from Mull mutation runs via `--test-suite-exclude`, while remaining part of the normal `make run` suite. This keeps the mutation baseline small without altering the test's assertions.

#### Scenario: Slow test is skipped during mutation
- **WHEN** `make mutation-run` invokes Mull with `--test-suite-exclude` for the slow suite
- **THEN** the slow test SHALL NOT run during Mull's warm-up, baseline, or mutant runs
- **THEN** the mutation baseline SHALL be reduced accordingly

#### Scenario: Slow test still runs in the normal suite
- **WHEN** `make run` executes the unit-test binary without exclusion filters
- **THEN** the slow test SHALL run and pass as before

### Requirement: Mutation scope is limited to project-owned code

The Mull mutation scope SHALL exclude third-party code via `excludePaths` in `mull.yml` (at minimum `src/infrastructure/compression/pletter.cpp`), so that the mutation score is computed over project-owned source files rather than adapted code.

#### Scenario: Third-party and embedded code is not mutated
- **WHEN** `make mutation-run` executes with `excludePaths` configured in `mull.yml`
- **THEN** no mutants SHALL be generated in the excluded paths
- **THEN** the reported mutation score SHALL be computed over project-owned files only

## MODIFIED Requirements

### Requirement: Mutation run timeout is proportionate to test duration

The Mull run SHALL use a timeout of 32 seconds (32000 ms) for both `--timeout` and `--minimum-timeout`, so that the warm-up (baseline) run of the whole doctest suite completes within the per-run timeout. The doctest suite has grown past the previously assumed 10–15 second baseline; the implementation SHALL measure the actual baseline of the mutation test binary and keep `--timeout` above the full-suite wall time. Because Mull executes the entire test binary on each run, `--timeout` is a per-run cap and SHALL stay above the full-suite wall time; per-mutant hangs are then bounded by Mull's effective timeout `max(baseline*10, minimum-timeout)`.

#### Scenario: Baseline warm-up run completes
- **WHEN** `make mutation-run` starts and performs its warm-up run of the full doctest suite
- **THEN** the warm-up run SHALL complete without being killed as timed out
- **AND** the mutation run SHALL proceed to mutant execution

#### Scenario: Hanging mutants are bounded and flagged
- **WHEN** a mutant causes an infinite loop during mutation testing
- **THEN** the mutant SHALL be reported with status "Timeout" instead of running indefinitely
- **AND** the mutation run SHALL NOT wait on a single hanging mutant for more than the effective per-mutant timeout (`max(baseline*10, 32000)`)
- **AND** a per-run cap below the full-suite duration SHALL NOT be used, because it aborts the warm-up run before any mutant is executed

### Requirement: Existing tests remain unchanged

No existing unit test SHALL be modified or removed while raising the mutation score, except for the slowest tests, which MAY be tagged with a slow suite and excluded from mutation runs (their assertions remaining unchanged). New coverage SHALL otherwise be added through new test cases only.

#### Scenario: Existing suite is preserved
- **WHEN** new test cases are added
- **THEN** all previously passing tests SHALL continue to pass
- **THEN** the total test count SHALL increase, not decrease

#### Scenario: Slow test tagging preserves behavior
- **WHEN** a slow test is tagged with a slow suite and excluded from mutation runs
- **THEN** the test SHALL still run and pass in the normal unit-test suite
- **THEN** the test's assertions SHALL remain unchanged
