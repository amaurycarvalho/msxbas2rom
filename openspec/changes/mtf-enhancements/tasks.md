## 1. Kernel Workarea Symbols

- [ ] 1.1 Add `MTF_WIN_W_PARM`, `MTF_WIN_H_PARM`, `MTF_SCR_X_PARM`, `MTF_SCR_Y_PARM`, `MTF_PAGE_PARM` equates in `header/31_cmd.asm` spanning DAC+8 through DAC+16
- [ ] 1.2 Export new symbols in `header.symbols.asm`

## 2. Kernel Runtime — Unified RAM Reading

- [ ] 2.1 Remove register-to-RAM store instructions at `cmd_mtf` entry (`ld (MTF_*), hl/de/bc/a`) — kernel now reads directly from DAC
- [ ] 2.2 Read `MTF_RESN_PARM`, `MTF_COLX_PARM`, `MTF_ROWY_PARM`, `MTF_OPER_PARM` from RAM at point of use in palette/tileset/map dispatch

## 3. Kernel Runtime — Page Offset

- [ ] 3.1 Add MSX2 detection (`ld a,(VERSION); or a; jr z,.msx1`) before VRAM write
- [ ] 3.2 Compute page VRAM address: page=0 → 0x1800; page≥1 → 0x3800 + (page-1)*0x400 on MSX2; always 0x1800 on MSX1
- [ ] 3.3 Apply page offset in `cmd_mtf.map_xy.copy_to_vram` and tileset copy routines

## 4. Kernel Runtime — Window Copy (Operation 2)

- [ ] 4.1 Add `cmd_mtf.window_copy` label and dispatch from operation value (A=2)
- [ ] 4.2 Read `MTF_WIN_W_PARM`, `MTF_WIN_H_PARM`, `MTF_SCR_X_PARM`, `MTF_SCR_Y_PARM`, `MTF_PAGE_PARM` from workarea
- [ ] 4.3 Validate window bounds against map dimensions (clip or skip out-of-bounds)
- [ ] 4.4 Zero-fill output buffer (FONTADDR, 768 bytes) before window copy
- [ ] 4.5 For each destination row (0 to height-1): compute source map row, navigate row-table linked list, copy `width` bytes from `map_x` offset
- [ ] 4.6 Place copied bytes at `(screen_y * 32 + screen_x)` offset in output buffer
- [ ] 4.7 Copy output buffer to VRAM at computed page address via LDIRVM

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
