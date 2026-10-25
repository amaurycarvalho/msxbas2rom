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
C7F44:	LD	C,(HL)			; size of string
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
C7FA0:
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

