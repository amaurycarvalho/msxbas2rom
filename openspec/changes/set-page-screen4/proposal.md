## Why

O MSX BASIC oficial suporta SET PAGE apenas para SCREEN 5 a 13 (modos bitmap com hardware de page flipping via DPPAGE/ACPAGE). Screen 4 (modo character-based 256x192, 16 cores de 512, SCRMOD=5) fica excluída — apesar de ter VRAM livre significativa (114KB em V9938 de 128KB). Jogos e demos que usam Screen 4 com tiles precisam de double-buffering e múltiplas "telas", forçando workarounds manuais com VPOKE/VPEEK e manipulação direta de registradores VDP. Esta change estende SET PAGE para Screen 4, aproveitando a VRAM livre como storage de páginas completas (tileset + name table) com troca via VDP command engine, mantendo compatibilidade total com SET TILE (3 bancos).

## What Changes

- Nova rotina kernel `screen4_set_page` em `90_support.asm` que gerencia páginas virtuais para Screen 4: reprograma R#2 (name table base), copia tileset (patterns + colors) entre VRAM storage e área ativa via VDP command engine, e atualiza BIOS variables (GRPNAM, GRPCOL, GRPCGP, NAMBAS, CGPBAS)
- Compiler: `cmd_set_page` em `compiler_set_statement_strategy.cpp` detecta SCRMOD=5 e emite chamada para `screen4_set_page` em vez de `C70CC` (que só funciona em screen 5+)
- Sintaxe mantida: `SET PAGE <DisplayPage>,<ActivePage>` — ambos opcionais, mínimo 1 parâmetro
- Até 8 páginas completas (page 0 = default nos endereços originais; pages 1-7 = armazenadas em VRAM livre a partir de 0x3900, ~13KB cada)
- CMD MTF: correção de endereços VRAM hardcoded em `31_cmd.asm` — passa a usar `(GRPCGP)`, `(GRPCOL)`, `(GRPNAM)` em vez de 0x0000/0x0800/0x1000/0x2000/0x2800/0x3000/0x1800
- Compatibilidade: SET TILE (3 bancos) coexiste — page 0 usa 3 bancos de patterns/colors normalmente; páginas 1-7 armazenam e restauram os 3 bancos completos
- CLS, PRINT, COPY, PUT TILE, GET TILE, SET TILE, TILE(), BASE(), SCREEN LOAD, BLOAD S respeitam SET PAGE automaticamente (já usam BIOS variables)
- Novos defines em `compiler_hooks.h` e `00_constants_and_workarea.asm` para constantes de page layout

## Capabilities

### New Capabilities

- `set-page-screen4`: Suporte a SET PAGE para Screen 4 no MSX2 — múltiplas páginas virtuais com troca de tileset e name table via VDP command engine
- `mtf-dynamic-vram`: CMD MTF usa endereços VRAM dinâmicos (lidos de BIOS variables) em vez de valores hardcoded, permitindo funcionar com layouts de VRAM customizados

### Modified Capabilities

<!-- Nenhum spec existente tem seus requirements alterados. SET PAGE para screen 5+ permanece inalterado. -->

## Impact

- **Kernel assembly**: `90_support.asm` (nova rotina `screen4_set_page`), `31_cmd.asm` (correção `cmd_mtf` hardcoded addresses), `00_constants_and_workarea.asm` (novas constantes de page layout), `header.symbols.asm` (novo símbolo exportado)
- **Compiler C++**: `compiler_set_statement_strategy.cpp` (detecção de SCRMOD e dispatch), `compiler_hooks.h` (novo define `def_screen4_set_page`), `compiler_context.h` (opcional: tracking de page state)
- **Testes**: `tests/unit/src/test_compiler.cpp` (testes unitários para `cmd_set_page` com Screen 4), `tests/integration/GRAPH/` (novo `.bas` de integração: SCREEN 4 + SET PAGE + verificação de isolamento entre páginas)
- **Release**: 1.4.0.0
