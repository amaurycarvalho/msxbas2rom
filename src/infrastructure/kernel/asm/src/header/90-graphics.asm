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

; Desativado: get sprite definition handler (C716E, J717D, J7183: 3 labels)
;C716E:
;	DEFS	27,0x00

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

