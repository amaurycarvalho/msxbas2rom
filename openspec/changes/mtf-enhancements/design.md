## Context

The MTF (MSX Tile Forge) runtime currently handles palette/tileset loading and full-screen map copies with two coordinate modes (relative col/row and absolute x/y). Parameters are passed via Z80 registers (HL, DE, BC, A), limiting input to 4 values. The compiler handler enforces `action->actions.size() > 4` as a syntax error. An existing integration test (`mtf5.bas`) targets an 8-parameter window-copy operation that does not compile.

Screen pages are unsupported in MTF — the name table is always written to VRAM `0x1800`. On MSX2 with 128KB VRAM, free space above `0x3800` is available for additional name tables.

**Cross-change dependency**: Real page offset support (computing VRAM addresses from page number) will be implemented by the `set-page-screen4` change, which replaces hardcoded VRAM addresses in `cmd_mtf` with reads from BIOS variables (`GRPNAM`, `GRPCGP`, `GRPCOL`). This change only provides the scaffolding: the `MTF_PAGE_PARM` workarea slot and compiler parameter emission. The kernel dummy ignores the page value and always targets `0x1800`.

## Goals / Non-Goals

**Goals:**
- Add operation 2 (window copy): `MTF <resource>,2,<map_x>,<map_y>,<width>,<height>,<screen_x>,<screen_y>[,<page>]`
- Add page parameter to operations 0 and 1
- Migrate to RAM-block parameter passing for all operations
- Maintain backward compatibility — all existing `.bas` programs compile and run identically
- Unit tests for all parameter combinations; integration test for window copy

**Non-Goals:**
- Map loading performance optimization (deferred)
- New palette/tileset operations (0 stays as palette/tileset load)
- CMD PAGE integration or VDP register manipulation by MTF
- `.mtf.json` project file reader (ResourceMtfReader stub)
- Real page offset computation — deferred to `set-page-screen4` (this change only provides the `MTF_PAGE_PARM` slot and compiler scaffolding; kernel uses dummy page=0 always)

## Decisions

### Decision 1: RAM block for all parameters

**Choice:** Store ALL parameters in DAC workarea RAM (0xF7F6–0xF806), read by kernel from RAM only.

**Rationale:** Operation 2 needs 9 parameters — registers alone can't hold them. A unified RAM approach is simpler than hybrid (register+RAM) and avoids conditional code paths in the kernel. The compiler emits `ld (addr), hl`-style writes, same as existing resource load patterns.

**Alternatives considered:**
- Hybrid registers+RAM: backward compat but two code paths in kernel, harder to maintain
- Stack passing: would need custom pop sequences, less readable

### Decision 2: Page parameter scaffolding — kernel dummy

**Choice:** The compiler accepts the page parameter (position 9) and emits `ld (MTF_PAGE_PARM), a` to the DAC workarea. The kernel defines the `MTF_PAGE_PARM` equate but **ignores the value** — the name table VRAM address remains hardcoded `0x1800` for all page values. This is intentional scaffolding: the `set-page-screen4` change will implement real page support by replacing hardcoded VRAM addresses with BIOS variable reads (`GRPNAM`, `GRPCGP`, `GRPCOL`), at which point the page mechanism becomes dynamic.

**Rationale:** Real page offset computation would involve hardcoded address arithmetic (e.g., `0x1800 + page*0x400`) that would be entirely rewritten by `set-page-screen4`. Providing the parameter slot and compiler pipeline now avoids a two-phase compiler change later. The kernel dummy ensures backward-compatible behavior (all output goes to page 0 at `0x1800`) while the compiler can already generate the full parameter sequence.

### Decision 3: Compiler emits direct RAM stores

**Choice:** The compiler handler emits Z80 instructions like `ld (MTF_COLX_PARM), de` to write each parameter directly to the DAC workarea. `HL` = resource number, `A` = operation, call `cmd_mtf`. No push/pop stack manipulation needed.

**Rationale:** Simple, explicit, and matches the existing pattern used by other commands (resource load, file handling). The compiler already knows all workarea addresses via `compiler_hooks.h`.

### Decision 4: Kernel reads all params from RAM at entry

**Choice:** `cmd_mtf` reads all parameters from DAC workarea at entry, replacing the register-based `ld (MTF_*_PARM), hl/de/bc/a` with explicit RAM reads via `ld hl, (MTF_COLX_PARM)` etc.

**Rationale:** This is the necessary counterpart to Decision 1. The existing code already writes registers to RAM as its first action — switching to pure RAM reads removes that step and is cleaner.

### Decision 5: Window-copy preserves surrounding VRAM via LDIRMV+LDIRVM

**Choice:** For operation 2, the kernel preserves tiles outside the window area using a minimal VDP read-modify-write approach with bulk transfers:

1. Compute VRAM buffer parameters:
   - `VRAM_start = 0x1800 + screen_y * 32 + screen_x`
   - `buffer_size = (height - 1) * 32 + width`
2. **LDIRMV**: read `buffer_size` bytes from VRAM at `VRAM_start` into FONTADDR — captures all tiles within and around the window bounding rectangle (left/right gutters on each row)
3. For each destination row `r` (0 to height-1):
   - Navigate the linked-list row table to source map row `map_y + r` (one `go_to_next_row` per iteration)
   - Skip the 3-byte linked-list header, advance `map_x` bytes into the row data
   - Copy `width` bytes via `ldir` to FONTADDR at offset `32 * r` — window data is always 32-byte aligned in the buffer regardless of `screen_x`, because the buffer starts mid-row at column `screen_x`
   - Advance to next map row via `go_to_next_row`
4. **LDIRVM**: write the modified `buffer_size` bytes from FONTADDR back to VRAM at `VRAM_start`

**VDP transactions: 1 LDIRMV + 1 LDIRVM = 2 total**, independent of window height.

**Buffer layout rationale:** Because the buffer starts at column `screen_x` of the first window row and VRAM rows are contiguous (32 bytes each), the window data for row `r` always lands at offset `32 * r`:
```
FONTADDR layout (screen_x=5, width=10, height=6, buffer_size=170):
  offset 0..9:    row 0 window cols 5..14           ← overwritten with map data
  offset 10..26:  row 0 cols 15..31 (preserved)      ← untouched from LDIRMV
  offset 27..31:  row 1 cols 0..4   (preserved)      ← untouched from LDIRMV
  offset 32..41:  row 1 window cols 5..14           ← overwritten with map data
  offset 42..58:  row 1 cols 15..31 (preserved)
  ...
  offset 160..169: row 5 window cols 5..14           ← overwritten with map data
  (cols 15..31 on row 5 are NOT in buffer — beyond LDIRVM range, untouched)
```
This means the destination offset per row is simply `32 * r`, which requires fewer register operations than `(screen_y + r) * 32 + screen_x`.

**What gets preserved:**
- Tiles **above** the window (rows 0..screen_y-1): never touched
- Tiles **below** the window (rows screen_y+height..23): never touched
- Tiles to the **left** of the window (cols 0..screen_x-1): read by LDIRMV, re-written unchanged by LDIRVM
- Tiles to the **right** on rows screen_y..screen_y+height-2: read and re-written unchanged
- Tiles to the **right** on the last window row: never read, never touched

**Alternatives considered:**
- Zero-fill FONTADDR (768 bytes) + full-screen LDIRVM: simpler (1 VDP transfer) but destroys all tiles outside the window — rejected because HUD overlays and dialog boxes need surrounding tiles preserved
- Per-row LDIRMV+LDIRVM (2 × height VDP transactions): preserves VRAM but each VDP address setup costs cycles — rejected; 2 bulk transfers are far faster than 2 × height small transfers
- No LDIRMV, fill FONTADDR with map data + explicit zero/pattern fill of gutters: preserves VRAM above/below but destroys left/right tiles on window rows — rejected (incomplete preservation)

### Decision 6: Incremental builds only

**Choice:** Use `make debug` or `make release` directly, never `make all` (which runs `clean` first). The Makefile already has dependency tracking (`-include $(DEP)`).

**Rationale:** Avoids full recompilation. Only modified `.cpp` and their dependency chain are recompiled.

## Risks / Trade-offs

- [RAM block change breaks inline ASM] → No known inline ASM depends on MTF register conventions; all usage goes through `CMD MTF` BASIC statement
- [Page parameter on MSX1 silently ignored] → Acceptable: MSX1 has no free VRAM for extra pages. Users targeting MSX1 should use page=0 (the default)
- [Page parameter is dummy in kernel] → All MTF output goes to 0x1800 regardless of page value. Real page support requires `set-page-screen4` to be implemented. Documented in proposal.
- [Window bounds validation in kernel] → If width/height exceed screen bounds, the kernel will clip (silently). Screen coordinates are clamped to 0..31 (x) and 0..23 (y), and width/height are reduced to fit. Clipping happens before buffer size calculation so the formula remains valid.
- [Window fully off-screen] → If screen_x ≥ 32, screen_y ≥ 24, or clipped width/height ≤ 0, the kernel performs no VDP operations (early return). The LDIRMV+LDIRVM are skipped entirely.
- [Kernel size increase] → New window-copy routine adds ~250 bytes (vs original ~200 byte estimate). The extra ~50 bytes cover the LDIRMV/LDIRVM setup, buffer size computation, and screen coordinate clipping. Acceptable within available ROM space.
- [Buffer size calculation correctness] → The formula `(height-1)*32 + width` assumes VRAM rows are contiguous (32 bytes each) and the buffer starts at `screen_x` within the first row. Verified correct for all screen_x/width/height combinations within screen bounds. Maximum buffer size = 768 bytes (full screen), which fits in existing FONTADDR.
