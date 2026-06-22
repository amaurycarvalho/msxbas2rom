## 1. C-Side: compiler*hooks.h — DISP* Constants

- [ ] 1.1 Replace the `+3` address chain with sequential `DISP_` prefixed index constants (e.g., `#define DISP_castParamFloatInt 0`, `#define DISP_cmd_clrkey 1`)
- [ ] 1.2 Remove `def_wrapper_routines_map_start` routine (20_runtime.asm)
- [ ] 1.3 Remove all old `def_*` wrapper constants (def_castParamFloatInt through def_cmd_fprint) — they are no longer valid addresses
- [ ] 1.4 Verify constant count: exactly 126 `DISP_` entries matching the wrapper table order

## 2. C-Side: compiler_code_optimizer — getKernelCallAddr Update

- [ ] 2.1 Update `getKernelCallAddr()` to read 2-byte word pointers from the `dw` table instead of checking for `0xC3` (`jp`) opcode
- [ ] 2.2 Remove the `0xC3` check; directly return `bin_header_bin[offset] | (bin_header_bin[offset + 1] << 8)` for kernel-page addresses
- [ ] 2.3 Verify the offset calculation: `offset = address - 0x4000` remains correct for the new `dw` table layout. Also, it must calculate the real call address at compile time instead of runtime

## 3. C-Side: Call Site Migration — cpu.addCall(def_xxx) → addKernelCall(DISP_xxx)

- [ ] 3.1 Update `compiler_expression_evaluator.cpp` — replace `optimizer.addKernelCall(def_intCompare*)` and `cpu.addCall(def_floatNEG)` with `DISP_*` equivalents
- [ ] 3.2 Update `compiler_for_statement_strategy.cpp` — replace `optimizer.addKernelCall(def_intCompareGT)` with `DISP_*`
- [ ] 3.3 Update integer shift operations (`def_intSHR`, `def_intSHL`) in expression evaluator
- [ ] 3.4 Update command handler files (`statements/strategies/io/cmd/handlers/`) — all `def_cmd_*` to `DISP_cmd_*`
- [ ] 3.5 Update player-related files — `def_cmd_ply*`, `def_player_*` to `DISP_*` equivalents
- [ ] 3.6 Update file I/O files — `def_cmd_f*`, `def_cmd_preflight_disk` to `DISP_*`
- [ ] 3.7 Update XBASIC routine call sites — `def_XBASIC_*` to `DISP_*`
- [ ] 3.8 Update sprite/tile operations — `def_set_tile_*`, `def_get_tile_*`, `def_set_sprite_*`, `def_get_sprite_*`
- [ ] 3.9 Update USR functions — `def_usr*` to `DISP_usr*`
- [ ] 3.10 Update MegaROM calls — `def_MR_*` to `DISP_MR_*`
- [ ] 3.11 Update date/time — `def_get_date`, `def_get_time`, `def_set_date`, `def_set_time`, `def_GET_NEXT_TEMP_STRING_ADDRESS`
- [ ] 3.12 Update remaining — `def_vdp_set`, `def_cmd_ramtoram`, `def_cmd_ramtovram`, `def_cmd_rsctoram`, `def_cmd_vramtoram`, `def_cmd_mtf`, `def_cmd_setfnt`, `def_cmd_updfntclr`, `def_cmd_screen_*`, `def_cmd_wrtspratr`, `def_cmd_wrtsprclr`, `def_cmd_wrtsprpat`, `def_cmd_wrtvram`, `def_cmd_wrtchr`, `def_cmd_wrtclr`, `def_cmd_wrtfnt`, `def_cmd_wrtscr`, `def_cmd_wrtspr`, `def_cmd_page`, `def_cmd_turbo`, `def_cmd_restore`, `def_cmd_runasm`, `def_cmd_draw`, `def_gfxTileAddress`, `def_tileAddress`, `def_floatNEG`

## 4. C-Side: Call Site Migration — getKernelCallAddr(def_xxx) Pattern

- [ ] 4.1 Update pattern `optimizer.getKernelCallAddr(def_xxx)` to use `DISP_xxx`
- [ ] 4.2 Update pattern `optimizer.getKernelCallAddr(def_xxx) + 1` (used in turbo, maker, vdp functions) to use `DISP_xxx`
- [ ] 4.3 Ensure all `getKernelCallAddr()` callers pass `DISP_*` index values that work with the updated `dw` table resolution

## 5. Build System: Kernel Size Check

- [ ] 5.1 Add a Makefile target or post-build step that checks `header.bin` size ≤ 16384 bytes
- [ ] 5.2 On breach, emit alert: `WARNING: Kernel size <X> bytes exceeds 0x4000 limit by <Y> bytes`
- [ ] 5.3 Verify the check runs as part of kernel build targets
- [ ] 5.4 Add an unit test for kernel size check

## 6. Cleanup and Verification

- [ ] 6.1 Verify no remaining `cpu.addCall(def_` calls exist in the codebase
- [ ] 6.2 Verify no remaining `getKernelCallAddr(def_` calls exist
- [ ] 6.3 Verify no remaining direct `def_*` wrapper constant references (grep for `def_castParamFloatInt`, `def_cmd_`, `def_intCompare`, `def_floatNEG`, `def_player_`, `def_usr`, `def_set_`, `def_get_`, `def_MR_`, `def_XBASIC_`, `def_get_`, `def_set_`, `def_vdp_set`, `def_GET_NEXT_TEMP_STRING_ADDRESS`, `def_gfxTileAddress`, `def_tileAddress`)
- [ ] 6.4 Build kernel and verify `header.bin` size ≤ 0x4000 bytes (no alert triggered)
- [ ] 6.5 Full release build (`make release`) — must compile without errors
- [ ] 6.6 Run `make test-unit` — all unit tests pass
- [ ] 6.7 Run `make test-integration` — all `.bas` files compile without errors
- [ ] 6.8 Run `make test-kernel` — kernel binary integrity verified
