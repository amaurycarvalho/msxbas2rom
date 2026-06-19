## 1. Parser — Add optional bank parameter

- [ ] 1.1 Update `parseSetTileColpat()` to accept 3rd optional bank parameter (tile, dir, bank)
- [ ] 1.2 Add parser unit tests in `tests/unit/src/test_parser.cpp`

## 2. Compiler — Pass bank parameter in B register

- [ ] 2.1 Update TILE FLIP compiler: if 2 params → `LD B, 3`; if 3 params → `LD B, <bank>`
- [ ] 2.2 Update TILE ROTATE compiler: same bank handling
- [ ] 2.3 Add compiler unit tests in `tests/unit/src/test_compiler.cpp`

## 3. Assembly — Implement tile flip/rotate routines

- [ ] 3.1 Implement `set_tile_flip` in `31_cmd.asm` (horizontal, vertical, both)
- [ ] 3.2 Implement `set_tile_rotate` in `31_cmd.asm` (left, right, 180)
- [ ] 3.3 Add helper: `get_tile_vram_addr` (compute VRAM address from tile number + screen mode)
- [ ] 3.4 Handle bank loop (single bank or all 3 banks)

## 4. Integration Test

- [ ] 4.1 Create `tests/integration/GRAPH/tile_flip_rotate.bas` testing all flip + rotate directions
