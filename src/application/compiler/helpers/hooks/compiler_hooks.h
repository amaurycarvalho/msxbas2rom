/***
 * @file compiler_hooks.h
 * @brief Compiler General Addresses, Hooks and Constants
 * @author Amaury Carvalho (2019-2025)
 */

#ifndef COMPILER_HOOKS_H_INCLUDED
#define COMPILER_HOOKS_H_INCLUDED

/***************************************************************
 * @name bios_workareas
 * @brief bios workareas and internal addresses
 */

#define def_DAC 0xF7F6
#define def_ARG 0xF847
#define def_ARG2 (def_ARG + 2)
#define def_VALTYP 0xF663
#define def_PRMFLG 0xF7B4

//! @warning
//! BASMEM must to be adjusted if any change in
//! compiler internal variables workspace size,
//! so search for BASMEM in header.symbols.asm
//! to get the correct value.
#define def_BASMEM 0xC038

/***************************************************************
 * @name kernel_workareas
 * @brief kernel workareas
 */

#define def_PLYBUF 0xEF00

//! @note 32 sprites * (test, x0, x1, y0, y1)
#define def_SPRTBL (def_PLYBUF - (32 * 5))

#define def_SPRSIZ (def_SPRTBL - 1)

//! @brief start of RAM
#define def_RAM_INIT 0xC010

//! @brief compiler internal variables buffer
//! @note RAM_INTVARSIZ = BASMEM - INIT
#define def_RAM_INTVARSIZ (def_BASMEM - def_RAM_INIT)

#define def_RAM_BOTTOM (def_RAM_INIT + def_RAM_INTVARSIZ)
#define def_RAM_TOP (def_SPRSIZ - 1)
#define def_RAM_SIZE (def_RAM_TOP - def_RAM_BOTTOM)

//! @brief free variable RAM available when file I/O is NOT enabled
//! @note empirical value; represents usable space within BASMEM..HEAPEND
//!       after accounting for temp strings and kernel variable bookkeeping
#define def_VAR_RAM_NON_DISK 10534

//! @brief free variable RAM available when file I/O IS enabled
//! @note 4603 bytes less than def_VAR_RAM_NON_DISK due to DSKBAS
//!       workarea and disk I/O internal structures
#define def_VAR_RAM_DISK 5931

//! @brief kernel internal RAM overhead above HEAPEND
//! @note covers sprite tables, player buffer, heap metadata, mapper
//!       state, MR routines, stack margin, and other kernel structures
#define def_KERNEL_RAM_OVERHEAD 0x538

//! @note 2176 bytes of font buffer in RAM
#define def_RAM_BUFSIZ 0x0880

#define def_SLTSTR 0xC010  // 1
#define def_SLTAD2 0xC01D  // 1

//! @brief screen output mode
//! @note 1 byte: 0=text, 1=graphical, 2=tiled
#define def_SOMODE 0xC024

//! @brief heap start address
//! @note 2 bytes
#define def_HEAPSTR 0xC025

//! @brief heap size
//! @note 2 bytes
#define def_HEAPSIZ 0xC027

//! @brief temporary string index
//! @note 1 byte
#define def_TMPSTR_IND 0xC029

//! @brief temporary string start address
//! @note 2 bytes
#define def_TMPSTR_ADDR 0xC02A

//! @brief temporary string next pointer
//! @note 2 bytes
#define def_TMPSTR_BUF 0xC02C

//! @brief old bios font address
//! @note 2 bytes
#define def_FONT_OLD 0xC02F

//! @brief new font space address
//! @note 2 bytes
#define def_FONT_NEW 0xC031

#define def_MR_TRAP_FLAG 0xFC82
#define def_MR_TRAP_SEGMS def_MR_TRAP_FLAG + 1

/***************************************************************
 * @name kernel_hooks
 * @brief kernel internal addresses hooks
 */

//! @defgroup header.symbols
//! @brief hooks adjustments group based on header.symbols.asm file
//! @remark start of header.symbols adjust
//! @{

//! @warning
//! see "wrapper_routines_map_table" correct address in
//! "header.symbols.asm" if Z80 kernel was recompiled
//! Table is now at logical address 0x0000 in the virtual
//! first half of header.bin (not written to ROM).
#define def_wrapper_routines_map_table 0x0000

// Dispatch indexes for wrapper routine word-pointer table
// (0-indexed, matching order in wrapper_routines_map_table)
#define DISP_castParamFloatInt 0
#define DISP_cmd_clrkey 1
#define DISP_cmd_clrscr 2
#define DISP_cmd_disscr 3
#define DISP_cmd_draw 4
#define DISP_cmd_enascr 5
#define DISP_cmd_keyclkoff 6
#define DISP_cmd_mute 7
#define DISP_cmd_play 8
#define DISP_cmd_pad 9

#define DISP_cmd_plyload 10
#define DISP_cmd_plyloop 11
#define DISP_cmd_plymute 12
#define DISP_cmd_plyplay 13
#define DISP_cmd_plyreplay 14
#define DISP_cmd_plysong 15
#define DISP_cmd_plysound 16

#define DISP_cmd_ramtoram 17
#define DISP_cmd_ramtovram 18
#define DISP_cmd_rsctoram 19
#define DISP_cmd_restore 20
#define DISP_cmd_runasm 21

#define DISP_cmd_screen_copy 22
#define DISP_cmd_screen_load 23
#define DISP_cmd_screen_paste 24

#define DISP_cmd_mtf 25

#define DISP_cmd_setfnt 26
#define DISP_cmd_turbo 27
#define DISP_cmd_updfntclr 28
#define DISP_cmd_vramtoram 29
#define DISP_cmd_wrtchr 30
#define DISP_cmd_wrtclr 31
#define DISP_cmd_wrtfnt 32
#define DISP_cmd_wrtscr 33
#define DISP_cmd_wrtspr 34
#define DISP_cmd_wrtspratr 35
#define DISP_cmd_wrtsprclr 36
#define DISP_cmd_wrtsprpat 37
#define DISP_cmd_wrtvram 38
#define DISP_cmd_page 39

#define DISP_floatNEG 40
#define DISP_tileAddress 41

#define DISP_intCompareAND 42
#define DISP_intCompareEQ 43
#define DISP_intCompareGE 44
#define DISP_intCompareGT 45
#define DISP_intCompareLE 46
#define DISP_intCompareLT 47
#define DISP_intCompareNE 48
#define DISP_intCompareNOT 49
#define DISP_intCompareOR 50
#define DISP_intCompareXOR 51
#define DISP_intNEG 52
#define DISP_intSHL 53
#define DISP_intSHR 54

#define DISP_player_initialize 55
#define DISP_player_unhook 56

#define DISP_set_tile_flip 57
#define DISP_set_tile_rotate 58
#define DISP_set_tile_color 59
#define DISP_set_tile_pattern 60
#define DISP_get_tile_color 61
#define DISP_get_tile_pattern 62
#define DISP_set_sprite_flip 63
#define DISP_set_sprite_rotate 64
#define DISP_set_sprite_color 65
#define DISP_set_sprite_pattern 66
#define DISP_get_sprite_color 67
#define DISP_get_sprite_pattern 68
#define DISP_set_tile_color_buf 69
#define DISP_set_tile_pattern_buffer 70

#define DISP_usr0 71
#define DISP_usr1 72
#define DISP_usr2 73
#define DISP_usr2_play 74
#define DISP_usr2_player_status 75
#define DISP_usr3 76
#define DISP_usr3_COLLISION_ALL 77
#define DISP_usr3_COLLISION_COUPLE 78
#define DISP_usr3_COLLISION_ONE 79
#define DISP_vdp_set 80

#define DISP_get_date 81
#define DISP_get_time 82
#define DISP_set_date 83
#define DISP_set_time 84

#define DISP_GET_NEXT_TEMP_STRING_ADDRESS 85

#define DISP_MR_CALL 86
#define DISP_MR_CALL_TRAP 87
#define DISP_MR_CHANGE_SGM 88
#define DISP_MR_GET_BYTE 89
#define DISP_MR_GET_DATA 90
#define DISP_MR_JUMP 91

#define DISP_XBASIC_BASE 92
#define DISP_XBASIC_CLS 93
#define DISP_XBASIC_COPY 94
#define DISP_XBASIC_COPY_FROM 95
#define DISP_XBASIC_COPY_TO 96
#define DISP_XBASIC_END 97
#define DISP_XBASIC_INIT 98
#define DISP_XBASIC_INPUT_1 99
#define DISP_XBASIC_INPUT_2 100
#define DISP_XBASIC_IREAD 101
#define DISP_XBASIC_LOCATE 102
#define DISP_XBASIC_PLAY 103
#define DISP_XBASIC_PRINT_STR 104
#define DISP_XBASIC_PUT_SPRITE 105
#define DISP_XBASIC_READ 106
#define DISP_XBASIC_RESTORE 107
#define DISP_XBASIC_SCREEN 108
#define DISP_XBASIC_WIDTH 109
#define DISP_XBASIC_SOUND 110
#define DISP_XBASIC_TAB 111
#define DISP_XBASIC_USING 112
#define DISP_XBASIC_USING_DO 113
#define DISP_XBASIC_USR 114
#define DISP_cmd_preflight_disk 115
#define DISP_cmd_fmaxfiles 116
#define DISP_cmd_fdskf 117
#define DISP_cmd_feof 118
#define DISP_cmd_floc 119
#define DISP_cmd_flof 120
#define DISP_cmd_fpos 121
#define DISP_cmd_fopen 122
#define DISP_cmd_fclose 123
#define DISP_cmd_finput 124
#define DISP_cmd_fprint 125

#define DISP_XBASIC_MULTIPLY_INTEGERS 126
#define DISP_XBASIC_DIVIDE_INTEGERS 127
#define DISP_XBASIC_ADD_FLOATS 128
#define DISP_XBASIC_SUBTRACT_FLOATS 129
#define DISP_XBASIC_MULTIPLY_FLOATS 130
#define DISP_XBASIC_DIVIDE_FLOATS 131
#define DISP_XBASIC_SIN 132
#define DISP_XBASIC_COS 133
#define DISP_XBASIC_TAN 134
#define DISP_XBASIC_ATN 135
#define DISP_XBASIC_EXP 136
#define DISP_XBASIC_LOG 137
#define DISP_XBASIC_SQR 138
#define DISP_XBASIC_RND 139
#define DISP_XBASIC_INT 140
#define DISP_XBASIC_FIX 141
#define DISP_XBASIC_SGN_INT 142
#define DISP_XBASIC_SGN_FLOAT 143
#define DISP_XBASIC_ABS_INT 144
#define DISP_XBASIC_LEFT 145
#define DISP_XBASIC_RIGHT 146
#define DISP_XBASIC_MID 147
#define DISP_XBASIC_MID_ASSIGN 148
#define DISP_XBASIC_INSTR 149
#define DISP_XBASIC_VAL 150
#define DISP_XBASIC_STRING 151
#define DISP_XBASIC_OCT_HEX_BIN 152
#define DISP_XBASIC_INKEY 153
#define DISP_XBASIC_COPY_STRING 154
#define DISP_XBASIC_COPY_STRING_TO_NULBUF 155
#define DISP_XBASIC_CONCAT_STRINGS 156
#define DISP_XBASIC_COMPARE_FLOATS_EQ 157
#define DISP_XBASIC_COMPARE_FLOATS_NE 158
#define DISP_XBASIC_COMPARE_FLOATS_GT 159
#define DISP_XBASIC_COMPARE_FLOATS_GE 160
#define DISP_XBASIC_COMPARE_FLOATS_LT 161
#define DISP_XBASIC_COMPARE_FLOATS_LE 162
#define DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ 163
#define DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_NE 164
#define DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_GT 165
#define DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_GE 166
#define DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_LT 167
#define DISP_XBASIC_COMPARE_STRING_WITH_NULBUF_LE 168
#define DISP_XBASIC_CAST_INTEGER_TO_FLOAT 169
#define DISP_XBASIC_CAST_INTEGER_TO_STRING 170
#define DISP_XBASIC_CAST_FLOAT_TO_INTEGER 171
#define DISP_XBASIC_CAST_FLOAT_TO_STRING 172
#define DISP_XBASIC_CAST_STRING_TO_FLOAT 173
#define DISP_XBASIC_POWER_FLOAT_TO_INTEGER 174
#define DISP_XBASIC_POWER_FLOAT_TO_FLOAT 175
#define DISP_XBASIC_POINT 176
#define DISP_XBASIC_LINE 177
#define DISP_XBASIC_BOX 178
#define DISP_XBASIC_BOXF 179
#define DISP_XBASIC_PSET 180
#define DISP_XBASIC_PAINT 181
#define DISP_XBASIC_CIRCLE 182
#define DISP_XBASIC_CIRCLE2 183
#define DISP_XBASIC_COLOR_RGB 184
#define DISP_XBASIC_COLOR_SPRITE 185
#define DISP_XBASIC_COLOR_SPRSTR 186
#define DISP_XBASIC_SCREEN_SPRITE 187
#define DISP_XBASIC_SPRITE_ASSIGN 188
#define DISP_XBASIC_SET_PAGE 189
#define DISP_XBASIC_SET_SCROLL 190
#define DISP_XBASIC_VDP 191
#define DISP_XBASIC_VPEEK 192
#define DISP_XBASIC_VPOKE 193
#define DISP_XBASIC_TRAP_ON 194
#define DISP_XBASIC_TRAP_OFF 195
#define DISP_XBASIC_TRAP_STOP 196
#define DISP_XBASIC_TRAP_CHECK 197
#define DISP_XBASIC_SWAP_INTEGER 198
#define DISP_XBASIC_SWAP_STRING 199
#define DISP_XBASIC_SWAP_FLOAT 200
#define DISP_WriteParamBCD 201
#define DISP_XBASIC_PRINT_TAB 202
#define DISP_XBASIC_PRINT_INT 203
#define DISP_XBASIC_PRINT_FLOAT 204
#define DISP_XBASIC_PRINT_CRLF 205
#define DISP_XBASIC_DUMMY_RET 206
#define DISP_KONAMI_PATCH_SGM_8000 207
#define DISP_KONAMI_PATCH_SGM_A000 208
#define DISP_KONAMI_PATCH_OMSX_0 209
#define DISP_KONAMI_PATCH_OMSX_1 210
#define DISP_KONAMI_PATCH_OMSX_2 211
#define DISP_KONAMI_PATCH_OMSX_3 212
#define DISP_KONAMI_PATCH_OMSX_4 213
#define DISP_KONAMI_PATCH_BUGFIX_6800 214
#define DISP_KONAMI_PATCH_BUGFIX_8000 215
#define DISP_KONAMI_PATCH_BUGFIX_A000 216
#define DISP_KONAMI_PATCH_VERIFY_READ 217
#define DISP_KONAMI_PATCH_VERIFY_WR0 218
#define DISP_KONAMI_PATCH_VERIFY_WR2 219
#define DISP_KONAMI_PATCH_VERIFY_RESTORE 220
#define DISP_ASCII16_PATCH_BUGFIX_INC1 221
#define DISP_ASCII16_PATCH_BUGFIX_NOPSEQ 222
#define DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK 223

#define DISP_ENTRIES 224

//! @remark end of header.symbols adjust
//! @}

/***************************************************************
 * @name basic
 * @brief ROM basic interpreter internal addresses and workarea
 */

#define def_IOREDIRECT \
  0x6AAA  // redirect interpreter input/output to i/o channel
#define def_IOTOSCREEN 0x4AFF  // return interpreter output to screen
#define def_BLTVM 0x0195       // ld ix, nnnn / call EXTROM
#define def_BLTMV 0x0199       // ld ix, nnnn / call EXTROM
#define def_EXTROM 0x015F
#define def_CALBAS 0x0159
#define def_CALSLT 0x001C
#define def_ENASLT 0x0024
#define def_DSKBAS 0xF37D  // disk basic routine
#define def_LDIRVM \
  0x005C  // hl = ram data address, de = vram data address, bc = length
#define def_LDIRMV \
  0x0059  // de = ram data address, hl = vram data address, bc = length
#define def_CHCOORD 0x0BF2    // in h=x, l=y; out hl = VDP address
#define def_CHCOORD2 0x0B98   // same above for MSX2
#define def_INIFNK 0x003E     // initialize function keys
#define def_DSPFNK 0x00CF     // display function key
#define def_INITXT 0x00D5     // initialize text screen
#define def_WRTVDP 0x0047     // write to VDP
#define def_CLRSPR 0x0069     // clear sprites
#define def_RSTPLT 0x0145     // Subroutine restore pallette (EXTROM->S.RSTPLT)
#define def_CHSNS 0x009C      // check keyboard status
#define def_CHGET 0x009F      // get character from keyboard
#define def_BEEP 0x00C0       // beep sound
#define def_FNKSB 0x00C9      // display function key in status bar
#define def_ERAFNK 0x00CC     // erase function key display
#define def_GTSTCK 0x00D5     // get joystick/trackball/cursor key status
#define def_GTTRIG 0x00D8     // get trigger/fire button status
#define def_GTPDL 0x00DE      // get paddle/paddles data
#define def_CHGCLR 0x0062     // change screen color
#define def_RDPSG 0x0096      // read value from PSG register
#define def_CALL_STMT 0x39C0  // call statement address table entry
#define def_COPY_STMT 0x39D8  // copy statement address table entry
#define def_SET_STMT 0x39D0   // set statement address table entry
#define def_CIRCLE_STMT 0x39A4  // circle statement address table entry

#define def_VERSION \
  0x002D  // BIOS VERSION - 0 = MSX1, 1 = MSX2, 2 = MSX2+, 3 = MSXturboR
#define def_SCRMOD 0xFCAF   // 1 - current screen mode
#define def_OLDSCR 0xFCB0   // 1 - old text screen mode
#define def_LINLEN 0xF3B0   // 1
#define def_CSRY 0xF3DC     // 1
#define def_CSRX 0xF3DD     // 1
#define def_ACPAGE 0xFAF6   // 1
#define def_DPPAGE 0xFAF5   // 1
#define def_SX 0xF562       // 2
#define def_SY 0xF564       // 2
#define def_MV_DPTR 0xF566  // 2
#define def_VM_DPTR 0xF562  // 2
#define def_DX 0xF566       // 2
#define def_DY 0xF568       // 2
#define def_NX 0xF56A       // 2
#define def_NY 0xF56C       // 2
#define def_ARGT 0xF56F     // 1
#define def_LOGOP 0xF570    // 1
#define def_LOGOPR 0xFB02   // 1 - graphical logical operation
#define def_FILNAM 0xF866   // 11
#define def_RECSIZ 0xF33D   // 2
#define def_TEMP 0xF6A7     // 2
#define def_TEMP3 0xF69D    // 2
#define def_TEMP8 0xF69F    // 2
#define def_TEMPPT 0xF678   // 2 - pointer to next free position into TEMPST
#define def_TEMPST 0xF67A   // 30 - temporary string descriptors buffer
#define def_PARM1 0xF6E8    // 100 - parameters buffer 1
#define def_PARM2 0xF750    // 100 - parameters buffer 2
#define def_DSCTMP 0xF698   // 3 - index and start address of temporary strings
#define def_ENDFOR 0xF6A1   // 2
#define def_ENDPRG 0xF40F   // fake empty line
#define def_ENDSTK 0xF6A1   // 2
#define def_STKTOP 0xF674   // 2
#define def_SAVSTK 0xf6b1   // 2
#define def_MAXFIL 0xF85F   // 1
#define def_DSKDIS 0xFD99   // 1
#define def_NLONLY 0xF87C   // 1
#define def_USRTAB 0xF39A   // 10 x 2
#define def_BDRATR 0xFCB2   // 1
#define def_ATRBYT 0xF3F2   // 1
#define def_FORCLR 0xF3E9   // 1
#define def_BAKCLR 0xF3EA   // 1
#define def_BDRCLR 0xF3EB   // 1
#define def_DATPTR 0xF6C8   // 2
#define def_DATLIN 0xF6A3   // 2
#define def_ONELIN 0xF6B9   // 2 - on error goto <line>
#define def_ERRLIN 0xF6B3   // 2 - ERL
#define def_CURLIN 0xF41C   // 2 - current line
#define def_JIFFY 0xFC9E    // 2 - time value
#define def_ERRFLG 0xF414   // 1 - error code
#define def_SUBFLG 0xF6A5   // 1
#define def_DORES 0xF664    // 1
#define def_RAMAD3 0xF344   // 1
#define def_EXPTBL 0xFCC1   // 1
#define def_HIMEM 0xFC4A    // 2
#define def_MEMSIZ 0xF672   // 2
#define def_BUF 0xF55E      // 259
#define def_KBUF 0xF41F     // 318
#define def_GRPACY 0xFCB9   // 2
#define def_GRPACX 0xFCB7   // 2
#define def_NTSC 0x002B     // bit 7 on = PAL, off = NTSC
#define def_NTMSXP 0xF417   // printer type (0=default)
#define def_CLIKSW \
  0xF3DB  //! key click (0=keyboard click off, 1=keyboard click on)
#define def_CS1200 0xF3FC
#define def_LOW 0xF406
#define def_AVCSAV 0xFAF7
#define def_RG7SAV 0xFFE6
#define def_RG8SAV 0xFFE7
#define def_RG9SAV 0xFFE8

#define def_WriteParamBCD 0x6C0B
#define def_SPADDR 0xF304   // SP register address copy
#define def_HDSKERR 0xF302  // disk error handler address copy

/***************************************************************
 * @name xbasic
 * @brief XBASIC internal addresses
 */

/*
#define def_XBASIC_MULTIPLY_INTEGERS 0x7631                             // 761b
#define def_XBASIC_DIVIDE_INTEGERS 0x7643                               // 762d
#define def_XBASIC_ADD_FLOATS 0x76D7                                    // 76C1
#define def_XBASIC_SUBTRACT_FLOATS 0x76D3                               // 76bd
#define def_XBASIC_MULTIPLY_FLOATS 0x7748                               // 7732
#define def_XBASIC_DIVIDE_FLOATS 0x782C                                 // 7775
#define def_XBASIC_COMPARE_FLOATS_EQ 0x792A                             // 7876
#define def_XBASIC_COMPARE_FLOATS_NE 0x7933                             // 787f
#define def_XBASIC_COMPARE_FLOATS_GT 0x793C                             // 7888
#define def_XBASIC_COMPARE_FLOATS_GE 0x7946                             // 7892
#define def_XBASIC_COMPARE_FLOATS_LT 0x794F                             // 789b
#define def_XBASIC_COMPARE_FLOATS_LE 0x7958                             // 78a4
#define def_XBASIC_COPY_STRING_TO_NULBUF 0x7F40                         // 7e99
#define def_XBASIC_COPY_STRING def_XBASIC_COPY_STRING_TO_NULBUF + 4     // 7e9D
#define def_XBASIC_CONCAT_STRINGS_NULBUF 0x7F99                         // 7efe
#define def_XBASIC_CONCAT_STRINGS def_XBASIC_CONCAT_STRINGS_NULBUF + 7  // 7f05
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ 0x7F55                 // 7eae
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_NE 0x7F67                 // 7ec9
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_GT 0x7F4B                 // 7ea4
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_GE 0x7F5B                 // 7eb7
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_LT 0x7F61                 // 7ec0
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_LE 0x7F6D                 // 7ed2
#define def_XBASIC_CAST_INTEGER_TO_FLOAT 0x78DF                         // 782D
#define def_XBASIC_CAST_INTEGER_TO_STRING 0x7BD1                        // 7b26
#define def_XBASIC_CAST_FLOAT_TO_INTEGER 0x7901                         // 784f
#define def_XBASIC_CAST_FLOAT_TO_STRING 0x7C2B                          // 7b80
#define def_XBASIC_CAST_STRING_TO_FLOAT 0x7EB0                          // 7e07
#define def_XBASIC_POWER_FLOAT_TO_INTEGER 0x7878                        // 77C1
#define def_XBASIC_POWER_FLOAT_TO_FLOAT 0x78BF                          // 780d
#define def_XBASIC_INKEY 0x7F01 + 6                                     // 7e5e
#define def_XBASIC_INT 0x7999                                           // 78e5
#define def_XBASIC_FIX 0x798C                                           // 78d8
#define def_XBASIC_RND 0x768E                                           // 7678
#define def_XBASIC_SIN 0x79EA                                           // 7936
#define def_XBASIC_COS 0x79E2                                           // 792e
#define def_XBASIC_TAN 0x7A41                                           // 7990
#define def_XBASIC_ATN 0x7A63                                           // 79b2
#define def_XBASIC_EXP 0x7AA8                                           // 79fa
#define def_XBASIC_LOG 0x7B01                                           // 7a53
#define def_XBASIC_SQR 0x7B60                                           // 7ab5
#define def_XBASIC_VAL 0x7EB0                                           // 7e07
#define def_XBASIC_VPEEK 0x70A3                                         // 70A1
#define def_XBASIC_VPOKE 0x70B7                                         // 70b5
#define def_XBASIC_VDP 0x7339                                           // 7337
#define def_XBASIC_STRING 0x7EF2 + 3                                    // 7e4c
#define def_XBASIC_OCT_HEX_BIN 0x7EC9 + 2                               // 7e22
#define def_XBASIC_POINT 0x6FA9                                         // 6fa7
#define def_XBASIC_LEFT 0x7E42                                          // 7d99
#define def_XBASIC_RIGHT 0x7E49                                         // 7da0
#define def_XBASIC_MID 0x7E5A                                           // 7db1
#define def_XBASIC_MID_ASSIGN 0x7E81                                    // 7dd8
#define def_XBASIC_INSTR 0x7F15                                         // 7e6c
#define def_XBASIC_SPRITE_ASSIGN 0x7145                                 // 7143
// #define def_XBASIC_PUT_SPRITE                      0x71BF   //71BD
#define def_XBASIC_SET_PAGE 0x70CC      // 70CA
#define def_XBASIC_SET_SCROLL 0x7004    // 7002
#define def_XBASIC_COLOR_RGB 0x710E     // 710C
#define def_XBASIC_COLOR_SPRITE 0x7189  // 7187
#define def_XBASIC_COLOR_SPRSTR 0x7194  // 7192
#define def_XBASIC_PRINT_TAB 0x74F3     // 74ea
#define def_XBASIC_PRINT_INT 0x7508     // 74ff
#define def_XBASIC_PRINT_FLOAT 0x750D   // 7504
#define def_XBASIC_PRINT_CRLF 0x74F7    // 74ee
#define def_XBASIC_LINE 0x6DA9          // 6DA7
#define def_XBASIC_BOX 0x6D4B           // 6D49
#define def_XBASIC_BOXF 0x6E29          // 6E27
#define def_XBASIC_PSET 0x6F73          // 6F71
#define def_XBASIC_PAINT 0x74BC         // 74B3
#define def_XBASIC_CIRCLE 0x74DC        // 74D3
#define def_XBASIC_CIRCLE2 0x74EE
#define def_XBASIC_SCREEN_SPRITE 0x70BE  // 70bc
#define def_XBASIC_TRAP_ON 0x6C8B        // 6c89
#define def_XBASIC_TRAP_OFF 0x6C9E       // 6c9c
#define def_XBASIC_TRAP_STOP 0x6CA7      // 6ca5
#define def_XBASIC_TRAP_CHECK 0x6C27     // 6c25
#define def_XBASIC_SWAP_INTEGER 0x6BF7   // 6bf5
#define def_XBASIC_SWAP_STRING 0x6BFB    // 6bf9
#define def_XBASIC_SWAP_FLOAT 0x6BFF     // 6bfd

#define def_XBASIC_SGN_INT 0x6BD5    //-
#define def_XBASIC_SGN_FLOAT 0x6BE1  //-
#define def_XBASIC_ABS_INT 0x6BED    //-
#define def_XBASIC_DUMMY_RET 0x6C26  // 368D
*/

#endif  // COMPILER_HOOKS_H_INCLUDED
