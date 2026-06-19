## 1. Parser — Add optional bank parameter

- [x] 1.1 Update `parseSetTileColpat()` to accept 3rd optional bank parameter (tile, dir, bank)
- [x] 1.2 Add parser unit tests in `tests/unit/src/test_parser.cpp`

## 2. Compiler — Pass bank parameter in B register

- [x] 2.1 Update TILE FLIP compiler: if 2 params → `LD B, 3`; if 3 params → `LD B, <bank>`
- [x] 2.2 Update TILE ROTATE compiler: same bank handling
- [x] 2.3 Add compiler unit tests in `tests/unit/src/test_compiler.cpp`

## 3. Assembly — Implement tile flip/rotate routines

- [x] 3.1 Implement `set_tile_flip` in `31_cmd.asm` (horizontal, vertical, both)
- [x] 3.2 Implement `set_tile_rotate` in `31_cmd.asm` (left, right, 180)
- [x] 3.3 Add helper: `get_tile_vram_addr` (compute VRAM address from tile number + screen mode)
- [x] 3.4 Handle bank loop (single bank or all 3 banks)

## 4. Integration Test

- [x] 4.1 Create `tests/integration/GRAPH/tile_flip_rotate.bas` testing all flip + rotate directions

## 5. Verification Fix

- [x] 5.1 Fix direction clobber bug: save HL (direction) on stack before VRAM operations in both set_tile_flip and set_tile_rotate
