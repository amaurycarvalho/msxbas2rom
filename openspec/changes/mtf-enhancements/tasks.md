## 1. Kernel Workarea Symbols

- [x] 1.1 Add `MTF_WIN_W_PARM`, `MTF_WIN_H_PARM`, `MTF_SCR_X_PARM`, `MTF_SCR_Y_PARM`, `MTF_PAGE_PARM` equates in `header/31_cmd.asm` spanning PARM1+8 through PARM1+16
- [x] 1.2 Export new symbols in `header.symbols.asm`

## 2. Kernel Runtime — Unified RAM Reading

- [x] 2.1 Remove register-to-RAM store instructions at `cmd_mtf` entry (`ld (MTF_*), hl/de/bc/a`) — kernel now reads directly from PARM1 workarea
- [x] 2.2 Read `MTF_RESN_PARM`, `MTF_COLX_PARM`, `MTF_ROWY_PARM`, `MTF_OPER_PARM` from RAM at point of use in palette/tileset/map dispatch

## 3. Kernel Runtime — Page Parameter Scaffolding (Dummy)

- [x] 3.1 Kernel ignores `MTF_PAGE_PARM` — all name table writes continue to use hardcoded `0x1800`
- [x] 3.2 Add comment in `cmd_mtf.window_copy` VRAM address usage noting real page support is deferred to `set-page-screen4` (will replace `0x1800` with `(GRPNAM)`)
- [x] 3.3 Verify tileset copy routines also remain unchanged (use hardcoded `0x0000`/`0x0800`/`0x1000` for patterns, `0x2000`/`0x2800`/`0x3000` for colors — these will be replaced by `set-page-screen4` with `(GRPCGP)`/`(GRPCOL)` offsets)

## 4. Kernel Runtime — Window Copy (Operation 2)

- [x] 4.1 Add `cmd_mtf.window_copy` label and dispatch from operation value (`cp 2` / `jr z, window_copy`)
- [x] 4.2 Read `MTF_WIN_W_PARM`, `MTF_WIN_H_PARM`, `MTF_SCR_X_PARM`, `MTF_SCR_Y_PARM`, `MTF_COLX_PARM` (map_x), `MTF_ROWY_PARM` (map_y) from workarea into registers
- [x] 4.3 Clip screen coordinates: `screen_x = min(screen_x, 31)`, `screen_y = min(screen_y, 23)` (upper-bound only; negative values are not clamped to 0). Clip window: `width = min(width, 32 - screen_x)`, `height = min(height, 24 - screen_y)`. Write clipped values back to workarea
- [x] 4.4 Early return if clipped width ≤ 0 or height ≤ 0 — no VDP operations, just close resource and `ret`
- [x] 4.5 Compute VRAM buffer: `vram_start = 0x1800 + screen_y * 32 + screen_x`, `buffer_size = (height - 1) * 32 + width`. Store `buffer_size` in BC for LDIRMV/LDIRVM
- [x] 4.6 **LDIRMV**: read `buffer_size` bytes from VRAM at `vram_start` into FONTADDR — this captures surrounding tiles on window rows (left/right gutters preserved)
- [x] 4.6a **Optimization**: skip LDIRMV when `screen_x == 0 && width == 32` (full-width rows — all bytes read would be overwritten by map data). Branch around the LDIRMV with a conditional check (~6 bytes of code). Full-screen copies (ops 0/1 delegation) benefit: only 1 VDP transfer instead of 2
- [x] 4.7 Navigate to first source map row: `hl = MTF_MAP_1ST_ROW + map_y * 3`, then `call go_to_next_row`
- [x] 4.8 Loop r = 0 to height-1:
  - Skip 3-byte linked-list header (`inc hl` × 3)
  - Advance `map_x` bytes into row data (`add hl, de` with de=map_x)
  - Copy `width` bytes via `ldir` to `FONTADDR + 32 * r`
  - Restore destination pointer: `de = FONTADDR + 32 * (r + 1)` (or `ex de,hl` / `ld de, 32` / `add hl, de` equivalent)
  - `call go_to_next_row` to advance to next source map row
- [x] 4.9 **LDIRVM**: write `buffer_size` bytes from FONTADDR back to VRAM at `vram_start`
- [x] 4.10 Close resource (`call resource.close`), `ei`, and `ret`
- [x] 4.11 Add `; real page support deferred to set-page-screen4 — will replace 0x1800 with (GRPNAM)` comment at the VRAM address usage points

## 5. Kernel Runtime — Unify Operations 0/1 as Window-Copy Delegates

- [x] 5.1 After computing map_x/map_y (existing coordinate wrapping and modulo logic), write `MTF_COLX_PARM` and `MTF_ROWY_PARM` with computed values
- [x] 5.2 Use `MTF_WIN_W_PARM = 32` and `MTF_WIN_H_PARM = 24` already set by the compiler as defaults (full-screen dimensions)
- [x] 5.3 Use `MTF_SCR_X_PARM = 0` and `MTF_SCR_Y_PARM = 0` already set by the compiler as defaults (top-left screen origin)
- [x] 5.4 Replace the 24-iteration `copy_to_buffer.loop` (ldir × 32 per row) — code now falls through to `cmd_mtf.window_copy`
- [x] 5.5 Remove `cmd_mtf.map_xy.copy_to_buffer`, `cmd_mtf.map_xy.copy_to_buffer.loop`, `cmd_mtf.map_xy.copy_to_vram` labels and their code (~70 bytes eliminated)
- [x] 5.6 Verify `cmd_mtf.map_xy.search_row_table` and `go_to_next_row` are preserved (window_copy reuses them)

## 6. Compiler Handler — Parameter Count Extension

- [x] 6.1 Change max parameter check from `> 4` to `> 9` in `compiler_cmd_mtf_handler.cpp`
- [x] 6.2 Validate minimum parameter count per operation (palette/tileset: 1–2, map op 0/1: 1–5, map op 2: 8–9)

## 7. Compiler Handler — RAM Block Emission

- [x] 7.1 Replace register-based parameter passing with `ld (addr), hl` emits to PARM1 workarea offsets
- [x] 7.2 Emit `xor a` / `ld (MTF_PAGE_PARM), a` as default for all omitted parameters (page=0, window=0)
- [x] 7.3 Handle all 9 parameter positions: resource, op, col_x/map_x, row_y/map_y, width, height, screen_x, screen_y, page
- [x] 7.4 Update kernel address constant `def_cmd_mtf` in `compiler_hooks.h` if the runtime grows past the 3-byte jump slot

## 8. Unit Tests

- [x] 8.1 Add compiler test cases for all parameter counts 1–9 in `tests/unit/src/test_compiler.cpp`
- [x] 8.2 Add compiler test cases for invalid parameter counts (0, 10+) expecting syntax error
- [x] 8.3 Add compiler test cases verifying emitted opcodes write to correct PARM1 offsets
- [x] 8.4 Run `make -C tests/unit run` to verify all tests pass

## 9. Integration Test

- [x] 9.1 Verify `tests/integration/MTF/mtf5.bas` compiles successfully with the updated compiler
- [x] 9.2 Confirm the existing `tests/integration/MTF/` tests (mtf1–mtf4) still compile and produce identical ROM output
- [x] 9.3 Run `make -C tests/integration run` to verify all integration tests pass

## 10. Build Verification

- [x] 10.1 Run `make release` (incremental) and confirm clean compilation
- [x] 10.2 Verify `make debug` also compiles without forced clean
- [x] 10.3 Confirm no regression in `make test-unit`
