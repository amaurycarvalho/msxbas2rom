## Context

SET TILE COLOR currently has two paths:

1. **FC/BC form** (`n, fc, bc [, bank]` and array variants) — works correctly. Packs FC<<4|BC per row, calls `set_tile_color` kernel routine with HL=tile, DE=line, B=bank, C=byte. 

2. **Buffer form** (`n, buffer`) — broken. Compiler generates `A=tile, HL=buffer` and calls `def_set_tile_color`, but the kernel routine expects `HL=tile, DE=line, B=bank, C=byte`. Calling convention mismatch.

GET TILE COLOR has the buffer form (`n, buffer`) but the kernel routine is a stub (`ret` only).

Neither form supports an explicit bank parameter for the buffer syntax.

The color table in VRAM (screen 2+) is 8 bytes per tile, one byte per row, format `FC<<4 | BC`.

## Goals / Non-Goals

**Goals:**
- Implement all 4 SET TILE COLOR syntaxes
- Implement GET TILE COLOR with bank parameter
- Buffer uses 4 integer array (4 × 16-bit = 8 bytes), each byte = FC<<4 | BC
- Use LDIRVM/LDIRMV for efficient block transfer
- Fix buffer form calling convention
- Add unit tests in `tests/unit/src/test_compiler.cpp` covering all syntax forms (following existing pattern from SPRITE COLOR / TILE PATTERN tests)
- Add integration test `tests/integration/GRAPH/test97.bas` covering all syntaxes (following test94.bas pattern)

**Non-Goals:**
- SET/GET TILE PATTERN (separate change)
- Other tile color formats (e.g., per-pixel color)

## Decisions

### Decision 1: New kernel routines for buffer form
Replace the stubs at `get_tile_color` / `get_tile_pattern` with real implementations. Add a new `set_tile_color_buf` routine specifically for the buffer form.

Calling conventions:

```
set_tile_color_buf:    ; A  = tile number
                       ; HL = 8-byte buffer (packed FC<<4|BC per byte)
                       ; B  = bank (0-2 specific, 3 = all 3 banks)
                       ; Uses LDIRVM: HL=source, DE=VRAM dest, BC=8

get_tile_color:        ; A  = tile number
                       ; HL = 8-byte buffer
                       ; B  = bank (0-2)
                       ; Uses LDIRMV: DE=buffer dest, HL=VRAM source, BC=8
```

### Decision 2: LDIRVM/LDIRMV with 3 passes for bank=all
VRAM color table layout per bank:
- Bank 0: 0x2000 + tile×8
- Bank 1: 0x2800 + tile×8  
- Bank 2: 0x3000 + tile×8

For SET with B=3 (all banks), the routine loops 3 times, incrementing VRAM address by 0x800 each time, reusing the same source buffer.

For GET, only one bank is read (default 0).

### Decision 3: Compiler restructured to disambiguate syntax forms
The same `t` value (parameter count) can map to different syntaxes. The compiler must check `actions[1]` type:

```
t=2:
├── actions[1] is ARRAY → Syntax 2 (FC array only, bank=3 default)
└── actions[1] is identifier → Syntax 3 (buffer, bank=3 default)

t=3:
├── actions[1] is ARRAY, actions[2] is ARRAY → Syntax 2 (FC+BC arrays, bank=3)
├── actions[1] is ARRAY, actions[2] is num → Syntax 2 (FC array + bank)
└── actions[1] is identifier → Syntax 3 (buffer + bank)

t=4:
├── actions[1] is ARRAY, actions[2] is ARRAY → Syntax 2 (FC+BC arrays + bank)
└── actions[1] is FC, actions[2] is BC → Syntax 1 (fc+bc+bank)
```

### Decision 4: Buffer format — 4 integers × 16-bit = 8 bytes
Each MSX-BASIC integer is stored as 2 bytes (little-endian). A 4-integer array occupies 8 contiguous bytes, matching the 8-byte tile color table. Each byte = FC<<4 | BC.

Layout:
```
Integer 0: low byte = row 0 color, high byte = row 1 color
Integer 1: low byte = row 2 color, high byte = row 3 color
Integer 2: low byte = row 4 color, high byte = row 5 color
Integer 3: low byte = row 6 color, high byte = row 7 color
```

The kernel routines treat the buffer as 8 raw bytes, ignoring the integer boundaries.

### Decision 5: Jump table entries
Replace the existing stubs in the runtime jump table (`20_runtime.asm`) with JPs to the new implementations. Add `set_tile_color_buf` as a new entry after `get_sprite_pattern`.

Corresponding `compiler_hooks.h` constants computed as offsets from `def_wrapper_routines_map_start`.

### Decision 6: Unit test pattern
Unit tests follow the existing `compileStatementProgram` helper pattern used by SPRITE COLOR and TILE PATTERN tests. Each syntax variant is a `SUBCASE` inside `TEST_CASE("All statement strategies execute successfully")`. The setup already tests compile-only success/failure and optionally checks error messages. No VRAM/hardware involved.

New test cases to add inside the existing `StatementCase cases[]` array at `tests/unit/src/test_compiler.cpp`:
- `SET_TILE_COLOR_FC_BC` — simplest form
- `SET_TILE_COLOR_FC_BC_BANK` — with bank
- `SET_TILE_COLOR_FC_ARRAY` — per-row FC array
- `SET_TILE_COLOR_FC_BC_ARRAYS` — FC+BC arrays
- `SET_TILE_COLOR_FC_ARRAY_BANK` — FC array + bank
- `SET_TILE_COLOR_BUFFER` — buffer form
- `SET_TILE_COLOR_BUFFER_BANK` — buffer + bank
- `GET_TILE_COLOR` — buffer (default bank)
- `GET_TILE_COLOR_BANK` — buffer + bank

### Decision 7: Integration test pattern
`test97.bas` follows the same structure as `test94.bas` (tile pattern integration test). It will:
- Set screen 2, enable tile mode
- Exercise all SET TILE COLOR syntax forms (fc/bc, arrays, buffer, with/without bank)
- Exercise both GET TILE COLOR forms (default bank, explicit bank)
- Use `PUT TILE` to display tiles on screen for visual verification
- Print hex values of read-back color data via `PRINT HEX$()`
- Wait for key press between operations to allow visual inspection

## Risks / Trade-offs

- **LDIRVM/LDIRMV** uses BIOS call interruptions → safe but slightly slower than WRTVDM loop. However, block transfer is much faster than 8 individual WRTVRM calls.
- **Jump table shift**: Adding a new entry shifts all subsequent addresses. Must update compiler_hooks.h accordingly.
- **Screen mode check**: Current routines check SCRMOD and bail out for modes 0 and ≥5. New routines must do the same to avoid VRAM corruption.
- **Unit test coverage**: Compiler-only tests verify translation but not runtime behavior. Integration tests fill this gap.
- **Integration test fragility**: `test97.bas` depends on screen mode 2 and tile mode being on — must match setup exactly. Following `test94.bas` pattern mitigates this.
