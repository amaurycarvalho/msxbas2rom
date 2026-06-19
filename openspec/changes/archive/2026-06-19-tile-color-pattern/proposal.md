## Why

Tile color manipulation is a gap in the current tile API — SET/GET TILE COLOR exist as stubs or use incompatible calling conventions, breaking the buffer-based syntax. This change implements the full tile color API so developers can read and write tile color tables using the same buffer pattern as SPRITE COLOR commands. Unit and integration tests are needed alongside implementation to prevent regressions and validate all syntax forms.

## What Changes

- Implement SET TILE COLOR with 3 syntax forms: simple (fc, bc), array (fc0..fc7 with optional bc0..bc7), and buffer (4-integer array)
- Optionally accept bank parameter (0-2, default 3=all for SET, default 0 for GET)
- Buffer uses MSX native color table format: each 16-bit integer packs two rows (low byte = row N, high byte = row N+1), each byte = FC<<4 | BC
- Implement GET TILE COLOR with buffer syntax and optional bank parameter
- Fix the SET TILE COLOR buffer form calling convention (currently generates wrong Z80 code)
- Implement the Z80 kernel routines for buffer-based SET and GET (currently stub/missing)
- Use LDIRVM/LDIRMV for block transfer (3 calls for 3 banks)
- Add unit tests in tests/unit/ covering all syntax forms for both SET and GET TILE COLOR (compiler translation layer)
- Add integration test program in tests/integration/graph/ similar to test94.bas to exercise all syntaxes on real/virtual hardware

## Capabilities

### New Capabilities
- `tile-color`: Read and write tile color tables via SET/GET TILE COLOR commands

### Modified Capabilities
- *(none — tile-color is a new capability separate from existing specs)*

## Impact

- `src/infrastructure/kernel/asm/src/header/31_cmd.asm` — new `set_tile_color_buf` and `get_tile_color` routines
- `src/infrastructure/kernel/asm/src/header/20_runtime.asm` — jump table entries
- `src/infrastructure/kernel/asm/src/header.symbols.asm` — new symbol entries
- `src/application/compiler/helpers/hooks/compiler_hooks.h` — new `def_` constants
- `src/application/compiler/statements/strategies/graphics/set/compiler_set_statement_strategy.cpp` — restructure COLOR handler logic
- `src/application/compiler/statements/strategies/graphics/get/compiler_get_statement_strategy.cpp` — add bank parameter support
- `tests/unit/src/test_compiler.cpp` — new test cases for all SET/GET TILE COLOR syntax forms
- `tests/integration/GRAPH/test97.bas` — integration test exercising all syntaxes
- Release 1.0.0.0
