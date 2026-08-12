## 1. Fase 1 — Fundação: compiler.cpp e contexto

- [x] 1.1 Adicionar testes para cenários de erro e opções alternativas de build em `compiler.cpp` (176 linhas não cobertas)
- [x] 1.2 Testar `compiler_let_statement_strategy.cpp` com atribuições de diferentes tipos (69 linhas não cobertas)

## 2. Fase 2 — Statements SET e ON

- [x] 2.1 Criar `tests/unit/src/test_compiler_set.cpp` para `compiler_set_statement_strategy.cpp`
- [x] 2.2 Testar sub-comandos SET: BGM, ADJUST, BEEP, DISPLAY, PAGE, TITLE, VIDEO
- [x] 2.3 Testar sub-comandos SET: CLIP, PASSWORD, INTERVAL, SCREEN, SEARCH, SPRITE
- [x] 2.4 Testar caminhos de erro e validação para SET
- [x] 2.5 Criar `tests/unit/src/test_compiler_on.cpp` para `compiler_on_statement_strategy.cpp`
- [x] 2.6 Testar ON ERROR GOTO, ON STOP GOTO, ON SPRITE GOSUB, ON INTERVAL, ON KEY
- [x] 2.7 Testar caminhos de erro para ON

## 3. Fase 3 — Statements Gráficos

- [x] 3.1 Criar `tests/unit/src/test_compiler_graphics.cpp`
- [x] 3.2 Testar COPY statement com variações source/dest (com e sem STEP)
- [x] 3.3 Testar PUT statement: modo KANJI, SPRITE, e variações de atributos
- [x] 3.4 Testar SCREEN statement: modos 0-4 e parâmetros de página
- [x] 3.5 Testar CIRCLE statement com diferentes estilos (filled, outline) e coordenadas
- [x] 3.6 Testar PAINT statement com borda e cor de preenchimento
- [x] 3.7 Testar PSET statement com diferentes modos de ponto
- [x] 3.8 Testar COLOR statement com foreground/background/border
- [x] 3.9 Testar KEY statement com ON/OFF e keylist
- [x] 3.10 Testar DRAW statement com string de comandos gráficos

## 4. Fase 4 — Expression Evaluator e Variable Emitter

- [x] 4.1 Adicionar testes em `test_compiler.cpp` para expressões com tipos mistos (int+float, int+string)
- [x] 4.2 Testar `compiler_expression_evaluator.cpp`: operadores unários, precedência, parênteses aninhados
- [x] 4.3 Testar `compiler_expression_evaluator.cpp`: chamadas de função dentro de expressões
- [x] 4.4 Testar `compiler_variable_emitter.cpp`: arrays, DEFUSR, verificação de tipos
- [x] 4.5 Testar `compiler_code_helper.cpp` com cenários de salto e geração de código
- [x] 4.6 Testar `compiler_float_converter.cpp` com conversões de precisão

## 5. Fase 5 — Parser Statements

- [x] 5.1 Adicionar testes em `test_parser.cpp` para parser de statements gráficos
- [x] 5.2 Testar parser de PUT com sintaxe completa
- [x] 5.3 Testar parser de ON com variantes (ON ERROR, ON STOP, ON SPRITE, ON KEY)
- [x] 5.4 Testar parser de SET com todos os sub-comandos
- [x] 5.5 Testar parser de SCREEN com modos e switches

## 6. Fase 6 — Resource Readers

- [x] 6.1 Criar `tests/unit/src/test_resources_extra.cpp` para readers não cobertos
- [x] 6.2 Criar fixture AKM binária mínima e testar `resource_akm_reader.cpp`
- [x] 6.3 Criar fixture AKX binária mínima e testar `resource_akx_reader.cpp`
- [x] 6.4 Criar fixture MTF map e testar `resource_mtf_map_reader.cpp`
- [x] 6.5 Testar `resource_mtf_reader.cpp` (28.6% → alvo 70%)
- [x] 6.6 Testar `resource_csv_reader.cpp` com dados estruturados (60.7% → alvo 80%)

## 7. Fase 7 — Cobertura Zero e Funções

- [x] 7.1 Adicionar smoke test para `compiler_time_statement_strategy.cpp` (0%, 4 linhas)
- [x] 7.2 Adicionar smoke test para `compiler_open_grp_statement_strategy.cpp` (0%, 3 linhas)
- [x] 7.3 Testar `compiler_mid_function_strategy.cpp` com diferentes argumentos (40.7%)
- [x] 7.4 Testar `compiler_instr_function_strategy.cpp` com busca em strings (39.5%)
- [x] 7.5 Testar `compiler_string_function_strategy.cpp` com strings repetidas (52.6%)
- [x] 7.6 Testar `compiler_usr_function_strategy.cpp` com múltiplos segmentos USR (50%)

## 8. Fase 8 — Polimento e Verificação

- [x] 8.1 Expandir cobertura de `z80.cpp` em instruções não exercitadas (73.3% → alvo 80%)
- [x] 8.2 Testar `rom.cpp` com cenários adicionais de layout de ROM (75.5% → alvo 85%)
- [x] 8.3 Testar `compiler_code_optimizer.cpp` com padrões de otimização adicionais (71.3% → alvo 80%)
- [x] 8.4 Verificar `compiler_return_statement_strategy.cpp` com RETURN em subrotinas (27.3%)
- [x] 8.5 Verificar `compiler_stop_statement_strategy.cpp` com diferentes eventos (53.3%)
- [x] 8.6 Rodar `make run` em `tests/unit/` — garantir que todos os testes passam
- [x] 8.7 Rodar `make coverage` em `tests/unit/` — verificar line coverage ≥ 85%
- [x] 8.8 Se cobertura ainda < 85%, identificar gaps remanescentes e adicionar testes pontuais
