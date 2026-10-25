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
C6C26:	RET

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

