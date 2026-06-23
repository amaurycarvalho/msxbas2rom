## Context

`90_support.asm` é o módulo de rotinas de suporte do header Z80 compilado com Pasmo. O arquivo exporta ~425 labels com endereços fixos em `header.symbols.asm`, que são referenciados por código C++ (via macros `def_*` em `compiler_hooks.h`) e outros arquivos `.asm`. Após análise de dependências, 58 labels foram identificados como completamente inacessíveis (nenhum entry point externo os alcança direta ou transitivamente).

O desafio técnico é que desativar código em assembly Z80 desloca endereços, quebrando todas as referências downstream. A solução é usar `DEFS` (define space) do Pasmo para preencher os bytes desativados, mantendo offsets idênticos.

## Goals / Non-Goals

**Goals:**
- Desativar 58 labels de código morto em `90_support.asm`
- Manter endereços absolutos de todos os labels subsequentes usando `DEFS` filler
- Preservar compatibilidade binária: nenhuma mudança em `header.symbols.asm`, código C++, ou outros `.asm`
- Compilação do header Z80 (`pasmo header.asm header.bin`) produz binário idêntico em tamanho e funcionalidade

**Non-Goals:**
- Não remover código (apenas comentar/desativar com filler)
- Não alterar `BASIC_KUN_START_FILLER`, `ARKOS_PLAYER`, `I7EA8 (CHR$)`, `I7EC3 (HEX$)`, `I7EC6 (OCT$)`, `I7EC9 (BIN$)`, `I7EEF (SPACE$)`, `BASIC_KUN_END_FILLER`
- Não modificar `header.symbols.asm` — todos os símbolos (incluindo os desativados) permanecem exportados

## Decisions

### Decisão 1: `DEFS <tamanho>, 0x00` vs `IF 0/ENDIF`

**Escolha**: Usar `DEFS <tamanho>, 0x00` com comentário documentando o que foi desativado.

**Alternativa considerada**: Blocos `IF 0`/`ENDIF` (condicional assembly). Rejeitado porque:
- O Pasmo pode ou não suportar condicionais (depende da versão). `DEFS` é primitivo garantido.
- `DEFS` torna explícito o tamanho exato do filler, facilitando verificação visual de que os offsets são preservados.

### Decisão 2: Granularidade — por label vs por bloco contíguo

**Escolha**: Agrupar labels contíguos em um único bloco `DEFS` quando eles formam uma unidade lógica (ex: toda a jump table J4DFA-I4E28, todo o bloco de INPUT I7517-C75BE).

**Justificativa**: Reduz o número de blocos `DEFS` de 58 para ~9, tornando o código mais legível. Como o filler é `0x00`, o resultado binário é idêntico ao que seria com blocos individuais.

### Decisão 3: Preservar ou remover labels do `header.symbols.asm`

**Escolha**: Preservar todos os labels em `header.symbols.asm`, incluindo os desativados.

**Justificativa**:
- Se código C++ futuro tentar referenciar um label desativado, o linker reportará erro com símbolo definido mas código ausente — mais seguro que símbolo indefinido.
- Evita mudanças em `header.symbols.asm`, que é compartilhado pelo build system.
- Símbolos desativados com `DEFS 0x00` produzem NOPs inofensivos se acidentalmente executados (melhor que comportamento indefinido).

### Decisão 4: Ordem de desativação

**Escolha**: Processar do endereço mais baixo ao mais alto, calculando `tamanho = próximo_label_útil - label_atual`.

**Justificativa**: Evita ter que recalcular offsets manualmente. Cada bloco `DEFS` usa a diferença entre o endereço do próximo label ativo e o início do bloco desativado.

## Riscos / Trade-offs

- **Risco**: Cálculo incorreto do tamanho do `DEFS` desloca labels subsequentes.
  - **Mitigação**: Verificar `pasmo --msx header.asm header.bin` e comparar `xxd header.bin` com o binário original. Ambos devem ser idênticos.

- **Risco**: Código em runtime (não compilado) que usa `USR` ou `CALL` para endereços hardcoded na faixa desativada.
  - **Mitigação**: Baixo — o código desativado nunca é alcançado via entry points documentados. Se existir código BASIC com `DEFUSR=&HXXXX`, ele já estaria quebrado antes (rotinas sem funcionalidade útil). Além disso, os endereços continuam existindo como `DEFS` (preenchidos com NOPs).

- **Trade-off**: Código comentado ocupa espaço visual no arquivo mas não contribui para o binário. Aceitável dado o requisito de não excluir.

## Open Questions

Nenhuma — o escopo está totalmente definido pela análise de dependências concluída na fase de exploração.
