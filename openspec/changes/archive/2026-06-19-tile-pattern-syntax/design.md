## Context

SET TILE PATTERN currently has two compiler paths:

1. **Inline array form** `(n, (<l0>...<l7>) [, bank])` — works. Compiler loops 8 times calling `set_tile_pattern` with HL=tile, DE=line (0-7), B=bank, C=byte. Each call writes 1 byte to VRAM.

2. **Buffer form** `(n, buffer)` — broken. Compiler generates `A=tile, HL=buffer` and calls `def_set_tile_pattern`, but the kernel routine expects `HL=tile, DE=line, B=bank, C=byte`. Calling convention mismatch.

GET TILE PATTERN has one form `(n, buffer)` — the kernel routine `get_tile_pattern` is a stub (`ret`).

Neither form properly supports:
- Bank parameter on the buffer/array form
- A DIM'd 4-integer array as the data source/destination
- Bulk VRAM transfer (LDIRVM/LDIRMV) for efficiency

The pattern table in VRAM (screens 0–4) is 8 bytes per tile, one byte per row, each byte is a bit pattern for 8 pixels.

## Goals / Non-Goals

**Goals:**
- Implement `SET TILE PATTERN <n>, (<l0>,...,<l7>) [, <bank>]` — already works, formalize
- Implement `SET TILE PATTERN <n>, <4-int-array> [, <bank>]` — new buffer form with bulk copy
- Implement `GET TILE PATTERN <n>, <4-int-array> [, <bank>]` — new read form with bulk copy
- Use LDIRVM/LDIRMV for efficient block transfer (8 bytes at once)
- Fix the broken buffer form calling convention in compiler and kernel
- Implement the stub `get_tile_pattern` kernel routine

**Non-Goals:**
- SET/GET TILE COLOR (separate change)
- Per-pixel pattern formats or rotation/flip (separate changes)

## Decisions

### Decision 1: Separate kernel routines for per-line and bulk modes

Keep the existing `set_tile_pattern` entry point for the inline form (per-line writes via compiler loop). Add a new `set_tile_pattern_buffer` entry for the bulk array form.

```
                  ┌─────────────────────────┐
                  │  Runtime Jump Table     │
                  │  (20_runtime.asm)       │
                  └───────┬─────────────────┘
                          │
          ┌───────────────┼───────────────┐
          ▼               ▼               ▼
  set_tile_pattern  set_tile_pattern_  get_tile_pattern
  (per-line,        buffer              (bulk read,
   keep as-is)       (bulk write,        implement)
                     NEW)
```

### Decision 2: Calling conventions

```
set_tile_pattern:         ; HL = tile number
                          ; DE = line number (0-7)
                          ; B  = bank (0-2 specific, 3 = all)
                          ; C  = byte to write
                          ; → WRTVRM (1 byte)

set_tile_pattern_buffer:  ; A  = tile number
                          ; HL = 8-byte buffer address
                          ; B  = bank (0-2 specific, 3 = all)
                          ; → LDIRVM block copy (8 bytes)

get_tile_pattern:         ; A  = tile number
                          ; HL = 8-byte buffer address (to fill)
                          ; B  = bank (0-2, default 0)
                          ; → LDIRMV block read (8 bytes)
```

### Decision 3: VRAM address calculation

Pattern table base addresses per screen mode:

| Screen Mode | Pattern Table | Bank Offset |
|-------------|--------------|-------------|
| 0           | 0x0800       | No banking (1 bank only) |
| 1           | 0x0800       | Bank 0, +0x0800 per bank |
| 2           | 0x0000       | Bank 0, +0x0800 per bank |
| 3           | 0x0000       | Bank 0, +0x0800 per bank |
| ≥5          | —            | Unsupported (return) |

Formula for VRAM address: `pattern_base + tile × 8 + bank × 0x0800`

In screen 0: `0x0800 + tile × 8` (fixed, only 1 bank)
In screen 1: `0x0800 + tile × 8 + bank × 0x0800`
In screen 2–3: `0x0000 + tile × 8 + bank × 0x0800`

### Decision 4: Bank=3 (all) in bulk mode — 3 LDIRVM passes

For `set_tile_pattern_buffer` with B=3:
1. Compute VRAM address for bank 0 → LDIRVM (8 bytes)
2. Add 0x0800 to VRAM address for bank 1 → LDIRVM (same source buffer)
3. Add 0x0800 for bank 2 → LDIRVM (same source buffer)

For `get_tile_pattern` with B=3: defaults to bank 0 only (reading all banks doesn't make sense).

### Decision 5: 4-integer array = 8 bytes, no packing transformation

A `DIM A%(3)` array occupies 8 contiguous bytes in memory:

```
Memory:  A%(0) lo │ A%(0) hi │ A%(1) lo │ A%(1) hi │ A%(2) lo │ A%(2) hi │ A%(3) lo │ A%(3) hi
Tile:    row 0    │ row 1    │ row 2    │ row 3    │ row 4    │ row 5    │ row 6    │ row 7
```

Because MSX BASIC stores integers little-endian (low byte first), the 8 bytes are already in the correct row order. The kernel routines treat the buffer as 8 raw bytes. No byte swapping or packing is needed.

### Decision 6: Jump table placement

Add new entries in `20_runtime.asm` after the existing `jp get_sprite_pattern`:

```asm
  jp set_tile_pattern_buffer   ; NEW
  jp get_tile_pattern
```

Wait — `get_tile_pattern` already has a `jp` entry that currently points to a stub. We keep that entry and just implement the routine. Only `set_tile_pattern_buffer` is new.

The new entry shifts all subsequent `def_usr*` addresses by 3 bytes. Update `compiler_hooks.h`:

```c
#define def_set_tile_pattern_buffer (def_get_sprite_pattern + 3)
#define def_usr0 (def_set_tile_pattern_buffer + 3)
#define def_usr1 (def_usr0 + 3)
// ... etc
```

### Decision 7: Compiler dispatch logic for SET TILE PATTERN

The parser produces the same `ActionNode` tree structure for both forms. The compiler distinguishes them by examining `action->actions[1]->lexeme->value`:

```
t=2:
├── actions[1] is ARRAY → inline form (l0..l7), bank=3
├── actions[1] is identifier → buffer form, bank=3 (call set_tile_pattern_buffer)

t=3:
├── actions[1] is ARRAY → inline form (l0..l7), bank=actions[2]
├── actions[1] is identifier → buffer form, bank=actions[2]
```

### Decision 8: GET TILE PATTERN with bank parameter

Current GET TILE PATTERN only handles t=2. Add t=3 support:

```
t=2:  GET TILE PATTERN <n>, <array>       → bank=0 (default)
t=3:  GET TILE PATTERN <n>, <array>, <b>  → bank=actions[2]
```

## Screen Mode Validation

The kernel routines check `SCRMOD`:
- Mode ≥5: return (pattern tables not relevant)
- Mode 0: use `0x0800` base, force B=0 (no banking)
- Mode 1: use `0x0800` base with bank offsets
- Mode 2-3: use `0x0000` base with bank offsets

## Risks / Trade-offs

- **Jump table shift**: Adding `set_tile_pattern_buffer` shifts `def_usr0` and subsequent by 3 bytes. Any code hardcoding USR entry addresses will break. Verify no hardcoded addresses exist.
- **LDIRVM interrupt handling**: LDIRVM enables interrupts during transfer. This is safe but may cause slight timing differences vs. WRTVRM loops.
- **Backward compatibility**: The old `SET TILE PATTERN <n>, <buffer>` form (broken) is replaced by `<n>, <4-int-array>`. Programs that used the broken form need updating.
- **Array vs. plain variable**: The compiler should accept any `type_identifier` for the buffer, but document that a DIM'd 4-integer array is required (8 bytes).
