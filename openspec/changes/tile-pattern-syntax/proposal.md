## Why

The existing SET/GET TILE PATTERN commands have several issues:

1. **Broken buffer form**: `SET TILE PATTERN <n>, <buffer>` generates code with calling convention `A=tile, HL=buffer` but the Z80 kernel's `set_tile_pattern` expects `HL=tile, DE=line, B=bank, C=byte` — a complete mismatch. The buffer form has never worked.

2. **No bank parameter on buffer form**: The inline form `(<l0>,...,<l7>)` supports an optional bank parameter, but the buffer/array form does not, creating inconsistency.

3. **GET TILE PATTERN is a stub**: The Z80 kernel's `get_tile_pattern` is just `ret` — not implemented.

4. **GET has no bank parameter**: GET TILE PATTERN doesn't support reading from a specific bank.

5. **No 4-integer array support**: The natural MSX BASIC way to hold pattern data is a `DIM` array of 4 integers (8 bytes), but this isn't properly supported.

This change formalizes and implements the correct syntaxes for SET and GET TILE PATTERN, aligning with the approach used by SET/GET TILE COLOR.

## What Changes

- Formalize `SET TILE PATTERN <n>, (<l0>,...,<l7>) [, <bank>]` — inline 8 bytes with optional bank select
- Add `SET TILE PATTERN <n>, <4-int-array> [, <bank>]` — DIM array of 4 integers, with optional bank select
- Add `GET TILE PATTERN <n>, <4-int-array> [, <bank>]` — read tile pattern into DIM array of 4 integers, with optional bank select
- Implement Z80 kernel bulk `set_tile_pattern_buffer` routine (LDIRVM block copy)
- Implement Z80 kernel `get_tile_pattern` routine (LDIRMV block read)
- Fix the calling convention mismatch in the compiler's buffer form path
- Fix broken jump table entries

## Capabilities

### New Capabilities
- `tile-pattern` — SET and GET TILE PATTERN with array and inline syntaxes

### Modified Capabilities
- *(none — tile-pattern is a new capability)*

## Impact

- `src/application/compiler/statements/strategies/graphics/set/compiler_set_statement_strategy.cpp` — SET TILE PATTERN buffer form dispatch
- `src/application/compiler/statements/strategies/graphics/get/compiler_get_statement_strategy.cpp` — GET TILE PATTERN with bank
- `src/application/compiler/helpers/hooks/compiler_hooks.h` — new hook constants for buffer routines
- `src/infrastructure/kernel/asm/src/header/31_cmd.asm` — implement get_tile_pattern, add set_tile_pattern_buffer
- `src/infrastructure/kernel/asm/src/header/20_runtime.asm` — add new jump table entries
- `src/infrastructure/kernel/asm/src/header/header.symbols.asm` — symbol addresses
- Release 1.0.0.0
