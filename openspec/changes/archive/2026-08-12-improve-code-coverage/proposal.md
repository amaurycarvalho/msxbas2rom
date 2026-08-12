## Why

A linha de cobertura atual do projeto está em 66.9% (9771/14612 linhas), bem abaixo da meta de 85% definida no próprio Makefile de testes (`--fail-under-line 85`). Ampla área do código — especialmente estratégias de statements gráficos (SET, ON, COPY, PUT, CIRCLE, SCREEN) e helpers de expressão — não possui cobertura de testes, aumentando o risco de regressões durante as mudanças ativas (`implement-double-via-float-float`, `set-page-screen4`). Subir a cobertura para ≥85% garante uma base de regressão sólida antes de novas features.

## What Changes

- Adicionar casos de teste unitários para statements do compilador com menor cobertura (SET, ON, COPY, PUT, SCREEN, CIRCLE, LET, PAINT, PSET, COLOR, KEY)
- Expandir cobertura de helpers semânticos do compilador (expression evaluator, variable emitter, code helper, float converter)
- Adicionar testes para leitores de recursos com cobertura zerada ou quase zerada (AKM reader, MTF map reader)
- Cobrir caminhos não testados em `compiler.cpp` e `rom.cpp`
- Adicionar testes para o parser de statements gráficos (graphics, put, on, set, screen)
- Testar estratégias de função com baixa cobertura (MID$, INSTR$, STRING$, USING, USR)
- Smoke tests para arquivos com 0% de cobertura (compiler_time_statement, compiler_open_grp_statement, vscode_helper)
- Expandir cobertura do kernel Z80 (`z80.cpp`) em caminhos não exercitados

## Capabilities

### New Capabilities

- `unit-test-coverage`: Conjunto de testes unitários que elevam a cobertura de linha do projeto para ≥85%, abrangendo statements do compilador, helpers semânticos, leitores de recursos, parser gráfico e estratégias de função.

### Modified Capabilities

<!-- Nenhuma mudança de requisitos em specs existentes. Este change é puramente de testes. -->

## Impact

- Apenas arquivos em `tests/unit/src/` serão criados ou modificados (novos arquivos de teste)
- Nenhum código de produção alterado
- O Makefile de testes (`tests/unit/Makefile`) já define `--fail-under-line 85` no target `coverage` — a mudança apenas fará os testes passarem essa validação
- Testes usam o framework doctest já estabelecido e seguem o padrão de criar arquivos `.bas` temporários e executar o pipeline lexer→parser→compiler
