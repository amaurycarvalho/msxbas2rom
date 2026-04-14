;---------------------------------------------------------------------------------------------------------
; MEGAROM SUPPORT CODE
; out (0x8E), a - MEGAROM SELECT TO READ
; https://www.msx.org/wiki/MegaROM_Mappers
;   ASCII 8K
;       Page (8kB)                              Switching address
;       6000h~7FFFh (mirror: E000h~FFFFh)	    6800h (mirrors: 6801h~6FFFh)
;       8000h~9FFFh (mirror: 0000h~1FFFh)       7000h (mirrors: 7001h~77FFh)
;       A000h~BFFFh (mirror: 2000h~3FFFh)       7800h (mirrors: 7801h~7FFFh)
;   Konami MegaROMs with SCC
;       8000h~9FFFh (mirror: 0000h~1FFFh)	    9000h (mirrors: 9001h~97FFh)
;       A000h~BFFFh (mirror: 2000h~3FFFh)	    B000h (mirrors: B001h~B7FFh)
;---------------------------------------------------------------------------------------------------------

Seg_P8000_SW:	 equ 0x7000 ; Segment switch on page 8000h-9FFFh (ASCII8 or Konami with SCC Mapper)
Seg_PA000_SW:	 equ 0x7800 ; Segment switch on page A000h-BFFFh (ASCII8 or Konami with SCC Mapper)
MR_TRAP_FLAG:    equ 0xFC82
MR_TRAP_SEGMS:   equ MR_TRAP_FLAG+1

MR_JUMP:
  push hl
MR_CHANGE_SGM:
  ld (SGMADR), a
  ld (Seg_P8000_SW), a
  inc a
  ld (Seg_PA000_SW), a
  dec a
  ret                       ; indirect jump

; OpenMSX MegaROM detection bug fix
; http://github.com/openMSX/openMSX/blob/master/src/memory/RomFactory.cc
OPENMSX_EMULATOR_AUTODETECTION:
  ld (Seg_P8000_SW), a
  ld (Seg_P8000_SW), a
  ld (Seg_P8000_SW), a
  ld (Seg_P8000_SW), a
  ld (Seg_P8000_SW), a

MR_CALL:
  ld bc, (SGMADR-1)         ; salve old segment number
  push bc
    call MR_CHANGE_SGM
    ld de, MR_CALL_RET      ; restore old segment code
    push de
    push hl                 ; called segment code
    ex af, af'              ; restore old registers values (call parameters)
    exx
    ret                     ; indirect call
MR_CALL_RET:
  pop af                    ; restore old segment number
  jp MR_CHANGE_SGM

MR_GET_DATA:
  ld bc, (SGMADR-1)         ; salve old segment number
  push bc
    call MR_CHANGE_SGM
    ; copy string from hl to TEMPORARY STRING BUFFER
    ld b, 0
    ld c, (hl)
    inc bc
    ex de, hl
      call GET_NEXT_TEMP_STRING_ADDRESS
      ld (TMPSTRBUF), hl
    ex de, hl
    ldir
  pop af                    ; restore old segment number
  call MR_CHANGE_SGM
  ex af, af'                ; restore old registers values (call parameters)
  exx
  ld hl, (TMPSTRBUF)        ; data restored from segment number
  ret

MR_GET_BYTE:
  ld bc, (SGMADR-1)         ; salve old segment number
  push bc
    call MR_CHANGE_SGM
    ld c, (hl)
    inc hl
    ld b, (hl)
  pop af                    ; restore old segment number
  call MR_CHANGE_SGM
  ld a, c
  ret

MR_CALL_TRAP:               ; call trap in DE (wrapper to C6C25 in J6C5F at 6C6F)
  ld a, (MR_TRAP_FLAG)      ; start of TRPTBL reserved area (megaROM flag)
  or a
  jp z, C6C25               ; jump if not megaROM trappers
    push de
      ld de, TRPTBL         ; start of TRPTBL
      xor a
      sbc hl, de
      ld de, 3
      call XBASIC_DIVIDE_INTEGERS
      ld de, MR_TRAP_SEGMS  ; start of TRPTBL reserved area (segments list)
      add hl, de
      ld a, (hl)            ; segment to call
    pop hl
    jp MR_CALL

GET_NEXT_TEMP_STRING_ADDRESS:
    push af
    push bc
      ld a, (TMPSTRIND)      ; temporary string list current position
      ld b, a
      ld c, 0
      ld hl, (TMPSTRADDR)    ; temporary string list start address
      add hl, bc
      inc a
      cp 5
      jr nz, GET_NEXT_TEMP_STRING_ADDRESS.1
        xor a
GET_NEXT_TEMP_STRING_ADDRESS.1:
      ld (TMPSTRIND), a
    pop bc
    pop af
    ret
