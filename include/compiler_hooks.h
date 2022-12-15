#ifndef COMPILER_HOOKS_H_INCLUDED
#define COMPILER_HOOKS_H_INCLUDED

//-- bios workareas

#define def_DAC         0xF7F6
#define def_ARG         0xF847
#define def_ARG2        (def_ARG + 2)
#define def_VALTYP      0xF663
#define def_PRMFLG      0xF7B4

//-- compiler workareas

#define def_PLYBUF          0xEF00
#define def_SPRTBL          (def_PLYBUF - (32*5))    // 32 sprites * (test, x0, x1, y0, y1)
#define def_SPRSIZ          (def_SPRTBL - 1)

#define def_RAM_INIT        0x0037
#define def_RAM_BOTTOM      0xC000 + def_RAM_INIT
#define def_RAM_TOP         (def_SPRSIZ - 1)
#define def_RAM_SIZE        (def_RAM_TOP - def_RAM_BOTTOM)
#define def_RAM_BUFSIZ      0x0880     // 2176 bytes of font buffer in RAM

#define def_SLTSTR          0xC010     // 1
#define def_SLTAD2          0xC01D     // 1
#define def_SOMODE          0xC024     // 1 - screen output mode (0=text, 1=graphical, 2=tiled)
#define def_HEAPSTR         0xC025     // 2 - heap start address
#define def_HEAPSIZ         0xC027     // 2 - heap size
#define def_TMPSTR_IND      0xC029     // 1 - temporary string index
#define def_TMPSTR_ADDR     0xC02A     // 2 - temporary string start address
#define def_TMPSTR_BUF      0xC02C     // 2 - temporary string next pointer
#define def_FONT_OLD        0xC02F     // 2 - old bios font address
#define def_FONT_NEW        0xC031     // 2 - new font space address

#define def_MR_TRAP_FLAG    0xFC82
#define def_MR_TRAP_SEGMS   def_MR_TRAP_FLAG+1

//-- start of header.symbols adjust

#define def_wrapper_routines_map_start   0x40C0

#define def_castParamFloatInt            def_wrapper_routines_map_start

#define def_cmd_clrkey	                 (def_castParamFloatInt + 3)
#define def_cmd_clrscr	                 (def_cmd_clrkey + 3)
#define def_cmd_disscr	                 (def_cmd_clrscr + 3)
#define def_cmd_draw	                 (def_cmd_disscr + 3)
#define def_cmd_enascr	                 (def_cmd_draw + 3)
#define def_cmd_keyclkoff	             (def_cmd_enascr + 3)
#define def_cmd_mute	                 (def_cmd_keyclkoff + 3)
#define def_cmd_play	                 (def_cmd_mute + 3)

#define def_cmd_plyload	                 (def_cmd_play + 3)
#define def_cmd_plyloop	                 (def_cmd_plyload + 3)
#define def_cmd_plymute	                 (def_cmd_plyloop + 3)
#define def_cmd_plyplay	                 (def_cmd_plymute + 3)
#define def_cmd_plyreplay	             (def_cmd_plyplay + 3)
#define def_cmd_plysong                  (def_cmd_plyreplay + 3)
#define def_cmd_plysound	             (def_cmd_plysong + 3)

#define def_cmd_ramtoram	             (def_cmd_plysound + 3)
#define def_cmd_ramtovram             	 (def_cmd_ramtoram + 3)
#define def_cmd_restore                  (def_cmd_ramtovram + 3)
#define def_cmd_runasm	                 (def_cmd_restore + 3)

#define def_cmd_screen_copy              (def_cmd_runasm + 3)
#define def_cmd_screen_load              (def_cmd_screen_copy + 3)
#define def_cmd_screen_paste             (def_cmd_screen_load + 3)

#define def_cmd_setfnt	                 (def_cmd_screen_paste + 3)
#define def_cmd_turbo                    (def_cmd_setfnt + 3)
#define def_cmd_updfntclr                (def_cmd_turbo + 3)
#define def_cmd_vramtoram	             (def_cmd_updfntclr + 3)
#define def_cmd_wrtchr	                 (def_cmd_vramtoram + 3)
#define def_cmd_wrtclr	                 (def_cmd_wrtchr + 3)
#define def_cmd_wrtfnt	                 (def_cmd_wrtclr + 3)
#define def_cmd_wrtscr	                 (def_cmd_wrtfnt + 3)
#define def_cmd_wrtspr                   (def_cmd_wrtscr + 3)
#define def_cmd_wrtspratr	             (def_cmd_wrtspr + 3)
#define def_cmd_wrtsprclr	             (def_cmd_wrtspratr + 3)
#define def_cmd_wrtsprpat	             (def_cmd_wrtsprclr + 3)
#define def_cmd_wrtvram	                 (def_cmd_wrtsprpat + 3)

#define def_floatNEG                     (def_cmd_wrtvram + 3)
#define def_tileAddress                  (def_floatNEG + 3)

#define def_intCompareAND                (def_tileAddress + 3)
#define def_intCompareEQ                 (def_intCompareAND + 3)
#define def_intCompareGE                 (def_intCompareEQ + 3)
#define def_intCompareGT                 (def_intCompareGE + 3)
#define def_intCompareLE                 (def_intCompareGT + 3)
#define def_intCompareLT                 (def_intCompareLE + 3)
#define def_intCompareNE                 (def_intCompareLT + 3)
#define def_intCompareNOT                (def_intCompareNE + 3)
#define def_intCompareOR                 (def_intCompareNOT + 3)
#define def_intCompareXOR                (def_intCompareOR + 3)
#define def_intNEG                       (def_intCompareXOR + 3)
#define def_intSHL                       (def_intNEG + 3)
#define def_intSHR                       (def_intSHL + 3)

#define def_player_initialize            (def_intSHR + 3)
#define def_player_unhook                (def_player_initialize + 3)

#define def_set_tile_color               (def_player_unhook + 3)
#define def_set_tile_pattern             (def_set_tile_color + 3)

#define def_usr0	                     (def_set_tile_pattern + 3)
#define def_usr1		                 (def_usr0 + 3)
#define def_usr2		                 (def_usr1 + 3)
#define def_usr2_play	                 (def_usr2 + 3)
#define def_usr2_player_status           (def_usr2_play + 3)
#define def_usr3		                 (def_usr2_player_status + 3)
#define def_usr3_COLLISION_ALL           (def_usr3 + 3)
#define def_usr3_COLLISION_COUPLE        (def_usr3_COLLISION_ALL + 3)
#define def_usr3_COLLISION_ONE           (def_usr3_COLLISION_COUPLE + 3)

#define def_GET_NEXT_TEMP_STRING_ADDRESS (def_usr3_COLLISION_ONE + 3)

#define def_MR_CALL                      (def_GET_NEXT_TEMP_STRING_ADDRESS + 3)
#define def_MR_CALL_TRAP                 (def_MR_CALL + 3)
#define def_MR_CHANGE_SGM                (def_MR_CALL_TRAP + 3)
#define def_MR_GET_BYTE                  (def_MR_CHANGE_SGM + 3)
#define def_MR_GET_DATA                  (def_MR_GET_BYTE + 3)
#define def_MR_JUMP                      (def_MR_GET_DATA + 3)

#define def_XBASIC_BASE                  (def_MR_JUMP + 3)
#define def_XBASIC_CLS                   (def_XBASIC_BASE + 3)
#define def_XBASIC_COPY                  (def_XBASIC_CLS + 3)
#define def_XBASIC_COPY_FROM             (def_XBASIC_COPY + 3)
#define def_XBASIC_COPY_TO               (def_XBASIC_COPY_FROM + 3)
#define def_XBASIC_END                   (def_XBASIC_COPY_TO + 3)
#define def_XBASIC_INIT                  (def_XBASIC_END + 3)
#define def_XBASIC_INPUT_1               (def_XBASIC_INIT + 3)
#define def_XBASIC_INPUT_2               (def_XBASIC_INPUT_1 + 3)
#define def_XBASIC_IREAD                 (def_XBASIC_INPUT_2 + 3)
#define def_XBASIC_LOCATE                (def_XBASIC_IREAD + 3)
#define def_XBASIC_PLAY                  (def_XBASIC_LOCATE + 3)
#define def_XBASIC_PRINT_STR             (def_XBASIC_PLAY + 3)
#define def_XBASIC_PUT_SPRITE            (def_XBASIC_PRINT_STR + 3)
#define def_XBASIC_READ                  (def_XBASIC_PUT_SPRITE + 3)
#define def_XBASIC_RESTORE               (def_XBASIC_READ + 3)
#define def_XBASIC_SCREEN                (def_XBASIC_RESTORE + 3)
#define def_XBASIC_SOUND                 (def_XBASIC_SCREEN + 3)
#define def_XBASIC_TAB                   (def_XBASIC_SOUND + 3)
#define def_XBASIC_USING                 (def_XBASIC_TAB + 3)
#define def_XBASIC_USING_DO              (def_XBASIC_USING + 3)
#define def_XBASIC_USR                   (def_XBASIC_USING_DO + 3)

//-- end of header.symbols adjust

#define def_CLEAR         0x62A1     // initialize the interpreter
#define def_OPEN          0x6AFA
#define def_FILESPEC      0x6A1F     // 0x6A0E
#define def_DEVICESPEC    0x6F15
#define def_CLOSE         0x6B24
#define def_CLOSE_ALL     0x6C1C
#define def_MAXFILES      0x7E6B
#define def_IOREDIRECT    0x6AAA     // redirect interpreter input/output to i/o channel
#define def_IOTOSCREEN    0x4AFF     // return interpreter output to screen
#define def_BLTVM         0x0195     // ld ix, nnnn / call EXTROM
#define def_BLTMV         0x0199     // ld ix, nnnn / call EXTROM
#define def_EXTROM	      0x015F
#define def_CALBAS        0x0159
#define def_CALSLT        0x001C
#define def_ENASLT        0x0024
#define def_DSKBAS	      0xF37D     // disk basic routine
#define def_LDIRVM        0x005C     // hl = ram data address, de = vram data address, bc = length
#define def_LDIRMV        0x0059     // de = ram data address, hl = vram data address, bc = length
#define def_CHCOORD       0x0BF2     // in h=x, l=y; out hl = VDP address
#define def_CHCOORD2      0x0B98     // same above for MSX2
#define def_INIFNK        0x003E     // initialize function keys
#define def_DSPFNK        0x00CF     // display function key
#define def_INITXT        0x00D5     // initialize text screen
#define def_WRTVDP        0x0047     // write to VDP
#define def_CLRSPR        0x0069     // clear sprites

#define def_CALL_STMT     0x39C0     // call statement address table entry
#define def_COPY_STMT     0x39D8     // copy statement address table entry
#define def_SET_STMT      0x39D0     // set statement address table entry
#define def_CIRCLE_STMT   0x39A4     // circle statement address table entry

#define def_VERSION       0x002D     // BIOS VERSION - 0 = MSX1, 1 = MSX2, 2 = MSX2+, 3 = MSXturboR
#define def_SCRMOD        0xFCAF     // 1 - current screen mode
#define def_OLDSCR        0xFCB0     // 1 - old text screen mode
#define def_LINLEN        0xF3B0     // 1
#define def_CSRY          0xF3DC     // 1
#define def_CSRX          0xF3DD     // 1
#define def_ACPAGE        0xFAF6     // 1
#define def_DPPAGE        0xFAF5     // 1
#define def_SX            0xF562     // 2
#define def_SY            0xF564     // 2
#define def_MV_DPTR       0xF566     // 2
#define def_VM_DPTR       0xF562     // 2
#define def_DX            0xF566     // 2
#define def_DY            0xF568     // 2
#define def_NX            0xF56A     // 2
#define def_NY            0xF56C     // 2
#define def_ARGT          0xF56F     // 1
#define def_LOGOP         0xF570     // 1
#define def_LOGOPR        0xFB02     // 1 - graphical logical operation
#define def_FILNAM        0xF866     // 11
#define def_RECSIZ        0xF33D     // 2
#define def_TEMP          0xF6A7     // 2
#define def_TEMP3         0xF69D     // 2
#define def_TEMP8         0xF69F     // 2
#define def_TEMPPT        0xF678     // 2 - pointer to next free position into TEMPST
#define def_TEMPST        0xF67A     // 30 - temporary string descriptors buffer
#define def_DSCTMP        0xF698     // 3 - index and start address of temporary strings
#define def_ENDFOR        0xF6A1     // 2
#define def_ENDPRG        0xF40F     // fake empty line
#define def_ENDSTK        0xF6A1     // 2
#define def_STKTOP        0xF674     // 2
#define def_SAVSTK        0xf6b1     // 2
#define def_MAXFIL        0xF85F     // 1
#define def_NLONLY        0xF87C     // 1
#define def_USRTAB        0xF39A     // 10 x 2
#define def_BDRATR        0xFCB2     // 1
#define def_ATRBYT        0xF3F2     // 1
#define def_FORCLR        0xF3E9     // 1
#define def_BAKCLR        0xF3EA     // 1
#define def_BDRCLR        0xF3EB     // 1
#define def_DATPTR        0xF6C8     // 2
#define def_DATLIN        0xF6A3     // 2
#define def_ONELIN        0xF6B9     // 2 - on error goto <line>
#define def_ERRLIN        0xF6B3     // 2 - ERL
#define def_CURLIN        0xF41C     // 2 - current line
#define def_SUBFLG        0xF6A5     // 1
#define def_DORES         0xF664     // 1
#define def_RAMAD3        0xF344     // 1
#define def_EXPTBL        0xFCC1     // 1
#define def_HIMEM         0xFC4A     // 2
#define def_BUF           0xF55E     // 259
#define def_KBUF          0xF41F     // 318
#define def_GRPACY        0xFCB9     // 2
#define def_GRPACX        0xFCB7     // 2
#define def_NTSC          0x002B     // bit 7 on = PAL, off = NTSC
#define def_NTMSXP        0xF417     // printer type (0=default)
#define def_CLIKSW        0xF3DB     // key click (0=keyboard click off, 1=keyboard click on)
#define def_CS1200        0xF3FC
#define def_LOW           0xF406
#define def_AVCSAV	      0xFAF7
#define def_RG7SAV        0xFFE6
#define def_RG8SAV        0xFFE7
#define def_RG9SAV        0xFFE8

#define def_WriteParamBCD                          0x6C0B

#define def_XBASIC_MULTIPLY_INTEGERS               0x7631   //761b
#define def_XBASIC_DIVIDE_INTEGERS                 0x7643   //762d
#define def_XBASIC_ADD_FLOATS                      0x76D7   //76C1
#define def_XBASIC_SUBTRACT_FLOATS                 0x76D3   //76bd
#define def_XBASIC_MULTIPLY_FLOATS                 0x7748   //7732
#define def_XBASIC_DIVIDE_FLOATS                   0x782C   //7775
#define def_XBASIC_COMPARE_FLOATS_EQ               0x792A   //7876
#define def_XBASIC_COMPARE_FLOATS_NE               0x7933   //787f
#define def_XBASIC_COMPARE_FLOATS_GT               0x793C   //7888
#define def_XBASIC_COMPARE_FLOATS_GE               0x7946   //7892
#define def_XBASIC_COMPARE_FLOATS_LT               0x794F   //789b
#define def_XBASIC_COMPARE_FLOATS_LE               0x7958   //78a4
#define def_XBASIC_COPY_STRING_TO_NULBUF           0x7F40   //7e99
#define def_XBASIC_COPY_STRING                     def_XBASIC_COPY_STRING_TO_NULBUF+4       //7e9D
#define def_XBASIC_CONCAT_STRINGS_NULBUF           0x7F99   //7efe
#define def_XBASIC_CONCAT_STRINGS                  def_XBASIC_CONCAT_STRINGS_NULBUF+7       //7f05
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_EQ   0x7F55   //7eae
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_NE   0x7F67   //7ec9
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_GT   0x7F4B   //7ea4
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_GE   0x7F5B   //7eb7
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_LT   0x7F61   //7ec0
#define def_XBASIC_COMPARE_STRING_WITH_NULBUF_LE   0x7F6D   //7ed2
#define def_XBASIC_CAST_INTEGER_TO_FLOAT           0x78DF   //782D
#define def_XBASIC_CAST_INTEGER_TO_STRING          0x7BD1   //7b26
#define def_XBASIC_CAST_FLOAT_TO_INTEGER           0x7901   //784f
#define def_XBASIC_CAST_FLOAT_TO_STRING            0x7C2B   //7b80
#define def_XBASIC_CAST_STRING_TO_FLOAT            0x7EB0   //7e07
#define def_XBASIC_POWER_FLOAT_TO_INTEGER          0x7878   //77C1
#define def_XBASIC_POWER_FLOAT_TO_FLOAT            0x78BF   //780d
#define def_XBASIC_INKEY                           0x7F01+6 //7e5e
#define def_XBASIC_INT                             0x7999   //78e5
#define def_XBASIC_FIX                             0x798C   //78d8
#define def_XBASIC_RND                             0x768E   //7678
#define def_XBASIC_SIN                             0x79EA   //7936
#define def_XBASIC_COS                             0x79E2   //792e
#define def_XBASIC_TAN                             0x7A41   //7990
#define def_XBASIC_ATN                             0x7A63   //79b2
#define def_XBASIC_EXP                             0x7AA8   //79fa
#define def_XBASIC_LOG                             0x7B01   //7a53
#define def_XBASIC_SQR                             0x7B60   //7ab5
#define def_XBASIC_VAL                             0x7EB0   //7e07
#define def_XBASIC_VPEEK                           0x70A3   //70A1
#define def_XBASIC_VPOKE                           0x70B7   //70b5
#define def_XBASIC_VDP                             0x7339   //7337
#define def_XBASIC_STRING                          0x7EF2+3 //7e4c
#define def_XBASIC_OCT_HEX_BIN                     0x7EC9+2 //7e22
#define def_XBASIC_POINT                           0x6FA9   //6fa7
#define def_XBASIC_LEFT                            0x7E42   //7d99
#define def_XBASIC_RIGHT                           0x7E49   //7da0
#define def_XBASIC_MID                             0x7E5A   //7db1
#define def_XBASIC_MID_ASSIGN                      0x7E81   //7dd8
#define def_XBASIC_INSTR                           0x7F15   //7e6c
#define def_XBASIC_SPRITE_ASSIGN                   0x7145   //7143
//#define def_XBASIC_PUT_SPRITE                      0x71BF   //71BD
#define def_XBASIC_SET_PAGE                        0x70CC   //70CA
#define def_XBASIC_SET_SCROLL                      0x7004   //7002
#define def_XBASIC_COLOR_RGB                       0x710E   //710C
#define def_XBASIC_COLOR_SPRITE                    0x7189   //7187
#define def_XBASIC_COLOR_SPRSTR                    0x7194   //7192
#define def_XBASIC_PRINT_TAB                       0x74F3   //74ea
#define def_XBASIC_PRINT_INT                       0x7508   //74ff
#define def_XBASIC_PRINT_FLOAT                     0x750D   //7504
#define def_XBASIC_PRINT_CRLF                      0x74F7   //74ee
#define def_XBASIC_LINE                            0x6DA9   //6DA7
#define def_XBASIC_BOX                             0x6D4B   //6D49
#define def_XBASIC_BOXF                            0x6E29   //6E27
#define def_XBASIC_PSET                            0x6F73   //6F71
#define def_XBASIC_PAINT                           0x74BC   //74B3
#define def_XBASIC_CIRCLE                          0x74DC   //74D3
#define def_XBASIC_CIRCLE2                         0x74EE
#define def_XBASIC_SCREEN_SPRITE                   0x70BE   //70bc
#define def_XBASIC_TRAP_ON                         0x6C8B   //6c89
#define def_XBASIC_TRAP_OFF                        0x6C9E   //6c9c
#define def_XBASIC_TRAP_STOP                       0x6CA7   //6ca5
#define def_XBASIC_TRAP_CHECK                      0x6C27   //6c25
#define def_XBASIC_SWAP_INTEGER                    0x6BF7   //6bf5
#define def_XBASIC_SWAP_STRING                     0x6BFB   //6bf9
#define def_XBASIC_SWAP_FLOAT                      0x6BFF   //6bfd

#define def_XBASIC_SGN_INT                         0x6BD5   //-
#define def_XBASIC_SGN_FLOAT                       0x6BE1   //-
#define def_XBASIC_ABS_INT                         0x6BED   //-
#define def_XBASIC_DUMMY_RET                       0x6C26   // 368D

#endif // COMPILER_HOOKS_H_INCLUDED
