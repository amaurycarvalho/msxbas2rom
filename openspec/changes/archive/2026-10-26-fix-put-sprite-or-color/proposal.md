# Proposal

## Why

`PUT SPRITE` on MSX2/MSX2+ drops the OR and collision-ignore bits from its color
parameter. Issue #21 reports that `12+96` (OR + collision ignore + color 12)
produces color-table bytes `0x0C` instead of `0x6C`. The compiler already passes
the full value; the kernel's MSX2 branch masks it with `AND 0Fh`.

## What Changes

- Preserve bits 5 and 6 of the PUT SPRITE color parameter when writing the
  sprite color table on MSX2 and later.
- Keep the MSX1 path limited to the 4-bit sprite color.
- Add `tests/integration/GRAPH/sprite3.bas` to the regression workflow with a
  semantic VRAM probe for sprite 1's 16 color-table bytes (`0x6C`).
- Generate a `GRAPH/sprite3` baseline after the fix for broader coverage.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `sprites-handling`: `PUT SPRITE` color now preserves OR and collision-ignore
  bits on MSX2 and later.

## Impact

- `src/infrastructure/kernel/asm/src/header/90-graphics.asm`
- `tests/integration/GRAPH/sprite3.bas`
- `tests/integration/regression/manifest.json`
- `tests/integration/regression/baselines/GRAPH/sprite3.json`
- `openspec/specs/sprites-handling/spec.md`
- Depends on: `add-regression-vram-probes`,
  `fix-regression-machine-selection`, `fix-regression-boot-timing`.
- Affects: `refresh-regression-baselines`.
