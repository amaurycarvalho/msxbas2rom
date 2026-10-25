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

; Desativado: rotinas de INPUT (I7517-C75BE + J761C: 22 labels de prompt numérico/string, busca de DATA, "?Redo from start")
;I7517:
;	DEFS	269,0x00

;	  Subroutine multiply
;	     Inputs  ________________________
;	     Outputs ________________________

