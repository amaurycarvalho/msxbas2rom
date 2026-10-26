# Design

## Context

See `proposal.md` - Why. The compiler passes the full color value in `D` to
`DISP_XBASIC_PUT_SPRITE`. For `12 + 96` it emits `HL=0x6C`, so the value reaching
the kernel is correct. The MSX2 path in `C71BF` then executes `AND 0Fh` and
writes `0x0C` to all 16 sprite color-table bytes.

A throwaway one-byte patch (`AND 0Fh` -> `AND 7Fh`) in a test ROM changed only
VRAM `0x1C10..0x1C1F` from `0x0C` to `0x6C`; every other VRAM byte was identical
and the screenshot showed the OR color in the overlap.

## Goals / Non-Goals

**Goals:**

- Preserve OR and collision-ignore bits in the MSX2 sprite color table.
- Add a semantic regression assertion for the exact bytes.
- Keep existing programs that pass colors 0-15 unchanged.

**Non-Goals:**

- Changing the compiler's color expression handling.
- Supporting MSX1 OR colors (the TMS9918 has no OR-color feature).
- Fixing the documented MSX-BASIC OR bug above sprite 26.

## Decisions

### D1: One-byte MSX2 kernel patch

Change `AND 0FH` to `AND 7FH` at
`src/infrastructure/kernel/asm/src/header/90-graphics.asm:1007` (the MSX2
branch). Leave the MSX1 branch at `:933` with `AND 0FH`. The mask keeps bits 0-6,
drops the invalid bit 7, and matches the existing `COLOR SPRITE` path
(`AND 7FH`). The instruction size is unchanged, so kernel addresses do not shift.

### D2: Preserve the existing merge semantics

The routine writes `(old & mask) | color_bits`. Bits 5-6 from the input color are
now ORed into the table; old high bits remain preserved, as before. To clear OR
or collision-ignore, use `COLOR SPRITE`/`COLOR SPRITE$`. This is acceptable and
consistent with the existing behavior.

### D3: Semantic VRAM probe, not only a baseline hash

Add `sprite3.bas` to `GRAPH` with a `vram.probes` entry for
`0x1C10..0x1C1F == 6C x16`. This fails against the current kernel regardless of
when the baseline was generated. Also generate a baseline after the fix for
broader RAM/VRAM coverage. The test pins `Panasonic_FS-A1WX` and uses the
boot-delay fix so the program has executed before capture.

### D4: Keep the test portable within the chosen layout

The probe address `0x1C10` is the screen-4 sprite color table for sprite 1 on the
pinned MSX2+ machine. Document that the address is layout-specific; if the test
machine/layout changes, adjust the probe and baseline together.

## Risks / Trade-offs

- **[Risk] Programs relying on OR bits being preserved from `COLOR SPRITE` see
  no change; programs expecting PUT SPRITE to clear them still cannot** ->
  Document the clearing path (`COLOR SPRITE`).
- **[Risk] OR above sprite 26 remains a known MSX-BASIC limitation** -> Out of
  scope; add a separate case only if required.
- **[Risk] The probe address depends on the screen-4 VRAM layout** -> Pin the
  machine and document the address; the regression baseline provides a second
  layer of detection.
- **[Risk] Boot timing could capture before the sprites are drawn** -> Depend on
  `fix-regression-boot-timing`; use 15 seconds for MSX2+.

## Migration Plan

- Rebuild the kernel and ROMs; run the sprite regression.
- Generate the `sprite3` baseline after the fix; review the color-table bytes.
- Roll back by restoring `AND 0FH` and removing the probe/baseline.
