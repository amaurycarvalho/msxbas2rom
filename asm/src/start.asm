; ------------------------------------------------------------------------------------------------------
; BASIC start code - compiler version
; by: Amaury Carvalho, 2021
; Compile with:
;   pasmo init.asm init.bin
;   xxd -i init.bin init.h
; ------------------------------------------------------------------------------------------------------

  org 0x8000

ENASLT:	       equ 0024h
EXPTBL:        equ 0xFCC1   ; Expanded Slot Table

LINL40:        equ 0xF3AE
LINL32:        equ 0xF3AF
LINLEN:        equ 0xF3B0
CRTCNT:        equ 0xF3B1
CLMSLT:        equ 0xF3B2
NLONLY:        equ 0xF87C   ; loading basic program flags (bit 0=not close i/o buffer 0, bit 7=not close user i/o buffer)

SCRMOD:        equ 0xFCAF   ; 0=40x24 Text Mode, 1=32x24 Text Mode, 2=Graphics Mode, 3=Multicolour Mode.
OLDSCR:        equ 0xFCB0   ; old text screen mode
NTMSXP:        equ 0xF417   ; printer type (0=default)
HCHPU:         equ 0xFDA4   ; char put hook

IOALLOC:       equ 0x7E6B   ; IO allocation (CLEAR stmt)
BASINIT:       equ 0x629A   ; initialize interpreter, basic pointer at start of program
NEWSTT:        equ 0x4601   ; execute a text (hl=text pointer; text must start with a ":")
CHGET:         equ 0x009F

XBASIC_SCREEN: equ 0x7369

header_filler:
  db 0,0,0,0,0,0,0,0,0,0,0
  dw 0x0000  ; resource map start address
  db 0x00    ; resource map segment
  dw msx_basic_pcode_startup

initialize:
  ld a, 39
  ld (LINL40), a
  ld (LINLEN), a
  ld a, 29
  ld (LINL32), a
  ld a, 24
  ld (CRTCNT), a
  ld a, 14
  ld (CLMSLT), a
  xor a
  ld (SCRMOD), a
  ld (OLDSCR), a
  ld (NTMSXP), a
  call XBASIC_SCREEN         ; xbasic SCREEN mode (in: a = screen mode)

select_basic_on_page_1:
  ld a, (EXPTBL)
  ld h, 0x40
  call ENASLT                ; Select main ROM on page 1 (4000h~7FFFh)

start_basic:
  ;call IOALLOC               ; clear statement

  ld a, 0x81                 ; 10000001b
  ld (NLONLY), a             ; dont close i/o buffers - its necessary to BASINIT run correctly on SofaRun
  call BASINIT               ; initialize interpreter, basic pointer at start of program

  ld a, 0xC9                 ; ret
  ld (0xF397), a             ; CALSLT bug fix to some MSX 1 BIOS (ex: Expert XP-800 and GPC-1)

  jp NEWSTT                  ; execute next line

; dummy DEF USR into code to run compiled code
msx_basic_pcode_startup:
  ; filler
  db 0x00
  ; next MSX BASIC statement
  dw msx_basic_startup_end
  ; current MSX BASIC line number
  dw 0x0000
  ; DEFUSR9=&H0000:X=USR9(0)
  db 0x97, 0xDD, 0x1A, 0xEF, 0x0C
  dw compiled_start_code 
  db 0x3A, 0x58, 0xEF, 0xDD, 0x1A
  db 0x28, 0x11, 0x29, 0x00
msx_basic_startup_end:
  dw 0x0000

compiled_start_code:
