;=========================================================================================================
; BASIC KUN (version 2.1) SUPPORT ROUTINES
;=========================================================================================================

;---------------------------------------------------------------------------------------------------------
; DECLARATIONS
;---------------------------------------------------------------------------------------------------------

VDP.DR	EQU	0006H
VDP.DW	EQU	0007H
IDBYT2	EQU	002DH
CHGMOD	EQU	005FH
CHGCLR	EQU	0062H
WRTPSG	EQU	0093H
;QINLIN	EQU	00B4H
CLS	EQU	00C3H
ERAFNK	EQU	00CCH
DSPFNK	EQU	00CFH
GTSTCK	EQU	00D5H
GTPAD	EQU	00DBH
GTPDL	EQU	00DEH
READC	EQU	011DH

M406F	EQU	406FH			; BASIC error
M57F1	EQU	57F1H			; PSET
M58C1	EQU	58C1H			; LINE boxfill
M58FC	EQU	58FCH			; LINE
M5912	EQU	5912H			; LINE box
M59E3	EQU	59E3H			; PAINT MSX1
M5B19	EQU	5B19H			; CIRCLE
M5EA4	EQU	5EA4H			; locate variable
M5F5D	EQU	5F5DH			; locate variable (search only)
M7810	EQU	7810H			; check ON <trap> GOTO

S266E	EQU	266EH			; PAINT MSX2 (subrom)


RG2SAV	EQU	0F3E1H
RG6SAV	EQU	0F3E5H
DATLIN	EQU	0F6A3H
SAVTXT	EQU	0F6AFH
DATPTR	EQU	0F6C8H
DEFTBL	EQU	0F6CAH
DPPAGE	EQU	0FAF5H
ACPAGE	EQU	0FAF6H
MODE	EQU	0FAFCH
FNKFLG	EQU	0FBCEH
FNKSTR  EQU 0F87FH   ; 160
ONGSBF	EQU	0FBD8H
LINWRK	EQU	0FC18H
TRPTBL	EQU	0FC4CH
INTFLG	EQU	0FC9BH
BRDATR	EQU	0FCB2H
GXPOS	EQU	0FCB3H
GYPOS	EQU	0FCB5H
GRPACX	EQU	0FCB7H
GRPACY	EQU	0FCB9H
PROCNM	EQU	0FD89H
RG11SAV	EQU	0FFEAH
RG23SAV	EQU	0FFF6H
RG25SAV	EQU	0FFFAH
RG26SAV	EQU	0FFFBH

;---------------------------------------------------------------------------------------------------------
; FIXED ADDRESS ROUTINES
; To do: 
;   Move ARKOS_PLAYER to the kernel's end (0x8000 - size of arkosplayer.bin)
;   removing BASIC_KUN_START_FILLER and KERNEL_END_FILLER and
;   adding another one (BEFORE_ARKOS_PLAYER_FILLER: DEFS 08000H - size of arkosplayer.bin - $, 000H) 
;   to release more free space to the kernel.
; Note:
;   arkosplayer.bin needs to be recompiled to the new address (org statement).
;---------------------------------------------------------------------------------------------------------

BASIC_KUN_START_FILLER:
    DEFS	06BF7H-34-1876-$,000H     ; 34 = J_* functions size, 1876 = arkosplayer.bin size

ARKOS_PLAYER:
    INCBIN "assets/arkos/arkosplayer.bin"

;---------------------------------------------------------------------------------------------------------
; DYNAMIC ADDRESS ROUTINES
;---------------------------------------------------------------------------------------------------------

; Desativado: jump table de erros BASIC + I4E28 erro de modo de tela (J4DFA-J4E25, I4E28: 13 labels)
;J4DFA:
;	DEFS	47,0x00

;PAINT_FIX.1:              ; same as PNTINI on bios
;    LD	(BRDATR),A
;    ld a, (SCRMOD)
;    cp 2                  ; screen 2?
;    jr z, PAINT_FIX.1.1
;    cp 4                  ; screen 4?
;    ret nz
;PAINT_FIX.1.1:
;      ld a, b
;      ld (BRDATR), a      ; border color = paint color
;      ret

PAINT_FIX.2:
    push af
      ld a, (VERSION)
      cp 3                ; MSX Turbo R?
      jr c, PAINT_FIX.2.1
         ld a, (BASVER)
         and 0xF0
         or a
         jr z, PAINT_FIX.2.1    ; jump if japanese TurboR
           inc ix
           inc ix
           inc ix
           inc ix
           inc ix
PAINT_FIX.2.1:
    pop af
	;jp	SUB_EXTROM			; PAINT handler (subrom) ===> see it below

; EXTROM alternative with interruption enabled

SUB_EXTROM:
  call EXTROM
  ei
  ret

; SUB_PUFOUT - a=format, b=thousand digits, c=decimal digits, DAC=value, out hl=string

SUB_PUFOUT:
  ld hl, PUFOUT                ; a=format, b=thousand digits, c=decimal digits, DAC=value, out hl=string
  ld ix, 0x4D86                ; jp (hl) - workaround to do MATH PACK interslot call
  call CALBAS
  ei
  ret

