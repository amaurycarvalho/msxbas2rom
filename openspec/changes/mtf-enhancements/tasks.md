## 1. Kernel Workarea Symbols

- [ ] 1.1 Add `MTF_WIN_W_PARM`, `MTF_WIN_H_PARM`, `MTF_SCR_X_PARM`, `MTF_SCR_Y_PARM`, `MTF_PAGE_PARM` equates in `header/31_cmd.asm` spanning DAC+8 through DAC+16
- [ ] 1.2 Export new symbols in `header.symbols.asm`

## 2. Kernel Runtime — Unified RAM Reading

- [ ] 2.1 Remove register-to-RAM store instructions at `cmd_mtf` entry (`ld (MTF_*), hl/de/bc/a`) — kernel now reads directly from DAC
- [ ] 2.2 Read `MTF_RESN_PARM`, `MTF_COLX_PARM`, `MTF_ROWY_PARM`, `MTF_OPER_PARM` from RAM at point of use in palette/tileset/map dispatch

## 3. Kernel Runtime — Page Parameter Scaffolding (Dummy)

- [ ] 3.1 Kernel ignores `MTF_PAGE_PARM` — all name table writes continue to use hardcoded `0x1800`
- [ ] 3.2 Add comment in `cmd_mtf.map_xy.copy_to_vram` noting real page support is deferred to `set-page-screen4` (will replace `0x1800` with `(GRPNAM)`)
- [ ] 3.3 Verify tileset copy routines also remain unchanged (use hardcoded `0x0000`/`0x0800`/`0x1000` for patterns, `0x2000`/`0x2800`/`0x3000` for colors — these will be replaced by `set-page-screen4` with `(GRPCGP)`/`(GRPCOL)` offsets)

## 4. Kernel Runtime — Window Copy (Operation 2)

- [ ] 4.1 Add `cmd_mtf.window_copy` label and dispatch from operation value (`cp 2` / `jr z, window_copy`)
- [ ] 4.2 Read `MTF_WIN_W_PARM`, `MTF_WIN_H_PARM`, `MTF_SCR_X_PARM`, `MTF_SCR_Y_PARM`, `MTF_COLX_PARM` (map_x), `MTF_ROWY_PARM` (map_y) from workarea into registers
- [ ] 4.3 Clip screen coordinates: `screen_x = max(0, min(screen_x, 31))`, `screen_y = max(0, min(screen_y, 23))`. Clip window: `width = min(width, 32 - screen_x)`, `height = min(height, 24 - screen_y)`. Write clipped values back to workarea
- [ ] 4.4 Early return if clipped width ≤ 0 or height ≤ 0 — no VDP operations, just close resource and `ret`
- [ ] 4.5 Compute VRAM buffer: `vram_start = 0x1800 + screen_y * 32 + screen_x`, `buffer_size = (height - 1) * 32 + width`. Store `buffer_size` in BC for LDIRMV/LDIRVM
- [ ] 4.6 **LDIRMV**: read `buffer_size` bytes from VRAM at `vram_start` into FONTADDR — this captures surrounding tiles on window rows (left/right gutters preserved)
- [ ] 4.7 Navigate to first source map row: `hl = MTF_MAP_1ST_ROW + map_y * 3`, then `call go_to_next_row`
- [ ] 4.8 Loop r = 0 to height-1:
  - Skip 3-byte linked-list header (`inc hl` × 3)
  - Advance `map_x` bytes into row data (`add hl, de` with de=map_x)
  - Copy `width` bytes via `ldir` to `FONTADDR + 32 * r`
  - Restore destination pointer: `de = FONTADDR + 32 * (r + 1)` (or `ex de,hl` / `ld de, 32` / `add hl, de` equivalent)
  - `call go_to_next_row` to advance to next source map row
- [ ] 4.9 **LDIRVM**: write `buffer_size` bytes from FONTADDR back to VRAM at `vram_start`
- [ ] 4.10 Close resource (`call resource.close`), `ei`, and `ret`
- [ ] 4.11 Add `; real page support deferred to set-page-screen4 — will replace 0x1800 with (GRPNAM)` comment at the VRAM address usage points

## 5. Compiler Handler — Parameter Count Extension

- [ ] 5.1 Change max parameter check from `> 4` to `> 9` in `compiler_cmd_mtf_handler.cpp`
- [ ] 5.2 Validate minimum parameter count per operation (palette/tileset: 1–2, map op 0/1: 1–5, map op 2: 8–9)

## 6. Compiler Handler — RAM Block Emission

- [ ] 6.1 Replace register-based parameter passing with `ld (addr), hl` emits to DAC workarea offsets
- [ ] 6.2 Emit `xor a` / `ld (MTF_PAGE_PARM), a` as default for all omitted parameters (page=0, window=0)
- [ ] 6.3 Handle all 9 parameter positions: resource, op, col_x/map_x, row_y/map_y, width, height, screen_x, screen_y, page
- [ ] 6.4 Update kernel address constant `def_cmd_mtf` in `compiler_hooks.h` if the runtime grows past the 3-byte jump slot

## 7. Unit Tests

- [ ] 7.1 Add compiler test cases for all parameter counts 1–9 in `tests/unit/src/test_compiler.cpp`
- [ ] 7.2 Add compiler test cases for invalid parameter counts (0, 10+) expecting syntax error
- [ ] 7.3 Add compiler test cases verifying emitted opcodes write to correct DAC offsets
- [ ] 7.4 Run `make -C tests/unit run` to verify all tests pass

## 8. Integration Test

- [ ] 8.1 Verify `tests/integration/mtf/mtf5.bas` compiles successfully with the updated compiler
- [ ] 8.2 Confirm the existing `tests/integration/MTF/` tests (mtf1–mtf4) still compile and produce identical ROM output
- [ ] 8.3 Run `make -C tests/integration run` to verify all integration tests pass

## 9. Build Verification

- [ ] 9.1 Run `make release` (incremental) and confirm clean compilation
- [ ] 9.2 Verify `make debug` also compiles without forced clean
- [ ] 9.3 Confirm no regression in `make test-unit`
