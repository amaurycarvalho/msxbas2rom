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

IOALLOC:       equ 0x7E6B   ; IO allocation from CLEAR statement
BASINIT:       equ 0x629A   ; initialize interpreter, basic pointer at start of program
NEWSTT:        equ 0x4601   ; execute a text (hl=text pointer; text must start with a ":")
CHGET:         equ 0x009F
INITIO:        equ 0x003B   ; Initialize I/O devices
XBASIC_SCREEN: equ 0x7369

STARTUP_CFG_FILEIO:  equ 0x800A  ; disk mode flag
DISK_ERROR_HANDLER:  equ 0xF323  ; disk error handler address (pointer to pointer)
ABORT_ERROR_HANDLER: equ 0xF1E2  ; abort error handler address
FCBBASE:             equ 0xF353  ; File Control Block base
FCBLIST:             equ 0xF355  ; File Control Block list
MAXFIL:              equ 0xF85F  ; number of currently allocated user I/O buffers
FILTAB:              equ 0xF860  ; address of the pointer table for the I/O buffer FCBs
DSKDIS:              equ 0xFD99  ; disable disks (DEVICE, 0xFF=yes)
BDOS:                equ 0xF37D  ; Disk BASIC commands handler
H_PHYD:              equ 0xFFA7  ; PHYDIO standard routine
SPADDRBAK:           equ 0xF304  ; SP register address copy
DSKERRBAK:           equ 0xF302  ; disk error handler address copy 

header_filler:
  db 0,0,0,0,0,0,0,0,0,0
startup_cfg_fileio:
  db 0x00    ; 0=non-disk mode, 1=disk mode
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
  ;ld a, (STARTUP_CFG_FILEIO)
  ;or a
  ;call nz, initialize_disk_mode

  ld a, 0x81                 ; 10000001b
  ld (NLONLY), a             ; dont close i/o buffers - its necessary to BASINIT run correctly on SofaRun
  call BASINIT               ; initialize interpreter, basic pointer at start of program

  ld a, 0xC9                 ; ret
  ld (0xF397), a             ; CALSLT bug fix to some MSX 1 BIOS (ex: Expert XP-800 and GPC-1)

  jp NEWSTT                  ; execute next line

;initialize_disk_mode:
;  xor a
;  ld (DSKDIS), a             ; enable disks
;  ld a, 1
;  ld (MAXFIL), a             ; default MAXFILES for disk mode
;  ret

; dummy DEF USR into code to run compiled code
msx_basic_pcode_startup:
  ; filler
  db 0x00
  ; next MSX BASIC statement
  dw msx_basic_startup_end
  ; current MSX BASIC line number
  dw 0x0000
  ; MAXFILES=1:
  ;db 0xCD, 0xB7, 0xEF, 0x12, 0x3A
  ; DEFUSR9=&H0000:
  db 0x97, 0xDD, 0x1A, 0xEF, 0x0C
  dw compiled_start_code 
  db 0x3A
  ; X=USR9(0)
  db 0x58, 0xEF, 0xDD, 0x1A
  db 0x28, 0x11, 0x29, 0x00
msx_basic_startup_end:
  dw 0x0000

compiled_start_code:
