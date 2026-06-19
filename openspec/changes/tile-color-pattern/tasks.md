## 1. Z80 Kernel: New routines

- [ ] 1.1 Implement `set_tile_color_buf` in `31_cmd.asm` — takes A=tile, HL=buffer, B=bank; uses LDIRVM (3 passes for bank=all)
- [ ] 1.2 Implement `get_tile_color` in `31_cmd.asm` — replaces existing stub; takes A=tile, HL=buffer, B=bank; uses LDIRMV
- [ ] 1.3 Add jump table entries in `20_runtime.asm` for `set_tile_color_buf` after existing sprite entries
- [ ] 1.4 Add symbol definitions in `header.symbols.asm` for new routines
- [ ] 1.5 Rebuild kernel (`make` in `src/infrastructure/kernel/asm/`) and update `header.h`
- [ ] 1.6 Add `def_set_tile_color_buf` constant in `compiler_hooks.h` matching the new address

## 2. Compiler: SET TILE COLOR

- [ ] 2.1 Restructure `cmd_set_tile()` COLOR handler to disambiguate syntax variants by actions[1] type
- [ ] 2.2 Handle t==2, actions[1]==ARRAY → FC-only array (bank=3 default), iterate WRTVRM per row
- [ ] 2.3 Handle t==2, actions[1]==identifier → buffer form (bank=3 default), call `def_set_tile_color_buf`
- [ ] 2.4 Handle t==3, actions[1]==ARRAY, actions[2]==ARRAY → FC+BC arrays (bank=3 default) [EXISTING]
- [ ] 2.5 Handle t==3, actions[1]==ARRAY, actions[2]≠ARRAY → FC array + bank, iterate WRTVRM per row
- [ ] 2.6 Handle t==3, actions[1]==identifier → buffer + bank, call `def_set_tile_color_buf`
- [ ] 2.7 Handle t==4, actions[1]==ARRAY, actions[2]==ARRAY → FC+BC arrays + bank [EXISTING]
- [ ] 2.8 Handle t==4, actions[1]≠ARRAY/identifier → fc+bc+bank [EXISTING]

## 3. Compiler: GET TILE COLOR

- [ ] 3.1 Add bank parameter support to `cmd_get_tile()` COLOR handler
- [ ] 3.2 Handle t==2 → A=tile, HL=buffer, B=0 (default bank), call `def_get_tile_color`
- [ ] 3.3 Handle t==3 → A=tile, HL=buffer, B=bank, call `def_get_tile_color`

## 4. Verification

- [ ] 4.1 Assemble kernel and verify new routine addresses match compiler hooks
- [ ] 4.2 Build full ROM with test BASIC sources exercising all 4 syntax forms
- [ ] 4.3 Run in emulator and verify tile color reads/writes produce correct VRAM content
