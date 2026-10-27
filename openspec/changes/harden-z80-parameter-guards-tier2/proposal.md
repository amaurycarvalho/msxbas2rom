# Proposal

## Why

Tier 1 protects fixed tables and resource loading. Tier 2 covers the remaining dynamic parameter contracts: malformed MTF headers can loop forever, destination clipping can wrap to the opposite edge, sprite/tile pattern and color indices can address outside their tables, `.SPR` resources can overrun their declared data, and font/page/turbo/play values are only partially masked.

## What Changes

- Harden MTF resource/map handling:
  - Reject or safely terminate malformed map headers with zero width/height and insufficient resource size.
  - Preserve the documented negative `map_x`/`map_y` wrapping for operations 1 and 2.
  - Validate operation `0..2`; invalid dynamic values become a no-op.
  - Clip destination `screen_x`/`screen_y` correctly, including negative values, instead of mapping them to `31`/`23`.
  - Handle full 16-bit `width`/`height` or clamp them deterministically.
- Validate sprite pattern/color indices and tile pattern/color indices against their table capacities.
- Validate the Tiny Sprite resource size in `cmd_wrtspr` against the declared sprite count before copying.
- Validate `CMD SETFNT` style and bank at runtime, clamping or no-op for invalid dynamic values.
- Mask/clamp dynamic values for `CMD TURBO`, `CMD PAGE`, `CMD CLIP`, and `CMD PLAY` to their documented ranges.
- Add regression programs and probes for MTF negative-map wrapping, malformed headers, destination clipping, invalid sprite/tile indices, and `.SPR` size overruns.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `kernel-parameter-guards`: extend runtime guards to MTF headers/clipping, fixed-table pattern/color indices, resource sizes, font/bank, and mode/channel values.
- `mtf-commands`: define safe behavior for malformed map headers and destination clipping while preserving negative map-coordinate wrapping.

## Impact

- Z80 kernel modules `31-cmd-mtf.asm`, `31-cmd-tile-sprite.asm`, `31-cmd-game-graphics.asm`, and player/command handlers.
- Existing MTF programs with negative map coordinates must remain valid; only malformed headers and invalid destination/window values change behavior.
- Emulator regression manifest entries and VRAM/RAM probes for the new guards.
