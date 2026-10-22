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

---

# Round 4 (73% → 85% — revisão após o Round 3)

O run do Round 3 reportou **73%** (`mutation_report.json` de set/08: **1791 killed + 10 timeout / 2445 = 644 sobreviventes**; `mutationScore` 73). A meta de 85% exige **2079 killed** (2445 × 0.85 = 2078,25), ou seja **~288 kills adicionais**. Diagnóstico do novo report:

- Sobreviventes por mutador: **174 `eq_to_ne`**, **99 `post_inc`**, **185 relacionais** (`lt`/`le`/`gt`/`ge`), **117 aritméticos** (`add`/`sub`/`mul`/`div`/`rem`), 11 `ne_to_eq`, 4 `pre_inc`, 2 `div`.
- Concentração por grupo: `compiler.cpp` relocação/layout 46; readers do builder 135 (dos quais `resource_akm_reader` 78, `resource_spr_reader` 19, `resource_akx_reader` 12); semantic do compiler 98; statements do compiler 201; parser 99; symbols 20; lexer 17; cli/domain/infra 7.
- Causa raiz remanescente: os fixtures do Round 3 usam **zeros/valores iguais**, então mutantes de **índice** (`dest[address - N]` → `address + N`, `data[i + 1]` → `i - 1`) leem/escrevem uma posição cujo valor coincide com o esperado — o mutante é equivalente *para aquele fixture*. Ex.: em `compiler.cpp` o mutante de `dest[address - 3]` (lado direito) lê `dest[address + 3]`, ambos 0x00 no seed. Ex. análogo em readers e `compiler_variable_emitter`.
- A correção é usar **sentinelas distintas por offset** (cada byte semeado com valor único, ex. `0xA0 + (i & 0x3F)`), de modo que qualquer leitura/escrita no offset errado produza um byte observavelmente diferente.

Meta histórica (superseded): matar **~300** sobreviventes do relatório de 73%. A verificação do Round 4 era `make -C tests/unit run`; nenhum `mutation-build`/`mutation-run`/`mutation-clean` era executado pelo implementador. O Round 6 abaixo substitui essa meta.

**Critério anti-fragilidade (vale para TODAS as fases do Round 4):** os testes não podem quebrar com uma simples modificação do kernel (`src/infrastructure/kernel/asm/**` → `bin_header_bin`). Portanto:
- **Valores derivados do kernel** (`bin_header_bin[DISP_*]`, `def_wrapper_routines_map_table`, `getKernelCallAddr(...)`, `mr_call_target`/`mr_jump_target`/`mr_get_data_target`, `kernelAddr`/`offset` de `rom.cpp`) DEVEM ser calculados em runtime a partir de `bin_header_bin[]` + `DISP_*`/`def_*`, espelhando a expressão de produção — NUNCA hardcodados.
- **Opcodes Z80** (`0x18`, `0xC3`, `0xCD`, `0xF5`, `0x3E`, `0x21`, …) e **aritmética de geometria MegaROM do compilador** (`0x4000`, `0x8000`, `0xA000`, `% 0x4000`, `dest[address±N]`) são assertados literalmente (ISA/constantes do compilador).
- **Contratos ABI compilador↔kernel** (`F5 08 D9 F1` = push/ex af,af'/exx/pop; `jr cc,$+11` = `0x38/0x30/0x28/0x20 0x0A`) são assertados literalmente, agrupados e comentados como "kernel ABI" (se `MR_CALL`/`MR_JUMP` mudar, compilador e teste mudam juntos).
- Fixtures de readers/recursos são bytes sintéticos (não kernel), com sentinela distinta por posição.

## Historical note (superseded)

Rounds 4 and 5 (the former phases 11-17 and tasks 24-29) are superseded by
Round 6 below. Their analysis remains in `design.md`; the unchecked historical
tasks SHALL NOT be executed. Round 6 starts only after CleanArch Phase A and a
fresh Mull baseline.

---

# Round 6 (after CleanArch Phase A; fresh baseline 70%; target >85%)

Round 6 supersedes Round 5. It starts only after
`refactor-clean-architecture-testability` Phase A is complete, its boundary
check is green, and a fresh `mutation_report.json` has been generated. The
fresh post-Phase-A report is **70%**: 1,783 killed, 733 survived, and 12
timeout out of 2,528; 85% requires **2,149 killed**, i.e. **366 additional
kills**. The planned budget is **~415 kills**, distributed by the current
survivor concentrations below. Historical pre-refactor figures (615 survivors,
271 kills, 345 provisional budget) are context only and SHALL NOT be reused.
No task runs Mull, `mutation-build`, or `mutation-clean`; verification is
`make -C tests/unit run`, and the fresh and final Mull runs are user-managed.

## 24. Prerequisite, fresh baseline, and fast test infrastructure

- [x] 24.1 Confirm CleanArch Phase A is complete: boundary check green, source/kernel/resource/symbol boundaries available, normal unit suite passing, and the Phase A state recorded (commit when committed, otherwise the current working-tree diff). Verified `make test-architecture` green (19 tests, no temporary `.bas` outside adapter tests) and `make -C tests/unit run` passing (582 tests / 3,997 assertions).
- [x] 24.2 (user-managed) Run `make mutation-run` once and regenerate the survivor inventory by module and mutator; recompute the total/killed/survived/timeout and the kills required for 85%. Verified against the fresh report: 2,528 total = 1,783 Killed + 733 Survived + 12 Timeout, score 70; 2,149 killed required, 366 additional kills. Do not reuse old line targets.
- [x] 24.3 Create or extend `tests/unit/src/test_support.h` with `compileSource(logicalName, source)` and `kernelImage()` seams backed by the Phase A boundaries. Add a regression check that a compilation completes without creating a `.bas` file. Verified: both seams exist and `make test-architecture` rejects temporary `.bas` files outside explicit adapter tests.
- [ ] 24.4 Profile the suite from `tests/unit` with `./bin/test_unit --duration=true`, record the top 10, and measure the mutation baseline excluding `Slow`. Post-Phase-A measurement: 582 tests / 3,997 assertions; top 10 = blob 8K 9.446 s (`Slow`), all statements 2.047 s, all functions 1.478 s, decimal/string 1.417 s, sprite cap 1.017 s, all CMD 0.853 s, MegaROM conditional 0.815 s, MegaROM GOSUB 0.754 s, MegaROM PRINT 0.705 s, ASCII8 retry 0.573 s; baseline excluding `Slow` = 24.95 s, below the 32 s cap. Tag a broad aggregate `Slow` only after its focused replacement tests exist; verify it still runs in the normal suite and is excluded only from mutation.

## 25. P1 — graphics statement strategies (target ~75 kills; 129 survivors)

- [ ] 25.1 SET (39 survivors, the largest single file): add a data-driven matrix for every sub-command at 0..4 arguments, `ARRAY` forms with one and two entries, null parameters, and the `i>1`/`t>=2`/`t>=3`/`t<=3` branches; assert exact emitted bytes (e.g. `pop bc`/`pop hl` versus `ld b,0xff`/`ld hl,0xffff`). Verify `make -C tests/unit run`.
- [ ] 25.2 COLOR (16) and SCREEN (14): cover 0..4 arguments, RGB tuples, `NEW`/`RESTORE`, sprite forms, and the `i<1`/`i<3`/`t<3`/`t<4`/null-subtype branches; assert exact byte counts and selected opcodes. Verify `make -C tests/unit run`.
- [ ] 25.3 COPY (13), LINE (12), GET (8), PUT (8), CIRCLE (4), PAINT (4), PSET (5), SPRITE (4), and KEY (2): cover omitted coordinates, literal versus identifier, operator codes, and argument-count boundaries; assert exact bytes/entities, not just success. Verify `make -C tests/unit run`.
- [ ] 25.4 Re-run the graphics strategy tests with distinct sentinels for any expression whose value is read back, so index/offset mutants change an asserted byte. Verify `make -C tests/unit run`.

## 26. P2 — control, I/O, basic, and sound statements (target ~60 kills; 100 survivors)

- [ ] 26.1 ON (16): assert the exact `def_MR_TRAP_SEGMS + N + i` loads, single/multi-digit literal versus dummy, `sub_lexeme->value.size() > 1`, and the numeric/double subtype branches; cover all reachable trap families. Verify `make -C tests/unit run`.
- [ ] 26.2 IF (5)/FOR (5) and basic statements: cover `isLastActionGoto`, subtype branches, DIM (3)/LET (5) and assignment forms with exact emitted bytes. Verify `make -C tests/unit run`.
- [ ] 26.3 I/O and CMD handlers (53): cover INPUT (7)/PRINT (7)/WAIT (3)/SWAP (2)/POKE/OUT (3)/BLOAD and the MTF handler thresholds `n>=2,3,4,5,6,7,8,9` (8); assert exact byte counts and opcodes. Verify `make -C tests/unit run`.
- [ ] 26.4 SOUND (3)/PLAY (4) and remaining small statement strategies: assert exact argument-count and subtype branches with output assertions. Verify `make -C tests/unit run`.

## 27. P3 — compiler semantic helpers and optimizer (target ~60 kills; 98 survivors)

- [ ] 27.1 `compiler_expression_evaluator` (39, 26 of them `eq_to_ne`): drive a matrix of scalar/array, numeric/single/double/string, casts in both directions, comparisons, `s[0] == 0x11` with `i = 0..5` and `i = 256/257`, and result-array initialization; assert exact opcodes and result subtype. Verify `make -C tests/unit run`.
- [ ] 27.2 `compiler_code_optimizer` (16): cover every peephole pattern and immediate 1/2/3, `getKernelCallAddr` with `DISP_*` and a raw address, and the dispatch/range branches; record the dead `== 11` branch as equivalent. Verify `make -C tests/unit run`.
- [ ] 27.3 `compiler_variable_emitter` (18, 11 `eq_to_ne`): cover `x_factor` 2/3/256/default, the `y_factor` power-of-two loop, and 1D/2D literal versus expression indices with exact emitted sequences. Verify `make -C tests/unit run`.
- [ ] 27.4 `compiler_symbol_resolver` (16, 8 `eq_to_ne`) and related helpers: cover every subtype, quote handling, `ram_page + ram_pointer`, and `ram_size > def_RAM_SIZE` with exact values. Verify `make -C tests/unit run`.

## 28. P4 — compiler core relocation and layout (target ~30 kills; 46 survivors)

- [ ] 28.1 Extend the white-box `Compiler::write` harness with a distinct sentinel per offset; cover every relocation-switch opcode, including never-emitted `call p/po/m/pe` and `jp m/pe/p/po`. The current survivors are concentrated in the switch at `compiler.cpp:531-705` and the fixup arithmetic at `compiler.cpp:562-590`/`631-705`. Assert every preamble/payload byte and derive `mr_call_target`/`mr_jump_target`/`mr_get_data_target` from `bin_header_bin[DISP_MR_* * 2]` at runtime. Verify `make -C tests/unit run`.
- [ ] 28.2 Cover the build/layout survivors at `compiler.cpp:169/188/201/210/248/287/303/329-332/397-457` with boundaries at `0x3FFF/0x4000` and `0x9FFF/0xA000/0xA001`, asserting `segm`, `addr_within_segm`, `segm_total`, `code_size`, and the skip trampoline. Verify `make -C tests/unit run`.
- [ ] 28.3 Keep all context/patch manipulation in this single harness so CleanArch Phase B can adapt it without touching other tests. Verify `make -C tests/unit run`.

## 29. P5 — builder resource readers (target ~85 kills; 145 survivors)

- [ ] 29.1 AKM non-linker (65): build sentinel-diverse synthetic files for the header (`62-112`), track/subsong tables (`178-240`), and instrument table (`278-300`), including every `guessBaseAddress` iteration; assert complete rewritten bytes and every `==`/`±1` boundary. Verify `make -C tests/unit run`.
- [ ] 29.2 AKX (13): cover `guessBaseAddress` iterations, first/last effect, and `length-1` boundaries; assert exact rewritten effect addresses. Verify `make -C tests/unit run`.
- [ ] 29.3 SPR (19): cover hex digit ranges, 8x8 quadrants, `spriteC<16`, MSX1/MSX2 paths, and the 64-sprite cap; assert exact decoded bytes. Verify `make -C tests/unit run`.
- [ ] 29.4 MTF (6)/CSV (6)/blob (6)/data (3) readers: cover exact/one-under/one-over sizes, truncation, and sentinel bytes; assert complete tables and distinct error messages. Verify `make -C tests/unit run`.
- [ ] 29.5 Reassess the AKM linker state machine (`123-168`, **26** current survivors, not the former ~43) and document each residual as reachable, equivalent, timeout, or deliberately out of scope. Do not change `mull.yml` merely to remove it.

## 30. P6 — parser (target ~55 kills; 98 survivors)

- [ ] 30.1 Graphics parser (21, 17 `post_inc`): cover `parmCount`/`sepCount` with valid and invalid parameter lists, nested parentheses, missing separators, and excess parameters; assert success/error and action-tree counts. Verify `make -C tests/unit run`.
- [ ] 30.2 FILE (13): cover file numbers 0, 1, 15, 16, `AS#`/`AS`, and identifier parameters; assert exact success/error. Verify `make -C tests/unit run`.
- [ ] 30.3 DATA (10)/DEF (9): cover `h/H/o/O/b/B` prefixes and DEF ranges including out-of-range letters; assert parsed values/errors. Verify `make -C tests/unit run`.
- [ ] 30.4 PRINT (8)/USING and PUT (7): cover the `0..3` USING loops, separator counts, and parameter boundaries; assert exact parse results. Verify `make -C tests/unit run`.
- [ ] 30.5 Remaining parser long tail (SET 4/IF 3/color 2/generic 2/next 2/dim 1/on 2 and parser core/helpers 14): one exact-value or error assertion per surviving location. Verify `make -C tests/unit run`.

## 31. P7 — builder core, symbols, lexer, functions, and long tail (target ~50 kills; 117 survivors)

- [ ] 31.1 `rom.cpp` (18) and `resource_manager` (10): cover mapper patch offsets, kernel-derived addresses via the `kernelImage()` seam, `mapSize > 0x4000`, block counts, segment > 255, `packedRate` arithmetic, and layout limits with exact bytes. Verify `make -C tests/unit run`.
- [ ] 31.2 Symbol exports (20: CDB 8, OMDS 4, NoICE 3, Symbol 3, ELF 2): assert exact strings, prefix comparisons, sizes 0/1, and ELF arithmetic. Verify `make -C tests/unit run`.
- [ ] 31.3 Lexer (18): cover character boundaries for identifiers/hex, `<= ' '`, `index + 1 < length`, line counting, comments, operators, and separators. Verify `make -C tests/unit run`.
- [ ] 31.4 Compiler functions (12) and domain/infrastructure long tail (6): one exact-value assertion per surviving location (USING 4/MID/PSG/LOC/VDP/PDL/BASE/TURBO/MAKER, action/tag nodes, file_source_resolver/fswrapper where reachable). Verify `make -C tests/unit run`.
- [ ] 31.5 `compilation_use_case` (15, mostly `eq_to_ne`): drive success, source-resolution failure, plain-ROM overflow/ASCII8 retry, and symbol-output branches through `CompilationRequest` with in-memory adapters; assert `exitCode`, `retriedWithAscii8`, artifact presence, and diagnostic text exactly. Verify `make -C tests/unit run`.
- [ ] 31.6 Kernel-image adapters (12: `kernel_image` 5, `generated_kernel_image` 5, `in_memory_kernel_image` 2): assert `resolveKernelCallAddress` null/size/dispatch/JP-follow branches and dispatch bounds with runtime-derived bytes, using `InMemoryKernelImage`. Verify `make -C tests/unit run`.

## 32. Z80 regression safety and slow-test isolation (non-scoring)

- [ ] 32.1 Add `tests/unit/src/test_generated_z80.cpp`: compile 6-8 representative programs, normalise/mask only kernel-derived address operands, and assert FNV-1a fingerprint + fragment size + selected Z80 opcodes/ABI bytes. Verify an unchanged generation preserves the fingerprint and an opcode/layout change does not; never hash a full ROM.
- [ ] 32.2 Add `tests/unit/src/test_kernel_layout.cpp`: validate every `DISP_*` target is in the runtime-derived kernel range and assert distances only for the declared-stable MR anchor set (`DISP_MR_CALL` through `DISP_MR_JUMP`); do not add global distance snapshots. Verify `make -C tests/unit run`.
- [ ] 32.3 Tag the identified broad aggregates (`All statement strategies`, `All function strategies`, `All CMD instruction handlers`, and other top-10 entries from task 24.4) `Slow` only after their focused replacements pass; verify the normal suite still runs them and the mutation command excludes only the tagged suite. Keep the post-Phase-A baseline (24.95 s excluding `Slow`) below the 32 s cap. Do not run Mull.

## 33. Round 6 verification and handoff

- [ ] 33.1 Run `make -C tests/unit run` from `tests/unit` and verify the complete normal suite passes (current baseline 582 tests / 3,997 assertions), the test count increased, and no existing assertion was weakened. Do not run Mull or mutation targets.
- [ ] 33.2 Review residual survivors against the fresh 733-survivor inventory and document demonstrated equivalents, unreachable paths, and timeouts without whole-file exclusions. Expected residuals: AKM linker state machine (26), the dead optimizer `== 11` branch, and the 12 timeout mutants. Verify `mull.yml` remains unchanged.
- [ ] 33.3 Confirm `make test-architecture` is green and that this change introduced no production source edits.
- [ ] 33.4 (user-managed) Run `make mutation-run` and `make mutation-check`; confirm at least **2,149 killed** (366 more than the 1,783 baseline) and a Mutation Score >= 85. If short, add long-tail tests rather than excluding whole files.
- [ ] 33.5 Record the final result and the Phase B handoff: after CleanArch Phase B, regenerate the baseline and reconcile the score with the refactored source.

## Workflow follow-up

- After Phase B and the final reconciliation, archive this change when the project's review requirements are satisfied.
