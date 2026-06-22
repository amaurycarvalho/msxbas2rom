## 1. Baseline and Preparation

- [ ] 1.1 Build current kernel and capture `header.bin` and `header.symbols.asm` as pre-restructure baseline
- [ ] 1.2 Capture list of all 279 `cpu.addCall(def_xxx)` call sites for migration tracking (116 files)

## 2. ASM-Side: Wrapper Table Restructure

- [ ] 2.1 Count the 126 `jp` entries in `20_runtime.asm` to verify exact count
- [ ] 2.2 Replace 126-entry `jp` table with `dw` word-pointer table in `20_runtime.asm` preserving exact order
- [ ] 2.3 Add dispatcher routine at `wrapper_routines_map_start` — loads table offset from HL, fetches word pointer, jumps via `jp (hl)`
- [ ] 2.4 Remove 5-byte NOP padding between `jp 0x8010` and the wrapper table (dispatcher fills this space)
- [ ] 2.5 Build kernel and verify: `wrapper_routines_map_start` still at 0x4102, `header.bin` is 16384 bytes, dispatcher + table < 378 bytes
- [ ] 2.6 Verify `header.symbols.asm` — all wrapper routine symbols still resolve correctly

## 3. C-Side: Headers and Helpers

- [ ] 3.1 Update `compiler_hooks.h`: replace `+3` address chain with `DISP_` prefix 0-based index constants (e.g., `#define DISP_cmd_clrkey 1`)
- [ ] 3.2 Keep `def_wrapper_routines_map_start` at 0x4102; keep `def_castParamFloatInt` as alias for backward compat during migration
- [ ] 3.3 Add `addKernelDispatch(uint8_t index)` method to `CompilerCodeOptimizer` (or `Z80OpcodeWriter`) — emits `ld hl, index*2; call 0x4102`
- [ ] 3.4 Update `compiler_code_optimizer.getKernelCallAddr()` to resolve through `dw` table entries instead of checking for `0xC3` (jp) opcode

## 4. C-Side: Call Site Migration — Integer/Float Ops

- [ ] 4.1 Update `compiler_expression_evaluator.cpp` — replace `optimizer.addKernelCall(def_intCompare*)` and `cpu.addCall(def_floatNEG)` etc. with dispatch
- [ ] 4.2 Update `compiler_for_statement_strategy.cpp` — replace `optimizer.addKernelCall(def_intCompareGT)` with dispatch
- [ ] 4.3 Update integer shift operations (`def_intSHR`, `def_intSHL`) in expression evaluator

## 5. C-Side: Call Site Migration — Command Handlers

- [ ] 5.1 Update all `src/application/compiler/statements/strategies/io/cmd/handlers/graphics/*/` files — `def_cmd_clrscr`, `def_cmd_wrtchr`, `def_cmd_wrtclr`, `def_cmd_wrtfnt`, `def_cmd_wrtscr`, `def_cmd_wrtspr`, etc.
- [ ] 5.2 Update `src/application/compiler/statements/strategies/io/cmd/handlers/io/clrkey/` — `def_cmd_clrkey`
- [ ] 5.3 Update `src/application/compiler/statements/strategies/io/cmd/handlers/control/` files — `def_cmd_disscr`, `def_cmd_enascr`, `def_cmd_keyclkoff`, `def_cmd_turbo`, `def_cmd_page`, `def_cmd_runasm`
- [ ] 5.4 Update remaining command handler files — `def_cmd_play`, `def_cmd_draw`, `def_cmd_mute`, `def_cmd_pad`, `def_cmd_restore`

## 6. C-Side: Call Site Migration — Player and Sound

- [ ] 6.1 Update player-related files — `def_cmd_plyload`, `def_cmd_plyloop`, `def_cmd_plymute`, `def_cmd_plyplay`, `def_cmd_plyreplay`, `def_cmd_plysong`, `def_cmd_plysound`
- [ ] 6.2 Update `def_player_initialize` and `def_player_unhook` call sites

## 7. C-Side: Call Site Migration — File I/O

- [ ] 7.1 Update file handling files — `def_cmd_fmaxfiles`, `def_cmd_fdskf`, `def_cmd_feof`, `def_cmd_floc`, `def_cmd_flof`, `def_cmd_fpos`, `def_cmd_fopen`, `def_cmd_fclose`, `def_cmd_finput`, `def_cmd_fprint`
- [ ] 7.2 Update `def_cmd_preflight_disk` call sites

## 8. C-Side: Call Site Migration — XBASIC and Functions

- [ ] 8.1 Update XBASIC routine call sites — `def_XBASIC_BASE`, `def_XBASIC_CLS`, `def_XBASIC_COPY`, `def_XBASIC_END`, `def_XBASIC_INIT`, `def_XBASIC_LOCATE`, `def_XBASIC_PRINT_STR`, `def_XBASIC_PUT_SPRITE`, `def_XBASIC_READ`, `def_XBASIC_RESTORE`, `def_XBASIC_SCREEN`, `def_XBASIC_WIDTH`, `def_XBASIC_SOUND`, `def_XBASIC_TAB`, `def_XBASIC_USING`, `def_XBASIC_USR`, `def_XBASIC_PLAY`
- [ ] 8.2 Update input/read functions — `def_XBASIC_INPUT_1`, `def_XBASIC_INPUT_2`, `def_XBASIC_IREAD`, `def_XBASIC_USING_DO`
- [ ] 8.3 Update `def_castParamFloatInt` call sites (if any remain outside optimizer)

## 9. C-Side: Call Site Migration — Misc and Remaining

- [ ] 9.1 Update sprite/tile operations — `def_set_tile_flip`, `def_set_tile_rotate`, `def_set_tile_color`, `def_set_tile_pattern`, `def_get_tile_color`, `def_get_tile_pattern`, `def_set_sprite_flip`, `def_set_sprite_rotate`, `def_set_sprite_color`, `def_set_sprite_pattern`, `def_get_sprite_color`, `def_get_sprite_pattern`, `def_set_tile_color_buf`, `def_set_tile_pattern_buffer`
- [ ] 9.2 Update USR functions — `def_usr0`, `def_usr1`, `def_usr2`, `def_usr2_play`, `def_usr2_player_status`, `def_usr3`, `def_usr3_COLLISION_ALL`, `def_usr3_COLLISION_COUPLE`, `def_usr3_COLLISION_ONE`
- [ ] 9.3 Update MegaROM calls — `def_MR_CALL`, `def_MR_CALL_TRAP`, `def_MR_CHANGE_SGM`, `def_MR_GET_BYTE`, `def_MR_GET_DATA`, `def_MR_JUMP`
- [ ] 9.4 Update date/time — `def_get_date`, `def_get_time`, `def_set_date`, `def_set_time`, `def_GET_NEXT_TEMP_STRING_ADDRESS`
- [ ] 9.5 Update remaining — `def_vdp_set`, `def_cmd_ramtoram`, `def_cmd_ramtovram`, `def_cmd_rsctoram`, `def_cmd_vramtoram`, `def_cmd_mtf`, `def_cmd_setfnt`, `def_cmd_updfntclr`, `def_cmd_screen_copy`, `def_cmd_screen_load`, `def_cmd_screen_paste`
- [ ] 9.6 Update screen/font — `def_cmd_wrtspratr`, `def_cmd_wrtsprclr`, `def_cmd_wrtsprpat`, `def_cmd_wrtvram`

## 10. C-Side: Cleanup and Verification

- [ ] 10.1 Verify no remaining `cpu.addCall(def_` calls exist in the codebase (grep for pattern)
- [ ] 10.2 Verify no remaining direct `def_` address usage that assumes absolute addresses
- [ ] 10.3 Remove old `def_*` constants from `compiler_hooks.h` that are no longer referenced

## 11. Build and Test

- [ ] 11.1 Build kernel and verify: filler space increased by at least 115 bytes, `wrapper_routines_map_start` at 0x4102
- [ ] 11.2 Full release build (`make release`) — must compile without errors
- [ ] 11.3 Run `make test-unit` — all unit tests pass
- [ ] 11.4 Run `make test-integration` — all `.bas` files compile without errors
- [ ] 11.5 Run `make test-kernel` — kernel binary integrity verified
- [ ] 11.6 Compare before/after ROM binaries — only filler and dispatch table region differ
- [ ] 11.7 Verify incremental build works
