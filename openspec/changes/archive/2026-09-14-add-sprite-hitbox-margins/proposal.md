## Why

The current sprite collision system treats every sprite as a full 16×16 (or 8×8) rectangle, producing collisions on transparent or visually irrelevant areas. This change adds per-sprite configurable hitbox margins so collision geometry can approximate the visible sprite shape without altering the sprite graphics.

## What Changes

- New BASIC command family `SET SPRITE HITBOX` with these syntax forms:
  - `SET SPRITE HITBOX ON` — initialize all 32 sprites with zero margins (runtime default)
  - `SET SPRITE HITBOX OFF` — disable collision for all 32 sprites
  - `SET SPRITE HITBOX AUTO` — one-shot bounding-box calculation from the VRAM sprite pattern for all sprites
  - `SET SPRITE HITBOX <sprite>` — reset one sprite's margins to zero (enabled)
  - `SET SPRITE HITBOX <sprite> ON` — same as `SET SPRITE HITBOX <sprite>` (zero margins, enabled)
  - `SET SPRITE HITBOX <sprite> OFF` — disable collision for one sprite
  - `SET SPRITE HITBOX <sprite> AUTO` — one-shot bounding-box calculation for one sprite
  - `SET SPRITE HITBOX <sprite>, <left>, <top>, <right>, <bottom>` — explicit margins (trailing params default to zero)
- New runtime table `HITBOX_TABLE` (32 × 5 bytes) storing pre-normalized relative bounds (`X0`, `X1`, `Y0`, `Y1`) plus a per-sprite enabled flag in the reserved byte.
- `COLLISION()` internally combines the sprite position from `SPRTBL` with relative bounds from `HITBOX_TABLE`. The public API (`COLLISION()`, `COLLISION(n)`, `COLLISION(n1,n2)`) is unchanged.
- New single kernel routine `set_sprite_hitbox` with a mode byte implementing all five command forms.
- New lexer keywords `HITBOX` and `AUTO`.
- `HITBOX_TABLE` is reset to default on sprite clear (`SUB_CLRSPR`), which also covers the SCREEN sprite-size path.
- Target release 1.3.0.0.

## Capabilities

### New Capabilities

- *(none — extends the existing `sprites-handling` capability)*

### Modified Capabilities

- `sprites-handling`: adds the `SET SPRITE HITBOX` command family and changes `COLLISION()` to use effective per-sprite hitboxes (relative bounds + position) while preserving the existing public API.

## Impact

- `src/domain/lexer/lexeme.cpp` — add `HITBOX` and `AUTO` keywords
- `src/application/parser/statements/strategies/set/set_statement_strategy.cpp` — parse `SET SPRITE HITBOX` (variable arity 0..5)
- `src/application/compiler/statements/strategies/graphics/set/compiler_set_statement_strategy.cpp` — emit `DISP_set_sprite_hitbox` calls
- `src/application/compiler/helpers/hooks/compiler_hooks.h` — add `DISP_set_sprite_hitbox` constant
- `src/infrastructure/kernel/asm/src/header/20_runtime.asm` — add `set_sprite_hitbox` dispatch entry
- `src/infrastructure/kernel/asm/src/header/00_constants_and_workarea.asm` — allocate `HITBOX_TABLE` (160 bytes)
- `src/infrastructure/kernel/asm/src/header/60_bios_helpers.asm` — reset `HITBOX_TABLE` in `SUB_CLRSPR`; rewrite collision hot path to combine position + relative bounds
- `src/infrastructure/kernel/asm/src/header/31_cmd.asm` — implement `set_sprite_hitbox`
- `src/infrastructure/kernel/asm/src/header.symbols.asm` — regenerate symbols
- `tests/unit/src/test_parser.cpp`, `tests/unit/src/test_compiler.cpp` — new unit tests
- `tests/integration/GRAPH/` — integration test
