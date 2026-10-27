# Proposal

## Why

Tier 1 fixes obvious parameter-loss and literal-range bugs. Tier 2 handles parameters whose validity depends on declared arrays, resource metadata, or command-specific windows: sprite buffers currently accept any identifier, resource numbers are not checked against the resource map, `CMD RUNASM` can silently truncate routines, and MTF parameters are only partially validated.

## What Changes

- Require `SET/GET SPRITE PATTERN` and `SET/GET SPRITE COLOR` to receive a declared array buffer, and validate the minimum element count where the command contract requires it.
- Harden MTF compiler parameters:
  - Validate operation `0..2`.
  - Validate `width`, `height`, and destination `screen_x`/`screen_y`.
  - **Do not** reject negative `map_x`/`map_y`; operations 1 and 2 intentionally support negative map coordinates with modulo wrapping.
  - Preserve documented clipping for destination coordinates that exceed the screen.
- Add literal range checks for fixed-table indices: sprite numbers for sprite operations, `KEY` and `STRIG` numbers, with `COLLISION(-1)` explicitly preserved as the documented "any sprite" sentinel.
- Validate literal resource numbers against the compiler's resource map for `CMD`/`BLOAD`/`SCREEN LOAD`/Arkos/MTF/TS commands where the resource index is a compile-time literal.
- Validate `CMD RUNASM` literal resources: reject resources larger than 255 bytes and resources whose storage format is compressed or otherwise not executable.
- Introduce shared compiler helpers for parameter count, numeric/string type, declared array, resource literal, and signed literal range so Tier 1 and Tier 2 handlers use one policy.
- Add table-driven unit coverage for command/function parameter contracts and integration coverage for the newly rejected programs.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `compiler`: add resource-aware parameter validation, MTF parameter validation, `RUNASM` resource checks, and shared parameter-validation policy.
- `sprites-handling`: require declared array buffers for sprite pattern/color access and range-check fixed-table sprite indices.
- `tile-commands`: validate tile bank literals and fixed-table tile indices.
- `mtf-commands`: validate operation and window/destination parameters while preserving negative map-coordinate wrapping.

## Impact

- C++ compiler strategies for CMD, sprite, tile, MTF, resource, screen, BLOAD, and TS commands.
- Resource manager lookups during compilation.
- Unit test matrix and integration programs that intentionally use invalid literals.
- No kernel changes; runtime values remain protected by the Z80 changes.
