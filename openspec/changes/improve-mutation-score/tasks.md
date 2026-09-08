# Round 1 (concluída — 66% → 70%)

Round 1 added representative tests (peephole, both-sides equality, relocation, reader fixtures) and the runtime/scope optimizations. Its final user-managed run reported **70%** (1714 killed + 4 timeout / 2445), short of 85%, which motivates Round 2 below.

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

## 6. Round 1 verification

- [x] 6.1 Run the full unit suite and verify all previously passing tests still pass and the total test count increased
- [x] 6.2 Confirm the mutation-score delta changes only test files, `mull.yml`, and the `Makefile` `mutation-run` target — no production source changes
- [x] 6.3 (user-managed) Run `make mutation-run` and `make mutation-check` — result was **70%** (1714 killed + 4 timeout / 2445), below the 85% threshold; see Round 2 below

---

# Round 2 (70% → 85%)

Ordered easiest → hardest, targeting the 727 surviving mutants in `mutation_report.json`. Every task verifies with `make -C tests/unit run` (fast, incremental); no `mutation-build`/`mutation-run`/`mutation-clean` is run by the implementer, and the full Mull run is the user's final step (task 10.3).

## 7. Phase 1 — easy (counters & boundary arithmetic)

- [x] 7.1 Add a `getUsingFormat` test in `test_compiler_expressions.cpp` asserting the exact returned format bits for format strings with multiple occurrences of each char (`#`, `0`, `+`, `*`, `$`, `,`, `.`, `^`), and verify `make -C tests/unit run` passes (kills the 15 `post_inc_to_post_dec` + 1 `add_to_sub` + 2 `lt` survivors in `compiler_float_converter.cpp`)
- [x] 7.2 Add byte/entity-count assertions for the graphics statement strategies in `test_compiler_graphics.cpp`, and verify `make -C tests/unit run` passes (kills the 18 `post_inc_to_post_dec` + 8 `ge` survivors in `graphics_statement_strategy.cpp`)
- [x] 7.3 Add buildMap limit and post-increment assertions for `resource_manager.cpp` in `test_resources.cpp`, and verify `make -C tests/unit run` passes (kills the 7 `post_inc` + 5 `gt`/`lt` survivors)
- [x] 7.4 Add exact layout-offset/segment assertions for `rom.cpp` in `test_rom.cpp`, and verify `make -C tests/unit run` passes (kills the ~13 arithmetic/relational survivors)
- [x] 7.5 Sweep the long-tail files (parser/lexer/domain/symbols/cli with ≤5 survivors each) adding one assertion per surviving inc/relational location, and verify `make -C tests/unit run` passes (kills a large share of the ~177 long-tail survivors)

## 8. Phase 2 — medium (binary readers & strategies)

- [x] 8.1 Add boundary fixtures for `resource_akm_reader.cpp` (header, subsong, instrument, and `guessBaseAddress` branches — the ~32 non-linker survivors), and verify `make -C tests/unit run` passes (the ~43 linker-state-machine survivors are out of scope per user decision)
- [x] 8.2 Add exhaustive boundary and malformed fixtures for `resource_spr_reader.cpp`, `resource_akx_reader.cpp`, `resource_mtf_map_reader.cpp`, and `resource_csv_reader.cpp`, and verify `make -C tests/unit run` passes (kills ~19 + 9 + 5 + 7 survivors)
- [x] 8.3 Add both-sides sub-command tests for `compiler_set_statement_strategy.cpp`, `compiler_color_statement_strategy.cpp`, and `compiler_on_statement_strategy.cpp`, and verify `make -C tests/unit run` passes (kills ~27 + 18 + 16 survivors)
- [x] 8.4 Add per-type equality coverage for `compiler_symbol_resolver.cpp` and `compiler_variable_emitter.cpp` in `test_compiler_expressions.cpp`, and verify `make -C tests/unit run` passes (kills ~14 + 13 survivors)

## 9. Phase 3 — hard (type dispatch & relocation)

- [x] 9.1 Drive every reachable type/subtype pair through `compiler_expression_evaluator.cpp` in `test_compiler_expressions.cpp`, and verify `make -C tests/unit run` passes (kills the ~30 `eq_to_ne` + arithmetic survivors)
- [x] 9.2 Complete every peephole pattern and immediate-value branch in `test_code_optimizer.cpp`, and verify `make -C tests/unit run` passes (kills the remaining ~23 survivors in `compiler_code_optimizer.cpp`)
- [x] 9.3 Add both-sides `eq` dispatch tests for the graphics copy/line/put/circle/get strategies in `test_compiler_graphics.cpp`, and verify `make -C tests/unit run` passes (kills ~44 `eq_to_ne` survivors)
- [x] 9.4 Extend the relocation test in `test_compiler.cpp` to seed every reachable opcode case (conditional `call`/`jp` `nc`/`nz`/`c`/`z`, unconditional `call`/`jp`, and `0xFF` LOAD) via `Compiler::write`, and verify `make -C tests/unit run` passes (kills the reachable portion of the ~85 arithmetic survivors in `compiler.cpp`; the never-emitted `jp m/pe/p/po` and `call p/po/m/pe` variants are recorded for task 10.1)

## 10. Phase 4 — fallback + final verification

- [x] 10.1 If provably equivalent or never-emitted mutants remain after Phase 3, add their specific regions to `mull.yml` `excludePaths` and record the justification; verify with `make -n mutation-run` that the config is unchanged otherwise
- [x] 10.2 Run the full unit suite and verify all previously passing tests still pass and the total test count increased
- [x] 10.3 (user-managed) Run `make mutation-run` and `make mutation-check` and verify the reported mutation score reaches ≥ 85.0%

---

# Round 3 (70% → 85% — revisão após o Round 2 não atingir a meta)

O Round 2 implementou as fases 7–9 (enumeração exaustiva), mas o run final gerenciado pelo usuário ainda reportou **70%** (`mutation_report.json` de set/07: 1729 killed + 5 timeout / 2445 = 711 sobreviventes; apenas ~16 dos 727 previstos foram mortos). Causa raiz identificada no novo report: os testes do Round 2 assertam propriedades de alto nível ("compilou", `written > 0`, `codeSize > 0x4000`, assinatura do trampolim) e **não assertam os bytes/valores exatos que cada mutante altera** — ex.: `dest[address - 10] = 0x18` mutado para `address + 10` escreve em outra posição e nenhum teste lê de volta `out[address-10]`. O Round 3 muda a estratégia de asserção de _"exercita o branch"_ para _"asserta o byte/valor emitido exato"_, reescrevendo os testes das fases 7–9 como white-box onde necessário.

Meta: matar **~350** dos 711 sobreviventes (2445 × 0.85 = 2078,25 → 2079 killed; hoje 1729). Cada tarefa verifica com `make -C tests/unit run` (rápido, incremental); nenhum `mutation-build`/`mutation-run`/`mutation-clean` é executado pelo implementador — o run mull completo é o passo final do usuário (tarefa 16.3).

## 11. Phase 5 — relocação byte-exact em `compiler.cpp` (~90 kills)

- [x] 11.1 Reescrever os testes de relocação MegaROM para assertar os **bytes emitidos exatos** de cada opcode alcançável (call condicional `nc`/`nz`/`c`/`z`, jp condicional `nc`/`nz`/`c`/`z`, call/jp incondicional, e o LOAD `0xFF`), seedando a lista de fixes via `Compiler::write` (white-box) e varrendo `out[]` pelas sequências de patch — same-segment `0x18 0x07` (`jr $+8`), cross-segment `F5 08 D9 F1` (push/ex af,af'/exx/pop), condicionais `0x38/0x30/0x28/0x20 0x0A` + `0xC3/0xCD`, e `0xFF` (`jr $+6` + nops) — assertando cada byte. Verificar `make -C tests/unit run` (mata os ~90 `sub_to_add`/`add_to_sub`/`div_to_mul`/`rem_to_div` em `dest[address±N]`, `segm_from/to`, `new_address`)
  - **Critério anti-fragilidade (obrigatório):** opcodes Z80 (categoria ISA: `0x18`, `0xC3`, `0xCD`, `0xF5`, `0x3E`, `0x21`, etc.) e aritmética de geometria MegaROM do compilador (`dest[address±N]`, `% 0x4000`, `>= 0xA000`) DEVEM ser assertados literalmente. Já os **endereços resolvidos do kernel** (`mr_call_target`/`mr_jump_target`/`mr_get_data_target` nos bytes `dest[address]`/`[address+1]`) DEVEM ser **derivados em runtime** de `bin_header_bin[DISP_MR_CALL*2]`/`[DISP_MR_JUMP*2]`/`[DISP_MR_GET_DATA*2]` (espelhando a expressão de `compiler.cpp:338-343`), NUNCA hardcodados — assim o teste não quebra quando o kernel assembly for remontado. Os bytes do **trampoline/ABI** (`F5 08 D9 F1`, `jr cc,$+11` = `0x38/0x30/0x28/0x20 0x0A`) são um contrato compilador↔kernel e devem ser agrupados e comentados como "kernel ABI" (se `MR_CALL`/`MR_JUMP` em `61_megarom.asm` mudar, compilador e teste mudam juntos).
- [x] 11.2 Assertar a aritmética de layout exata (`segm_from`, `segm_to`, `new_address % 0x4000`, `segm_total`, e o boundary `addr_within_segm >= 0xA000`) posicionando código exatamente nos limites de 0x4000/0xA000 e verificando o número de segmento/byte emitido. Verificar `make -C tests/unit run` (mata os ~20 relacionais/aritméticos do loop de layout e os guards `length >= 0x4000`)

## 12. Phase 6 — fixtures de boundary de readers com asserção de valor exato (~90 kills)

- [x] 12.1 `resource_spr_reader.cpp`: assertar os bytes de sprite decodificados exatos para hex `0-9`/`A-F`/`a-f`, os limites `spriteX<8`/`spriteY<8`/`spriteC<16`, e o guard `*spriteCount>63`. Verificar `make -C tests/unit run` (mata ~25)
- [x] 12.2 `resource_akm_reader.cpp`/`resource_akx_reader.cpp`: fixtures de tamanho exato e `±1` assertando as tabelas reconstruídas (aritmética de índice e `baseAddress`, `subsongIndexTable±N`, `data[i±N]`). Verificar `make -C tests/unit run` (mata os ~44 aritméticos não-linker; os ~43 do linker state machine permanecem equivalentes/out-of-scope — registrados, não testados)
- [x] 12.3 `resource_mtf_map_reader.cpp`/`resource_csv_reader.cpp`/blob/data readers: fixtures de boundary exato. Verificar `make -C tests/unit run` (mata ~20)

## 13. Phase 7 — type-dispatch com asserção nos dois lados (~105 kills)

- [x] 13.1 `compiler_expression_evaluator.cpp`: dirigir todo par tipo/subtipo alcançável e assertar o opcode/resultado emitido **nos dois lados** de cada `==`. Verificar `make -C tests/unit run` (mata ~31, sendo 18 `eq_to_ne`)
- [x] 13.2 Estratégias graphics copy/line/put/circle/get/pset: `eq_to_ne` nos dois lados. Verificar `make -C tests/unit run` (mata ~44)
- [x] 13.3 `compiler_symbol_resolver.cpp` + `compiler_variable_emitter.cpp` + `compiler_float_converter.cpp` (`getUsingFormat` com os format bits exatos de `#`, `0`, `+`, `*`, `$`, `,`, `.`, `^`). Verificar `make -C tests/unit run` (mata ~30)

## 14. Phase 8 — sub-comandos both-sides das estratégias graphics (~70 kills)

- [x] 14.1 `compiler_set`/`color`/`screen`/`copy` (e `on`/`let`/`if`): assertar contagens de bytes/entidades exatas e os dois ramos de `t>=2`/`t>=3`/`i>1`/`t<=3`/`i<sub_sub_action->actions.size()`. Verificar `make -C tests/unit run` (mata ~70 combinados, incluindo os 29 de `compiler_set`, 15 de `color`, 14 de `screen`, 13 de `copy`)

## 15. Phase 9 — optimizer e long tail (~65 kills)

- [x] 15.1 `compiler_code_optimizer.cpp`: completar todos os padrões peephole com asserção dos bytes emitidos exatos (pop/ld/push/ld/pop, `add hl,de` imediato 1/2/3, e os guards de `code_pipeline`). Verificar `make -C tests/unit run` (mata ~25)
- [x] 15.2 Long tail (parser/lexer/domain/symbols/cli com ≤5 cada; e `rom.cpp` 17, `graphics_statement_strategy` 19): uma asserção de valor exato por local inc/relacional sobrevivente. Verificar `make -C tests/unit run` (mata ~40; completa a tarefa 7.5 não concluída)
  - **Critério anti-fragilidade (obrigatório) para `rom.cpp`:** os sobreviventes de `rom.cpp` (`kernelAddr = bin_header_bin[tableAddr] | ...`, `offset = kernelAddr - 0x4000`, posições de patch) devem ter o valor esperado **derivado em runtime** de `bin_header_bin[]` + `def_wrapper_routines_map_table`/`DISP_*` (espelhando `rom.cpp:245-249`), NUNCA hardcodado — assim o teste não quebra quando o kernel assembly for remontado. Os offsets/posições relativos (`- 0x4000`, `+ 2`, limites `0..0x3FFF-2`) podem ser assertados literalmente.

## 16. Phase 10 — fallback + verificação final

- [x] 16.1 Reavaliar os sobreviventes remanescentes e registrar os equivalentes/unreachable aceitos. Registro: (a) opcodes de relocação nunca emitidos `jp m/pe/p/po` (0xFA/0xEA/0xF2/0xE2) e `call p/po/m/pe` (0xF4/0xE4/0xFC/0xEC) — não testados por decisão; (b) os condicionais `call nc/nz/c/z`/`jp nc/c` também nunca são emitidos, mas agora são cobertos via white-box `getContext()` (Decision 12); (c) branch morto `*pipeline[0] == 11` em `compiler_code_optimizer.cpp:123` (deveria ser `0x11`); (d) linker state machine do AKM (~43). `mull.yml` permanece inalterado — a Decision 8 já estabeleceu que `excludePaths` é whole-file e excluiria também código alcançável
- [x] 16.2 Rodar a suíte unitária completa e confirmar que todos os testes anteriores passam e a contagem total aumentou
- [ ] 16.3 (user-managed) Rodar `make mutation-run` e `make mutation-check` e confirmar ≥ 85.0%
