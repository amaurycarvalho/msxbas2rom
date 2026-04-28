; ------------------------------------------------------------------------------------------------------
; INITIALIZE
; ------------------------------------------------------------------------------------------------------

	org 4000h

; ### ROM header ###

	db "AB"		    ; ID for auto-executable ROM
	dw INIT1	    ; Main program execution address.
  dw 0x0000	    ; STATEMENT
	dw 0		      ; DEVICE
	dw 0		      ; TEXT
  db 'MSXB2R'   ; MSXBAS2ROM signature
	;dw 0,0,0	    ; Reserved

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
  ld a, (0x8000)            ; ASCII8 MegaROM loading bug fix
  cp 0x41
  jr nz, clear_basic_environment
    ld a, (0x8001)
    cp 0x42
    jr nz, clear_basic_environment
      ld a, 1
      ld (0x6800), a
      inc a
      ld (0x7000), a
      inc a
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
  ld (ENDPRG+1),a            ; fake line
  ld (ENDPRG+2),a            ; fake line
  ld (ENDPRG+3),a            ; fake line
  ld (ENDPRG+4),a            ; fake line
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
  ld hl, 0xF1C9              ; disk mode top memory
  jr run_user_basic_code_on_rom.himem_done

run_user_basic_code_on_rom.non_disk_himem:
  ld HL,VARWRK               ; non-disk mode top memory (0xF380)

run_user_basic_code_on_rom.himem_done:
  ld (HIMEM),HL              ; highest BASIC RAM address
  ld (MEMSIZ),hl
  ld a, (STARTUP_CFG_FILEIO)
  and a
  ld bc, 200                 ; default stack margin
  jr z, run_user_basic_code_on_rom.stack_margin_done
  ld bc, 256                 ; safer margin for disk mode

run_user_basic_code_on_rom.stack_margin_done:
  and a
  sbc hl, bc
  ld (STKTOP), hl

  jp 0x8010      	         ; Jump to above page (start code)

;---------------------------------------------------------------------------------------------------------
; ROUTINES ENTRY MAP FOR EXTERNAL ACCESS
;---------------------------------------------------------------------------------------------------------

wrapper_routines_map_start:
  jp castParamFloatInt
  jp cmd_clrkey
  jp cmd_clrscr
  jp cmd_disscr
  jp cmd_draw
  jp cmd_enascr
  jp cmd_keyclkoff
  jp cmd_mute
  jp cmd_play
  jp cmd_pad

  jp cmd_plyload
  jp cmd_plyloop
  jp cmd_plymute
  jp cmd_plyplay
  jp cmd_plyreplay
  jp cmd_plysong
  jp cmd_plysound

  jp cmd_ramtoram
  jp cmd_ramtovram
  jp cmd_rsctoram
  jp cmd_restore
  jp cmd_runasm

  jp cmd_screen_copy
  jp cmd_screen_load
  jp cmd_screen_paste

  jp cmd_mtf

  jp cmd_setfnt
  jp cmd_turbo
  jp cmd_updfntclr
  jp cmd_vramtoram
  jp cmd_wrtchr
  jp cmd_wrtclr
  jp cmd_wrtfnt
  jp cmd_wrtscr
  jp cmd_wrtspr
  jp cmd_wrtspratr
  jp cmd_wrtsprclr
  jp cmd_wrtsprpat
  jp cmd_wrtvram
  jp cmd_page

  jp floatNeg
  jp gfxTileAddress

  jp intCompareAND
  jp intCompareEQ
  jp intCompareGE
  jp intCompareGT
  jp intCompareLE
  jp intCompareLT
  jp intCompareNE
  jp intCompareNOT
  jp intCompareOR
  jp intCompareXOR
  jp intNeg
  jp intSHL
  jp intSHR

  jp player.initialize
  jp player.unhook

  jp set_tile_flip
  jp set_tile_rotate
  jp set_tile_color
  jp set_tile_pattern
  jp get_tile_color
  jp get_tile_pattern
  jp set_sprite_flip
  jp set_sprite_rotate
  jp set_sprite_color
  jp set_sprite_pattern
  jp get_sprite_color
  jp get_sprite_pattern

  jp usr0
  jp usr1
  jp usr2
  jp usr2_play
  jp usr2_player_status
  jp usr3
  jp usr3.COLLISION_ALL
  jp usr3.COLLISION_COUPLE
  jp usr3.COLLISION_ONE
  jp gfxVDP.set

  jp cmd_get_date
  jp cmd_get_time
  jp cmd_set_date
  jp cmd_set_time

  jp GET_NEXT_TEMP_STRING_ADDRESS

  jp MR_CALL
  jp MR_CALL_TRAP
  jp MR_CHANGE_SGM
  jp MR_GET_BYTE
  jp MR_GET_DATA
  jp MR_JUMP

  jp XBASIC_BASE
  jp XBASIC_CLS
  jp XBASIC_COPY
  jp XBASIC_COPY_FROM
  jp XBASIC_COPY_TO
  jp XBASIC_END
  jp XBASIC_INIT
  jp XBASIC_INPUT_1
  jp XBASIC_INPUT_2
  jp XBASIC_IREAD
  jp XBASIC_LOCATE
  jp XBASIC_PLAY
  jp XBASIC_PRINT_STR
  jp XBASIC_PUT_SPRITE
  jp XBASIC_READ
  jp XBASIC_RESTORE
  jp XBASIC_SCREEN
  jp XBASIC_WIDTH
  jp XBASIC_SOUND
  jp XBASIC_TAB
  jp XBASIC_USING
  jp XBASIC_USING.do
  jp XBASIC_USR
  jp cmd_preflight_disk
  jp cmd_maxfiles
