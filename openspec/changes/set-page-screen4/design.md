## Context

O MSX2 V9938 possui 128KB de VRAM (0x00000-0x1FFFF). Em Screen 4 (SCRMOD=5, modo character-based 256x192 com 16 cores), o layout default ocupa ~14KB (patterns 0x0000-0x17FF, colors 0x2000-0x37FF, name table 0x1800, sprites 0x1E00/0x3800), deixando ~114KB livres a partir de 0x3900.

O oficial `SET PAGE` do MSX BASIC só funciona em SCREEN 5+ porque usa hardware de page flipping (DPPAGE/ACPAGE nos registradores VDP #6, #11, #14). Para Screen 4, isso não existe — os registradores VDP #2 (name table), #3 (color table) e #4 (pattern generator) têm granularidade e alcance limitados:

- R#2: 16 posições (N10-N13 × 0x400, max 0x3C00)
- R#3: 2 posições (C13 → 0x0000 ou 0x2000)
- R#4: 2 posições (F13 → 0x0000 ou 0x2000), complementar ao R#3

Com 3 bancos de patterns/colors (0x1800 cada), só ~2-3 posições de name table ficam livres de conflito. Para suportar múltiplas páginas completas (com tileset independente), é necessário armazenar os dados de cada página em VRAM livre e copiá-los para a área ativa na troca.

A rotina kernel existente `C70CC` (XBASIC_SET_PAGE) modifica DPPAGE e registradores #2, #6, #11 — desenhada para screen 5+. Para Screen 4, precisamos de uma nova rotina que use LDIRVM para copiar tilesets entre storage e área ativa, e WRTVDP para reprogramar R#2 (name table).

## Goals / Non-Goals

**Goals:**
- Suportar `SET PAGE <DisplayPage>,<ActivePage>` em Screen 4 (SCRMOD=5)
- Até 8 páginas completas (page 0 default, pages 1-7 em VRAM livre)
- Cada página contém: 3 bancos de patterns (0x1800 bytes), 3 bancos de colors (0x1800 bytes), name table (0x300 bytes)
- Troca de página via VDP command engine (LDIRVM) — patterns + colors + name table copiados do storage para área ativa
- Page 0 preserva comportamento default (endereços VDP padrão)
- Compatível com SET TILE (3 bancos): page 0 usa bancos normalmente; páginas 1-7 salvam/restauram os 3 bancos completos durante troca
- CMD MTF corrigido para usar BIOS variables em vez de endereços hardcoded
- CLS, PRINT, COPY, PUT TILE, GET TILE, SET TILE, TILE(), BASE(), SCREEN LOAD, BLOAD S funcionam corretamente com qualquer página ativa

**Non-Goals:**
- Não altera comportamento de SET PAGE para screen 5+ (dispatch decide qual rotina chamar)
- Não suporta Screen 0-3 (não têm VRAM suficiente ou páginas não fazem sentido)
- Não implementa page flipping automático/wave (isso é para screen 5+)
- Não altera sintaxe do SET PAGE (mantém 1-2 parâmetros numéricos)
- Não modifica comportamento de sprites entre páginas (sprites são globais)

## Decisions

### D1: Estratégia de páginas — "Full page" storage com VRAM copy

**Escolha**: Cada página armazena dados completos (patterns 3 bancos + colors 3 bancos + name table) em VRAM livre. Na troca, copia via LDIRVM.

**Alternativa considerada**: Name-table-only (apenas R#2), tileset compartilhado. Mais simples mas com apenas ~2-3 páginas utilizáveis no layout default de 3 bancos. Menos útil para jogos que precisam de tilesets distintos por fase/menu.

**Alternativa considerada**: Page flipping via R#3+R#4 swap (2 posições de pattern/color × 16 name tables). Apenas 2 conjuntos de tiles, e conflito com bancos do SET TILE.

**Rationale**: Estratégia B (full page) é a mais poderosa e a que o usuário solicitou. Permite até 8 páginas com tilesets completamente independentes.

### D2: Layout VRAM — Storage a partir de 0x3900

**Escolha**: Área de storage começa em 0x3900 (após sprite patterns em 0x3800-0x38FF). Cada página ocupa 0x3400 bytes (13KB):
- Patterns (3 bancos): 3 × 0x800 = 0x1800
- Colors (3 bancos): 3 × 0x800 = 0x1800
- Name table: 0x400 (alinhado, usa 0x300)

Páginas alocadas sequencialmente:
```
Page 1: 0x03900 - 0x06CFF (0x3400 bytes)
Page 2: 0x06D00 - 0x0A0FF
Page 3: 0x0A100 - 0x0D4FF
Page 4: 0x0D500 - 0x108FF
Page 5: 0x10900 - 0x13CFF
Page 6: 0x13D00 - 0x170FF
Page 7: 0x17100 - 0x1A4FF
```

**Rationale**: Endereços previsíveis, sem sobreposição com áreas de sistema (0x0000-0x38FF). 7 páginas extras + page 0 = 8 páginas totais.

### D3: Mecanismo de troca de página

**Escolha**: Rotina kernel `screen4_set_page` implementa:
1. Se a página atual != 0 e teve tiles modificados: salva patterns + colors + name table da área ativa de volta para o storage da página atual (LDIRMV)
2. Carrega dados da nova página do storage para área ativa (LDIRVM)
3. Se for display page: reprograma R#2 com WRTVDP para o name table correto (0x1800 para todas as páginas, pois o name table é carregado na mesma posição)
4. Atualiza BIOS variables: GRPNAM=0x1800, GRPCOL=0x2000, GRPCGP=0x0000, NAMBAS, CGPBAS

**Alternativa considerada**: Não salvar página atual automaticamente — exigiria que o programador chamasse explicitamente uma rotina de save. Mais rápido na troca, mas propenso a perda de dados.

**Alternativa considerada**: Usar dirty flag para evitar save desnecessário. Adiciona complexidade e consumo de RAM de trabalho.

**Rationale**: Save automático simplifica a API. O custo de LDIRMV (~30ms para 13KB) é aceitável e só ocorre se a página foi modificada (detectável via dirty flag futuro).

### D4: Coexistência SET PAGE + SET TILE (3 bancos)

**Escolha**: SET TILE ON inicializa page 0 com font/tiles nos 3 bancos normalmente. Ao trocar para page N>0, os 3 bancos completos da page 0 são salvos no storage da page 0, e os 3 bancos da page N são carregados. Ao retornar para page 0, os bancos originais são restaurados.

**Rationale**: 3 bancos são necessários para SET TILE (fontes múltiplas). O storage por página inclui todos os 3 bancos, então a troca preserva a configuração de tiles de cada página.

### D5: CMD MTF — Corrigir endereços hardcoded

**Escolha**: Substituir todos os endereços VRAM hardcoded em `cmd_mtf` por leituras de `(GRPCGP)`, `(GRPCOL)`, `(GRPNAM)` com offsets de banco calculados em runtime:
- Pattern bank 0: (GRPCGP)
- Pattern bank 1: (GRPCGP) + 0x0800
- Pattern bank 2: (GRPCGP) + 0x1000
- Color bank 0: (GRPCOL)
- Color bank 1: (GRPCOL) + 0x0800
- Color bank 2: (GRPCOL) + 0x1000
- Name table: (GRPNAM)

**Rationale**: Isso é um bug independente do SET PAGE — MTF nunca funcionaria com layouts VRAM customizados. A correção beneficia todos os usos de CMD MTF, não só Screen 4 com páginas.

### D6: Detecção de Screen 4 no compiler

**Escolha**: `cmd_set_page` verifica SCRMOD em runtime (não em compile-time). Emite:
```asm
ld a, (SCRMOD)
cp 5
jr nz, .use_c70cc    ; screen 5+ usa C70CC
; screen 4 path: chama screen4_set_page
call screen4_set_page
jr .end
.use_c70cc:
call C70CC
.end:
```

**Alternativa considerada**: Verificar em compile-time se o programa usa SCREEN 4. Frágil — o modo pode mudar em runtime via `SCREEN` statement. A verificação em runtime é mais robusta.

### D7: Active Page vs Display Page

**Escolha**: Mantida a semântica do MSX BASIC:
- `SET PAGE displayPage` — muda apenas o que é exibido (display page = R#2)
- `SET PAGE displayPage, activePage` — muda display + onde writes vão (active page = BIOS variables)
- Quando só displayPage é especificado, activePage = displayPage

**Rationale**: Consistente com o comportamento existente para screen 5+.

## Risks / Trade-offs

- **[Risco] Tempo de troca de página**: LDIRVM de 13KB leva ~30ms (em Z80 a 3.58MHz). Durante a cópia, interrupções podem causar glitches visuais. → **Mitigação**: Executar durante VBLANK ou usar DISSCR/ENASCR. Documentar que trocas de página devem ocorrer fora de loops críticos de renderização.

- **[Risco] Consumo de VRAM**: 7 páginas × 13KB = 91KB de storage. Se o programa usar muitos sprites ou recursos adicionais em VRAM, pode faltar espaço. → **Mitigação**: Documentar o limite. Se necessário no futuro, pode-se reduzir para 2 bancos por página (9KB cada → mais páginas).

- **[Risco] Compatibilidade com MSX1**: Screen 4 é MSX2-only. SET PAGE para screen 4 deve retornar silenciosamente ou gerar erro em MSX1. → **Mitigação**: Rotina kernel verifica VERSION antes de executar.

- **[Trade-off] Save automático vs performance**: Salvar 13KB em toda troca de página custa ~60ms (LDIRMV + LDIRVM). Para jogos que trocam de página frequentemente, isso pode ser um gargalo. → **Mitigação**: Futura otimização com dirty flag (só salva se modificado). Por ora, documentar.

- **[Risco] Colisão com outras rotinas**: `cmd_wrtscr`, `cmd_wrtchr`, `cmd_wrtclr` já usam BIOS variables, mas podem ter assumido endereços fixos em outros lugares. → **Mitigação**: Testes de integração cobrindo todos os comandos listados.
