# Proposal

## Why

Tile bank selection is inconsistent: a bank value of 3 already means "all banks" in the SET routines, but values above 3 can compute invalid VRAM addresses or silently saturate to bank 2. The compiler also accepts incompatible parameter shapes, so scalar variables can be mistaken for buffers and invalid destinations can corrupt RAM.

## What Changes

- Normalize tile bank selection in the Z80 kernel: any bank value at or above 3 means "all banks" for SET TILE PATTERN, SET TILE COLOR, SET TILE FLIP, and SET TILE ROTATE.
- Normalize GET TILE PATTERN and GET TILE COLOR bank values at or above 3 to bank 0, the documented default, so they never compute an invalid VRAM address.
- **BREAKING**: require an array destination for `GET TILE PATTERN` and `GET TILE COLOR`; literals and scalar variables are rejected instead of being used as RAM pointers.
- **BREAKING**: distinguish buffer arrays from scalar FC/BC values with the declared array flag, so `SET TILE COLOR 5, F%, G%` means foreground/background values rather than a buffer copy.
- **BREAKING**: reject tile tuple/array parameters with more than 8 rows, bank parameters that are arrays, and inline `SET TILE PATTERN` parameter counts above 3 instead of compiling a silent no-op.
- Hardening: mask background color values to 4 bits, validate tile FLIP/ROTATE direction and bank parameter shapes, and treat SCREEN 0/1 pattern writes as single-bank operations.
- Fix `SET TILE COLOR` bank forms so the bank parameter never overwrites the FC/BC operands or skips the FC array loop.
- Add deterministic VRAM probes to `test87`, `test88`, `test97`, and `tile_bank_all`, and require the affected tile programs (`test87`, `test88`, `test94`, `test97`, `tile_flip_rotate`, `tile_bank_all`) to run in the GRAPH regression.
- Add unit, kernel-build, and emulator regression coverage for bank values 3 and well above 3.

## Capabilities

### New Capabilities

- `tile-commands`: durable contract for MSX tile bank selection, parameter disambiguation, and validation across SET/GET TILE PATTERN, SET/GET TILE COLOR, and SET TILE FLIP/ROTATE.

### Modified Capabilities

None. The project has no existing tile capability under `openspec/specs/`; related behavior currently lives only in archived change specs.

## Impact

- `src/infrastructure/kernel/asm/src/header/31-cmd-tile-sprite.asm`
- `src/application/compiler/statements/strategies/graphics/set/compiler_set_statement_strategy.cpp`
- `src/application/compiler/statements/strategies/graphics/get/compiler_get_statement_strategy.cpp`
- `src/application/parser/statements/strategies/set/set_statement_strategy.cpp`
- Unit tests under `tests/unit/src/` and emulator regression assets under `tests/integration/`.
- Regenerated kernel binary, symbols, and manifests after the assembly change.
