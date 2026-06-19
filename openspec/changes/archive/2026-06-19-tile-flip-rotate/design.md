## Context

SET SPRITE FLIP/ROTATE already exist with full assembly implementations. SET TILE FLIP/ROTATE already have parser and compiler layers implemented (generating calls to `def_set_tile_flip`/`def_set_tile_rotate`), but the assembly routines in `31_cmd.asm` are stubs (just `ret`). The bank parameter (0-2, default 3=all) and tests are also missing.

Tile patterns are **8 bytes** (8x8 pixels, 1 bpp) vs sprite patterns 32 bytes (16x16 pixels). Existing `blockRotateL`/`blockRotateR` routines operate on 8-byte blocks and can be reused directly.

Supported screens: 0, 1, 2. Screen 3 (multicolor) is excluded.

## Goals / Non-Goals

**Goals:**
- `SET TILE FLIP <n>, <dir>[, <bank>]` with 3 directions
- `SET TILE ROTATE <n>, <dir>[, <bank>]` with 3 directions
- Optional bank parameter (0-2, default 3=all banks)
- Screen 0, 1, 2 support (not screen 3+)
- Unit tests for parser + compiler
- Integration test in GRAPH/

**Non-Goals:**
- Screen 3 (multicolor) or screen 4+ bitmap modes
- Tile color table manipulation (separate command)
- Performance optimization beyond sprite patterns

## Decisions

### Decision 1: Calling convention — B register for bank parameter
Follow the existing `set_tile_pattern_buffer` convention: `B` = bank (0-2, 3=all). The current calling convention (DE=tile, HL=direction) is preserved; `B` is added as a third parameter.

```
Entry:
  DE = tile number
  HL = direction (flip: 0=horiz, 1=vert, 2=both; rotate: 0=left, 1=right, 2=180)
  B  = bank (0-2 specific, 3=all, default 3)
```

### Decision 2: Reuse existing flip/rotate subroutines
- `binaryReverseA` — bit reversal (horizontal flip)
- `blockRotateL` / `blockRotateR` — full 8-byte block rotation (90° left/right)
- 180° rotation = flip both (same as sprite approach)

### Decision 3: STRBUF for temporary workspace
The 43-byte `STRBUF` work area is sufficient (tile is only 8 bytes). Follow same read-modify-write pattern as sprite: copy from VRAM → RAM → transform → paste back to VRAM.

### Decision 4: Bank loop same as set_tile_pattern_buffer
For `B=3` (all banks), iterate banks 0, 1, 2 writing to each. For `B=0-2`, write to specific bank only. Screen 0 always reads from/writes to bank 0 only (fixed pattern table at VRAM 0x0800).

### Decision 5: Parser handles 2 or 3 parameters
`parseSetTileColpat()` already uses a state-machine parser for comma-separated parameters. It will accept 2 or 3 params and pass the bank value (default 3 if omitted) to the compiler layer.

### Decision 6: Compiler passes bank in B register
When 2 parameters (no bank): `LD B, 3` (default all).
When 3 parameters: `LD B, <bank>`.

## VRAM Layout

```
Screen 0:
  Pattern table:   0x0800
  Bank count:      1

Screen 1-2:
  Pattern table:   0x0000
  Banks:           3 (offset +0x0800 each)

Each tile:         8 bytes (tile_number * 8)
```

## Tile Pattern Operations

```
FLIP Horizontal:   reverse bits of each byte (binaryReverseA × 8)
FLIP Vertical:     swap byte pairs: 0↔7, 1↔6, 2↔5, 3↔4
FLIP Both:         do horizontal then vertical (= 180° rotation)

ROTATE Left (CCW): blockRotateL on 8-byte tile
ROTATE Right (CW): blockRotateR on 8-byte tile
ROTATE 180:        FLIP Both (reuse)
```

## Risks / Trade-offs

- Minimal — reuses proven sprite flip/rotate patterns and subroutines
- Screen 3 multicolor mode has different pattern structure (4x4 blocks); excluded intentionally to avoid complexity
- Bank parameter adds minor parsing complexity but follows established pattern from SET TILE PATTERN
