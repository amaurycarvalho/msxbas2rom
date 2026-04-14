;----------------------------------------------------------------------------------
; BASIC KUN (version 2.1) SUPPORT ROUTINES
;----------------------------------------------------------------------------------

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
CURLIN	EQU	0F41CH
DATLIN	EQU	0F6A3H
SAVTXT	EQU	0F6AFH
DATPTR	EQU	0F6C8H
DEFTBL	EQU	0F6CAH
FBUFFR	EQU	0F7C5H
NULBUF	EQU	0F862H
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

J4DFA:
J4E07:	LD	E,4
	DEFB	001H
J4E0A:	LD	E,12
	DEFB	001H
J4E0D:	LD	E,5
	DEFB	001H
J4E10:	LD	E,13
	DEFB	001H
J4E13:	LD	E,16
	DEFB	001H
J4E16:	LD	E,2
	DEFB	001H
J4E19:	LD	E,8
	DEFB	001H
J4E1C:	LD	E,1
	DEFB	001H
J4E1F:	LD	E,7
	DEFB	001H
J4E22:	LD	E,10
	DEFB	001H
J4E25:	LD	E,9
	DEFB	001H
I4E28:	LD	E,3
	LD	HL,MODE
	RES	4,(HL)			; 212 lines mode
	LD	IX,M406F
	JP	CALBAS			; BASIC error

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

;---------------------------------------------------------------------------------------------------------
; FIXED ADDRESS ROUTINES
;---------------------------------------------------------------------------------------------------------

BASIC_KUN_START_FILLER:
    DEFS	06BF7H-34-1876-$,000H     ; 34 = J_* functions size, 1876 = arkosplayer.bin size

ARKOS_PLAYER:
    INCBIN "assets/arkos/arkosplayer.bin"

J_SGN_INT:
    ld a, l
	or h
	ret z
	  bit 7,h
	  ld hl,0ffffh
	  ret nz
	    inc hl
	    inc hl
        ret

J_SGN_FLOAT:
    ld a, b
    and a
    ret z
      ld b, 0x81
      ld l, 0
      ld a, h
      and 0x80
      ld h, a
      ret

J_ABS_INT:
    ld a, h
	add a, a
	ret nc
	  xor a
	  sub l
	  ld l, a
	  sbc a, a
	  sub h
	  ld h, a
	  ret

;	  Subroutine swap integer
;	     Inputs  ________________________
;	     Outputs ________________________

I6BF7:	LD	B,2			; integer is 2 bytes
	JR	J6C01

;	  Subroutine swap string
;	     Inputs  ________________________
;	     Outputs ________________________

I6BFB:	LD	B,0			; string is 256 bytes
	JR	J6C01


;	  Subroutine swap float
;	     Inputs  ________________________
;	     Outputs ________________________

I6BFF:	LD	B,3			; float is 3 bytes

J6C01:	LD	C,(HL)
	LD	A,(DE)
	LD	(HL),A
	LD	A,C
	LD	(DE),A
	INC	DE
	INC	HL
	DJNZ	J6C01
	RET

;     WriteParamBCD
;	  Subroutine write a float in BCD double parameter form (Basic Interpreter)
;	     Inputs  b:hl = float, de = start of basic line to write the parameter
;	     Outputs hl = next position in basic line

WriteParamBCD:
        push de
          call FloatToBCD
          call DECNRM  ; normalize
        pop hl
        ld (hl), 0x1D  ; BCD single marker
        inc hl
        ex de, hl
          ld hl, DAC
          ld bc, 4
          ldir
        ex de, hl
        ret            ; 22 bytes

  nop
  nop
  nop
  nop

;	  Subroutine CALL DE
;	     Inputs  ________________________
;	     Outputs ________________________

C6C25:	PUSH	DE
	RET

;	  Subroutine check for traps handler
;	     Inputs  ________________________
;	     Outputs ________________________

C6C27:	LD	HL,TRPTBL+10*3		; STOP trap
	BIT	0,(HL)			; trap enabled ?
	JR	Z,J6C4A			; nope, skip STOP trap
	EX	DE,HL
	LD	HL,INTFLG
	DI
	LD	A,(HL)			; get STOP or CTRL-STOP status
	EI
	LD	(HL),0			; reset STOP status
	CP	3			; CTRL-STOP pressed ?
	JR	NZ,J6C4A		; nope, skip STOP trap
	EX	DE,HL
	LD	A,(HL)
	SET	2,(HL)			; trap raised
	CP	(HL)			; trap already raised ?
	JR	Z,J6C4A			; yep, skip STOP trap
	AND	02H			; trap paused ?
	JR	NZ,J6C4A		; yep, skip STOP trap
	LD	HL,ONGSBF
	INC	(HL)			; increase trap counter
J6C4A:	LD	A,(ONGSBF)
	AND	A			; trap occured ?
	RET	Z			; nope, quit
	LD	HL,TRPTBL		; trap table
	LD	B,26
J6C54:	LD	A,(HL)
	CP	05H			; trap enabled AND trap not paused AND trap occured ?
	INC	HL
	JR	Z,J6C5F			; yep, invoke trap handler
	INC	HL
J6C5B:	INC	HL
	DJNZ	J6C54			; next trap
	RET

J6C5F:	LD	E,(HL)
	INC	HL
	LD	D,(HL)			; trap handler
	LD	A,E
	OR	D			; trap handler defined ?
	JR	Z,J6C5B			; nope, next trap
	DEC	HL
	DEC	HL
	CALL	C6CB1			; decrease trap counter
	CALL	C6CA7			; pause trap
	PUSH	HL
	;CALL	C6C25			; call trap handler
	CALL MR_CALL_TRAP
	POP	HL
	BIT	0,(HL)			; trap enabled ?
	RET	Z			; nope, quit
	DI
	LD	A,(HL)
	AND	05H			; ignore trap paused
	CP	(HL)			; trap enabled AND trap occured AND trap not paused ?
	LD	(HL),A			; resume trap
	JR	Z,J6CBE			; yep, quit
	CP	05H			; trap enabled AND trap occured ?
	JR	NZ,J6CBE		; yep, quit
J6C82:	LD	A,(ONGSBF)
	INC	A
	LD	(ONGSBF),A		; increase trap counter
	EI
	RET

;	  Subroutine enable trap
;	     Inputs  ________________________
;	     Outputs ________________________

I6C8B:	CALL	C6CC0			; disable normal function key behaviour if function key trap
	DI
	LD	A,(HL)
	AND	04H			; trap raised status
	OR	01H
	CP	(HL)			; trap already enabled AND trap not paused ?
	LD	(HL),A			; enable trap, resume trap
	JR	Z,J6CBE			; yep, quit
	AND	04H			; trap raised ?
	JR	NZ,J6C82		; yep, increase trap counter and quit
	EI
	RET

;	  Subroutine disable trap
;	     Inputs  ________________________
;	     Outputs ________________________

I6C9E:	CALL	C6CC8			; enable normal function key behaviour if function key trap
	DI
	LD	A,(HL)			; current trap status
	LD	(HL),0			; trap disabled, trap not paused, trap not occured
	JR	J6CAB			; if trap occured, reset trap occured and decrease trap counter

;	  Subroutine pause trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CA7:	DI
	LD	A,(HL)			; current trap status
	SET	1,(HL)			; pause trap

; if trap occured, reset trap occured and decrease trap counter

J6CAB:	CP	05H			; was trap enabled AND trap not paused AND trap occured ?
	JR	Z,J6CB4			; yep, reset trap occured and decrease trap counter
	EI
	RET

;	  Subroutine decrease trap counter
;	     Inputs  ________________________
;	     Outputs ________________________

C6CB1:	DI
	RES	2,(HL)			; trap not occured
J6CB4:	LD	A,(ONGSBF)
	SUB	1			; decrease trap counter
	JR	C,J6CBE			; already zero, quit
	LD	(ONGSBF),A
J6CBE:	EI
	RET

;	  Subroutine disable normal function key behaviour if function key trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CC0:	CALL	C6CCF			; get FNKFLG entry if trap is a function key trap
	RET	NC			; not a function key trap, quit
	LD	A,1
	LD	(DE),A			; function key trap enabled, disable normal function key behavior
	RET

;	  Subroutine enable normal function key behaviour if function key trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CC8:	CALL	C6CCF			; get FNKFLG entry if trap is a function key trap
	RET	NC			; not a function key trap, quit
	XOR	A
	LD	(DE),A			; function key trap disabled, enable normal function key behavior
	RET

;	  Subroutine get FNKFLG entry if trap is a function key trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CCF:	PUSH	HL
	LD	DE,TRPTBL+10*3
	AND	A
	SBC	HL,DE			; function key trap ?
	LD	C,L
	POP	HL
	RET	NC			; nope, quit
	LD	DE,FNKFLG+9+1
J6CDC:	DEC	DE
	INC	C
	INC	C
	INC	C
	JR	NZ,J6CDC
	RET

;	  Subroutine clear screen
;	     Inputs  ________________________
;	     Outputs ________________________

I6CE3:	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J746E			; yep, use CLS BIOS handler
	CALL	C73D3			; ?? screen mode 10 action
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	XOR	A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	LD	A,(ACPAGE)
	OUT	(C),A
	XOR	A
	OUT	(C),A
	LD	A,(SCRMOD)
	RRCA
	AND	01H
	INC	A
	OUT	(C),A
	LD	A,(MODE)
	AND	10H			; 212 lines mode ?
	JR	Z,J6D18			; yep,
	XOR	A
	OUT	(C),A
	INC	A
	JR	J6D1D

J6D18:	LD	A,0D4H
	OUT	(C),A
	XOR	A
J6D1D:	OUT	(C),A
	LD	A,(BAKCLR)
	CALL	C6D2F			; convert background color to background color byte
	OUT	(C),H
	XOR	A
	OUT	(C),A
	LD	A,0C0H
	OUT	(C),A
	RET

;	  Subroutine convert background color to background color byte
;	     Inputs  ________________________
;	     Outputs ________________________

C6D2F:	LD	H,A
	LD	A,(SCRMOD)
	CP	8			; screen mode 8 ?
	RET	Z			; yep, quit
	CP	7			; screen mode 7 ?
	JR	Z,J6D43			; yep, b7-b4 = b3-b0
	CP	5			; screen mode 5 ?
	JR	Z,J6D43			; yep, b7-b4 = b3-b0
					; b7-b6 = b5-b4 = b3-b2 = b1-b0
	LD	A,H
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
J6D43:	LD	A,H
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
	RET

;	  Subroutine draw box
;	     Inputs  ________________________
;	     Outputs ________________________

I6D4B:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J747B			; yep, use MSX1 draw box handler
	POP	AF
	PUSH	HL
	PUSH	IX
	POP	HL
	AND	A
	PUSH	HL
	SBC	HL,DE
	POP	HL
	JR	NC,J6D62
	EX	DE,HL
	AND	A
J6D62:	EX	(SP),HL
	PUSH	DE
	PUSH	IY
	POP	DE
	PUSH	HL
	SBC	HL,DE
	POP	HL
	JR	C,J6D6E
	EX	DE,HL
J6D6E:	PUSH	DE
	POP	IY
	POP	DE
	POP	IX
	PUSH	IY
	PUSH	HL
	POP	IY
	DEC	IX
	PUSH	AF
	PUSH	BC
	CALL	C6DA9			; draw line
	POP	BC
	POP	AF
	INC	IX
	POP	HL
	DEC	HL
	PUSH	IY
	PUSH	DE
	PUSH	IX
	POP	DE
	PUSH	AF
	PUSH	BC
	CALL	C6DA9			; draw line
	POP	BC
	POP	AF
	INC	HL
	POP	IX
	INC	IX
	PUSH	HL
	POP	IY
	PUSH	AF
	PUSH	BC
	CALL	C6DA9			; draw line
	POP	BC
	POP	AF
	DEC	IX
	POP	HL
	INC	HL
	PUSH	IX
	POP	DE
					; draw line and quit

;	  Subroutine draw line
;	     Inputs  ________________________
;	     Outputs ________________________

C6DA9:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J7476			; yep,use MSX1 draw line handler
	POP	AF
	CALL	C73D3			; ?? screen mode 10 action
	PUSH	AF
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled (DE,HL)
	CALL	C742D			; force X and Y coordinate within screen limits, when limits enabled (IX,IY)
	POP	AF
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	PUSH	AF
	PUSH	AF
	PUSH	HL
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	PUSH	IX
	POP	HL
	OUT	(C),L
	OUT	(C),H
	PUSH	IY
	POP	HL
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A
	LD	A,0CH	; 12
	PUSH	IX
	POP	HL
	SBC	HL,DE
	JR	NC,J6DEF
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	AND	0FBH
J6DEF:	PUSH	HL
	POP	IX
	POP	DE
	PUSH	IY
	POP	HL
	SBC	HL,DE
	JR	NC,J6E02
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	AND	0F7H
J6E02:	PUSH	IX
	POP	DE
	PUSH	HL
	SBC	HL,DE
	POP	HL
	JR	C,J6E0E
	EX	DE,HL
	OR	01H
J6E0E:	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	OUT	(C),H
	LD	E,A
	POP	AF
	OUT	(C),A
	OUT	(C),E
	LD	A,B
	OR	70H	; "p"
	OUT	(C),A
	POP	AF
	POP	IY
	POP	IX
	POP	HL
	POP	DE
	RET

;	  Subroutine draw filled box
;	     Inputs  ________________________
;	     Outputs ________________________

I6E29:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J7480			; yep, use MSX1 draw filled box handler
	POP	AF
	CALL	C73D3			; ?? screen mode 10 action

;	  Subroutine filled box
;	     Inputs  ________________________
;	     Outputs ________________________

C6E36:	PUSH	AF
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled (DE,HL)
	CALL	C742D			; force X and Y coordinate within screen limits, when limits enabled (IX,IY)
	POP	AF
	PUSH	AF
	PUSH	HL			; save HL
	PUSH	IX
	POP	HL			; HL = second X
	AND	A
	PUSH	HL			; save second X
	SBC	HL,DE			; second X smaller as first X ?
	POP	HL
	JR	NC,J6E4C
	EX	DE,HL			; yep, swap
	AND	A
J6E4C:	EX	(SP),HL
	PUSH	DE			; save DE
	PUSH	IY
	POP	DE			; DE = second Y
	PUSH	HL
	SBC	HL,DE			; second Y smaller as first Y ?
	POP	HL
	JR	C,J6E58
	EX	DE,HL			; yep, swap
J6E58:	PUSH	DE
	POP	IY			; IY = DE
	POP	DE
	POP	IX
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D			; DX = DE
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A			; DY = HL
	PUSH	HL
	PUSH	IX
	POP	HL
	AND	A
	SBC	HL,DE
	INC	HL
	OUT	(C),L
	OUT	(C),H			; NX
	LD	A,L
	OR	E
	PUSH	IY
	POP	HL
	POP	DE
	SBC	HL,DE
	INC	HL
	OUT	(C),L
	OUT	(C),H			; NY
	LD	E,A
	POP	HL
	INC	B
	DJNZ	J6EA7
	LD	A,(SCRMOD)
	CP	8
	JR	Z,J6EBE
	CP	7
	JR	Z,J6E9C
	CP	6
	JR	Z,J6EA2
J6E9C:	LD	A,E
	RRCA
	JR	NC,J6EB7
	JR	J6EA7

J6EA2:	LD	A,E
	AND	3
	JR	Z,J6EB2
J6EA7:	OUT	(C),H
	XOR	A
	OUT	(C),A
	LD	A,B
	OR	80H
	OUT	(C),A
	RET

J6EB2:	LD	A,H
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
J6EB7:	LD	A,H
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
J6EBE:	OUT	(C),H
	XOR	A
	OUT	(C),A
	LD	A,0C0H
	OUT	(C),A
	RET

;	  Subroutine copy
;	     Inputs  DE=X0, HL=Y0, IX=X1, IY=Y1, B=operator, FCB7=XD, FCB9=YD, FC18=PS, FC19=PD, FAFC=mode (screen limits)
;	     Outputs ________________________

I6EC8:	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled (DE,HL)
	CALL	C742D			; force X and Y coordinate within screen limits, when limits enabled (IX,IY)
	PUSH	HL
	PUSH	DE
	LD	DE,(GRPACX)
	LD	HL,(GRPACY)
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	LD	(GRPACX),DE
	LD	(GRPACY),HL
	POP	DE
	POP	HL
	PUSH	DE
	PUSH	BC
	LD	A,32			; VDP register SX, auto increment
	CALL	C707B			; set VDP controlregister
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	LD	A,(LINWRK+0)
	OR	H
	OUT	(C),A
	PUSH	HL
	LD	HL,(GRPACX)
	OUT	(C),L
	OUT	(C),H
	LD	HL,(GRPACY)
	OUT	(C),L
	LD	A,(LINWRK+1)
	OR	H
	OUT	(C),A
	LD	B,0
	LD	A,(GRPACX)
	OR	E
	PUSH	IX
	POP	HL
	SBC	HL,DE
	JR	NC,J6F28
	LD	A,(GRPACX)
	CPL
	LD	D,A
	LD	A,E
	CPL
	OR	D
	EX	DE,HL
	LD	HL,0
	SBC	HL,DE
	SET	2,B
J6F28:	INC	HL
	OUT	(C),L
	OUT	(C),H
	OR	L
	POP	DE
	PUSH	IY
	POP	HL
	SBC	HL,DE
	JR	NC,J6F3E
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	SET	3,B
J6F3E:	INC	HL
	OUT	(C),L
	OUT	(C),H
	OUT	(C),A
	OUT	(C),B
	LD	B,A
	POP	HL
	POP	DE
	LD	A,H
	AND	A
	JR	NZ,J6F68
	LD	A,(SCRMOD)
	CP	8
	JR	Z,J6F6E
	CP	7
	JR	Z,J6F5D
	CP	6
	JR	Z,J6F63
J6F5D:	LD	A,B
	RRCA
	JR	NC,J6F6E
	JR	J6F68

J6F63:	LD	A,B
	AND	3
	JR	Z,J6F6E
J6F68:	LD	A,H
	OR	90H
	OUT	(C),A
	RET

J6F6E:	LD	A,0D0H
	OUT	(C),A
	RET

;	  Subroutine draw point
;	     Inputs  ________________________
;	     Outputs ________________________

I6F73:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J749A			; yep, use MSX1 draw point handler
	POP	AF
	CALL	C73D3			; ?? screen mode 10 action
	PUSH	AF
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	POP	AF
	OUT	(C),A
	XOR	A
	OUT	(C),A
	LD	A,B
	OR	50H	; "P"
	OUT	(C),A
	RET

;	  Subroutine point
;	     Inputs  ________________________
;	     Outputs ________________________

I6FA9:	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J74A8			; yep, use MSX1 point handler
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	LD	A,32			; VDP register SX, auto increment
	CALL	C707B			; set VDP controlregister
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	XOR	A
	OUT	(C),A
	LD	A,40H
	OUT	(C),A
	DEC	C
	DEC	C
	LD	H,0
	CALL	C708A			; wait until VDP command is done
	LD	A,7			; VDP status register 7
	DI
	CALL	C709C			; read VDP status register
	IN	L,(C)
	XOR	A			; VDP status register 0
	CALL	C709C			; read VDP status register
	EI
	CALL	C73BE			; check if screen mode 10
	RET	NZ			; nope, quit
	LD	A,L
	AND	0F8H
	RRA
	RRA
	RRA
	RRA
	LD	L,A
	RET	C
	LD	L,0FFH
	RET

;	  Subroutine setup scroll
;	     Inputs  ________________________
;	     Outputs ________________________

C7004:	PUSH	AF
	LD	A,E
	AND	D
	INC	A
	JR	Z,J7023
	LD	C,E
	DEC	DE
	RR	D
	RR	E
	SRL	E
	SRL	E
	INC	E
	LD	A,E
	AND	3FH
	LD	E,A
	LD	A,C
	NEG
	AND	07H
	LD	D,A
	LD	(RG26SAV),DE
J7023:	LD	A,L
	AND	H
	INC	A
	JR	Z,J702C
	LD	A,L
	LD	(RG23SAV),A
J702C:	LD	HL,RG25SAV
	INC	B
	JR	Z,J7038
	SET	1,(HL)
	DJNZ	J7038
	RES	1,(HL)
J7038:	POP	BC
	INC	B
	JR	Z,J7042
	SET	0,(HL)
	DJNZ	J7042
	RES	0,(HL)
J7042:	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	LD	A,2			; VDP status register 2
	DI
	CALL	C709C			; read VDP status register
J704D:	IN	A,(C)
	ADD	A,A
	JP	M,J704D
J7053:	IN	A,(C)
	ADD	A,A
	JP	P,J7053
	XOR	A			; VDP status register 0
	CALL	C709C			; read VDP status register
	LD	A,(RG23SAV)
	OUT	(C),A
	LD	A,128+23
	OUT	(C),A
	LD	A,25
	OUT	(C),A
	LD	A,128+17
	OUT	(C),A
	INC	C
	INC	C
	LD	B,3
	LD	HL,RG25SAV
	OTIR
	EI
	RET

;	  Subroutine set VDP controlregister = DX, auto increment
;	     Inputs  ________________________
;	     Outputs ________________________

C7079:	LD	A,36			; VDP register DX, auto increment

;	  Subroutine set VDP controlregister
;	     Inputs  ________________________
;	     Outputs ________________________

C707B:	PUSH	AF
	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	POP	AF
	DI
	OUT	(C),A
	LD	A,128+17
	EI
	OUT	(C),A

;	  Subroutine wait until VDP command is done
;	     Inputs  ________________________
;	     Outputs ________________________

C708A:	LD	A,2			; VDP status register 2
	DI
	CALL	C709C			; read VDP status register
	IN	A,(C)
	RRA
	LD	A,0			; VDP status register 0
	CALL	C709C			; read VDP status register
	EI
	JR	C,C708A
	RET

;	  Subroutine read VDP status register
;	     Inputs  ________________________
;	     Outputs ________________________

C709C:	OUT	(C),A
	LD	A,128+15
	OUT	(C),A
	RET

;	  Subroutine read VRAM
;	     Inputs  HL = VRAM address
;	     Outputs HL = VRAM value

I70A3:	CALL	C70AB			; setup VDP for VRAM read
	LD	H,0
	IN	L,(C)
	RET

;	  Subroutine setup VDP for VRAM read
;	     Inputs  HL = VRAM address
;	     Outputs ________________________

C70AB:	LD	A,(VDP.DR)
	LD	C,A

;	  Subroutine setup VDP for VRAM read
;	     Inputs  HL = VRAM address, C = VDP data read port
;	     Outputs ________________________

C70AF:	CALL	C7403			; setup VDP active screen page and low byte VRAM address
	EI
	OUT	(C),A			; high byte VRAM address, read VRAM operation
	DEC	C			; data port
	RET

;	  Subroutine write VRAM
;	     Inputs  HL = VRAM address, A = byte
;	     Outputs ________________________

I70B7:	LD	B,A
	CALL	C71B1			; setup VDP for VRAM write
	OUT	(C),B
	RET

;	  Subroutine set sprite mode
;	     Inputs  ________________________
;	     Outputs ________________________

I70BE:	AND	3
	LD	HL,RG1SAV
	RES	0,(HL)
	RES	1,(HL)
	OR	(HL)
	LD	(HL),A
	JP	SUB_CLRSPR

;	  Subroutine change display page
;	     Inputs  ________________________
;	     Outputs ________________________

C70CC:	LD	(DPPAGE),A
	RRCA
	RRCA
	RRCA
	AND	0E0H
	LD	E,A			; OR mask
	LD	A,(RG2SAV)
	AND	1FH
	LD	C,2			; register 2
	CALL	C7109			; write VDP register with OR mask
	LD	A,(SCRMOD)
	CP	7
	LD	A,(DPPAGE)
	LD	D,0FCH			; AND mask
	JR	C,J70EE
	ADD	A,A
	LD	D,0FDH			; AND mask
J70EE:	LD	E,A			; OR mask
	LD	A,(RG11SAV)
	LD	C,11			; register 11
	CALL	C7108			; write VDP register with AND and OR mask
	LD	A,D
	RLCA
	RLCA
	RLCA
	RLCA
	LD	D,A			; AND mask
	LD	A,E
	RLCA
	RLCA
	RLCA
	RLCA
	LD	E,A			; OR mask
	LD	A,(RG6SAV)
	LD	C,6			; register 6

;	  Subroutine write VDP register with AND and OR mask
;	     Inputs  ________________________
;	     Outputs ________________________

C7108:	AND	D

;	  Subroutine write VDP register with OR mask
;	     Inputs  ________________________
;	     Outputs ________________________

C7109:	OR	E
	LD	B,A
	JP	WRTVDP

;	  Subroutine change color definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C710E:	PUSH	AF
	PUSH	BC
	PUSH	DE
	LD	IX,S.GETPLT
	CALL	EXTROM
	POP	DE
	LD	L,C
	LD	A,B
	RLCA
	RLCA
	RLCA
	RLCA
	AND	0FH
	LD	E,A
	LD	A,B
	POP	BC
	AND	0FH
	LD	C,A
	LD	A,D
	INC	A
	JR	NZ,J712C
	LD	D,E
J712C:	LD	A,H
	INC	A
	JR	NZ,J7131
	LD	H,L
J7131:	LD	A,B
	INC	A
	JR	NZ,J7136
	LD	B,C
J7136:	LD	A,D
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	B
	POP	DE
	LD	E,H
	LD	IX,S.SETPLT
	JP	EXTROM

;	  Subroutine set sprite definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7145:	PUSH	HL
	CALL	CALPAT
	CALL	C71B1			; setup VDP for VRAM write
	CALL	GSPSIZ
	LD	A,8
	JR	NC,J7155
	LD	A,32
J7155:	LD	B,A			; size of sprite definition
	POP	HL
	SUB	(HL)			; string length > size of sprite definition ?
	PUSH	AF
	JR	C,J7160			; yep, use only the size of sprite definition
	LD	B,(HL)			; string length
	INC	B
	DEC	B			; empty string ?
	JR	Z,J7164			; yep, skip changing the sprite definition
J7160:	INC	HL
	CALL	C71A4			; write bytes to VRAM
J7164:	POP	AF
	RET	Z			; complete sprite definition, quit
	RET	C			; complete sprite definition, quit
	LD	B,A
	XOR	A
J7169:	OUT	(C),A
	DJNZ	J7169			; clear remainer of sprite defintion
	RET

;	  Subroutine get sprite definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C716E:	CALL	CALPAT
	CALL	C70AB			; setup VDP for VRAM read
	CALL	GSPSIZ
	LD	B,8
	JR	NC,J717D
	LD	B,32
J717D:	LD	HL,BUF
	PUSH	HL
	LD	(HL),B			; string length
	INC	HL
J7183:	INI
	JR	NZ,J7183
	POP	HL
	RET

;	  Subroutine change sprite color handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7189:	AND	7FH
	PUSH	AF
	CALL	C71A9			; setup VDP for VRAM write of sprite color table
	POP	AF
	LD	B,16
	JR	J7169

;	  Subroutine change sprite color definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7194:	PUSH	HL
	CALL	C71A9			; setup VDP for VRAM write of sprite color table
	POP	HL
	LD	A,(HL)
	AND	A
	RET	Z
	CP	16+1
	JR	C,J71A2
	LD	A,16
J71A2:	LD	B,A
	INC	HL

;	  Subroutine write bytes to VRAM
;	     Inputs  ________________________
;	     Outputs ________________________

C71A4:	OUTI
	JR	NZ,C71A4
	RET

;	  Subroutine setup VDP for VRAM write of sprite color table
;	     Inputs  ________________________
;	     Outputs ________________________

C71A9:	LD	A,B
	ADD	A,A			; *2
	ADD	A,A			; *4
	CALL	CALATR
	DEC	H
	DEC	H

;	  Subroutine setup VDP for VRAM write
;	     Inputs  HL = VRAM address
;	     Outputs ________________________

C71B1:	LD	A,(VDP.DR)
	LD	C,A

;	  Subroutine setup VDP for VRAM write
;	     Inputs  HL = VRAM address, C = VDP data write port
;	     Outputs ________________________

C71B5:	CALL	C7403			; setup VDP active screen page and low byte VRAM address
	OR	40H
	EI
	OUT	(C),A			; high byte VRAM address, write VRAM operation
	DEC	C			; data port
	RET

;	  Subroutine set sprite attributes handler
;	     Inputs  ________________________
;	     Outputs ________________________

C71BF:	PUSH	AF
	LD	A,(VDP.DR)
	LD	C,A
	LD	A,(SCRMOD)
	CP	4			; TMS9918 screen mode ?
	JP	NC,J722E		; nope,

; set sprite attributes handler sprite mode 0

	POP	AF
	PUSH	HL
	PUSH	DE
	CALL	CALATR
	POP	DE
	PUSH	HL
	CALL	C70AF			; setup VDP for VRAM read
	LD	A,B
	LD	HL,LINWRK
	LD	B,4
J71DD:	INI
	JR	NZ,J71DD
	LD	B,A
	BIT	7,B
	JR	Z,J7201
	LD	(LINWRK+0),IY
	PUSH	IX
	POP	HL
	LD	A,H
	AND	80H
	LD	E,A
	LD	A,L
	LD	HL,LINWRK+3
	JR	Z,J71F9
	ADD	A,20H	; " "
J71F9:	LD	(LINWRK+1),A
	LD	A,(HL)
	AND	7FH
	OR	E
	LD	(HL),A
J7201:	BIT	6,B
	JR	Z,J7211
	LD	HL,LINWRK+3
	LD	A,D
	AND	0FH	; 15
	LD	D,A
	LD	A,(HL)
	AND	0F0H
	OR	D
	LD	(HL),A
J7211:	POP	HL
	EX	(SP),HL
	BIT	5,B
	JR	Z,J7222
	CALL	GSPSIZ
	LD	A,H
	JR	NC,J721F
	ADD	A,A
	ADD	A,A
J721F:	LD	(LINWRK+2),A
J7222:	POP	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK
	LD	B,4			; 4 bytes
	JP	C71A4			; write bytes to VRAM

; set sprite attributes handler sprite mode 1

J722E:	POP	AF
	PUSH	AF
	PUSH	HL
	PUSH	DE
	PUSH	AF
	CALL	CALATR
	POP	AF
	PUSH	HL
	PUSH	AF
	CALL	C70AF			; setup VDP for VRAM read
	LD	A,B
	LD	HL,LINWRK
	LD	B,3
J7242:	INI
	JR	NZ,J7242
	LD	B,A
	POP	AF
	ADD	A,A
	ADD	A,A
	CALL	CALATR
	DEC	H
	DEC	H
	PUSH	HL
	CALL	C70AF			; setup VDP for VRAM read
	LD	A,B
	LD	HL,LINWRK+4
	LD	B,16
J7259:	INI
	JR	NZ,J7259
	LD	B,A
	LD	E,0
	LD	L,0FFH
	BIT	7,B
	JR	Z,J727B
	LD	(LINWRK+0),IY
	PUSH	IX
	POP	HL
	LD	A,H
	AND	80H
	LD	E,A
	LD	A,L
	JR	Z,J7276
	ADD	A,20H	; " "
J7276:	LD	(LINWRK+1),A
	LD	L,7FH
J727B:	POP	IX
	POP	IY
	POP	AF
	BIT	6,B
	JR	Z,J728C
	AND	0FH	; 15
	OR	E
	LD	E,A
	LD	A,L
	AND	0F0H
	LD	L,A
J728C:	PUSH	BC
	LD	D,L
	LD	HL,LINWRK+4
	LD	B,16
J7293:	LD	A,(HL)
	AND	D
	OR	E
	LD	(HL),A
	INC	HL
	DJNZ	J7293
	POP	BC
	POP	HL
	BIT	5,B
	JR	Z,J72AB
	CALL	GSPSIZ
	LD	A,H
	JR	NC,J72A8
	ADD	A,A
	ADD	A,A
J72A8:	LD	(LINWRK+2),A
J72AB:	PUSH	IX
	POP	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK+4
	LD	B,16			; 16 bytes
	CALL	C71A4			; write bytes to VRAM
	PUSH	IY
	POP	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK
	LD	B,3			; 3 bytes
	CALL	C71A4			; write bytes to VRAM
	POP	DE
	LD	A,1FH
	SUB	D
	RET	Z
	LD	E,A
	LD	D,A
	PUSH	IX
	POP	HL
J72D1:	LD	A,L
	ADD	A,10H	; 16
	LD	L,A
	JR	NC,J72D8
	INC	H
J72D8:	CALL	C70AF			; setup VDP for VRAM read
	LD	B,16
J72DD:	IN	A,(C)
	AND	40H	; "@"
	JR	NZ,J72E7
	DJNZ	J72DD
	JR	J72EA

J72E7:	DEC	E
	JR	NZ,J72D1
J72EA:	LD	A,D
	SUB	E
	RET	Z
	LD	E,A
	PUSH	IX
	POP	HL
	PUSH	DE
	LD	A,(LINWRK+4)
	AND	80H
	LD	D,A
J72F8:	LD	A,L
	ADD	A,10H	; 16
	LD	L,A
	JR	NC,J72FF
	INC	H
J72FF:	PUSH	HL
	CALL	C70AF			; setup VDP for VRAM read
	LD	HL,LINWRK+20
	LD	B,16
J7308:	IN	A,(C)
	AND	7FH
	OR	D
	LD	(HL),A
	INC	HL
	DJNZ	J7308
	POP	HL
	PUSH	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK+20
	LD	B,16			; 16 bytes
	CALL	C71A4			; write bytes to VRAM
	POP	HL
	DEC	E
	JR	NZ,J72F8
	POP	DE
	LD	B,E
	PUSH	IY
	POP	HL
J7327:	INC	HL
	INC	HL
	INC	HL
	INC	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	DE,(LINWRK+0)
	OUT	(C),E
	OUT	(C),D
	DJNZ	J7327
	RET

;	  Subroutine read vdp register
;	     Inputs  ________________________
;	     Outputs ________________________

I7339:	LD	A,H
	AND	A
	JR	Z,J7352
	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	XOR	A
	SUB	L
	DI
	CALL	C709C			; read VDP status register
	IN	L,(C)
	XOR	A			; VDP status register 0
	CALL	C709C			; read VDP status register
	EI
	LD	H,0
	RET

J7352:	LD	DE,RG0SAV
	LD	A,L
	CP	9
	JR	C,J7364
	LD	DE,RG8SAV-9
	CP	25
	JR	C,J7364
	LD	DE,RG25SAV-26
J7364:	ADD	HL,DE
	LD	L,(HL)
	LD	H,0
	RET

;	  Subroutine change screen mode handler (with MSX2+ screen mode support)
;	     Inputs  A = screen mode (0-12)
;	     Outputs ________________________

C7369:	CP	8+1			; special MSX2+ screen mode ?
	JP	C,CHGMOD		; nope, use CHGMOD BIOS
	LD	HL,RG25SAV
	LD	B,(HL)
	LD	C,B			; save current VDP register 25
	SET	3,B
	RES	4,B			; assume YAE = 0, YJK = 1
	CP	12			; screen mode 12 ?
	JR	Z,J73AE			; yep, setup screen mode 8 if needed, setup YAE/YJK and quit
	SET	4,B			; assume YAE = 1, YJK = 1
	LD	HL,MODE
	CP	11			; screen mode 11 ?
	SET	5,(HL)			; assume flag screen mode 11
	JR	Z,J7388			; yep,
	RES	5,(HL)			; flag not screen mode 11
J7388:	BIT	3,C			; screen already in YJK mode ?
	JR	Z,J73B2			; nope, setup screen mode 8, setup YAE/YJK and quit
	BIT	4,C			; screen already in YAE mode ?
	RET	NZ			; yep, quit
	PUSH	BC
	LD	A,0F7H			; YAE attribute
	LD	B,1
	LD	DE,0			; X1 = 0
	LD	L,E
	LD	H,D			; Y1 = 0
	LD	IX,255			; X2 = 255
	LD	IY,255			; Y2 = 255
	CALL	C6E36			; filled box
	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	CALL	C708A			; wait until VDP command is done
	JR	J73B8			; setup YAE/YJK and quit

J73AE:	BIT	3,C			; screen already in YJK mode ?
	JR	NZ,J73B9		; yep, setup YAE/YJK and quit
J73B2:	PUSH	BC
	LD	A,8
	CALL	CHGMOD			; screen mode 8
J73B8:	POP	BC
J73B9:	LD	C,25
	JP	WRTVDP			; setup YAE/YJK

;	  Subroutine check if screen mode 10
;	     Inputs  ________________________
;	     Outputs ________________________

C73BE:	LD	A,(IDBYT2)
	CP	2			; MSX2+ or higher ?
	SBC	A,A
	RET	NZ			; nope, quit
	LD	A,(RG25SAV)
	AND	18H			; YJK/YAE
	CP	18H			; color via color palette ?
	RET	NZ			; nope, quit
	LD	A,(MODE)
	AND	20H			; screen 11
	RET

;	  Subroutine ?? screen mode 10 action
;	     Inputs  ________________________
;	     Outputs ________________________

C73D3:	LD	C,A
	CALL	C73BE			; check if screen mode 10
	LD	A,C
	RET	NZ			; nope, quit
	EX	(SP),HL
	LD	(SWPTMP+4),HL		; save return address
	POP	HL
	INC	B
	DJNZ	J73F9

; B=0

	PUSH	AF
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	LD	A,0FH
	LD	B,1
	CALL	C73FD			; invoke saved return address
	POP	IY
	POP	IX
	POP	HL
	POP	DE
	POP	AF
	LD	B,2
	SCF

J73F9:	RLA
	RLA
	RLA
	RLA

;	  Subroutine invoke saved return address
;	     Inputs  ________________________
;	     Outputs ________________________

C73FD:	PUSH	HL
	LD	HL,(SWPTMP+4)
	EX	(SP),HL
	RET

;	  Subroutine setup VDP active screen page and low byte VRAM address
;	     Inputs  HL = VRAM address
;	     Outputs ________________________

C7403:	INC	C
	LD	A,(IDBYT2)
	AND	A			; MSX1 ?
	JR	Z,J7426			; yep, skip active screen page support
	PUSH	DE
	LD	A,H
	RLCA
	RLCA
	AND	03H
	LD	E,A
	LD	A,(SCRMOD)
	CP	7
	LD	A,(ACPAGE)
	JR	C,J741C
	ADD	A,A
J741C:	ADD	A,A
	ADD	A,E
	POP	DE
	DI
	OUT	(C),A
	LD	A,128+14
	OUT	(C),A			; setup page
J7426:	DI
	OUT	(C),L			; low byte VRAM address
	LD	A,H
	AND	3FH
	RET

;	  Subroutine force X and Y coordinate within screen limits, when limits enabled
;	     Inputs  IX = X value, IY = Y value
;	     Outputs ________________________

C742D:	PUSH	HL
	PUSH	DE
	PUSH	IX
	POP	DE
	PUSH	IY
	POP	HL
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	PUSH	HL
	POP	IY
	PUSH	DE
	POP	IX
	POP	DE
	POP	HL
	RET

;	  Subroutine force X and Y coordinate within screen limits, when limits enabled
;	     Inputs  DE = X value, HL = Y value
;	     Outputs ________________________

C7441:	LD	A,(MODE)
	AND	10H			; 212 lines mode ?
	RET	NZ			; nope, quit
	LD	A,D
	ADD	A,A			; X value negative ?
	JR	NC,J7450		; nope,
	LD	DE,0			; yep, use X = 0
	JR	J745C

J7450:	LD	A,(SCRMOD)
	RRCA
	AND	01H			; screen mode 5 & 7 512, screen mode 6 & 8 256
	CP	D			; X value bigger or equal to max X ?
	JR	NC,J745C		; nope,
	LD	E,0FFH
	LD	D,A			; yep, use X = 255 or 511
J745C:	LD	A,H
	ADD	A,A			; Y value negative ?
	JR	NC,J7464		; nope,
	LD	HL,0			; yep, use Y = 0
	RET

J7464:	JR	NZ,J746A		; Y > 255, use Y = 211
	LD	A,L
	CP	211+1
	RET	C
J746A:	LD	HL,211
	RET

J746E:	XOR	A
	LD	IX,CLS
	JP	J74EE

J7476:	LD	BC,M58FC		; line handler
	JR	J7483

J747B:	LD	BC,M5912		; line box handler
	JR	J7483

J7480:	LD	BC,M58C1		; line boxfill handler
J7483:	POP	AF
	LD	(ATRBYT),A
	PUSH	BC
	LD	(GXPOS),DE
	LD	(GYPOS),HL
	PUSH	IX
	POP	BC
	PUSH	IY
	POP	DE
	POP	IX
	JP	J74EE

J749A:	POP	AF
	LD	(ATRBYT),A
	LD	C,E
	LD	B,D
	EX	DE,HL
	LD	IX,M57F1+4
	JP	J74EE			; PSET handler

J74A8:	LD	C,E
	LD	B,D
	EX	DE,HL
	CALL	SCALXY
	LD	HL,-1
	RET	NC
	CALL	MAPXYC
	CALL	READC
	LD	L,A
	LD	H,0
	RET

;	  Subroutine paint handler
;	     Inputs  ________________________
;	     Outputs ________________________

C74BC:	CALL PNTINI   ;CALL PAINT_FIX.1
	LD	A,B
	LD	(ATRBYT),A
	LD	C,E
	LD	B,D
	EX	DE,HL
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J74D5			; yep, use MSX1 paint handler
	LD	IX,S266E
	JP	PAINT_FIX.2         ; EXTROM			; PAINT handler (subrom)

J74D5:	LD	IX,M59E3		; PAINT handler
	JP	J74EE

;	  Subroutine circle
;	     Inputs  A = color
;	     Outputs ________________________

I74DC:	EX	DE,HL
	LD	HL,LINWRK+3
	LD	(HL),0
	DEC	HL
	LD	(HL),A
	DEC	HL
	LD	(HL),0FH
	DEC	HL
	LD	(HL),","		; ,<int constant>
J74EA:
	LD	IX,M5B19		; CIRCLE handler
J74EE:	CALL	CALBAS
	EI
	RET

;	  Subroutine print tab
;	     Inputs  ________________________
;	     Outputs ________________________

C74F3:	LD	A,9
	RST	18H
	RET

;	  Subroutine print new line
;	     Inputs  ________________________
;	     Outputs ________________________

C74F7:	LD	A,13
	RST	18H
	LD	A,10
	RST	18H
	RET

;	  Subroutine print string
;	     Inputs  ________________________
;	     Outputs ________________________

C74FE:	LD	B,(HL)
	LD	A,B
	AND	A
	RET	Z
J7502:	INC	HL
	LD	A,(HL)
	RST	18H
	DJNZ	J7502
	RET

;	  Subroutine print integer
;	     Inputs  ________________________
;	     Outputs ________________________

C7508:	CALL	XBASIC_CAST_INTEGER_TO_STRING			; integer to text
	JR	J7510

;	  Subroutine print float
;	     Inputs  ________________________
;	     Outputs ________________________

C750D:	CALL	C7C2B			; float to text
J7510:	CALL	XBASIC_PRINT_STR			; print string
	LD	A," "
	RST	18H
	RET

;	  Subroutine print string and get numeric input
;	     Inputs  ________________________
;	     Outputs ________________________

I7517:	CALL	XBASIC_PRINT_STR			; print string

;	  Subroutine get numeric input
;	     Inputs  ________________________
;	     Outputs ________________________

J751A:	CALL	QINLIN
	JP	C,J4DFA			; abort, end program
	EX	DE,HL
	INC	DE
	CALL	C7D18			; convert numeric string to float
	LD	A,(DE)
	AND	A			; at end of string ?
	RET	Z			; yep, quit
	LD	HL,I7530
	CALL	XBASIC_PRINT_STR			; print string
	JR	J751A			; try again

I7530:	DEFB	18
	DEFB	"?Redo from start",13,10

;	  Subroutine print string and get string input
;	     Inputs  ________________________
;	     Outputs ________________________

I7543:	CALL	XBASIC_PRINT_STR			; print string

;	  Subroutine get string input
;	     Inputs  ________________________
;	     Outputs ________________________

I7546:	CALL	QINLIN
	JP	C,J4DFA			; abort, end program
	INC	H
	LD	E,L
	LD	D,H
	INC	DE
	LD	BC,256
	LDDR
	INC	HL
	LD	B,0FFH
J7558:	INC	B
	INC	DE
	LD	A,(DE)
	AND	A
	JR	NZ,J7558
	LD	(HL),B
	RET

; read string input handler

I7560:	CALL	C75BE			; skip to the start of DATA
J7563:	LD	A,(HL)
	INC	HL
	CP	" "
	JR	Z,J7563			; skip spaces
	LD	DE,BUF+1
	LD	B,0			; current string length = 0
	CP	'"'			; string indicator ?
	JR	Z,J7593			; yep,
	DEC	HL
J7573:	LD	A,(HL)
	AND	A			; end of line ?
	JR	Z,J758B			; yep, end of input
	CP	":"			; statement seperator ?
	JR	Z,J758B			; yep, end of input
	CP	","			; DATA item seperator ?
	JR	Z,J758B			; yep, end of input
	LD	(DE),A
	INC	DE
	INC	HL
	INC	B
	JR	J7573			; next character

J7585:	INC	HL
	LD	A,(HL)
	CP	" "
	JR	Z,J7585
J758B:	LD	(DATPTR),HL		; update current DATA pointer
	LD	HL,BUF
	LD	(HL),B			; set string length
	RET

J7593:	LD	A,(HL)
	AND	A			; end of line ?
	JR	Z,J758B			; yep, end of input
	CP	'"'			; string indicator ?
	JR	Z,J7585			; yep, end of string
	LD	(DE),A
	INC	DE
	INC	HL
J759E:	INC	B
	JR	J7593			; next character

; read numeric input handler

I75A1:	CALL	C75BE			; skip to the start of DATA
	EX	DE,HL
	CALL	C7D18			; convert numeric string to float
	LD	(DATPTR),DE		; save DATA pointer
	LD	A,(DE)
	AND	A			; end of line ?
	RET	Z			; yep, quit
	CP	":"			; end of statement ?
	RET	Z			; yep, quit
	CP	","			; next data item ?
	RET	Z			; yep, quit
	LD	HL,(DATLIN)
	LD	(CURLIN),HL		; update current BASIC line number (generated error contains the current line number)
	JP	J4E16			; BASIC error: syntax error

;	  Subroutine skip to the start of DATA
;	     Inputs  ________________________
;	     Outputs ________________________

C75BE:	LD	HL,(DATPTR)		; current DATA pointer
	LD	A,(HL)
	CP	","
	JR	NZ,J75DC
	INC	HL
	RET

J75C8:	XOR	A
	LD	B,0FFH
	CPIR				; skip to end of line
J75CD:	LD	A,(HL)
	INC	HL
	OR	(HL)			; end of program ?
	JP	Z,J4E07			; yep, BASIC error: out of data
	INC	HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)			; line number
	LD	(DATLIN),DE		; update current DATA line number
	INC	HL

J75DC:	LD	B,0			; reset in a string flag
J75DE:	LD	A,(HL)
	INC	HL
	AND	A			; end of line ?
	JR	Z,J75CD			; yep, to the next line
	CP	84H			; DATA token ?
	JR	NZ,J75EA		; nope,
	INC	B
	DEC	B			; string indicator ?
	RET	Z			; nope, quit
J75EA:	CP	'"'			; start of DATA string ?
	JR	NZ,J75F2		; nope,
	INC	B
	DJNZ	J75DC			; in a string, continue to next character
	INC	B			; B=1, set in a sting flag
J75F2:	CP	0FFH			; function token ?
	INC	HL
	JR	Z,J75DE			; yep, continue to next character
	DEC	HL
	CP	8FH			; REM token ?
	JR	Z,J75C8			; yep, skip line and continue
	CP	1FH+1			; special number token ?
	JR	NC,J75DE		; nope, continue to next character
	CP	0BH			; unknown token ?
	JR	C,J75DE			; yep, continue to next character
	CP	0FH			; integer 11-255 ?
	LD	C,1
	JR	Z,J761C			; yep, skip next byte
	INC	C
	JR	C,J761C			; octal number, hexadecimal number, linepointer or linenumber, skip next 2 bytes
	CP	1BH			; integer 0 - integer 9 ? (?? BUG: should be CP 1CH, because of integer 10)
	JR	C,J75DE			; yep, continue to next character
	SUB	1CH			; integer ?
	JR	Z,J761C			; yep, skip next 2 bytes
	DEC	A			; single real ?
	LD	C,4
	JR	Z,J761C			; yep, skip next 4 bytes
	LD	C,8			; double real, skip next 8 bytes
J761C:	LD	A,L
	ADD	A,C
	LD	L,A
	JR	NC,J75DE
	INC	H
	JR	J75DE			; continue to next character

;	  Subroutine multiply
;	     Inputs  ________________________
;	     Outputs ________________________

C7624:	LD	A,40H
	DEFB	0CBH,037H		; SLL A (Z80), TST A (R800)
	CP	81H
	JR	Z,J7631			; Z80,
	LD	C,E
	LD	B,D
	DEFB	0EDH,0C3H		; MULUW HL,BC
	RET

J7631:	LD	C,L
	LD	B,H
	LD	HL,0
	LD	A,16
J7638:	ADD	HL,HL
	EX	DE,HL
	ADD	HL,HL
	EX	DE,HL
	JR	NC,J763F
	ADD	HL,BC
J763F:	DEC	A
	JR	NZ,J7638
	RET

;	  Subroutine divide
;	     Inputs  ________________________
;	     Outputs ________________________

C7643:	PUSH	DE
	PUSH	HL
	BIT	7,D
	JR	NZ,J764F
	XOR	A
	SUB	E
	LD	E,A
	SBC	A,A
	SUB	D
	LD	D,A
J764F:	LD	C,E
J7650:	LD	B,D
	BIT	7,H
	JR	Z,J765B
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A
J765B:	EX	DE,HL
	XOR	A
	LD	L,A
	LD	H,A
	LD	A,E
	LD	E,16
	RLA
	RL	D
J7665:	ADC	HL,HL
	PUSH	HL
	ADD	HL,BC
	JR	NC,J766E
	INC	SP
	INC	SP
	DEFB	030H			; pseudo JR NC,xx (skip next instruction)
J766E:	POP	HL
	RLA
	RL	D
	DEC	E
	JR	NZ,J7665
	LD	E,A
	EX	DE,HL
	POP	AF
	AND	A
	JP	P,J7684
	XOR	A
	SUB	E
	LD	E,A
	SBC	A,A
	SUB	D
	LD	D,A
	LD	A,80H
J7684:	POP	BC
	XOR	B
	RET	P
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A
	RET

;	  Subroutine RND handler
;	     Inputs  ________________________
;	     Outputs ________________________

I768E:	LD	A,B
	AND	A
	JR	Z,J76C9
	BIT	7,H
	JR	Z,J769F
	LD	(SWPTMP+2),HL
	LD	H,B
	LD	L,"5"
	LD	(SWPTMP+0),HL
J769F:	LD	HL,(SWPTMP+0)
	LD	E,L
	LD	D,H
	ADD	HL,HL
	LD	C,L
	LD	B,H
	JR	NC,J76AA
	INC	BC
J76AA:	ADD	HL,DE
	LD	(SWPTMP+0),HL
	LD	HL,(SWPTMP+2)
	LD	E,L
	LD	D,H
	ADC	HL,HL
	ADD	HL,DE
	ADD	HL,BC
	LD	(SWPTMP+2),HL
J76BA:	LD	A,L
	OR	H
	LD	B,A
	RET	Z
	LD	B,80H
J76C0:	BIT	7,H
	RES	7,H
	RET	NZ
	ADD	HL,HL
	DEC	B
	JR	J76C0

J76C9:	LD	HL,(SWPTMP+2)
	JR	J76BA

;	  Subroutine remainer of divide integer
;	     Inputs  ________________________
;	     Outputs ________________________

C76CE:	CALL	C7643			; divide
	EX	DE,HL
	RET

;	  Subroutine subtract floats
;	     Inputs  ________________________
;	     Outputs ________________________

C76D3:	LD	A,D
	XOR	80H
	LD	D,A

;	  Subroutine add floats
;	     Inputs  ________________________
;	     Outputs ________________________

C76D7:	LD	A,C
	AND	A
	RET	Z
	LD	A,B
	AND	A
	JP	Z,J7745
	SUB	C
	JR	NC,J76E6
	NEG
	EX	DE,HL
	LD	B,C
J76E6:	CP	11H
	RET	NC
	LD	C,A
	LD	A,H
	XOR	D
	ADD	A,A
	LD	A,H
	PUSH	AF
	SET	7,H
	SET	7,D
	INC	C
	DEC	C
	JR	Z,J7701
J76F7:	SRL	D
	RR	E
	DEC	C
	JR	NZ,J76F7
	JR	NC,J7701
	INC	DE
J7701:	POP	AF
	JR	C,J771C
	ADD	HL,DE
	JR	NC,J770E
	INC	B
	JR	Z,J7715
	RR	H
	RR	L
J770E:	RES	7,H
J7710:	AND	A
	RET	P
	SET	7,H
	RET

J7715:	LD	B,0FFH
	LD	HL,7FFFH		; (max positive float) 1.701E+38
	JR	J7710

J771C:	AND	A
	SBC	HL,DE
	JR	NC,J7728
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	CPL
J7728:	LD	C,A
J7729:	LD	A,H
	AND	A
	JR	NZ,J773C
	OR	L
	JR	Z,J7739
	LD	H,L
	LD	L,0
	LD	A,B
	SUB	08H	; 8
	LD	B,A
	JR	NC,J773C
J7739:	LD	B,0
	RET

J773C:	LD	A,C
J773D:	BIT	7,H
	JR	NZ,J770E
	ADD	HL,HL
	DJNZ	J773D
	RET

J7745:	LD	B,C
	EX	DE,HL
	RET

;	  Subroutine multipy floats
;	     Inputs  ________________________
;	     Outputs ________________________

C7748:	LD	A,B
	AND	A
	RET	Z
	LD	A,C
	AND	A
	JR	Z,J7739
	ADD	A,B
	LD	B,A
	RRA
	XOR	B
	LD	A,B
	JP	P,J7823
	ADD	A,80H
	LD	B,A
	RET	Z
	LD	A,H
	XOR	D
	LD	C,A
	PUSH	BC
	SET	7,H
	SET	7,D
	LD	A,40H
	DEFB	0CBH,037H		; SLL A (Z80), TST A (R800)
	CP	81H
	JR	Z,J7774			; Z80,
	LD	C,E
	LD	B,D
	DEFB	0EDH,0C3H		; MULUW HL,BC
	LD	A,H
	EX	DE,HL
	JP	J781A

J7774:	LD	C,L
	LD	A,H
	LD	HL,0
	RRA
	RR	C
	JR	NC,J777F
	ADD	HL,DE
J777F:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7789
	ADD	HL,DE
J7789:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7793
	ADD	HL,DE
J7793:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J779D
	ADD	HL,DE
J779D:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77A7
	ADD	HL,DE
J77A7:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77B1
	ADD	HL,DE
J77B1:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77BB
	ADD	HL,DE
J77BB:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77C5
	ADD	HL,DE
J77C5:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77CF
	ADD	HL,DE
J77CF:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77D9
	ADD	HL,DE
J77D9:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77E3
	ADD	HL,DE
J77E3:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77ED
	ADD	HL,DE
J77ED:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77F7
	ADD	HL,DE
J77F7:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7801
	ADD	HL,DE
J7801:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J780B
	ADD	HL,DE
J780B:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7815
	ADD	HL,DE
J7815:	RR	H
	RR	L
	RRA
J781A:	POP	BC
	ADD	A,A
	JP	NC,J773C
	INC	HL
	JP	J773C

J7823:	AND	A
	JP	P,J7739
	LD	A,H
	XOR	D
	JP	J7715

;	  Subroutine divide floats
;	     Inputs  ________________________
;	     Outputs ________________________

C782C:	LD	A,C
	AND	A
	LD	A,H
	JP	Z,J7715
	LD	A,B
	AND	A
	RET	Z
	SUB	C
	LD	B,A
	RRA
	XOR	B
	LD	A,B
	JP	M,J7823
	ADD	A,80H
	LD	B,A
	RET	Z
	LD	A,H
	XOR	D
	INC	B
	JP	Z,J7715
	LD	C,A
	PUSH	BC
	SET	7,H
	SET	7,D
	XOR	A
	LD	C,A
	LD	B,16
	JR	J7857

J7853:	ADC	HL,HL
	JR	C,J7873
J7857:	SBC	HL,DE
	JR	NC,J785C
	ADD	HL,DE
J785C:	CCF
J785D:	RL	C
	RLA
	DJNZ	J7853
	SRL	D
	RR	E
	AND	A
	SBC	HL,DE
	LD	L,C
	LD	H,A
	POP	BC
	JP	C,J7729
	INC	HL
	JP	J7729

J7873:	OR	A
	SBC	HL,DE
	JR	J785D

;	  Subroutine ^ handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7878:	LD	A,E
	OR	D
	JP	Z,C7924			; quit with 1.0
	BIT	7,D
	PUSH	AF
	JP	P,J7889
	XOR	A
	SUB	E
	LD	E,A
	SBC	A,A
	SUB	D
	LD	D,A
J7889:	PUSH	DE
	LD	C,B
	EX	DE,HL
	CALL	C7924			; 1.0
J788F:	EX	(SP),HL
	SRL	H
	RR	L
	EX	(SP),HL
	JR	NC,J789F
	PUSH	DE
	PUSH	BC
	CALL	C7748			; multipy floats
	POP	DE
	LD	C,E
	POP	DE
J789F:	EX	(SP),HL
	LD	A,L
	OR	H
	EX	(SP),HL
	JR	Z,J78B4
	PUSH	HL
	PUSH	BC
	LD	B,C
	LD	H,D
	LD	L,E
	CALL	C7748			; multipy floats
	LD	C,B
	EX	DE,HL
	POP	HL
	LD	B,H
	POP	HL
	JR	J788F

J78B4:	POP	AF
	POP	AF
	RET	P

;	  Subroutine 1/x
;	     Inputs  B:HL = float
;	     Outputs ________________________

C78B7:	LD	C,B
	EX	DE,HL			; C:DE = float
	CALL	C7924			; 1.0
	JP	C782C			; divide floats

;	  Subroutine ^
;	     Inputs  ________________________
;	     Outputs ________________________

I78BF:	PUSH	DE
	PUSH	BC
	CALL	C7B01			; LOG handler
	POP	DE
	LD	C,E
	POP	DE			; C:DE = float
	CALL	C7748			; multipy floats
	JP	J7AA8			; EXP handler

;	  Subroutine convert unsigned integer to float
;	     Inputs  HL = unsigned integer
;	     Outputs B:HL = float

C78CD:	BIT	7,H			; positive integer ?
	PUSH	AF
	RES	7,H			; to postive
	CALL	C78DF			; convert integer to float
	POP	AF
	RET	Z			; positive, quit
	LD	C,90H
	LD	DE,0			; 3.277E+04
	JP	C76D7			; add floats

;	  Subroutine convert integer to float
;	     Inputs  HL = integer
;	     Outputs B:HL = float

C78DF:	LD	A,L
	OR	H
	JR	Z,J78FF
	BIT	7,H
	PUSH	AF
	JR	Z,J78EE
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A
J78EE:	LD	B,90H
J78F0:	BIT	7,H
	JR	NZ,J78F8
	ADD	HL,HL
	DEC	B
	JR	J78F0

J78F8:	RES	7,H
	POP	AF
	RET	Z
	SET	7,H
	RET

J78FF:	LD	B,A			; 0.0
	RET

;	  Subroutine convert float to integer
;	     Inputs  ________________________
;	     Outputs ________________________

C7901:	LD	A,B
	AND	A			; B=0 ?
	JR	Z,J7926			; yep, return float 0.0
	DEC	B			; B=1 ?
	JP	P,J7926			; yep, return float 0.0
	BIT	7,H
	PUSH	AF			; save sign
	SET	7,H
	LD	DE,0
J7911:	ADD	HL,HL
	RL	E
	RL	D
	DEC	B
	JP	M,J7911
	EX	DE,HL
	POP	AF			; sign set ?
	RET	Z			; nope, quit
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A			; yep, negate
	RET

;	  Subroutine float 1.0
;	     Inputs  ________________________
;	     Outputs ________________________

C7924:	LD	B,81H
J7926:	LD	HL,0
	RET

;	  Subroutine float =
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C792A:	CALL	C7966			; compare floats
	LD	HL,0
	RET	NZ			; not equal, return false
	DEC	HL			; equal, return true
	RET

;	  Subroutine float <>
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7933:	CALL	C7966			; compare floats
	LD	HL,0
	RET	Z			; equal, return false
	DEC	HL			; not equal, return true
	RET

;	  Subroutine float >
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C793C:	CALL	C7966			; compare floats
	LD	HL,0
	RET	C			; smaller, return false
	RET	Z			; equal, return false
	DEC	HL			; bigger, return true
	RET

;	  Subroutine float >=
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7946:	CALL	C7966			; compare floats
	LD	HL,0
	RET	C			; smaller, return false
	DEC	HL			; bigger or equal, return true
	RET

;	  Subroutine float <
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C794F:	CALL	C7966			; compare floats
	LD	HL,0
	RET	NC			; bigger or equal, return false
	DEC	HL			; smaller, return true
	RET

;	  Subroutine float <=
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7958:	CALL	C7966			; compare floats
	LD	HL,-1
	RET	C			; smaller, return true
	RET	Z			; equal, return true
	INC	HL			; bigger, return false
	RET

;	  Subroutine float <=>
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7962:	LD	HL,-1			; return true
	RET

;	  Subroutine compare floats
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs ________________________

C7966:	LD	A,C
	AND	A			; float2 0.0 ?
	JR	Z,J7986			; yep,
	LD	A,B
	AND	A			; float1 0.0 ?
	LD	A,D
	JR	Z,J7981			; yep,
	XOR	H			; float1 has same sign as float2 ?
	LD	A,D
	JP	M,J7981			; nope,
	LD	A,C
	CP	B
	JR	NZ,J797F
	LD	A,D
	CP	H
	JR	NZ,J797F
	LD	A,E
	SUB	L
	RET	Z
J797F:	RRA
	XOR	D
J7981:	CPL
	OR	01H
	RLA
	RET

J7986:	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit
	LD	A,H
	RLA
	RET

;	  Subroutine FIX handler
;	     Inputs  ________________________
;	     Outputs ________________________

C798C:	BIT	7,H
	PUSH	AF			; save sign
	RES	7,H			; force positive
	CALL	C7999			; INT handler
	POP	AF			; restore sign
	RET	Z			; was a positive, quit
	SET	7,H			; force negative
	RET

;	  Subroutine INT handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7999:	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit
	CP	91H			; > float 65540 ?
	RET	NC			; yep, quit
	SUB	81H			; < 1 or < -1 ?
	JR	C,J79D7			; yep,
	LD	DE,8000H
	JR	Z,J79AF
J79A8:	SRA	D
	RR	E
	DEC	A
	JR	NZ,J79A8
J79AF:	BIT	7,H
	JR	NZ,J79BA
	LD	A,H
	AND	D
	LD	H,A
	LD	A,L
	AND	E
	LD	L,A
	RET

J79BA:	PUSH	HL
	LD	A,H
	AND	D
	LD	H,A
	LD	A,L
	AND	E
	LD	L,A
	EX	(SP),HL
	LD	A,D
	CPL
	AND	H
	JR	NZ,J79CE
	LD	A,E
	CPL
	AND	L
	JR	NZ,J79CE
	POP	HL
	RET

J79CE:	POP	HL
	LD	C,81H
	LD	DE,8000H		; float -1.0
	JP	C76D7			; add floats

J79D7:	BIT	7,H
	LD	B,0			; float 0.0
	RET	Z
	LD	B,81H
	LD	HL,8000H		; float -1.0
	RET

;	  Subroutine COS handler
;	     Inputs  ________________________
;	     Outputs ________________________

C79E2:	LD	C,81H
	LD	DE,4910H		; float 1.571 (PI/2)
	CALL	C76D7			; add floats

;	  Subroutine SIN handler
;	     Inputs  ________________________
;	     Outputs ________________________

C79EA:	LD	A,B
	CP	77H			; < 0.0004883 or < -0.0004883 ?
	RET	C			; yep, quit
	BIT	7,H			; positive float ?
	JR	Z,C79FC			; yep, calculate sinus
	RES	7,H			; force positive
	CALL	C79FC			; calculate sinus

;	  Subroutine change sign
;	     Inputs  ________________________
;	     Outputs ________________________

J79F7:	LD	A,H
	XOR	80H
	LD	H,A
	RET

;	  Subroutine calculate sinus
;	     Inputs  ________________________
;	     Outputs ________________________

C79FC:	LD	C,7EH
	LD	DE,22FAH		; float 0.1592
	CALL	C7748			; multipy floats
	PUSH	HL
	PUSH	BC
	CALL	C7999			; INT handler
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	CALL	C76D3			; subtract floats
	LD	C,7FH
	LD	DE,0			; float 0.25
	CALL	C7966			; compare floats
	JR	C,J7A31			; smaller,
	LD	C,80H
	LD	DE,0C000H		; float -0.75
	CALL	C76D7			; add floats
	SET	7,H			; force negative
	LD	C,7FH
	LD	DE,0			; float 0.25
	CALL	C76D7			; add floats
	LD	A,H
	XOR	80H
	LD	H,A			; change sign
J7A31:	BIT	7,H
	PUSH	AF			; save sign
	RES	7,H			; force positive
	LD	IX,I7A53
	CALL	C7B88			; polynomial approximation odd series
	POP	AF			; restore sign
	RET	Z			; positive float, quit
	JR	J79F7			; change sign and quit

;	  Subroutine TAN handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7A41:	PUSH	HL
	PUSH	BC
	CALL	C79E2			; COS handler
	LD	A,B
	POP	BC
	EX	(SP),HL
	PUSH	AF
	CALL	C79EA			; SIN handler
	POP	AF
	LD	C,A
	POP	DE
	JP	C782C			; divide floats

I7A53:	DEFB	5
	DEFB	086H,01EH,0D8H		; float  39.71
	DEFB	087H,099H,026H		; float -76.58
	DEFB	087H,023H,034H		; float  81.60
	DEFB	086H,0A5H,05EH		; float -41.34
	DEFB	083H,049H,010H		; float  6.283

;	  Subroutine ATN handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7A63:	BIT	7,H			; postive float ?
	JR	Z,C7A6E			; yep, calculate arc tangent and quit
	RES	7,H			; force positive
	CALL	C7A6E			; calculate arc tangent
	JR	J79F7			; change sign and quit

;	  Subroutine calculate arc tangent
;	     Inputs  ________________________
;	     Outputs ________________________

C7A6E:	LD	A,B
	CP	81H			; < 1.0 or < -1.0 ?
	JR	C,C7A85			; yep,
	CALL	C78B7			; 1/x
	CALL	C7A85			; calculate ATN
	LD	A,H
	XOR	80H
	LD	H,A			; change sign
	LD	C,81H
	LD	DE,4910H		; float 1.571 (PI/2)
	JP	C76D7			; add floats

;	  Subroutine calculate ATN
;	     Inputs  ________________________
;	     Outputs ________________________

C7A85:	LD	IX,I7A8C		; ATN table
	JP	C7B88			; polynomial approximation odd series

I7A8C:	DEFB	9
	DEFB	078H,03BH,0D7H		; float  0.002866
	DEFB	07BH,084H,06EH		; float -0.01617
	DEFB	07CH,02FH,0C2H		; float  0.04291
	DEFB	07DH,09AH,031H		; float -0.07529
	DEFB	07DH,05AH,03EH		; float  0.1066
	DEFB	07EH,091H,080H		; float -0.1421
	DEFB	07EH,04CH,0BCH		; float  0.1999
	DEFB	07FH,0AAH,0AAH		; float -0.3333
	DEFB	081H,000H,000H		; float  1.0

;	  Subroutine EXP handler
;	     Inputs  ________________________
;	     Outputs ________________________

J7AA8:	LD	C,81H
	LD	DE,38AAH		; float 1.443
	CALL	C7748			; multipy floats
	LD	A,B
	CP	88H			; > float 128 ?
	JR	NC,J7AE1		; yep,
	CP	68H			; < float -0.0000000298
	JP	C,C7924			; yep, uit with 1.0
	PUSH	HL
	PUSH	BC
	CALL	C7999			; INT handler
	PUSH	HL
	PUSH	BC
	CALL	C7901			; convert float to integer
	LD	A,L
	ADD	A,81H
	POP	HL
	POP	DE
	POP	BC
	LD	C,H
	POP	HL
	JR	Z,J7AE1
	PUSH	AF
	CALL	C76D3			; subtract floats
	LD	IX,I7AEB
	CALL	C7B99			; polynomial approximation even series
	POP	AF
	LD	C,A
	LD	DE,0
	JP	C7748			; multipy floats

J7AE1:	BIT	7,H
	LD	B,0			; float 0.0
	RET	NZ
	DEC	B
	LD	HL,7FFFH		; float 1.701E+38 (max float)
	RET

I7AEB:	DEFB	7
	DEFB	074H,059H,088H		; float 0.0002075
	DEFB	077H,026H,098H		; float 0.001271
	DEFB	07AH,01EH,01EH		; float 0.009651
	DEFB	07CH,063H,050H		; float 0.0555
	DEFB	07EH,075H,0FEH		; float 0.2402
	DEFB	080H,031H,072H		; float 0.6932
	DEFB	081H,000H,000H		; float 1.0

;	  Subroutine LOG handler
;	     Inputs  B:HL = float
;	     Outputs ________________________

C7B01:	BIT	7,H			; sign negative ?
	RET	NZ			; yep, quit (error)
	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit (error)
	LD	C,B
	LD	D,H
	LD	E,L			; C:DE = float
	LD	A,80H
	LD	B,A
	XOR	C
	PUSH	AF
	PUSH	HL
	PUSH	BC
	LD	IX,I7B46
	CALL	C7B99			; polynomial approximation even series
	POP	AF
	POP	DE
	PUSH	HL
	PUSH	BC
	LD	B,A
	EX	DE,HL
	LD	IX,I7B53
	CALL	C7B99			; polynomial approximation even series
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	CALL	C782C			; divide floats
	POP	AF
	PUSH	HL
	PUSH	BC
	RLCA
	RRCA
	LD	L,A
	SBC	A,A
	LD	H,A
	CALL	C78DF			; convert integer to float
	POP	AF
	LD	C,A
	POP	DE
	CALL	C76D7			; add floats
	LD	C,80H
	LD	DE,3172H		; float 0.6932
	JP	C7748			; multipy floats and quit

I7B46:	DEFB	4
	DEFB	083H,019H,0F8H		; float  4.812
	DEFB	083H,043H,063H		; float  6.106
	DEFB	084H,08DH,0CDH		; float -8.863
	DEFB	082H,083H,080H		; float -4.109

I7B53:	DEFB	4
	DEFB	081H,000H,000H		; float 1.0
	DEFB	083H,04DH,0B1H		; float 6.428
	DEFB	083H,011H,072H		; float 4.545
	DEFB	07FH,035H,005H		; float 0.3536

;	  Subroutine SQR handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7B60:	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit
	RES	7,H			; force positive
	RRA
	ADC	A,40H
	LD	C,A
	LD	D,H
	LD	E,L
	LD	A,3
J7B6D:	PUSH	AF
	PUSH	HL
	PUSH	BC
	PUSH	DE
	PUSH	BC
	CALL	C782C			; divide floats
	POP	DE
	LD	C,E
	POP	DE
	CALL	C76D7			; add floats
	DEC	B
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	POP	AF
	DEC	A
	JR	NZ,J7B6D
	LD	B,C
	EX	DE,HL
	RET

;	  Subroutine polynomial approximation odd series
;	     Inputs  ________________________
;	     Outputs ________________________

C7B88:	PUSH	HL
	PUSH	BC			; save float
	LD	C,B
	LD	D,H
	LD	E,L			; C:DE = float
	CALL	C7748			; multipy floats
	CALL	C7B99			; polynomial approximation even series
	POP	AF
	LD	C,A
	POP	DE			; restore float in C:DE
	JP	C7748			; multipy floats

;	  Subroutine polynomial approximation even series
;	     Inputs  B:HL = float, IX = pointer to serie of floats
;	     Outputs ________________________
; X*N1+N2

C7B99:	LD	A,(IX+0)		; number of items in serie
	INC	IX
	PUSH	AF			; save number of items in serie
	PUSH	HL
	PUSH	BC			; save float
	LD	B,(IX+0)
	LD	H,(IX+1)
	LD	L,(IX+2)		; B:HL = first float of serie
	INC	IX
	INC	IX
	INC	IX			; update pointer to next float in serie
J7BB0:	POP	AF
	LD	C,A
	POP	DE			; restore float in C:DE
	POP	AF			; restore number of items in serie
	DEC	A			; more items in serie ?
	RET	Z			; nope, quit
	PUSH	AF			; save number of items in serie
	PUSH	DE
	LD	A,C
	PUSH	AF			; save float in C:DE
	CALL	C7748			; multipy floats
	LD	C,(IX+0)
	LD	D,(IX+1)
	LD	E,(IX+2)
	CALL	C76D7			; add floats
	INC	IX
	INC	IX
	INC	IX			; update pointer to next float in serie
	JR	J7BB0			; next

;     XBASIC_CAST_INTEGER_TO_STRING
;	  Subroutine integer to text
;	     Inputs  HL (integer)
;	     Outputs HL (string pointer)

C7BD1:	LD	IX,FBUFFR+1
	BIT	7,H			; postive integer ?
	LD	A," "
	JR	Z,J7BE6			; yep, use space as first character
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A			; negate
	JP	M,J7BF9			; still negative (was -32768), -32768 string and quit
	LD	A,"-"			; - as first character
J7BE6:	LD	(IX+0),A
	INC	IX
	CALL	C7C0E			; generate string from integer
J7BEE:	PUSH	IX
	POP	HL
	LD	A,L
	SUB	LOW (FBUFFR+1)		; calculate string length
	LD	HL,FBUFFR+0
	LD	(HL),A			; string length
	RET

J7BF9:	LD	DE,FBUFFR+6
	LD	HL,I7C07+6
	LD	BC,7
	LDDR
	INC	DE
	EX	DE,HL
	RET

I7C07:	DEFB	6
	DEFB	"-32768"

;	  Subroutine generate string from integer
;	     Inputs  ________________________
;	     Outputs ________________________

C7C0E:	INC	H
	DEC	H
	JR	NZ,J7C17
	LD	A,L
	CP	9+1
	JR	C,J7C22
J7C17:	LD	DE,10
	CALL	C7643			; / 10
	PUSH	DE
	CALL	C7C0E
	POP	HL
J7C22:	LD	A,L
	ADD	A,"0"
	LD	(IX+0),A
	INC	IX
	RET

;	  Subroutine float to text
;	     Inputs  ________________________
;	     Outputs ________________________

C7C2B:	LD	IX,FBUFFR+1
	CALL	C7C42			; generate string from float
	JP	J7BEE			; set string length and quit

J7C35:	LD	(IX+0)," "
	INC	IX
	LD	(IX+0),"0"
	INC	IX
	RET

;	  Subroutine generate string from float
;	     Inputs  ________________________
;	     Outputs ________________________

C7C42:	LD	A,B
	AND	A			; float 0.0 ?
	JR	Z,J7C35			; yep, string 0 and quit
	BIT	7,H			; positive float ?
	LD	A," "
	JR	Z,J7C50			; yep, use space as first character
	RES	7,H			; negate
	LD	A,"-"			; use - as first character
J7C50:	LD	(IX+0),A
	INC	IX
	XOR	A
J7C56:	PUSH	AF
	LD	C,8AH
	LD	DE,79FCH		; float 1000
	CALL	C7966			; compare floats
	JR	NC,J7C6D		; > 1000,
	LD	C,84H
	LD	DE,2000H		; float 10
	CALL	C7748			; multipy floats
	POP	AF
	DEC	A
	JR	J7C56

J7C6D:	LD	C,8EH
	LD	DE,1C3EH		; float 10000
	CALL	C7966			; compare floats
	JR	C,J7C83			; < 10000,
	LD	C,84H
	LD	DE,2000H		; float 10
	CALL	C782C			; divide floats
	POP	AF
	INC	A
	JR	J7C56

J7C83:	LD	C,80H
	LD	DE,0			; float 0.5
	CALL	C76D7			; add floats
	CALL	C7901			; convert float to integer
	POP	AF
	ADD	A,5
	CP	6
	JR	NC,J7CE3
	DEC	A
	JP	P,C7CA7			; convert integer to digits
	LD	(IX+0),"."
	INC	IX
	LD	(IX+0),"0"
	INC	IX
	LD	A,0FFH			; force remainer check

;	  Subroutine convert integer to digits
;	     Inputs  HL = integer, A = flag
;	     Outputs ________________________

C7CA7:	LD	B,A
	LD	DE,1000
	CALL	C7CC0			; divide and add digit
	RET	NC			; remainer = 0, quit
	LD	DE,100
	CALL	C7CC0			; divide and add digit
	RET	NC			; remainer = 0, quit
	LD	DE,10
	CALL	C7CC0			; divide and add digit
	RET	NC			; remainer = 0, quit
	LD	DE,1

;	  Subroutine divide and add digit
;	     Inputs  HL = integer, DE = divider
;	     Outputs ________________________

C7CC0:	INC	B
	DJNZ	J7CCB			; B<>0, skip
	LD	(IX+0),"."
	INC	IX
	LD	B,0FFH			; force remainer check
J7CCB:	LD	A,"0"-1
J7CCD:	INC	A
	AND	A
	SBC	HL,DE
	JR	NC,J7CCD		; again
	ADD	HL,DE
	LD	(IX+0),A		; add digit
	INC	IX
	DEC	B			; B=1 ?
	JR	Z,J7CDE			; yep,
	SCF
	RET	P			; b7=0, quit with Cx set
J7CDE:	LD	A,L
	OR	H			; integer left = 0 ?
	RET	Z			; yep, quit with Cx reset, Zx set
	SCF				; quit with Cx set
	RET

J7CE3:	PUSH	AF
	LD	A,1			; force 1 digit
	CALL	C7CA7			; convert integer to digits
	LD	(IX+0),"E"
	INC	IX
	POP	AF
	SUB	2
	LD	L,A
	LD	A,"+"
	JP	P,J7CFE
	LD	A,L
	NEG
	LD	L,A
	LD	A,"-"
J7CFE:	LD	(IX+0),A
	INC	IX
	LD	A,L
	LD	B,"0"-1
J7D06:	INC	B
	SUB	10
	JR	NC,J7D06
	ADD	A,'0'+10
	LD	(IX+0),B
	INC	IX
	LD	(IX+0),A
	INC	IX
	RET

;	  Subroutine convert numeric string to float
;	     Inputs  ________________________
;	     Outputs ________________________

C7D18:	LD	B,0
	CALL	C7D34			; skip spaces
	CP	"-"
	JR	NZ,J7D28
	INC	DE
	CALL	C7D18			; convert numeric string to float
	JP	J79F7			; change sign and quit

J7D28:	CP	"&"
	JR	NZ,J7D8D		; nope, convert numeric string
	CALL	C7D3B			; convert &B,&O,&H string
	PUSH	DE
	CALL	C78DF			; convert integer to float
	POP	DE

;	  Subroutine skip spaces
;	     Inputs  ________________________
;	     Outputs ________________________

C7D34:	LD	A,(DE)
	CP	" "
	RET	NZ
	INC	DE
	JR	C7D34

;	  Subroutine convert &B,&O,&H string
;	     Inputs  ________________________
;	     Outputs ________________________

C7D3B:	LD	HL,0
	INC	DE
	CALL	C7E38		; get character and to upper
	INC	DE
	CP	"B"
	JR	Z,J7D7E
	CP	"O"
	JR	Z,J7D6D
	CP	"H"
	DEC	DE
	RET	NZ

; convert &H

	INC	DE
J7D50:	CALL	C7E38		; get character and to upper
	AND	A
	RET	Z
	SUB	"0"
	RET	C
	CP	9+1
	JR	C,J7D64
	CP	11H
	RET	C
	CP	17H
	RET	NC
	SUB	07H
J7D64:	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	A,L
	LD	L,A
	INC	DE
	JR	J7D50

; convert &O

J7D6D:	LD	A,(DE)
	AND	A
	RET	Z
	SUB	"0"
	RET	C
	CP	7+1
	RET	NC
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	A,L
	LD	L,A
	INC	DE
	JR	J7D6D

; convert &B

J7D7E:	LD	A,(DE)
	AND	A
	RET	Z
	SUB	"0"
	RET	C
	CP	1+1
	RET	NC
	ADD	HL,HL
	ADD	A,L
	LD	L,A
	INC	DE
	JR	J7D7E

; convert numeric string

J7D8D:	LD	C,1
	DEC	DE
J7D90:	INC	DE
	CALL	C7D34			; skip spaces
	CP	"0"
	JR	C,J7DC4
	CP	"9"+1
	JR	NC,J7DC4
	PUSH	DE
	PUSH	BC
	PUSH	AF
	LD	C,84H
	LD	DE,2000H		; float 10
	CALL	C7748			; multipy floats
	POP	AF
	SUB	"0"
	JR	Z,J7DBB
	LD	C,88H
J7DAE:	DEC	C
	ADD	A,A
	JP	P,J7DAE
	AND	7FH
	LD	D,A
	LD	E,0
	CALL	C76D7			; add floats
J7DBB:	POP	DE
	LD	C,E
	POP	DE
	DEC	C
	JR	NZ,J7D90
	INC	C
	JR	J7D90

J7DC4:	CP	"."
	JR	NZ,J7DCC
	DEC	C
	JR	Z,J7D90
	RET

J7DCC:	DEC	C
	JR	Z,J7DD0
	INC	C
J7DD0:	CALL	C7E39			; to upper
	CP	"E"			; exponent indicator ?
	JR	Z,J7DDB			; yep,
	CP	"D"			; exponent indicator (double real) ?
	JR	NZ,J7DF5		; nope, skip exponent
J7DDB:	INC	DE
	CALL	C7D34			; skip spaces
	CP	"+"
	JR	Z,J7DEF
	CP	"-"
	JR	NZ,J7DF0
	INC	DE			; to next character
	CALL	C7E1C			; get exponent
	NEG				; negate
	JR	J7DF3

J7DEF:	INC	DE			; to next character
J7DF0:	CALL	C7E1C			; get exponent
J7DF3:	ADD	A,C
	LD	C,A
J7DF5:	LD	A,C
	AND	A			;
	RET	Z
	PUSH	DE
	PUSH	AF
	JP	P,J7DFF
	NEG
J7DFF:	LD	E,A
	LD	D,0
	PUSH	HL
	PUSH	BC
	LD	B,84H
	LD	HL,2000H		; float 10.0
	CALL	C7878			; ^ handler
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	POP	AF
	PUSH	AF
	CALL	P,C7748			; multipy floats
	POP	AF
	CALL	M,C782C			; divide floats
	POP	DE
	RET

;	  Subroutine get exponent
;	     Inputs  ________________________
;	     Outputs ________________________

C7E1C:	PUSH	HL
	LD	L,0
J7E1F:	CALL	C7D34			; skip spaces
	CP	"9"+1
	JR	NC,J7E35
	SUB	"0"
	JR	C,J7E35
	LD	H,A
	LD	A,L
	ADD	A,A
	ADD	A,A
	ADD	A,L
	ADD	A,A			; *10
	ADD	A,H
	LD	L,A
	INC	DE
	JR	J7E1F

J7E35:	LD	A,L
	POP	HL
	RET

;	  Subroutine get character and to upper
;	     Inputs  ________________________
;	     Outputs ________________________

C7E38:	LD	A,(DE)

;	  Subroutine to upper
;	     Inputs  ________________________
;	     Outputs ________________________

C7E39:	CP	"a"
	RET	C
	CP	"z"+1
	RET	NC
	AND	0DFH
	RET

;	  Subroutine left of string
;	     Inputs  ________________________
;	     Outputs ________________________

I7E42:	CP	(HL)
	JR	C,J7E46
	LD	A,(HL)
J7E46:	INC	HL
	JR	J7E70

;	  Subroutine right of string
;	     Inputs  ________________________
;	     Outputs ________________________

I7E49:	CP	(HL)
	JR	C,J7E4D
	LD	A,(HL)
J7E4D:	PUSH	AF
	LD	E,(HL)
	NEG
	ADD	A,E
	INC	A
	LD	E,A
	LD	D,0
	ADD	HL,DE
	POP	AF
	JR	J7E70

;	  Subroutine part of string
;	     Inputs  ________________________
;	     Outputs ________________________

C7E5A:	LD	C,B
	PUSH	AF
	ADD	A,C
	JR	C,J7E63
	DEC	A
	CP	(HL)
	JR	C,J7E6C
J7E63:	POP	AF
	LD	A,(HL)
	SUB	C
	JR	NC,J7E6A
	LD	A,0FFH
J7E6A:	INC	A
	PUSH	AF
J7E6C:	LD	B,0
	ADD	HL,BC
	POP	AF
J7E70:	LD	DE,BUF
	LD	(DE),A
	AND	A
	JR	Z,J7E7D
	INC	DE
	LD	C,A
	LD	B,0
	LDIR
J7E7D:	LD	HL,BUF
	RET

;	  Subroutine replace in string
;	     Inputs  ________________________
;	     Outputs ________________________

C7E81:	EX	DE,HL
	CP	(HL)
	JR	Z,J7E86
	RET	NC
J7E86:	LD	C,A
	LD	A,(DE)
	CP	B
	JR	NC,J7E8C
	LD	B,A
J7E8C:	LD	A,C
	DEC	A
	ADD	A,B
	JR	C,J7E96
	CP	(HL)
	JR	C,J7E9A
	JR	Z,J7E9A
J7E96:	LD	A,(HL)
	SUB	C
	INC	A
	LD	B,A
J7E9A:	LD	A,L
	ADD	A,C
	LD	L,A
	JR	NZ,J7EA0
	INC	H
J7EA0:	EX	DE,HL
	INC	HL
	LD	C,B
	LD	B,0
	LDIR
	RET

;	  Subroutine CHR$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EA8:	LD	HL,BUF+1
	LD	(HL),A			; character
	DEC	HL
	LD	(HL),1			; string length = 1
	RET

;	  Subroutine VAL handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7EB0:	LD	DE,BUF+1
	PUSH	DE
	LD	A,(HL)
	AND	A			; string length
	JR	Z,J7EBE			; empty string,
	LD	C,A
	XOR	A
	LD	B,A			; string length
	INC	HL
	LDIR				; copy string to BUF+1
J7EBE:	LD	(DE),A			; string end marker
	POP	DE
	JP	C7D18			; convert numeric string to float

;	  Subroutine HEX$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EC3:	LD	C,4
	DEFB	011H

;	  Subroutine OCT$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EC6:	LD	C,3
	DEFB	011H

;	  Subroutine BIN$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EC9:	LD	C,1
	LD	DE,FBUFFR+16
J7ECE:	LD	B,C
	XOR	A
J7ED0:	SRL	H
	RR	L
	RRA
	DJNZ	J7ED0
	LD	B,C
J7ED8:	RLCA
	DJNZ	J7ED8
	CP	9+1
	JR	C,J7EE1
	ADD	A,7
J7EE1:	ADD	A,"0"
	LD	(DE),A
	DEC	DE
	LD	A,L
	OR	H
	JR	NZ,J7ECE
	LD	A,0D5H
	SUB	E
	LD	(DE),A
	EX	DE,HL
	RET

;	  Subroutine SPACE$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EEF:	LD	B,A
	LD	A," "

;	  Subroutine STRING$ handler
;	     Inputs  B = number of characters, A = character
;	     Outputs ________________________

C7EF2:	LD	HL,BUF
	LD	(HL),B			; string length
	INC	B
	DEC	B			; length = 0 ?
	RET	Z			; yep, quit
	PUSH	HL
	INC	HL
J7EFB:	LD	(HL),A			; character
	INC	HL
	DJNZ	J7EFB
	POP	HL
	RET

;	  Subroutine INKEY$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7F01:	CALL	CHSNS			; key in buffer ?
	LD	HL,BUF
	LD	A,0
	JR	Z,J7F13			; nope, quit with empty string
	CALL	CHGET			; get key from buffer
	INC	HL
	LD	(HL),A			; character (key)
	DEC	HL
	LD	A,1			; string length = 1
J7F13:	LD	(HL),A
	RET

;	  Subroutine INSTR handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7F15:	LD	C,A
	LD	A,(DE)
	SUB	C
	SUB	(HL)
	ADD	A,2
	LD	B,A
	LD	A,C
	DEC	A
	ADD	A,E
	LD	E,A
	JR	NC,J7F23
	INC	D
J7F23:	PUSH	BC
	PUSH	HL
	LD	B,(HL)
	INC	HL
	INC	DE
	PUSH	DE
J7F29:	LD	A,(DE)
	CP	(HL)
	JR	NZ,J7F37
	INC	DE
	INC	HL
	DJNZ	J7F29
	POP	AF
	POP	AF
	POP	AF
	LD	L,C
	LD	H,B
	RET

J7F37:	POP	DE
	POP	HL
	POP	BC
	INC	C
	DJNZ	J7F23
	LD	L,B
	LD	H,B
	RET

;	  Subroutine copy string to NULBUF
;	     Inputs  HL = pointer to string
;	     Outputs ________________________

C7F40:	LD	DE,(NULBUF)
	LD	C,(HL)			; size of string
	LD	B,0
	INC	BC			; include size byte
	LDIR
	RET

;	  Subroutine string1 > string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F4B:	CALL	C7F7B			; compare strings
	LD	HL,0
	RET	C			; smaller, return false
	RET	Z			; equal, return false
	DEC	HL			; bigger, return true
	RET

;	  Subroutine string1 = string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F55:	CALL	C7F74			; compare strings, assume true
	RET	Z			; equal, return true
	INC	HL			; not equal, return false
	RET

;	  Subroutine string1 >= string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F5B:	CALL	C7F74			; compare strings, assume true
	RET	NC			; bigger or equal, return true
	INC	HL			; smaller, return false
	RET

;	  Subroutine string1 < string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F61:	CALL	C7F74			; compare strings, assume true
	RET	C			; smaller, return true
	INC	HL			; bigger or equal, return false
	RET

;	  Subroutine string1 <> string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F67:	CALL	C7F74			; compare strings, assume true
	RET	NZ			; not equal, return true
	INC	HL			; equal, return false
	RET

;	  Subroutine string1 <= string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F6D:	CALL	C7F74			; compare strings, assume true
	RET	C			; smaller, return true
	RET	Z			; equal, return true
	INC	HL			; bigger, return false
	RET

;	  Subroutine compare strings, assume true
;	     Inputs  ________________________
;	     Outputs ________________________

C7F74:	CALL	C7F7B			; compare strings

;	  Subroutine string1 <=> string2
;	     Inputs  ________________________
;	     Outputs ________________________

C7F77:	LD	HL,-1			; return true
	RET

;	  Subroutine compare strings
;	     Inputs  NULBUF = first string, HL = second string
;	     Outputs ________________________

C7F7B:	LD	B,(HL)			; size second string
	INC	HL
	LD	DE,(NULBUF)
	LD	A,(DE)
	LD	C,A			; size first string
	INC	DE
J7F84:	LD	A,C
	OR	B			; at the end of both strings ?
	RET	Z			; yep, quit with Zx set
	LD	A,C
	SUB	1			; at the end of first string ?
	RET	C			; yep, quit with Cx set, Zx reset
	LD	A,B
	SUB	1			; at the end of second string
	CCF
	RET	NC			; yep, quit with Cx reset, Zx reset
	DEC	B
	DEC	C
	LD	A,(DE)
	CP	(HL)			; compare character
	INC	DE
	INC	HL
	JR	Z,J7F84			; equal, next character
	RET

;	  Subroutine concat strings
;	     Inputs  NULBUF = string1, HL = string2
;	     Outputs BUF = result string

C7F99:	LD	DE,BUF
	LD	BC,(NULBUF)
	AND	A
	PUSH	HL
	SBC	HL,DE			; string2 in BUF ?
	POP	HL
	JR	Z,J7FD2			; yep,
	LD	A,(BC)			; size of string1
	ADD	A,(HL)			; size of string2
	JR	NC,J7FAD
	LD	A,255			; concat string to 255 characters
J7FAD:	LD	(DE),A			; result string size
	INC	DE
	PUSH	HL
	LD	A,(BC)			; size of string1
	PUSH	AF
	AND	A			; empty string1 ?
	JR	Z,J7FBD			; yep, skip copy string1
	INC	BC
	LD	L,C
	LD	H,B
	LD	C,A
	LD	B,0
	LDIR				; copy string1
J7FBD:	POP	AF			; size of string1
	POP	HL			; string2
	LD	B,A
	LD	A,(HL)			; size string2
	JR	NC,J7FC5		; string2 not limited
	LD	A,B
J7FC4:	CPL
J7FC5:	AND	A			; empty string2 ?
	JR	Z,J7FCE			; yep, skip copy string2
	INC	HL
	LD	C,A
	LD	B,0
	LDIR				; copy string2
J7FCE:	LD	HL,BUF			; string in BUF
	RET

J7FD2:	PUSH	BC
	PUSH	HL
	LD	E,C
	LD	D,B
	LD	A,(DE)
	CP	(HL)
	JR	NC,J7FDB
	LD	A,(HL)
J7FDB:	LD	B,A
	INC	B
J7FDD:	LD	C,(HL)
	LD	A,(DE)
	LD	(HL),A
	LD	A,C
	LD	(DE),A
	INC	DE
	INC	HL
	DJNZ	J7FDD
	POP	BC
	LD	A,(BC)
	LD	L,A
	LD	H,0
	INC	HL
	ADD	HL,BC
	EX	DE,HL
	POP	HL
	ADD	A,(HL)
	JR	C,J7FF6
	LD	(BC),A
	LD	A,(HL)
	JR	J7FC5

J7FF6:	LD	A,(BC)
	PUSH	AF
	LD	A,0FFH
	LD	(BC),A
	POP	AF
	JR	J7FC4

BASIC_KUN_END_FILLER:
	DEFS	08000H-$,0FFH


