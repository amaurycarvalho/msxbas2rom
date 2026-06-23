## Why

`90_support.asm` (4081 linhas, ~425 labels) contém 58 labels completamente inacessíveis de qualquer entry point externo (~13.6% do total). Este código morto infla o binário, aumenta o tempo de compilação do header Z80, e torna manutenção mais complexa ao dificultar a identificação do que realmente está ativo. A desativação preserva compatibilidade binária (endereços fixos nos símbolos exportados) e não altera o comportamento do runtime.

## What Changes

- Desativar 58 labels em `90_support.asm` que não possuem caminho de execução a partir de nenhum entry point externo (C++ via `def_*` macros ou outros `.asm`)
- Substituir cada bloco desativado por `DEFS <tamanho>, 0x00` para manter endereços absolutos dos labels subsequentes
- Blocos a desativar (agrupados por proximidade):
  - **Erros BASIC** (`J4DFA` a `I4E28`, 13 labels): jump table de códigos de erro nunca referenciada
  - **Get Sprite Definition** (`C716E` + helpers `J717D`/`J7183`, 3 labels): contraparte de `C7145` (usado), mas nunca chamada
  - **Rotinas de INPUT** (`I7517` a `C75BE` + tabela `I7530`, 22 labels): prompt numérico/string, busca de DATA, mensagem `"?Redo from start"`
  - **J761C** (1 label): helper de skip de bytes (só chamado por `J75F2`, no bloco acima)
  - **C76CE** (1 label): remainder de divisão inteira (ninguém chama; usa-se `C7643` diretamente)
  - **C78CD** (1 label): conversão unsigned int→float (ninguém chama)
  - **C7962** (1 label): float `<=>` trivial (retorna true; ninguém chama)
  - **C7F99** + helpers `J7FAD` a `J7FF6` (10 labels): concatenação de strings (macro definida, nunca usada)
- **NÃO alterar** (conforme solicitado): `BASIC_KUN_START_FILLER`, `ARKOS_PLAYER`, `I7EA8 (CHR$)`, `I7EC3 (HEX$)`, `I7EC6 (OCT$)`, `I7EC9 (BIN$)`, `I7EEF (SPACE$)`, `BASIC_KUN_END_FILLER`
- **Nenhuma alteração** em `header.symbols.asm`: os símbolos permanecem exportados nos mesmos endereços
- **Nenhuma alteração** em código C++ ou outros `.asm`

## Capabilities

### New Capabilities

Nenhuma — esta é uma mudança puramente interna de manutenção de código.

### Modified Capabilities

Nenhuma — os requisitos de comportamento não mudam. Apenas código morto é desativado.

## Impact

- **Afetado**: `src/infrastructure/kernel/asm/src/header/90_support.asm` (~58 labels desativados com DEFS filler)
- **Binário**: Tamanho idêntico (DEFS preserva offsets)
- **Símbolos exportados**: Inalterados (`header.symbols.asm` sem mudanças)
- **Compilação**: `header.asm` compila normalmente (DEFS é suportado pelo Pasmo)
- **Runtime**: Zero impacto funcional — código desativado nunca era executado
- Release 1.0.1.0
