## 1. Assembly Kernel: Expand Wrapper Table

- [ ] 1.1 Add ~80 new `dw` entries to `wrapper_routines_map_table` in `src/infrastructure/kernel/asm/src/header/20_runtime.asm`, after the last existing entry (`dw cmd_fprint`, line 147), grouped as:
  - XBASIC math integer: `XBASIC_MULTIPLY_INTEGERS`, `XBASIC_DIVIDE_INTEGERS`
  - XBASIC math float: `XBASIC_ADD_FLOATS`, `XBASIC_SUBTRACT_FLOATS`, `XBASIC_MULTIPLY_FLOATS`, `XBASIC_DIVIDE_FLOATS`
  - XBASIC math trig: `XBASIC_SIN`, `XBASIC_COS`, `XBASIC_TAN`, `XBASIC_ATN`, `XBASIC_EXP`, `XBASIC_LOG`, `XBASIC_SQR`
  - XBASIC math other: `XBASIC_RND`, `XBASIC_INT`, `XBASIC_FIX`, `XBASIC_SGN_INT`, `XBASIC_SGN_FLOAT`, `XBASIC_ABS_INT`
  - XBASIC string: `XBASIC_LEFT`, `XBASIC_RIGHT`, `XBASIC_MID`, `XBASIC_MID_ASSIGN`, `XBASIC_INSTR`, `XBASIC_VAL`, `XBASIC_STRING`, `XBASIC_OCT_HEX_BIN`, `XBASIC_INKEY`
  - XBASIC string ops: `XBASIC_COPY_STRING`, `XBASIC_COPY_STRING_TO_NULBUF`, `XBASIC_CONCAT_STRINGS`
  - XBASIC float compares: `XBASIC_COMPARE_FLOATS_EQ`, `XBASIC_COMPARE_FLOATS_NE`, `XBASIC_COMPARE_FLOATS_GT`, `XBASIC_COMPARE_FLOATS_GE`, `XBASIC_COMPARE_FLOATS_LT`, `XBASIC_COMPARE_FLOATS_LE`
  - XBASIC string compares: `XBASIC_COMPARE_STRING_WITH_NULBUF_EQ`, `XBASIC_COMPARE_STRING_WITH_NULBUF_NE`, `XBASIC_COMPARE_STRING_WITH_NULBUF_GT`, `XBASIC_COMPARE_STRING_WITH_NULBUF_GE`, `XBASIC_COMPARE_STRING_WITH_NULBUF_LT`, `XBASIC_COMPARE_STRING_WITH_NULBUF_LE`
  - XBASIC casts: `XBASIC_CAST_INTEGER_TO_FLOAT`, `XBASIC_CAST_INTEGER_TO_STRING`, `XBASIC_CAST_FLOAT_TO_INTEGER`, `XBASIC_CAST_FLOAT_TO_STRING`, `XBASIC_CAST_STRING_TO_FLOAT`
  - XBASIC power: `XBASIC_POWER_FLOAT_TO_INTEGER`, `XBASIC_POWER_FLOAT_TO_FLOAT`
  - XBASIC graphics: `XBASIC_POINT`, `XBASIC_LINE`, `XBASIC_BOX`, `XBASIC_BOXF`, `XBASIC_PSET`, `XBASIC_PAINT`, `XBASIC_CIRCLE`, `XBASIC_CIRCLE2`
  - XBASIC graphics color/sprites: `XBASIC_COLOR_RGB`, `XBASIC_COLOR_SPRITE`, `XBASIC_COLOR_SPRSTR`, `XBASIC_SCREEN_SPRITE`, `XBASIC_SPRITE_ASSIGN`
  - XBASIC graphics page/scroll: `XBASIC_SET_PAGE`, `XBASIC_SET_SCROLL`
  - XBASIC graphics VDP: `XBASIC_VDP`, `XBASIC_VPEEK`, `XBASIC_VPOKE`
  - XBASIC traps: `XBASIC_TRAP_ON`, `XBASIC_TRAP_OFF`, `XBASIC_TRAP_STOP`, `XBASIC_TRAP_CHECK`
  - XBASIC swaps: `XBASIC_SWAP_INTEGER`, `XBASIC_SWAP_STRING`, `XBASIC_SWAP_FLOAT`
  - BIOS helper: `WriteParamBCD`
  - XBASIC print: `XBASIC_PRINT_TAB`, `XBASIC_PRINT_INT`, `XBASIC_PRINT_FLOAT`, `XBASIC_PRINT_CRLF`
- [ ] 1.2 Rebuild kernel binary and update `bin_header_bin` / `header.h` with the new binary content

## 2. C++ Header: Update compiler_hooks.h

- [ ] 2.1 Add new `DISP_*` dispatch index constants after `DISP_cmd_fprint` (line 265), with sequential integer values starting from 126, following the same order as the assembly entries:
  - Math integer: `DISP_XBASIC_MULTIPLY_INTEGERS 126`, `DISP_XBASIC_DIVIDE_INTEGERS 127`
  - Math float: `DISP_XBASIC_ADD_FLOATS 128`, `DISP_XBASIC_SUBTRACT_FLOATS 129`, `DISP_XBASIC_MULTIPLY_FLOATS 130`, `DISP_XBASIC_DIVIDE_FLOATS 131`
  - Math trig: `DISP_XBASIC_SIN 132`, `DISP_XBASIC_COS 133`, `DISP_XBASIC_TAN 134`, `DISP_XBASIC_ATN 135`, `DISP_XBASIC_EXP 136`, `DISP_XBASIC_LOG 137`, `DISP_XBASIC_SQR 138`
  - Math other: `DISP_XBASIC_RND 139`, `DISP_XBASIC_INT 140`, `DISP_XBASIC_FIX 141`, `DISP_XBASIC_SGN_INT 142`, `DISP_XBASIC_SGN_FLOAT 143`, `DISP_XBASIC_ABS_INT 144`
  - String: `DISP_XBASIC_LEFT 145` through `DISP_XBASIC_INKEY 153`
  - String ops: `DISP_XBASIC_COPY_STRING 154`, `DISP_XBASIC_COPY_STRING_TO_NULBUF 155`, `DISP_XBASIC_CONCAT_STRINGS 156`
  - Float compares: `DISP_XBASIC_COMPARE_FLOATS_EQ 157` through `DISP_XBASIC_COMPARE_FLOATS_LE 162`
  - String compares: `DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ 163` through `DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_LE 168`
  - Casts: `DISP_XBASIC_CAST_INTEGER_TO_FLOAT 169` through `DISP_XBASIC_CAST_STRING_TO_FLOAT 173`
  - Power: `DISP_XBASIC_POWER_FLOAT_TO_INTEGER 174`, `DISP_XBASIC_POWER_FLOAT_TO_FLOAT 175`
  - Graphics core: `DISP_XBASIC_POINT 176` through `DISP_XBASIC_CIRCLE2 183`
  - Graphics color/sprites: `DISP_XBASIC_COLOR_RGB 184` through `DISP_XBASIC_SPRITE_ASSIGN 188`
  - Graphics page/scroll: `DISP_XBASIC_SET_PAGE 189`, `DISP_XBASIC_SET_SCROLL 190`
  - Graphics VDP: `DISP_XBASIC_VDP 191`, `DISP_XBASIC_VPEEK 192`, `DISP_XBASIC_VPOKE 193`
  - Traps: `DISP_XBASIC_TRAP_ON 194` through `DISP_XBASIC_TRAP_CHECK 197`
  - Swaps: `DISP_XBASIC_SWAP_INTEGER 198`, `DISP_XBASIC_SWAP_STRING 199`, `DISP_XBASIC_SWAP_FLOAT 200`
  - BIOS helper: `DISP_WriteParamBCD 201`
  - Print: `DISP_XBASIC_PRINT_TAB 202`, `DISP_XBASIC_PRINT_INT 203`, `DISP_XBASIC_PRINT_FLOAT 204`, `DISP_XBASIC_PRINT_CRLF 205`
- [ ] 2.2 Update `DISP_ENTRIES` from `126` to `206`
- [ ] 2.3 Add named BIOS constants under the `@name basic` section (or a new `@name bios_entry_points` section): `def_CHSNS 0x009C`, `def_CHGET 0x009F`, `def_BEEP 0x00C0`, `def_FNKSB 0x00C9`, `def_ERAFNK 0x00CC`, `def_GTSTCK 0x00D5`, `def_GTTRIG 0x00D8`, `def_GTPDL 0x00DE`, `def_CHGCLR 0x0062`, `def_RDPSG 0x0096`

## 3. C++ Compiler: Expression Evaluator

- [ ] 3.1 `compiler_expression_evaluator.cpp`: Replace all `cpu.addCall(def_XBASIC_COMPARE_FLOATS_*)` with `optimizer.addKernelCall(DISP_XBASIC_COMPARE_FLOATS_*)` (6 calls)
- [ ] 3.2 Replace all `cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_*)` with `optimizer.addKernelCall(DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_*)` (6 calls)
- [ ] 3.3 Replace all `cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF)` with `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING_TO_NULBUF)` (6 calls)
- [ ] 3.4 Replace `cpu.addCall(def_XBASIC_ADD_FLOATS)` with `optimizer.addKernelCall(DISP_XBASIC_ADD_FLOATS)`
- [ ] 3.5 Replace `cpu.addCall(def_XBASIC_CONCAT_STRINGS)` with `optimizer.addKernelCall(DISP_XBASIC_CONCAT_STRINGS)`
- [ ] 3.6 Replace `cpu.addCall(def_XBASIC_SUBTRACT_FLOATS)` with `optimizer.addKernelCall(DISP_XBASIC_SUBTRACT_FLOATS)`
- [ ] 3.7 Replace all `cpu.addCall(def_XBASIC_MULTIPLY_*)` with `optimizer.addKernelCall(DISP_XBASIC_MULTIPLY_*)` (2 calls)
- [ ] 3.8 Replace all `cpu.addCall(def_XBASIC_DIVIDE_*)` with `optimizer.addKernelCall(DISP_XBASIC_DIVIDE_*)` (7 calls)
- [ ] 3.9 Replace all `cpu.addCall(def_XBASIC_CAST_*)` with `optimizer.addKernelCall(DISP_XBASIC_CAST_*)` (5 calls for cast routines)
- [ ] 3.10 Replace `cpu.addCall(def_XBASIC_POWER_FLOAT_TO_INTEGER)` with `optimizer.addKernelCall(DISP_XBASIC_POWER_FLOAT_TO_INTEGER)`
- [ ] 3.11 Replace `cpu.addCall(def_XBASIC_POWER_FLOAT_TO_FLOAT)` with `optimizer.addKernelCall(DISP_XBASIC_POWER_FLOAT_TO_FLOAT)`

## 4. C++ Compiler: Variable Emitter

- [ ] 4.1 Replace `cpu.addCall(def_XBASIC_MULTIPLY_INTEGERS)` with `optimizer.addKernelCall(DISP_XBASIC_MULTIPLY_INTEGERS)` (2 calls)
- [ ] 4.2 Replace `cpu.addCall(def_XBASIC_COPY_STRING)` with `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)`

## 5. C++ Compiler: Code Helper

- [ ] 5.1 Replace `cpu.addCall(def_XBASIC_TRAP_CHECK)` with `optimizer.addKernelCall(DISP_XBASIC_TRAP_CHECK)`

## 6. C++ Compiler: Control Statements

- [ ] 6.1 `for/compiler_for_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_ADD_FLOATS)` with `optimizer.addKernelCall(DISP_XBASIC_ADD_FLOATS)`
- [ ] 6.2 `for/compiler_for_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_COMPARE_FLOATS_LE)` with `optimizer.addKernelCall(DISP_XBASIC_COMPARE_FLOATS_LE)`
- [ ] 6.3 `stop/compiler_stop_statement_strategy.cpp`: Replace trap calls `cpu.addCall(def_XBASIC_TRAP_*)` with `optimizer.addKernelCall(DISP_XBASIC_TRAP_*)` (3 calls)
- [ ] 6.4 `interval/compiler_interval_statement_strategy.cpp`: Replace trap calls with `optimizer.addKernelCall(DISP_XBASIC_TRAP_*)` (3 calls)

## 7. C++ Compiler: Math Functions

- [ ] 7.1 `abs/compiler_abs_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_ABS_INT)` with `optimizer.addKernelCall(DISP_XBASIC_ABS_INT)`
- [ ] 7.2 `atn/compiler_atn_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_ATN)`
- [ ] 7.3 `cos/compiler_cos_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_COS)`
- [ ] 7.4 `exp/compiler_exp_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_EXP)`
- [ ] 7.5 `fix/compiler_fix_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_FIX)`
- [ ] 7.6 `int/compiler_int_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_INT)`
- [ ] 7.7 `log/compiler_log_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_LOG)`
- [ ] 7.8 `rnd/compiler_rnd_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_RND)`
- [ ] 7.9 `sgn/compiler_sgn_function_strategy.cpp`: Replace both calls with `optimizer.addKernelCall(DISP_XBASIC_SGN_*)` (2 calls)
- [ ] 7.10 `sin/compiler_sin_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_SIN)`
- [ ] 7.11 `sqr/compiler_sqr_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_SQR)`
- [ ] 7.12 `tan/compiler_tan_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_TAN)`

## 8. C++ Compiler: String Functions

- [ ] 8.1 `left/compiler_left_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_LEFT)` with `optimizer.addKernelCall(DISP_XBASIC_LEFT)` and `cpu.addCall(def_XBASIC_COPY_STRING)` with `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)`
- [ ] 8.2 `right/compiler_right_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_RIGHT)` and `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)`
- [ ] 8.3 `mid/compiler_mid_function_strategy.cpp`: Replace all calls with `optimizer.addKernelCall(DISP_XBASIC_MID)` and `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)` (4 calls)
- [ ] 8.4 `instr/compiler_instr_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_INSTR)` (2 calls)
- [ ] 8.5 `val/compiler_val_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_VAL)`
- [ ] 8.6 `string/compiler_string_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_STRING)`
- [ ] 8.7 `space/compiler_space_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_STRING)`
- [ ] 8.8 `bin/compiler_bin_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_OCT_HEX_BIN)` and `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)`
- [ ] 8.9 `oct/compiler_oct_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_OCT_HEX_BIN)` and `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)`
- [ ] 8.10 `hex/compiler_hex_function_strategy.cpp`: Replace with `optimizer.addKernelCall(DISP_XBASIC_OCT_HEX_BIN)` and `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)`
- [ ] 8.11 `using/compiler_using_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_COPY_STRING)` with `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)` (2 calls) — NOTE: `DISP_XBASIC_USING` and `DISP_XBASIC_USING_DO` already used

## 9. C++ Compiler: Graphics Functions

- [ ] 9.1 `point/compiler_point_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_POINT)` with `optimizer.addKernelCall(DISP_XBASIC_POINT)`
- [ ] 9.2 `tile/compiler_tile_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VPEEK)` with `optimizer.addKernelCall(DISP_XBASIC_VPEEK)` — NOTE: `DISP_tileAddress` already used
- [ ] 9.3 `vdp/compiler_vdp_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VDP)` with `optimizer.addKernelCall(DISP_XBASIC_VDP)`
- [ ] 9.4 `peek/compiler_peek_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VPEEK)` with `optimizer.addKernelCall(DISP_XBASIC_VPEEK)`

## 10. C++ Compiler: I/O Functions

- [ ] 10.1 `inkey/compiler_inkey_function_strategy.cpp`: Replace `cpu.addCall(0x009C)` with `cpu.addCall(def_CHSNS)` and `cpu.addCall(0x009F)` with `cpu.addCall(def_CHGET)`
- [ ] 10.2 `inkey/compiler_inkey_string_function_strategy.cpp`: Replace `cpu.addCall(0x009C)` with `cpu.addCall(def_CHSNS)` and `cpu.addCall(def_XBASIC_INKEY)` with `optimizer.addKernelCall(DISP_XBASIC_INKEY)`
- [ ] 10.3 `input/compiler_input_string_function_strategy.cpp`: Replace `cpu.addCall(0x009F)` with `cpu.addCall(def_CHGET)`
- [ ] 10.4 `stick/compiler_stick_function_strategy.cpp`: Replace `cpu.addCall(0x00D5)` with `cpu.addCall(def_GTSTCK)`
- [ ] 10.5 `strig/compiler_strig_function_strategy.cpp`: Replace `cpu.addCall(0x00D8)` with `cpu.addCall(def_GTTRIG)`

## 11. C++ Compiler: Basic Functions

- [ ] 11.1 `pdl/compiler_pdl_function_strategy.cpp`: Replace `cpu.addCall(0x00DE)` with `cpu.addCall(def_GTPDL)`
- [ ] 11.2 `psg/compiler_psg_function_strategy.cpp`: Replace `cpu.addCall(0x0096)` with `cpu.addCall(def_RDPSG)`
- [ ] 11.3 `vpeek/compiler_vpeek_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VPEEK)` with `optimizer.addKernelCall(DISP_XBASIC_VPEEK)`

## 12. C++ Compiler: Graphics Statements

- [ ] 12.1 `screen/compiler_screen_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_SCREEN_SPRITE)` with `optimizer.addKernelCall(DISP_XBASIC_SCREEN_SPRITE)`
- [ ] 12.2 `set/compiler_set_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_SET_PAGE)` with `optimizer.addKernelCall(DISP_XBASIC_SET_PAGE)` and `cpu.addCall(def_XBASIC_SET_SCROLL)` with `optimizer.addKernelCall(DISP_XBASIC_SET_SCROLL)`
- [ ] 12.3 `line/compiler_line_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_LINE)` with `optimizer.addKernelCall(DISP_XBASIC_LINE)`, `cpu.addCall(def_XBASIC_BOX)` with `optimizer.addKernelCall(DISP_XBASIC_BOX)`, `cpu.addCall(def_XBASIC_BOXF)` with `optimizer.addKernelCall(DISP_XBASIC_BOXF)`
- [ ] 12.4 `circle/compiler_circle_statement_strategy.cpp`: Replace `cpu.addCall(def_WriteParamBCD)` with `optimizer.addKernelCall(DISP_WriteParamBCD)` (3 calls), `cpu.addCall(def_XBASIC_CIRCLE)` with `optimizer.addKernelCall(DISP_XBASIC_CIRCLE)`, `cpu.addCall(def_XBASIC_CIRCLE2)` with `optimizer.addKernelCall(DISP_XBASIC_CIRCLE2)`
- [ ] 12.5 `paint/compiler_paint_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_PAINT)` with `optimizer.addKernelCall(DISP_XBASIC_PAINT)`
- [ ] 12.6 `pset/compiler_pset_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_PSET)` with `optimizer.addKernelCall(DISP_XBASIC_PSET)`
- [ ] 12.7 `put/compiler_put_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VPOKE)` with `optimizer.addKernelCall(DISP_XBASIC_VPOKE)`
- [ ] 12.8 `color/compiler_color_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_COLOR_RGB)` with `optimizer.addKernelCall(DISP_XBASIC_COLOR_RGB)`, `cpu.addCall(def_XBASIC_COLOR_SPRITE)` with `optimizer.addKernelCall(DISP_XBASIC_COLOR_SPRITE)`, `cpu.addCall(def_XBASIC_COLOR_SPRSTR)` with `optimizer.addKernelCall(DISP_XBASIC_COLOR_SPRSTR)`, and `cpu.addCall(0x0062)` with `cpu.addCall(def_CHGCLR)`
- [ ] 12.9 `key/compiler_key_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_TRAP_*)` with `optimizer.addKernelCall(DISP_XBASIC_TRAP_*)` (3 calls), and magic numbers `0x00CF` → `def_DSPFNK`, `0x00CC` → `def_ERAFNK`, `0x00C9` → `def_FNKSB`
- [ ] 12.10 `sprite/compiler_sprite_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_TRAP_*)` with `optimizer.addKernelCall(DISP_XBASIC_TRAP_*)` (3 calls)
- [ ] 12.11 `strig/compiler_strig_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_TRAP_*)` with `optimizer.addKernelCall(DISP_XBASIC_TRAP_*)` (3 calls)

## 13. C++ Compiler: I/O Statements

- [ ] 13.1 `swap/compiler_swap_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_SWAP_*)` with `optimizer.addKernelCall(DISP_XBASIC_SWAP_*)` (3 calls)
- [ ] 13.2 `vpoke/compiler_vpoke_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VPOKE)` with `optimizer.addKernelCall(DISP_XBASIC_VPOKE)`
- [ ] 13.3 `print/compiler_print_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_PRINT_TAB)` with `optimizer.addKernelCall(DISP_XBASIC_PRINT_TAB)`, `cpu.addCall(def_XBASIC_PRINT_INT)` with `optimizer.addKernelCall(DISP_XBASIC_PRINT_INT)`, `cpu.addCall(def_XBASIC_PRINT_FLOAT)` with `optimizer.addKernelCall(DISP_XBASIC_PRINT_FLOAT)`, and `cpu.addCall(def_XBASIC_PRINT_CRLF)` with `optimizer.addKernelCall(DISP_XBASIC_PRINT_CRLF)` (2 calls)
- [ ] 13.4 `input/compiler_input_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_PRINT_TAB)` with `optimizer.addKernelCall(DISP_XBASIC_PRINT_TAB)`, `cpu.addCall(def_XBASIC_PRINT_INT)` with `optimizer.addKernelCall(DISP_XBASIC_PRINT_INT)`, and `cpu.addCall(def_XBASIC_PRINT_FLOAT)` with `optimizer.addKernelCall(DISP_XBASIC_PRINT_FLOAT)`

## 14. C++ Compiler: Basic Statements

- [ ] 14.1 `beep/compiler_beep_statement_strategy.cpp`: Replace `cpu.addCall(0x00C0)` with `cpu.addCall(def_BEEP)`
- [ ] 14.2 `let/compiler_let_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_MID_ASSIGN)` with `optimizer.addKernelCall(DISP_XBASIC_MID_ASSIGN)` (2 calls) and `cpu.addCall(def_XBASIC_SPRITE_ASSIGN)` with `optimizer.addKernelCall(DISP_XBASIC_SPRITE_ASSIGN)`

## 15. Verification

- [ ] 15.1 Run full test suite (`make tests`) and verify all tests pass
- [ ] 15.2 Run integration tests (`tests/integration/test.sh`) and verify all ROM outputs are byte-identical to pre-change outputs
- [ ] 15.3 Search codebase for remaining `cpu.addCall(0x` patterns (excluding `0x0000` fixup placeholders) — should be zero
- [ ] 15.4 Verify that `DISP_ENTRIES` matches the actual count of entries in the assembly table
- [ ] 15.5 Verify that `bin_header_bin` at each new dispatch address contains valid 2-byte word pointers encoding addresses in the 0x4000-0x7FFF range
- [ ] 15.6 Run `make lint` and fix any issues
