## Context

O projeto MSXBAS2ROM compila programas MSX-BASIC para ROMs executáveis. A suíte de testes unitários utiliza o framework **doctest** e compila todo o código fonte (exceto `main.cpp`) com `--coverage -O0`. O relatório de cobertura atual (`tests/unit/coverage-summary.json`) mostra 66.9% de cobertura de linha. O Makefile dos testes já define `--fail-under-line 85` como critério de aprovação.

O pipeline de teste segue o padrão: criar arquivo `.bas` temporário → carregar no Lexer → avaliar no Parser → compilar com Compiler → verificar opcodes gerados ou resultados.

**Restrições:**
- Nenhum código de produção pode ser alterado (test-only change)
- Os testes devem seguir as convenções de nomenclatura e estilo já usadas nos arquivos existentes
- O framework doctest já está configurado e não requer mudanças

## Goals / Non-Goals

**Goals:**
- Elevar line coverage de 66.9% para ≥85% (~2649 novas linhas cobertas)
- Priorizar arquivos com maior volume de linhas não cobertas (maior impacto por teste)
- Garantir que `make coverage` no `tests/unit/` passe sem erros
- Manter todos os testes existentes passando

**Non-Goals:**
- Branch coverage (38.7%) — foco exclusivo em line coverage
- Function coverage (80.9%) — não é o alvo, mas será elevada indiretamente
- Testes de integração (`tests/integration/`)
- Testes de kernel (`tests/kernel/`)
- Refatoração de código de produção para testabilidade
- Cobertura de `main.cpp` (excluído da compilação de testes)

## Decisions

### 1. Ordem de ataque por impacto

**Decisão**: Priorizar os arquivos com maior número absoluto de linhas não cobertas, divididos em fases.

**Alternativa considerada**: Priorizar arquivos por % mais baixa.
**Razão**: Arquivos com 0% de cobertura mas 3 linhas totais contribuem pouco. O objetivo é 2649 linhas novas — precisamos de volume.

**Fases definidas**:

```
Fase 1 (Fundação): compiler.cpp, compiler_statement helpers     ~+120 linhas
Fase 2 (SET/ON):    compiler_set_statement, compiler_on           ~+350 linhas
Fase 3 (Gráficos):  COPY, PUT, SCREEN, CIRCLE, PAINT              ~+250 linhas
Fase 4 (Expressão): expression_evaluator, variable_emitter        ~+200 linhas
Fase 5 (Parser):    graphics, put, on, set, screen parsers        ~+200 linhas
Fase 6 (Recursos):  AKM reader, MTF map reader                    ~+120 linhas
Fase 7 (Zerados):   time_statement, open_grp, vscode_helper       ~+50 linhas
Fase 8 (Polimento): z80, rom, code_optimizer, code_helper         ~+150 linhas
```

### 2. Estrutura dos novos arquivos de teste

**Decisão**: Criar `test_compiler_graphics.cpp` e `test_compiler_statements.cpp` para statements gráficos e de controle com baixa cobertura, separando a responsabilidade do `test_compiler.cpp` existente (1474 linhas).

**Razão**: O `test_compiler.cpp` atual é grande e cobre principalmente statements básicos (PRINT, INPUT, LOCATE, etc.). Separar statements gráficos em arquivo próprio evita um arquivo monolítico de 4000+ linhas.

### 3. Abordagem de teste para statements

**Decisão**: Para cada statement, testar pelo menos: (a) caminho feliz com parâmetros mínimos, (b) caminho feliz com parâmetros completos, (c) cenário de erro quando suportado.

**Razão**: O código tem muitas branches para validação de parâmetros e formatos alternativos (ex: COPY com/sem STEP, PUT com/sem SPRITE). Exercitar ambos os caminhos cobre mais linhas.

### 4. Cobertura de resource readers

**Decisão**: Para AKM reader (2.5%), criar arquivos AKM binários mínimos válidos e alimentar o reader. Para MTF map reader (22.6%), usar arquivos MTF de teste existentes ou criar fixtures mínimas.

**Razão**: Esses readers dependem de formatos binários específicos. Criar fixtures mínimas (headers válidos + dados mínimos) é o caminho padrão da suíte.

## Risks / Trade-offs

- **[Risco] Testes frágeis com fixtures binárias**: AKM/AKX são formatos proprietários. Se os readers tiverem validações restritivas, criar fixtures válidas pode ser complexo.
  → **Mitigação**: Se um reader for muito difícil de testar com fixture, priorizar outros arquivos da mesma fase.

- **[Risco] Statements com muitas branches**: SET tem 704 linhas e 15 funções. Pode ser difícil cobrir todas as branches sem testes excessivos.
  → **Mitigação**: Focar em line coverage, não branch coverage. Cobrir os caminhos principais de cada sub-comando é suficiente.

- **[Trade-off] Tempo de compilação**: Adicionar mais arquivos de teste aumentará o tempo de build.
  → Aceitável. O build de testes atual já compila todo o projeto.

- **[Trade-off] vscode_helper.cpp (0%, 42 linhas)**: Gera configuração VS Code e depende de ambiente de desenvolvimento.
  → **Mitigação**: Teste básico de smoke (instanciar e verificar que não crasha) ou pular. O ganho é pequeno (42 linhas).
