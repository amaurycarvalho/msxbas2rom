## 1. Assembly Kernel: Expand Wrapper Table

- [ ] 1.1 Add ~60 new `jp` entries to `wrapper_routines_map_start` in `src/infrastructure/kernel/asm/src/header/20_runtime.asm`, after the last existing entry (`jp cmd_fprint`), grouped as:
  - XBASIC math (MULTIPLY_INTEGERS, DIVIDE_INTEGERS, ADD_FLOATS, SUBTRACT_FLOATS, MULTIPLY_FLOATS, DIVIDE_FLOATS, SIN, COS, TAN, ATN, EXP, LOG, SQR, RND, INT, FIX, SGN_INT, SGN_FLOAT, ABS_INT)
  - XBASIC casts (CAST_INTEGER_TO_FLOAT, CAST_INTEGER_TO_STRING, CAST_FLOAT_TO_INTEGER, CAST_FLOAT_TO_STRING, CAST_STRING_TO_FLOAT, POWER_FLOAT_TO_INTEGER, POWER_FLOAT_TO_FLOAT)
  - XBASIC float compares (COMPARE_FLOATS_EQ, NE, GT, GE, LT, LE)
  - XBASIC string (LEFT, RIGHT, MID, MID_ASSIGN, INSTR, VAL, STRING, OCT_HEX_BIN, COPY_STRING, COPY_STRING_TO_NULBUF, CONCAT_STRINGS, INKEY)
  - XBASIC string compares (COMPARE_STRING_WITH_NULBUF_EQ, NE, GT, GE, LT, LE)
  - XBASIC graphics (POINT, LINE, BOX, BOXF, PSET, PAINT, CIRCLE, CIRCLE2, COLOR_RGB, COLOR_SPRITE, COLOR_SPRSTR, SCREEN_SPRITE, SET_PAGE, SET_SCROLL, VDP, VPEEK, VPOKE, SPRITE_ASSIGN, PUT_SPRITE)
  - XBASIC traps (TRAP_ON, TRAP_OFF, TRAP_STOP, TRAP_CHECK)
  - XBASIC swap (SWAP_INTEGER, SWAP_STRING, SWAP_FLOAT)
  - BIOS helper (WriteParamBCD)
- [ ] 1.2 Rebuild kernel binary and update `header.h` with the new `bin_header_bin` content
- [ ] 1.3 Update `header.symbols.asm` with the new absolute addresses for all new wrapper entries

## 2. C++ Header: Update compiler_hooks.h

- [ ] 2.1 Extend the `+3` define chain (after `def_cmd_fprint`, end of `@name header.symbols` block): add `#define def_XBASIC_MULTIPLY_INTEGERS (def_cmd_fprint + 3)` and chain all new entries through to `#define def_WriteParamBCD`
- [ ] 2.2 Replace the hardcoded XBASIC section (lines 378-460 in `compiler_hooks.h`) — change each `#define def_XBASIC_* 0xHHHH` to reference the new wrapper-chain computed value instead (or remove the redundant defines if the chain ones suffice)
- [ ] 2.3 Add named `#define` constants for all BIOS magic numbers under a new `@name bios_entry_points` section: `def_CHSNS` (0x009C), `def_CHGET` (0x009F), `def_BEEP` (0x00C0), `def_FNKSB` (0x00C9), `def_ERAFNK` (0x00CC), `def_DSPFNK` (0x00CF), `def_GTSTCK` (0x00D5), `def_GTTRIG` (0x00D8), `def_GTPDL` (0x00DE), `def_CHGCLR` (0x0062), `def_RDPSG` (0x0096)
- [ ] 2.4 Ensure the `@remark end of header.symbols adjust` marker is still correctly placed after the new chain

## 3. C++ Compiler: Expression Evaluator

- [ ] 3.1 `compiler_expression_evaluator.cpp`: Replace all `cpu.addCall(def_XBASIC_COMPARE_FLOATS_*)` with `optimizer.addKernelCall(def_XBASIC_COMPARE_FLOATS_*)` (lines 324, 361, 398, 435, 472, 509)
- [ ] 3.2 Replace all `cpu.addCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_*)` with `optimizer.addKernelCall(def_XBASIC_COMPARE_STRING_WITH_NULBUF_*)` (lines 335, 372, 409, 446, 483, 520)
- [ ] 3.3 Replace all `cpu.addCall(def_XBASIC_COPY_STRING_TO_NULBUF)` with `optimizer.addKernelCall(def_XBASIC_COPY_STRING_TO_NULBUF)` (lines 330, 367, 404, 441, 478, 515)
- [ ] 3.4 Replace `cpu.addCall(def_XBASIC_ADD_FLOATS)` with `optimizer.addKernelCall(def_XBASIC_ADD_FLOATS)` (line 543)
- [ ] 3.5 Replace `cpu.addCall(def_XBASIC_CONCAT_STRINGS)` with `optimizer.addKernelCall(def_XBASIC_CONCAT_STRINGS)` (line 554)
- [ ] 3.6 Replace `cpu.addCall(def_XBASIC_SUBTRACT_FLOATS)` with `optimizer.addKernelCall(def_XBASIC_SUBTRACT_FLOATS)` (line 598)
- [ ] 3.7 Replace all `cpu.addCall(def_XBASIC_MULTIPLY_*)` with `optimizer.addKernelCall(def_XBASIC_MULTIPLY_*)` (lines 681, 693)
- [ ] 3.8 Replace all `cpu.addCall(def_XBASIC_DIVIDE_*)` with `optimizer.addKernelCall(def_XBASIC_DIVIDE_*)` (lines 952, 958, 974, 986, 1009, 1057, 1082)
- [ ] 3.9 Replace `cpu.addCall(def_XBASIC_CAST_INTEGER_TO_FLOAT)` with `optimizer.addKernelCall(def_XBASIC_CAST_INTEGER_TO_FLOAT)` (lines 1025, 1258)
- [ ] 3.10 Replace `cpu.addCall(def_XBASIC_POWER_FLOAT_TO_INTEGER)` with `optimizer.addKernelCall(def_XBASIC_POWER_FLOAT_TO_INTEGER)` (line 1028)
- [ ] 3.11 Replace `cpu.addCall(def_XBASIC_POWER_FLOAT_TO_FLOAT)` with `optimizer.addKernelCall(def_XBASIC_POWER_FLOAT_TO_FLOAT)` (line 1045)
- [ ] 3.12 Replace `cpu.addCall(def_castParamFloatInt)` with `optimizer.addKernelCall(def_castParamFloatInt)` (line 1178)
- [ ] 3.13 Replace `cpu.addCall(def_XBASIC_CAST_INTEGER_TO_STRING)` with `optimizer.addKernelCall(def_XBASIC_CAST_INTEGER_TO_STRING)` (line 1254)
- [ ] 3.14 Replace `cpu.addCall(def_XBASIC_CAST_STRING_TO_FLOAT)` with `optimizer.addKernelCall(def_XBASIC_CAST_STRING_TO_FLOAT)` (lines 1265, 1274)
- [ ] 3.15 Replace `cpu.addCall(def_XBASIC_CAST_FLOAT_TO_INTEGER)` with `optimizer.addKernelCall(def_XBASIC_CAST_FLOAT_TO_INTEGER)` (lines 1267, 1281)
- [ ] 3.16 Replace `cpu.addCall(def_XBASIC_CAST_FLOAT_TO_STRING)` with `optimizer.addKernelCall(def_XBASIC_CAST_FLOAT_TO_STRING)` (line 1284)

## 4. C++ Compiler: Variable Emitter

- [ ] 4.1 Replace `cpu.addCall(def_XBASIC_MULTIPLY_INTEGERS)` with `optimizer.addKernelCall(def_XBASIC_MULTIPLY_INTEGERS)` (lines 91, 177)
- [ ] 4.2 Replace `cpu.addCall(def_GET_NEXT_TEMP_STRING_ADDRESS)` with `optimizer.addKernelCall(def_GET_NEXT_TEMP_STRING_ADDRESS)` (lines 290, 295)
- [ ] 4.3 Replace `cpu.addCall(def_XBASIC_COPY_STRING)` with `optimizer.addKernelCall(def_XBASIC_COPY_STRING)` (line 350)

## 5. C++ Compiler: Code Helper

- [ ] 5.1 Replace `cpu.addCall(def_XBASIC_TRAP_CHECK)` with `optimizer.addKernelCall(def_XBASIC_TRAP_CHECK)` (line 21)

## 6. C++ Compiler: Control Statements

- [ ] 6.1 `for/compiler_for_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_ADD_FLOATS)` with `optimizer.addKernelCall(def_XBASIC_ADD_FLOATS)` (line 249)
- [ ] 6.2 `for/compiler_for_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_COMPARE_FLOATS_LE)` with `optimizer.addKernelCall(def_XBASIC_COMPARE_FLOATS_LE)` (line 301)
- [ ] 6.3 `start/compiler_start_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_INIT)` with `optimizer.addKernelCall(def_XBASIC_INIT)` (line 52)
- [ ] 6.4 `start/compiler_start_statement_strategy.cpp`: Replace `cpu.addCall(def_MR_CHANGE_SGM)` with `optimizer.addKernelCall(def_MR_CHANGE_SGM)` (line 71)
- [ ] 6.5 `start/compiler_start_statement_strategy.cpp`: Replace `cpu.addCall(def_cmd_restore)` with `optimizer.addKernelCall(def_cmd_restore)` (line 80)
- [ ] 6.6 `start/compiler_start_statement_strategy.cpp`: Replace `cpu.addCall(def_player_initialize)` with `optimizer.addKernelCall(def_player_initialize)` (line 85)
- [ ] 6.7 `end/compiler_end_statement_strategy.cpp`: Replace `cpu.addCall(def_player_unhook)` with `optimizer.addKernelCall(def_player_unhook)` (line 28)
- [ ] 6.8 `end/compiler_end_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_END)` with `optimizer.addKernelCall(def_XBASIC_END)` (line 32)
- [ ] 6.9 `stop/compiler_stop_statement_strategy.cpp`: Replace trap calls with `optimizer.addKernelCall` (lines 27, 31, 35)

## 7. C++ Compiler: Math Functions

- [ ] 7.1 `abs/compiler_abs_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_ABS_INT)` with `optimizer.addKernelCall(def_XBASIC_ABS_INT)` (line 19)
- [ ] 7.2 `atn/compiler_atn_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_ATN)` (line 28)
- [ ] 7.3 `cos/compiler_cos_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_COS)` (line 28)
- [ ] 7.4 `exp/compiler_exp_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_EXP)` (line 28)
- [ ] 7.5 `fix/compiler_fix_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_FIX)` (line 28)
- [ ] 7.6 `int/compiler_int_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_INT)` (line 24)
- [ ] 7.7 `log/compiler_log_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_LOG)` (line 28)
- [ ] 7.8 `rnd/compiler_rnd_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_RND)` (line 28)
- [ ] 7.9 `sgn/compiler_sgn_function_strategy.cpp`: Replace both calls (lines 19, 24) with `optimizer.addKernelCall`
- [ ] 7.10 `sin/compiler_sin_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_SIN)` (line 28)
- [ ] 7.11 `sqr/compiler_sqr_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_SQR)` (line 28)
- [ ] 7.12 `tan/compiler_tan_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_TAN)` (line 28)

## 8. C++ Compiler: String Functions

- [ ] 8.1 `left/compiler_left_function_strategy.cpp`: Replace with `optimizer.addKernelCall` (lines 36, 43)
- [ ] 8.2 `right/compiler_right_function_strategy.cpp`: Replace with `optimizer.addKernelCall` (lines 38, 45)
- [ ] 8.3 `mid/compiler_mid_function_strategy.cpp`: Replace all calls with `optimizer.addKernelCall` (lines 41, 49, 98, 106)
- [ ] 8.4 `instr/compiler_instr_function_strategy.cpp`: Replace with `optimizer.addKernelCall` (lines 33, 74)
- [ ] 8.5 `val/compiler_val_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_VAL)` (line 28)
- [ ] 8.6 `string/compiler_string_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_STRING)` (line 69)
- [ ] 8.7 `space/compiler_space_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_STRING)` (line 38)
- [ ] 8.8 `bin/compiler_bin_function_strategy.cpp`: Replace with `optimizer.addKernelCall` (lines 33, 40)
- [ ] 8.9 `oct/compiler_oct_function_strategy.cpp`: Replace with `optimizer.addKernelCall` (lines 33, 40)
- [ ] 8.10 `hex/compiler_hex_function_strategy.cpp`: Replace with `optimizer.addKernelCall` (lines 33, 40)
- [ ] 8.11 `using/compiler_using_function_strategy.cpp`: Replace with `optimizer.addKernelCall` (lines 64, 71, 108, 115)
- [ ] 8.12 `tab/compiler_tab_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_TAB)` (line 28)

## 9. C++ Compiler: Graphics Functions

- [ ] 9.1 `point/compiler_point_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_POINT)` (line 58)
- [ ] 9.2 `tile/compiler_tile_function_strategy.cpp`: Replace `cpu.addCall(def_tileAddress)` with `optimizer.addKernelCall(def_tileAddress)` (line 64)
- [ ] 9.3 `tile/compiler_tile_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VPEEK)` with `optimizer.addKernelCall(def_XBASIC_VPEEK)` (line 67)
- [ ] 9.4 `collision/compiler_collision_function_strategy.cpp`: Replace `cpu.addCall(def_usr3_COLLISION_*)` with `optimizer.addKernelCall(def_usr3_COLLISION_*)` (lines 22, 36, 84)
- [ ] 9.5 `pad/compiler_pad_function_strategy.cpp`: Replace `cpu.addCall(def_cmd_pad)` with `optimizer.addKernelCall(def_cmd_pad)` (line 29)
- [ ] 9.6 `base/compiler_base_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_BASE)` (line 29)
- [ ] 9.7 `vdp/compiler_vdp_function_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VDP)` with `optimizer.addKernelCall(def_XBASIC_VDP)` (line 41)

## 10. C++ Compiler: Sound Functions

- [ ] 10.1 `play/compiler_play_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_usr2_play)` (line 29)
- [ ] 10.2 `plystatus/compiler_plystatus_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_usr2_player_status)` (line 21)

## 11. C++ Compiler: I/O Functions

- [ ] 11.1 `eof/compiler_eof_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_feof)` (line 34)
- [ ] 11.2 `loc/compiler_loc_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_floc)` (line 34)
- [ ] 11.3 `lof/compiler_lof_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_flof)` (line 34)
- [ ] 11.4 `fpos/compiler_fpos_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_fpos)` (line 34)
- [ ] 11.5 `dskf/compiler_dskf_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_fdskf)` (line 36)
- [ ] 11.6 `peek/compiler_peek_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_VPEEK)` (line 30)
- [ ] 11.7 `inkey/compiler_inkey_function_strategy.cpp`: Replace `cpu.addCall(0x009C)` with `cpu.addCall(def_CHSNS)` and `cpu.addCall(0x009F)` with `cpu.addCall(def_CHGET)` (lines 25, 29)
- [ ] 11.8 `inkey/compiler_inkey_string_function_strategy.cpp`: Replace `cpu.addCall(0x009C)` with `cpu.addCall(def_CHSNS)` and `cpu.addCall(def_XBASIC_INKEY)` with `optimizer.addKernelCall(def_XBASIC_INKEY)` (lines 23, 27)
- [ ] 11.9 `input/compiler_input_string_function_strategy.cpp`: Replace `cpu.addCall(0x009F)` with `cpu.addCall(def_CHGET)` (line 41)
- [ ] 11.10 `stick/compiler_stick_function_strategy.cpp`: Replace `cpu.addCall(0x00D5)` with `cpu.addCall(def_GTSTCK)` (line 31)
- [ ] 11.11 `strig/compiler_strig_function_strategy.cpp`: Replace `cpu.addCall(0x00D8)` with `cpu.addCall(def_GTTRIG)` (line 31)

## 12. C++ Compiler: Basic Functions

- [ ] 12.1 `usr/compiler_usr_function_strategy.cpp`: Replace all 5 `cpu.addCall(def_usr*)` with `optimizer.addKernelCall(def_usr*)` (lines 64, 68, 71, 74, 77)
- [ ] 12.2 `resource/compiler_resource_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_usr0)` (line 24)
- [ ] 12.3 `resource/compiler_resource_size_function_strategy.cpp`: Replace with `optimizer.addKernelCall(def_usr1)` (line 24)
- [ ] 12.4 `pdl/compiler_pdl_function_strategy.cpp`: Replace `cpu.addCall(0x00DE)` with `cpu.addCall(def_GTPDL)` (line 30)
- [ ] 12.5 `psg/compiler_psg_function_strategy.cpp`: Replace `cpu.addCall(0x0096)` with `cpu.addCall(def_RDPSG)` (line 34)

## 13. C++ Compiler: Graphics Statements

- [ ] 13.1 `screen/compiler_screen_statement_strategy.cpp`: Replace all 8 `cpu.addCall(def_cmd_*)` with `optimizer.addKernelCall(def_cmd_*)` (lines 213, 252, 288, 294, 320, 332, 339)
- [ ] 13.2 `screen/compiler_screen_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_SCREEN)` with `optimizer.addKernelCall(def_XBASIC_SCREEN)` (line 66)
- [ ] 13.3 `screen/compiler_screen_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_SCREEN_SPRITE)` with `optimizer.addKernelCall(def_XBASIC_SCREEN_SPRITE)` (line 72)
- [ ] 13.4 `set/compiler_set_statement_strategy.cpp`: Replace all 22 `cpu.addCall(def_set_*)`, `cpu.addCall(def_cmd_*)`, `cpu.addCall(def_XBASIC_*)` with `optimizer.addKernelCall(def_*)` (lines 449, 530, 571, 577, 619, 660, 729, 784, 846, 946, 998, 1043, 1061, 1107, 1133, 1167, 1201, 1251, 1296)
- [ ] 13.5 `line/compiler_line_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_LINE/BOX/BOXF)` with `optimizer.addKernelCall` (lines 353, 357, 361)
- [ ] 13.6 `circle/compiler_circle_statement_strategy.cpp`: Replace `cpu.addCall(def_WriteParamBCD)` with `optimizer.addKernelCall(def_WriteParamBCD)` (lines 211, 240, 270) and `cpu.addCall(def_XBASIC_CIRCLE/CIRCLE2)` with `optimizer.addKernelCall` (lines 296, 319)
- [ ] 13.7 `paint/compiler_paint_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_PAINT)` (line 188)
- [ ] 13.8 `pset/compiler_pset_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_PSET)` (line 197)
- [ ] 13.9 `put/compiler_put_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_PUT_SPRITE)` with `optimizer.addKernelCall(def_XBASIC_PUT_SPRITE)` (line 241) and `cpu.addCall(def_tileAddress)` with `optimizer.addKernelCall(def_tileAddress)` (line 399)
- [ ] 13.10 `put/compiler_put_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_VPOKE)` with `optimizer.addKernelCall(def_XBASIC_VPOKE)` (line 410)
- [ ] 13.11 `copy/compiler_copy_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_COPY_FROM/COPY_TO/COPY)` with `optimizer.addKernelCall` (lines 411, 468, 546)
- [ ] 13.12 `color/compiler_color_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_COLOR_*)` with `optimizer.addKernelCall(def_XBASIC_COLOR_*)` (lines 81, 108, 156)
- [ ] 13.13 `color/compiler_color_statement_strategy.cpp`: Replace `cpu.addCall(0x0062)` with `cpu.addCall(def_CHGCLR)` (line 203)
- [ ] 13.14 `key/compiler_key_statement_strategy.cpp`: Replace all 3 magic numbers with named defines (lines 26, 30, 107) — `def_DSPFNK`, `def_ERAFNK`, `def_FNKSB`
- [ ] 13.15 `key/compiler_key_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_TRAP_*)` with `optimizer.addKernelCall(def_XBASIC_TRAP_*)` (lines 128, 132, 136)
- [ ] 13.16 `locate/compiler_locate_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_LOCATE)` (line 38)
- [ ] 13.17 `width/compiler_width_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_WIDTH)` (line 24)
- [ ] 13.18 `get/compiler_get_statement_strategy.cpp`: Replace all 6 `cpu.addCall(def_get_*)` with `optimizer.addKernelCall(def_get_*)` (lines 51, 115, 220, 261, 320, 354)
- [ ] 13.19 `sprite/compiler_sprite_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_TRAP_*)` with `optimizer.addKernelCall` (lines 32, 36, 40) and `cpu.addCall(def_cmd_wrtspr)` with `optimizer.addKernelCall(def_cmd_wrtspr)` (line 72)
- [ ] 13.20 `strig/compiler_strig_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_TRAP_*)` with `optimizer.addKernelCall` (lines 48, 52, 56)

## 14. C++ Compiler: Sound Statements

- [ ] 14.1 `play/compiler_play_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_PLAY)` (line 47)
- [ ] 14.2 `sound/compiler_sound_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_SOUND)` (line 40)

## 15. C++ Compiler: I/O Statements

- [ ] 15.1 `open/compiler_open_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_fopen)` (line 91)
- [ ] 15.2 `close/compiler_close_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_fclose)` (lines 29, 37)
- [ ] 15.3 `restore/compiler_restore_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_RESTORE)` (lines 22, 35)
- [ ] 15.4 `read/compiler_read_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_READ)` (line 30)
- [ ] 15.5 `swap/compiler_swap_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_SWAP_*)` (lines 46, 49, 52)
- [ ] 15.6 `bload/compiler_bload_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_cmd_wrtspr)` and `optimizer.addKernelCall(def_cmd_screen_load)` (lines 55, 58)
- [ ] 15.7 `vpoke/compiler_vpoke_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_VPOKE)` (line 37)
- [ ] 15.8 `input/compiler_input_statement_strategy.cpp`: Replace `cpu.addCall(def_cmd_preflight_disk)` with `optimizer.addKernelCall(def_cmd_preflight_disk)` (line 51), file I/O calls with `optimizer.addKernelCall` (lines 98, 102), and print calls (lines 155, 158, 162)
- [ ] 15.9 `print/compiler_print_statement_strategy.cpp`: Replace all `cpu.addCall(def_cmd_*)` and `cpu.addCall(def_XBASIC_PRINT_*)` with `optimizer.addKernelCall` (lines 49, 93, 99, 106, 119, 148, 163, 176, 188, 191, 195, 206, 212, 223)
- [ ] 15.10 `iread/compiler_iread_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_IREAD)` (line 30)

## 16. C++ Compiler: Basic Statements

- [ ] 16.1 `cls/compiler_cls_statement_strategy.cpp`: Replace with `optimizer.addKernelCall(def_XBASIC_CLS)` (line 9)
- [ ] 16.2 `beep/compiler_beep_statement_strategy.cpp`: Replace `cpu.addCall(0x00C0)` with `cpu.addCall(def_BEEP)` (line 9)
- [ ] 16.3 `let/compiler_let_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_MID_ASSIGN)` with `optimizer.addKernelCall(def_XBASIC_MID_ASSIGN)` (lines 78, 129)
- [ ] 16.4 `let/compiler_let_statement_strategy.cpp`: Replace `cpu.addCall(def_vdp_set)` with `optimizer.addKernelCall(def_vdp_set)` (line 166)
- [ ] 16.5 `let/compiler_let_statement_strategy.cpp`: Replace `cpu.addCall(def_XBASIC_SPRITE_ASSIGN)` with `optimizer.addKernelCall(def_XBASIC_SPRITE_ASSIGN)` (line 200)

## 17. C++ Compiler: CMD Handlers

- [ ] 17.1 Replace `cpu.addCall(def_cmd_*)` with `optimizer.addKernelCall(def_cmd_*)` in all CMD handler files: `graphics/clrscr`, `graphics/disscr`, `graphics/draw`, `graphics/enascr`, `graphics/mtf`, `graphics/page`, `graphics/ramtovram`, `graphics/setfnt`, `graphics/updfntclr`, `graphics/vramtoram`, `graphics/wrtchr`, `graphics/wrtclr`, `graphics/wrtfnt`, `graphics/wrtscr`, `graphics/wrtspr`, `graphics/wrtspratr`, `graphics/wrtsprclr`, `graphics/wrtsprpat`, `graphics/wrtvram`
- [ ] 17.2 Replace `cpu.addCall(def_cmd_*)` with `optimizer.addKernelCall(def_cmd_*)` in all I/O CMD handler files: `io/clrkey`, `io/keyclkoff`, `io/ramtoram`, `io/restore`, `io/rsctoram`
- [ ] 17.3 Replace `cpu.addCall(def_cmd_*)` with `optimizer.addKernelCall(def_cmd_*)` in all basic CMD handler files: `basic/runasm`, `basic/turbo`
- [ ] 17.4 Replace `cpu.addCall(def_cmd_*)` with `optimizer.addKernelCall(def_cmd_*)` in all sound CMD handler files: `sound/mute`, `sound/play`, `sound/arkos/plyload`, `sound/arkos/plyloop`, `sound/arkos/plymute`, `sound/arkos/plyplay`, `sound/arkos/plyreplay`, `sound/arkos/plysong`, `sound/arkos/plysound`

## 18. C++ Compiler: Symbol Resolver (if applicable)

- [ ] 18.1 `compiler_symbol_resolver.cpp`: If any `cpu.addCall(def_*)` calls remain that are not BIOS-external (`def_ENASLT`, `def_CALBAS`), replace with `optimizer.addKernelCall`

## 19. Verification

- [ ] 19.1 Run full test suite (`make tests`) and verify all tests pass
- [ ] 19.2 Run integration tests (`tests/integration/test.sh`) and verify all ROM outputs are byte-identical to pre-change outputs
- [ ] 19.3 Search codebase for remaining `cpu.addCall(0x` patterns (excluding `0x0000` fixup placeholders) — should be zero
- [ ] 19.4 Verify that `bin_header_bin` at each new wrapper address contains `0xC3` (confirm the `jp` entries are correctly compiled into the binary)
- [ ] 19.5 Run `make lint` and fix any issues
