## Why

Complete tile manipulation by adding SET TILE FLIP and SET TILE ROTATE commands, mirroring the existing SET SPRITE FLIP/ROTATE functionality. Fills a gap in tile editing capabilities needed for MSX screen 2 game development.

## What Changes

- Implement `SET TILE FLIP <n>, <dir>[, <bank>]`
  - dir: 0=horizontal, 1=vertical, 2=both
  - bank: optional 0-2 (default 3=all)
- Implement `SET TILE ROTATE <n>, <dir>[, <bank>]`
  - dir: 0=left, 1=right, 2=180 degrees
  - bank: optional 0-2 (default 3=all)
- Add assembly-level tile flip/rotate routines in kernel (screen 2, text modes 0-1)
- Add unit tests (parser + compiler) and integration test (GRAPH/)

## Capabilities

### New Capabilities
- *(none — extends tile functionality under existing `tile-handling`)*

### Modified Capabilities
- *(none)*

## Impact

- `src/application/parser/statements/strategies/set/set_statement_strategy.cpp` — update parseSetTileColpat for 3rd optional bank parameter
- `src/application/compiler/statements/strategies/graphics/set/compiler_set_statement_strategy.cpp` — update tile FLIP/ROTATE compiler to pass bank parameter
- `src/infrastructure/kernel/asm/src/header/31_cmd.asm` — implement set_tile_flip and set_tile_rotate assembly routines
- `src/infrastructure/kernel/asm/src/header/20_runtime.asm` — no change (already in dispatch table)
- `tests/unit/src/test_parser.cpp` — add parser unit tests
- `tests/unit/src/test_compiler.cpp` — add compiler unit tests
- `tests/integration/GRAPH/` — add integration test
- Release 1.0.0.0
