## 1. Measurement and baseline

- [x] 1.1 Measure per-test durations with `./bin/test_unit --duration` and capture the 10 slowest tests; verify the list matches the design (blob 8 KB at ~12.7 s, the three "All … strategies execute successfully" meta-tests, and the six compile-style tests)
- [x] 1.2 Time the actual mutation test binary (`bin/Mutation/test_unit`) baseline and verify whether it exceeds the current 32 s `--timeout`; record the number (38.47 s; exceeds 32 s floor, but effective per-mutant timeout is `baseline*10 ≈ 385 s` so the 32 s flag is a non-issue — no Makefile change)
- [x] 1.3 Evaluate slow-test simplification feasibility per design Decision 5 and record the conclusion; verify no test was changed that reduces mutation detection (conclusion: the 8 KB boundary test cannot shrink without losing coverage — later refined in Decision 5 to exclude it from mutation via a `Slow` suite filter)

## 2. Phase A — resource readers, pletter, and rom.cpp

- [x] 2.1 Add exact-size boundary and truncated/malformed fixtures for `resource_akm_reader.cpp` in `test_resources_extra.cpp` and verify `make -C tests/unit run` passes with new cases
- [x] 2.2 Add exact-size boundary and truncated/malformed fixtures for `resource_akx_reader.cpp` in `test_resources_extra.cpp` and verify `make -C tests/unit run` passes
- [x] 2.3 Add boundary and malformed fixtures for `resource_mtf_map_reader.cpp` in `test_resources_extra.cpp` and verify `make -C tests/unit run` passes
- [x] 2.4 Add boundary fixtures for `resource_spr_reader.cpp` (size and loop boundaries) in `test_resources.cpp` and verify `make -C tests/unit run` passes
- [x] 2.5 Add boundary-arithmetic cases for `resource_manager.cpp` (buildMap limits) and `rom.cpp` layout arithmetic in `test_resources.cpp`/`test_rom.cpp` and verify `make -C tests/unit run` passes
- [x] 2.6 Add pack/unpack edge-case tests for `pletter.cpp` (via the blob reader path) in `test_resources.cpp` and verify `make -C tests/unit run` passes without adding a second slow 8 KB fixture

## 3. Phase B — mull scope and runtime optimization

- [x] 3.1 Add `excludePaths` for third-party code (`src/infrastructure/compression/pletter.cpp`) to `mull.yml`; the embedded kernel assembly is already outside the mutation scope (only `.cpp` files are compiled with the Mull frontend)
- [x] 3.2 Add `--ld-search-path /lib/x86_64-linux-gnu:/lib64` to the `mutation-run` target in `Makefile` (the "Could not find dynamic library" warnings are silenced; confirmed via `make -n mutation-run` that the flag is present)
- [x] 3.3 Verify and record that doctest/test code is already excluded from mutation (test objects are compiled without `-fpass-plugin`); no code change required
- [x] 3.4 Tag the `ResourceBlobPackedReader packs data at the 8K boundary` test with a dedicated `TEST_SUITE("Slow")` (body/assertions unchanged) and pass `--test-suite-exclude=Slow` to the test binary in the `mutation-run` target; verified `make run` still runs it and `--test-suite-exclude=Slow` skips exactly that test (456 passed + 1 skipped)

## 4. Phase C — code optimizer and semantic helpers

- [x] 4.1 Create `tests/unit/src/test_code_optimizer.cpp` exercising each `CompilerCodeOptimizer::addByteOptimized` peephole pattern and the `add hl,de` immediate 1/2/3 branches via a primed `CompilerContext`, and verify `make -C tests/unit run` passes with the new suite
- [x] 4.2 Add both-sides `==`/`!=` guard tests for `compiler_expression_evaluator.cpp` type/subtype checks in `test_compiler_expressions.cpp` and verify `make -C tests/unit run` passes
- [x] 4.3 Add both-sides equality tests for `compiler_symbol_resolver.cpp`, `compiler_variable_emitter.cpp`, and `compiler_float_converter.cpp` in `test_compiler_expressions.cpp` and verify `make -C tests/unit run` passes

## 5. Cross-segment relocation conditional opcodes

- [x] 5.1 Add a MegaROM cross-segment relocation test in `test_compiler.cpp` (`IF … THEN GOTO` / `GOTO` across a 16 KB boundary) that exercises the reachable `jp z`/`jp nz`/`jp`/`0xFF` fixup opcodes via `Compiler::write`, and verify `make -C tests/unit run` passes (the conditional `call` and `jp m/pe/p/po` variants are never emitted by the compiler, so they are left unreachable)

## 6. Final verification

- [x] 6.1 Run the full unit suite and verify all previously passing tests still pass and the total test count increased
- [x] 6.2 Confirm the mutation-score delta changes only test files, `mull.yml`, and the `Makefile` `mutation-run` target — no production source changes
- [ ] 6.3 (user-managed) Run `make mutation-run` and `make mutation-check` and verify the reported mutation score reaches ≥ 85.0%
