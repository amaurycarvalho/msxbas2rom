# Design

## Context

See `proposal.md`. Tier 1 protected fixed tables and resource lookup. Tier 2 concerns MTF map metadata, destination clipping, pattern/color table bounds, Tiny Sprite sizes, and dynamic font/mode values. The MTF kernel is resident/fixed and must stay within its module budget.

## Goals / Non-Goals

**Goals:**
- Make malformed map resources terminate safely.
- Preserve negative map-coordinate wrapping.
- Correct destination clipping without changing valid map scrolling.
- Bound remaining table accesses and dynamic mode values.

**Non-Goals:**
- Shared guard macros (Tier 3).
- Compiler-side literal validation (C++ Tier 2).
- New MTF features such as page support.

## Decisions

### Decision 1: Validate header before coordinate math

`cmd_mtf.map` will check the resource size and read width/height, then reject zero dimensions before any modulo/negative-wrap loop. This prevents the infinite loop and division by zero at the source.

Alternative: guard each loop. Rejected because duplicate checks increase fixed-module size.

### Decision 2: Keep negative map wrap, fix destination clipping

The negative-wrap loops remain for `map_x`/`map_y`. Destination `screen_x`/`screen_y` use signed clipping: negative values clip to `0` (or skip), values above the screen clip to the edge. They are not passed through the map modulo logic.

Alternative: apply modulo to destination coordinates. Rejected because destination is a screen position, not a map position.

### Decision 3: 16-bit window size handling

Width/height will be interpreted as 16-bit and clamped to the remaining screen area. The current low-byte-only behavior is replaced with deterministic clamping.

Alternative: reject all values above 255. Rejected because clipping is already part of the window contract.

### Decision 4: Table bounds at the wrapper entry points

Pattern/color entry points will normalize indices using table capacities before calling BIOS address routines. SETFNT/TURBO/PAGE/CLIP/PLAY guards will sit at their command entries.

Alternative: guard inside BIOS helpers. Rejected because BIOS helpers are shared and some callers use valid wider pattern ranges.

## Risks / Trade-offs

- Additional MTF checks increase fixed-module size → measure with the kernel size check and relocate non-fixed code if needed.
- Destination clipping policy may change visible behavior for previously wrapped negatives → document as a bug fix.
- Pattern range policy must not break multi-bank tile/pattern usage → define per-table capacity and test bank variants.
- `.SPR` size checks need the resource size returned by the loader → use the documented loader output, not the header alone.

## Migration Plan

1. Add MTF header/destination guards.
2. Add pattern/color and `.SPR` bounds.
3. Add font/mode/channel guards.
4. Run MTF/sprite/tile regression probes and kernel size checks.
5. Roll back per command family if needed.

## Open Questions

- Exact table capacity for sprite pattern operations on MSX1 versus MSX2. Resolve from the BIOS contract during implementation; use the most restrictive safe bound for shared code.
