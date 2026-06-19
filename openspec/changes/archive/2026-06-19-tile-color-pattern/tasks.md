## 1. Z80 Kernel: New routines

- [x] 1.1 Implement `set_tile_color_buf` in `31_cmd.asm` — takes A=tile, HL=buffer, B=bank; uses LDIRVM (3 passes for bank=all)
- [x] 1.2 Implement `get_tile_color` in `31_cmd.asm` — replaces existing stub; takes A=tile, HL=buffer, B=bank; uses LDIRMV
- [x] 1.3 Add jump table entries in `20_runtime.asm` for `set_tile_color_buf` after existing sprite entries
- [x] 1.4 Add symbol definitions in `header.symbols.asm` for new routines
- [x] 1.5 Rebuild kernel (`make` in `src/infrastructure/kernel/asm/`) and update `header.h`
- [x] 1.6 Add `def_set_tile_color_buf` constant in `compiler_hooks.h` matching the new address

## 2. Compiler: SET TILE COLOR

- [x] 2.1 Restructure `cmd_set_tile()` COLOR handler to disambiguate syntax variants by actions[1] type
- [x] 2.2 Handle t==2, actions[1]==ARRAY → FC-only array (bank=3 default), iterate WRTVRM per row
- [x] 2.3 Handle t==2, actions[1]==identifier → buffer form (bank=3 default), call `def_set_tile_color_buf`
- [x] 2.4 Handle t==3, actions[1]==ARRAY, actions[2]==ARRAY → FC+BC arrays (bank=3 default)
- [x] 2.5 Handle t==3, actions[1]==ARRAY, actions[2]≠ARRAY → FC array + bank, iterate WRTVRM per row
- [x] 2.6 Handle t==3, actions[1]==identifier → buffer + bank, call `def_set_tile_color_buf`
- [x] 2.7 Handle t==4, actions[1]==ARRAY, actions[2]==ARRAY → FC+BC arrays + bank
- [x] 2.8 Handle t==4, actions[1]≠ARRAY/identifier → fc+bc+bank

## 3. Compiler: GET TILE COLOR

- [x] 3.1 Add bank parameter support to `cmd_get_tile()` COLOR handler
- [x] 3.2 Handle t==2 → A=tile, HL=buffer, B=0 (default bank), call `def_get_tile_color`
- [x] 3.3 Handle t==3 → A=tile, HL=buffer, B=bank, call `def_get_tile_color`

## 4. Unit Tests (tests/unit/src/test_compiler.cpp)

- [x] 4.1 Add test case for `SET_TILE_COLOR_FC_BC` — simplest fc/bc form
- [x] 4.2 Add test case for `SET_TILE_COLOR_FC_BC_BANK` — fc/bc with bank parameter
- [x] 4.3 Add test case for `SET_TILE_COLOR_FC_ARRAY` — per-row FC array (no BC)
- [x] 4.4 Add test case for `SET_TILE_COLOR_FC_BC_ARRAYS` — per-row FC and BC arrays
- [x] 4.5 Add test case for `SET_TILE_COLOR_FC_ARRAY_BANK` — FC array + bank
- [x] 4.6 Add test case for `SET_TILE_COLOR_BUFFER` — buffer form (4-integer array)
- [x] 4.7 Add test case for `SET_TILE_COLOR_BUFFER_BANK` — buffer form + bank
- [x] 4.8 Add test case for `GET_TILE_COLOR` — buffer form (default bank 0)
- [x] 4.9 Add test case for `GET_TILE_COLOR_BANK` — buffer form with explicit bank
- [x] 4.10 Add test case for invalid syntax forms (wrong parameter count)
- [x] 4.11 Build and run unit tests — all 163 test cases pass (1418 assertions)

## 5. Integration Test (tests/integration/GRAPH/test97.bas)

- [x] 5.1 Create `test97.bas` with screen 2 setup and tile mode enabled, following test94.bas structure
- [x] 5.2 Add SET TILE COLOR inline fc/bc forms (with and without bank) + PUT TILE for visual display
- [x] 5.3 Add SET TILE COLOR per-row array forms (FC only, FC+BC arrays, with and without bank)
- [x] 5.4 Add SET TILE COLOR buffer array form (with and without bank)
- [x] 5.5 Add GET TILE COLOR buffer read-back (default bank 0 and explicit bank 2)
- [x] 5.6 Print hex values of read-back color data via PRINT HEX$() for visual verification
- [x] 5.7 Add key-press pauses between sections for step-by-step emulator inspection
- [x] 5.8 Verify program compiles successfully in integration test build

## 6. Verification

- [x] 6.1 Assemble kernel and verify new routine addresses match compiler hooks
- [x] 6.2 Build full ROM and run unit tests — all tests green
- [x] 6.3 Build full ROM with integration test — test97.bas compiles successfully (emulator verification pending manual testing)
