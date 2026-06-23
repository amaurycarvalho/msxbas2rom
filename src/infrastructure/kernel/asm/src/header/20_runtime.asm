; ------------------------------------------------------------------------------------------------------
; VIRTUAL DISPATCH TABLE (compile-time oracle only, not written to ROM)
; ------------------------------------------------------------------------------------------------------

	org 0000h

wrapper_routines_map_table:
  dw castParamFloatInt
  dw cmd_clrkey
  dw cmd_clrscr
  dw cmd_disscr
  dw cmd_draw
  dw cmd_enascr
  dw cmd_keyclkoff
  dw cmd_mute
  dw cmd_play
  dw cmd_pad

  dw cmd_plyload
  dw cmd_plyloop
  dw cmd_plymute
  dw cmd_plyplay
  dw cmd_plyreplay
  dw cmd_plysong
  dw cmd_plysound

  dw cmd_ramtoram
  dw cmd_ramtovram
  dw cmd_rsctoram
  dw cmd_restore
  dw cmd_runasm

  dw cmd_screen_copy
  dw cmd_screen_load
  dw cmd_screen_paste

  dw cmd_mtf

  dw cmd_setfnt
  dw cmd_turbo
  dw cmd_updfntclr
  dw cmd_vramtoram
  dw cmd_wrtchr
  dw cmd_wrtclr
  dw cmd_wrtfnt
  dw cmd_wrtscr
  dw cmd_wrtspr
  dw cmd_wrtspratr
  dw cmd_wrtsprclr
  dw cmd_wrtsprpat
  dw cmd_wrtvram
  dw cmd_page

  dw floatNeg
  dw gfxTileAddress

  dw intCompareAND
  dw intCompareEQ
  dw intCompareGE
  dw intCompareGT
  dw intCompareLE
  dw intCompareLT
  dw intCompareNE
  dw intCompareNOT
  dw intCompareOR
  dw intCompareXOR
  dw intNeg
  dw intSHL
  dw intSHR

  dw player.initialize
  dw player.unhook

  dw set_tile_flip
  dw set_tile_rotate
  dw set_tile_color
  dw set_tile_pattern
  dw get_tile_color
  dw get_tile_pattern
  dw set_sprite_flip
  dw set_sprite_rotate
  dw set_sprite_color
  dw set_sprite_pattern
  dw get_sprite_color
  dw get_sprite_pattern
  dw set_tile_color_buf
  dw set_tile_pattern_buffer

  dw usr0
  dw usr1
  dw usr2
  dw usr2_play
  dw usr2_player_status
  dw usr3
  dw usr3.COLLISION_ALL
  dw usr3.COLLISION_COUPLE
  dw usr3.COLLISION_ONE
  dw gfxVDP.set

  dw cmd_get_date
  dw cmd_get_time
  dw cmd_set_date
  dw cmd_set_time

  dw GET_NEXT_TEMP_STRING_ADDRESS

  dw MR_CALL
  dw MR_CALL_TRAP
  dw MR_CHANGE_SGM
  dw MR_GET_BYTE
  dw MR_GET_DATA
  dw MR_JUMP

  dw XBASIC_BASE
  dw XBASIC_CLS
  dw XBASIC_COPY
  dw XBASIC_COPY_FROM
  dw XBASIC_COPY_TO
  dw XBASIC_END
  dw XBASIC_INIT
  dw XBASIC_INPUT_1
  dw XBASIC_INPUT_2
  dw XBASIC_IREAD
  dw XBASIC_LOCATE
  dw XBASIC_PLAY
  dw XBASIC_PRINT_STR
  dw XBASIC_PUT_SPRITE
  dw XBASIC_READ
  dw XBASIC_RESTORE
  dw XBASIC_SCREEN
  dw XBASIC_WIDTH
  dw XBASIC_SOUND
  dw XBASIC_TAB
  dw XBASIC_USING
  dw XBASIC_USING.do
  dw XBASIC_USR
  dw cmd_preflight_disk
  dw cmd_fmaxfiles
  dw cmd_fdskf
  dw cmd_feof
  dw cmd_floc
  dw cmd_flof
  dw cmd_fpos
  dw cmd_fopen
  dw cmd_fclose
  dw cmd_finput
  dw cmd_fprint

  dw XBASIC_MULTIPLY_INTEGERS
  dw XBASIC_DIVIDE_INTEGERS
  dw XBASIC_ADD_FLOATS
  dw XBASIC_SUBTRACT_FLOATS
  dw XBASIC_MULTIPLY_FLOATS
  dw XBASIC_DIVIDE_FLOATS
  dw XBASIC_SIN
  dw XBASIC_COS
  dw XBASIC_TAN
  dw XBASIC_ATN
  dw XBASIC_EXP
  dw XBASIC_LOG
  dw XBASIC_SQR
  dw XBASIC_RND
  dw XBASIC_INT
  dw XBASIC_FIX
  dw XBASIC_SGN_INT
  dw XBASIC_SGN_FLOAT
  dw XBASIC_ABS_INT
  dw XBASIC_LEFT
  dw XBASIC_RIGHT
  dw XBASIC_MID
  dw XBASIC_MID_ASSIGN
  dw XBASIC_INSTR
  dw XBASIC_VAL
  dw XBASIC_STRING
  dw XBASIC_OCT_HEX_BIN
  dw XBASIC_INKEY
  dw XBASIC_COPY_STRING
  dw XBASIC_COPY_STRING_TO_NULBUF
  dw XBASIC_CONCAT_STRINGS
  dw XBASIC_COMPARE_FLOATS_EQ
  dw XBASIC_COMPARE_FLOATS_NE
  dw XBASIC_COMPARE_FLOATS_GT
  dw XBASIC_COMPARE_FLOATS_GE
  dw XBASIC_COMPARE_FLOATS_LT
  dw XBASIC_COMPARE_FLOATS_LE
  dw XBASIC_COMPARE_STRING_WITH_NULBUF_EQ
  dw XBASIC_COMPARE_STRING_WITH_NULBUF_NE
  dw XBASIC_COMPARE_STRING_WITH_NULBUF_GT
  dw XBASIC_COMPARE_STRING_WITH_NULBUF_GE
  dw XBASIC_COMPARE_STRING_WITH_NULBUF_LT
  dw XBASIC_COMPARE_STRING_WITH_NULBUF_LE
  dw XBASIC_CAST_INTEGER_TO_FLOAT
  dw XBASIC_CAST_INTEGER_TO_STRING
  dw XBASIC_CAST_FLOAT_TO_INTEGER
  dw XBASIC_CAST_FLOAT_TO_STRING
  dw XBASIC_CAST_STRING_TO_FLOAT
  dw XBASIC_POWER_FLOAT_TO_INTEGER
  dw XBASIC_POWER_FLOAT_TO_FLOAT
  dw XBASIC_POINT
  dw XBASIC_LINE
  dw XBASIC_BOX
  dw XBASIC_BOXF
  dw XBASIC_PSET
  dw XBASIC_PAINT
  dw XBASIC_CIRCLE
  dw XBASIC_CIRCLE2
  dw XBASIC_COLOR_RGB
  dw XBASIC_COLOR_SPRITE
  dw XBASIC_COLOR_SPRSTR
  dw XBASIC_SCREEN_SPRITE
  dw XBASIC_SPRITE_ASSIGN
  dw XBASIC_SET_PAGE
  dw XBASIC_SET_SCROLL
  dw XBASIC_VDP
  dw XBASIC_VPEEK
  dw XBASIC_VPOKE
  dw XBASIC_TRAP_ON
  dw XBASIC_TRAP_OFF
  dw XBASIC_TRAP_STOP
  dw XBASIC_TRAP_CHECK
  dw XBASIC_SWAP_INTEGER
  dw XBASIC_SWAP_STRING
  dw XBASIC_SWAP_FLOAT
  dw WriteParamBCD
  dw XBASIC_PRINT_TAB
  dw XBASIC_PRINT_INT
  dw XBASIC_PRINT_FLOAT
  dw XBASIC_PRINT_CRLF
  dw XBASIC_DUMMY_RET
  
  dw konami_patch_sgm_8000
  dw konami_patch_sgm_A000
  dw konami_patch_omsx_0
  dw konami_patch_omsx_1
  dw konami_patch_omsx_2
  dw konami_patch_omsx_3
  dw konami_patch_omsx_4
  dw konami_patch_bugfix_6800
  dw konami_patch_bugfix_8000
  dw konami_patch_bugfix_A000
  dw konami_patch_verify_read
  dw konami_patch_verify_wr0
  dw konami_patch_verify_wr2
  dw konami_patch_verify_restore
  
  dw ascii16_patch_bugfix_inc1
  dw ascii16_patch_bugfix_nopseq
  dw ascii16x_patch_bugfix_ab_check

	DEFS 0x4000 - $

; ------------------------------------------------------------------------------------------------------
; INITIALIZE (actual kernel, written to ROM)
; ------------------------------------------------------------------------------------------------------

; ### ROM header ###

	db "AB"		    ; ID for auto-executable ROM
	dw INIT1	    ; Main program execution address.
  dw 0x0000	    ; STATEMENT
	dw 0		      ; DEVICE
	dw 0		      ; TEXT
  db 'MSXB2R'   ; MSXBAS2ROM signature
	;dw 0,0,0	    ; Reserved
	ds 8, 0x00      ; ASCII16X autodetection signature space (filled at build time for ASCII16X mode)

INIT1:	                    ; Program code entry point label - 'c' has rom slot id

    ld a, (EXPTBL)
    ld h, 0x00
    call SUB_ENASLT         ; page 0 - enable bios rom (Victor HC-95/Sony HB-F500 bug fix on ExecROM with disk driver)

    ld h, 0x40              ; page 1 - this rom
    call page.getslt

    ld (SLTSTR), a          ; start slot

    ld hl, pre_start.hook_data
    ld de, HSTKE
    ld bc, 5
    ldir

    ld (HSTKE+1), a         ; program rom slot

    ret

pre_start.hook_data:
  db 0xF7, 0x00             ; CALLF
  dw start
  db 0xC9

start:
  call verify.slots

  call select_rom_on_page_2

megarom_ascii8_bug_fix:
ascii16x_patch_bugfix_ab_check:
  ld a, (0x8000)            ; ASCII8 MegaROM loading bug fix
  cp 0x41
  jr nz, clear_basic_environment
    ld a, (0x8001)
    cp 0x42
    jr nz, clear_basic_environment
      ld a, 1
konami_patch_bugfix_6800:
      ld (0x6800), a
ascii16_patch_bugfix_inc1:
      inc a
konami_patch_bugfix_8000:
      ld (0x7000), a
ascii16_patch_bugfix_nopseq:
      inc a
konami_patch_bugfix_A000:
      ld (0x7800), a

clear_basic_environment:
  ld a, 0xC9
  ld (HSTKE), a              ; remove the hook
  ld (HSTKE+1), a
  ld (HSTKE+2), a
  ld (HSTKE+3), a
  ld (HSTKE+4), a

  xor a
  ld (BUF), a                ; clear BUF (keyboard)
  ld (ENDBUF), a             ; endmarker for BUF
  ld (KBUF), a               ; clear KBUF (interpreter command line)
  ld (AUTFLG), a             ; quit auto linenumber mode
  ld (LPTPOS), a             ; printer position
  ld (TRCFLG), a             ; disable trace
  ld (PRTFLG), a             ; output to screen
  ld hl, ENDPRG+1
  ld (hl), a                 ; fake line
  inc hl
  ld (hl), a                 ; fake line
  inc hl
  ld (hl), a                 ; fake line
  inc hl
  ld (hl), a                 ; fake line
  ld (TRGFLG), a             ; joysticks ports
  ld (SOMODE), a             ; default screen output mode = text
  ld (MAXFIL), a             ; reset max files
  ld (NLONLY), a             ; reset i/o buffers

  ld A,','
  ld (BUFMIN),A              ; dummy prefix for BUF

  ld A,":"
  ld (KBFMIN),A              ; dummy prefix for KBUF
  ld (ENDPRG),a              ; fake line

  ld a, 0xFF
  ld (PTRFLG), a             ; line number converted to pointer (0=false)
  ld a, (STARTUP_CFG_FILEIO)
  and a
  jr z, clear_basic_environment.non_disk_mode

clear_basic_environment.disk_mode:
  xor a
  ld (DSKDIS), a             ; enable disks
  ld a, 1
  ld (MAXFIL), a             ; default MAXFILES for disk mode
  jr clear_basic_environment.disk_mode_done

clear_basic_environment.non_disk_mode:
  ld a, 0xFF
  ld (DSKDIS), a             ; disable disks

clear_basic_environment.disk_mode_done:
  call KILBUF
  call INITXT                ; screen 0
  call SUB_CLRSPR            ; clear sprites

run_user_basic_code_on_rom:
  ld hl, BASMEM              ; variable starts at page 3 (0xC000...)
  ld (VARTAB), hl
  ld (BOTTOM), hl

  ld hl, (BASTEXT)           ; start of user basic code
  inc hl
  ld (TXTTAB), hl
  ld a, h
  ld (BASROM), a             ; basic code location (0=RAM, not 0 = ROM)

  ld a, (STARTUP_CFG_FILEIO)
  and a
  jr z, run_user_basic_code_on_rom.non_disk_himem

run_user_basic_code_on_rom.disk_himem:
  ; honor Disk ROM/BIOS negotiated top-of-memory to avoid trampling disk work area
  ld hl, (HIMEM)
  jr run_user_basic_code_on_rom.himem_done

run_user_basic_code_on_rom.non_disk_himem:
  ld HL,VARWRK               ; non-disk mode top memory (0xF380)

run_user_basic_code_on_rom.himem_done:
  ld (HIMEM),HL              ; highest BASIC RAM address
  ld (MEMSIZ),hl
  and a
  ld bc, 200                 ; default stack margin
  jr z, run_user_basic_code_on_rom.stack_margin_done
  ld bc, 256                 ; safer margin for disk mode

run_user_basic_code_on_rom.stack_margin_done:
  and a
  sbc hl, bc
  ld (STKTOP), hl

  jp 0x8010      	         ; Jump to above page (start code)
