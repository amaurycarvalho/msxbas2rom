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

; Desativado: remainder de divisão inteira (C76CE: 1 label)
;C76CE:
;	DEFS	5,0x00

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

; Desativado: conversão unsigned int para float (C78CD: 1 label)
;C78CD:
;	DEFS	18,0x00

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

; Desativado: float <=> trivial (C7962: 1 label)
;C7962:
;	DEFS	4,0x00

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

