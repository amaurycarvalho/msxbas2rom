## Context

The MTF (MSX Tile Forge) runtime currently handles palette/tileset loading and full-screen map copies with two coordinate modes (relative col/row and absolute x/y). Parameters are passed via Z80 registers (HL, DE, BC, A), limiting input to 4 values. The compiler handler enforces `action->actions.size() > 4` as a syntax error. An existing integration test (`mtf5.bas`) targets an 8-parameter window-copy operation that does not compile.

Screen pages are unsupported in MTF — the name table is always written to VRAM `0x1800`. On MSX2 with 128KB VRAM, free space above `0x3800` is available for additional name tables.

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
- SCREEN 4 page support (only SCREEN 2 on MSX2 in this change)

## Decisions

### Decision 1: RAM block for all parameters

**Choice:** Store ALL parameters in DAC workarea RAM (0xF7F6–0xF806), read by kernel from RAM only.

**Rationale:** Operation 2 needs 9 parameters — registers alone can't hold them. A unified RAM approach is simpler than hybrid (register+RAM) and avoids conditional code paths in the kernel. The compiler emits `ld (addr), hl`-style writes, same as existing resource load patterns.

**Alternatives considered:**
- Hybrid registers+RAM: backward compat but two code paths in kernel, harder to maintain
- Stack passing: would need custom pop sequences, less readable

### Decision 2: MSX2-only page offset, MSX1 ignored

**Choice:** On MSX2 (`VERSION != 0`), page multiplies the name table VRAM address offset. Page 0 = 0x1800 (default). Page >= 1 = 0x3800 + (page - 1) × 0x400. On MSX1, the page parameter is read but the VRAM address stays 0x1800.

**Rationale:** MSX1 has only 16KB VRAM — no room for extra name tables. MSX2 has 128KB VRAM with ~10KB free above the color table (0x2000–0x37FF). The 0x400 alignment is required by VDP register 2 for name table base addressing. The user is responsible for VDP register reconfiguration to display the target page.

### Decision 3: Compiler emits direct RAM stores

**Choice:** The compiler handler emits Z80 instructions like `ld (MTF_COLX_PARM), de` to write each parameter directly to the DAC workarea. `HL` = resource number, `A` = operation, call `cmd_mtf`. No push/pop stack manipulation needed.

**Rationale:** Simple, explicit, and matches the existing pattern used by other commands (resource load, file handling). The compiler already knows all workarea addresses via `compiler_hooks.h`.

### Decision 4: Kernel reads all params from RAM at entry

**Choice:** `cmd_mtf` reads all parameters from DAC workarea at entry, replacing the register-based `ld (MTF_*_PARM), hl/de/bc/a` with explicit RAM reads via `ld hl, (MTF_COLX_PARM)` etc.

**Rationale:** This is the necessary counterpart to Decision 1. The existing code already writes registers to RAM as its first action — switching to pure RAM reads removes that step and is cleaner.

### Decision 5: Window-copy uses row-table traversal per output row

**Choice:** For operation 2, for each destination row (0 to height-1):
1. Calculate source map row = map_y + current_row
2. Navigate the linked-list row table to find that row
3. Copy `width` bytes starting at `map_x` offset from that row
4. Place at `screen_x + screen_y * 32` offset in the output buffer

**Rationale:** Reuses the existing `cmd_mtf.map_xy.go_to_next_row` linked-list traversal. The output buffer (FONTADDR) holds one complete screen (768 bytes) with only the window region filled; blank areas are zeroed.

### Decision 6: Incremental builds only

**Choice:** Use `make debug` or `make release` directly, never `make all` (which runs `clean` first). The Makefile already has dependency tracking (`-include $(DEP)`).

**Rationale:** Avoids full recompilation. Only modified `.cpp` and their dependency chain are recompiled.

## Risks / Trade-offs

- [RAM block change breaks inline ASM] → No known inline ASM depends on MTF register conventions; all usage goes through `CMD MTF` BASIC statement
- [Page parameter on MSX1 silently ignored] → Acceptable: MSX1 has no free VRAM for extra pages. Users targeting MSX1 should use page=0 (the default)
- [Window bounds validation in kernel] → If width/height exceed map bounds or screen bounds, the kernel will clip (silently) rather than error. BASIC-level validation could be added later via the compiler
- [Kernel size increase] → New window-copy routine adds ~200 bytes. Acceptable within available ROM space
