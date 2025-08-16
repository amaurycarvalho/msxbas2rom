/*

   Compiler Class - ported from TurboBasic (xBasic) Z80 assembly
   
*/

#include "xbasic.h"

XBasic::XBasic() {
	
	hl = 0;
	bc = 0;
	de = 0;
	af = 0;
	ix = 0;
	iy = 0;
	sp = 0;
	
	l = (unsigned char *) &hl;
	h = l++;
	c = (unsigned char *) &bc;
	b = c++;
	e = (unsigned char *) &de;
	d = e++;
	f = (unsigned char *) &af;
	a = f++;	
	
	memset(ram, 0, 0xFFFF);
	
	RAMAD1 = 0xf342;
	FORCLR = 0xf3e9;
    BAKCLR = 0xf3ea;
    BDRCLR = 0xf3eb;
    ATRBYT = 0xf3f2;
    CURLIN = 0xf41c;
    BUF    = 0xf55e;
    FNPTR  = 0xf562;
    DUMMY  = 0xf564;
    VAR_DY = 0xf568;
	NX     = 0xf56a;
	NY     = 0xf56c;
	ARGT   = 0xf56f;
	VALTYP = 0xf663;
	TXTTAB = 0xf676;
	DATLIN = 0xf6a3;
	SUBFLG = 0xf6a5;
	SAVSTK = 0xf6b1;
	STREND = 0xf6c6;
	DATPTR = 0xf6c8;
	DEFTBL = 0xf6ca;
	SWPTMP = 0xf7bc;
	SWPTMP1 = 0xf7bd;
	STRBUF  = 0xf7c5;
	DAC     = 0xf7f6;
	MODE    = 0xfafc;
	ONGSBF  = 0xfbd8;
	TRPTBL  = 0xfc4c;
	TRPTBL_STOP     = 0xfc6a;
	TRPTBL_SPRITE   = 0xfc6d;
	TRPTBL_STRIG    = 0xfc70;
	TRPTBL_INTERVAL = 0xfc7f;
	INTFLG  = 0xfc9b;
	JIFFY   = 0xfc9e;
	SCRMOD  = 0xfcaf;
	GRPACX  = 0xfcb7;
	GRPACY  = 0xfcb9;
	EXPTBL  = 0xfcc1;
	PROCNM  = 0xfd89;
	VARTMP1 = 0xf6af;
	VARTMP2 = 0xf565;
	VARTMP3 = 0xf563;
	VARTMP4 = 0xf571;
	
}

void XBasic::setword(int addr, int value) {
	ram[addr] = value & 0xFF;
	ram[addr+1] = (value >> 8) & 0xFF;
}

int XBasic::getword(int addr) {
	return ram[addr] | ram[addr+1]<<8;
}

void XBasic::call_run(int address) {

    hl = address;
	
	setword(VARTMP1, hl);
	
	memset(&ram[DEFTBL], 8, 0x1a);
		
	hl = getword(TXTTAB);
	setword(DATPTR, hl - 1);
	*a = 0;
	
	call_run_parse();
}

void XBasic::call_run_parse() {
	
	setword(VARTMP2, hl);
	ram[VARTMP3] = *a;
	setword(SAVSTK, sp);
	
	if(*a) {
		  *b = *a;
		  *a = 0;
		  while(ram[hl] && bc) { hl++; bc--; }
	}
	
	bc = 0;
	
	do {
		bc++;
		*a = ram[hl];
		hl++;
		ix = hl;
		*h = ram[hl];
		*l = *a;
		if(hl == 0) { bc --; break; }
		ix += 3;
		GET_NEXT_TOKEN();
		if(*a == 0xca) break;   // token 0xCA = CALL
	} while(*a <> 0x5f);        // token 0x5F = _
	
	bc++;
	hl = bc*6;
	setword(VARTMP4, hl);
	de = hl;
	hl = sp - de;
	setword(ARGT, hl);
	sp = hl;
	setword(hl, 0xffff);
	*h -= 2;
	hl -= 4;
	ram[hl] = 0;
    setword(0xf573, hl);
    setword(0xf575, hl)
    *a = 0;
    ram[VAR_FNPTR] = *a;
    ram[VAR_DUMMY] = *a;
    ram[VAR_NX] = *a;
    ram[0xf56b] = *a;
    ram[VAR_NY] = *a;
    hl = getword(BASIC_STREND);
    setword(BASIC_BUF, hl);
    setword(0xf560, hl);
    ix = getword(0xf565);
    *a = ram[0xf563];
	
    // l41d9
	if(*a) {
		*a = ram[ix];
		if(*a == 0x3a) {   // token 0x3A = :
            proc_l420f();
			proc_l41f1();

		} else {
			ix ++;
			if(*a == 0) {
				proc_l41f1();
			} else if(*a == 0x28) {   // token 0x28 = (
				if(PARSE_LINE()) {          // carry = 1
					proc_l420f();
					proc_l41f1();
				}
			} else {
				// jp nz,04e16h
				return sub_04e16();    // error
			}
		}
    } else {
		proc_l41f1();

	}
	// l421ah
	*a = ram[0xf563];
	if(*a) {
		hl = getword(0xf56d);
		*a = ram[hl];
		hl++;
		*h = ram[hl]
		*l = *a;
		hl --;
		setword(0xf6af, hl);
	}
	//EXEC_COMPILED_CODE_INIT
	TOKEN_81_END();
	sub_4387();
	sub_648a();
	hl = getword(0xf571);
	hl += sp;
	sp = hl;
	hl = 0xfffe;                       // ajuste da numeração de linha do BASIC
	setword(BASIC_CURLIN, hl);
	hl = 0x3579;
	setword(BASIC_SWPTMP, hl);
	hl = 0x7531;
	setword(0xF7BE, hl);
	hl = MODE;
    ram[hl] &= 0xF7;
	hl = 0x4e28;                       // possivelmente endereço de retorno do XBasic após execução do codigo compilado
	stk.push(hl);
	hl = getword(BASIC_BUF);
	// EXEC_JUMP_TO_COMPILED_CODE
	// jp (hl)                         // chamada do codigo compilado (vai retornar para 0x4e28 ao final)
}

void XBasic::proc_l41f1() {
	while( (*a = ram[ix] | ram[ix+1]) ) {
		setword( 0xf56d, ix );
		hl = getword(ix+2);
		setword( BASIC_CURLIN, hl );
		sub_42d9();
		ix += 3;
		proc_l420f()
	}
}

// avalia os tokens da linha
void XBasic::proc_l420f() {
	do {
		ix ++;
	} while( sub_4258() );  // carry = 1
	ix ++;
}


// avalia o token atual
int XBasic::sub_4258() {
	GET_NEXT_TOKEN();
	if(!*a) return 0;
	if(*a == 0x3a) return 1;     // token 0x3A = :
	stk.push(af);
	*a = ram[VAR_DUMMY];
	if(*a) {
		char data[] = { 0xcd, 0x25, 0x6c };         //call sub_6c25h		;426b	cd 25 6c
		WRITE_NEXT_BYTES_UNTIL_2_NOPS(data, 3);		// já grava no binario o trecho de codigo para esse token
	}
	// l4270h
	af = stk.pop();
	ix ++;
	if(*a == 0x5f) {   // token 0x5F = _
		sub_4e36()
	} else {
		// l427ch
		if((*a & 128) == 0) {
			sub_4464();
		} else {
			// l4285h
			if(*a == 0xff) {
				*a = ram[ix];
				ix++;
				hl = 0x42cc;
				stk.push(hl);
				if(*a == 0x83) {          // token 0x83 = NEXT
					return proc_l61b4();
				} else if(*a == 0xa3) {   // token 0xA3 = TROFF
					return proc_l4d2f();
				} else if(*a == 0x85) {   // token 0x85 = INPUT
					*a = 0x45;
					ASSERT_NEXT_TOKEN_BE();
					*a = 0x52;
					ASSERT_NEXT_TOKEN_BE();
					*a = 0xff;
					ASSERT_NEXT_TOKEN_BE();
					*a = 0x94;
					ASSERT_NEXT_TOKEN_BE();
					return CMD_ON_INTERVAL();
				} else {
					// jp nz,04e16h
					return sub_04e16();  // error
				}
			} else {
				// l42b8h
				if(*a >= 0xd9) {    // token 0xD9 = TO
					// jp nz,04e16h
					return sub_04e16();  // error
				} else {
					*a += *a;
					*l = *a;
					*h = 0;
					bc = 0x4097;   // tabela de ponteiros para os trechos de parse por token
					hl += bc;
					*a = ram[hl];
					hl++;
					*h = ram[hl];
					*l = *a;
					sub_42d8();  // chama o trecho de programa que está apontado por HL 
				}
			}
		}
	}
	// l42cch
	GET_NEXT_TOKEN();
	if(!*a) return 0;
	if(*a == 0x3a) return 1;      // token 0x3A = :
	return sub_04e16();  // error
}

// chama o trecho que está em HL
void XBasic::sub_42d8() {
	//jp (hl)
}

int XBasic::sub_42d9() {
	stk.push(ix);
	ix = getword(VAR_ARGT);
	while(1) {
		*a = ram[ix] & ram[ix+1];
		*a ++;
		if(*a == 0) {
			//l42fb
			setword(ix, hl);
			*a = 0;
			setword(ix+4, *a);
			setword(ix+6, 0xff);
			break;
		}
		*a = *l;
		if(*a == ram[ix]) {
			*a = *h;
			if(*a == ram[ix+1]) break;
		}
		bc = 6;
		ix += bc;
	}
	//l4310
	hl = getword(0xf560);
	setword(ix+2, hl);
	ix = stk.pop();
	return *a;
}

int XBasic::sub_431c() {    // semelhante ao sub_42d9
	stk.push(ix);
	ix = getword(VAR_ARGT);
	while(1) {
		*a = ram[ix] & ram[ix+1];
		*a ++;
		if(*a == 0) {
			//l433e
			setword(ix, hl);
			*a = 0;
			setword(ix+2, *a);
			hl = getword(0xf560);
			setword(ix+4, hl);
			setword(ix+6, 0xff);
			hl = 0;
			break;
		}
		*a = *l;
		if(*a == ram[ix]) {
			*a = *h;
			if(*a == ram[ix+1]) {
				//l4360
				*a = ram[ix+2] | ram[ix+3];
				if(*a == 0) {
					hl = getword(ix+4);
					stk.push(hl);
					hl = getword(0xf560);
					setword(ix+4, hl);
					hl = stk.pop();
				} else {
					hl = getword(ix+2);
				}
				break;
			}
		}
		bc = 6;
		ix += bc;
	}
	
	//l4381h
	sub_60ach();
	ix = stk.pop();	
	return *a;
}

int XBasic::sub_4387() {
	ix = getword(VAR_ARGT);
	while(1) {
		*a = ram[ix] & ram[ix+1];
		*a ++;
		if(*a == 0) break;
		de = getword(ix+2);
		*a = *e | *d;
		if(*a == 0) return 0;  // error
		hl = getword(ix+4);
		*a = *l | *h;
		while(*a) {
			*c = ram[hl];
			ram[hl] = *e;
			hl++;
			*b = ram[hl];
			ram[hl] = *d;
			hl = bc;
			*a = *l | *h;			
		}
		bc = 6;
		ix += bc;
	}
	return 1;
}

/*

	
TOKEN_8F_REM:
	ld a,(ix+000h)		;43ba	dd 7e 00 	. ~ . 
	cp 0e6h		;43bd	fe e6 	. . 
	jr nz,l43c3h		;43bf	20 02 	  . 
	inc ix		;43c1	dd 23 	. # 
l43c3h:
	call GET_NEXT_TOKEN		;43c3	cd 30 60 	. 0 ` 
	cp 023h		;43c6	fe 23 	. # 
	jp nz,l4453h		;43c8	c2 53 44 	. S D 
	ld a,(ix+001h)		;43cb	dd 7e 01 	. ~ . 
	call sub_7d90h		;43ce	cd 90 7d 	. . } 
	cp 049h		;43d1	fe 49 	. I 
	jr nz,l441fh		;43d3	20 4a 	  J 
	inc ix		;43d5	dd 23 	. # 
l43d7h:
	inc ix		;43d7	dd 23 	. # 
	call GET_NEXT_TOKEN		;43d9	cd 30 60 	. 0 ` 
	push ix		;43dc	dd e5 	. . 
	pop de			;43de	d1 	. 
	call sub_7d90h		;43df	cd 90 7d 	. . } 
	cp 040h		;43e2	fe 40 	. @ 
	jr z,l43f8h		;43e4	28 12 	( . 
	cp 041h		;43e6	fe 41 	. A 
	jr c,l4405h		;43e8	38 1b 	8 . 
	cp 05bh		;43ea	fe 5b 	. [ 
	jr nc,l4405h		;43ec	30 17 	0 . 
	call GET_VAR_PARAMETER		;43ee	cd 01 65 	. . e 
	ld c,000h		;43f1	0e 00 	. . 
	call sub_62e5h		;43f3	cd e5 62 	. . b 
	jr l4412h		;43f6	18 1a 	. . 
l43f8h:
	inc de			;43f8	13 	. 
	call sub_7c6dh		;43f9	cd 6d 7c 	. m | 
	push de			;43fc	d5 	. 
	call sub_784fh		;43fd	cd 4f 78 	. O x 
	call sub_431ch		;4400	cd 1c 43 	. . C 
	jr l4410h		;4403	18 0b 	. . 
l4405h:
	call sub_7c6dh		;4405	cd 6d 7c 	. m | 
	push de			;4408	d5 	. 
	call sub_784fh		;4409	cd 4f 78 	. O x 
	ld a,l			;440c	7d 	} 
	call WRITE_OPCODE		;440d	cd b7 60 	. . ` 
l4410h:
	pop ix		;4410	dd e1 	. . 
l4412h:
	call GET_NEXT_TOKEN		;4412	cd 30 60 	. 0 ` 
	cp 02ch		;4415	fe 2c 	. , 
	jr z,l43d7h		;4417	28 be 	( . 
	and a			;4419	a7 	. 
	jp nz,04e16h		;441a	c2 16 4e 	. . N 
	jr l4453h		;441d	18 34 	. 4 
l441fh:
	cp 043h		;441f	fe 43 	. C 
	jr nz,l4441h		;4421	20 1e 	  . 
	ld a,(ix+002h)		;4423	dd 7e 02 	. ~ . 
	cp 02bh		;4426	fe 2b 	. + 
	ld hl,0a6cbh		;4428	21 cb a6 	! . . 
	jr z,l4434h		;442b	28 07 	( . 
	cp 02dh		;442d	fe 2d 	. - 
	jr nz,l4453h		;442f	20 22 	  " 
	ld hl,0e6cbh		;4431	21 cb e6 	! . . 
l4434h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4434	cd 56 60 	. V ` 
	ld hl,MODE		;4437	21 fc fa 	! . . 
	nop			;443a	00 	. 
	nop			;443b	00 	. 
	call sub_60ach		;443c	cd ac 60 	. . ` 
	jr l4453h		;443f	18 12 	. . 
l4441h:
	cp 04eh		;4441	fe 4e 	. N 
	jr nz,l4453h		;4443	20 0e 	  . 
	ld a,(ix+002h)		;4445	dd 7e 02 	. ~ . 
	cp 02bh		;4448	fe 2b 	. + 
	jr z,l4450h		;444a	28 04 	( . 
	sub 02dh		;444c	d6 2d 	. - 
	jr nz,l4453h		;444e	20 03 	  . 
l4450h:
	ld (VAR_NY),a		;4450	32 6c f5 	2 l . 
l4453h:
	ld hl,(0f56dh)		;4453	2a 6d f5 	* m . 
	ld a,(hl)			;4456	7e 	~ 
	inc hl			;4457	23 	# 
	ld h,(hl)			;4458	66 	f 
	ld l,a			;4459	6f 	o 
	dec hl			;445a	2b 	+ 
	push hl			;445b	e5 	. 
	pop ix		;445c	dd e1 	. . 
	ret			;445e	c9 	. 
TOKEN_88_LET:
	call GET_NEXT_TOKEN		;445f	cd 30 60 	. 0 ` 
	inc ix		;4462	dd 23 	. # 
sub_4464h:
	dec ix		;4464	dd 2b 	. + 
	call sub_5bf1h		;4466	cd f1 5b 	. . [ 
	push af			;4469	f5 	. 
	call ASSERT_NEXT_IS_EQUAL		;446a	cd 3a 60 	. : ` 
	pop af			;446d	f1 	. 
	inc b			;446e	04 	. 
	bit 5,a		;446f	cb 6f 	. o 
	jp nz,l44dfh		;4471	c2 df 44 	. . D 
	bit 6,a		;4474	cb 77 	. w 
	jr nz,l449fh		;4476	20 27 	  ' 
	djnz l4481h		;4478	10 07 	. . 
	push hl			;447a	e5 	. 
	call sub_4f97h		;447b	cd 97 4f 	. . O 
	pop hl			;447e	e1 	. 
	jr l44b2h		;447f	18 31 	. 1 
l4481h:
	djnz l448bh		;4481	10 08 	. . 
	push hl			;4483	e5 	. 
	call sub_4f97h		;4484	cd 97 4f 	. . O 
	pop hl			;4487	e1 	. 
	jp l6089h		;4488	c3 89 60 	. . ` 
l448bh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;448b	cd 56 60 	. V ` 
	push hl			;448e	e5 	. 
	nop			;448f	00 	. 
	nop			;4490	00 	. 
	call sub_4f97h		;4491	cd 97 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4494	cd 56 60 	. V ` 
	pop de			;4497	d1 	. 
	ex de,hl			;4498	eb 	. 
	ld (hl),e			;4499	73 	s 
	inc hl			;449a	23 	# 
	ld (hl),d			;449b	72 	r 
	nop			;449c	00 	. 
	nop			;449d	00 	. 
	ret			;449e	c9 	. 
l449fh:
	djnz l44b7h		;449f	10 16 	. . 
	push hl			;44a1	e5 	. 
	call sub_4faeh		;44a2	cd ae 4f 	. . O 
	pop hl			;44a5	e1 	. 
l44a6h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;44a6	cd 56 60 	. V ` 
	ld a,b			;44a9	78 	x 
	nop			;44aa	00 	. 
	nop			;44ab	00 	. 
	ld a,032h		;44ac	3e 32 	> 2 
	call WRITE_OPCODE_A_L_H		;44ae	cd a9 60 	. . ` 
	inc hl			;44b1	23 	# 
l44b2h:
	ld a,022h		;44b2	3e 22 	> " 
	jp WRITE_OPCODE_A_L_H		;44b4	c3 a9 60 	. . ` 
l44b7h:
	djnz l44c9h		;44b7	10 10 	. . 
	push hl			;44b9	e5 	. 
	call sub_4faeh		;44ba	cd ae 4f 	. . O 
	pop hl			;44bd	e1 	. 
sub_44beh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;44be	cd 56 60 	. V ` 
	ld a,b			;44c1	78 	x 
	nop			;44c2	00 	. 
	nop			;44c3	00 	. 
	ld c,002h		;44c4	0e 02 	. . 
	jp sub_62e5h		;44c6	c3 e5 62 	. . b 
l44c9h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;44c9	cd 56 60 	. V ` 
	push hl			;44cc	e5 	. 
	nop			;44cd	00 	. 
	nop			;44ce	00 	. 
	call sub_4faeh		;44cf	cd ae 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;44d2	cd 56 60 	. V ` 
	pop de			;44d5	d1 	. 
	ex de,hl			;44d6	eb 	. 
	ld (hl),b			;44d7	70 	p 
	inc hl			;44d8	23 	# 
	ld (hl),e			;44d9	73 	s 
	inc hl			;44da	23 	# 
	ld (hl),d			;44db	72 	r 
	nop			;44dc	00 	. 
	nop			;44dd	00 	. 
	ret			;44de	c9 	. 
l44dfh:
	djnz l44edh		;44df	10 0c 	. . 
	push hl			;44e1	e5 	. 
	call sub_4ffeh		;44e2	cd fe 4f 	. . O 
	pop hl			;44e5	e1 	. 
l44e6h:
	ld a,011h		;44e6	3e 11 	> . 
	call WRITE_OPCODE_A_L_H		;44e8	cd a9 60 	. . ` 
	jr l450dh		;44eb	18 20 	.   
l44edh:
	djnz l44feh		;44ed	10 0f 	. . 
	push hl			;44ef	e5 	. 
	call sub_4ffeh		;44f0	cd fe 4f 	. . O 
	pop hl			;44f3	e1 	. 
l44f4h:
	ld a,011h		;44f4	3e 11 	> . 
	call WRITE_OPCODE		;44f6	cd b7 60 	. . ` 
	call sub_62e5h		;44f9	cd e5 62 	. . b 
	jr l450dh		;44fc	18 0f 	. . 
l44feh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;44fe	cd 56 60 	. V ` 
	push hl			;4501	e5 	. 
	nop			;4502	00 	. 
	nop			;4503	00 	. 
	call sub_4ffeh		;4504	cd fe 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4507	cd 56 60 	. V ` 
	pop de			;450a	d1 	. 
	nop			;450b	00 	. 
	nop			;450c	00 	. 
l450dh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;450d	cd 56 60 	. V ` 
	ld c,(hl)			;4510	4e 	N 
	ld b,000h		;4511	06 00 	. . 
	inc bc			;4513	03 	. 
	ldir		;4514	ed b0 	. . 
	nop			;4516	00 	. 
	nop			;4517	00 	. 
	ret			;4518	c9 	. 
TOKEN_87_READ:
	ld hl,l7598h		;4519	21 98 75 	! . u 
	ld de,l7557h		;451c	11 57 75 	. W u 
	call sub_4a88h		;451f	cd 88 4a 	. . J 
	call GET_NEXT_TOKEN		;4522	cd 30 60 	. 0 ` 
	cp 02ch		;4525	fe 2c 	. , 
	ret nz			;4527	c0 	. 
	inc ix		;4528	dd 23 	. # 
	jr TOKEN_87_READ		;452a	18 ed 	. . 
TOKEN_86_DIM:
	ld a,(VAR_FNPTR)		;452c	3a 62 f5 	: b . 
	and a			;452f	a7 	. 
	jp nz,04e22h		;4530	c2 22 4e 	. " N 
	call GET_VAR_PARAMETER		;4533	cd 01 65 	. . e 
	ld (0f577h),hl		;4536	22 77 f5 	" w . 
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;4539	cd 42 60 	. B ` 
	ld d,000h		;453c	16 00 	. . 
l453eh:
	push de			;453e	d5 	. 
	call sub_4fa9h		;453f	cd a9 4f 	. . O 
	pop de			;4542	d1 	. 
	ld a,b			;4543	78 	x 
	and a			;4544	a7 	. 
	jp nz,04e0dh		;4545	c2 0d 4e 	. . N 
	inc hl			;4548	23 	# 
	ld a,l			;4549	7d 	} 
	or h			;454a	b4 	. 
	jp z,04e1fh		;454b	ca 1f 4e 	. . N 
	push hl			;454e	e5 	. 
	inc d			;454f	14 	. 
	call GET_NEXT_TOKEN		;4550	cd 30 60 	. 0 ` 
	inc ix		;4553	dd 23 	. # 
	cp 029h		;4555	fe 29 	. ) 
	jr z,l4560h		;4557	28 07 	( . 
	cp 02ch		;4559	fe 2c 	. , 
	jr z,l453eh		;455b	28 e1 	( . 
	jp 04e16h		;455d	c3 16 4e 	. . N 
l4560h:
	push de			;4560	d5 	. 
	ld hl,(0f577h)		;4561	2a 77 f5 	* w . 
	ld de,(BASIC_BUF)		;4564	ed 5b 5e f5 	. [ ^ . 
	push de			;4568	d5 	. 
	call sub_62f9h		;4569	cd f9 62 	. . b 
	pop hl			;456c	e1 	. 
	pop bc			;456d	c1 	. 
	ld (hl),b			;456e	70 	p 
l456fh:
	pop de			;456f	d1 	. 
	inc hl			;4570	23 	# 
	ld (hl),e			;4571	73 	s 
	inc hl			;4572	23 	# 
	ld (hl),d			;4573	72 	r 
	djnz l456fh		;4574	10 f9 	. . 
	ld hl,(BASIC_BUF)		;4576	2a 5e f5 	* ^ . 
	ld b,(hl)			;4579	46 	F 
	inc hl			;457a	23 	# 
	ld e,(hl)			;457b	5e 	^ 
	inc hl			;457c	23 	# 
	ld d,(hl)			;457d	56 	V 
	inc hl			;457e	23 	# 
	dec b			;457f	05 	. 
	jr z,l4591h		;4580	28 0f 	( . 
l4582h:
	ld a,(hl)			;4582	7e 	~ 
	inc hl			;4583	23 	# 
	push hl			;4584	e5 	. 
	push bc			;4585	c5 	. 
	ld h,(hl)			;4586	66 	f 
	ld l,a			;4587	6f 	o 
	call sub_761bh		;4588	cd 1b 76 	. . v 
	ex de,hl			;458b	eb 	. 
	pop bc			;458c	c1 	. 
	pop hl			;458d	e1 	. 
	inc hl			;458e	23 	# 
	djnz l4582h		;458f	10 f1 	. . 
l4591h:
	ex de,hl			;4591	eb 	. 
	ld a,(0f578h)		;4592	3a 78 f5 	: x . 
	bit 5,a		;4595	cb 6f 	. o 
	jr z,l45a3h		;4597	28 0a 	( . 
	ld a,h			;4599	7c 	| 
	and a			;459a	a7 	. 
	jp nz,04e1fh		;459b	c2 1f 4e 	. . N 
	ld h,l			;459e	65 	e 
	ld l,000h		;459f	2e 00 	. . 
	jr l45b1h		;45a1	18 0e 	. . 
l45a3h:
	ld c,l			;45a3	4d 	M 
	ld b,h			;45a4	44 	D 
	add hl,hl			;45a5	29 	) 
	jp c,04e1fh		;45a6	da 1f 4e 	. . N 
	bit 6,a		;45a9	cb 77 	. w 
	jr z,l45b1h		;45ab	28 04 	( . 
	add hl,bc			;45ad	09 	. 
	jp c,04e1fh		;45ae	da 1f 4e 	. . N 
l45b1h:
	ld c,l			;45b1	4d 	M 
	ld b,h			;45b2	44 	D 
	add hl,de			;45b3	19 	. 
	jp c,04e1fh		;45b4	da 1f 4e 	. . N 
	push hl			;45b7	e5 	. 
	push de			;45b8	d5 	. 
	ld de,(0f575h)		;45b9	ed 5b 75 f5 	. [ u . 
	sbc hl,de		;45bd	ed 52 	. R 
	jp nc,04e1fh		;45bf	d2 1f 4e 	. . N 
	pop de			;45c2	d1 	. 
	pop hl			;45c3	e1 	. 
	ld (BASIC_BUF),hl		;45c4	22 5e f5 	" ^ . 
	ld (0f560h),hl		;45c7	22 60 f5 	" ` . 
l45cah:
	xor a			;45ca	af 	. 
	ld (de),a			;45cb	12 	. 
	inc de			;45cc	13 	. 
	dec bc			;45cd	0b 	. 
	ld a,c			;45ce	79 	y 
	or b			;45cf	b0 	. 
	jr nz,l45cah		;45d0	20 f8 	  . 
	call GET_NEXT_TOKEN		;45d2	cd 30 60 	. 0 ` 
	cp 02ch		;45d5	fe 2c 	. , 
	ret nz			;45d7	c0 	. 
	inc ix		;45d8	dd 23 	. # 
	jp TOKEN_86_DIM		;45da	c3 2c 45 	. , E 
TOKEN_8B_IF:
	call sub_4fdah		;45dd	cd da 4f 	. . O 
	call GET_NEXT_TOKEN		;45e0	cd 30 60 	. 0 ` 
	cp 089h		;45e3	fe 89 	. . 
	jr z,l45ech		;45e5	28 05 	( . 
	ld a,0dah		;45e7	3e da 	> . 
	call ASSERT_NEXT_TOKEN_BE		;45e9	cd 48 60 	. H ` 
l45ech:
	ld a,b			;45ec	78 	x 
	cp 002h		;45ed	fe 02 	. . 
	jr z,l45fah		;45ef	28 09 	( . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;45f1	cd 56 60 	. V ` 
	ld a,b			;45f4	78 	x 
	and a			;45f5	a7 	. 
	nop			;45f6	00 	. 
	nop			;45f7	00 	. 
	jr l4601h		;45f8	18 07 	. . 
l45fah:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;45fa	cd 56 60 	. V ` 
	ld a,l			;45fd	7d 	} 
	or h			;45fe	b4 	. 
	nop			;45ff	00 	. 
	nop			;4600	00 	. 
l4601h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4601	cd 56 60 	. V ` 
	jp z,0ffffh		;4604	ca ff ff 	. . . 
	nop			;4607	00 	. 
	nop			;4608	00 	. 
	ld hl,(0f560h)		;4609	2a 60 f5 	* ` . 
	push hl			;460c	e5 	. 
	call GET_NEXT_TOKEN		;460d	cd 30 60 	. 0 ` 
	cp 00dh		;4610	fe 0d 	. . 
	jr z,l4618h		;4612	28 04 	( . 
	cp 00eh		;4614	fe 0e 	. . 
	jr nz,l4623h		;4616	20 0b 	  . 
l4618h:
	call TOKEN_89_GOTO		;4618	cd 96 46 	. . F 
	call sub_4f00h		;461b	cd 00 4f 	. . O 
	jr z,l4626h		;461e	28 06 	( . 
	jp 04e16h		;4620	c3 16 4e 	. . N 
l4623h:
	call sub_4258h		;4623	cd 58 42 	. X B 
l4626h:
	and a			;4626	a7 	. 
	jr z,l4671h		;4627	28 48 	( H 
	inc ix		;4629	dd 23 	. # 
	call GET_NEXT_TOKEN		;462b	cd 30 60 	. 0 ` 
	cp 0a1h		;462e	fe a1 	. . 
	jr nz,l4623h		;4630	20 f1 	  . 
	inc ix		;4632	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4634	cd 56 60 	. V ` 
	jp 0ffffh		;4637	c3 ff ff 	. . . 
	nop			;463a	00 	. 
	nop			;463b	00 	. 
	ld hl,(0f560h)		;463c	2a 60 f5 	* ` . 
	ld e,l			;463f	5d 	] 
	ld d,h			;4640	54 	T 
	ex (sp),hl			;4641	e3 	. 
	dec hl			;4642	2b 	+ 
	ld (hl),d			;4643	72 	r 
	dec hl			;4644	2b 	+ 
	ld (hl),e			;4645	73 	s 
	call GET_NEXT_TOKEN		;4646	cd 30 60 	. 0 ` 
	cp 00dh		;4649	fe 0d 	. . 
	jr z,l4651h		;464b	28 04 	( . 
	cp 00eh		;464d	fe 0e 	. . 
	jr nz,l465ch		;464f	20 0b 	  . 
l4651h:
	call TOKEN_89_GOTO		;4651	cd 96 46 	. . F 
	call sub_4f00h		;4654	cd 00 4f 	. . O 
	jr z,l465fh		;4657	28 06 	( . 
	jp 04e16h		;4659	c3 16 4e 	. . N 
l465ch:
	call sub_4258h		;465c	cd 58 42 	. X B 
l465fh:
	push ix		;465f	dd e5 	. . 
	pop hl			;4661	e1 	. 
	and a			;4662	a7 	. 
	jr z,l4671h		;4663	28 0c 	( . 
	inc ix		;4665	dd 23 	. # 
	call GET_NEXT_TOKEN		;4667	cd 30 60 	. 0 ` 
	cp 0a1h		;466a	fe a1 	. . 
	jr nz,l465ch		;466c	20 ee 	  . 
	push hl			;466e	e5 	. 
	pop ix		;466f	dd e1 	. . 
l4671h:
	pop hl			;4671	e1 	. 
	dec hl			;4672	2b 	+ 
	dec hl			;4673	2b 	+ 
	ld de,(0f560h)		;4674	ed 5b 60 f5 	. [ ` . 
	ld (hl),e			;4678	73 	s 
	inc hl			;4679	23 	# 
	ld (hl),d			;467a	72 	r 
	ret			;467b	c9 	. 
TOKEN_8A_RUN:
	call sub_4f00h		;467c	cd 00 4f 	. . O 
	jr nz,TOKEN_89_GOTO		;467f	20 15 	  . 
	ld a,0c3h		;4681	3e c3 	> . 
	ld hl,(BASIC_BUF)		;4683	2a 5e f5 	* ^ . 
	jp WRITE_OPCODE_A_L_H		;4686	c3 a9 60 	. . ` 
TOKEN_8E_RETURN:
	call sub_4f00h		;4689	cd 00 4f 	. . O 
	ld a,0c9h		;468c	3e c9 	> . 
	jp z,WRITE_OPCODE		;468e	ca b7 60 	. . ` 
	ld a,0f1h		;4691	3e f1 	> . 
	call WRITE_OPCODE		;4693	cd b7 60 	. . ` 
TOKEN_89_GOTO:
	ld a,0c3h		;4696	3e c3 	> . 
	ld bc,0cd3eh		;4698	01 3e cd 	. > . 
sub_469bh:
	call WRITE_OPCODE		;469b	cd b7 60 	. . ` 
	call GET_NEXT_TOKEN		;469e	cd 30 60 	. 0 ` 
	ld l,(ix+001h)		;46a1	dd 6e 01 	. n . 
	ld h,(ix+002h)		;46a4	dd 66 02 	. f . 
	inc ix		;46a7	dd 23 	. # 
	inc ix		;46a9	dd 23 	. # 
	inc ix		;46ab	dd 23 	. # 
	cp 00eh		;46ad	fe 0e 	. . 
	jr z,l46bdh		;46af	28 0c 	( . 
	cp 00dh		;46b1	fe 0d 	. . 
	jp nz,04e16h		;46b3	c2 16 4e 	. . N 
	inc hl			;46b6	23 	# 
	inc hl			;46b7	23 	# 
	inc hl			;46b8	23 	# 
	ld a,(hl)			;46b9	7e 	~ 
	inc hl			;46ba	23 	# 
	ld h,(hl)			;46bb	66 	f 
	ld l,a			;46bc	6f 	o 
l46bdh:
	jp sub_431ch		;46bd	c3 1c 43 	. . C 
TOKEN_95_ON:
	call GET_NEXT_TOKEN		;46c0	cd 30 60 	. 0 ` 
	push ix		;46c3	dd e5 	. . 
	pop hl			;46c5	e1 	. 
	ld ix,07810h		;46c6	dd 21 10 78 	. ! . x 
	call BIOS_CALBAS		;46ca	cd 59 01 	. Y . 
	ei			;46cd	fb 	. 
	push hl			;46ce	e5 	. 
	pop ix		;46cf	dd e1 	. . 
	jp nc,l4744h		;46d1	d2 44 47 	. D G 
	call GET_NUM_PARAMETER		;46d4	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;46d7	cd 56 60 	. V ` 
	and a			;46da	a7 	. 
	jp z,0ffffh		;46db	ca ff ff 	. . . 
	nop			;46de	00 	. 
	nop			;46df	00 	. 
	ld hl,(0f560h)		;46e0	2a 60 f5 	* ` . 
	push hl			;46e3	e5 	. 
	call GET_NEXT_TOKEN		;46e4	cd 30 60 	. 0 ` 
	cp 089h		;46e7	fe 89 	. . 
	jr nz,l4701h		;46e9	20 16 	  . 
l46ebh:
	inc ix		;46eb	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;46ed	cd 56 60 	. V ` 
	dec a			;46f0	3d 	= 
	nop			;46f1	00 	. 
	nop			;46f2	00 	. 
	ld a,0cah		;46f3	3e ca 	> . 
	call sub_469bh		;46f5	cd 9b 46 	. . F 
	call GET_NEXT_TOKEN		;46f8	cd 30 60 	. 0 ` 
	cp 02ch		;46fb	fe 2c 	. , 
	jr z,l46ebh		;46fd	28 ec 	( . 
	jr l4733h		;46ff	18 32 	. 2 
l4701h:
	sub 08dh		;4701	d6 8d 	. . 
	jp nz,04e16h		;4703	c2 16 4e 	. . N 
	ld l,a			;4706	6f 	o 
	ld h,a			;4707	67 	g 
	push hl			;4708	e5 	. 
l4709h:
	inc ix		;4709	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;470b	cd 56 60 	. V ` 
	dec a			;470e	3d 	= 
	jr nz,$+8		;470f	20 06 	  . 
	nop			;4711	00 	. 
	nop			;4712	00 	. 
	call TOKEN_8D_GOSUB		;4713	cd 99 46 	. . F 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4716	cd 56 60 	. V ` 
	jp 0ffffh		;4719	c3 ff ff 	. . . 
	nop			;471c	00 	. 
	nop			;471d	00 	. 
	ld hl,(0f560h)		;471e	2a 60 f5 	* ` . 
	push hl			;4721	e5 	. 
	call GET_NEXT_TOKEN		;4722	cd 30 60 	. 0 ` 
	cp 02ch		;4725	fe 2c 	. , 
	jr z,l4709h		;4727	28 e0 	( . 
l4729h:
	pop hl			;4729	e1 	. 
	ld a,l			;472a	7d 	} 
	or h			;472b	b4 	. 
	jr z,l4733h		;472c	28 05 	( . 
	call sub_4734h		;472e	cd 34 47 	. 4 G 
	jr l4729h		;4731	18 f6 	. . 
l4733h:
	pop hl			;4733	e1 	. 
sub_4734h:
	ld bc,(0f560h)		;4734	ed 4b 60 f5 	. K ` . 
	dec hl			;4738	2b 	+ 
	ld (hl),b			;4739	70 	p 
	dec hl			;473a	2b 	+ 
	ld (hl),c			;473b	71 	q 
	ret			;473c	c9 	. 
sub_473dh:
	ld a,(0f560h)		;473d	3a 60 f5 	: ` . 
	sub c			;4740	91 	. 
	dec bc			;4741	0b 	. 
	ld (bc),a			;4742	02 	. 
	ret			;4743	c9 	. 
l4744h:
	inc ix		;4744	dd 23 	. # 
	ld a,08dh		;4746	3e 8d 	> . 
	ld (VAR_DUMMY),a		;4748	32 64 f5 	2 d . 
	call ASSERT_NEXT_TOKEN_BE		;474b	cd 48 60 	. H ` 
l474eh:
	call GET_NEXT_TOKEN		;474e	cd 30 60 	. 0 ` 
	cp 02ch		;4751	fe 2c 	. , 
	jr z,l478eh		;4753	28 39 	( 9 
	ld a,021h		;4755	3e 21 	> ! 
	call WRITE_OPCODE		;4757	cd b7 60 	. . ` 
	call GET_NEXT_TOKEN		;475a	cd 30 60 	. 0 ` 
	ld l,(ix+001h)		;475d	dd 6e 01 	. n . 
	ld h,(ix+002h)		;4760	dd 66 02 	. f . 
	inc ix		;4763	dd 23 	. # 
	inc ix		;4765	dd 23 	. # 
	inc ix		;4767	dd 23 	. # 
	cp 00eh		;4769	fe 0e 	. . 
	jp nz,04e16h		;476b	c2 16 4e 	. . N 
	ld a,l			;476e	7d 	} 
	or h			;476f	b4 	. 
	jr z,l4779h		;4770	28 07 	( . 
	push bc			;4772	c5 	. 
	call sub_431ch		;4773	cd 1c 43 	. . C 
	pop bc			;4776	c1 	. 
	jr l477ch		;4777	18 03 	. . 
l4779h:
	call sub_60ach		;4779	cd ac 60 	. . ` 
l477ch:
	ld hl,0fc4dh		;477c	21 4d fc 	! M . 
	ld e,b			;477f	58 	X 
	ld d,000h		;4780	16 00 	. . 
	add hl,de			;4782	19 	. 
	add hl,de			;4783	19 	. 
	add hl,de			;4784	19 	. 
	call l44b2h		;4785	cd b2 44 	. . D 
	call GET_NEXT_TOKEN		;4788	cd 30 60 	. 0 ` 
	cp 02ch		;478b	fe 2c 	. , 
	ret nz			;478d	c0 	. 
l478eh:
	inc ix		;478e	dd 23 	. # 
	inc b			;4790	04 	. 
	dec c			;4791	0d 	. 
	jr nz,l474eh		;4792	20 ba 	  . 
	jp 04e16h		;4794	c3 16 4e 	. . N 
TOKEN_AC_DEFINT:
	ld e,002h		;4797	1e 02 	. . 
	ld bc,0031eh		;4799	01 1e 03 	. . . 
	ld bc,0041eh		;479c	01 1e 04 	. . . 
	ld bc,0081eh		;479f	01 1e 08 	. . . 
l47a2h:
	call sub_47d1h		;47a2	cd d1 47 	. . G 
	ld c,a			;47a5	4f 	O 
	inc ix		;47a6	dd 23 	. # 
	call GET_NEXT_TOKEN		;47a8	cd 30 60 	. 0 ` 
	cp 0f2h		;47ab	fe f2 	. . 
	jr nz,l47b6h		;47ad	20 07 	  . 
	inc ix		;47af	dd 23 	. # 
	call sub_47d1h		;47b1	cd d1 47 	. . G 
	inc ix		;47b4	dd 23 	. # 
l47b6h:
	ld a,b			;47b6	78 	x 
	sub c			;47b7	91 	. 
	jp c,04e16h		;47b8	da 16 4e 	. . N 
	inc a			;47bb	3c 	< 
	ld hl,BASIC_DEFTBL		;47bc	21 ca f6 	! . . 
	ld b,000h		;47bf	06 00 	. . 
	add hl,bc			;47c1	09 	. 
l47c2h:
	ld (hl),e			;47c2	73 	s 
	inc hl			;47c3	23 	# 
	dec a			;47c4	3d 	= 
	jr nz,l47c2h		;47c5	20 fb 	  . 
	call GET_NEXT_TOKEN		;47c7	cd 30 60 	. 0 ` 
	cp 02ch		;47ca	fe 2c 	. , 
	ret nz			;47cc	c0 	. 
	inc ix		;47cd	dd 23 	. # 
	jr l47a2h		;47cf	18 d1 	. . 
sub_47d1h:
	call GET_NEXT_TOKEN		;47d1	cd 30 60 	. 0 ` 
	sub 041h		;47d4	d6 41 	. A 
	ld b,a			;47d6	47 	G 
	jr c,l47dch		;47d7	38 03 	8 . 
	cp 01ah		;47d9	fe 1a 	. . 
	ret c			;47db	d8 	. 
l47dch:
	jp 04e16h		;47dc	c3 16 4e 	. . N 
TOKEN_A4_SWAP:
	call sub_5bdch		;47df	cd dc 5b 	. . [ 
	push af			;47e2	f5 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;47e3	cd 56 60 	. V ` 
	push hl			;47e6	e5 	. 
	nop			;47e7	00 	. 
	nop			;47e8	00 	. 
	call ASSERT_NEXT_IS_COMMA		;47e9	cd 3e 60 	. > ` 
	call sub_5bdch		;47ec	cd dc 5b 	. . [ 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;47ef	cd 56 60 	. V ` 
	pop de			;47f2	d1 	. 
	nop			;47f3	00 	. 
	nop			;47f4	00 	. 
	pop bc			;47f5	c1 	. 
	cp b			;47f6	b8 	. 
	jp nz,04e10h		;47f7	c2 10 4e 	. . N 
	ld hl,l6bf9h		;47fa	21 f9 6b 	! . k 
	bit 5,a		;47fd	cb 6f 	. o 
	jr nz,l480bh		;47ff	20 0a 	  . 
	ld hl,l6bfdh		;4801	21 fd 6b 	! . k 
	bit 6,a		;4804	cb 77 	. w 
	jr nz,l480bh		;4806	20 03 	  . 
	ld hl,l6bf5h		;4808	21 f5 6b 	! . k 
l480bh:
	jp l4da0h		;480b	c3 a0 4d 	. . M 
TOKEN_82_FOR:
	call GET_VAR_PARAMETER		;480e	cd 01 65 	. . e 
	call ASSERT_NEXT_IS_EQUAL		;4811	cd 3a 60 	. : ` 
	push hl			;4814	e5 	. 
	bit 6,h		;4815	cb 74 	. t 
	jp nz,l487fh		;4817	c2 7f 48 	.  H 
	call sub_4f97h		;481a	cd 97 4f 	. . O 
	pop hl			;481d	e1 	. 
	push hl			;481e	e5 	. 
	call l6089h		;481f	cd 89 60 	. . ` 
	ld a,0d9h		;4822	3e d9 	> . 
	call ASSERT_NEXT_TOKEN_BE		;4824	cd 48 60 	. H ` 
	call sub_4fe9h		;4827	cd e9 4f 	. . O 
	ex de,hl			;482a	eb 	. 
	pop hl			;482b	e1 	. 
	inc b			;482c	04 	. 
	djnz l483bh		;482d	10 0c 	. . 
	push de			;482f	d5 	. 
	ld d,b			;4830	50 	P 
	ld e,b			;4831	58 	X 
	call sub_62ffh		;4832	cd ff 62 	. . b 
	pop de			;4835	d1 	. 
	call sub_6303h		;4836	cd 03 63 	. . c 
	jr l4849h		;4839	18 0e 	. . 
l483bh:
	ld de,00200h		;483b	11 00 02 	. . . 
	call sub_62ffh		;483e	cd ff 62 	. . b 
	ld a,022h		;4841	3e 22 	> " 
	call WRITE_OPCODE		;4843	cd b7 60 	. . ` 
	call sub_62e9h		;4846	cd e9 62 	. . b 
l4849h:
	call GET_NEXT_TOKEN		;4849	cd 30 60 	. 0 ` 
	cp 0dch		;484c	fe dc 	. . 
	jr z,l4858h		;484e	28 08 	( . 
	ld de,00001h		;4850	11 01 00 	. . . 
	call sub_6309h		;4853	cd 09 63 	. . c 
	jr l4878h		;4856	18 20 	.   
l4858h:
	inc ix		;4858	dd 23 	. # 
	push hl			;485a	e5 	. 
	call sub_4fe9h		;485b	cd e9 4f 	. . O 
	ex de,hl			;485e	eb 	. 
	pop hl			;485f	e1 	. 
	inc b			;4860	04 	. 
	djnz l4868h		;4861	10 05 	. . 
	call sub_6309h		;4863	cd 09 63 	. . c 
	jr l4878h		;4866	18 10 	. . 
l4868h:
	call sub_643dh		;4868	cd 3d 64 	. = d 
	ld e,002h		;486b	1e 02 	. . 
	call sub_62ffh		;486d	cd ff 62 	. . b 
	ld a,022h		;4870	3e 22 	> " 
	call WRITE_OPCODE		;4872	cd b7 60 	. . ` 
l4875h:
	call sub_62efh		;4875	cd ef 62 	. . b 
l4878h:
	ld de,(0f560h)		;4878	ed 5b 60 f5 	. [ ` . 
	jp l6311h		;487c	c3 11 63 	. . c 
l487fh:
	call sub_4faeh		;487f	cd ae 4f 	. . O 
	pop hl			;4882	e1 	. 
	push hl			;4883	e5 	. 
	call sub_44beh		;4884	cd be 44 	. . D 
	ld a,0d9h		;4887	3e d9 	> . 
	call ASSERT_NEXT_TOKEN_BE		;4889	cd 48 60 	. H ` 
	call sub_4faeh		;488c	cd ae 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;488f	cd 56 60 	. V ` 
	ld a,b			;4892	78 	x 
	nop			;4893	00 	. 
	nop			;4894	00 	. 
	pop hl			;4895	e1 	. 
	push hl			;4896	e5 	. 
	ld c,002h		;4897	0e 02 	. . 
	call sub_62e9h		;4899	cd e9 62 	. . b 
	call GET_NEXT_TOKEN		;489c	cd 30 60 	. 0 ` 
	cp 0dch		;489f	fe dc 	. . 
	jr nz,l48b0h		;48a1	20 0d 	  . 
	inc ix		;48a3	dd 23 	. # 
	call sub_4faeh		;48a5	cd ae 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;48a8	cd 56 60 	. V ` 
	ld a,b			;48ab	78 	x 
	nop			;48ac	00 	. 
	nop			;48ad	00 	. 
	jr l48bah		;48ae	18 0a 	. . 
l48b0h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;48b0	cd 56 60 	. V ` 
	ld a,081h		;48b3	3e 81 	> . 
	ld hl,00000h		;48b5	21 00 00 	! . . 
	nop			;48b8	00 	. 
	nop			;48b9	00 	. 
l48bah:
	pop hl			;48ba	e1 	. 
	ld c,002h		;48bb	0e 02 	. . 
	jr l4875h		;48bd	18 b6 	. . 
TOKEN_83_NEXT:
	call GET_VAR_PARAMETER		;48bf	cd 01 65 	. . e 
	bit 6,h		;48c2	cb 74 	. t 
	jp nz,l49dbh		;48c4	c2 db 49 	. . I 
	call sub_6084h		;48c7	cd 84 60 	. . ` 
	call sub_643dh		;48ca	cd 3d 64 	. = d 
	ld a,e			;48cd	7b 	{ 
	and a			;48ce	a7 	. 
	jr nz,l491eh		;48cf	20 4d 	  M 
	call sub_6447h		;48d1	cd 47 64 	. G d 
	ld a,(VAR_NY)		;48d4	3a 6c f5 	: l . 
	and a			;48d7	a7 	. 
	jr nz,l48fdh		;48d8	20 23 	  # 
	or d			;48da	b2 	. 
	jr nz,l48e2h		;48db	20 05 	  . 
	ld a,e			;48dd	7b 	{ 
	ld c,023h		;48de	0e 23 	. # 
	jr l48eah		;48e0	18 08 	. . 
l48e2h:
	inc a			;48e2	3c 	< 
	jr nz,l48fdh		;48e3	20 18 	  . 
	ld a,e			;48e5	7b 	{ 
	neg		;48e6	ed 44 	. D 
	ld c,02bh		;48e8	0e 2b 	. + 
l48eah:
	cp 005h		;48ea	fe 05 	. . 
	jr nc,l48fdh		;48ec	30 0f 	0 . 
	and a			;48ee	a7 	. 
	jr z,l48f8h		;48ef	28 07 	( . 
	ld b,a			;48f1	47 	G 
	ld a,c			;48f2	79 	y 
l48f3h:
	call WRITE_OPCODE		;48f3	cd b7 60 	. . ` 
	djnz l48f3h		;48f6	10 fb 	. . 
l48f8h:
	ld bc,00000h		;48f8	01 00 00 	. . . 
	jr l4905h		;48fb	18 08 	. . 
l48fdh:
	ex de,hl			;48fd	eb 	. 
	call sub_6077h		;48fe	cd 77 60 	. w ` 
	ex de,hl			;4901	eb 	. 
	call sub_4951h		;4902	cd 51 49 	. Q I 
l4905h:
	push bc			;4905	c5 	. 
	call l6089h		;4906	cd 89 60 	. . ` 
	bit 7,d		;4909	cb 7a 	. z 
	call sub_4971h		;490b	cd 71 49 	. q I 
l490eh:
	pop hl			;490e	e1 	. 
	ld a,l			;490f	7d 	} 
l4910h:
	or h			;4910	b4 	. 
	call nz,sub_4734h		;4911	c4 34 47 	. 4 G 
l4914h:
	call GET_NEXT_TOKEN		;4914	cd 30 60 	. 0 ` 
	cp 02ch		;4917	fe 2c 	. , 
	ret nz			;4919	c0 	. 
	inc ix		;491a	dd 23 	. # 
	jr TOKEN_83_NEXT		;491c	18 a1 	. . 
l491eh:
	push hl			;491e	e5 	. 
	ld hl,05bedh		;491f	21 ed 5b 	! . [ 
	call sub_60ach		;4922	cd ac 60 	. . ` 
	pop hl			;4925	e1 	. 
	call sub_62efh		;4926	cd ef 62 	. . b 
	call sub_4951h		;4929	cd 51 49 	. Q I 
	push bc			;492c	c5 	. 
	call l6089h		;492d	cd 89 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4930	cd 56 60 	. V ` 
	bit 7,d		;4933	cb 7a 	. z 
l4935h:
	jr nz,l4935h		;4935	20 fe 	  . 
	nop			;4937	00 	. 
	nop			;4938	00 	. 
	xor a			;4939	af 	. 
	call sub_4971h		;493a	cd 71 49 	. q I 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;493d	cd 56 60 	. V ` 
l4940h:
	jr l4940h		;4940	18 fe 	. . 
	nop			;4942	00 	. 
	nop			;4943	00 	. 
	call sub_473dh		;4944	cd 3d 47 	. = G 
	or 001h		;4947	f6 01 	. . 
	call sub_4971h		;4949	cd 71 49 	. q I 
	call sub_473dh		;494c	cd 3d 47 	. = G 
	jr l490eh		;494f	18 bd 	. . 
sub_4951h:
	ld a,(VAR_NY)		;4951	3a 6c f5 	: l . 
	and a			;4954	a7 	. 
	jr nz,l4961h		;4955	20 0a 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4957	cd 56 60 	. V ` 
	add hl,de			;495a	19 	. 
	nop			;495b	00 	. 
	nop			;495c	00 	. 
	ld bc,00000h		;495d	01 00 00 	. . . 
	ret			;4960	c9 	. 
l4961h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4961	cd 56 60 	. V ` 
	and a			;4964	a7 	. 
	adc hl,de		;4965	ed 5a 	. Z 
l4967h:
	jp pe,l4967h		;4967	ea 67 49 	. g I 
	nop			;496a	00 	. 
	nop			;496b	00 	. 
	ld bc,(0f560h)		;496c	ed 4b 60 f5 	. K ` . 
	ret			;4970	c9 	. 
sub_4971h:
	ld bc,(0f560h)		;4971	ed 4b 60 f5 	. K ` . 
	push bc			;4975	c5 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4976	cd 56 60 	. V ` 
	ld a,h			;4979	7c 	| 
	xor 080h		;497a	ee 80 	. . 
	ld h,a			;497c	67 	g 
	nop			;497d	00 	. 
	nop			;497e	00 	. 
	push af			;497f	f5 	. 
	ld a,0ebh		;4980	3e eb 	> . 
	call z,WRITE_OPCODE		;4982	cc b7 60 	. . ` 
	call sub_643dh		;4985	cd 3d 64 	. = d 
	ld a,d			;4988	7a 	z 
	and a			;4989	a7 	. 
	jr nz,l499eh		;498a	20 12 	  . 
	call sub_6441h		;498c	cd 41 64 	. A d 
	ex de,hl			;498f	eb 	. 
	ld a,h			;4990	7c 	| 
	xor 080h		;4991	ee 80 	. . 
	ld h,a			;4993	67 	g 
	pop af			;4994	f1 	. 
	call z,sub_607ch		;4995	cc 7c 60 	. | ` 
	call nz,sub_6077h		;4998	c4 77 60 	. w ` 
	ex de,hl			;499b	eb 	. 
	jr l49c8h		;499c	18 2a 	. * 
l499eh:
	pop af			;499e	f1 	. 
	jr nz,l49b4h		;499f	20 13 	  . 
	ld a,02ah		;49a1	3e 2a 	> * 
	call WRITE_OPCODE		;49a3	cd b7 60 	. . ` 
	call sub_62e9h		;49a6	cd e9 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;49a9	cd 56 60 	. V ` 
	ld a,h			;49ac	7c 	| 
	xor 080h		;49ad	ee 80 	. . 
	ld h,a			;49af	67 	g 
	nop			;49b0	00 	. 
	nop			;49b1	00 	. 
	jr l49c8h		;49b2	18 14 	. . 
l49b4h:
	push hl			;49b4	e5 	. 
	ld hl,05bedh		;49b5	21 ed 5b 	! . [ 
	call sub_60ach		;49b8	cd ac 60 	. . ` 
	pop hl			;49bb	e1 	. 
	call sub_62e9h		;49bc	cd e9 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;49bf	cd 56 60 	. V ` 
	ld a,d			;49c2	7a 	z 
	xor 080h		;49c3	ee 80 	. . 
	ld d,a			;49c5	57 	W 
	nop			;49c6	00 	. 
	nop			;49c7	00 	. 
l49c8h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;49c8	cd 56 60 	. V ` 
	sbc hl,de		;49cb	ed 52 	. R 
	nop			;49cd	00 	. 
	nop			;49ce	00 	. 
	call sub_644fh		;49cf	cd 4f 64 	. O d 
	ex de,hl			;49d2	eb 	. 
	ld a,0d2h		;49d3	3e d2 	> . 
	call WRITE_OPCODE_A_L_H		;49d5	cd a9 60 	. . ` 
	ex de,hl			;49d8	eb 	. 
	pop bc			;49d9	c1 	. 
	ret			;49da	c9 	. 
l49dbh:
	ld c,00ah		;49db	0e 0a 	. . 
	call sub_62e5h		;49dd	cd e5 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;49e0	cd 56 60 	. V ` 
	ld c,a			;49e3	4f 	O 
	ex de,hl			;49e4	eb 	. 
	nop			;49e5	00 	. 
	nop			;49e6	00 	. 
	ld c,00ah		;49e7	0e 0a 	. . 
	call sub_62efh		;49e9	cd ef 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;49ec	cd 56 60 	. V ` 
	push hl			;49ef	e5 	. 
	ld b,a			;49f0	47 	G 
	call sub_76c1h		;49f1	cd c1 76 	. . v 
	nop			;49f4	00 	. 
	nop			;49f5	00 	. 
	call sub_44beh		;49f6	cd be 44 	. . D 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;49f9	cd 56 60 	. V ` 
	ld c,b			;49fc	48 	H 
	ex de,hl			;49fd	eb 	. 
	nop			;49fe	00 	. 
	nop			;49ff	00 	. 
	ld c,00ah		;4a00	0e 0a 	. . 
	call sub_62e9h		;4a02	cd e9 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4a05	cd 56 60 	. V ` 
	ld b,a			;4a08	47 	G 
	call sub_78b2h		;4a09	cd b2 78 	. . x 
	pop hl			;4a0c	e1 	. 
	nop			;4a0d	00 	. 
	nop			;4a0e	00 	. 
	call sub_644fh		;4a0f	cd 4f 64 	. O d 
	ex de,hl			;4a12	eb 	. 
	ld a,0cah		;4a13	3e ca 	> . 
	call WRITE_OPCODE_A_L_H		;4a15	cd a9 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4a18	cd 56 60 	. V ` 
	bit 7,h		;4a1b	cb 7c 	. | 
	jr z,l4a20h		;4a1d	28 01 	( . 
	ccf			;4a1f	3f 	? 
l4a20h:
	nop			;4a20	00 	. 
	nop			;4a21	00 	. 
	ld a,0d2h		;4a22	3e d2 	> . 
	call WRITE_OPCODE_A_L_H		;4a24	cd a9 60 	. . ` 
	jp l4914h		;4a27	c3 14 49 	. . I 
TOKEN_8C_RESTORE:
	ld hl,(BASIC_TXTTAB)		;4a2a	2a 76 f6 	* v . 
	call sub_4f00h		;4a2d	cd 00 4f 	. . O 
	jr z,l4a5eh		;4a30	28 2c 	( , 
	cp 00eh		;4a32	fe 0e 	. . 
	jp nz,04e16h		;4a34	c2 16 4e 	. . N 
	ld c,(ix+001h)		;4a37	dd 4e 01 	. N . 
	ld b,(ix+002h)		;4a3a	dd 46 02 	. F . 
	inc ix		;4a3d	dd 23 	. # 
	inc ix		;4a3f	dd 23 	. # 
	inc ix		;4a41	dd 23 	. # 
	ld hl,(BASIC_TXTTAB)		;4a43	2a 76 f6 	* v . 
l4a46h:
	ld e,(hl)			;4a46	5e 	^ 
	inc hl			;4a47	23 	# 
	ld d,(hl)			;4a48	56 	V 
	ld a,e			;4a49	7b 	{ 
	or d			;4a4a	b2 	. 
	jp z,04e19h		;4a4b	ca 19 4e 	. . N 
	inc hl			;4a4e	23 	# 
	ld a,(hl)			;4a4f	7e 	~ 
	cp c			;4a50	b9 	. 
	jr nz,l4a58h		;4a51	20 05 	  . 
	inc hl			;4a53	23 	# 
	ld a,(hl)			;4a54	7e 	~ 
	cp b			;4a55	b8 	. 
	jr z,l4a5bh		;4a56	28 03 	( . 
l4a58h:
	ex de,hl			;4a58	eb 	. 
	jr l4a46h		;4a59	18 eb 	. . 
l4a5bh:
	dec hl			;4a5b	2b 	+ 
	dec hl			;4a5c	2b 	+ 
	dec hl			;4a5d	2b 	+ 
l4a5eh:
	dec hl			;4a5e	2b 	+ 
	call sub_607ch		;4a5f	cd 7c 60 	. | ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4a62	cd 56 60 	. V ` 
	ld (BASIC_DATPTR),hl		;4a65	22 c8 f6 	" . . 
	nop			;4a68	00 	. 
	nop			;4a69	00 	. 
	ret			;4a6a	c9 	. 
TOKEN_85_INPUT:
	call GET_NEXT_TOKEN		;4a6b	cd 30 60 	. 0 ` 
	cp 022h		;4a6e	fe 22 	. " 
	ld hl,l7511h		;4a70	21 11 75 	! . u 
	ld de,l753dh		;4a73	11 3d 75 	. = u 
	jr nz,sub_4a88h		;4a76	20 10 	  . 
	inc ix		;4a78	dd 23 	. # 
	call sub_5839h		;4a7a	cd 39 58 	. 9 X 
	ld a,03bh		;4a7d	3e 3b 	> ; 
	call ASSERT_NEXT_TOKEN_BE		;4a7f	cd 48 60 	. H ` 
	ld hl,l750eh		;4a82	21 0e 75 	! . u 
	ld de,l753ah		;4a85	11 3a 75 	. : u 
sub_4a88h:
	push hl			;4a88	e5 	. 
	push de			;4a89	d5 	. 
	push ix		;4a8a	dd e5 	. . 
	call GET_VAR_PARAMETER		;4a8c	cd 01 65 	. . e 
	pop ix		;4a8f	dd e1 	. . 
	bit 5,h		;4a91	cb 6c 	. l 
	jr z,l4abeh		;4a93	28 29 	( ) 
	pop hl			;4a95	e1 	. 
	pop af			;4a96	f1 	. 
	call l4da0h		;4a97	cd a0 4d 	. . M 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4a9a	cd 56 60 	. V ` 
	push hl			;4a9d	e5 	. 
	nop			;4a9e	00 	. 
	nop			;4a9f	00 	. 
	call sub_5bf1h		;4aa0	cd f1 5b 	. . [ 
	inc b			;4aa3	04 	. 
	djnz l4aach		;4aa4	10 06 	. . 
	call sub_60d5h		;4aa6	cd d5 60 	. . ` 
	jp l44e6h		;4aa9	c3 e6 44 	. . D 
l4aach:
	djnz l4ab4h		;4aac	10 06 	. . 
	call sub_60d5h		;4aae	cd d5 60 	. . ` 
	jp l44f4h		;4ab1	c3 f4 44 	. . D 
l4ab4h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4ab4	cd 56 60 	. V ` 
	ex de,hl			;4ab7	eb 	. 
	pop hl			;4ab8	e1 	. 
	nop			;4ab9	00 	. 
	nop			;4aba	00 	. 
	jp l450dh		;4abb	c3 0d 45 	. . E 
l4abeh:
	pop af			;4abe	f1 	. 
	ex (sp),hl			;4abf	e3 	. 
	call l4da0h		;4ac0	cd a0 4d 	. . M 
	pop hl			;4ac3	e1 	. 
	bit 6,h		;4ac4	cb 74 	. t 
	jr nz,l4aefh		;4ac6	20 27 	  ' 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4ac8	cd 56 60 	. V ` 
	call sub_784fh		;4acb	cd 4f 78 	. O x 
	push hl			;4ace	e5 	. 
	nop			;4acf	00 	. 
	nop			;4ad0	00 	. 
	call sub_5bf1h		;4ad1	cd f1 5b 	. . [ 
	inc b			;4ad4	04 	. 
	djnz l4addh		;4ad5	10 06 	. . 
	call sub_60d5h		;4ad7	cd d5 60 	. . ` 
	jp l44b2h		;4ada	c3 b2 44 	. . D 
l4addh:
	djnz l4ae5h		;4add	10 06 	. . 
	call sub_60d5h		;4adf	cd d5 60 	. . ` 
	jp l6089h		;4ae2	c3 89 60 	. . ` 
l4ae5h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4ae5	cd 56 60 	. V ` 
	pop de			;4ae8	d1 	. 
	ld (hl),e			;4ae9	73 	s 
	inc hl			;4aea	23 	# 
	ld (hl),d			;4aeb	72 	r 
	nop			;4aec	00 	. 
	nop			;4aed	00 	. 
	ret			;4aee	c9 	. 
l4aefh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4aef	cd 56 60 	. V ` 
	push hl			;4af2	e5 	. 
	push bc			;4af3	c5 	. 
	nop			;4af4	00 	. 
	nop			;4af5	00 	. 
	call sub_5bf1h		;4af6	cd f1 5b 	. . [ 
	inc b			;4af9	04 	. 
	djnz l4b05h		;4afa	10 09 	. . 
	call sub_60d5h		;4afc	cd d5 60 	. . ` 
	call sub_60d5h		;4aff	cd d5 60 	. . ` 
	jp l44a6h		;4b02	c3 a6 44 	. . D 
l4b05h:
	djnz l4b10h		;4b05	10 09 	. . 
	call sub_60d5h		;4b07	cd d5 60 	. . ` 
	call sub_60d5h		;4b0a	cd d5 60 	. . ` 
	jp sub_44beh		;4b0d	c3 be 44 	. . D 
l4b10h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b10	cd 56 60 	. V ` 
	pop bc			;4b13	c1 	. 
	pop de			;4b14	d1 	. 
	ld (hl),b			;4b15	70 	p 
	inc hl			;4b16	23 	# 
	ld (hl),e			;4b17	73 	s 
	inc hl			;4b18	23 	# 
	ld (hl),d			;4b19	72 	r 
	nop			;4b1a	00 	. 
	nop			;4b1b	00 	. 
	ret			;4b1c	c9 	. 
TOKEN_9D_LPRINT:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b1d	cd 56 60 	. V ` 
	ld a,001h		;4b20	3e 01 	> . 
	ld (0f416h),a		;4b22	32 16 f4 	2 . . 
	nop			;4b25	00 	. 
	nop			;4b26	00 	. 
	call TOKEN_91_PRINT		;4b27	cd 34 4b 	. 4 K 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b2a	cd 56 60 	. V ` 
	xor a			;4b2d	af 	. 
	ld (0f416h),a		;4b2e	32 16 f4 	2 . . 
	nop			;4b31	00 	. 
	nop			;4b32	00 	. 
	ret			;4b33	c9 	. 
TOKEN_91_PRINT:
	call sub_4f00h		;4b34	cd 00 4f 	. . O 
	jr z,WRITE_CMD_PRINT_CRLF		;4b37	28 22 	( " 
l4b39h:
	call sub_4b64h		;4b39	cd 64 4b 	. d K 
	call sub_4f00h		;4b3c	cd 00 4f 	. . O 
	jr z,WRITE_CMD_PRINT_CRLF		;4b3f	28 1a 	( . 
	cp 02ch		;4b41	fe 2c 	. , 
	jr nz,l4b4fh		;4b43	20 0a 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b45	cd 56 60 	. V ` 
	call sub_74eah		;4b48	cd ea 74 	. . t 
	nop			;4b4b	00 	. 
	nop			;4b4c	00 	. 
	jr l4b53h		;4b4d	18 04 	. . 
l4b4fh:
	cp 03bh		;4b4f	fe 3b 	. ; 
	jr nz,l4b55h		;4b51	20 02 	  . 
l4b53h:
	inc ix		;4b53	dd 23 	. # 
l4b55h:
	call sub_4f00h		;4b55	cd 00 4f 	. . O 
	jr nz,l4b39h		;4b58	20 df 	  . 
	ret			;4b5a	c9 	. 
WRITE_CMD_PRINT_CRLF:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b5b	cd 56 60 	. V ` 
	call CMD_PRINT_CRLF		;4b5e	cd ee 74 	. . t 
	nop			;4b61	00 	. 
	nop			;4b62	00 	. 
	ret			;4b63	c9 	. 
sub_4b64h:
	call sub_5012h		;4b64	cd 12 50 	. . P 
	inc b			;4b67	04 	. 
	djnz l4b6fh		;4b68	10 05 	. . 
	call sub_607ch		;4b6a	cd 7c 60 	. | ` 
	jr WRITE_CMD_PRINT_INT		;4b6d	18 09 	. . 
l4b6fh:
	djnz l4b76h		;4b6f	10 05 	. . 
	call sub_6084h		;4b71	cd 84 60 	. . ` 
	jr WRITE_CMD_PRINT_INT		;4b74	18 02 	. . 
l4b76h:
	djnz l4b81h		;4b76	10 09 	. . 
WRITE_CMD_PRINT_INT:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b78	cd 56 60 	. V ` 
	call CMD_PRINT_INT		;4b7b	cd ff 74 	. . t 
	nop			;4b7e	00 	. 
	nop			;4b7f	00 	. 
	ret			;4b80	c9 	. 
l4b81h:
	djnz WRITE_CMD_PRINT_STR		;4b81	10 09 	. . 
WRITE_CMD_PRINT_FLOAT:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b83	cd 56 60 	. V ` 
	call CMD_PRINT_FLOAT		;4b86	cd 04 75 	. . u 
	nop			;4b89	00 	. 
	nop			;4b8a	00 	. 
	ret			;4b8b	c9 	. 
WRITE_CMD_PRINT_STR:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b8c	cd 56 60 	. V ` 
	call CMD_PRINT_STR		;4b8f	cd f5 74 	. . t 
	nop			;4b92	00 	. 
	nop			;4b93	00 	. 
	ret			;4b94	c9 	. 
TOKEN_C0_BEEP:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4b95	cd 56 60 	. V ` 
	call BIOS_BEEP		;4b98	cd c0 00 	. . . 
	nop			;4b9b	00 	. 
	nop			;4b9c	00 	. 
	ret			;4b9d	c9 	. 
TOKEN_98_POKE:
	call sub_4fa9h		;4b9e	cd a9 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;4ba1	cd 3e 60 	. > ` 
	dec ix		;4ba4	dd 2b 	. + 
	ld de,sub_4fa9h		;4ba6	11 a9 4f 	. . O 
	call sub_5f02h		;4ba9	cd 02 5f 	. . _ 
	call 0d24bh		;4bac	cd 4b d2 	. K . 
	ld c,e			;4baf	4b 	K 
	rst 10h			;4bb0	d7 	. 
	ld c,e			;4bb1	4b 	K 
	ex (sp),hl			;4bb2	e3 	. 
	ld c,e			;4bb3	4b 	K 
	jp pe,0f14bh		;4bb4	ea 4b f1 	. K . 
	ld c,e			;4bb7	4b 	K 
	ld (bc),a			;4bb8	02 	. 
	ld c,h			;4bb9	4c 	L 
	ex af,af'			;4bba	08 	. 
	ld c,h			;4bbb	4c 	L 
	ld (de),a			;4bbc	12 	. 
	ld c,h			;4bbd	4c 	L 
	nop			;4bbe	00 	. 
	nop			;4bbf	00 	. 
	nop			;4bc0	00 	. 
	nop			;4bc1	00 	. 
	nop			;4bc2	00 	. 
	nop			;4bc3	00 	. 
	nop			;4bc4	00 	. 
	nop			;4bc5	00 	. 
	nop			;4bc6	00 	. 
	nop			;4bc7	00 	. 
	nop			;4bc8	00 	. 
	nop			;4bc9	00 	. 
	nop			;4bca	00 	. 
	nop			;4bcb	00 	. 
	ret			;4bcc	c9 	. 
	call sub_4f5ah		;4bcd	cd 5a 4f 	. Z O 
	jr l4bddh		;4bd0	18 0b 	. . 
	call sub_4f75h		;4bd2	cd 75 4f 	. u O 
	jr l4bddh		;4bd5	18 06 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4bd7	cd 56 60 	. V ` 
	ld a,l			;4bda	7d 	} 
	nop			;4bdb	00 	. 
	nop			;4bdc	00 	. 
l4bddh:
	ex de,hl			;4bdd	eb 	. 
	ld a,032h		;4bde	3e 32 	> 2 
	jp WRITE_OPCODE_A_L_H		;4be0	c3 a9 60 	. . ` 
	ex de,hl			;4be3	eb 	. 
	call sub_6084h		;4be4	cd 84 60 	. . ` 
	ex de,hl			;4be7	eb 	. 
	jr l4c02h		;4be8	18 18 	. . 
	ex de,hl			;4bea	eb 	. 
	call sub_6084h		;4beb	cd 84 60 	. . ` 
	ex de,hl			;4bee	eb 	. 
	jr l4c08h		;4bef	18 17 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4bf1	cd 56 60 	. V ` 
	ld a,l			;4bf4	7d 	} 
	nop			;4bf5	00 	. 
	nop			;4bf6	00 	. 
	ex de,hl			;4bf7	eb 	. 
	call sub_6084h		;4bf8	cd 84 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4bfb	cd 56 60 	. V ` 
	ld (hl),a			;4bfe	77 	w 
	nop			;4bff	00 	. 
	nop			;4c00	00 	. 
	ret			;4c01	c9 	. 
l4c02h:
	ld h,l			;4c02	65 	e 
	ld l,036h		;4c03	2e 36 	. 6 
	jp sub_60ach		;4c05	c3 ac 60 	. . ` 
l4c08h:
	call sub_4f75h		;4c08	cd 75 4f 	. u O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4c0b	cd 56 60 	. V ` 
	ld (hl),a			;4c0e	77 	w 
	nop			;4c0f	00 	. 
	nop			;4c10	00 	. 
	ret			;4c11	c9 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4c12	cd 56 60 	. V ` 
	ld a,l			;4c15	7d 	} 
	pop hl			;4c16	e1 	. 
	ld (hl),a			;4c17	77 	w 
	nop			;4c18	00 	. 
	nop			;4c19	00 	. 
	ret			;4c1a	c9 	. 
TOKEN_9C_OUT:
	call sub_4fa9h		;4c1b	cd a9 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;4c1e	cd 3e 60 	. > ` 
	dec ix		;4c21	dd 2b 	. + 
	ld de,sub_4fa9h		;4c23	11 a9 4f 	. . O 
	call sub_5f02h		;4c26	cd 02 5f 	. . _ 
	ld c,d			;4c29	4a 	J 
	ld c,h			;4c2a	4c 	L 
	ld c,a			;4c2b	4f 	O 
	ld c,h			;4c2c	4c 	L 
	ld d,h			;4c2d	54 	T 
	ld c,h			;4c2e	4c 	L 
	ld h,b			;4c2f	60 	` 
	ld c,h			;4c30	4c 	L 
	ld h,l			;4c31	65 	e 
	ld c,h			;4c32	4c 	L 
	ld a,d			;4c33	7a 	z 
	ld c,h			;4c34	4c 	L 
	adc a,h			;4c35	8c 	. 
	ld c,h			;4c36	4c 	L 
	sub c			;4c37	91 	. 
	ld c,h			;4c38	4c 	L 
	sbc a,l			;4c39	9d 	. 
	ld c,h			;4c3a	4c 	L 
	nop			;4c3b	00 	. 
	nop			;4c3c	00 	. 
	nop			;4c3d	00 	. 
	nop			;4c3e	00 	. 
	nop			;4c3f	00 	. 
	nop			;4c40	00 	. 
	nop			;4c41	00 	. 
	nop			;4c42	00 	. 
	nop			;4c43	00 	. 
	nop			;4c44	00 	. 
	nop			;4c45	00 	. 
	nop			;4c46	00 	. 
	nop			;4c47	00 	. 
	nop			;4c48	00 	. 
	ret			;4c49	c9 	. 
	call sub_4f5ah		;4c4a	cd 5a 4f 	. Z O 
	jr l4c5ah		;4c4d	18 0b 	. . 
	call sub_4f75h		;4c4f	cd 75 4f 	. u O 
	jr l4c5ah		;4c52	18 06 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4c54	cd 56 60 	. V ` 
	ld a,l			;4c57	7d 	} 
	nop			;4c58	00 	. 
	nop			;4c59	00 	. 
l4c5ah:
	ld h,e			;4c5a	63 	c 
	ld l,0d3h		;4c5b	2e d3 	. . 
	jp sub_60ach		;4c5d	c3 ac 60 	. . ` 
	call sub_4f5ah		;4c60	cd 5a 4f 	. Z O 
	jr l4c68h		;4c63	18 03 	. . 
	call sub_4f75h		;4c65	cd 75 4f 	. u O 
l4c68h:
	ld hl,04bedh		;4c68	21 ed 4b 	! . K 
	call sub_60ach		;4c6b	cd ac 60 	. . ` 
	ex de,hl			;4c6e	eb 	. 
	call sub_62e5h		;4c6f	cd e5 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4c72	cd 56 60 	. V ` 
	out (c),a		;4c75	ed 79 	. y 
	nop			;4c77	00 	. 
	nop			;4c78	00 	. 
	ret			;4c79	c9 	. 
	ld hl,04bedh		;4c7a	21 ed 4b 	! . K 
	call sub_60ach		;4c7d	cd ac 60 	. . ` 
	ex de,hl			;4c80	eb 	. 
	call sub_62e5h		;4c81	cd e5 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4c84	cd 56 60 	. V ` 
	out (c),l		;4c87	ed 69 	. i 
	nop			;4c89	00 	. 
	nop			;4c8a	00 	. 
	ret			;4c8b	c9 	. 
	call sub_4f5ah		;4c8c	cd 5a 4f 	. Z O 
	jr l4c94h		;4c8f	18 03 	. . 
	call sub_4f75h		;4c91	cd 75 4f 	. u O 
l4c94h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4c94	cd 56 60 	. V ` 
	ld c,l			;4c97	4d 	M 
	out (c),a		;4c98	ed 79 	. y 
	nop			;4c9a	00 	. 
	nop			;4c9b	00 	. 
	ret			;4c9c	c9 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4c9d	cd 56 60 	. V ` 
	pop bc			;4ca0	c1 	. 
	out (c),l		;4ca1	ed 69 	. i 
	nop			;4ca3	00 	. 
	nop			;4ca4	00 	. 
	ret			;4ca5	c9 	. 
TOKEN_96_WAIT:
	call sub_4f97h		;4ca6	cd 97 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4ca9	cd 56 60 	. V ` 
	push hl			;4cac	e5 	. 
	nop			;4cad	00 	. 
	nop			;4cae	00 	. 
	call ASSERT_NEXT_IS_COMMA		;4caf	cd 3e 60 	. > ` 
	call GET_NUM_PARAMETER		;4cb2	cd 54 4f 	. T O 
	call GET_NEXT_TOKEN		;4cb5	cd 30 60 	. 0 ` 
	cp 02ch		;4cb8	fe 2c 	. , 
	jr z,l4cc9h		;4cba	28 0d 	( . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4cbc	cd 56 60 	. V ` 
	ld d,a			;4cbf	57 	W 
	pop bc			;4cc0	c1 	. 
l4cc1h:
	in a,(c)		;4cc1	ed 78 	. x 
	and d			;4cc3	a2 	. 
	jr z,l4cc1h		;4cc4	28 fb 	( . 
	nop			;4cc6	00 	. 
	nop			;4cc7	00 	. 
	ret			;4cc8	c9 	. 
l4cc9h:
	inc ix		;4cc9	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4ccb	cd 56 60 	. V ` 
	push af			;4cce	f5 	. 
	nop			;4ccf	00 	. 
	nop			;4cd0	00 	. 
	call GET_NUM_PARAMETER		;4cd1	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4cd4	cd 56 60 	. V ` 
	ld h,a			;4cd7	67 	g 
	pop de			;4cd8	d1 	. 
	pop bc			;4cd9	c1 	. 
l4cdah:
	in a,(c)		;4cda	ed 78 	. x 
	xor h			;4cdc	ac 	. 
	and d			;4cdd	a2 	. 
	jr z,l4cdah		;4cde	28 fa 	( . 
	nop			;4ce0	00 	. 
	nop			;4ce1	00 	. 
	ret			;4ce2	c9 	. 
TOKEN_97_DEF:
	ld a,0ddh		;4ce3	3e dd 	> . 
	call ASSERT_NEXT_TOKEN_BE		;4ce5	cd 48 60 	. H ` 
	call GET_NEXT_TOKEN		;4ce8	cd 30 60 	. 0 ` 
	ld c,000h		;4ceb	0e 00 	. . 
	sub 011h		;4ced	d6 11 	. . 
	jr c,l4cf8h		;4cef	38 07 	8 . 
	cp 00ah		;4cf1	fe 0a 	. . 
	jr nc,l4cf8h		;4cf3	30 03 	0 . 
	ld c,a			;4cf5	4f 	O 
	inc ix		;4cf6	dd 23 	. # 
l4cf8h:
	call ASSERT_NEXT_IS_EQUAL		;4cf8	cd 3a 60 	. : ` 
	push bc			;4cfb	c5 	. 
	call sub_4f97h		;4cfc	cd 97 4f 	. . O 
	pop bc			;4cff	c1 	. 
	ld b,000h		;4d00	06 00 	. . 
	ld hl,0f39ah		;4d02	21 9a f3 	! . . 
	add hl,bc			;4d05	09 	. 
	add hl,bc			;4d06	09 	. 
	jp l44b2h		;4d07	c3 b2 44 	. . D 
TOKEN_81_END:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4d0a	cd 56 60 	. V ` 
	jp EXEC_RETURN_TO_BASIC		;4d0d	c3 fa 4d 	. . M 
	nop			;4d10	00 	. 
	nop			;4d11	00 	. 
	ret			;4d12	c9 	. 
TOKEN_90_STOP:
	call sub_4f00h		;4d13	cd 00 4f 	. . O 
	jr z,TOKEN_81_END		;4d16	28 f2 	( . 
	ld hl,BASIC_TRPTBL_STOP		;4d18	21 6a fc 	! j . 
	jr l4d6ah		;4d1b	18 4d 	. M 
CMD_ON_INTERVAL:
	ld hl,BASIC_TRPTBL_INTERVAL		;4d1d	21 7f fc 	!  . 
	jr l4d6ah		;4d20	18 48 	. H 
TOKEN_C7_SPRITE:
	call GET_NEXT_TOKEN		;4d22	cd 30 60 	. 0 ` 
	cp 024h		;4d25	fe 24 	. $ 
	jp z,l68fah		;4d27	ca fa 68 	. . h 
	ld hl,BASIC_TRPTBL_SPRITE		;4d2a	21 6d fc 	! m . 
	jr l4d6ah		;4d2d	18 3b 	. ; 
l4d2fh:
	ld de,BASIC_TRPTBL_STRIG		;4d2f	11 70 fc 	. p . 
	jr l4d58h		;4d32	18 24 	. $ 
TOKEN_CC_KEY:
	call GET_NEXT_TOKEN		;4d34	cd 30 60 	. 0 ` 
	cp 095h		;4d37	fe 95 	. . 
	jr nz,l4d46h		;4d39	20 0b 	  . 
	inc ix		;4d3b	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4d3d	cd 56 60 	. V ` 
	call BIOS_DSPFNK		;4d40	cd cf 00 	. . . 
	nop			;4d43	00 	. 
	nop			;4d44	00 	. 
	ret			;4d45	c9 	. 
l4d46h:
	cp 0ebh		;4d46	fe eb 	. . 
	jr nz,l4d55h		;4d48	20 0b 	  . 
	inc ix		;4d4a	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4d4c	cd 56 60 	. V ` 
	call BIOS_ERAFNK		;4d4f	cd cc 00 	. . . 
	nop			;4d52	00 	. 
	nop			;4d53	00 	. 
	ret			;4d54	c9 	. 
l4d55h:
	ld de,0fc49h		;4d55	11 49 fc 	. I . 
l4d58h:
	push de			;4d58	d5 	. 
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;4d59	cd 42 60 	. B ` 
	call sub_4fa9h		;4d5c	cd a9 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;4d5f	cd 46 60 	. F ` 
	pop de			;4d62	d1 	. 
	inc b			;4d63	04 	. 
	djnz l4d6fh		;4d64	10 09 	. . 
	ex de,hl			;4d66	eb 	. 
	add hl,de			;4d67	19 	. 
	add hl,de			;4d68	19 	. 
	add hl,de			;4d69	19 	. 
l4d6ah:
	call sub_607ch		;4d6a	cd 7c 60 	. | ` 
	jr l4d85h		;4d6d	18 16 	. . 
l4d6fh:
	dec b			;4d6f	05 	. 
	call z,sub_6084h		;4d70	cc 84 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4d73	cd 56 60 	. V ` 
	ex de,hl			;4d76	eb 	. 
	nop			;4d77	00 	. 
	nop			;4d78	00 	. 
	ex de,hl			;4d79	eb 	. 
	call sub_607ch		;4d7a	cd 7c 60 	. | ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4d7d	cd 56 60 	. V ` 
	add hl,de			;4d80	19 	. 
	add hl,de			;4d81	19 	. 
	add hl,de			;4d82	19 	. 
	nop			;4d83	00 	. 
	nop			;4d84	00 	. 
l4d85h:
	call GET_NEXT_TOKEN		;4d85	cd 30 60 	. 0 ` 
	inc ix		;4d88	dd 23 	. # 
	cp 095h		;4d8a	fe 95 	. . 
	ld hl,l6c89h		;4d8c	21 89 6c 	! . l 
	jr z,l4da0h		;4d8f	28 0f 	( . 
	cp 0ebh		;4d91	fe eb 	. . 
	ld hl,l6c9ch		;4d93	21 9c 6c 	! . l 
	jr z,l4da0h		;4d96	28 08 	( . 
	cp 090h		;4d98	fe 90 	. . 
	jp nz,04e16h		;4d9a	c2 16 4e 	. . N 
	ld hl,l6ca5h		;4d9d	21 a5 6c 	! . l 
l4da0h:
	ld a,0cdh		;4da0	3e cd 	> . 
	jp WRITE_OPCODE_A_L_H		;4da2	c3 a9 60 	. . ` 
TOKEN_C4_SOUND:
	call GET_NUM_PARAMETER		;4da5	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_COMMA		;4da8	cd 3e 60 	. > ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4dab	cd 56 60 	. V ` 
	push af			;4dae	f5 	. 
	nop			;4daf	00 	. 
	nop			;4db0	00 	. 
	call GET_NUM_PARAMETER		;4db1	cd 54 4f 	. T O 
WRITE_CMD_SOUND:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4db4	cd 56 60 	. V ` 
	ld e,a			;4db7	5f 	_ 
	pop af			;4db8	f1 	. 
	cp 007h		;4db9	fe 07 	. . 
	jr nz,l4dc1h		;4dbb	20 04 	  . 
	res 6,e		;4dbd	cb b3 	. . 
	set 7,e		;4dbf	cb fb 	. . 
l4dc1h:
	call BIOS_WRTPSG		;4dc1	cd 93 00 	. . . 
	nop			;4dc4	00 	. 
	nop			;4dc5	00 	. 
	ret			;4dc6	c9 	. 
TOKEN_D8_LOCATE:
	call GET_NEXT_TOKEN		;4dc7	cd 30 60 	. 0 ` 
	call GET_NUM_PARAMETER		;4dca	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4dcd	cd 56 60 	. V ` 
	push af			;4dd0	f5 	. 
	nop			;4dd1	00 	. 
	nop			;4dd2	00 	. 
	call ASSERT_NEXT_IS_COMMA		;4dd3	cd 3e 60 	. > ` 
	call GET_NUM_PARAMETER		;4dd6	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4dd9	cd 56 60 	. V ` 
	pop hl			;4ddc	e1 	. 
	ld l,a			;4ddd	6f 	o 
	inc l			;4dde	2c 	, 
	inc h			;4ddf	24 	$ 
	call BIOS_POSIT		;4de0	cd c6 00 	. . . 
	nop			;4de3	00 	. 
	nop			;4de4	00 	. 
	ret			;4de5	c9 	. 
TOKEN_CB_TIME:
	call ASSERT_NEXT_IS_EQUAL		;4de6	cd 3a 60 	. : ` 
	call sub_4f97h		;4de9	cd 97 4f 	. . O 
	ld hl,BIOS_JIFFY		;4dec	21 9e fc 	! . . 
	jp l44b2h		;4def	c3 b2 44 	. . D 
TOKEN_84_DATA:
	call sub_4f00h		;4df2	cd 00 4f 	. . O 
	ret z			;4df5	c8 	. 
	inc ix		;4df6	dd 23 	. # 
	jr TOKEN_84_DATA		;4df8	18 f8 	. . 
EXEC_RETURN_TO_BASIC:
	ld hl,MODE		;4dfa	21 fc fa 	! . . 
	res 4,(hl)		;4dfd	cb a6 	. . 
	ld sp,(SAVSTK)		;4dff	ed 7b b1 f6 	. { . . 
	ld hl,(0f6afh)		;4e03	2a af f6 	* . . 
	ret			;4e06	c9 	. 
ERROR_MSG_CODES:
	ld e,004h		;4e07	1e 04 	. . 
	ld bc,00c1eh		;4e09	01 1e 0c 	. . . 
	ld bc,0051eh		;4e0c	01 1e 05 	. . . 
	ld bc,00d1eh		;4e0f	01 1e 0d 	. . . 
	ld bc,0101eh		;4e12	01 1e 10 	. . . 
	ld bc,0021eh		;4e15	01 1e 02 	. . . 
	ld bc,0081eh		;4e18	01 1e 08 	. . . 
	ld bc,0011eh		;4e1b	01 1e 01 	. . . 
	ld bc,0071eh		;4e1e	01 1e 07 	. . . 
	ld bc,00a1eh		;4e21	01 1e 0a 	. . . 
	ld bc,0091eh		;4e24	01 1e 09 	. . . 
	ld bc,0031eh		;4e27	01 1e 03 	. . . 
	ld hl,MODE		;4e2a	21 fc fa 	! . . 
	res 4,(hl)		;4e2d	cb a6 	. . 
	ld ix,BASIC_ERROR_HANDLER		;4e2f	dd 21 6f 40 	. ! o @ 
	jp BIOS_CALBAS		;4e33	c3 59 01 	. Y . 
TOKEN_CA_CALL:
	ld a,(0f563h)		;4e36	3a 63 f5 	: c . 
	and a			;4e39	a7 	. 
	jp z,04e16h		;4e3a	ca 16 4e 	. . N 
	call GET_NEXT_TOKEN		;4e3d	cd 30 60 	. 0 ` 
	ld de,PROCNM		;4e40	11 89 fd 	. . . 
	ld b,00fh		;4e43	06 0f 	. . 
l4e45h:
	ld a,(ix+000h)		;4e45	dd 7e 00 	. ~ . 
	call sub_4f03h		;4e48	cd 03 4f 	. . O 
	jr z,l4e57h		;4e4b	28 0a 	( . 
	cp 028h		;4e4d	fe 28 	. ( 
	jr z,l4e57h		;4e4f	28 06 	( . 
	ld (de),a			;4e51	12 	. 
	inc ix		;4e52	dd 23 	. # 
	inc de			;4e54	13 	. 
	djnz l4e45h		;4e55	10 ee 	. . 
l4e57h:
	ld a,b			;4e57	78 	x 
	cp 00fh		;4e58	fe 0f 	. . 
	jp z,04e16h		;4e5a	ca 16 4e 	. . N 
l4e5dh:
	xor a			;4e5d	af 	. 
	ld (de),a			;4e5e	12 	. 
	dec de			;4e5f	1b 	. 
	ld a,(de)			;4e60	1a 	. 
	cp 020h		;4e61	fe 20 	.   
	jr z,l4e5dh		;4e63	28 f8 	( . 
	ld de,init_end		;4e65	11 47 40 	. G @ 
	ld hl,PROCNM		;4e68	21 89 fd 	! . . 
	call kernel_start		;4e6b	cd 65 40 	. e @ 
	jp c,04e16h		;4e6e	da 16 4e 	. . N 
	ld hl,CMD_CALL_TURBO_OFF		;4e71	21 ff 4e 	! . N 
	sbc hl,de		;4e74	ed 52 	. R 
	jp nz,04e16h		;4e76	c2 16 4e 	. . N 
	call GET_NEXT_TOKEN		;4e79	cd 30 60 	. 0 ` 
	and a			;4e7c	a7 	. 
	jp nz,04e16h		;4e7d	c2 16 4e 	. . N 
	ld ix,l4e85h		;4e80	dd 21 85 4e 	. ! . N 
	ret			;4e84	c9 	. 
l4e85h:
	nop			;4e85	00 	. 
	nop			;4e86	00 	. 
	nop			;4e87	00 	. 
CMD_CALL_TURBO_ON:
	ld de,(BASIC_CURLIN)		;4e88	ed 5b 1c f4 	. [ . . 
	ld a,e			;4e8c	7b 	{ 
	and d			;4e8d	a2 	. 
	inc a			;4e8e	3c 	< 
	jp z,04e0ah		;4e8f	ca 0a 4e 	. . N 
	ld a,001h		;4e92	3e 01 	> . 
	jp CMD_CALL_RUN.PARSE		;4e94	c3 60 41 	. ` A 
PARSE_LINE:
	call GET_VAR_PARAMETER		;4e97	cd 01 65 	. . e 
	ld a,h			;4e9a	7c 	| 
	and 060h		;4e9b	e6 60 	. ` 
	jp nz,04e0dh		;4e9d	c2 0d 4e 	. . N 
	push hl			;4ea0	e5 	. 
	ld hl,SWPTMP1		;4ea1	21 bd f7 	! . . 
	ld a,(hl)			;4ea4	7e 	~ 
	and a			;4ea5	a7 	. 
	jr z,l4ea9h		;4ea6	28 01 	( . 
	inc hl			;4ea8	23 	# 
l4ea9h:
	ld (hl),025h		;4ea9	36 25 	6 % 
	call GET_NEXT_TOKEN		;4eab	cd 30 60 	. 0 ` 
	cp 028h		;4eae	fe 28 	. ( 
	jr z,l4edbh		;4eb0	28 29 	( ) 
	ld hl,BASIC_SWPTMP		;4eb2	21 bc f7 	! . . 
	push ix		;4eb5	dd e5 	. . 
	ld ix,l5f5dh		;4eb7	dd 21 5d 5f 	. ! ] _ 
	call BIOS_CALBAS		;4ebb	cd 59 01 	. Y . 
	ei			;4ebe	fb 	. 
	pop ix		;4ebf	dd e1 	. . 
	ld a,e			;4ec1	7b 	{ 
	or d			;4ec2	b2 	. 
	jp z,04e0dh		;4ec3	ca 0d 4e 	. . N 
	pop hl			;4ec6	e1 	. 
	call sub_63dfh		;4ec7	cd df 63 	. . c 
l4ecah:
	call GET_NEXT_TOKEN		;4eca	cd 30 60 	. 0 ` 
	inc ix		;4ecd	dd 23 	. # 
	cp 02ch		;4ecf	fe 2c 	. , 
	jr z,PARSE_LINE		;4ed1	28 c4 	( . 
	cp 029h		;4ed3	fe 29 	. ) 
	jp nz,04e16h		;4ed5	c2 16 4e 	. . N 
	jp l42cch		;4ed8	c3 cc 42 	. . B 
l4edbh:
	inc ix		;4edb	dd 23 	. # 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;4edd	cd 46 60 	. F ` 
	ld hl,BASIC_SWPTMP		;4ee0	21 bc f7 	! . . 
	ld a,001h		;4ee3	3e 01 	> . 
	ld (SUBFLG),a		;4ee5	32 a5 f6 	2 . . 
	push ix		;4ee8	dd e5 	. . 
	ld ix,l5ea4h		;4eea	dd 21 a4 5e 	. ! . ^ 
	call BIOS_CALBAS		;4eee	cd 59 01 	. Y . 
	ei			;4ef1	fb 	. 
	pop ix		;4ef2	dd e1 	. . 
	ld (SUBFLG),a		;4ef4	32 a5 f6 	2 . . 
	ld e,c			;4ef7	59 	Y 
	ld d,b			;4ef8	50 	P 
	pop hl			;4ef9	e1 	. 
	call sub_63e3h		;4efa	cd e3 63 	. . c 
	jr l4ecah		;4efd	18 cb 	. . 
CMD_CALL_TURBO_OFF:
	ret			;4eff	c9 	. 
sub_4f00h:
	call GET_NEXT_TOKEN		;4f00	cd 30 60 	. 0 ` 
sub_4f03h:
	and a			;4f03	a7 	. 
	ret z			;4f04	c8 	. 
	cp 03ah		;4f05	fe 3a 	. : 
	ret			;4f07	c9 	. 
l4f08h:
	ld d,04eh		;4f08	16 4e 	. N 
	ld b,a			;4f0a	47 	G 
	ld h,c			;4f0b	61 	a 
	ld c,h			;4f0c	4c 	L 
	ld h,c			;4f0d	61 	a 
	ld (hl),c			;4f0e	71 	q 
	ld h,c			;4f0f	61 	a 
	ld c,a			;4f10	4f 	O 
	ld e,e			;4f11	5b 	[ 
	ld (bc),a			;4f12	02 	. 
	ld e,e			;4f13	5b 	[ 
	jr z,$+93		;4f14	28 5b 	( [ 
	call p,0d15ah		;4f16	f4 5a d1 	. Z . 
	ld e,d			;4f19	5a 	Z 
	ret po			;4f1a	e0 	. 
	ld e,d			;4f1b	5a 	Z 
	in a,(05ah)		;4f1c	db 5a 	. Z 
	sub 05ah		;4f1e	d6 5a 	. Z 
	push hl			;4f20	e5 	. 
	ld e,d			;4f21	5a 	Z 
	jp pe,0ef5ah		;4f22	ea 5a ef 	. Z . 
	ld e,d			;4f25	5a 	Z 
	ld d,04eh		;4f26	16 4e 	. N 
	sbc a,c			;4f28	99 	. 
	ld e,e			;4f29	5b 	[ 
	adc a,c			;4f2a	89 	. 
	ld e,h			;4f2b	5c 	\ 
	ld (hl),c			;4f2c	71 	q 
	ld h,d			;4f2d	62 	b 
	ld e,b			;4f2e	58 	X 
	ld h,d			;4f2f	62 	b 
	sub h			;4f30	94 	. 
	ld h,d			;4f31	62 	b 
	add a,d			;4f32	82 	. 
	ld h,d			;4f33	62 	b 
	add hl,de			;4f34	19 	. 
	ld h,d			;4f35	62 	b 
	add a,e			;4f36	83 	. 
	ld e,e			;4f37	5b 	[ 
	ld (hl),c			;4f38	71 	q 
	ld l,d			;4f39	6a 	j 
	inc d			;4f3a	14 	. 
	ld h,d			;4f3b	62 	b 
	ld b,062h		;4f3c	06 62 	. b 
	ld bc,08e62h		;4f3e	01 62 8e 	. b . 
	ld e,h			;4f41	5c 	\ 
	dec bc			;4f42	0b 	. 
	ld h,d			;4f43	62 	b 
	ld d,04eh		;4f44	16 4e 	. N 
	ld d,04eh		;4f46	16 4e 	. N 
	ld d,04eh		;4f48	16 4e 	. N 
	dec d			;4f4a	15 	. 
	ld e,e			;4f4b	5b 	[ 
	and c			;4f4c	a1 	. 
	ld e,h			;4f4d	5c 	\ 
	or d			;4f4e	b2 	. 
	ld e,h			;4f4f	5c 	\ 
	ld (de),a			;4f50	12 	. 
	ld e,l			;4f51	5d 	] 
	push bc			;4f52	c5 	. 
	ld e,h			;4f53	5c 	\ 
GET_NUM_PARAMETER:
	call sub_5008h		;4f54	cd 08 50 	. . P 
sub_4f57h:
	inc b			;4f57	04 	. 
	djnz l4f73h		;4f58	10 19 	. . 
sub_4f5ah:
	ld a,l			;4f5a	7d 	} 
	and a			;4f5b	a7 	. 
	jr nz,l4f67h		;4f5c	20 09 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4f5e	cd 56 60 	. V ` 
	xor a			;4f61	af 	. 
	nop			;4f62	00 	. 
	nop			;4f63	00 	. 
	ld b,002h		;4f64	06 02 	. . 
	ret			;4f66	c9 	. 
l4f67h:
	ld a,03eh		;4f67	3e 3e 	> > 
	call WRITE_OPCODE		;4f69	cd b7 60 	. . ` 
	ld a,l			;4f6c	7d 	} 
	call WRITE_OPCODE		;4f6d	cd b7 60 	. . ` 
	ld b,002h		;4f70	06 02 	. . 
	ret			;4f72	c9 	. 
l4f73h:
	djnz l4f80h		;4f73	10 0b 	. . 
sub_4f75h:
	ld a,03ah		;4f75	3e 3a 	> : 
	call WRITE_OPCODE		;4f77	cd b7 60 	. . ` 
	call sub_62e5h		;4f7a	cd e5 62 	. . b 
	ld b,002h		;4f7d	06 02 	. . 
	ret			;4f7f	c9 	. 
l4f80h:
	djnz l4f8bh		;4f80	10 09 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4f82	cd 56 60 	. V ` 
	ld a,l			;4f85	7d 	} 
	nop			;4f86	00 	. 
	nop			;4f87	00 	. 
	ld b,002h		;4f88	06 02 	. . 
	ret			;4f8a	c9 	. 
l4f8bh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4f8b	cd 56 60 	. V ` 
	call sub_784fh		;4f8e	cd 4f 78 	. O x 
	ld a,l			;4f91	7d 	} 
	nop			;4f92	00 	. 
	nop			;4f93	00 	. 
	ld b,002h		;4f94	06 02 	. . 
	ret			;4f96	c9 	. 
sub_4f97h:
	call sub_5008h		;4f97	cd 08 50 	. . P 
	call sub_4fefh		;4f9a	cd ef 4f 	. . O 
	call sub_4fe0h		;4f9d	cd e0 4f 	. . O 
sub_4fa0h:
	ld a,b			;4fa0	78 	x 
	dec a			;4fa1	3d 	= 
	ret nz			;4fa2	c0 	. 
	call sub_6084h		;4fa3	cd 84 60 	. . ` 
	ld b,002h		;4fa6	06 02 	. . 
	ret			;4fa8	c9 	. 
sub_4fa9h:
	call sub_5008h		;4fa9	cd 08 50 	. . P 
	jr sub_4fefh		;4fac	18 41 	. A 
sub_4faeh:
	ld a,001h		;4fae	3e 01 	> . 
	ld (VAR_NX),a		;4fb0	32 6a f5 	2 j . 
	call sub_5008h		;4fb3	cd 08 50 	. . P 
l4fb6h:
	call sub_4fa0h		;4fb6	cd a0 4f 	. . O 
	ld a,b			;4fb9	78 	x 
	cp 002h		;4fba	fe 02 	. . 
	jr nz,l4fc4h		;4fbc	20 06 	  . 
	call sub_613eh		;4fbe	cd 3e 61 	. > a 
	ld b,003h		;4fc1	06 03 	. . 
	ret			;4fc3	c9 	. 
l4fc4h:
	inc b			;4fc4	04 	. 
	dec b			;4fc5	05 	. 
	ret nz			;4fc6	c0 	. 
	call TOKEN_MULTI_FUNCTION		;4fc7	cd 2d 78 	. - x 
l4fcah:
	ld a,006h		;4fca	3e 06 	> . 
	call WRITE_OPCODE		;4fcc	cd b7 60 	. . ` 
	ld a,b			;4fcf	78 	x 
	call WRITE_OPCODE		;4fd0	cd b7 60 	. . ` 
	and a			;4fd3	a7 	. 
	call nz,sub_607ch		;4fd4	c4 7c 60 	. | ` 
	ld b,003h		;4fd7	06 03 	. . 
	ret			;4fd9	c9 	. 
sub_4fdah:
	call sub_5008h		;4fda	cd 08 50 	. . P 
	call sub_4fa0h		;4fdd	cd a0 4f 	. . O 
sub_4fe0h:
	inc b			;4fe0	04 	. 
	dec b			;4fe1	05 	. 
	ret nz			;4fe2	c0 	. 
	call sub_607ch		;4fe3	cd 7c 60 	. | ` 
	ld b,002h		;4fe6	06 02 	. . 
	ret			;4fe8	c9 	. 
sub_4fe9h:
	call sub_5008h		;4fe9	cd 08 50 	. . P 
	call sub_4fa0h		;4fec	cd a0 4f 	. . O 
sub_4fefh:
	ld a,b			;4fef	78 	x 
	cp 003h		;4ff0	fe 03 	. . 
	ret nz			;4ff2	c0 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;4ff3	cd 56 60 	. V ` 
	call sub_784fh		;4ff6	cd 4f 78 	. O x 
	nop			;4ff9	00 	. 
	nop			;4ffa	00 	. 
	ld b,002h		;4ffb	06 02 	. . 
	ret			;4ffd	c9 	. 
sub_4ffeh:
	call sub_5012h		;4ffe	cd 12 50 	. . P 
sub_5001h:
	ld a,b			;5001	78 	x 
	cp 004h		;5002	fe 04 	. . 
	ret z			;5004	c8 	. 
	jp 04e10h		;5005	c3 10 4e 	. . N 
sub_5008h:
	call sub_5012h		;5008	cd 12 50 	. . P 
sub_500bh:
	ld a,b			;500b	78 	x 
	cp 004h		;500c	fe 04 	. . 
	ret nz			;500e	c0 	. 
	jp 04e10h		;500f	c3 10 4e 	. . N 
sub_5012h:
	call sub_5025h		;5012	cd 25 50 	. % P 
l5015h:
	call GET_NEXT_TOKEN		;5015	cd 30 60 	. 0 ` 
	cp 0f8h		;5018	fe f8 	. . 
	ret nz			;501a	c0 	. 
	ld de,sub_5025h		;501b	11 25 50 	. % P 
	ld a,008h		;501e	3e 08 	> . 
	call sub_5d23h		;5020	cd 23 5d 	. # ] 
	jr l5015h		;5023	18 f0 	. . 
sub_5025h:
	call sub_5038h		;5025	cd 38 50 	. 8 P 
l5028h:
	call GET_NEXT_TOKEN		;5028	cd 30 60 	. 0 ` 
	cp 0f7h		;502b	fe f7 	. . 
	ret nz			;502d	c0 	. 
	ld de,sub_5038h		;502e	11 38 50 	. 8 P 
	ld a,010h		;5031	3e 10 	> . 
	call sub_5d23h		;5033	cd 23 5d 	. # ] 
	jr l5028h		;5036	18 f0 	. . 
sub_5038h:
	call sub_504bh		;5038	cd 4b 50 	. K P 
l503bh:
	call GET_NEXT_TOKEN		;503b	cd 30 60 	. 0 ` 
	cp 0f6h		;503e	fe f6 	. . 
	ret nz			;5040	c0 	. 
	ld de,sub_504bh		;5041	11 4b 50 	. K P 
	ld a,000h		;5044	3e 00 	> . 
	call sub_5d23h		;5046	cd 23 5d 	. # ] 
	jr l503bh		;5049	18 f0 	. . 
sub_504bh:
	call sub_5120h		;504b	cd 20 51 	.   Q 
l504eh:
	ld d,000h		;504e	16 00 	. . 
l5050h:
	call GET_NEXT_TOKEN		;5050	cd 30 60 	. 0 ` 
	sub 0eeh		;5053	d6 ee 	. . 
	jr c,l5068h		;5055	38 11 	8 . 
	cp 003h		;5057	fe 03 	. . 
	jr nc,l5068h		;5059	30 0d 	0 . 
	cp 001h		;505b	fe 01 	. . 
	rla			;505d	17 	. 
	xor d			;505e	aa 	. 
	cp d			;505f	ba 	. 
	jp c,04e16h		;5060	da 16 4e 	. . N 
	ld d,a			;5063	57 	W 
	inc ix		;5064	dd 23 	. # 
	jr l5050h		;5066	18 e8 	. . 
l5068h:
	ld a,d			;5068	7a 	z 
	and a			;5069	a7 	. 
	ret z			;506a	c8 	. 
	dec a			;506b	3d 	= 
	add a,a			;506c	87 	. 
	ld d,a			;506d	57 	W 
	ld a,b			;506e	78 	x 
	cp 004h		;506f	fe 04 	. . 
	ld a,d			;5071	7a 	z 
	jr z,l507eh		;5072	28 0a 	( . 
	dec ix		;5074	dd 2b 	. + 
	ld de,sub_5120h		;5076	11 20 51 	.   Q 
	call sub_5e85h		;5079	cd 85 5e 	. . ^ 
	jr l504eh		;507c	18 d0 	. . 
l507eh:
	push af			;507e	f5 	. 
	call sub_50a1h		;507f	cd a1 50 	. . P 
	call sub_5120h		;5082	cd 20 51 	.   Q 
	call sub_5001h		;5085	cd 01 50 	. . P 
	pop af			;5088	f1 	. 
	ld e,a			;5089	5f 	_ 
	ld d,000h		;508a	16 00 	. . 
	ld hl,l50deh		;508c	21 de 50 	! . P 
	add hl,de			;508f	19 	. 
	ld a,(hl)			;5090	7e 	~ 
	inc hl			;5091	23 	# 
	ld h,(hl)			;5092	66 	f 
	ld l,a			;5093	6f 	o 
	ld a,0cdh		;5094	3e cd 	> . 
	call WRITE_OPCODE_A_L_H		;5096	cd a9 60 	. . ` 
	xor a			;5099	af 	. 
	ld (0f56bh),a		;509a	32 6b f5 	2 k . 
	ld b,002h		;509d	06 02 	. . 
	jr l504eh		;509f	18 ad 	. . 
sub_50a1h:
	ld hl,0f56bh		;50a1	21 6b f5 	! k . 
	ld a,(hl)			;50a4	7e 	~ 
	and a			;50a5	a7 	. 
	jp nz,04e13h		;50a6	c2 13 4e 	. . N 
	ld (hl),001h		;50a9	36 01 	6 . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;50ab	cd 56 60 	. V ` 
	call sub_7e99h		;50ae	cd 99 7e 	. . ~ 
	nop			;50b1	00 	. 
	nop			;50b2	00 	. 
	ret			;50b3	c9 	. 
l50b4h:
	rst 38h			;50b4	ff 	. 
	ld d,b			;50b5	50 	P 
	nop			;50b6	00 	. 
	ld d,c			;50b7	51 	Q 
	call pe,0ec50h		;50b8	ec 50 ec 	. P . 
	ld d,b			;50bb	50 	P 
	ld c,051h		;50bc	0e 51 	. Q 
	dec c			;50be	0d 	. 
	ld d,c			;50bf	51 	Q 
	nop			;50c0	00 	. 
	ld d,c			;50c1	51 	Q 
	rst 38h			;50c2	ff 	. 
	ld d,b			;50c3	50 	P 
	or 050h		;50c4	f6 50 	. P 
	or 050h		;50c6	f6 50 	. P 
	dec c			;50c8	0d 	. 
	ld d,c			;50c9	51 	Q 
	ld c,051h		;50ca	0e 51 	. Q 
	inc e			;50cc	1c 	. 
	ld d,c			;50cd	51 	Q 
	inc e			;50ce	1c 	. 
	ld d,c			;50cf	51 	Q 
l50d0h:
	adc a,b			;50d0	88 	. 
	ld a,b			;50d1	78 	x 
	halt			;50d2	76 	v 
	ld a,b			;50d3	78 	x 
	sub d			;50d4	92 	. 
	ld a,b			;50d5	78 	x 
	sbc a,e			;50d6	9b 	. 
	ld a,b			;50d7	78 	x 
	ld a,a			;50d8	7f 	 
	ld a,b			;50d9	78 	x 
	and h			;50da	a4 	. 
	ld a,b			;50db	78 	x 
	xor (hl)			;50dc	ae 	. 
	ld a,b			;50dd	78 	x 
l50deh:
	and h			;50de	a4 	. 
	ld a,(hl)			;50df	7e 	~ 
	xor (hl)			;50e0	ae 	. 
	ld a,(hl)			;50e1	7e 	~ 
	or a			;50e2	b7 	. 
	ld a,(hl)			;50e3	7e 	~ 
	ret nz			;50e4	c0 	. 
	ld a,(hl)			;50e5	7e 	~ 
	ret			;50e6	c9 	. 
	ld a,(hl)			;50e7	7e 	~ 
	jp nc,0dc7eh		;50e8	d2 7e dc 	. ~ . 
	ld a,(hl)			;50eb	7e 	~ 
	and a			;50ec	a7 	. 
	sbc hl,de		;50ed	ed 52 	. R 
	ld hl,0ffffh		;50ef	21 ff ff 	! . . 
	jr z,l50f5h		;50f2	28 01 	( . 
	inc hl			;50f4	23 	# 
l50f5h:
	nop			;50f5	00 	. 
	and a			;50f6	a7 	. 
	sbc hl,de		;50f7	ed 52 	. R 
	jr z,l50feh		;50f9	28 03 	( . 
	ld hl,0ffffh		;50fb	21 ff ff 	! . . 
l50feh:
	nop			;50fe	00 	. 
	ex de,hl			;50ff	eb 	. 
	ld a,h			;5100	7c 	| 
	xor 080h		;5101	ee 80 	. . 
	ld h,a			;5103	67 	g 
	ld a,d			;5104	7a 	z 
	xor 080h		;5105	ee 80 	. . 
	ld d,a			;5107	57 	W 
	sbc hl,de		;5108	ed 52 	. R 
	sbc hl,hl		;510a	ed 62 	. b 
	nop			;510c	00 	. 
	ex de,hl			;510d	eb 	. 
	ld a,h			;510e	7c 	| 
	xor 080h		;510f	ee 80 	. . 
	ld h,a			;5111	67 	g 
	ld a,d			;5112	7a 	z 
	xor 080h		;5113	ee 80 	. . 
	ld d,a			;5115	57 	W 
	sbc hl,de		;5116	ed 52 	. R 
	ccf			;5118	3f 	? 
	sbc hl,hl		;5119	ed 62 	. b 
	nop			;511b	00 	. 
	ld hl,0ffffh		;511c	21 ff ff 	! . . 
	nop			;511f	00 	. 
sub_5120h:
	call sub_5258h		;5120	cd 58 52 	. X R 
l5123h:
	ld de,sub_5258h		;5123	11 58 52 	. X R 
	call GET_NEXT_TOKEN		;5126	cd 30 60 	. 0 ` 
	cp 0f1h		;5129	fe f1 	. . 
	jr z,l5159h		;512b	28 2c 	( , 
	cp 0f2h		;512d	fe f2 	. . 
	ret nz			;512f	c0 	. 
	ld iy,l76bdh		;5130	fd 21 bd 76 	. ! . v 
	call sub_5f02h		;5134	cd 02 5f 	. . _ 
	and d			;5137	a2 	. 
	ld d,c			;5138	51 	Q 
	xor c			;5139	a9 	. 
	ld d,c			;513a	51 	Q 
	or b			;513b	b0 	. 
	ld d,c			;513c	51 	Q 
	cp h			;513d	bc 	. 
	ld d,c			;513e	51 	Q 
	jp 0ca51h		;513f	c3 51 ca 	. Q . 
	ld d,c			;5142	51 	Q 
	sub 051h		;5143	d6 51 	. Q 
	or 051h		;5145	f6 51 	. Q 
	ei			;5147	fb 	. 
	ld d,c			;5148	51 	Q 
	rst 18h			;5149	df 	. 
	ld h,b			;514a	60 	` 
	rst 28h			;514b	ef 	. 
	ld h,b			;514c	60 	` 
	call m,00760h		;514d	fc 60 07 	. ` . 
	ld h,c			;5150	61 	a 
	add hl,hl			;5151	29 	) 
	ld h,c			;5152	61 	a 
	inc l			;5153	2c 	, 
	ld h,c			;5154	61 	a 
	cpl			;5155	2f 	/ 
	ld h,c			;5156	61 	a 
	jr l5123h		;5157	18 ca 	. . 
l5159h:
	ld a,b			;5159	78 	x 
	cp 004h		;515a	fe 04 	. . 
	jr z,l5187h		;515c	28 29 	( ) 
	ld iy,sub_76c1h		;515e	fd 21 c1 76 	. ! . v 
	call sub_5f02h		;5162	cd 02 5f 	. . _ 
	dec c			;5165	0d 	. 
	ld d,d			;5166	52 	R 
	ld de,01452h		;5167	11 52 14 	. R . 
	ld d,d			;516a	52 	R 
	rla			;516b	17 	. 
	ld d,d			;516c	52 	R 
	ld e,052h		;516d	1e 52 	. R 
	dec h			;516f	25 	% 
	ld d,d			;5170	52 	R 
	jr z,$+84		;5171	28 52 	( R 
	ld b,h			;5173	44 	D 
	ld d,d			;5174	52 	R 
	ld c,c			;5175	49 	I 
	ld d,d			;5176	52 	R 
	rst 18h			;5177	df 	. 
	ld h,b			;5178	60 	` 
	rst 28h			;5179	ef 	. 
	ld h,b			;517a	60 	` 
	call m,00760h		;517b	fc 60 07 	. ` . 
	ld h,c			;517e	61 	a 
	add hl,de			;517f	19 	. 
	ld h,c			;5180	61 	a 
	inc e			;5181	1c 	. 
	ld h,c			;5182	61 	a 
	rra			;5183	1f 	. 
	ld h,c			;5184	61 	a 
	jr l5123h		;5185	18 9c 	. . 
l5187h:
	inc ix		;5187	dd 23 	. # 
	call sub_50a1h		;5189	cd a1 50 	. . P 
	call sub_5258h		;518c	cd 58 52 	. X R 
	call sub_5001h		;518f	cd 01 50 	. . P 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5192	cd 56 60 	. V ` 
	call sub_7efeh		;5195	cd fe 7e 	. . ~ 
	nop			;5198	00 	. 
	nop			;5199	00 	. 
	xor a			;519a	af 	. 
	ld (0f56bh),a		;519b	32 6b f5 	2 k . 
	ld b,004h		;519e	06 04 	. . 
	jr l5123h		;51a0	18 81 	. . 
	ex de,hl			;51a2	eb 	. 
	and a			;51a3	a7 	. 
	sbc hl,de		;51a4	ed 52 	. R 
	ld b,000h		;51a6	06 00 	. . 
	ret			;51a8	c9 	. 
	ex de,hl			;51a9	eb 	. 
	call sub_607ch		;51aa	cd 7c 60 	. | ` 
	ex de,hl			;51ad	eb 	. 
	jr l51f6h		;51ae	18 46 	. F 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;51b0	cd 56 60 	. V ` 
	ex de,hl			;51b3	eb 	. 
	nop			;51b4	00 	. 
	nop			;51b5	00 	. 
	ex de,hl			;51b6	eb 	. 
	call sub_607ch		;51b7	cd 7c 60 	. | ` 
	jr l5202h		;51ba	18 46 	. F 
	ex de,hl			;51bc	eb 	. 
	call sub_6084h		;51bd	cd 84 60 	. . ` 
	ex de,hl			;51c0	eb 	. 
	jr l51d6h		;51c1	18 13 	. . 
	ex de,hl			;51c3	eb 	. 
	call sub_6084h		;51c4	cd 84 60 	. . ` 
	ex de,hl			;51c7	eb 	. 
	jr l51f6h		;51c8	18 2c 	. , 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;51ca	cd 56 60 	. V ` 
	ex de,hl			;51cd	eb 	. 
	nop			;51ce	00 	. 
	nop			;51cf	00 	. 
	ex de,hl			;51d0	eb 	. 
	call sub_6084h		;51d1	cd 84 60 	. . ` 
	jr l5202h		;51d4	18 2c 	. , 
l51d6h:
	inc h			;51d6	24 	$ 
	dec h			;51d7	25 	% 
	jr nz,l51dfh		;51d8	20 05 	  . 
	ld a,l			;51da	7d 	} 
	cp 005h		;51db	fe 05 	. . 
	jr c,l51e8h		;51dd	38 09 	8 . 
l51dfh:
	xor a			;51df	af 	. 
	sub l			;51e0	95 	. 
	ld l,a			;51e1	6f 	o 
	sbc a,a			;51e2	9f 	. 
	sub h			;51e3	94 	. 
	ld h,a			;51e4	67 	g 
	jp l5228h		;51e5	c3 28 52 	. ( R 
l51e8h:
	and a			;51e8	a7 	. 
	ret z			;51e9	c8 	. 
	ld b,a			;51ea	47 	G 
l51ebh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;51eb	cd 56 60 	. V ` 
	dec hl			;51ee	2b 	+ 
	nop			;51ef	00 	. 
	nop			;51f0	00 	. 
	djnz l51ebh		;51f1	10 f8 	. . 
	ld b,002h		;51f3	06 02 	. . 
	ret			;51f5	c9 	. 
l51f6h:
	call sub_6094h		;51f6	cd 94 60 	. . ` 
	jr l5202h		;51f9	18 07 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;51fb	cd 56 60 	. V ` 
	ex de,hl			;51fe	eb 	. 
	pop hl			;51ff	e1 	. 
	nop			;5200	00 	. 
	nop			;5201	00 	. 
l5202h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5202	cd 56 60 	. V ` 
	and a			;5205	a7 	. 
	sbc hl,de		;5206	ed 52 	. R 
	nop			;5208	00 	. 
	nop			;5209	00 	. 
	ld b,002h		;520a	06 02 	. . 
	ret			;520c	c9 	. 
	add hl,de			;520d	19 	. 
	ld b,000h		;520e	06 00 	. . 
	ret			;5210	c9 	. 
	ex de,hl			;5211	eb 	. 
	jr l5217h		;5212	18 03 	. . 
	ex de,hl			;5214	eb 	. 
	jr l5228h		;5215	18 11 	. . 
l5217h:
	ex de,hl			;5217	eb 	. 
	call sub_6084h		;5218	cd 84 60 	. . ` 
	ex de,hl			;521b	eb 	. 
	jr l5228h		;521c	18 0a 	. . 
	ex de,hl			;521e	eb 	. 
	call sub_6084h		;521f	cd 84 60 	. . ` 
	ex de,hl			;5222	eb 	. 
	jr l5244h		;5223	18 1f 	. . 
	ex de,hl			;5225	eb 	. 
	jr l5244h		;5226	18 1c 	. . 
l5228h:
	inc h			;5228	24 	$ 
	dec h			;5229	25 	% 
	jr nz,l5231h		;522a	20 05 	  . 
	ld a,l			;522c	7d 	} 
	cp 005h		;522d	fe 05 	. . 
	jr c,l5236h		;522f	38 05 	8 . 
l5231h:
	call sub_6077h		;5231	cd 77 60 	. w ` 
	jr l524fh		;5234	18 19 	. . 
l5236h:
	and a			;5236	a7 	. 
	ret z			;5237	c8 	. 
	ld b,a			;5238	47 	G 
l5239h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5239	cd 56 60 	. V ` 
	inc hl			;523c	23 	# 
	nop			;523d	00 	. 
	nop			;523e	00 	. 
	djnz l5239h		;523f	10 f8 	. . 
	ld b,002h		;5241	06 02 	. . 
	ret			;5243	c9 	. 
l5244h:
	call sub_6094h		;5244	cd 94 60 	. . ` 
	jr l524fh		;5247	18 06 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5249	cd 56 60 	. V ` 
	pop de			;524c	d1 	. 
	nop			;524d	00 	. 
	nop			;524e	00 	. 
l524fh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;524f	cd 56 60 	. V ` 
	add hl,de			;5252	19 	. 
	nop			;5253	00 	. 
	nop			;5254	00 	. 
	ld b,002h		;5255	06 02 	. . 
	ret			;5257	c9 	. 
sub_5258h:
	call sub_52deh		;5258	cd de 52 	. . R 
l525bh:
	call GET_NEXT_TOKEN		;525b	cd 30 60 	. 0 ` 
	cp 0fbh		;525e	fe fb 	. . 
	ret nz			;5260	c0 	. 
	ld de,sub_52deh		;5261	11 de 52 	. . R 
	ld iy,l76b8h		;5264	fd 21 b8 76 	. ! . v 
	call sub_5f02h		;5268	cd 02 5f 	. . _ 
	adc a,l			;526b	8d 	. 
	ld d,d			;526c	52 	R 
	sub h			;526d	94 	. 
	ld d,d			;526e	52 	R 
	sbc a,h			;526f	9c 	. 
	ld d,d			;5270	52 	R 
	xor b			;5271	a8 	. 
	ld d,d			;5272	52 	R 
	xor a			;5273	af 	. 
	ld d,d			;5274	52 	R 
	or (hl)			;5275	b6 	. 
	ld d,d			;5276	52 	R 
	jp nz,0c752h		;5277	c2 52 c7 	. R . 
	ld d,d			;527a	52 	R 
	call z,09052h		;527b	cc 52 90 	. R . 
	ld d,e			;527e	53 	S 
	sbc a,a			;527f	9f 	. 
	ld d,e			;5280	53 	S 
	xor (hl)			;5281	ae 	. 
	ld d,e			;5282	53 	S 
	cp d			;5283	ba 	. 
	ld d,e			;5284	53 	S 
	rst 0			;5285	c7 	. 
	ld d,e			;5286	53 	S 
	call c,0d453h		;5287	dc 53 d4 	. S . 
	ld d,e			;528a	53 	S 
	jr l525bh		;528b	18 ce 	. . 
	ex de,hl			;528d	eb 	. 
	call l76b8h		;528e	cd b8 76 	. . v 
	ld b,000h		;5291	06 00 	. . 
	ret			;5293	c9 	. 
	ex de,hl			;5294	eb 	. 
	call sub_607ch		;5295	cd 7c 60 	. | ` 
	ex de,hl			;5298	eb 	. 
	jp l52c7h		;5299	c3 c7 52 	. . R 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;529c	cd 56 60 	. V ` 
	ex de,hl			;529f	eb 	. 
	nop			;52a0	00 	. 
	nop			;52a1	00 	. 
	ex de,hl			;52a2	eb 	. 
	call sub_607ch		;52a3	cd 7c 60 	. | ` 
	jr l52d3h		;52a6	18 2b 	. + 
	ex de,hl			;52a8	eb 	. 
	call sub_6084h		;52a9	cd 84 60 	. . ` 
	ex de,hl			;52ac	eb 	. 
	jr l52c2h		;52ad	18 13 	. . 
	ex de,hl			;52af	eb 	. 
	call sub_6084h		;52b0	cd 84 60 	. . ` 
	ex de,hl			;52b3	eb 	. 
	jr l52c7h		;52b4	18 11 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;52b6	cd 56 60 	. V ` 
	ex de,hl			;52b9	eb 	. 
	nop			;52ba	00 	. 
	nop			;52bb	00 	. 
	ex de,hl			;52bc	eb 	. 
	call sub_6084h		;52bd	cd 84 60 	. . ` 
	jr l52d3h		;52c0	18 11 	. . 
l52c2h:
	call sub_6077h		;52c2	cd 77 60 	. w ` 
	jr l52d3h		;52c5	18 0c 	. . 
l52c7h:
	call sub_6094h		;52c7	cd 94 60 	. . ` 
	jr l52d3h		;52ca	18 07 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;52cc	cd 56 60 	. V ` 
	ex de,hl			;52cf	eb 	. 
	pop hl			;52d0	e1 	. 
	nop			;52d1	00 	. 
	nop			;52d2	00 	. 
l52d3h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;52d3	cd 56 60 	. V ` 
	call l76b8h		;52d6	cd b8 76 	. . v 
	nop			;52d9	00 	. 
	nop			;52da	00 	. 
	ld b,002h		;52db	06 02 	. . 
	ret			;52dd	c9 	. 
sub_52deh:
	call sub_53f1h		;52de	cd f1 53 	. . S 
l52e1h:
	call GET_NEXT_TOKEN		;52e1	cd 30 60 	. 0 ` 
	cp 0fch		;52e4	fe fc 	. . 
	ret nz			;52e6	c0 	. 
	ld de,sub_53f1h		;52e7	11 f1 53 	. . S 
	ld iy,l762dh		;52ea	fd 21 2d 76 	. ! - v 
	call sub_5f02h		;52ee	cd 02 5f 	. . _ 
	inc de			;52f1	13 	. 
	ld d,e			;52f2	53 	S 
	ld a,(de)			;52f3	1a 	. 
	ld d,e			;52f4	53 	S 
	ld (02e53h),hl		;52f5	22 53 2e 	" S . 
	ld d,e			;52f8	53 	S 
	dec (hl)			;52f9	35 	5 
	ld d,e			;52fa	53 	S 
	inc a			;52fb	3c 	< 
	ld d,e			;52fc	53 	S 
	ld c,b			;52fd	48 	H 
	ld d,e			;52fe	53 	S 
	ld a,c			;52ff	79 	y 
	ld d,e			;5300	53 	S 
	ld a,(hl)			;5301	7e 	~ 
	ld d,e			;5302	53 	S 
	sub b			;5303	90 	. 
	ld d,e			;5304	53 	S 
	sbc a,a			;5305	9f 	. 
	ld d,e			;5306	53 	S 
	xor (hl)			;5307	ae 	. 
	ld d,e			;5308	53 	S 
	cp d			;5309	ba 	. 
	ld d,e			;530a	53 	S 
	rst 0			;530b	c7 	. 
	ld d,e			;530c	53 	S 
	call c,0d453h		;530d	dc 53 d4 	. S . 
	ld d,e			;5310	53 	S 
	jr l52e1h		;5311	18 ce 	. . 
	ex de,hl			;5313	eb 	. 
	call l762dh		;5314	cd 2d 76 	. - v 
	ld b,000h		;5317	06 00 	. . 
	ret			;5319	c9 	. 
	ex de,hl			;531a	eb 	. 
	call sub_607ch		;531b	cd 7c 60 	. | ` 
	ex de,hl			;531e	eb 	. 
	jp l5379h		;531f	c3 79 53 	. y S 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5322	cd 56 60 	. V ` 
	ex de,hl			;5325	eb 	. 
	nop			;5326	00 	. 
	nop			;5327	00 	. 
	ex de,hl			;5328	eb 	. 
	call sub_607ch		;5329	cd 7c 60 	. | ` 
	jr l5385h		;532c	18 57 	. W 
	ex de,hl			;532e	eb 	. 
	call sub_6084h		;532f	cd 84 60 	. . ` 
	ex de,hl			;5332	eb 	. 
	jr l5348h		;5333	18 13 	. . 
	ex de,hl			;5335	eb 	. 
	call sub_6084h		;5336	cd 84 60 	. . ` 
	ex de,hl			;5339	eb 	. 
	jr l5379h		;533a	18 3d 	. = 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;533c	cd 56 60 	. V ` 
	ex de,hl			;533f	eb 	. 
	nop			;5340	00 	. 
	nop			;5341	00 	. 
	ex de,hl			;5342	eb 	. 
	call sub_6084h		;5343	cd 84 60 	. . ` 
	jr l5385h		;5346	18 3d 	. = 
l5348h:
	call sub_552bh		;5348	cd 2b 55 	. + U 
	jr c,l5352h		;534b	38 05 	8 . 
	call sub_6077h		;534d	cd 77 60 	. w ` 
	jr l5385h		;5350	18 33 	. 3 
l5352h:
	ld a,h			;5352	7c 	| 
	dec a			;5353	3d 	= 
	or l			;5354	b5 	. 
	jr z,l536ch		;5355	28 15 	( . 
	ld b,(iy+020h)		;5357	fd 46 20 	. F   
	inc b			;535a	04 	. 
	dec b			;535b	05 	. 
	jr z,l5369h		;535c	28 0b 	( . 
l535eh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;535e	cd 56 60 	. V ` 
	sra h		;5361	cb 2c 	. , 
	rr l		;5363	cb 1d 	. . 
	nop			;5365	00 	. 
	nop			;5366	00 	. 
	djnz l535eh		;5367	10 f5 	. . 
l5369h:
	ld b,002h		;5369	06 02 	. . 
	ret			;536b	c9 	. 
l536ch:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;536c	cd 56 60 	. V ` 
	ld l,h			;536f	6c 	l 
	rl h		;5370	cb 14 	. . 
	sbc a,a			;5372	9f 	. 
	ld h,a			;5373	67 	g 
	nop			;5374	00 	. 
	nop			;5375	00 	. 
	ld b,002h		;5376	06 02 	. . 
	ret			;5378	c9 	. 
l5379h:
	call sub_6094h		;5379	cd 94 60 	. . ` 
	jr l5385h		;537c	18 07 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;537e	cd 56 60 	. V ` 
	ex de,hl			;5381	eb 	. 
	pop hl			;5382	e1 	. 
	nop			;5383	00 	. 
	nop			;5384	00 	. 
l5385h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5385	cd 56 60 	. V ` 
	call l762dh		;5388	cd 2d 76 	. - v 
	nop			;538b	00 	. 
	nop			;538c	00 	. 
	ld b,002h		;538d	06 02 	. . 
	ret			;538f	c9 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5390	cd 56 60 	. V ` 
	call sub_784fh		;5393	cd 4f 78 	. O x 
	ex de,hl			;5396	eb 	. 
	nop			;5397	00 	. 
	nop			;5398	00 	. 
	ex de,hl			;5399	eb 	. 
	call sub_607ch		;539a	cd 7c 60 	. | ` 
	jr l53e7h		;539d	18 48 	. H 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;539f	cd 56 60 	. V ` 
	call sub_784fh		;53a2	cd 4f 78 	. O x 
	ex de,hl			;53a5	eb 	. 
	nop			;53a6	00 	. 
	nop			;53a7	00 	. 
	ex de,hl			;53a8	eb 	. 
	call sub_6084h		;53a9	cd 84 60 	. . ` 
	jr l53e7h		;53ac	18 39 	. 9 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;53ae	cd 56 60 	. V ` 
	call sub_784fh		;53b1	cd 4f 78 	. O x 
	ex de,hl			;53b4	eb 	. 
	pop hl			;53b5	e1 	. 
	nop			;53b6	00 	. 
	nop			;53b7	00 	. 
	jr l53e7h		;53b8	18 2d 	. - 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;53ba	cd 56 60 	. V ` 
	call sub_784fh		;53bd	cd 4f 78 	. O x 
	nop			;53c0	00 	. 
	nop			;53c1	00 	. 
	call sub_6077h		;53c2	cd 77 60 	. w ` 
	jr l53e7h		;53c5	18 20 	.   
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;53c7	cd 56 60 	. V ` 
	call sub_784fh		;53ca	cd 4f 78 	. O x 
	nop			;53cd	00 	. 
	nop			;53ce	00 	. 
	call sub_6094h		;53cf	cd 94 60 	. . ` 
	jr l53e7h		;53d2	18 13 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;53d4	cd 56 60 	. V ` 
	call sub_784fh		;53d7	cd 4f 78 	. O x 
	nop			;53da	00 	. 
	nop			;53db	00 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;53dc	cd 56 60 	. V ` 
	pop bc			;53df	c1 	. 
	ex (sp),hl			;53e0	e3 	. 
	call sub_784fh		;53e1	cd 4f 78 	. O x 
	pop de			;53e4	d1 	. 
	nop			;53e5	00 	. 
	nop			;53e6	00 	. 
l53e7h:
	ld b,002h		;53e7	06 02 	. . 
l53e9h:
	push iy		;53e9	fd e5 	. . 
	pop hl			;53eb	e1 	. 
l53ech:
	ld a,0cdh		;53ec	3e cd 	> . 
	jp WRITE_OPCODE_A_L_H		;53ee	c3 a9 60 	. . ` 
sub_53f1h:
	call sub_5620h		;53f1	cd 20 56 	.   V 
l53f4h:
	ld de,sub_5620h		;53f4	11 20 56 	.   V 
	call GET_NEXT_TOKEN		;53f7	cd 30 60 	. 0 ` 
	cp 0f4h		;53fa	fe f4 	. . 
	jr z,l542ah		;53fc	28 2c 	( , 
	cp 0f3h		;53fe	fe f3 	. . 
	ret nz			;5400	c0 	. 
	ld iy,l7732h		;5401	fd 21 32 77 	. ! 2 w 
	call sub_5f02h		;5405	cd 02 5f 	. . _ 
	rst 8			;5408	cf 	. 
	ld d,h			;5409	54 	T 
	push de			;540a	d5 	. 
	ld d,h			;540b	54 	T 
	ret c			;540c	d8 	. 
	ld d,h			;540d	54 	T 
	call c,0e354h		;540e	dc 54 e3 	. T . 
	ld d,h			;5411	54 	T 
	jp (hl)			;5412	e9 	. 
	ld d,h			;5413	54 	T 
	call pe,01554h		;5414	ec 54 15 	. T . 
	ld d,l			;5417	55 	U 
	ld a,(de)			;5418	1a 	. 
	ld d,l			;5419	55 	U 
	rst 18h			;541a	df 	. 
	ld h,b			;541b	60 	` 
	rst 28h			;541c	ef 	. 
	ld h,b			;541d	60 	` 
	call m,00760h		;541e	fc 60 07 	. ` . 
	ld h,c			;5421	61 	a 
	add hl,de			;5422	19 	. 
	ld h,c			;5423	61 	a 
	inc e			;5424	1c 	. 
	ld h,c			;5425	61 	a 
	rra			;5426	1f 	. 
	ld h,c			;5427	61 	a 
	jr l53f4h		;5428	18 ca 	. . 
l542ah:
	ld iy,l7775h		;542a	fd 21 75 77 	. ! u w 
	call sub_5f02h		;542e	cd 02 5f 	. . _ 
	ld d,e			;5431	53 	S 
	ld d,h			;5432	54 	T 
	ld h,a			;5433	67 	g 
	ld d,h			;5434	54 	T 
	ld l,d			;5435	6a 	j 
	ld d,h			;5436	54 	T 
	ld a,l			;5437	7d 	} 
	ld d,h			;5438	54 	T 
	sub a			;5439	97 	. 
	ld d,h			;543a	54 	T 
	sbc a,d			;543b	9a 	. 
	ld d,h			;543c	54 	T 
	add a,d			;543d	82 	. 
	ld d,h			;543e	54 	T 
	xor d			;543f	aa 	. 
	ld d,h			;5440	54 	T 
	xor l			;5441	ad 	. 
	ld d,h			;5442	54 	T 
	rst 18h			;5443	df 	. 
	ld h,b			;5444	60 	` 
	rst 28h			;5445	ef 	. 
	ld h,b			;5446	60 	` 
	call m,00760h		;5447	fc 60 07 	. ` . 
	ld h,c			;544a	61 	a 
	add hl,hl			;544b	29 	) 
	ld h,c			;544c	61 	a 
	inc l			;544d	2c 	, 
	ld h,c			;544e	61 	a 
	cpl			;544f	2f 	/ 
	ld h,c			;5450	61 	a 
	jr l53f4h		;5451	18 a1 	. . 
	push de			;5453	d5 	. 
	call TOKEN_MULTI_FUNCTION		;5454	cd 2d 78 	. - x 
	pop de			;5457	d1 	. 
	push hl			;5458	e5 	. 
	push bc			;5459	c5 	. 
	ex de,hl			;545a	eb 	. 
	call TOKEN_MULTI_FUNCTION		;545b	cd 2d 78 	. - x 
	pop de			;545e	d1 	. 
	ld c,d			;545f	4a 	J 
	pop de			;5460	d1 	. 
	call l7775h		;5461	cd 75 77 	. u w 
	jp l4fcah		;5464	c3 ca 4f 	. . O 
	call sub_6084h		;5467	cd 84 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;546a	cd 56 60 	. V ` 
	call TOKEN_MULTI_FUNCTION		;546d	cd 2d 78 	. - x 
	ld c,b			;5470	48 	H 
	ex de,hl			;5471	eb 	. 
	nop			;5472	00 	. 
	nop			;5473	00 	. 
	ex de,hl			;5474	eb 	. 
	call TOKEN_MULTI_FUNCTION		;5475	cd 2d 78 	. - x 
	call l4fcah		;5478	cd ca 4f 	. . O 
	jr l54c4h		;547b	18 47 	. G 
	ex de,hl			;547d	eb 	. 
	call sub_6084h		;547e	cd 84 60 	. . ` 
	ex de,hl			;5481	eb 	. 
	call sub_613eh		;5482	cd 3e 61 	. > a 
	call TOKEN_MULTI_FUNCTION		;5485	cd 2d 78 	. - x 
	ld a,00eh		;5488	3e 0e 	> . 
	call WRITE_OPCODE		;548a	cd b7 60 	. . ` 
	ld a,b			;548d	78 	x 
	call WRITE_OPCODE		;548e	cd b7 60 	. . ` 
	and a			;5491	a7 	. 
	call nz,sub_6077h		;5492	c4 77 60 	. w ` 
	jr l54c4h		;5495	18 2d 	. - 
	call sub_6084h		;5497	cd 84 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;549a	cd 56 60 	. V ` 
	call TOKEN_MULTI_FUNCTION		;549d	cd 2d 78 	. - x 
	push hl			;54a0	e5 	. 
	push bc			;54a1	c5 	. 
	nop			;54a2	00 	. 
	nop			;54a3	00 	. 
	ex de,hl			;54a4	eb 	. 
	call sub_6084h		;54a5	cd 84 60 	. . ` 
	jr l54b9h		;54a8	18 0f 	. . 
	call sub_6084h		;54aa	cd 84 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;54ad	cd 56 60 	. V ` 
	call TOKEN_MULTI_FUNCTION		;54b0	cd 2d 78 	. - x 
	pop de			;54b3	d1 	. 
	push hl			;54b4	e5 	. 
	push bc			;54b5	c5 	. 
	ex de,hl			;54b6	eb 	. 
	nop			;54b7	00 	. 
	nop			;54b8	00 	. 
l54b9h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;54b9	cd 56 60 	. V ` 
	call TOKEN_MULTI_FUNCTION		;54bc	cd 2d 78 	. - x 
	pop de			;54bf	d1 	. 
	ld c,d			;54c0	4a 	J 
	pop de			;54c1	d1 	. 
	nop			;54c2	00 	. 
	nop			;54c3	00 	. 
l54c4h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;54c4	cd 56 60 	. V ` 
	call l7775h		;54c7	cd 75 77 	. u w 
	nop			;54ca	00 	. 
	nop			;54cb	00 	. 
	ld b,003h		;54cc	06 03 	. . 
	ret			;54ce	c9 	. 
	call sub_761bh		;54cf	cd 1b 76 	. . v 
	ld b,000h		;54d2	06 00 	. . 
	ret			;54d4	c9 	. 
	ex de,hl			;54d5	eb 	. 
	jr l54dch		;54d6	18 04 	. . 
	ex de,hl			;54d8	eb 	. 
	jp l54ech		;54d9	c3 ec 54 	. . T 
l54dch:
	ex de,hl			;54dc	eb 	. 
	call sub_6084h		;54dd	cd 84 60 	. . ` 
	ex de,hl			;54e0	eb 	. 
	jr l54ech		;54e1	18 09 	. . 
	call sub_6084h		;54e3	cd 84 60 	. . ` 
	ex de,hl			;54e6	eb 	. 
	jr l5515h		;54e7	18 2c 	. , 
	ex de,hl			;54e9	eb 	. 
	jr l5515h		;54ea	18 29 	. ) 
l54ech:
	call sub_5585h		;54ec	cd 85 55 	. . U 
	jr c,l54fbh		;54ef	38 0a 	8 . 
	call sub_552bh		;54f1	cd 2b 55 	. + U 
	jr c,l5507h		;54f4	38 11 	8 . 
	call sub_6077h		;54f6	cd 77 60 	. w ` 
	jr l5520h		;54f9	18 25 	. % 
l54fbh:
	ld a,(hl)			;54fb	7e 	~ 
	cp 0ffh		;54fc	fe ff 	. . 
	ld b,002h		;54fe	06 02 	. . 
	ret z			;5500	c8 	. 
	call WRITE_OPCODE		;5501	cd b7 60 	. . ` 
	inc hl			;5504	23 	# 
	jr l54fbh		;5505	18 f4 	. . 
l5507h:
	ld b,(iy+020h)		;5507	fd 46 20 	. F   
l550ah:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;550a	cd 56 60 	. V ` 
	add hl,hl			;550d	29 	) 
	nop			;550e	00 	. 
	nop			;550f	00 	. 
	djnz l550ah		;5510	10 f8 	. . 
	ld b,002h		;5512	06 02 	. . 
	ret			;5514	c9 	. 
l5515h:
	call sub_6094h		;5515	cd 94 60 	. . ` 
	jr l5520h		;5518	18 06 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;551a	cd 56 60 	. V ` 
	pop de			;551d	d1 	. 
	nop			;551e	00 	. 
	nop			;551f	00 	. 
l5520h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5520	cd 56 60 	. V ` 
	call sub_761bh		;5523	cd 1b 76 	. . v 
	nop			;5526	00 	. 
	nop			;5527	00 	. 
	ld b,002h		;5528	06 02 	. . 
	ret			;552a	c9 	. 
sub_552bh:
	ld iy,l5545h		;552b	fd 21 45 55 	. ! E U 
	ld b,010h		;552f	06 10 	. . 
l5531h:
	ld a,l			;5531	7d 	} 
	cp (iy+000h)		;5532	fd be 00 	. . . 
	jr nz,l553dh		;5535	20 06 	  . 
	ld a,h			;5537	7c 	| 
	cp (iy+001h)		;5538	fd be 01 	. . . 
	scf			;553b	37 	7 
	ret z			;553c	c8 	. 
l553dh:
	inc iy		;553d	fd 23 	. # 
	inc iy		;553f	fd 23 	. # 
	djnz l5531h		;5541	10 ee 	. . 
	and a			;5543	a7 	. 
	ret			;5544	c9 	. 
l5545h:
	ld bc,00200h		;5545	01 00 02 	. . . 
	nop			;5548	00 	. 
	inc b			;5549	04 	. 
	nop			;554a	00 	. 
	ex af,af'			;554b	08 	. 
	nop			;554c	00 	. 
	djnz l554fh		;554d	10 00 	. . 
l554fh:
	jr nz,l5551h		;554f	20 00 	  . 
l5551h:
	ld b,b			;5551	40 	@ 
	nop			;5552	00 	. 
	add a,b			;5553	80 	. 
	nop			;5554	00 	. 
	nop			;5555	00 	. 
	ld bc,00200h		;5556	01 00 02 	. . . 
	nop			;5559	00 	. 
	inc b			;555a	04 	. 
	nop			;555b	00 	. 
	ex af,af'			;555c	08 	. 
	nop			;555d	00 	. 
	djnz l5560h		;555e	10 00 	. . 
l5560h:
	jr nz,l5562h		;5560	20 00 	  . 
l5562h:
	ld b,b			;5562	40 	@ 
	nop			;5563	00 	. 
	add a,b			;5564	80 	. 
	nop			;5565	00 	. 
	nop			;5566	00 	. 
	ld bc,00200h		;5567	01 00 02 	. . . 
	nop			;556a	00 	. 
	inc bc			;556b	03 	. 
	nop			;556c	00 	. 
	inc b			;556d	04 	. 
	nop			;556e	00 	. 
	dec b			;556f	05 	. 
	nop			;5570	00 	. 
	ld b,000h		;5571	06 00 	. . 
	rlca			;5573	07 	. 
	nop			;5574	00 	. 
	ex af,af'			;5575	08 	. 
	nop			;5576	00 	. 
	add hl,bc			;5577	09 	. 
	nop			;5578	00 	. 
	ld a,(bc)			;5579	0a 	. 
	nop			;557a	00 	. 
	dec bc			;557b	0b 	. 
	nop			;557c	00 	. 
	inc c			;557d	0c 	. 
	nop			;557e	00 	. 
	dec c			;557f	0d 	. 
	nop			;5580	00 	. 
	ld c,000h		;5581	0e 00 	. . 
	rrca			;5583	0f 	. 
	nop			;5584	00 	. 
sub_5585h:
	ld iy,l55aah		;5585	fd 21 aa 55 	. ! . U 
l5589h:
	ld a,(iy+000h)		;5589	fd 7e 00 	. ~ . 
	or (iy+001h)		;558c	fd b6 01 	. . . 
	ret z			;558f	c8 	. 
	ld a,(iy+000h)		;5590	fd 7e 00 	. ~ . 
	cp l			;5593	bd 	. 
	jr nz,l559ch		;5594	20 06 	  . 
	ld a,(iy+001h)		;5596	fd 7e 01 	. ~ . 
	cp h			;5599	bc 	. 
	jr z,l55a2h		;559a	28 06 	( . 
l559ch:
	inc iy		;559c	fd 23 	. # 
	inc iy		;559e	fd 23 	. # 
	jr l5589h		;55a0	18 e7 	. . 
l55a2h:
	ld l,(iy+022h)		;55a2	fd 6e 22 	. n " 
	ld h,(iy+023h)		;55a5	fd 66 23 	. f # 
	scf			;55a8	37 	7 
	ret			;55a9	c9 	. 
l55aah:
	ld bc,00300h		;55aa	01 00 03 	. . . 
	nop			;55ad	00 	. 
	dec b			;55ae	05 	. 
	nop			;55af	00 	. 
	ld b,000h		;55b0	06 00 	. . 
	rlca			;55b2	07 	. 
	nop			;55b3	00 	. 
	add hl,bc			;55b4	09 	. 
	nop			;55b5	00 	. 
	ld a,(bc)			;55b6	0a 	. 
	nop			;55b7	00 	. 
	inc d			;55b8	14 	. 
	nop			;55b9	00 	. 
	add hl,de			;55ba	19 	. 
	nop			;55bb	00 	. 
	jr z,l55beh		;55bc	28 00 	( . 
l55beh:
	ld (sub_4ffeh+2),a		;55be	32 00 50 	2 . P 
	nop			;55c1	00 	. 
	ld h,h			;55c2	64 	d 
	nop			;55c3	00 	. 
	ret z			;55c4	c8 	. 
	nop			;55c5	00 	. 
	nop			;55c6	00 	. 
	ld bc,BIOS_GSPSIZ2		;55c7	01 01 01 	. . . 
	nop			;55ca	00 	. 
	nop			;55cb	00 	. 
	pop af			;55cc	f1 	. 
	ld d,l			;55cd	55 	U 
	defb 0edh;next byte illegal after ed		;55ce	ed 	. 
	ld d,l			;55cf	55 	U 
	or 055h		;55d0	f6 55 	. U 
	call pe,0fc55h		;55d2	ec 55 fc 	. U . 
	ld d,l			;55d5	55 	U 
	inc bc			;55d6	03 	. 
	ld d,(hl)			;55d7	56 	V 
	push af			;55d8	f5 	. 
	ld d,l			;55d9	55 	U 
	call p,00d55h		;55da	f4 55 0d 	. U . 
	ld d,(hl)			;55dd	56 	V 
	di			;55de	f3 	. 
	ld d,l			;55df	55 	U 
	inc c			;55e0	0c 	. 
	ld d,(hl)			;55e1	56 	V 
	jp p,00b55h		;55e2	f2 55 0b 	. U . 
	ld d,(hl)			;55e5	56 	V 
	ld a,(bc)			;55e6	0a 	. 
	ld d,(hl)			;55e7	56 	V 
	jr l5640h		;55e8	18 56 	. V 
	inc e			;55ea	1c 	. 
	ld d,(hl)			;55eb	56 	V 
	add hl,hl			;55ec	29 	) 
	ld e,l			;55ed	5d 	] 
	ld d,h			;55ee	54 	T 
	add hl,hl			;55ef	29 	) 
	add hl,de			;55f0	19 	. 
	rst 38h			;55f1	ff 	. 
	add hl,hl			;55f2	29 	) 
	add hl,hl			;55f3	29 	) 
	add hl,hl			;55f4	29 	) 
	add hl,hl			;55f5	29 	) 
	ld e,l			;55f6	5d 	] 
	ld d,h			;55f7	54 	T 
	add hl,hl			;55f8	29 	) 
	add hl,hl			;55f9	29 	) 
	add hl,de			;55fa	19 	. 
	rst 38h			;55fb	ff 	. 
	ld e,l			;55fc	5d 	] 
	ld d,h			;55fd	54 	T 
	add hl,hl			;55fe	29 	) 
	add hl,de			;55ff	19 	. 
	add hl,hl			;5600	29 	) 
	add hl,de			;5601	19 	. 
	rst 38h			;5602	ff 	. 
	ld e,l			;5603	5d 	] 
	ld d,h			;5604	54 	T 
	add hl,hl			;5605	29 	) 
	add hl,hl			;5606	29 	) 
	add hl,hl			;5607	29 	) 
	add hl,de			;5608	19 	. 
	rst 38h			;5609	ff 	. 
	add hl,hl			;560a	29 	) 
	add hl,hl			;560b	29 	) 
	add hl,hl			;560c	29 	) 
	ld e,l			;560d	5d 	] 
	ld d,h			;560e	54 	T 
	add hl,hl			;560f	29 	) 
	add hl,hl			;5610	29 	) 
	add hl,de			;5611	19 	. 
	ld e,l			;5612	5d 	] 
	ld d,h			;5613	54 	T 
	add hl,hl			;5614	29 	) 
	add hl,hl			;5615	29 	) 
	add hl,de			;5616	19 	. 
	rst 38h			;5617	ff 	. 
	ld h,l			;5618	65 	e 
	ld l,000h		;5619	2e 00 	. . 
	rst 38h			;561b	ff 	. 
	ld a,l			;561c	7d 	} 
	add a,h			;561d	84 	. 
	ld h,a			;561e	67 	g 
	rst 38h			;561f	ff 	. 
sub_5620h:
	call sub_56eah		;5620	cd ea 56 	. . V 
l5623h:
	call GET_NEXT_TOKEN		;5623	cd 30 60 	. 0 ` 
	cp 0f5h		;5626	fe f5 	. . 
	ret nz			;5628	c0 	. 
	ld de,sub_56eah		;5629	11 ea 56 	. . V 
	ld iy,l780dh		;562c	fd 21 0d 78 	. ! . x 
	call sub_5f02h		;5630	cd 02 5f 	. . _ 
	ld d,l			;5633	55 	U 
	ld d,(hl)			;5634	56 	V 
	ld h,h			;5635	64 	d 
	ld d,(hl)			;5636	56 	V 
	ld (hl),b			;5637	70 	p 
	ld d,(hl)			;5638	56 	V 
	ld a,a			;5639	7f 	 
	ld d,(hl)			;563a	56 	V 
	add a,(hl)			;563b	86 	. 
	ld d,(hl)			;563c	56 	V 
	adc a,l			;563d	8d 	. 
	ld d,(hl)			;563e	56 	V 
	and d			;563f	a2 	. 
l5640h:
	ld d,(hl)			;5640	56 	V 
	xor d			;5641	aa 	. 
	ld d,(hl)			;5642	56 	V 
	or d			;5643	b2 	. 
	ld d,(hl)			;5644	56 	V 
	rst 18h			;5645	df 	. 
	ld h,b			;5646	60 	` 
	rst 28h			;5647	ef 	. 
	ld h,b			;5648	60 	` 
	call m,0c760h		;5649	fc 60 c7 	. ` . 
	ld d,(hl)			;564c	56 	V 
	call z,0d756h		;564d	cc 56 d7 	. V . 
	ld d,(hl)			;5650	56 	V 
	cpl			;5651	2f 	/ 
	ld h,c			;5652	61 	a 
	jr l5623h		;5653	18 ce 	. . 
	push hl			;5655	e5 	. 
	ex de,hl			;5656	eb 	. 
	call TOKEN_MULTI_FUNCTION		;5657	cd 2d 78 	. - x 
	pop de			;565a	d1 	. 
	call sub_77c1h		;565b	cd c1 77 	. . w 
	call l4fcah		;565e	cd ca 4f 	. . O 
	ld b,003h		;5661	06 03 	. . 
	ret			;5663	c9 	. 
	push hl			;5664	e5 	. 
	ex de,hl			;5665	eb 	. 
	call TOKEN_MULTI_FUNCTION		;5666	cd 2d 78 	. - x 
	call l4fcah		;5669	cd ca 4f 	. . O 
	pop hl			;566c	e1 	. 
	jp l56adh		;566d	c3 ad 56 	. . V 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5670	cd 56 60 	. V ` 
	ex de,hl			;5673	eb 	. 
	nop			;5674	00 	. 
	nop			;5675	00 	. 
	ex de,hl			;5676	eb 	. 
	call TOKEN_MULTI_FUNCTION		;5677	cd 2d 78 	. - x 
	call l4fcah		;567a	cd ca 4f 	. . O 
	jr l56bch		;567d	18 3d 	. = 
	ex de,hl			;567f	eb 	. 
	call sub_6084h		;5680	cd 84 60 	. . ` 
	ex de,hl			;5683	eb 	. 
	jr l56a2h		;5684	18 1c 	. . 
	ex de,hl			;5686	eb 	. 
	call sub_6084h		;5687	cd 84 60 	. . ` 
	ex de,hl			;568a	eb 	. 
	jr l56aah		;568b	18 1d 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;568d	cd 56 60 	. V ` 
	push hl			;5690	e5 	. 
	nop			;5691	00 	. 
	nop			;5692	00 	. 
	ex de,hl			;5693	eb 	. 
	call sub_6084h		;5694	cd 84 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5697	cd 56 60 	. V ` 
	call TOKEN_MULTI_FUNCTION		;569a	cd 2d 78 	. - x 
	pop de			;569d	d1 	. 
	nop			;569e	00 	. 
	nop			;569f	00 	. 
	jr l56bch		;56a0	18 1a 	. . 
l56a2h:
	call sub_613eh		;56a2	cd 3e 61 	. > a 
	call sub_6077h		;56a5	cd 77 60 	. w ` 
	jr l56bch		;56a8	18 12 	. . 
l56aah:
	call sub_613eh		;56aa	cd 3e 61 	. > a 
l56adh:
	call sub_6094h		;56ad	cd 94 60 	. . ` 
	jr l56bch		;56b0	18 0a 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;56b2	cd 56 60 	. V ` 
	ex (sp),hl			;56b5	e3 	. 
	call TOKEN_MULTI_FUNCTION		;56b6	cd 2d 78 	. - x 
	pop de			;56b9	d1 	. 
	nop			;56ba	00 	. 
	nop			;56bb	00 	. 
l56bch:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;56bc	cd 56 60 	. V ` 
	call sub_77c1h		;56bf	cd c1 77 	. . w 
	nop			;56c2	00 	. 
	nop			;56c3	00 	. 
	ld b,003h		;56c4	06 03 	. . 
	ret			;56c6	c9 	. 
	call sub_6077h		;56c7	cd 77 60 	. w ` 
	jr l56dfh		;56ca	18 13 	. . 
	call sub_60d5h		;56cc	cd d5 60 	. . ` 
	call sub_60d5h		;56cf	cd d5 60 	. . ` 
	call sub_6094h		;56d2	cd 94 60 	. . ` 
	jr l56dfh		;56d5	18 08 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;56d7	cd 56 60 	. V ` 
	ex de,hl			;56da	eb 	. 
	pop bc			;56db	c1 	. 
	pop hl			;56dc	e1 	. 
	nop			;56dd	00 	. 
	nop			;56de	00 	. 
l56dfh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;56df	cd 56 60 	. V ` 
	call sub_77c1h		;56e2	cd c1 77 	. . w 
	nop			;56e5	00 	. 
	nop			;56e6	00 	. 
	ld b,003h		;56e7	06 03 	. . 
	ret			;56e9	c9 	. 
sub_56eah:
	ld hl,(0f573h)		;56ea	2a 73 f5 	* s . 
	ld bc,0008ch		;56ed	01 8c 00 	. . . 
	add hl,bc			;56f0	09 	. 
	sbc hl,sp		;56f1	ed 72 	. r 
	jp nc,04e1fh		;56f3	d2 1f 4e 	. . N 
	call GET_NEXT_TOKEN		;56f6	cd 30 60 	. 0 ` 
	inc ix		;56f9	dd 23 	. # 
	cp 028h		;56fb	fe 28 	. ( 
	jr nz,l5705h		;56fd	20 06 	  . 
	call sub_5012h		;56ff	cd 12 50 	. . P 
	jp ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5702	c3 46 60 	. F ` 
l5705h:
	ld hl,VAR_NX		;5705	21 6a f5 	! j . 
	bit 0,(hl)		;5708	cb 46 	. F 
	res 0,(hl)		;570a	cb 86 	. . 
	jr z,l5714h		;570c	28 06 	( . 
	call l5714h		;570e	cd 14 57 	. . W 
	jp l4fb6h		;5711	c3 b6 4f 	. . O 
l5714h:
	cp 011h		;5714	fe 11 	. . 
	jr c,l5723h		;5716	38 0b 	8 . 
	cp 01bh		;5718	fe 1b 	. . 
	jr nc,l5723h		;571a	30 07 	0 . 
	sub 011h		;571c	d6 11 	. . 
	ld l,a			;571e	6f 	o 
	ld h,000h		;571f	26 00 	& . 
	ld b,h			;5721	44 	D 
	ret			;5722	c9 	. 
l5723h:
	cp 00fh		;5723	fe 0f 	. . 
	jr nz,l5730h		;5725	20 09 	  . 
	ld l,(ix+000h)		;5727	dd 6e 00 	. n . 
	inc ix		;572a	dd 23 	. # 
	ld h,000h		;572c	26 00 	& . 
	ld b,h			;572e	44 	D 
	ret			;572f	c9 	. 
l5730h:
	cp 01ch		;5730	fe 1c 	. . 
	jr z,l573ch		;5732	28 08 	( . 
	cp 00ch		;5734	fe 0c 	. . 
	jr z,l573ch		;5736	28 04 	( . 
	cp 00bh		;5738	fe 0b 	. . 
	jr nz,l5749h		;573a	20 0d 	  . 
l573ch:
	ld l,(ix+000h)		;573c	dd 6e 00 	. n . 
	ld h,(ix+001h)		;573f	dd 66 01 	. f . 
	inc ix		;5742	dd 23 	. # 
	inc ix		;5744	dd 23 	. # 
	ld b,000h		;5746	06 00 	. . 
	ret			;5748	c9 	. 
l5749h:
	cp 026h		;5749	fe 26 	. & 
	jr nz,l5768h		;574b	20 1b 	  . 
	ld a,(ix+000h)		;574d	dd 7e 00 	. ~ . 
	cp 042h		;5750	fe 42 	. B 
	jp nz,04e16h		;5752	c2 16 4e 	. . N 
	ld b,000h		;5755	06 00 	. . 
	ld l,b			;5757	68 	h 
	ld h,b			;5758	60 	` 
l5759h:
	inc ix		;5759	dd 23 	. # 
	ld a,(ix+000h)		;575b	dd 7e 00 	. ~ . 
	sub 030h		;575e	d6 30 	. 0 
	cp 002h		;5760	fe 02 	. . 
	ret nc			;5762	d0 	. 
	add hl,hl			;5763	29 	) 
	or l			;5764	b5 	. 
	ld l,a			;5765	6f 	o 
	jr l5759h		;5766	18 f1 	. . 
l5768h:
	cp 030h		;5768	fe 30 	. 0 
	jr c,l578ch		;576a	38 20 	8   
	cp 03ah		;576c	fe 3a 	. : 
	jr nc,l578ch		;576e	30 1c 	0 . 
	ld b,000h		;5770	06 00 	. . 
	sub 030h		;5772	d6 30 	. 0 
	ld l,a			;5774	6f 	o 
	ld h,b			;5775	60 	` 
l5776h:
	call GET_NEXT_TOKEN		;5776	cd 30 60 	. 0 ` 
	sub 030h		;5779	d6 30 	. 0 
	cp 00ah		;577b	fe 0a 	. . 
	ret nc			;577d	d0 	. 
	ld e,l			;577e	5d 	] 
	ld d,h			;577f	54 	T 
	add hl,hl			;5780	29 	) 
	add hl,hl			;5781	29 	) 
	add hl,de			;5782	19 	. 
	add hl,hl			;5783	29 	) 
	ld e,a			;5784	5f 	_ 
	ld d,000h		;5785	16 00 	. . 
	add hl,de			;5787	19 	. 
	inc ix		;5788	dd 23 	. # 
	jr l5776h		;578a	18 ea 	. . 
l578ch:
	cp 01dh		;578c	fe 1d 	. . 
	ld c,003h		;578e	0e 03 	. . 
	jr z,l5799h		;5790	28 07 	( . 
	cp 01fh		;5792	fe 1f 	. . 
	ld c,007h		;5794	0e 07 	. . 
	jp nz,l5835h		;5796	c2 35 58 	. 5 X 
l5799h:
	ld b,000h		;5799	06 00 	. . 
	ld a,(ix+000h)		;579b	dd 7e 00 	. ~ . 
	add ix,bc		;579e	dd 09 	. . 
	and a			;57a0	a7 	. 
	jr z,l57ech		;57a1	28 49 	( I 
	push ix		;57a3	dd e5 	. . 
	ld a,c			;57a5	79 	y 
l57a6h:
	push af			;57a6	f5 	. 
	ld a,(ix+000h)		;57a7	dd 7e 00 	. ~ . 
	call sub_57f8h		;57aa	cd f8 57 	. . W 
	ld a,(ix+000h)		;57ad	dd 7e 00 	. ~ . 
	rrca			;57b0	0f 	. 
	rrca			;57b1	0f 	. 
	rrca			;57b2	0f 	. 
	rrca			;57b3	0f 	. 
	call sub_57f8h		;57b4	cd f8 57 	. . W 
	pop af			;57b7	f1 	. 
	dec ix		;57b8	dd 2b 	. + 
	dec a			;57ba	3d 	= 
	jr nz,l57a6h		;57bb	20 e9 	  . 
	ld a,(ix+000h)		;57bd	dd 7e 00 	. ~ . 
	pop ix		;57c0	dd e1 	. . 
	inc ix		;57c2	dd 23 	. # 
	and a			;57c4	a7 	. 
	jp p,l57cch		;57c5	f2 cc 57 	. . W 
	set 7,h		;57c8	cb fc 	. . 
	and 07fh		;57ca	e6 7f 	.  
l57cch:
	cp 040h		;57cc	fe 40 	. @ 
	jp z,l4fcah		;57ce	ca ca 4f 	. . O 
	push af			;57d1	f5 	. 
	jr nc,l57e0h		;57d2	30 0c 	0 . 
	ld c,07dh		;57d4	0e 7d 	. } 
	ld de,04ccch		;57d6	11 cc 4c 	. . L 
	call l7732h		;57d9	cd 32 77 	. 2 w 
	pop af			;57dc	f1 	. 
	inc a			;57dd	3c 	< 
	jr l57cch		;57de	18 ec 	. . 
l57e0h:
	ld c,084h		;57e0	0e 84 	. . 
	ld de,02000h		;57e2	11 00 20 	. .   
	call l7732h		;57e5	cd 32 77 	. 2 w 
	pop af			;57e8	f1 	. 
	dec a			;57e9	3d 	= 
	jr l57cch		;57ea	18 e0 	. . 
l57ech:
	inc ix		;57ec	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;57ee	cd 56 60 	. V ` 
	ld b,000h		;57f1	06 00 	. . 
	nop			;57f3	00 	. 
	nop			;57f4	00 	. 
l57f5h:
	ld b,003h		;57f5	06 03 	. . 
	ret			;57f7	c9 	. 
sub_57f8h:
	and 00fh		;57f8	e6 0f 	. . 
	push hl			;57fa	e5 	. 
	push bc			;57fb	c5 	. 
	ld c,a			;57fc	4f 	O 
	ld b,000h		;57fd	06 00 	. . 
	ld hl,l5817h		;57ff	21 17 58 	! . X 
	add hl,bc			;5802	09 	. 
	add hl,bc			;5803	09 	. 
	add hl,bc			;5804	09 	. 
	pop bc			;5805	c1 	. 
	ld c,(hl)			;5806	4e 	N 
	inc hl			;5807	23 	# 
	ld d,(hl)			;5808	56 	V 
	inc hl			;5809	23 	# 
	ld e,(hl)			;580a	5e 	^ 
	pop hl			;580b	e1 	. 
	call sub_76c1h		;580c	cd c1 76 	. . v 
	ld c,07dh		;580f	0e 7d 	. } 
	ld de,04ccch		;5811	11 cc 4c 	. . L 
	jp l7732h		;5814	c3 32 77 	. 2 w 
l5817h:
	nop			;5817	00 	. 
	nop			;5818	00 	. 
	nop			;5819	00 	. 
	add a,c			;581a	81 	. 
	nop			;581b	00 	. 
	nop			;581c	00 	. 
	add a,d			;581d	82 	. 
	nop			;581e	00 	. 
	nop			;581f	00 	. 
	add a,d			;5820	82 	. 
	ld b,b			;5821	40 	@ 
	nop			;5822	00 	. 
	add a,e			;5823	83 	. 
	nop			;5824	00 	. 
	nop			;5825	00 	. 
	add a,e			;5826	83 	. 
	jr nz,l5829h		;5827	20 00 	  . 
l5829h:
	add a,e			;5829	83 	. 
	ld b,b			;582a	40 	@ 
	nop			;582b	00 	. 
	add a,e			;582c	83 	. 
	ld h,b			;582d	60 	` 
	nop			;582e	00 	. 
	add a,h			;582f	84 	. 
	nop			;5830	00 	. 
	nop			;5831	00 	. 
	add a,h			;5832	84 	. 
	djnz l5835h		;5833	10 00 	. . 
l5835h:
	cp 022h		;5835	fe 22 	. " 
	jr nz,l586dh		;5837	20 34 	  4 
sub_5839h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5839	cd 56 60 	. V ` 
l583ch:
	jp l583ch		;583c	c3 3c 58 	. < X 
	rst 38h			;583f	ff 	. 
	nop			;5840	00 	. 
	nop			;5841	00 	. 
	ld hl,(0f560h)		;5842	2a 60 f5 	* ` . 
	push hl			;5845	e5 	. 
	ld e,000h		;5846	1e 00 	. . 
l5848h:
	ld a,(ix+000h)		;5848	dd 7e 00 	. ~ . 
	and a			;584b	a7 	. 
	jr z,l585ah		;584c	28 0c 	( . 
	inc ix		;584e	dd 23 	. # 
	cp 022h		;5850	fe 22 	. " 
	jr z,l585ah		;5852	28 06 	( . 
	call WRITE_OPCODE		;5854	cd b7 60 	. . ` 
	inc e			;5857	1c 	. 
	jr l5848h		;5858	18 ee 	. . 
l585ah:
	pop hl			;585a	e1 	. 
	dec hl			;585b	2b 	+ 
	ld (hl),e			;585c	73 	s 
	ld de,(0f560h)		;585d	ed 5b 60 f5 	. [ ` . 
	dec hl			;5861	2b 	+ 
	dec hl			;5862	2b 	+ 
	ld (hl),e			;5863	73 	s 
	inc hl			;5864	23 	# 
	ld (hl),d			;5865	72 	r 
	inc hl			;5866	23 	# 
	call sub_607ch		;5867	cd 7c 60 	. | ` 
	ld b,004h		;586a	06 04 	. . 
	ret			;586c	c9 	. 
l586dh:
	cp 041h		;586d	fe 41 	. A 
	jp c,l591eh		;586f	da 1e 59 	. . Y 
	cp 05bh		;5872	fe 5b 	. [ 
	jp nc,l591eh		;5874	d2 1e 59 	. . Y 
	dec ix		;5877	dd 2b 	. + 
	call GET_VAR_PARAMETER		;5879	cd 01 65 	. . e 
	cp 028h		;587c	fe 28 	. ( 
	jr z,l58a2h		;587e	28 22 	( " 
	bit 5,h		;5880	cb 6c 	. l 
	jr nz,l5897h		;5882	20 13 	  . 
	bit 6,h		;5884	cb 74 	. t 
	ld b,001h		;5886	06 01 	. . 
	ret z			;5888	c8 	. 
	ld c,00ah		;5889	0e 0a 	. . 
	call sub_62e5h		;588b	cd e5 62 	. . b 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;588e	cd 56 60 	. V ` 
	ld b,a			;5891	47 	G 
	nop			;5892	00 	. 
	nop			;5893	00 	. 
	ld b,003h		;5894	06 03 	. . 
	ret			;5896	c9 	. 
l5897h:
	ld a,021h		;5897	3e 21 	> ! 
	call WRITE_OPCODE		;5899	cd b7 60 	. . ` 
	call sub_62e5h		;589c	cd e5 62 	. . b 
	ld b,004h		;589f	06 04 	. . 
	ret			;58a1	c9 	. 
l58a2h:
	inc ix		;58a2	dd 23 	. # 
	call sub_59fah		;58a4	cd fa 59 	. . Y 
	bit 5,a		;58a7	cb 6f 	. o 
	jp nz,l5902h		;58a9	c2 02 59 	. . Y 
	bit 6,a		;58ac	cb 77 	. w 
	jr nz,l58cfh		;58ae	20 1f 	  . 
	inc b			;58b0	04 	. 
	djnz l58bdh		;58b1	10 0a 	. . 
	add hl,hl			;58b3	29 	) 
	add hl,de			;58b4	19 	. 
	ld a,02ah		;58b5	3e 2a 	> * 
	call WRITE_OPCODE_A_L_H		;58b7	cd a9 60 	. . ` 
	ld b,002h		;58ba	06 02 	. . 
	ret			;58bc	c9 	. 
l58bdh:
	ex de,hl			;58bd	eb 	. 
	call sub_6077h		;58be	cd 77 60 	. w ` 
l58c1h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;58c1	cd 56 60 	. V ` 
	add hl,hl			;58c4	29 	) 
	add hl,de			;58c5	19 	. 
	ld a,(hl)			;58c6	7e 	~ 
	inc hl			;58c7	23 	# 
	ld h,(hl)			;58c8	66 	f 
	ld l,a			;58c9	6f 	o 
	nop			;58ca	00 	. 
	nop			;58cb	00 	. 
	ld b,002h		;58cc	06 02 	. . 
	ret			;58ce	c9 	. 
l58cfh:
	inc b			;58cf	04 	. 
	djnz l58ebh		;58d0	10 19 	. . 
	ld c,l			;58d2	4d 	M 
	ld b,h			;58d3	44 	D 
	add hl,hl			;58d4	29 	) 
	add hl,bc			;58d5	09 	. 
	add hl,de			;58d6	19 	. 
	ld a,03ah		;58d7	3e 3a 	> : 
	call WRITE_OPCODE_A_L_H		;58d9	cd a9 60 	. . ` 
	ld a,02ah		;58dc	3e 2a 	> * 
	inc hl			;58de	23 	# 
	call WRITE_OPCODE_A_L_H		;58df	cd a9 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;58e2	cd 56 60 	. V ` 
	ld b,a			;58e5	47 	G 
	nop			;58e6	00 	. 
	nop			;58e7	00 	. 
	ld b,003h		;58e8	06 03 	. . 
	ret			;58ea	c9 	. 
l58ebh:
	ex de,hl			;58eb	eb 	. 
	call sub_6077h		;58ec	cd 77 60 	. w ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;58ef	cd 56 60 	. V ` 
	ld c,l			;58f2	4d 	M 
	ld b,h			;58f3	44 	D 
	add hl,hl			;58f4	29 	) 
	add hl,bc			;58f5	09 	. 
	add hl,de			;58f6	19 	. 
	ld b,(hl)			;58f7	46 	F 
	inc hl			;58f8	23 	# 
	ld a,(hl)			;58f9	7e 	~ 
	inc hl			;58fa	23 	# 
	ld h,(hl)			;58fb	66 	f 
l58fch:
	ld l,a			;58fc	6f 	o 
	nop			;58fd	00 	. 
	nop			;58fe	00 	. 
	ld b,003h		;58ff	06 03 	. . 
	ret			;5901	c9 	. 
l5902h:
	inc b			;5902	04 	. 
	djnz l590eh		;5903	10 09 	. . 
	ld h,l			;5905	65 	e 
	ld l,000h		;5906	2e 00 	. . 
	add hl,de			;5908	19 	. 
	ld b,004h		;5909	06 04 	. . 
	jp sub_607ch		;590b	c3 7c 60 	. | ` 
l590eh:
	ex de,hl			;590e	eb 	. 
	call sub_6077h		;590f	cd 77 60 	. w ` 
l5912h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5912	cd 56 60 	. V ` 
	ld h,l			;5915	65 	e 
	ld l,000h		;5916	2e 00 	. . 
	add hl,de			;5918	19 	. 
	nop			;5919	00 	. 
	nop			;591a	00 	. 
	ld b,004h		;591b	06 04 	. . 
	ret			;591d	c9 	. 
l591eh:
	cp 0e0h		;591e	fe e0 	. . 
	jr nz,l5948h		;5920	20 26 	  & 
	call sub_56eah		;5922	cd ea 56 	. . V 
	call sub_500bh		;5925	cd 0b 50 	. . P 
	call sub_4fa0h		;5928	cd a0 4f 	. . O 
	call sub_4fefh		;592b	cd ef 4f 	. . O 
	inc b			;592e	04 	. 
	djnz l593ah		;592f	10 09 	. . 
	ld a,l			;5931	7d 	} 
	cpl			;5932	2f 	/ 
	ld l,a			;5933	6f 	o 
	ld a,h			;5934	7c 	| 
	cpl			;5935	2f 	/ 
	ld h,a			;5936	67 	g 
	ld b,000h		;5937	06 00 	. . 
	ret			;5939	c9 	. 
l593ah:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;593a	cd 56 60 	. V ` 
	ld a,l			;593d	7d 	} 
	cpl			;593e	2f 	/ 
	ld l,a			;593f	6f 	o 
	ld a,h			;5940	7c 	| 
	cpl			;5941	2f 	/ 
	ld h,a			;5942	67 	g 
	nop			;5943	00 	. 
	nop			;5944	00 	. 
	ld b,002h		;5945	06 02 	. . 
	ret			;5947	c9 	. 
l5948h:
	cp 0f2h		;5948	fe f2 	. . 
	jr nz,l597ch		;594a	20 30 	  0 
	call sub_56eah		;594c	cd ea 56 	. . V 
	call sub_500bh		;594f	cd 0b 50 	. . P 
	call sub_4fa0h		;5952	cd a0 4f 	. . O 
	inc b			;5955	04 	. 
	djnz l595fh		;5956	10 07 	. . 
	xor a			;5958	af 	. 
	sub l			;5959	95 	. 
	ld l,a			;595a	6f 	o 
	sbc a,a			;595b	9f 	. 
	sub h			;595c	94 	. 
	ld h,a			;595d	67 	g 
	ret			;595e	c9 	. 
l595fh:
	dec b			;595f	05 	. 
	djnz l5970h		;5960	10 0e 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5962	cd 56 60 	. V ` 
	xor a			;5965	af 	. 
	sub l			;5966	95 	. 
	ld l,a			;5967	6f 	o 
	sbc a,a			;5968	9f 	. 
	sub h			;5969	94 	. 
	ld h,a			;596a	67 	g 
	nop			;596b	00 	. 
	nop			;596c	00 	. 
	ld b,002h		;596d	06 02 	. . 
	ret			;596f	c9 	. 
l5970h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5970	cd 56 60 	. V ` 
	ld a,h			;5973	7c 	| 
	xor 080h		;5974	ee 80 	. . 
	ld h,a			;5976	67 	g 
	nop			;5977	00 	. 
	nop			;5978	00 	. 
	ld b,003h		;5979	06 03 	. . 
	ret			;597b	c9 	. 
l597ch:
	cp 0cbh		;597c	fe cb 	. . 
	jr nz,l5993h		;597e	20 13 	  . 
	ld a,02ah		;5980	3e 2a 	> * 
	ld hl,BIOS_JIFFY		;5982	21 9e fc 	! . . 
	call WRITE_OPCODE_A_L_H		;5985	cd a9 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5988	cd 56 60 	. V ` 
	call sub_781bh		;598b	cd 1b 78 	. . x 
	nop			;598e	00 	. 
	nop			;598f	00 	. 
	ld b,003h		;5990	06 03 	. . 
	ret			;5992	c9 	. 
l5993h:
	cp 0ech		;5993	fe ec 	. . 
	jr nz,l59a2h		;5995	20 0b 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5997	cd 56 60 	. V ` 
	call sub_7e58h		;599a	cd 58 7e 	. X ~ 
	nop			;599d	00 	. 
	nop			;599e	00 	. 
	ld b,004h		;599f	06 04 	. . 
	ret			;59a1	c9 	. 
l59a2h:
	cp 0e8h		;59a2	fe e8 	. . 
	jr nz,l59b2h		;59a4	20 0c 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;59a6	cd 56 60 	. V ` 
	ld hl,(0f3dch)		;59a9	2a dc f3 	* . . 
	dec l			;59ac	2d 	- 
	nop			;59ad	00 	. 
	nop			;59ae	00 	. 
	jp l5b8fh		;59af	c3 8f 5b 	. . [ 
l59b2h:
	cp 0e3h		;59b2	fe e3 	. . 
	jp z,l6226h		;59b4	ca 26 62 	. & b 
	cp 0e5h		;59b7	fe e5 	. . 
	jp z,l62a5h		;59b9	ca a5 62 	. . b 
	cp 0e7h		;59bc	fe e7 	. . 
	jp z,l5bd1h		;59be	ca d1 5b 	. . [ 
	cp 0ddh		;59c1	fe dd 	. . 
	jp z,l5c53h		;59c3	ca 53 5c 	. S \ 
	cp 0c8h		;59c6	fe c8 	. . 
	jp z,l6aadh		;59c8	ca ad 6a 	. . j 
	cp 0edh		;59cb	fe ed 	. . 
	jp z,l6aa1h		;59cd	ca a1 6a 	. . j 
	cp 0c7h		;59d0	fe c7 	. . 
	jp z,l6918h		;59d2	ca 18 69 	. . i 
	cp 0ffh		;59d5	fe ff 	. . 
	jr nz,l59f7h		;59d7	20 1e 	  . 
	ld a,(ix+000h)		;59d9	dd 7e 00 	. ~ . 
	inc ix		;59dc	dd 23 	. # 
	and a			;59de	a7 	. 
	jp p,04e16h		;59df	f2 16 4e 	. . N 
	cp 0a6h		;59e2	fe a6 	. . 
	jp nc,04e16h		;59e4	d2 16 4e 	. . N 
	add a,a			;59e7	87 	. 
	ld l,a			;59e8	6f 	o 
	ld h,000h		;59e9	26 00 	& . 
	ld de,l4f08h		;59eb	11 08 4f 	. . O 
	add hl,de			;59ee	19 	. 
	ld a,(hl)			;59ef	7e 	~ 
	inc hl			;59f0	23 	# 
	ld h,(hl)			;59f1	66 	f 
	ld l,a			;59f2	6f 	o 
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;59f3	cd 42 60 	. B ` 
	jp (hl)			;59f6	e9 	. 
l59f7h:
	jp 04e16h		;59f7	c3 16 4e 	. . N 
sub_59fah:
	push hl			;59fa	e5 	. 
	call sub_6439h		;59fb	cd 39 64 	. 9 d 
	push de			;59fe	d5 	. 
	ld a,(de)			;59ff	1a 	. 
	ld b,a			;5a00	47 	G 
	call sub_5ac9h		;5a01	cd c9 5a 	. . Z 
	ld hl,0f57ah		;5a04	21 7a f5 	! z . 
	call sub_5a7ch		;5a07	cd 7c 5a 	. | Z 
	dec b			;5a0a	05 	. 
	call nz,sub_5a4eh		;5a0b	c4 4e 5a 	. N Z 
	sub 029h		;5a0e	d6 29 	. ) 
	jp nz,04e25h		;5a10	c2 25 4e 	. % N 
	ld (hl),a			;5a13	77 	w 
	inc hl			;5a14	23 	# 
	ld a,l			;5a15	7d 	} 
	ld de,0f579h		;5a16	11 79 f5 	. y . 
	sub e			;5a19	93 	. 
	ld (de),a			;5a1a	12 	. 
	ld c,a			;5a1b	4f 	O 
	ld b,000h		;5a1c	06 00 	. . 
	ex de,hl			;5a1e	eb 	. 
	dec de			;5a1f	1b 	. 
	ld hl,0ffffh		;5a20	21 ff ff 	! . . 
	add hl,sp			;5a23	39 	9 
	ex de,hl			;5a24	eb 	. 
	di			;5a25	f3 	. 
	lddr		;5a26	ed b8 	. . 
	ex de,hl			;5a28	eb 	. 
	inc hl			;5a29	23 	# 
	ei			;5a2a	fb 	. 
	ld sp,hl			;5a2b	f9 	. 
	push ix		;5a2c	dd e5 	. . 
	inc hl			;5a2e	23 	# 
	push hl			;5a2f	e5 	. 
	pop ix		;5a30	dd e1 	. . 
	call sub_4fe9h		;5a32	cd e9 4f 	. . O 
	pop ix		;5a35	dd e1 	. . 
	ld a,b			;5a37	78 	x 
	push hl			;5a38	e5 	. 
	pop iy		;5a39	fd e1 	. . 
	ld hl,00000h		;5a3b	21 00 00 	! . . 
	add hl,sp			;5a3e	39 	9 
	ld c,(hl)			;5a3f	4e 	N 
	ld b,000h		;5a40	06 00 	. . 
	add hl,bc			;5a42	09 	. 
	ld sp,hl			;5a43	f9 	. 
	push iy		;5a44	fd e5 	. . 
	pop hl			;5a46	e1 	. 
	ld b,a			;5a47	47 	G 
	pop de			;5a48	d1 	. 
	call sub_5ac9h		;5a49	cd c9 5a 	. . Z 
	pop af			;5a4c	f1 	. 
	ret			;5a4d	c9 	. 
sub_5a4eh:
	ld (hl),0f1h		;5a4e	36 f1 	6 . 
	inc hl			;5a50	23 	# 
	ld (hl),01ch		;5a51	36 1c 	6 . 
	inc hl			;5a53	23 	# 
	inc hl			;5a54	23 	# 
	dec de			;5a55	1b 	. 
	ld a,(de)			;5a56	1a 	. 
	ld (hl),a			;5a57	77 	w 
	dec de			;5a58	1b 	. 
	dec hl			;5a59	2b 	+ 
	ld a,(de)			;5a5a	1a 	. 
	ld (hl),a			;5a5b	77 	w 
	inc hl			;5a5c	23 	# 
	inc hl			;5a5d	23 	# 
	ld (hl),0f3h		;5a5e	36 f3 	6 . 
	inc hl			;5a60	23 	# 
	ld (hl),028h		;5a61	36 28 	6 ( 
	inc hl			;5a63	23 	# 
	ld (hl),028h		;5a64	36 28 	6 ( 
	inc hl			;5a66	23 	# 
	call sub_5a7ch		;5a67	cd 7c 5a 	. | Z 
	ld (hl),029h		;5a6a	36 29 	6 ) 
	inc hl			;5a6c	23 	# 
	dec b			;5a6d	05 	. 
	jr z,l5a78h		;5a6e	28 08 	( . 
	cp 02ch		;5a70	fe 2c 	. , 
	jp nz,04e25h		;5a72	c2 25 4e 	. % N 
	call sub_5a4eh		;5a75	cd 4e 5a 	. N Z 
l5a78h:
	ld (hl),029h		;5a78	36 29 	6 ) 
	inc hl			;5a7a	23 	# 
	ret			;5a7b	c9 	. 
sub_5a7ch:
	call GET_NEXT_TOKEN		;5a7c	cd 30 60 	. 0 ` 
	inc ix		;5a7f	dd 23 	. # 
	and a			;5a81	a7 	. 
	jp z,04e25h		;5a82	ca 25 4e 	. % N 
	cp 02ch		;5a85	fe 2c 	. , 
	ret z			;5a87	c8 	. 
	cp 029h		;5a88	fe 29 	. ) 
	ret z			;5a8a	c8 	. 
	ld (hl),a			;5a8b	77 	w 
	inc hl			;5a8c	23 	# 
	cp 020h		;5a8d	fe 20 	.   
	jr c,l5aa0h		;5a8f	38 0f 	8 . 
	cp 028h		;5a91	fe 28 	. ( 
	jr nz,sub_5a7ch		;5a93	20 e7 	  . 
l5a95h:
	call sub_5a7ch		;5a95	cd 7c 5a 	. | Z 
	ld (hl),a			;5a98	77 	w 
	inc hl			;5a99	23 	# 
	cp 029h		;5a9a	fe 29 	. ) 
	jr nz,l5a95h		;5a9c	20 f7 	  . 
	jr sub_5a7ch		;5a9e	18 dc 	. . 
l5aa0h:
	cp 00bh		;5aa0	fe 0b 	. . 
	jr c,sub_5a7ch		;5aa2	38 d8 	8 . 
	push bc			;5aa4	c5 	. 
	cp 00fh		;5aa5	fe 0f 	. . 
	ld b,001h		;5aa7	06 01 	. . 
	jr z,l5abdh		;5aa9	28 12 	( . 
	inc b			;5aab	04 	. 
	jr c,l5abdh		;5aac	38 0f 	8 . 
	cp 01bh		;5aae	fe 1b 	. . 
	jr c,l5ac6h		;5ab0	38 14 	8 . 
	sub 01ch		;5ab2	d6 1c 	. . 
	jr z,l5abdh		;5ab4	28 07 	( . 
	dec a			;5ab6	3d 	= 
	ld b,004h		;5ab7	06 04 	. . 
	jr z,l5abdh		;5ab9	28 02 	( . 
	ld b,008h		;5abb	06 08 	. . 
l5abdh:
	ld a,(ix+000h)		;5abd	dd 7e 00 	. ~ . 
	ld (hl),a			;5ac0	77 	w 
	inc ix		;5ac1	dd 23 	. # 
	inc hl			;5ac3	23 	# 
	djnz l5abdh		;5ac4	10 f7 	. . 
l5ac6h:
	pop bc			;5ac6	c1 	. 
	jr sub_5a7ch		;5ac7	18 b3 	. . 
sub_5ac9h:
	ld a,(de)			;5ac9	1a 	. 
	inc de			;5aca	13 	. 
l5acbh:
	inc de			;5acb	13 	. 
	inc de			;5acc	13 	. 
	dec a			;5acd	3d 	= 
	ret z			;5ace	c8 	. 
	jr l5acbh		;5acf	18 fa 	. . 
	ld hl,TOKEN_08_RND		;5ad1	21 78 76 	! x v 
	jr l5af7h		;5ad4	18 21 	. ! 
	ld hl,TOKEN_0B_EXP		;5ad6	21 fa 79 	! . y 
	jr l5af7h		;5ad9	18 1c 	. . 
	ld hl,TOKEN_0A_LOG		;5adb	21 53 7a 	! S z 
	jr l5af7h		;5ade	18 17 	. . 
	ld hl,TOKEN_09_SIN		;5ae0	21 36 79 	! 6 y 
	jr l5af7h		;5ae3	18 12 	. . 
	ld hl,TOKEN_0C_COS		;5ae5	21 2e 79 	! . y 
	jr l5af7h		;5ae8	18 0d 	. . 
	ld hl,TOKEN_0D_TAN		;5aea	21 90 79 	! . y 
	jr l5af7h		;5aed	18 08 	. . 
	ld hl,TOKEN_0E_ATN		;5aef	21 b2 79 	! . y 
	jr l5af7h		;5af2	18 03 	. . 
	ld hl,TOKEN_07_SQR		;5af4	21 b5 7a 	! . z 
l5af7h:
	push hl			;5af7	e5 	. 
	call sub_4faeh		;5af8	cd ae 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5afb	cd 46 60 	. F ` 
	pop hl			;5afe	e1 	. 
	jp l53ech		;5aff	c3 ec 53 	. . S 
	call sub_4fdah		;5b02	cd da 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5b05	cd 46 60 	. F ` 
	ld a,b			;5b08	78 	x 
	cp 002h		;5b09	fe 02 	. . 
	ret z			;5b0b	c8 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b0c	cd 56 60 	. V ` 
	call sub_78e5h		;5b0f	cd e5 78 	. . x 
	nop			;5b12	00 	. 
	nop			;5b13	00 	. 
	ret			;5b14	c9 	. 
	call sub_4fdah		;5b15	cd da 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5b18	cd 46 60 	. F ` 
	ld a,b			;5b1b	78 	x 
	cp 002h		;5b1c	fe 02 	. . 
	ret z			;5b1e	c8 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b1f	cd 56 60 	. V ` 
	call sub_78d8h		;5b22	cd d8 78 	. . x 
	nop			;5b25	00 	. 
	nop			;5b26	00 	. 
	ret			;5b27	c9 	. 
	call sub_4fdah		;5b28	cd da 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5b2b	cd 46 60 	. F ` 
	ld a,b			;5b2e	78 	x 
	cp 002h		;5b2f	fe 02 	. . 
	jr nz,l5b45h		;5b31	20 12 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b33	cd 56 60 	. V ` 
	ld a,h			;5b36	7c 	| 
	add a,a			;5b37	87 	. 
	jr nc,l5b40h		;5b38	30 06 	0 . 
	xor a			;5b3a	af 	. 
	sub l			;5b3b	95 	. 
	ld l,a			;5b3c	6f 	o 
	sbc a,a			;5b3d	9f 	. 
	sub h			;5b3e	94 	. 
	ld h,a			;5b3f	67 	g 
l5b40h:
	nop			;5b40	00 	. 
	nop			;5b41	00 	. 
	ld b,002h		;5b42	06 02 	. . 
	ret			;5b44	c9 	. 
l5b45h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b45	cd 56 60 	. V ` 
	res 7,h		;5b48	cb bc 	. . 
	nop			;5b4a	00 	. 
	nop			;5b4b	00 	. 
	ld b,003h		;5b4c	06 03 	. . 
	ret			;5b4e	c9 	. 
	call sub_4fdah		;5b4f	cd da 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5b52	cd 46 60 	. F ` 
	ld a,b			;5b55	78 	x 
	cp 002h		;5b56	fe 02 	. . 
	jr nz,l5b6fh		;5b58	20 15 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b5a	cd 56 60 	. V ` 
	ld a,l			;5b5d	7d 	} 
	or h			;5b5e	b4 	. 
	jr z,l5b6ah		;5b5f	28 09 	( . 
	bit 7,h		;5b61	cb 7c 	. | 
	ld hl,0ffffh		;5b63	21 ff ff 	! . . 
	jr nz,l5b6ah		;5b66	20 02 	  . 
	inc hl			;5b68	23 	# 
	inc hl			;5b69	23 	# 
l5b6ah:
	nop			;5b6a	00 	. 
	nop			;5b6b	00 	. 
	ld b,002h		;5b6c	06 02 	. . 
	ret			;5b6e	c9 	. 
l5b6fh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b6f	cd 56 60 	. V ` 
	ld a,b			;5b72	78 	x 
	and a			;5b73	a7 	. 
	jr z,l5b7eh		;5b74	28 08 	( . 
	ld b,081h		;5b76	06 81 	. . 
	ld l,000h		;5b78	2e 00 	. . 
	ld a,h			;5b7a	7c 	| 
	and 080h		;5b7b	e6 80 	. . 
	ld h,a			;5b7d	67 	g 
l5b7eh:
	nop			;5b7e	00 	. 
	nop			;5b7f	00 	. 
	ld b,003h		;5b80	06 03 	. . 
	ret			;5b82	c9 	. 
	call sub_4f97h		;5b83	cd 97 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5b86	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b89	cd 56 60 	. V ` 
	ld l,(hl)			;5b8c	6e 	n 
	nop			;5b8d	00 	. 
	nop			;5b8e	00 	. 
l5b8fh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5b8f	cd 56 60 	. V ` 
	ld h,000h		;5b92	26 00 	& . 
	nop			;5b94	00 	. 
	nop			;5b95	00 	. 
	ld b,002h		;5b96	06 02 	. . 
	ret			;5b98	c9 	. 
	call sub_4fa9h		;5b99	cd a9 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5b9c	cd 46 60 	. F ` 
	inc b			;5b9f	04 	. 
	djnz l5bb3h		;5ba0	10 11 	. . 
	ld a,0dbh		;5ba2	3e db 	> . 
	call WRITE_OPCODE		;5ba4	cd b7 60 	. . ` 
	ld a,l			;5ba7	7d 	} 
	call WRITE_OPCODE		;5ba8	cd b7 60 	. . ` 
l5babh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5bab	cd 56 60 	. V ` 
	ld l,a			;5bae	6f 	o 
	nop			;5baf	00 	. 
	nop			;5bb0	00 	. 
	jr l5b8fh		;5bb1	18 dc 	. . 
l5bb3h:
	djnz l5bc2h		;5bb3	10 0d 	. . 
	push hl			;5bb5	e5 	. 
	ld hl,04bedh		;5bb6	21 ed 4b 	! . K 
	call sub_60ach		;5bb9	cd ac 60 	. . ` 
	pop hl			;5bbc	e1 	. 
	call sub_62e5h		;5bbd	cd e5 62 	. . b 
	jr l5bc8h		;5bc0	18 06 	. . 
l5bc2h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5bc2	cd 56 60 	. V ` 
	ld c,l			;5bc5	4d 	M 
	nop			;5bc6	00 	. 
	nop			;5bc7	00 	. 
l5bc8h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5bc8	cd 56 60 	. V ` 
	in l,(c)		;5bcb	ed 68 	. h 
	nop			;5bcd	00 	. 
	nop			;5bce	00 	. 
	jr l5b8fh		;5bcf	18 be 	. . 
l5bd1h:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;5bd1	cd 42 60 	. B ` 
	call sub_5bdch		;5bd4	cd dc 5b 	. . [ 
	ld b,002h		;5bd7	06 02 	. . 
	jp ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5bd9	c3 46 60 	. F ` 
sub_5bdch:
	call sub_5bf1h		;5bdc	cd f1 5b 	. . [ 
	push af			;5bdf	f5 	. 
	call sub_4fe0h		;5be0	cd e0 4f 	. . O 
	djnz l5befh		;5be3	10 0a 	. . 
	ld a,021h		;5be5	3e 21 	> ! 
	call WRITE_OPCODE		;5be7	cd b7 60 	. . ` 
	ld c,000h		;5bea	0e 00 	. . 
	call sub_62e5h		;5bec	cd e5 62 	. . b 
l5befh:
	pop af			;5bef	f1 	. 
	ret			;5bf0	c9 	. 
sub_5bf1h:
	call GET_VAR_PARAMETER		;5bf1	cd 01 65 	. . e 
	cp 028h		;5bf4	fe 28 	. ( 
	jr z,l5bfeh		;5bf6	28 06 	( . 
	ld b,001h		;5bf8	06 01 	. . 
	ld a,h			;5bfa	7c 	| 
	and 060h		;5bfb	e6 60 	. ` 
	ret			;5bfd	c9 	. 
l5bfeh:
	inc ix		;5bfe	dd 23 	. # 
	push hl			;5c00	e5 	. 
	call sub_5c08h		;5c01	cd 08 5c 	. . \ 
	pop af			;5c04	f1 	. 
	and 060h		;5c05	e6 60 	. ` 
	ret			;5c07	c9 	. 
sub_5c08h:
	call sub_59fah		;5c08	cd fa 59 	. . Y 
	inc b			;5c0b	04 	. 
	djnz l5c23h		;5c0c	10 15 	. . 
	bit 5,a		;5c0e	cb 6f 	. o 
	jr nz,l5c1ch		;5c10	20 0a 	  . 
	ld c,l			;5c12	4d 	M 
	ld b,h			;5c13	44 	D 
	add hl,hl			;5c14	29 	) 
	bit 6,a		;5c15	cb 77 	. w 
	jr z,l5c1fh		;5c17	28 06 	( . 
	add hl,bc			;5c19	09 	. 
	jr l5c1fh		;5c1a	18 03 	. . 
l5c1ch:
	ld h,l			;5c1c	65 	e 
	ld l,000h		;5c1d	2e 00 	. . 
l5c1fh:
	add hl,de			;5c1f	19 	. 
	ld b,000h		;5c20	06 00 	. . 
	ret			;5c22	c9 	. 
l5c23h:
	bit 5,a		;5c23	cb 6f 	. o 
	jr nz,l5c3eh		;5c25	20 17 	  . 
	bit 6,a		;5c27	cb 77 	. w 
	jr z,l5c36h		;5c29	28 0b 	( . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5c2b	cd 56 60 	. V ` 
	ld c,l			;5c2e	4d 	M 
	ld b,h			;5c2f	44 	D 
	add hl,hl			;5c30	29 	) 
	add hl,bc			;5c31	09 	. 
	nop			;5c32	00 	. 
	nop			;5c33	00 	. 
	jr l5c46h		;5c34	18 10 	. . 
l5c36h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5c36	cd 56 60 	. V ` 
	add hl,hl			;5c39	29 	) 
	nop			;5c3a	00 	. 
	nop			;5c3b	00 	. 
	jr l5c46h		;5c3c	18 08 	. . 
l5c3eh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5c3e	cd 56 60 	. V ` 
	ld h,l			;5c41	65 	e 
	ld l,000h		;5c42	2e 00 	. . 
	nop			;5c44	00 	. 
	nop			;5c45	00 	. 
l5c46h:
	ex de,hl			;5c46	eb 	. 
	call sub_6077h		;5c47	cd 77 60 	. w ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5c4a	cd 56 60 	. V ` 
	add hl,de			;5c4d	19 	. 
	nop			;5c4e	00 	. 
	nop			;5c4f	00 	. 
	ld b,002h		;5c50	06 02 	. . 
	ret			;5c52	c9 	. 
l5c53h:
	call GET_NEXT_TOKEN		;5c53	cd 30 60 	. 0 ` 
	ld c,000h		;5c56	0e 00 	. . 
	sub 011h		;5c58	d6 11 	. . 
	jr c,l5c63h		;5c5a	38 07 	8 . 
	cp 00ah		;5c5c	fe 0a 	. . 
	jr nc,l5c63h		;5c5e	30 03 	0 . 
	ld c,a			;5c60	4f 	O 
	inc ix		;5c61	dd 23 	. # 
l5c63h:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;5c63	cd 42 60 	. B ` 
	push bc			;5c66	c5 	. 
	call sub_4f97h		;5c67	cd 97 4f 	. . O 
	pop bc			;5c6a	c1 	. 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5c6b	cd 46 60 	. F ` 
	ld hl,05bedh		;5c6e	21 ed 5b 	! . [ 
	call sub_60ach		;5c71	cd ac 60 	. . ` 
	ld hl,0f39ah		;5c74	21 9a f3 	! . . 
	ld b,000h		;5c77	06 00 	. . 
	add hl,bc			;5c79	09 	. 
	add hl,bc			;5c7a	09 	. 
	call sub_60ach		;5c7b	cd ac 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5c7e	cd 56 60 	. V ` 
	call sub_6c09h		;5c81	cd 09 6c 	. . l 
	nop			;5c84	00 	. 
	nop			;5c85	00 	. 
	ld b,002h		;5c86	06 02 	. . 
	ret			;5c88	c9 	. 
	ld hl,0f661h		;5c89	21 61 f6 	! a . 
	jr l5c91h		;5c8c	18 03 	. . 
	ld hl,0f415h		;5c8e	21 15 f4 	! . . 
l5c91h:
	push hl			;5c91	e5 	. 
	call sub_5012h		;5c92	cd 12 50 	. . P 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5c95	cd 46 60 	. F ` 
	pop hl			;5c98	e1 	. 
	ld a,02ah		;5c99	3e 2a 	> * 
	call WRITE_OPCODE_A_L_H		;5c9b	cd a9 60 	. . ` 
	jp l5b8fh		;5c9e	c3 8f 5b 	. . [ 
	call GET_NUM_PARAMETER		;5ca1	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5ca4	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5ca7	cd 56 60 	. V ` 
	call BIOS_GTSTCK		;5caa	cd d5 00 	. . . 
	nop			;5cad	00 	. 
	nop			;5cae	00 	. 
	jp l5babh		;5caf	c3 ab 5b 	. . [ 
	call GET_NUM_PARAMETER		;5cb2	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5cb5	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5cb8	cd 56 60 	. V ` 
	call BIOS_GTTRIG		;5cbb	cd d8 00 	. . . 
	ld l,a			;5cbe	6f 	o 
	ld h,a			;5cbf	67 	g 
	nop			;5cc0	00 	. 
	nop			;5cc1	00 	. 
	ld b,002h		;5cc2	06 02 	. . 
	ret			;5cc4	c9 	. 
	call sub_4fa9h		;5cc5	cd a9 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5cc8	cd 46 60 	. F ` 
	inc b			;5ccb	04 	. 
	djnz l5cf2h		;5ccc	10 24 	. $ 
	push hl			;5cce	e5 	. 
	call sub_4f5ah		;5ccf	cd 5a 4f 	. Z O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5cd2	cd 56 60 	. V ` 
	call BIOS_GTPAD		;5cd5	cd db 00 	. . . 
	nop			;5cd8	00 	. 
	nop			;5cd9	00 	. 
	pop hl			;5cda	e1 	. 
	ld a,l			;5cdb	7d 	} 
	add a,004h		;5cdc	c6 04 	. . 
	and 0f3h		;5cde	e6 f3 	. . 
	dec a			;5ce0	3d 	= 
	cp 002h		;5ce1	fe 02 	. . 
	jp c,l5babh		;5ce3	da ab 5b 	. . [ 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5ce6	cd 56 60 	. V ` 
	ld l,a			;5ce9	6f 	o 
	add a,a			;5cea	87 	. 
	sbc a,a			;5ceb	9f 	. 
	ld h,a			;5cec	67 	g 
	nop			;5ced	00 	. 
	nop			;5cee	00 	. 
	ld b,002h		;5cef	06 02 	. . 
	ret			;5cf1	c9 	. 
l5cf2h:
	call l4f73h		;5cf2	cd 73 4f 	. s O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5cf5	cd 56 60 	. V ` 
	push af			;5cf8	f5 	. 
	call BIOS_GTPAD		;5cf9	cd db 00 	. . . 
	ld l,a			;5cfc	6f 	o 
	ld h,000h		;5cfd	26 00 	& . 
	pop af			;5cff	f1 	. 
	add a,004h		;5d00	c6 04 	. . 
	and 0f3h		;5d02	e6 f3 	. . 
	dec a			;5d04	3d 	= 
	cp 002h		;5d05	fe 02 	. . 
	jr c,l5d0dh		;5d07	38 04 	8 . 
	ld a,l			;5d09	7d 	} 
	add a,a			;5d0a	87 	. 
	sbc a,a			;5d0b	9f 	. 
	ld h,a			;5d0c	67 	g 
l5d0dh:
	nop			;5d0d	00 	. 
	nop			;5d0e	00 	. 
	ld b,002h		;5d0f	06 02 	. . 
	ret			;5d11	c9 	. 
	call GET_NUM_PARAMETER		;5d12	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;5d15	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5d18	cd 56 60 	. V ` 
	call BIOS_GTPDL		;5d1b	cd de 00 	. . . 
	nop			;5d1e	00 	. 
	nop			;5d1f	00 	. 
	jp l5babh		;5d20	c3 ab 5b 	. . [ 
sub_5d23h:
	call sub_5f02h		;5d23	cd 02 5f 	. . _ 
	ld b,a			;5d26	47 	G 
	ld e,l			;5d27	5d 	] 
	ld a,d			;5d28	7a 	z 
	ld e,l			;5d29	5d 	] 
	ld a,a			;5d2a	7f 	 
	ld e,l			;5d2b	5d 	] 
	ld a,e			;5d2c	7b 	{ 
	ld e,l			;5d2d	5d 	] 
	jr nc,$+96		;5d2e	30 5e 	0 ^ 
	inc sp			;5d30	33 	3 
	ld e,(hl)			;5d31	5e 	^ 
	add a,b			;5d32	80 	. 
	ld e,l			;5d33	5d 	] 
	inc (hl)			;5d34	34 	4 
	ld e,(hl)			;5d35	5e 	^ 
	add hl,sp			;5d36	39 	9 
	ld e,(hl)			;5d37	5e 	^ 
	ld h,(hl)			;5d38	66 	f 
	ld e,l			;5d39	5d 	] 
	call m,0065dh		;5d3a	fc 5d 06 	. ] . 
	ld e,(hl)			;5d3d	5e 	^ 
	ld (hl),b			;5d3e	70 	p 
	ld e,l			;5d3f	5d 	] 
	djnz $+96		;5d40	10 5e 	. ^ 
	ld (01a5eh),hl		;5d42	22 5e 1a 	" ^ . 
	ld e,(hl)			;5d45	5e 	^ 
	ret			;5d46	c9 	. 
	or 0e6h		;5d47	f6 e6 	. . 
	ld (0f567h),a		;5d49	32 67 f5 	2 g . 
	ld a,0c9h		;5d4c	3e c9 	> . 
	ld (0f569h),a		;5d4e	32 69 f5 	2 i . 
	ld a,e			;5d51	7b 	{ 
	ld (VAR_DY),a		;5d52	32 68 f5 	2 h . 
	ld a,l			;5d55	7d 	} 
	call 0f567h		;5d56	cd 67 f5 	. g . 
	ld l,a			;5d59	6f 	o 
	ld a,d			;5d5a	7a 	z 
	ld (VAR_DY),a		;5d5b	32 68 f5 	2 h . 
	ld a,h			;5d5e	7c 	| 
	call 0f567h		;5d5f	cd 67 f5 	. g . 
	ld h,a			;5d62	67 	g 
	ld b,000h		;5d63	06 00 	. . 
	ret			;5d65	c9 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5d66	cd 56 60 	. V ` 
	call sub_784fh		;5d69	cd 4f 78 	. O x 
	nop			;5d6c	00 	. 
	nop			;5d6d	00 	. 
	jr l5d7fh		;5d6e	18 0f 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5d70	cd 56 60 	. V ` 
	call sub_784fh		;5d73	cd 4f 78 	. O x 
	nop			;5d76	00 	. 
	nop			;5d77	00 	. 
	jr l5d80h		;5d78	18 06 	. . 
	ex de,hl			;5d7a	eb 	. 
	ex de,hl			;5d7b	eb 	. 
	call sub_6084h		;5d7c	cd 84 60 	. . ` 
l5d7fh:
	ex de,hl			;5d7f	eb 	. 
l5d80h:
	or 0e6h		;5d80	f6 e6 	. . 
	ld b,a			;5d82	47 	G 
	ld a,l			;5d83	7d 	} 
	call sub_5e62h		;5d84	cd 62 5e 	. b ^ 
	jr z,l5dbeh		;5d87	28 35 	( 5 
	ld a,l			;5d89	7d 	} 
	call sub_5e72h		;5d8a	cd 72 5e 	. r ^ 
	jr nz,l5d99h		;5d8d	20 0a 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5d8f	cd 56 60 	. V ` 
	ld a,l			;5d92	7d 	} 
	cpl			;5d93	2f 	/ 
	ld l,a			;5d94	6f 	o 
	nop			;5d95	00 	. 
	nop			;5d96	00 	. 
	jr l5dbeh		;5d97	18 25 	. % 
l5d99h:
	ld a,l			;5d99	7d 	} 
	call sub_5e78h		;5d9a	cd 78 5e 	. x ^ 
	jr nz,l5daah		;5d9d	20 0b 	  . 
	ld a,02eh		;5d9f	3e 2e 	> . 
	call WRITE_OPCODE		;5da1	cd b7 60 	. . ` 
	ld a,l			;5da4	7d 	} 
	call WRITE_OPCODE		;5da5	cd b7 60 	. . ` 
	jr l5dbeh		;5da8	18 14 	. . 
l5daah:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5daa	cd 56 60 	. V ` 
	ld a,l			;5dad	7d 	} 
	nop			;5dae	00 	. 
	nop			;5daf	00 	. 
	ld a,b			;5db0	78 	x 
	call WRITE_OPCODE		;5db1	cd b7 60 	. . ` 
	ld a,l			;5db4	7d 	} 
	call WRITE_OPCODE		;5db5	cd b7 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5db8	cd 56 60 	. V ` 
	ld l,a			;5dbb	6f 	o 
	nop			;5dbc	00 	. 
	nop			;5dbd	00 	. 
l5dbeh:
	ld a,h			;5dbe	7c 	| 
	call sub_5e62h		;5dbf	cd 62 5e 	. b ^ 
	jr z,l5df9h		;5dc2	28 35 	( 5 
	ld a,h			;5dc4	7c 	| 
	call sub_5e72h		;5dc5	cd 72 5e 	. r ^ 
	jr nz,l5dd4h		;5dc8	20 0a 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5dca	cd 56 60 	. V ` 
	ld a,h			;5dcd	7c 	| 
	cpl			;5dce	2f 	/ 
	ld h,a			;5dcf	67 	g 
	nop			;5dd0	00 	. 
	nop			;5dd1	00 	. 
	jr l5df9h		;5dd2	18 25 	. % 
l5dd4h:
	ld a,h			;5dd4	7c 	| 
	call sub_5e78h		;5dd5	cd 78 5e 	. x ^ 
	jr nz,l5de5h		;5dd8	20 0b 	  . 
	ld a,026h		;5dda	3e 26 	> & 
	call WRITE_OPCODE		;5ddc	cd b7 60 	. . ` 
	ld a,h			;5ddf	7c 	| 
	call WRITE_OPCODE		;5de0	cd b7 60 	. . ` 
	jr l5df9h		;5de3	18 14 	. . 
l5de5h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5de5	cd 56 60 	. V ` 
	ld a,h			;5de8	7c 	| 
	nop			;5de9	00 	. 
	nop			;5dea	00 	. 
	ld a,b			;5deb	78 	x 
	call WRITE_OPCODE		;5dec	cd b7 60 	. . ` 
	ld a,h			;5def	7c 	| 
	call WRITE_OPCODE		;5df0	cd b7 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5df3	cd 56 60 	. V ` 
	ld h,a			;5df6	67 	g 
	nop			;5df7	00 	. 
	nop			;5df8	00 	. 
l5df9h:
	ld b,002h		;5df9	06 02 	. . 
	ret			;5dfb	c9 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5dfc	cd 56 60 	. V ` 
	call sub_784fh		;5dff	cd 4f 78 	. O x 
	nop			;5e02	00 	. 
	nop			;5e03	00 	. 
	jr l5e33h		;5e04	18 2d 	. - 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e06	cd 56 60 	. V ` 
	call sub_784fh		;5e09	cd 4f 78 	. O x 
	nop			;5e0c	00 	. 
	nop			;5e0d	00 	. 
	jr l5e39h		;5e0e	18 29 	. ) 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e10	cd 56 60 	. V ` 
	call sub_784fh		;5e13	cd 4f 78 	. O x 
	nop			;5e16	00 	. 
	nop			;5e17	00 	. 
	jr l5e34h		;5e18	18 1a 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e1a	cd 56 60 	. V ` 
	call sub_784fh		;5e1d	cd 4f 78 	. O x 
	nop			;5e20	00 	. 
	nop			;5e21	00 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e22	cd 56 60 	. V ` 
	pop bc			;5e25	c1 	. 
	pop de			;5e26	d1 	. 
	push hl			;5e27	e5 	. 
	ex de,hl			;5e28	eb 	. 
	call sub_784fh		;5e29	cd 4f 78 	. O x 
	nop			;5e2c	00 	. 
	nop			;5e2d	00 	. 
	jr l5e39h		;5e2e	18 09 	. . 
	call sub_6084h		;5e30	cd 84 60 	. . ` 
l5e33h:
	ex de,hl			;5e33	eb 	. 
l5e34h:
	call sub_6094h		;5e34	cd 94 60 	. . ` 
	jr l5e3fh		;5e37	18 06 	. . 
l5e39h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e39	cd 56 60 	. V ` 
	pop de			;5e3c	d1 	. 
	nop			;5e3d	00 	. 
	nop			;5e3e	00 	. 
l5e3fh:
	ld b,a			;5e3f	47 	G 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e40	cd 56 60 	. V ` 
	ld a,l			;5e43	7d 	} 
	nop			;5e44	00 	. 
	nop			;5e45	00 	. 
	ld a,b			;5e46	78 	x 
	or 0a3h		;5e47	f6 a3 	. . 
	call WRITE_OPCODE		;5e49	cd b7 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e4c	cd 56 60 	. V ` 
	ld l,a			;5e4f	6f 	o 
	ld a,h			;5e50	7c 	| 
	nop			;5e51	00 	. 
	nop			;5e52	00 	. 
	ld a,b			;5e53	78 	x 
	or 0a2h		;5e54	f6 a2 	. . 
	call WRITE_OPCODE		;5e56	cd b7 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5e59	cd 56 60 	. V ` 
	ld h,a			;5e5c	67 	g 
	nop			;5e5d	00 	. 
	nop			;5e5e	00 	. 
	ld b,002h		;5e5f	06 02 	. . 
	ret			;5e61	c9 	. 
sub_5e62h:
	and a			;5e62	a7 	. 
	jr z,l5e6bh		;5e63	28 06 	( . 
	inc a			;5e65	3c 	< 
	ret nz			;5e66	c0 	. 
	ld a,b			;5e67	78 	x 
	cp 0e6h		;5e68	fe e6 	. . 
	ret			;5e6a	c9 	. 
l5e6bh:
	ld a,b			;5e6b	78 	x 
	cp 0f6h		;5e6c	fe f6 	. . 
	ret z			;5e6e	c8 	. 
	cp 0eeh		;5e6f	fe ee 	. . 
	ret			;5e71	c9 	. 
sub_5e72h:
	inc a			;5e72	3c 	< 
	ret nz			;5e73	c0 	. 
	ld a,b			;5e74	78 	x 
	cp 0eeh		;5e75	fe ee 	. . 
	ret			;5e77	c9 	. 
sub_5e78h:
	and a			;5e78	a7 	. 
	jr z,l5e81h		;5e79	28 06 	( . 
	inc a			;5e7b	3c 	< 
	ret nz			;5e7c	c0 	. 
	ld a,b			;5e7d	78 	x 
	cp 0f6h		;5e7e	fe f6 	. . 
	ret			;5e80	c9 	. 
l5e81h:
	ld a,b			;5e81	78 	x 
	cp 0e6h		;5e82	fe e6 	. . 
	ret			;5e84	c9 	. 
sub_5e85h:
	push hl			;5e85	e5 	. 
	push bc			;5e86	c5 	. 
	push af			;5e87	f5 	. 
	ld c,a			;5e88	4f 	O 
	ld b,000h		;5e89	06 00 	. . 
	ld hl,l50d0h		;5e8b	21 d0 50 	! . P 
	add hl,bc			;5e8e	09 	. 
	ld c,(hl)			;5e8f	4e 	N 
	inc hl			;5e90	23 	# 
	ld b,(hl)			;5e91	46 	F 
	push bc			;5e92	c5 	. 
	pop iy		;5e93	fd e1 	. . 
	pop af			;5e95	f1 	. 
	pop bc			;5e96	c1 	. 
	pop hl			;5e97	e1 	. 
	call sub_5f02h		;5e98	cd 02 5f 	. . _ 
	cp (hl)			;5e9b	be 	. 
	ld e,(hl)			;5e9c	5e 	^ 
	jp 0c65eh		;5e9d	c3 5e c6 	. ^ . 
	ld e,(hl)			;5ea0	5e 	^ 
	call z,0d35eh		;5ea1	cc 5e d3 	. ^ . 
l5ea4h:
	ld e,(hl)			;5ea4	5e 	^ 
	sub 05eh		;5ea5	d6 5e 	. ^ 
	call c,0e15eh		;5ea7	dc 5e e1 	. ^ . 
	ld e,(hl)			;5eaa	5e 	^ 
	and 05eh		;5eab	e6 5e 	. ^ 
	rst 18h			;5ead	df 	. 
	ld h,b			;5eae	60 	` 
	rst 28h			;5eaf	ef 	. 
	ld h,b			;5eb0	60 	` 
	call m,00760h		;5eb1	fc 60 07 	. ` . 
	ld h,c			;5eb4	61 	a 
	add hl,hl			;5eb5	29 	) 
	ld h,c			;5eb6	61 	a 
	inc l			;5eb7	2c 	, 
	ld h,c			;5eb8	61 	a 
	cpl			;5eb9	2f 	/ 
	ld h,c			;5eba	61 	a 
	ld b,002h		;5ebb	06 02 	. . 
	ret			;5ebd	c9 	. 
	call sub_607ch		;5ebe	cd 7c 60 	. | ` 
	jr l5ec6h		;5ec1	18 03 	. . 
	call sub_6084h		;5ec3	cd 84 60 	. . ` 
l5ec6h:
	ex de,hl			;5ec6	eb 	. 
	call sub_6077h		;5ec7	cd 77 60 	. w ` 
	jr l5eech		;5eca	18 20 	.   
	ex de,hl			;5ecc	eb 	. 
	call sub_6084h		;5ecd	cd 84 60 	. . ` 
	ex de,hl			;5ed0	eb 	. 
	jr l5edch		;5ed1	18 09 	. . 
	call sub_6084h		;5ed3	cd 84 60 	. . ` 
	ex de,hl			;5ed6	eb 	. 
	call sub_6094h		;5ed7	cd 94 60 	. . ` 
	jr l5eech		;5eda	18 10 	. . 
l5edch:
	call sub_6077h		;5edc	cd 77 60 	. w ` 
	jr l5eedh		;5edf	18 0c 	. . 
	call sub_6094h		;5ee1	cd 94 60 	. . ` 
	jr l5eedh		;5ee4	18 07 	. . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5ee6	cd 56 60 	. V ` 
	pop de			;5ee9	d1 	. 
	nop			;5eea	00 	. 
	nop			;5eeb	00 	. 
l5eech:
	inc a			;5eec	3c 	< 
l5eedh:
	add a,a			;5eed	87 	. 
	ld l,a			;5eee	6f 	o 
	ld h,000h		;5eef	26 00 	& . 
	ld de,l50b4h		;5ef1	11 b4 50 	. . P 
	add hl,de			;5ef4	19 	. 
	ld a,(hl)			;5ef5	7e 	~ 
	inc hl			;5ef6	23 	# 
	ld h,(hl)			;5ef7	66 	f 
	ld l,a			;5ef8	6f 	o 
l5ef9h:
	ld a,(hl)			;5ef9	7e 	~ 
	and a			;5efa	a7 	. 
	ret z			;5efb	c8 	. 
	call WRITE_OPCODE		;5efc	cd b7 60 	. . ` 
	inc hl			;5eff	23 	# 
	jr l5ef9h		;5f00	18 f7 	. . 
sub_5f02h:
	inc ix		;5f02	dd 23 	. # 
	inc b			;5f04	04 	. 
	djnz l5f48h		;5f05	10 41 	. A 
	push hl			;5f07	e5 	. 
	push af			;5f08	f5 	. 
	push iy		;5f09	fd e5 	. . 
	call sub_602eh		;5f0b	cd 2e 60 	. . ` 
	call sub_500bh		;5f0e	cd 0b 50 	. . P 
	pop iy		;5f11	fd e1 	. . 
	pop af			;5f13	f1 	. 
	pop de			;5f14	d1 	. 
	inc b			;5f15	04 	. 
	djnz l5f23h		;5f16	10 0b 	. . 
	ex (sp),iy		;5f18	fd e3 	. . 
	ld c,(iy+000h)		;5f1a	fd 4e 00 	. N . 
	ld b,(iy+001h)		;5f1d	fd 46 01 	. F . 
	jp l601eh		;5f20	c3 1e 60 	. . ` 
l5f23h:
	djnz l5f30h		;5f23	10 0b 	. . 
	ex (sp),iy		;5f25	fd e3 	. . 
	ld c,(iy+002h)		;5f27	fd 4e 02 	. N . 
	ld b,(iy+003h)		;5f2a	fd 46 03 	. F . 
	jp l601eh		;5f2d	c3 1e 60 	. . ` 
l5f30h:
	djnz l5f3dh		;5f30	10 0b 	. . 
	ex (sp),iy		;5f32	fd e3 	. . 
	ld c,(iy+004h)		;5f34	fd 4e 04 	. N . 
	ld b,(iy+005h)		;5f37	fd 46 05 	. F . 
	jp l601eh		;5f3a	c3 1e 60 	. . ` 
l5f3dh:
	ex (sp),iy		;5f3d	fd e3 	. . 
	ld c,(iy+012h)		;5f3f	fd 4e 12 	. N . 
	ld b,(iy+013h)		;5f42	fd 46 13 	. F . 
	jp l601eh		;5f45	c3 1e 60 	. . ` 
l5f48h:
	djnz l5f8bh		;5f48	10 41 	. A 
	push hl			;5f4a	e5 	. 
	push af			;5f4b	f5 	. 
	push iy		;5f4c	fd e5 	. . 
	call sub_602eh		;5f4e	cd 2e 60 	. . ` 
	call sub_500bh		;5f51	cd 0b 50 	. . P 
	pop iy		;5f54	fd e1 	. . 
	pop af			;5f56	f1 	. 
	pop de			;5f57	d1 	. 
	inc b			;5f58	04 	. 
	djnz l5f66h		;5f59	10 0b 	. . 
	ex (sp),iy		;5f5b	fd e3 	. . 
l5f5dh:
	ld c,(iy+006h)		;5f5d	fd 4e 06 	. N . 
	ld b,(iy+007h)		;5f60	fd 46 07 	. F . 
	jp l601eh		;5f63	c3 1e 60 	. . ` 
l5f66h:
	djnz l5f73h		;5f66	10 0b 	. . 
	ex (sp),iy		;5f68	fd e3 	. . 
	ld c,(iy+008h)		;5f6a	fd 4e 08 	. N . 
	ld b,(iy+009h)		;5f6d	fd 46 09 	. F . 
	jp l601eh		;5f70	c3 1e 60 	. . ` 
l5f73h:
	djnz l5f80h		;5f73	10 0b 	. . 
	ex (sp),iy		;5f75	fd e3 	. . 
	ld c,(iy+00ah)		;5f77	fd 4e 0a 	. N . 
	ld b,(iy+00bh)		;5f7a	fd 46 0b 	. F . 
	jp l601eh		;5f7d	c3 1e 60 	. . ` 
l5f80h:
	ex (sp),iy		;5f80	fd e3 	. . 
	ld c,(iy+014h)		;5f82	fd 4e 14 	. N . 
	ld b,(iy+015h)		;5f85	fd 46 15 	. F . 
	jp l601eh		;5f88	c3 1e 60 	. . ` 
l5f8bh:
	djnz l5fd4h		;5f8b	10 47 	. G 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5f8d	cd 56 60 	. V ` 
	push hl			;5f90	e5 	. 
	nop			;5f91	00 	. 
	nop			;5f92	00 	. 
	push af			;5f93	f5 	. 
	push iy		;5f94	fd e5 	. . 
	call sub_602eh		;5f96	cd 2e 60 	. . ` 
	call sub_500bh		;5f99	cd 0b 50 	. . P 
	pop iy		;5f9c	fd e1 	. . 
	pop af			;5f9e	f1 	. 
	inc b			;5f9f	04 	. 
	djnz l5fafh		;5fa0	10 0d 	. . 
	call sub_60d5h		;5fa2	cd d5 60 	. . ` 
	ex (sp),iy		;5fa5	fd e3 	. . 
	ld c,(iy+00ch)		;5fa7	fd 4e 0c 	. N . 
	ld b,(iy+00dh)		;5faa	fd 46 0d 	. F . 
	jr l601eh		;5fad	18 6f 	. o 
l5fafh:
	djnz l5fbeh		;5faf	10 0d 	. . 
	call sub_60d5h		;5fb1	cd d5 60 	. . ` 
	ex (sp),iy		;5fb4	fd e3 	. . 
	ld c,(iy+00eh)		;5fb6	fd 4e 0e 	. N . 
	ld b,(iy+00fh)		;5fb9	fd 46 0f 	. F . 
	jr l601eh		;5fbc	18 60 	. ` 
l5fbeh:
	djnz l5fcah		;5fbe	10 0a 	. . 
	ex (sp),iy		;5fc0	fd e3 	. . 
	ld c,(iy+010h)		;5fc2	fd 4e 10 	. N . 
	ld b,(iy+011h)		;5fc5	fd 46 11 	. F . 
	jr l601eh		;5fc8	18 54 	. T 
l5fcah:
	ex (sp),iy		;5fca	fd e3 	. . 
	ld c,(iy+016h)		;5fcc	fd 4e 16 	. N . 
	ld b,(iy+017h)		;5fcf	fd 46 17 	. F . 
	jr l601eh		;5fd2	18 4a 	. J 
l5fd4h:
	dec b			;5fd4	05 	. 
	jp nz,04e10h		;5fd5	c2 10 4e 	. . N 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;5fd8	cd 56 60 	. V ` 
	push hl			;5fdb	e5 	. 
	push bc			;5fdc	c5 	. 
	nop			;5fdd	00 	. 
	nop			;5fde	00 	. 
	push af			;5fdf	f5 	. 
	push iy		;5fe0	fd e5 	. . 
	call sub_602eh		;5fe2	cd 2e 60 	. . ` 
	call sub_500bh		;5fe5	cd 0b 50 	. . P 
	pop iy		;5fe8	fd e1 	. . 
	pop af			;5fea	f1 	. 
	inc b			;5feb	04 	. 
	djnz l5ffeh		;5fec	10 10 	. . 
	call sub_60d5h		;5fee	cd d5 60 	. . ` 
	call sub_60d5h		;5ff1	cd d5 60 	. . ` 
	ex (sp),iy		;5ff4	fd e3 	. . 
	ld c,(iy+018h)		;5ff6	fd 4e 18 	. N . 
	ld b,(iy+019h)		;5ff9	fd 46 19 	. F . 
	jr l601eh		;5ffc	18 20 	.   
l5ffeh:
	djnz l600ah		;5ffe	10 0a 	. . 
	ex (sp),iy		;6000	fd e3 	. . 
	ld c,(iy+01ah)		;6002	fd 4e 1a 	. N . 
	ld b,(iy+01bh)		;6005	fd 46 1b 	. F . 
	jr l601eh		;6008	18 14 	. . 
l600ah:
	djnz l6016h		;600a	10 0a 	. . 
	ex (sp),iy		;600c	fd e3 	. . 
	ld c,(iy+01ch)		;600e	fd 4e 1c 	. N . 
	ld b,(iy+01dh)		;6011	fd 46 1d 	. F . 
	jr l601eh		;6014	18 08 	. . 
l6016h:
	ex (sp),iy		;6016	fd e3 	. . 
	ld c,(iy+01eh)		;6018	fd 4e 1e 	. N . 
	ld b,(iy+01fh)		;601b	fd 46 1f 	. F . 
l601eh:
	ex (sp),iy		;601e	fd e3 	. . 
	call sub_602ch		;6020	cd 2c 60 	. , ` 
	ex (sp),hl			;6023	e3 	. 
	push de			;6024	d5 	. 
	ld de,00020h		;6025	11 20 00 	.   . 
	add hl,de			;6028	19 	. 
	pop de			;6029	d1 	. 
	ex (sp),hl			;602a	e3 	. 
	ret			;602b	c9 	. 
sub_602ch:
	push bc			;602c	c5 	. 
	ret			;602d	c9 	. 
sub_602eh:
	push de			;602e	d5 	. 
	ret			;602f	c9 	. 
GET_NEXT_TOKEN:
	ld a,(ix+000h)		;6030	dd 7e 00 	. ~ . 
	cp 020h		;6033	fe 20 	.   
	ret nz			;6035	c0 	. 
	inc ix		;6036	dd 23 	. # 
	jr GET_NEXT_TOKEN		;6038	18 f6 	. . 
ASSERT_NEXT_IS_EQUAL:
	ld a,0efh		;603a	3e ef 	> . 
	jr ASSERT_NEXT_TOKEN_BE		;603c	18 0a 	. . 
ASSERT_NEXT_IS_COMMA:
	ld a,02ch		;603e	3e 2c 	> , 
	jr ASSERT_NEXT_TOKEN_BE		;6040	18 06 	. . 
ASSERT_NEXT_IS_PARENTESIS_OPENED:
	ld a,028h		;6042	3e 28 	> ( 
	jr ASSERT_NEXT_TOKEN_BE		;6044	18 02 	. . 
ASSERT_NEXT_IS_PARENTESIS_CLOSED:
	ld a,029h		;6046	3e 29 	> ) 
ASSERT_NEXT_TOKEN_BE:
	push af			;6048	f5 	. 
	call GET_NEXT_TOKEN		;6049	cd 30 60 	. 0 ` 
	pop af			;604c	f1 	. 
	cp (ix+000h)		;604d	dd be 00 	. . . 
	jp nz,04e16h		;6050	c2 16 4e 	. . N 
	inc ix		;6053	dd 23 	. # 
	ret			;6055	c9 	. 
WRITE_NEXT_BYTES_UNTIL_2_NOPS:
	ex (sp),ix		;6056	dd e3 	. . 
	push af			;6058	f5 	. 
l6059h:
	ld a,(ix+000h)		;6059	dd 7e 00 	. ~ . 
	inc ix		;605c	dd 23 	. # 
	and a			;605e	a7 	. 
	jr z,l6066h		;605f	28 05 	( . 
l6061h:
	call WRITE_OPCODE		;6061	cd b7 60 	. . ` 
	jr l6059h		;6064	18 f3 	. . 
l6066h:
	xor a			;6066	af 	. 
	cp (ix+000h)		;6067	dd be 00 	. . . 
	jr nz,l6061h		;606a	20 f5 	  . 
	cp (ix+001h)		;606c	dd be 01 	. . . 
	jr z,l6061h		;606f	28 f0 	( . 
	inc ix		;6071	dd 23 	. # 
	pop af			;6073	f1 	. 
	ex (sp),ix		;6074	dd e3 	. . 
	ret			;6076	c9 	. 
sub_6077h:
	push af			;6077	f5 	. 
	ld a,011h		;6078	3e 11 	> . 
	jr l607fh		;607a	18 03 	. . 
sub_607ch:
	push af			;607c	f5 	. 
	ld a,021h		;607d	3e 21 	> ! 
l607fh:
	call WRITE_OPCODE_A_L_H		;607f	cd a9 60 	. . ` 
	pop af			;6082	f1 	. 
	ret			;6083	c9 	. 
sub_6084h:
	push af			;6084	f5 	. 
	ld a,02ah		;6085	3e 2a 	> * 
	jr l608ch		;6087	18 03 	. . 
l6089h:
	push af			;6089	f5 	. 
	ld a,022h		;608a	3e 22 	> " 
l608ch:
	call WRITE_OPCODE		;608c	cd b7 60 	. . ` 
	call sub_62e5h		;608f	cd e5 62 	. . b 
	pop af			;6092	f1 	. 
	ret			;6093	c9 	. 
sub_6094h:
	push af			;6094	f5 	. 
	push hl			;6095	e5 	. 
	ld hl,05bedh		;6096	21 ed 5b 	! . [ 
	jr l60a0h		;6099	18 05 	. . 
	push af			;609b	f5 	. 
	push hl			;609c	e5 	. 
	ld hl,l53ech+1		;609d	21 ed 53 	! . S 
l60a0h:
	call sub_60ach		;60a0	cd ac 60 	. . ` 
	pop hl			;60a3	e1 	. 
	call sub_62e5h		;60a4	cd e5 62 	. . b 
	pop af			;60a7	f1 	. 
	ret			;60a8	c9 	. 
WRITE_OPCODE_A_L_H:
	call WRITE_OPCODE		;60a9	cd b7 60 	. . ` 
sub_60ach:
	push af			;60ac	f5 	. 
	ld a,l			;60ad	7d 	} 
	call WRITE_OPCODE		;60ae	cd b7 60 	. . ` 
	ld a,h			;60b1	7c 	| 
	call WRITE_OPCODE		;60b2	cd b7 60 	. . ` 
	pop af			;60b5	f1 	. 
	ret			;60b6	c9 	. 
WRITE_OPCODE:
	push hl			;60b7	e5 	. 
	ld hl,(0f560h)		;60b8	2a 60 f5 	* ` . 
	push hl			;60bb	e5 	. 
	push de			;60bc	d5 	. 
	ld de,(0f575h)		;60bd	ed 5b 75 f5 	. [ u . 
	and a			;60c1	a7 	. 
	sbc hl,de		;60c2	ed 52 	. R 
	pop de			;60c4	d1 	. 
	pop hl			;60c5	e1 	. 
	jp nc,04e1fh		;60c6	d2 1f 4e 	. . N 
	ld (hl),a			;60c9	77 	w 
	inc hl			;60ca	23 	# 
	ld (0f560h),hl		;60cb	22 60 f5 	" ` . 
	ld hl,VAR_FNPTR		;60ce	21 62 f5 	! b . 
	ld (hl),001h		;60d1	36 01 	6 . 
	pop hl			;60d3	e1 	. 
	ret			;60d4	c9 	. 
sub_60d5h:
	push hl			;60d5	e5 	. 
	ld hl,(0f560h)		;60d6	2a 60 f5 	* ` . 
	dec hl			;60d9	2b 	+ 
	ld (0f560h),hl		;60da	22 60 f5 	" ` . 
	pop hl			;60dd	e1 	. 
	ret			;60de	c9 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;60df	cd 56 60 	. V ` 
	ex de,hl			;60e2	eb 	. 
	ld c,b			;60e3	48 	H 
	nop			;60e4	00 	. 
	nop			;60e5	00 	. 
	ex de,hl			;60e6	eb 	. 
	call TOKEN_MULTI_FUNCTION		;60e7	cd 2d 78 	. - x 
	call l4fcah		;60ea	cd ca 4f 	. . O 
	jr l6139h		;60ed	18 4a 	. J 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;60ef	cd 56 60 	. V ` 
	push hl			;60f2	e5 	. 
	push bc			;60f3	c5 	. 
	nop			;60f4	00 	. 
	nop			;60f5	00 	. 
	ex de,hl			;60f6	eb 	. 
	call sub_6084h		;60f7	cd 84 60 	. . ` 
	jr l611ch		;60fa	18 20 	.   
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;60fc	cd 56 60 	. V ` 
	pop de			;60ff	d1 	. 
	push hl			;6100	e5 	. 
	push bc			;6101	c5 	. 
	ex de,hl			;6102	eb 	. 
	nop			;6103	00 	. 
	nop			;6104	00 	. 
	jr l611ch		;6105	18 15 	. . 
	call TOKEN_MULTI_FUNCTION		;6107	cd 2d 78 	. - x 
	ld a,00eh		;610a	3e 0e 	> . 
	call WRITE_OPCODE		;610c	cd b7 60 	. . ` 
	ld a,b			;610f	78 	x 
	call WRITE_OPCODE		;6110	cd b7 60 	. . ` 
	and a			;6113	a7 	. 
	call nz,sub_6077h		;6114	c4 77 60 	. w ` 
	jr l6139h		;6117	18 20 	.   
	call sub_6084h		;6119	cd 84 60 	. . ` 
l611ch:
	call sub_613eh		;611c	cd 3e 61 	. > a 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;611f	cd 56 60 	. V ` 
	pop af			;6122	f1 	. 
	ld c,a			;6123	4f 	O 
	pop de			;6124	d1 	. 
	nop			;6125	00 	. 
	nop			;6126	00 	. 
	jr l6139h		;6127	18 10 	. . 
	call sub_6084h		;6129	cd 84 60 	. . ` 
	call sub_613eh		;612c	cd 3e 61 	. > a 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;612f	cd 56 60 	. V ` 
	ld c,b			;6132	48 	H 
	ex de,hl			;6133	eb 	. 
	pop af			;6134	f1 	. 
	ld b,a			;6135	47 	G 
	pop hl			;6136	e1 	. 
	nop			;6137	00 	. 
	nop			;6138	00 	. 
l6139h:
	ld b,003h		;6139	06 03 	. . 
	jp l53e9h		;613b	c3 e9 53 	. . S 
sub_613eh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;613e	cd 56 60 	. V ` 
	call TOKEN_MULTI_FUNCTION		;6141	cd 2d 78 	. - x 
	nop			;6144	00 	. 
	nop			;6145	00 	. 
	ret			;6146	c9 	. 
	ld hl,l7d99h		;6147	21 99 7d 	! . } 
	jr l614fh		;614a	18 03 	. . 
	ld hl,l7da0h		;614c	21 a0 7d 	! . } 
l614fh:
	push hl			;614f	e5 	. 
	call sub_4ffeh		;6150	cd fe 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;6153	cd 3e 60 	. > ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6156	cd 56 60 	. V ` 
	push hl			;6159	e5 	. 
	nop			;615a	00 	. 
	nop			;615b	00 	. 
	call GET_NUM_PARAMETER		;615c	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;615f	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6162	cd 56 60 	. V ` 
	pop hl			;6165	e1 	. 
	nop			;6166	00 	. 
	nop			;6167	00 	. 
	pop hl			;6168	e1 	. 
	ld a,0cdh		;6169	3e cd 	> . 
	call WRITE_OPCODE_A_L_H		;616b	cd a9 60 	. . ` 
	ld b,004h		;616e	06 04 	. . 
	ret			;6170	c9 	. 
	call sub_4ffeh		;6171	cd fe 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;6174	cd 3e 60 	. > ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6177	cd 56 60 	. V ` 
	push hl			;617a	e5 	. 
	nop			;617b	00 	. 
	nop			;617c	00 	. 
	call GET_NUM_PARAMETER		;617d	cd 54 4f 	. T O 
	call GET_NEXT_TOKEN		;6180	cd 30 60 	. 0 ` 
	cp 02ch		;6183	fe 2c 	. , 
	jr nz,l619dh		;6185	20 16 	  . 
	inc ix		;6187	dd 23 	. # 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6189	cd 56 60 	. V ` 
	push af			;618c	f5 	. 
	nop			;618d	00 	. 
	nop			;618e	00 	. 
	call GET_NUM_PARAMETER		;618f	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6192	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6195	cd 56 60 	. V ` 
	pop bc			;6198	c1 	. 
	nop			;6199	00 	. 
	nop			;619a	00 	. 
	jr l61a8h		;619b	18 0b 	. . 
l619dh:
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;619d	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;61a0	cd 56 60 	. V ` 
	ld b,a			;61a3	47 	G 
	ld a,0ffh		;61a4	3e ff 	> . 
	nop			;61a6	00 	. 
	nop			;61a7	00 	. 
l61a8h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;61a8	cd 56 60 	. V ` 
	pop hl			;61ab	e1 	. 
	call sub_7db1h		;61ac	cd b1 7d 	. . } 
	nop			;61af	00 	. 
	nop			;61b0	00 	. 
	ld b,004h		;61b1	06 04 	. . 
	ret			;61b3	c9 	. 
l61b4h:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;61b4	cd 42 60 	. B ` 
	call sub_5bdch		;61b7	cd dc 5b 	. . [ 
	bit 5,a		;61ba	cb 6f 	. o 
	jp z,04e10h		;61bc	ca 10 4e 	. . N 
	call ASSERT_NEXT_IS_COMMA		;61bf	cd 3e 60 	. > ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;61c2	cd 56 60 	. V ` 
	push hl			;61c5	e5 	. 
	nop			;61c6	00 	. 
	nop			;61c7	00 	. 
	call GET_NUM_PARAMETER		;61c8	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;61cb	cd 56 60 	. V ` 
	push af			;61ce	f5 	. 
	nop			;61cf	00 	. 
	nop			;61d0	00 	. 
	call GET_NEXT_TOKEN		;61d1	cd 30 60 	. 0 ` 
	cp 02ch		;61d4	fe 2c 	. , 
	jr nz,l61dfh		;61d6	20 07 	  . 
	inc ix		;61d8	dd 23 	. # 
	call GET_NUM_PARAMETER		;61da	cd 54 4f 	. T O 
	jr l61e6h		;61dd	18 07 	. . 
l61dfh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;61df	cd 56 60 	. V ` 
	ld a,0ffh		;61e2	3e ff 	> . 
	nop			;61e4	00 	. 
	nop			;61e5	00 	. 
l61e6h:
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;61e6	cd 46 60 	. F ` 
	call ASSERT_NEXT_IS_EQUAL		;61e9	cd 3a 60 	. : ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;61ec	cd 56 60 	. V ` 
	push af			;61ef	f5 	. 
	nop			;61f0	00 	. 
	nop			;61f1	00 	. 
	call sub_4ffeh		;61f2	cd fe 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;61f5	cd 56 60 	. V ` 
	pop bc			;61f8	c1 	. 
	pop af			;61f9	f1 	. 
	pop de			;61fa	d1 	. 
	call sub_7dd8h		;61fb	cd d8 7d 	. . } 
	nop			;61fe	00 	. 
	nop			;61ff	00 	. 
	ret			;6200	c9 	. 
	ld hl,l7e1ah		;6201	21 1a 7e 	! . ~ 
	jr l620eh		;6204	18 08 	. . 
	ld hl,07e1dh		;6206	21 1d 7e 	! . ~ 
	jr l620eh		;6209	18 03 	. . 
	ld hl,07e20h		;620b	21 20 7e 	!   ~ 
l620eh:
	push hl			;620e	e5 	. 
	call sub_4f97h		;620f	cd 97 4f 	. . O 
	jr l6220h		;6212	18 0c 	. . 
	ld hl,l7e46h		;6214	21 46 7e 	! F ~ 
	jr l621ch		;6217	18 03 	. . 
	ld hl,TOKEN_13_STR		;6219	21 ff 7d 	! . } 
l621ch:
	push hl			;621c	e5 	. 
	call GET_NUM_PARAMETER		;621d	cd 54 4f 	. T O 
l6220h:
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6220	cd 46 60 	. F ` 
	pop hl			;6223	e1 	. 
	jr l6269h		;6224	18 43 	. C 
l6226h:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;6226	cd 42 60 	. B ` 
	call GET_NUM_PARAMETER		;6229	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_COMMA		;622c	cd 3e 60 	. > ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;622f	cd 56 60 	. V ` 
	push af			;6232	f5 	. 
	nop			;6233	00 	. 
	nop			;6234	00 	. 
	call sub_5012h		;6235	cd 12 50 	. . P 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6238	cd 46 60 	. F ` 
	ld a,b			;623b	78 	x 
	cp 004h		;623c	fe 04 	. . 
	jr z,l6245h		;623e	28 05 	( . 
	call sub_4f57h		;6240	cd 57 4f 	. W O 
	jr l624ch		;6243	18 07 	. . 
l6245h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6245	cd 56 60 	. V ` 
	inc hl			;6248	23 	# 
	ld a,(hl)			;6249	7e 	~ 
	nop			;624a	00 	. 
	nop			;624b	00 	. 
l624ch:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;624c	cd 56 60 	. V ` 
	pop bc			;624f	c1 	. 
	call sub_7e49h		;6250	cd 49 7e 	. I ~ 
	nop			;6253	00 	. 
	nop			;6254	00 	. 
	ld b,004h		;6255	06 04 	. . 
	ret			;6257	c9 	. 
	call sub_4fdah		;6258	cd da 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;625b	cd 46 60 	. F ` 
	ld a,b			;625e	78 	x 
	cp 002h		;625f	fe 02 	. . 
	ld hl,AUX_INT_TO_STR		;6261	21 26 7b 	! & { 
	jr z,l6269h		;6264	28 03 	( . 
	ld hl,AUX_FLOAT_TO_STR		;6266	21 80 7b 	! . { 
l6269h:
	ld a,0cdh		;6269	3e cd 	> . 
	call WRITE_OPCODE_A_L_H		;626b	cd a9 60 	. . ` 
	ld b,004h		;626e	06 04 	. . 
	ret			;6270	c9 	. 
	call sub_4ffeh		;6271	cd fe 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6274	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6277	cd 56 60 	. V ` 
	ld l,(hl)			;627a	6e 	n 
	ld h,000h		;627b	26 00 	& . 
	nop			;627d	00 	. 
	nop			;627e	00 	. 
	ld b,002h		;627f	06 02 	. . 
	ret			;6281	c9 	. 
	call sub_4ffeh		;6282	cd fe 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6285	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6288	cd 56 60 	. V ` 
	inc hl			;628b	23 	# 
	ld l,(hl)			;628c	6e 	n 
	ld h,000h		;628d	26 00 	& . 
	nop			;628f	00 	. 
	nop			;6290	00 	. 
	ld b,002h		;6291	06 02 	. . 
	ret			;6293	c9 	. 
	call sub_4ffeh		;6294	cd fe 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6297	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;629a	cd 56 60 	. V ` 
	call sub_7e07h		;629d	cd 07 7e 	. . ~ 
	nop			;62a0	00 	. 
	nop			;62a1	00 	. 
	ld b,003h		;62a2	06 03 	. . 
	ret			;62a4	c9 	. 
l62a5h:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;62a5	cd 42 60 	. B ` 
	call sub_5012h		;62a8	cd 12 50 	. . P 
	call ASSERT_NEXT_IS_COMMA		;62ab	cd 3e 60 	. > ` 
	ld a,b			;62ae	78 	x 
	cp 004h		;62af	fe 04 	. . 
	jr z,l62c4h		;62b1	28 11 	( . 
	call sub_4f57h		;62b3	cd 57 4f 	. W O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;62b6	cd 56 60 	. V ` 
	push af			;62b9	f5 	. 
	nop			;62ba	00 	. 
	nop			;62bb	00 	. 
	call sub_4ffeh		;62bc	cd fe 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;62bf	cd 3e 60 	. > ` 
	jr l62cch		;62c2	18 08 	. . 
l62c4h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;62c4	cd 56 60 	. V ` 
	ld a,001h		;62c7	3e 01 	> . 
	push af			;62c9	f5 	. 
	nop			;62ca	00 	. 
	nop			;62cb	00 	. 
l62cch:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;62cc	cd 56 60 	. V ` 
	push hl			;62cf	e5 	. 
	nop			;62d0	00 	. 
	nop			;62d1	00 	. 
	call sub_4ffeh		;62d2	cd fe 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;62d5	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;62d8	cd 56 60 	. V ` 
	pop de			;62db	d1 	. 
	pop af			;62dc	f1 	. 
	call sub_7e6ch		;62dd	cd 6c 7e 	. l ~ 
	nop			;62e0	00 	. 
	nop			;62e1	00 	. 
	ld b,002h		;62e2	06 02 	. . 
	ret			;62e4	c9 	. 
sub_62e5h:
	res 7,l		;62e5	cb bd 	. . 
	jr l62f1h		;62e7	18 08 	. . 
sub_62e9h:
	res 7,l		;62e9	cb bd 	. . 
	set 6,l		;62eb	cb f5 	. . 
	jr l62f3h		;62ed	18 04 	. . 
sub_62efh:
	set 7,l		;62ef	cb fd 	. . 
l62f1h:
	res 6,l		;62f1	cb b5 	. . 
l62f3h:
	res 5,l		;62f3	cb ad 	. . 
	res 7,h		;62f5	cb bc 	. . 
	jr l6319h		;62f7	18 20 	.   
sub_62f9h:
	res 7,l		;62f9	cb bd 	. . 
	res 6,l		;62fb	cb b5 	. . 
	jr l630dh		;62fd	18 0e 	. . 
sub_62ffh:
	res 7,l		;62ff	cb bd 	. . 
	jr l630bh		;6301	18 08 	. . 
sub_6303h:
	set 7,l		;6303	cb fd 	. . 
	res 6,l		;6305	cb b5 	. . 
	jr l630dh		;6307	18 04 	. . 
sub_6309h:
	set 7,l		;6309	cb fd 	. . 
l630bh:
	set 6,l		;630b	cb f5 	. . 
l630dh:
	res 5,l		;630d	cb ad 	. . 
	jr l6317h		;630f	18 06 	. . 
l6311h:
	set 7,l		;6311	cb fd 	. . 
	set 6,l		;6313	cb f5 	. . 
	set 5,l		;6315	cb ed 	. . 
l6317h:
	set 7,h		;6317	cb fc 	. . 
l6319h:
	push ix		;6319	dd e5 	. . 
BASIC_TRAP_ENABLE:
	push hl			;631b	e5 	. 
	push de			;631c	d5 	. 
	call sub_6325h		;631d	cd 25 63 	. % c 
	pop de			;6320	d1 	. 
	pop hl			;6321	e1 	. 
	pop ix		;6322	dd e1 	. . 
	ret			;6324	c9 	. 
sub_6325h:
	ld ix,(0f573h)		;6325	dd 2a 73 f5 	. * s . 
l6329h:
	ld a,(ix+000h)		;6329	dd 7e 00 	. ~ . 
	and a			;632c	a7 	. 
	jr z,l638dh		;632d	28 5e 	( ^ 
	cp l			;632f	bd 	. 
	ld a,(ix+001h)		;6330	dd 7e 01 	. ~ . 
	dec ix		;6333	dd 2b 	. + 
	dec ix		;6335	dd 2b 	. + 
	dec ix		;6337	dd 2b 	. + 
	dec ix		;6339	dd 2b 	. + 
	jr nz,l6329h		;633b	20 ec 	  . 
	cp h			;633d	bc 	. 
BASIC_TRAP_UNPAUSE:
	jr nz,l6329h		;633e	20 e9 	  . 
	bit 7,h		;6340	cb 7c 	. | 
	jr nz,l637dh		;6342	20 39 	  9 
	ld a,h			;6344	7c 	| 
	ld e,(ix+006h)		;6345	dd 5e 06 	. ^ . 
	ld d,(ix+007h)		;6348	dd 56 07 	. V . 
	push de			;634b	d5 	. 
	set 7,d		;634c	cb fa 	. . 
	ld hl,(BASIC_STREND)		;634e	2a c6 f6 	* . . 
	and a			;6351	a7 	. 
	sbc hl,de		;6352	ed 52 	. R 
	pop de			;6354	d1 	. 
	jr nc,l6379h		;6355	30 22 	0 " 
	bit 6,a		;6357	cb 77 	. w 
	ld hl,(0f560h)		;6359	2a 60 f5 	* ` . 
	jr z,l636fh		;635c	28 11 	( . 
	ld a,c			;635e	79 	y 
	and a			;635f	a7 	. 
	jr z,l636dh		;6360	28 0b 	( . 
	or 030h		;6362	f6 30 	. 0 
	call WRITE_OPCODE		;6364	cd b7 60 	. . ` 
	inc hl			;6367	23 	# 
	call l636fh		;6368	cd 6f 63 	. o c 
	jr l63cbh		;636b	18 5e 	. ^ 
l636dh:
	res 7,h		;636d	cb bc 	. . 
l636fh:
	ld (ix+006h),l		;636f	dd 75 06 	. u . 
	ld (ix+007h),h		;6372	dd 74 07 	. t . 
	ex de,hl			;6375	eb 	. 
	jp sub_60ach		;6376	c3 ac 60 	. . ` 
l6379h:
	ex de,hl			;6379	eb 	. 
	jp sub_60ach		;637a	c3 ac 60 	. . ` 
l637dh:
	bit 7,l		;637d	cb 7d 	. } 
	jr nz,l6386h		;637f	20 05 	  . 
	bit 6,l		;6381	cb 75 	. u 
	jp z,04e22h		;6383	ca 22 4e 	. " N 
l6386h:
	ld (ix+006h),e		;6386	dd 73 06 	. s . 
	ld (ix+007h),d		;6389	dd 72 07 	. r . 
	ret			;638c	c9 	. 
l638dh:
	ld (ix+000h),l		;638d	dd 75 00 	. u . 
	ld (ix+001h),h		;6390	dd 74 01 	. t . 
	push hl			;6393	e5 	. 
	push de			;6394	d5 	. 
	ld de,(0f560h)		;6395	ed 5b 60 f5 	. [ ` . 
	push ix		;6399	dd e5 	. . 
	pop hl			;639b	e1 	. 
	dec hl			;639c	2b 	+ 
	dec hl			;639d	2b 	+ 
	dec hl			;639e	2b 	+ 
	dec hl			;639f	2b 	+ 
	ld (0f575h),hl		;63a0	22 75 f5 	" u . 
	xor a			;63a3	af 	. 
	ld (hl),a			;63a4	77 	w 
	sbc hl,de		;63a5	ed 52 	. R 
	pop de			;63a7	d1 	. 
	pop hl			;63a8	e1 	. 
	jp c,04e1fh		;63a9	da 1f 4e 	. . N 
	bit 7,h		;63ac	cb 7c 	. | 
	jr z,l63b7h		;63ae	28 07 	( . 
	ld (ix+002h),e		;63b0	dd 73 02 	. s . 
	ld (ix+003h),d		;63b3	dd 72 03 	. r . 
	ret			;63b6	c9 	. 
l63b7h:
	bit 6,h		;63b7	cb 74 	. t 
	ld hl,(0f560h)		;63b9	2a 60 f5 	* ` . 
	jr z,l63d3h		;63bc	28 15 	( . 
	ld a,c			;63be	79 	y 
	and a			;63bf	a7 	. 
	jr z,l63d1h		;63c0	28 0f 	( . 
	or 030h		;63c2	f6 30 	. 0 
	call WRITE_OPCODE		;63c4	cd b7 60 	. . ` 
	inc hl			;63c7	23 	# 
	call l63d3h		;63c8	cd d3 63 	. . c 
l63cbh:
	ld a,c			;63cb	79 	y 
	or 020h		;63cc	f6 20 	.   
	jp WRITE_OPCODE_A_L_H		;63ce	c3 a9 60 	. . ` 
l63d1h:
	res 7,h		;63d1	cb bc 	. . 
l63d3h:
	ld (ix+002h),l		;63d3	dd 75 02 	. u . 
	ld (ix+003h),h		;63d6	dd 74 03 	. t . 
	ld hl,00000h		;63d9	21 00 00 	! . . 
	jp sub_60ach		;63dc	c3 ac 60 	. . ` 
sub_63dfh:
	res 7,h		;63df	cb bc 	. . 
	jr l63e5h		;63e1	18 02 	. . 
sub_63e3h:
	set 7,h		;63e3	cb fc 	. . 
l63e5h:
	res 7,l		;63e5	cb bd 	. . 
	res 6,l		;63e7	cb b5 	. . 
	res 5,l		;63e9	cb ad 	. . 
	push ix		;63eb	dd e5 	. . 
	push hl			;63ed	e5 	. 
	push de			;63ee	d5 	. 
	ld ix,(0f573h)		;63ef	dd 2a 73 f5 	. * s . 
l63f3h:
	ld a,(ix+000h)		;63f3	dd 7e 00 	. ~ . 
	and a			;63f6	a7 	. 
	jr z,l640fh		;63f7	28 16 	( . 
	cp l			;63f9	bd 	. 
	ld a,(ix+001h)		;63fa	dd 7e 01 	. ~ . 
	dec ix		;63fd	dd 2b 	. + 
	dec ix		;63ff	dd 2b 	. + 
	dec ix		;6401	dd 2b 	. + 
	dec ix		;6403	dd 2b 	. + 
	jr nz,l63f3h		;6405	20 ec 	  . 
	cp h			;6407	bc 	. 
	jr nz,l63f3h		;6408	20 e9 	  . 
	pop de			;640a	d1 	. 
	pop hl			;640b	e1 	. 
	pop ix		;640c	dd e1 	. . 
	ret			;640e	c9 	. 
l640fh:
	ld (ix+000h),l		;640f	dd 75 00 	. u . 
	ld (ix+001h),h		;6412	dd 74 01 	. t . 
	push hl			;6415	e5 	. 
	push de			;6416	d5 	. 
	ld de,(0f560h)		;6417	ed 5b 60 f5 	. [ ` . 
	push ix		;641b	dd e5 	. . 
	pop hl			;641d	e1 	. 
	dec hl			;641e	2b 	+ 
	dec hl			;641f	2b 	+ 
	dec hl			;6420	2b 	+ 
	dec hl			;6421	2b 	+ 
	ld (0f575h),hl		;6422	22 75 f5 	" u . 
	xor a			;6425	af 	. 
	ld (hl),a			;6426	77 	w 
	sbc hl,de		;6427	ed 52 	. R 
	pop de			;6429	d1 	. 
	pop hl			;642a	e1 	. 
	jp c,04e1fh		;642b	da 1f 4e 	. . N 
	ld (ix+002h),e		;642e	dd 73 02 	. s . 
	ld (ix+003h),d		;6431	dd 72 03 	. r . 
	pop de			;6434	d1 	. 
	pop hl			;6435	e1 	. 
	pop ix		;6436	dd e1 	. . 
	ret			;6438	c9 	. 
sub_6439h:
	res 7,l		;6439	cb bd 	. . 
	jr l6443h		;643b	18 06 	. . 
sub_643dh:
	res 7,l		;643d	cb bd 	. . 
	jr l6449h		;643f	18 08 	. . 
sub_6441h:
	set 7,l		;6441	cb fd 	. . 
l6443h:
	res 6,l		;6443	cb b5 	. . 
	jr l644bh		;6445	18 04 	. . 
sub_6447h:
	set 7,l		;6447	cb fd 	. . 
l6449h:
	set 6,l		;6449	cb f5 	. . 
l644bh:
	res 5,l		;644b	cb ad 	. . 
	jr l6455h		;644d	18 06 	. . 
sub_644fh:
	set 7,l		;644f	cb fd 	. . 
	set 6,l		;6451	cb f5 	. . 
	set 5,l		;6453	cb ed 	. . 
l6455h:
	set 7,h		;6455	cb fc 	. . 
	push ix		;6457	dd e5 	. . 
	ld ix,(0f573h)		;6459	dd 2a 73 f5 	. * s . 
l645dh:
	ld a,(ix+000h)		;645d	dd 7e 00 	. ~ . 
	and a			;6460	a7 	. 
	jr z,l647dh		;6461	28 1a 	( . 
	cp l			;6463	bd 	. 
	ld a,(ix+001h)		;6464	dd 7e 01 	. ~ . 
	dec ix		;6467	dd 2b 	. + 
	dec ix		;6469	dd 2b 	. + 
	dec ix		;646b	dd 2b 	. + 
	dec ix		;646d	dd 2b 	. + 
	jr nz,l645dh		;646f	20 ec 	  . 
	cp h			;6471	bc 	. 
	jr nz,l645dh		;6472	20 e9 	  . 
	ld e,(ix+006h)		;6474	dd 5e 06 	. ^ . 
	ld d,(ix+007h)		;6477	dd 56 07 	. V . 
	pop ix		;647a	dd e1 	. . 
	ret			;647c	c9 	. 
l647dh:
	bit 7,l		;647d	cb 7d 	. } 
	jp nz,04e1ch		;647f	c2 1c 4e 	. . N 
	bit 6,l		;6482	cb 75 	. u 
	jp nz,04e1ch		;6484	c2 1c 4e 	. . N 
	jp 04e25h		;6487	c3 25 4e 	. % N 
sub_648ah:
	ld ix,(0f575h)		;648a	dd 2a 75 f5 	. * u . 
	ld de,(0f560h)		;648e	ed 5b 60 f5 	. [ ` . 
l6492h:
	inc ix		;6492	dd 23 	. # 
	inc ix		;6494	dd 23 	. # 
	inc ix		;6496	dd 23 	. # 
	inc ix		;6498	dd 23 	. # 
	ld hl,(0f573h)		;649a	2a 73 f5 	* s . 
	push ix		;649d	dd e5 	. . 
	pop bc			;649f	c1 	. 
	and a			;64a0	a7 	. 
	sbc hl,bc		;64a1	ed 42 	. B 
	ret c			;64a3	d8 	. 
	bit 7,(ix+001h)		;64a4	dd cb 01 7e 	. . . ~ 
	jr nz,l6492h		;64a8	20 e8 	  . 
	ld l,(ix+002h)		;64aa	dd 6e 02 	. n . 
	ld h,(ix+003h)		;64ad	dd 66 03 	. f . 
	push hl			;64b0	e5 	. 
	set 7,h		;64b1	cb fc 	. . 
	ld bc,(BASIC_STREND)		;64b3	ed 4b c6 f6 	. K . . 
	sbc hl,bc		;64b7	ed 42 	. B 
	pop hl			;64b9	e1 	. 
	jr c,l6492h		;64ba	38 d6 	8 . 
	bit 6,(ix+001h)		;64bc	dd cb 01 76 	. . . v 
	jr nz,l64e2h		;64c0	20 20 	    
	xor a			;64c2	af 	. 
	ld (de),a			;64c3	12 	. 
	inc de			;64c4	13 	. 
	ld (de),a			;64c5	12 	. 
	dec de			;64c6	1b 	. 
l64c7h:
	ld c,(hl)			;64c7	4e 	N 
	inc hl			;64c8	23 	# 
	ld b,(hl)			;64c9	46 	F 
	ld (hl),d			;64ca	72 	r 
	dec hl			;64cb	2b 	+ 
	ld (hl),e			;64cc	73 	s 
	ld l,c			;64cd	69 	i 
	ld h,b			;64ce	60 	` 
	ld a,l			;64cf	7d 	} 
	or h			;64d0	b4 	. 
	jr nz,l64c7h		;64d1	20 f4 	  . 
	bit 5,(ix+001h)		;64d3	dd cb 01 6e 	. . . n 
	jr z,l64deh		;64d7	28 05 	( . 
	ld hl,000feh		;64d9	21 fe 00 	! . . 
	add hl,de			;64dc	19 	. 
	ex de,hl			;64dd	eb 	. 
l64deh:
	inc de			;64de	13 	. 
	inc de			;64df	13 	. 
	jr l6492h		;64e0	18 b0 	. . 
l64e2h:
	xor a			;64e2	af 	. 
	ld (de),a			;64e3	12 	. 
l64e4h:
	bit 7,h		;64e4	cb 7c 	. | 
	set 7,h		;64e6	cb fc 	. . 
	ld c,(hl)			;64e8	4e 	N 
	inc hl			;64e9	23 	# 
	ld b,(hl)			;64ea	46 	F 
	dec hl			;64eb	2b 	+ 
	ld (hl),e			;64ec	73 	s 
	inc hl			;64ed	23 	# 
	ld (hl),d			;64ee	72 	r 
	jr z,l64f8h		;64ef	28 07 	( . 
	inc hl			;64f1	23 	# 
	inc hl			;64f2	23 	# 
	inc de			;64f3	13 	. 
	ld (hl),e			;64f4	73 	s 
	inc hl			;64f5	23 	# 
	ld (hl),d			;64f6	72 	r 
	dec de			;64f7	1b 	. 
l64f8h:
	ld l,c			;64f8	69 	i 
	ld h,b			;64f9	60 	` 
	ld a,l			;64fa	7d 	} 
	or h			;64fb	b4 	. 
	jr nz,l64e4h		;64fc	20 e6 	  . 
	inc de			;64fe	13 	. 
	jr l64deh		;64ff	18 dd 	. . 
GET_VAR_PARAMETER:
	call GET_NEXT_TOKEN		;6501	cd 30 60 	. 0 ` 
	call sub_7d90h		;6504	cd 90 7d 	. . } 
	cp 041h		;6507	fe 41 	. A 
	jp c,04e16h		;6509	da 16 4e 	. . N 
	cp 05bh		;650c	fe 5b 	. [ 
	jp nc,04e16h		;650e	d2 16 4e 	. . N 
	ld l,a			;6511	6f 	o 
	ld h,000h		;6512	26 00 	& . 
l6514h:
	inc ix		;6514	dd 23 	. # 
	call GET_NEXT_TOKEN		;6516	cd 30 60 	. 0 ` 
	cp 021h		;6519	fe 21 	. ! 
	jr z,l656bh		;651b	28 4e 	( N 
	cp 023h		;651d	fe 23 	. # 
	jr z,l656bh		;651f	28 4a 	( J 
	cp 025h		;6521	fe 25 	. % 
	jr z,l654ch		;6523	28 27 	( ' 
	cp 024h		;6525	fe 24 	. $ 
	jr z,l6543h		;6527	28 1a 	( . 
	call sub_7d90h		;6529	cd 90 7d 	. . } 
	cp 030h		;652c	fe 30 	. 0 
	jr c,l6558h		;652e	38 28 	8 ( 
	cp 03ah		;6530	fe 3a 	. : 
	jr c,l653ch		;6532	38 08 	8 . 
	cp 041h		;6534	fe 41 	. A 
	jr c,l6558h		;6536	38 20 	8   
	cp 05bh		;6538	fe 5b 	. [ 
	jr nc,l6558h		;653a	30 1c 	0 . 
l653ch:
	inc h			;653c	24 	$ 
	dec h			;653d	25 	% 
	jr nz,l6514h		;653e	20 d4 	  . 
	ld h,a			;6540	67 	g 
	jr l6514h		;6541	18 d1 	. . 
l6543h:
	call sub_6574h		;6543	cd 74 65 	. t e 
	res 6,h		;6546	cb b4 	. . 
	set 5,h		;6548	cb ec 	. . 
	jr l6553h		;654a	18 07 	. . 
l654ch:
	call sub_6574h		;654c	cd 74 65 	. t e 
	res 6,h		;654f	cb b4 	. . 
	res 5,h		;6551	cb ac 	. . 
l6553h:
	inc ix		;6553	dd 23 	. # 
	jp GET_NEXT_TOKEN		;6555	c3 30 60 	. 0 ` 
l6558h:
	push hl			;6558	e5 	. 
	push bc			;6559	c5 	. 
	ld bc,0f689h		;655a	01 89 f6 	. . . 
	ld h,000h		;655d	26 00 	& . 
	add hl,bc			;655f	09 	. 
	ld a,(hl)			;6560	7e 	~ 
	pop bc			;6561	c1 	. 
	pop hl			;6562	e1 	. 
	dec ix		;6563	dd 2b 	. + 
	sub 003h		;6565	d6 03 	. . 
	jr z,l6543h		;6567	28 da 	( . 
	jr c,l654ch		;6569	38 e1 	8 . 
l656bh:
	call sub_6574h		;656b	cd 74 65 	. t e 
	set 6,h		;656e	cb f4 	. . 
	res 5,h		;6570	cb ac 	. . 
	jr l6553h		;6572	18 df 	. . 
sub_6574h:
	push de			;6574	d5 	. 
	push bc			;6575	c5 	. 
	ld (BASIC_SWPTMP),hl		;6576	22 bc f7 	" . . 
	push hl			;6579	e5 	. 
	ld a,l			;657a	7d 	} 
	sub 041h		;657b	d6 41 	. A 
	ld l,a			;657d	6f 	o 
	ld h,000h		;657e	26 00 	& . 
	ld de,00025h		;6580	11 25 00 	. % . 
	call sub_761bh		;6583	cd 1b 76 	. . v 
	pop af			;6586	f1 	. 
	and a			;6587	a7 	. 
	jr z,l6592h		;6588	28 08 	( . 
	cp 041h		;658a	fe 41 	. A 
	jr c,l6590h		;658c	38 02 	8 . 
	sub 007h		;658e	d6 07 	. . 
l6590h:
	sub 02fh		;6590	d6 2f 	. / 
l6592h:
	ld e,a			;6592	5f 	_ 
	ld d,000h		;6593	16 00 	. . 
	add hl,de			;6595	19 	. 
	ld de,0001fh		;6596	11 1f 00 	. . . 
	call l762dh		;6599	cd 2d 76 	. - v 
	ld h,l			;659c	65 	e 
	ld l,e			;659d	6b 	k 
	inc l			;659e	2c 	, 
	pop bc			;659f	c1 	. 
	pop de			;65a0	d1 	. 
	ret			;65a1	c9 	. 
TOKEN_BD_COLOR:
	call sub_4f00h		;65a2	cd 00 4f 	. . O 
	jp z,l6681h		;65a5	ca 81 66 	. . f 
	cp 0c7h		;65a8	fe c7 	. . 
	jp z,l669bh		;65aa	ca 9b 66 	. . f 
	cp 0efh		;65ad	fe ef 	. . 
	jp nz,l663ah		;65af	c2 3a 66 	. : f 
	inc ix		;65b2	dd 23 	. # 
	call GET_NEXT_TOKEN		;65b4	cd 30 60 	. 0 ` 
	inc ix		;65b7	dd 23 	. # 
	cp 094h		;65b9	fe 94 	. . 
	jp z,l6681h		;65bb	ca 81 66 	. . f 
	cp 08ch		;65be	fe 8c 	. . 
	jp z,l668eh		;65c0	ca 8e 66 	. . f 
	cp 028h		;65c3	fe 28 	. ( 
	jp nz,04e16h		;65c5	c2 16 4e 	. . N 
	call GET_NUM_PARAMETER		;65c8	cd 54 4f 	. T O 
	call sub_6a6dh		;65cb	cd 6d 6a 	. m j 
	call ASSERT_NEXT_IS_COMMA		;65ce	cd 3e 60 	. > ` 
	call GET_NEXT_TOKEN		;65d1	cd 30 60 	. 0 ` 
	cp 02ch		;65d4	fe 2c 	. , 
	jr z,l65e7h		;65d6	28 0f 	( . 
	call GET_NUM_PARAMETER		;65d8	cd 54 4f 	. T O 
	call sub_6a6dh		;65db	cd 6d 6a 	. m j 
	call GET_NEXT_TOKEN		;65de	cd 30 60 	. 0 ` 
	cp 02ch		;65e1	fe 2c 	. , 
	jr nz,l661ch		;65e3	20 37 	  7 
	jr l65efh		;65e5	18 08 	. . 
l65e7h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;65e7	cd 56 60 	. V ` 
	ld a,0ffh		;65ea	3e ff 	> . 
	push af			;65ec	f5 	. 
	nop			;65ed	00 	. 
	nop			;65ee	00 	. 
l65efh:
	inc ix		;65ef	dd 23 	. # 
	call GET_NEXT_TOKEN		;65f1	cd 30 60 	. 0 ` 
	cp 02ch		;65f4	fe 2c 	. , 
	jr z,l6607h		;65f6	28 0f 	( . 
	call GET_NUM_PARAMETER		;65f8	cd 54 4f 	. T O 
	call sub_6a6dh		;65fb	cd 6d 6a 	. m j 
	call GET_NEXT_TOKEN		;65fe	cd 30 60 	. 0 ` 
	cp 02ch		;6601	fe 2c 	. , 
	jr nz,l6624h		;6603	20 1f 	  . 
	jr l660fh		;6605	18 08 	. . 
l6607h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6607	cd 56 60 	. V ` 
	ld a,0ffh		;660a	3e ff 	> . 
	push af			;660c	f5 	. 
	nop			;660d	00 	. 
	nop			;660e	00 	. 
l660fh:
	inc ix		;660f	dd 23 	. # 
	call GET_NUM_PARAMETER		;6611	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6614	cd 56 60 	. V ` 
	ld b,a			;6617	47 	G 
	nop			;6618	00 	. 
	nop			;6619	00 	. 
	jr l662bh		;661a	18 0f 	. . 
l661ch:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;661c	cd 56 60 	. V ` 
	ld a,0ffh		;661f	3e ff 	> . 
	push af			;6621	f5 	. 
	nop			;6622	00 	. 
	nop			;6623	00 	. 
l6624h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6624	cd 56 60 	. V ` 
	ld b,0ffh		;6627	06 ff 	. . 
	nop			;6629	00 	. 
	nop			;662a	00 	. 
l662bh:
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;662b	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;662e	cd 56 60 	. V ` 
	pop hl			;6631	e1 	. 
	pop de			;6632	d1 	. 
	pop af			;6633	f1 	. 
	call sub_710ch		;6634	cd 0c 71 	. . q 
	nop			;6637	00 	. 
	nop			;6638	00 	. 
	ret			;6639	c9 	. 
l663ah:
	cp 02ch		;663a	fe 2c 	. , 
	jr z,l6650h		;663c	28 12 	( . 
	call GET_NUM_PARAMETER		;663e	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6641	cd 56 60 	. V ` 
	ld (BIOS_FORCLR),a		;6644	32 e9 f3 	2 . . 
	nop			;6647	00 	. 
	nop			;6648	00 	. 
	call GET_NEXT_TOKEN		;6649	cd 30 60 	. 0 ` 
	cp 02ch		;664c	fe 2c 	. , 
	jr nz,WRITE_CMD_COLOR		;664e	20 28 	  ( 
l6650h:
	inc ix		;6650	dd 23 	. # 
	call GET_NEXT_TOKEN		;6652	cd 30 60 	. 0 ` 
	cp 02ch		;6655	fe 2c 	. , 
	jr z,l666bh		;6657	28 12 	( . 
	call GET_NUM_PARAMETER		;6659	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;665c	cd 56 60 	. V ` 
	ld (BIOS_BAKCLR),a		;665f	32 ea f3 	2 . . 
	nop			;6662	00 	. 
	nop			;6663	00 	. 
	call GET_NEXT_TOKEN		;6664	cd 30 60 	. 0 ` 
	cp 02ch		;6667	fe 2c 	. , 
	jr nz,WRITE_CMD_COLOR		;6669	20 0d 	  . 
l666bh:
	inc ix		;666b	dd 23 	. # 
	call GET_NUM_PARAMETER		;666d	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6670	cd 56 60 	. V ` 
	ld (BIOS_BDRCLR),a		;6673	32 eb f3 	2 . . 
	nop			;6676	00 	. 
	nop			;6677	00 	. 
WRITE_CMD_COLOR:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6678	cd 56 60 	. V ` 
	call BIOS_CHGCLR		;667b	cd 62 00 	. b . 
	nop			;667e	00 	. 
	nop			;667f	00 	. 
	ret			;6680	c9 	. 
l6681h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6681	cd 56 60 	. V ` 
	ld ix,00141h		;6684	dd 21 41 01 	. ! A . 
	call BIOS_EXTROM		;6688	cd 5f 01 	. _ . 
	nop			;668b	00 	. 
	nop			;668c	00 	. 
	ret			;668d	c9 	. 
l668eh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;668e	cd 56 60 	. V ` 
	ld ix,00145h		;6691	dd 21 45 01 	. ! E . 
	call BIOS_EXTROM		;6695	cd 5f 01 	. _ . 
	nop			;6698	00 	. 
	nop			;6699	00 	. 
	ret			;669a	c9 	. 
l669bh:
	inc ix		;669b	dd 23 	. # 
	call GET_NEXT_TOKEN		;669d	cd 30 60 	. 0 ` 
	cp 024h		;66a0	fe 24 	. $ 
	push af			;66a2	f5 	. 
	jr nz,l66a7h		;66a3	20 02 	  . 
	inc ix		;66a5	dd 23 	. # 
l66a7h:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;66a7	cd 42 60 	. B ` 
	call GET_NUM_PARAMETER		;66aa	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;66ad	cd 46 60 	. F ` 
	call ASSERT_NEXT_IS_EQUAL		;66b0	cd 3a 60 	. : ` 
	call sub_6a6dh		;66b3	cd 6d 6a 	. m j 
	pop af			;66b6	f1 	. 
	jr z,l66c6h		;66b7	28 0d 	( . 
	call GET_NUM_PARAMETER		;66b9	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;66bc	cd 56 60 	. V ` 
	pop bc			;66bf	c1 	. 
	call sub_7187h		;66c0	cd 87 71 	. . q 
	nop			;66c3	00 	. 
	nop			;66c4	00 	. 
	ret			;66c5	c9 	. 
l66c6h:
	call sub_4ffeh		;66c6	cd fe 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;66c9	cd 56 60 	. V ` 
	pop bc			;66cc	c1 	. 
	call sub_7192h		;66cd	cd 92 71 	. . q 
	nop			;66d0	00 	. 
	nop			;66d1	00 	. 
	ret			;66d2	c9 	. 
TOKEN_AF_LINE:
	call GET_NEXT_TOKEN		;66d3	cd 30 60 	. 0 ` 
	cp 0f2h		;66d6	fe f2 	. . 
	jr nz,l66ebh		;66d8	20 11 	  . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;66da	cd 56 60 	. V ` 
	ld hl,(BIOS_GRPACX)		;66dd	2a b7 fc 	* . . 
	push hl			;66e0	e5 	. 
	ld hl,(BIOS_GRPACY)		;66e1	2a b9 fc 	* . . 
	push hl			;66e4	e5 	. 
	nop			;66e5	00 	. 
	nop			;66e6	00 	. 
	inc ix		;66e7	dd 23 	. # 
	jr l66f9h		;66e9	18 0e 	. . 
l66ebh:
	call sub_6b89h		;66eb	cd 89 6b 	. . k 
	ld a,0f2h		;66ee	3e f2 	> . 
	call ASSERT_NEXT_TOKEN_BE		;66f0	cd 48 60 	. H ` 
	call sub_60d5h		;66f3	cd d5 60 	. . ` 
	call sub_6a68h		;66f6	cd 68 6a 	. h j 
l66f9h:
	call sub_6b89h		;66f9	cd 89 6b 	. . k 
	call GET_NEXT_TOKEN		;66fc	cd 30 60 	. 0 ` 
	cp 02ch		;66ff	fe 2c 	. , 
	jr nz,l673bh		;6701	20 38 	  8 
	inc ix		;6703	dd 23 	. # 
	call GET_NEXT_TOKEN		;6705	cd 30 60 	. 0 ` 
	cp 02ch		;6708	fe 2c 	. , 
	jr z,l673bh		;670a	28 2f 	( / 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;670c	cd 56 60 	. V ` 
	push de			;670f	d5 	. 
	push hl			;6710	e5 	. 
	nop			;6711	00 	. 
	nop			;6712	00 	. 
	call sub_4fa9h		;6713	cd a9 4f 	. . O 
	inc b			;6716	04 	. 
	djnz l6724h		;6717	10 0b 	. . 
	call sub_60d5h		;6719	cd d5 60 	. . ` 
	call sub_60d5h		;671c	cd d5 60 	. . ` 
	call sub_4f5ah		;671f	cd 5a 4f 	. Z O 
	jr l6743h		;6722	18 1f 	. . 
l6724h:
	djnz l6731h		;6724	10 0b 	. . 
	call sub_60d5h		;6726	cd d5 60 	. . ` 
	call sub_60d5h		;6729	cd d5 60 	. . ` 
	call sub_4f75h		;672c	cd 75 4f 	. u O 
	jr l6743h		;672f	18 12 	. . 
l6731h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6731	cd 56 60 	. V ` 
	ld a,l			;6734	7d 	} 
	pop hl			;6735	e1 	. 
	pop de			;6736	d1 	. 
	nop			;6737	00 	. 
	nop			;6738	00 	. 
	jr l6743h		;6739	18 08 	. . 
l673bh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;673b	cd 56 60 	. V ` 
	ld a,(BIOS_FORCLR)		;673e	3a e9 f3 	: . . 
	nop			;6741	00 	. 
	nop			;6742	00 	. 
l6743h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6743	cd 56 60 	. V ` 
	pop iy		;6746	fd e1 	. . 
	pop ix		;6748	dd e1 	. . 
	nop			;674a	00 	. 
	nop			;674b	00 	. 
	call GET_NEXT_TOKEN		;674c	cd 30 60 	. 0 ` 
	cp 02ch		;674f	fe 2c 	. , 
	ld de,CMD_LINE		;6751	11 a7 6d 	. . m 
	jr nz,l6775h		;6754	20 1f 	  . 
	inc ix		;6756	dd 23 	. # 
	call GET_NEXT_TOKEN		;6758	cd 30 60 	. 0 ` 
	cp 02ch		;675b	fe 2c 	. , 
	jr z,l6775h		;675d	28 16 	( . 
	cp 042h		;675f	fe 42 	. B 
	jp nz,04e16h		;6761	c2 16 4e 	. . N 
	inc ix		;6764	dd 23 	. # 
	call GET_NEXT_TOKEN		;6766	cd 30 60 	. 0 ` 
	cp 046h		;6769	fe 46 	. F 
	ld de,l6d49h		;676b	11 49 6d 	. I m 
	jr nz,l6775h		;676e	20 05 	  . 
	inc ix		;6770	dd 23 	. # 
	ld de,l6e27h		;6772	11 27 6e 	. ' n 
l6775h:
	jp l67cfh		;6775	c3 cf 67 	. . g 
TOKEN_C2_PSET:
	or 0afh		;6778	f6 af 	. . 
	push af			;677a	f5 	. 
	call sub_6b89h		;677b	cd 89 6b 	. . k 
	call GET_NEXT_TOKEN		;677e	cd 30 60 	. 0 ` 
	cp 02ch		;6781	fe 2c 	. , 
	jr nz,l67beh		;6783	20 39 	  9 
	inc ix		;6785	dd 23 	. # 
	call GET_NEXT_TOKEN		;6787	cd 30 60 	. 0 ` 
	cp 02ch		;678a	fe 2c 	. , 
	jr z,l67beh		;678c	28 30 	( 0 
	pop af			;678e	f1 	. 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;678f	cd 56 60 	. V ` 
	push hl			;6792	e5 	. 
	push de			;6793	d5 	. 
	nop			;6794	00 	. 
	nop			;6795	00 	. 
	call sub_4fa9h		;6796	cd a9 4f 	. . O 
	inc b			;6799	04 	. 
	djnz l67a7h		;679a	10 0b 	. . 
	call sub_60d5h		;679c	cd d5 60 	. . ` 
	call sub_60d5h		;679f	cd d5 60 	. . ` 
	call sub_4f5ah		;67a2	cd 5a 4f 	. Z O 
	jr l67cch		;67a5	18 25 	. % 
l67a7h:
	djnz l67b4h		;67a7	10 0b 	. . 
	call sub_60d5h		;67a9	cd d5 60 	. . ` 
	call sub_60d5h		;67ac	cd d5 60 	. . ` 
	call sub_4f75h		;67af	cd 75 4f 	. u O 
	jr l67cch		;67b2	18 18 	. . 
l67b4h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;67b4	cd 56 60 	. V ` 
	ld a,l			;67b7	7d 	} 
	pop de			;67b8	d1 	. 
	pop hl			;67b9	e1 	. 
	nop			;67ba	00 	. 
	nop			;67bb	00 	. 
	jr l67cch		;67bc	18 0e 	. . 
l67beh:
	pop af			;67be	f1 	. 
	ld hl,BIOS_FORCLR		;67bf	21 e9 f3 	! . . 
	jr nz,l67c7h		;67c2	20 03 	  . 
	ld hl,BIOS_BAKCLR		;67c4	21 ea f3 	! . . 
l67c7h:
	ld a,03ah		;67c7	3e 3a 	> : 
	call WRITE_OPCODE_A_L_H		;67c9	cd a9 60 	. . ` 
l67cch:
	ld de,l6f71h		;67cc	11 71 6f 	. q o 
l67cfh:
	call sub_67d8h		;67cf	cd d8 67 	. . g 
	ex de,hl			;67d2	eb 	. 
WRITE_CALL:
	ld a,0cdh		;67d3	3e cd 	> . 
	jp WRITE_OPCODE_A_L_H		;67d5	c3 a9 60 	. . ` 
sub_67d8h:
	call GET_NEXT_TOKEN		;67d8	cd 30 60 	. 0 ` 
	cp 02ch		;67db	fe 2c 	. , 
	jr nz,l680fh		;67dd	20 30 	  0 
	inc ix		;67df	dd 23 	. # 
	call GET_NEXT_TOKEN		;67e1	cd 30 60 	. 0 ` 
	inc ix		;67e4	dd 23 	. # 
	cp 054h		;67e6	fe 54 	. T 
	jr z,l6816h		;67e8	28 2c 	( , 
	cp 0d9h		;67ea	fe d9 	. . 
	jr z,l6830h		;67ec	28 42 	( B 
	cp 0ffh		;67ee	fe ff 	. . 
	jr z,l683eh		;67f0	28 4c 	( L 
	cp 0f8h		;67f2	fe f8 	. . 
	ld h,003h		;67f4	26 03 	& . 
	jr z,l6811h		;67f6	28 19 	( . 
	cp 0f7h		;67f8	fe f7 	. . 
	ld h,002h		;67fa	26 02 	& . 
	jr z,l6811h		;67fc	28 13 	( . 
	cp 0f6h		;67fe	fe f6 	. . 
	ld h,001h		;6800	26 01 	& . 
	jr z,l6811h		;6802	28 0d 	( . 
	cp 0c3h		;6804	fe c3 	. . 
	ld h,004h		;6806	26 04 	& . 
	jr z,l6811h		;6808	28 07 	( . 
	cp 0c2h		;680a	fe c2 	. . 
	jp nz,04e16h		;680c	c2 16 4e 	. . N 
l680fh:
	ld h,000h		;680f	26 00 	& . 
l6811h:
	ld l,006h		;6811	2e 06 	. . 
	jp sub_60ach		;6813	c3 ac 60 	. . ` 
l6816h:
	call GET_NEXT_TOKEN		;6816	cd 30 60 	. 0 ` 
	inc ix		;6819	dd 23 	. # 
	cp 0f8h		;681b	fe f8 	. . 
	ld h,00bh		;681d	26 0b 	& . 
	jr z,l6811h		;681f	28 f0 	( . 
	cp 0c3h		;6821	fe c3 	. . 
	ld h,00ch		;6823	26 0c 	& . 
	jr z,l6811h		;6825	28 ea 	( . 
	cp 0c2h		;6827	fe c2 	. . 
	ld h,008h		;6829	26 08 	& . 
	jr z,l6811h		;682b	28 e4 	( . 
	jp 04e16h		;682d	c3 16 4e 	. . N 
l6830h:
	call GET_NEXT_TOKEN		;6830	cd 30 60 	. 0 ` 
	inc ix		;6833	dd 23 	. # 
	cp 052h		;6835	fe 52 	. R 
	ld h,00ah		;6837	26 0a 	& . 
	jr z,l6811h		;6839	28 d6 	( . 
	jp 04e16h		;683b	c3 16 4e 	. . N 
l683eh:
	ld a,08dh		;683e	3e 8d 	> . 
	call ASSERT_NEXT_TOKEN_BE		;6840	cd 48 60 	. H ` 
	call GET_NEXT_TOKEN		;6843	cd 30 60 	. 0 ` 
	inc ix		;6846	dd 23 	. # 
	cp 044h		;6848	fe 44 	. D 
	ld h,009h		;684a	26 09 	& . 
	jr z,l6811h		;684c	28 c3 	( . 
	jp 04e16h		;684e	c3 16 4e 	. . N 
TOKEN_BC_CIRCLE:
	call sub_6b89h		;6851	cd 89 6b 	. . k 
	call ASSERT_NEXT_IS_COMMA		;6854	cd 3e 60 	. > ` 
	call sub_4f97h		;6857	cd 97 4f 	. . O 
	call GET_NEXT_TOKEN		;685a	cd 30 60 	. 0 ` 
	cp 02ch		;685d	fe 2c 	. , 
	jr nz,l6870h		;685f	20 0f 	  . 
	inc ix		;6861	dd 23 	. # 
	call sub_6a68h		;6863	cd 68 6a 	. h j 
	call GET_NUM_PARAMETER		;6866	cd 54 4f 	. T O 
	ld a,0e1h		;6869	3e e1 	> . 
	call WRITE_OPCODE		;686b	cd b7 60 	. . ` 
	jr WRITE_CMD_CIRCLE		;686e	18 08 	. . 
l6870h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6870	cd 56 60 	. V ` 
	ld a,(BIOS_FORCLR)		;6873	3a e9 f3 	: . . 
	nop			;6876	00 	. 
	nop			;6877	00 	. 
WRITE_CMD_CIRCLE:
	ld hl,CMD_CIRCLE		;6878	21 d3 74 	! . t 
	jp WRITE_CALL		;687b	c3 d3 67 	. . g 
TOKEN_B3_PUT:
	ld a,0c7h		;687e	3e c7 	> . 
	call ASSERT_NEXT_TOKEN_BE		;6880	cd 48 60 	. H ` 
	xor a			;6883	af 	. 
	push af			;6884	f5 	. 
	call GET_NUM_PARAMETER		;6885	cd 54 4f 	. T O 
	call sub_6a6dh		;6888	cd 6d 6a 	. m j 
	call ASSERT_NEXT_IS_COMMA		;688b	cd 3e 60 	. > ` 
	call GET_NEXT_TOKEN		;688e	cd 30 60 	. 0 ` 
	cp 02ch		;6891	fe 2c 	. , 
	jr z,l68abh		;6893	28 16 	( . 
	call sub_6b89h		;6895	cd 89 6b 	. . k 
	call sub_60d5h		;6898	cd d5 60 	. . ` 
	call sub_6a68h		;689b	cd 68 6a 	. h j 
	pop af			;689e	f1 	. 
	or 080h		;689f	f6 80 	. . 
	push af			;68a1	f5 	. 
	call GET_NEXT_TOKEN		;68a2	cd 30 60 	. 0 ` 
	cp 02ch		;68a5	fe 2c 	. , 
	jr z,l68b2h		;68a7	28 09 	( . 
	jr l68e2h		;68a9	18 37 	. 7 
l68abh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;68ab	cd 56 60 	. V ` 
	push af			;68ae	f5 	. 
	push af			;68af	f5 	. 
	nop			;68b0	00 	. 
	nop			;68b1	00 	. 
l68b2h:
	inc ix		;68b2	dd 23 	. # 
	call GET_NEXT_TOKEN		;68b4	cd 30 60 	. 0 ` 
	cp 02ch		;68b7	fe 2c 	. , 
	jr z,l68ceh		;68b9	28 13 	( . 
	call GET_NUM_PARAMETER		;68bb	cd 54 4f 	. T O 
	call sub_6a6dh		;68be	cd 6d 6a 	. m j 
	pop af			;68c1	f1 	. 
	or 040h		;68c2	f6 40 	. @ 
	push af			;68c4	f5 	. 
	call GET_NEXT_TOKEN		;68c5	cd 30 60 	. 0 ` 
	cp 02ch		;68c8	fe 2c 	. , 
	jr z,l68d1h		;68ca	28 05 	( . 
	jr l68e5h		;68cc	18 17 	. . 
l68ceh:
	call sub_6a6dh		;68ce	cd 6d 6a 	. m j 
l68d1h:
	inc ix		;68d1	dd 23 	. # 
	call GET_NUM_PARAMETER		;68d3	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;68d6	cd 56 60 	. V ` 
	ld h,a			;68d9	67 	g 
	nop			;68da	00 	. 
	nop			;68db	00 	. 
	pop af			;68dc	f1 	. 
	or 020h		;68dd	f6 20 	.   
	push af			;68df	f5 	. 
	jr l68e5h		;68e0	18 03 	. . 
l68e2h:
	call sub_6a6dh		;68e2	cd 6d 6a 	. m j 
l68e5h:
	pop hl			;68e5	e1 	. 
	ld l,006h		;68e6	2e 06 	. . 
	call sub_60ach		;68e8	cd ac 60 	. . ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;68eb	cd 56 60 	. V ` 
	pop de			;68ee	d1 	. 
	pop iy		;68ef	fd e1 	. . 
	pop ix		;68f1	dd e1 	. . 
	pop af			;68f3	f1 	. 
	call sub_71bdh		;68f4	cd bd 71 	. . q 
	nop			;68f7	00 	. 
	nop			;68f8	00 	. 
	ret			;68f9	c9 	. 
l68fah:
	inc ix		;68fa	dd 23 	. # 
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;68fc	cd 42 60 	. B ` 
	call GET_NUM_PARAMETER		;68ff	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6902	cd 46 60 	. F ` 
	call ASSERT_NEXT_IS_EQUAL		;6905	cd 3a 60 	. : ` 
	call sub_6a6dh		;6908	cd 6d 6a 	. m j 
	call sub_4ffeh		;690b	cd fe 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;690e	cd 56 60 	. V ` 
	pop af			;6911	f1 	. 
	call sub_7143h		;6912	cd 43 71 	. C q 
	nop			;6915	00 	. 
	nop			;6916	00 	. 
	ret			;6917	c9 	. 
l6918h:
	ld a,024h		;6918	3e 24 	> $ 
	call ASSERT_NEXT_TOKEN_BE		;691a	cd 48 60 	. H ` 
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;691d	cd 42 60 	. B ` 
	call GET_NUM_PARAMETER		;6920	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6923	cd 46 60 	. F ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6926	cd 56 60 	. V ` 
	call sub_716ch		;6929	cd 6c 71 	. l q 
	nop			;692c	00 	. 
	nop			;692d	00 	. 
	ld b,004h		;692e	06 04 	. . 
	ret			;6930	c9 	. 
TOKEN_C5_SCREEN:
	call GET_NEXT_TOKEN		;6931	cd 30 60 	. 0 ` 
	cp 02ch		;6934	fe 2c 	. , 
	jr z,l6949h		;6936	28 11 	( . 
	call GET_NUM_PARAMETER		;6938	cd 54 4f 	. T O 
WRITE_CMD_SCREEN:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;693b	cd 56 60 	. V ` 
	call CMD_SCREEN		;693e	cd 67 73 	. g s 
	nop			;6941	00 	. 
	nop			;6942	00 	. 
	call GET_NEXT_TOKEN		;6943	cd 30 60 	. 0 ` 
	cp 02ch		;6946	fe 2c 	. , 
	ret nz			;6948	c0 	. 
l6949h:
	inc ix		;6949	dd 23 	. # 
	call GET_NUM_PARAMETER		;694b	cd 54 4f 	. T O 
	ld hl,l70bch		;694e	21 bc 70 	! . p 
	jp WRITE_CALL		;6951	c3 d3 67 	. . g 
TOKEN_9F_CLS:
	ld hl,CMD_CLS		;6954	21 e1 6c 	! . l 
	jp WRITE_CALL		;6957	c3 d3 67 	. . g 
TOKEN_C6_VPOKE:
	call sub_4fa9h		;695a	cd a9 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;695d	cd 3e 60 	. > ` 
	inc b			;6960	04 	. 
	djnz l696dh		;6961	10 0a 	. . 
	push hl			;6963	e5 	. 
	call GET_NUM_PARAMETER		;6964	cd 54 4f 	. T O 
	pop hl			;6967	e1 	. 
	call sub_607ch		;6968	cd 7c 60 	. | ` 
	jr l699bh		;696b	18 2e 	. . 
l696dh:
	djnz l6979h		;696d	10 0a 	. . 
	push hl			;696f	e5 	. 
	call GET_NUM_PARAMETER		;6970	cd 54 4f 	. T O 
	pop hl			;6973	e1 	. 
	call sub_6084h		;6974	cd 84 60 	. . ` 
	jr l699bh		;6977	18 22 	. " 
l6979h:
	call sub_6a68h		;6979	cd 68 6a 	. h j 
	call sub_4fa9h		;697c	cd a9 4f 	. . O 
	inc b			;697f	04 	. 
	djnz l698ah		;6980	10 08 	. . 
	call sub_60d5h		;6982	cd d5 60 	. . ` 
	call sub_4f5ah		;6985	cd 5a 4f 	. Z O 
	jr l699bh		;6988	18 11 	. . 
l698ah:
	djnz l6994h		;698a	10 08 	. . 
	call sub_60d5h		;698c	cd d5 60 	. . ` 
	call sub_4f75h		;698f	cd 75 4f 	. u O 
	jr l699bh		;6992	18 07 	. . 
l6994h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6994	cd 56 60 	. V ` 
	ld a,l			;6997	7d 	} 
	pop hl			;6998	e1 	. 
	nop			;6999	00 	. 
	nop			;699a	00 	. 
l699bh:
	ld hl,l70b5h		;699b	21 b5 70 	! . p 
	jp WRITE_CALL		;699e	c3 d3 67 	. . g 
TOKEN_D2_SET:
	call GET_NEXT_TOKEN		;69a1	cd 30 60 	. 0 ` 
	cp 053h		;69a4	fe 53 	. S 
	jr z,l69e2h		;69a6	28 3a 	( : 
	ld a,050h		;69a8	3e 50 	> P 
	call ASSERT_NEXT_TOKEN_BE		;69aa	cd 48 60 	. H ` 
	ld a,041h		;69ad	3e 41 	> A 
	call ASSERT_NEXT_TOKEN_BE		;69af	cd 48 60 	. H ` 
	ld a,047h		;69b2	3e 47 	> G 
	call ASSERT_NEXT_TOKEN_BE		;69b4	cd 48 60 	. H ` 
	ld a,045h		;69b7	3e 45 	> E 
	call ASSERT_NEXT_TOKEN_BE		;69b9	cd 48 60 	. H ` 
	call GET_NEXT_TOKEN		;69bc	cd 30 60 	. 0 ` 
	cp 02ch		;69bf	fe 2c 	. , 
	jr z,l69d4h		;69c1	28 11 	( . 
	call GET_NUM_PARAMETER		;69c3	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;69c6	cd 56 60 	. V ` 
	call sub_70cah		;69c9	cd ca 70 	. . p 
	nop			;69cc	00 	. 
	nop			;69cd	00 	. 
	call GET_NEXT_TOKEN		;69ce	cd 30 60 	. 0 ` 
	cp 02ch		;69d1	fe 2c 	. , 
	ret nz			;69d3	c0 	. 
l69d4h:
	inc ix		;69d4	dd 23 	. # 
	call GET_NUM_PARAMETER		;69d6	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;69d9	cd 56 60 	. V ` 
	ld (0faf6h),a		;69dc	32 f6 fa 	2 . . 
	nop			;69df	00 	. 
	nop			;69e0	00 	. 
	ret			;69e1	c9 	. 
l69e2h:
	inc ix		;69e2	dd 23 	. # 
	ld a,043h		;69e4	3e 43 	> C 
	call ASSERT_NEXT_TOKEN_BE		;69e6	cd 48 60 	. H ` 
	ld a,052h		;69e9	3e 52 	> R 
	call ASSERT_NEXT_TOKEN_BE		;69eb	cd 48 60 	. H ` 
	ld a,04fh		;69ee	3e 4f 	> O 
	call ASSERT_NEXT_TOKEN_BE		;69f0	cd 48 60 	. H ` 
	ld a,04ch		;69f3	3e 4c 	> L 
	call ASSERT_NEXT_TOKEN_BE		;69f5	cd 48 60 	. H ` 
	call ASSERT_NEXT_TOKEN_BE		;69f8	cd 48 60 	. H ` 
	call sub_6a44h		;69fb	cd 44 6a 	. D j 
	call sub_6a44h		;69fe	cd 44 6a 	. D j 
	call sub_4f00h		;6a01	cd 00 4f 	. . O 
	jr z,l6a1dh		;6a04	28 17 	( . 
	cp 02ch		;6a06	fe 2c 	. , 
	jr z,l6a1bh		;6a08	28 11 	( . 
	call GET_NUM_PARAMETER		;6a0a	cd 54 4f 	. T O 
	call sub_4f00h		;6a0d	cd 00 4f 	. . O 
	jr z,l6a24h		;6a10	28 12 	( . 
	cp 02ch		;6a12	fe 2c 	. , 
	jp nz,04e16h		;6a14	c2 16 4e 	. . N 
	inc ix		;6a17	dd 23 	. # 
	jr l6a24h		;6a19	18 09 	. . 
l6a1bh:
	inc ix		;6a1b	dd 23 	. # 
l6a1dh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6a1d	cd 56 60 	. V ` 
	ld a,0ffh		;6a20	3e ff 	> . 
	nop			;6a22	00 	. 
	nop			;6a23	00 	. 
l6a24h:
	call sub_6a6dh		;6a24	cd 6d 6a 	. m j 
	call sub_4f00h		;6a27	cd 00 4f 	. . O 
	jr z,l6a31h		;6a2a	28 05 	( . 
	call GET_NUM_PARAMETER		;6a2c	cd 54 4f 	. T O 
	jr l6a38h		;6a2f	18 07 	. . 
l6a31h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6a31	cd 56 60 	. V ` 
	ld a,0ffh		;6a34	3e ff 	> . 
	nop			;6a36	00 	. 
	nop			;6a37	00 	. 
l6a38h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6a38	cd 56 60 	. V ` 
	pop bc			;6a3b	c1 	. 
	pop hl			;6a3c	e1 	. 
	pop de			;6a3d	d1 	. 
	call sub_7002h		;6a3e	cd 02 70 	. . p 
	nop			;6a41	00 	. 
	nop			;6a42	00 	. 
	ret			;6a43	c9 	. 
sub_6a44h:
	call sub_4f00h		;6a44	cd 00 4f 	. . O 
	jr z,l6a60h		;6a47	28 17 	( . 
	cp 02ch		;6a49	fe 2c 	. , 
	jr z,l6a5eh		;6a4b	28 11 	( . 
	call sub_4f97h		;6a4d	cd 97 4f 	. . O 
	call sub_4f00h		;6a50	cd 00 4f 	. . O 
	jr z,sub_6a68h		;6a53	28 13 	( . 
	cp 02ch		;6a55	fe 2c 	. , 
	jp nz,04e16h		;6a57	c2 16 4e 	. . N 
	inc ix		;6a5a	dd 23 	. # 
	jr sub_6a68h		;6a5c	18 0a 	. . 
l6a5eh:
	inc ix		;6a5e	dd 23 	. # 
l6a60h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6a60	cd 56 60 	. V ` 
	ld hl,0ffffh		;6a63	21 ff ff 	! . . 
	nop			;6a66	00 	. 
	nop			;6a67	00 	. 
sub_6a68h:
	ld a,0e5h		;6a68	3e e5 	> . 
l6a6ah:
	jp WRITE_OPCODE		;6a6a	c3 b7 60 	. . ` 
sub_6a6dh:
	ld a,0f5h		;6a6d	3e f5 	> . 
	jr l6a6ah		;6a6f	18 f9 	. . 
	call sub_4f97h		;6a71	cd 97 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6a74	cd 46 60 	. F ` 
	ld b,002h		;6a77	06 02 	. . 
	ld hl,l70a1h		;6a79	21 a1 70 	! . p 
	jp WRITE_CALL		;6a7c	c3 d3 67 	. . g 
TOKEN_C8_VDP:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;6a7f	cd 42 60 	. B ` 
	call GET_NUM_PARAMETER		;6a82	cd 54 4f 	. T O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6a85	cd 46 60 	. F ` 
	call sub_6a6dh		;6a88	cd 6d 6a 	. m j 
	call ASSERT_NEXT_IS_EQUAL		;6a8b	cd 3a 60 	. : ` 
	call GET_NUM_PARAMETER		;6a8e	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6a91	cd 56 60 	. V ` 
	ld b,a			;6a94	47 	G 
	pop af			;6a95	f1 	. 
	cp 008h		;6a96	fe 08 	. . 
	adc a,0ffh		;6a98	ce ff 	. . 
	ld c,a			;6a9a	4f 	O 
	call 00047h		;6a9b	cd 47 00 	. G . 
	nop			;6a9e	00 	. 
	nop			;6a9f	00 	. 
	ret			;6aa0	c9 	. 
l6aa1h:
	scf			;6aa1	37 	7 
	call sub_6b8ah		;6aa2	cd 8a 6b 	. . k 
	ld b,002h		;6aa5	06 02 	. . 
	ld hl,l6fa7h		;6aa7	21 a7 6f 	! . o 
	jp WRITE_CALL		;6aaa	c3 d3 67 	. . g 
l6aadh:
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;6aad	cd 42 60 	. B ` 
	call sub_4f97h		;6ab0	cd 97 4f 	. . O 
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6ab3	cd 46 60 	. F ` 
	ld b,002h		;6ab6	06 02 	. . 
	ld hl,l7337h		;6ab8	21 37 73 	! 7 s 
	jp WRITE_CALL		;6abb	c3 d3 67 	. . g 
TOKEN_BF_PAINT:
	call sub_6b89h		;6abe	cd 89 6b 	. . k 
	call sub_60d5h		;6ac1	cd d5 60 	. . ` 
	call sub_6a68h		;6ac4	cd 68 6a 	. h j 
	call GET_NEXT_TOKEN		;6ac7	cd 30 60 	. 0 ` 
	cp 02ch		;6aca	fe 2c 	. , 
	jr nz,l6b02h		;6acc	20 34 	  4 
	inc ix		;6ace	dd 23 	. # 
	call GET_NEXT_TOKEN		;6ad0	cd 30 60 	. 0 ` 
	cp 02ch		;6ad3	fe 2c 	. , 
	jr z,l6af0h		;6ad5	28 19 	( . 
	call GET_NUM_PARAMETER		;6ad7	cd 54 4f 	. T O 
	call GET_NEXT_TOKEN		;6ada	cd 30 60 	. 0 ` 
	cp 02ch		;6add	fe 2c 	. , 
	jr nz,l6b0ah		;6adf	20 29 	  ) 
	inc ix		;6ae1	dd 23 	. # 
	call sub_6a6dh		;6ae3	cd 6d 6a 	. m j 
	call GET_NUM_PARAMETER		;6ae6	cd 54 4f 	. T O 
	ld a,0c1h		;6ae9	3e c1 	> . 
	call WRITE_OPCODE		;6aeb	cd b7 60 	. . ` 
	jr WRITE_CMD_PAINT		;6aee	18 20 	.   
l6af0h:
	inc ix		;6af0	dd 23 	. # 
	call GET_NUM_PARAMETER		;6af2	cd 54 4f 	. T O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6af5	cd 56 60 	. V ` 
	ld c,a			;6af8	4f 	O 
	ld a,(BIOS_FORCLR)		;6af9	3a e9 f3 	: . . 
	ld b,a			;6afc	47 	G 
	ld a,c			;6afd	79 	y 
	nop			;6afe	00 	. 
	nop			;6aff	00 	. 
	jr WRITE_CMD_PAINT		;6b00	18 0e 	. . 
l6b02h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b02	cd 56 60 	. V ` 
	ld a,(BIOS_FORCLR)		;6b05	3a e9 f3 	: . . 
	nop			;6b08	00 	. 
	nop			;6b09	00 	. 
l6b0ah:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b0a	cd 56 60 	. V ` 
	ld b,a			;6b0d	47 	G 
	nop			;6b0e	00 	. 
	nop			;6b0f	00 	. 
WRITE_CMD_PAINT:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b10	cd 56 60 	. V ` 
	pop hl			;6b13	e1 	. 
	pop de			;6b14	d1 	. 
	call CMD_PAINT		;6b15	cd b3 74 	. . t 
	nop			;6b18	00 	. 
	nop			;6b19	00 	. 
	ret			;6b1a	c9 	. 
TOKEN_D6_COPY:
	call sub_6b89h		;6b1b	cd 89 6b 	. . k 
	ld a,0f2h		;6b1e	3e f2 	> . 
	call ASSERT_NEXT_TOKEN_BE		;6b20	cd 48 60 	. H ` 
	call sub_60d5h		;6b23	cd d5 60 	. . ` 
	call sub_6a68h		;6b26	cd 68 6a 	. h j 
	call sub_6b89h		;6b29	cd 89 6b 	. . k 
	call sub_60d5h		;6b2c	cd d5 60 	. . ` 
	call sub_6a68h		;6b2f	cd 68 6a 	. h j 
	call GET_NEXT_TOKEN		;6b32	cd 30 60 	. 0 ` 
	cp 02ch		;6b35	fe 2c 	. , 
	jr nz,l6b40h		;6b37	20 07 	  . 
	inc ix		;6b39	dd 23 	. # 
	call GET_NUM_PARAMETER		;6b3b	cd 54 4f 	. T O 
	jr l6b48h		;6b3e	18 08 	. . 
l6b40h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b40	cd 56 60 	. V ` 
	ld a,(0faf6h)		;6b43	3a f6 fa 	: . . 
	nop			;6b46	00 	. 
	nop			;6b47	00 	. 
l6b48h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b48	cd 56 60 	. V ` 
	ld (0fc18h),a		;6b4b	32 18 fc 	2 . . 
	nop			;6b4e	00 	. 
	nop			;6b4f	00 	. 
	ld a,0d9h		;6b50	3e d9 	> . 
	call ASSERT_NEXT_TOKEN_BE		;6b52	cd 48 60 	. H ` 
	call sub_6b89h		;6b55	cd 89 6b 	. . k 
	call GET_NEXT_TOKEN		;6b58	cd 30 60 	. 0 ` 
	cp 02ch		;6b5b	fe 2c 	. , 
	jr nz,l6b6dh		;6b5d	20 0e 	  . 
	inc ix		;6b5f	dd 23 	. # 
	call GET_NEXT_TOKEN		;6b61	cd 30 60 	. 0 ` 
	cp 02ch		;6b64	fe 2c 	. , 
	jr z,l6b6dh		;6b66	28 05 	( . 
	call GET_NUM_PARAMETER		;6b68	cd 54 4f 	. T O 
	jr l6b75h		;6b6b	18 08 	. . 
l6b6dh:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b6d	cd 56 60 	. V ` 
	ld a,(0faf6h)		;6b70	3a f6 fa 	: . . 
	nop			;6b73	00 	. 
	nop			;6b74	00 	. 
l6b75h:
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b75	cd 56 60 	. V ` 
	ld (0fc19h),a		;6b78	32 19 fc 	2 . . 
	pop iy		;6b7b	fd e1 	. . 
	pop ix		;6b7d	dd e1 	. . 
	pop hl			;6b7f	e1 	. 
	pop de			;6b80	d1 	. 
	nop			;6b81	00 	. 
	nop			;6b82	00 	. 
	ld de,l6ec6h		;6b83	11 c6 6e 	. . n 
	jp l67cfh		;6b86	c3 cf 67 	. . g 
sub_6b89h:
	and a			;6b89	a7 	. 
sub_6b8ah:
	push af			;6b8a	f5 	. 
	call GET_NEXT_TOKEN		;6b8b	cd 30 60 	. 0 ` 
	inc ix		;6b8e	dd 23 	. # 
	cp 028h		;6b90	fe 28 	. ( 
	jr nz,l6baeh		;6b92	20 1a 	  . 
	call sub_4f97h		;6b94	cd 97 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;6b97	cd 3e 60 	. > ` 
	pop af			;6b9a	f1 	. 
	push af			;6b9b	f5 	. 
	jr c,l6ba6h		;6b9c	38 08 	8 . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6b9e	cd 56 60 	. V ` 
	ld (BIOS_GRPACX),hl		;6ba1	22 b7 fc 	" . . 
	nop			;6ba4	00 	. 
	nop			;6ba5	00 	. 
l6ba6h:
	call sub_6a68h		;6ba6	cd 68 6a 	. h j 
	call sub_4f97h		;6ba9	cd 97 4f 	. . O 
	jr l6be2h		;6bac	18 34 	. 4 
l6baeh:
	cp 0dch		;6bae	fe dc 	. . 
	jp nz,04e16h		;6bb0	c2 16 4e 	. . N 
	call ASSERT_NEXT_IS_PARENTESIS_OPENED		;6bb3	cd 42 60 	. B ` 
	call sub_4f97h		;6bb6	cd 97 4f 	. . O 
	call ASSERT_NEXT_IS_COMMA		;6bb9	cd 3e 60 	. > ` 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6bbc	cd 56 60 	. V ` 
	ld de,(BIOS_GRPACX)		;6bbf	ed 5b b7 fc 	. [ . . 
	add hl,de			;6bc3	19 	. 
	nop			;6bc4	00 	. 
	nop			;6bc5	00 	. 
	pop af			;6bc6	f1 	. 
	push af			;6bc7	f5 	. 
	jr c,l6bd2h		;6bc8	38 08 	8 . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6bca	cd 56 60 	. V ` 
	ld (BIOS_GRPACX),hl		;6bcd	22 b7 fc 	" . . 
	nop			;6bd0	00 	. 
	nop			;6bd1	00 	. 
l6bd2h:
	call sub_6a68h		;6bd2	cd 68 6a 	. h j 
	call sub_4f97h		;6bd5	cd 97 4f 	. . O 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6bd8	cd 56 60 	. V ` 
	ld de,(BIOS_GRPACY)		;6bdb	ed 5b b9 fc 	. [ . . 
	add hl,de			;6bdf	19 	. 
	nop			;6be0	00 	. 
	nop			;6be1	00 	. 
l6be2h:
	call ASSERT_NEXT_IS_PARENTESIS_CLOSED		;6be2	cd 46 60 	. F ` 
	pop af			;6be5	f1 	. 
	jr c,l6bf0h		;6be6	38 08 	8 . 
	call WRITE_NEXT_BYTES_UNTIL_2_NOPS		;6be8	cd 56 60 	. V ` 
	ld (BIOS_GRPACY),hl		;6beb	22 b9 fc 	" . . 
	nop			;6bee	00 	. 
	nop			;6bef	00 	. 
l6bf0h:
	ld a,0d1h		;6bf0	3e d1 	> . 
	jp WRITE_OPCODE		;6bf2	c3 b7 60 	. . ` 
l6bf5h:
	ld b,002h		;6bf5	06 02 	. . 
	jr l6bffh		;6bf7	18 06 	. . 
l6bf9h:
	ld b,000h		;6bf9	06 00 	. . 
	jr l6bffh		;6bfb	18 02 	. . 
l6bfdh:
	ld b,003h		;6bfd	06 03 	. . 
l6bffh:
	ld c,(hl)			;6bff	4e 	N 
	ld a,(de)			;6c00	1a 	. 
	ld (hl),a			;6c01	77 	w 
	ld a,c			;6c02	79 	y 
	ld (de),a			;6c03	12 	. 
	inc de			;6c04	13 	. 
	inc hl			;6c05	23 	# 
	djnz l6bffh		;6c06	10 f7 	. . 
	ret			;6c08	c9 	. 
sub_6c09h:
	ld (0f7f8h),hl		;6c09	22 f8 f7 	" . . 
	ld a,002h		;6c0c	3e 02 	> . 
	ld (BASIC_VALTYP),a		;6c0e	32 63 f6 	2 c . 
	ld hl,BASIC_DAC		;6c11	21 f6 f7 	! . . 
	call sub_6c23h		;6c14	cd 23 6c 	. # l 
	ld a,(BASIC_VALTYP)		;6c17	3a 63 f6 	: c . 
	cp 002h		;6c1a	fe 02 	. . 
	jp nz,04e10h		;6c1c	c2 10 4e 	. . N 
	ld hl,(0f7f8h)		;6c1f	2a f8 f7 	* . . 
	ret			;6c22	c9 	. 
sub_6c23h:
	push de			;6c23	d5 	. 
	ret			;6c24	c9 	. 
sub_6c25h:
	ld hl,BASIC_TRPTBL_STOP		;6c25	21 6a fc 	! j . 
	bit 0,(hl)		;6c28	cb 46 	. F 
	jr z,l6c48h		;6c2a	28 1c 	( . 
	ex de,hl			;6c2c	eb 	. 
	ld hl,BIOS_INTFLG		;6c2d	21 9b fc 	! . . 
	di			;6c30	f3 	. 
	ld a,(hl)			;6c31	7e 	~ 
	ei			;6c32	fb 	. 
	ld (hl),000h		;6c33	36 00 	6 . 
	cp 003h		;6c35	fe 03 	. . 
	jr nz,l6c48h		;6c37	20 0f 	  . 
	ex de,hl			;6c39	eb 	. 
	ld a,(hl)			;6c3a	7e 	~ 
	set 2,(hl)		;6c3b	cb d6 	. . 
	cp (hl)			;6c3d	be 	. 
	jr z,l6c48h		;6c3e	28 08 	( . 
	and 002h		;6c40	e6 02 	. . 
	jr nz,l6c48h		;6c42	20 04 	  . 
	ld hl,BASIC_ONGSBF		;6c44	21 d8 fb 	! . . 
	inc (hl)			;6c47	34 	4 
l6c48h:
	ld a,(BASIC_ONGSBF)		;6c48	3a d8 fb 	: . . 
	and a			;6c4b	a7 	. 
	ret z			;6c4c	c8 	. 
	ld hl,BASIC_TRPTBL		;6c4d	21 4c fc 	! L . 
	ld b,01ah		;6c50	06 1a 	. . 
l6c52h:
	ld a,(hl)			;6c52	7e 	~ 
	cp 005h		;6c53	fe 05 	. . 
	inc hl			;6c55	23 	# 
	jr z,l6c5dh		;6c56	28 05 	( . 
	inc hl			;6c58	23 	# 
l6c59h:
	inc hl			;6c59	23 	# 
	djnz l6c52h		;6c5a	10 f6 	. . 
	ret			;6c5c	c9 	. 
l6c5dh:
	ld e,(hl)			;6c5d	5e 	^ 
	inc hl			;6c5e	23 	# 
	ld d,(hl)			;6c5f	56 	V 
	ld a,e			;6c60	7b 	{ 
	or d			;6c61	b2 	. 
	jr z,l6c59h		;6c62	28 f5 	( . 
	dec hl			;6c64	2b 	+ 
	dec hl			;6c65	2b 	+ 
	call sub_6cafh		;6c66	cd af 6c 	. . l 
	call l6ca5h		;6c69	cd a5 6c 	. . l 
	push hl			;6c6c	e5 	. 
	call sub_6c23h		;6c6d	cd 23 6c 	. # l 
	pop hl			;6c70	e1 	. 
	bit 0,(hl)		;6c71	cb 46 	. F 
	ret z			;6c73	c8 	. 
	di			;6c74	f3 	. 
	ld a,(hl)			;6c75	7e 	~ 
	and 005h		;6c76	e6 05 	. . 
	cp (hl)			;6c78	be 	. 
	ld (hl),a			;6c79	77 	w 
	jr z,l6cbch		;6c7a	28 40 	( @ 
	cp 005h		;6c7c	fe 05 	. . 
	jr nz,l6cbch		;6c7e	20 3c 	  < 
l6c80h:
	ld a,(BASIC_ONGSBF)		;6c80	3a d8 fb 	: . . 
	inc a			;6c83	3c 	< 
	ld (BASIC_ONGSBF),a		;6c84	32 d8 fb 	2 . . 
	ei			;6c87	fb 	. 
	ret			;6c88	c9 	. 
l6c89h:
	call sub_6cbeh		;6c89	cd be 6c 	. . l 
	di			;6c8c	f3 	. 
	ld a,(hl)			;6c8d	7e 	~ 
	and 004h		;6c8e	e6 04 	. . 
	or 001h		;6c90	f6 01 	. . 
	cp (hl)			;6c92	be 	. 
	ld (hl),a			;6c93	77 	w 
	jr z,l6cbch		;6c94	28 26 	( & 
	and 004h		;6c96	e6 04 	. . 
	jr nz,l6c80h		;6c98	20 e6 	  . 
	ei			;6c9a	fb 	. 
	ret			;6c9b	c9 	. 
l6c9ch:
	call sub_6cc6h		;6c9c	cd c6 6c 	. . l 
	di			;6c9f	f3 	. 
	ld a,(hl)			;6ca0	7e 	~ 
	ld (hl),000h		;6ca1	36 00 	6 . 
	jr l6ca9h		;6ca3	18 04 	. . 
l6ca5h:
	di			;6ca5	f3 	. 
	ld a,(hl)			;6ca6	7e 	~ 
	set 1,(hl)		;6ca7	cb ce 	. . 
l6ca9h:
	cp 005h		;6ca9	fe 05 	. . 
	jr z,l6cb2h		;6cab	28 05 	( . 
	ei			;6cad	fb 	. 
	ret			;6cae	c9 	. 
sub_6cafh:
	di			;6caf	f3 	. 
	res 2,(hl)		;6cb0	cb 96 	. . 
l6cb2h:
	ld a,(BASIC_ONGSBF)		;6cb2	3a d8 fb 	: . . 
	sub 001h		;6cb5	d6 01 	. . 
	jr c,l6cbch		;6cb7	38 03 	8 . 
	ld (BASIC_ONGSBF),a		;6cb9	32 d8 fb 	2 . . 
l6cbch:
	ei			;6cbc	fb 	. 
	ret			;6cbd	c9 	. 
sub_6cbeh:
	call sub_6ccdh		;6cbe	cd cd 6c 	. . l 
	ret nc			;6cc1	d0 	. 
	ld a,001h		;6cc2	3e 01 	> . 
	ld (de),a			;6cc4	12 	. 
	ret			;6cc5	c9 	. 
sub_6cc6h:
	call sub_6ccdh		;6cc6	cd cd 6c 	. . l 
	ret nc			;6cc9	d0 	. 
	xor a			;6cca	af 	. 
	ld (de),a			;6ccb	12 	. 
	ret			;6ccc	c9 	. 
sub_6ccdh:
	push hl			;6ccd	e5 	. 
	ld de,BASIC_TRPTBL_STOP		;6cce	11 6a fc 	. j . 
	and a			;6cd1	a7 	. 
	sbc hl,de		;6cd2	ed 52 	. R 
	ld c,l			;6cd4	4d 	M 
	pop hl			;6cd5	e1 	. 
	ret nc			;6cd6	d0 	. 
	ld de,BASIC_ONGSBF		;6cd7	11 d8 fb 	. . . 
l6cdah:
	dec de			;6cda	1b 	. 
	inc c			;6cdb	0c 	. 
	inc c			;6cdc	0c 	. 
	inc c			;6cdd	0c 	. 
	jr nz,l6cdah		;6cde	20 fa 	  . 
	ret			;6ce0	c9 	. 
CMD_CLS:
	ld a,(BIOS_SCRMOD)		;6ce1	3a af fc 	: . . 
	cp 005h		;6ce4	fe 05 	. . 
	jp c,l7465h		;6ce6	da 65 74 	. e t 
	call sub_73cah		;6ce9	cd ca 73 	. . s 
	call sub_7077h		;6cec	cd 77 70 	. w p 
	inc c			;6cef	0c 	. 
	inc c			;6cf0	0c 	. 
	xor a			;6cf1	af 	. 
	out (c),a		;6cf2	ed 79 	. y 
	out (c),a		;6cf4	ed 79 	. y 
	out (c),a		;6cf6	ed 79 	. y 
	ld a,(0faf6h)		;6cf8	3a f6 fa 	: . . 
	out (c),a		;6cfb	ed 79 	. y 
	xor a			;6cfd	af 	. 
	out (c),a		;6cfe	ed 79 	. y 
	ld a,(BIOS_SCRMOD)		;6d00	3a af fc 	: . . 
	rrca			;6d03	0f 	. 
	and 001h		;6d04	e6 01 	. . 
	inc a			;6d06	3c 	< 
	out (c),a		;6d07	ed 79 	. y 
	ld a,(MODE)		;6d09	3a fc fa 	: . . 
	and 010h		;6d0c	e6 10 	. . 
	jr z,l6d16h		;6d0e	28 06 	( . 
	xor a			;6d10	af 	. 
	out (c),a		;6d11	ed 79 	. y 
	inc a			;6d13	3c 	< 
	jr l6d1bh		;6d14	18 05 	. . 
l6d16h:
	ld a,0d4h		;6d16	3e d4 	> . 
	out (c),a		;6d18	ed 79 	. y 
	xor a			;6d1a	af 	. 
l6d1bh:
	out (c),a		;6d1b	ed 79 	. y 
	ld a,(BIOS_BAKCLR)		;6d1d	3a ea f3 	: . . 
	call sub_6d2dh		;6d20	cd 2d 6d 	. - m 
	out (c),h		;6d23	ed 61 	. a 
	xor a			;6d25	af 	. 
	out (c),a		;6d26	ed 79 	. y 
	ld a,0c0h		;6d28	3e c0 	> . 
	out (c),a		;6d2a	ed 79 	. y 
	ret			;6d2c	c9 	. 
sub_6d2dh:
	ld h,a			;6d2d	67 	g 
	ld a,(BIOS_SCRMOD)		;6d2e	3a af fc 	: . . 
	cp 008h		;6d31	fe 08 	. . 
	ret z			;6d33	c8 	. 
	cp 007h		;6d34	fe 07 	. . 
	jr z,l6d41h		;6d36	28 09 	( . 
	cp 005h		;6d38	fe 05 	. . 
	jr z,l6d41h		;6d3a	28 05 	( . 
	ld a,h			;6d3c	7c 	| 
	add a,a			;6d3d	87 	. 
	add a,a			;6d3e	87 	. 
	or h			;6d3f	b4 	. 
	ld h,a			;6d40	67 	g 
l6d41h:
	ld a,h			;6d41	7c 	| 
	add a,a			;6d42	87 	. 
	add a,a			;6d43	87 	. 
	add a,a			;6d44	87 	. 
	add a,a			;6d45	87 	. 
	or h			;6d46	b4 	. 
	ld h,a			;6d47	67 	g 
	ret			;6d48	c9 	. 
l6d49h:
	push af			;6d49	f5 	. 
	ld a,(BIOS_SCRMOD)		;6d4a	3a af fc 	: . . 
	cp 005h		;6d4d	fe 05 	. . 
	jp c,l7472h		;6d4f	da 72 74 	. r t 
	pop af			;6d52	f1 	. 
	push hl			;6d53	e5 	. 
	push ix		;6d54	dd e5 	. . 
	pop hl			;6d56	e1 	. 
	and a			;6d57	a7 	. 
	push hl			;6d58	e5 	. 
	sbc hl,de		;6d59	ed 52 	. R 
	pop hl			;6d5b	e1 	. 
	jr nc,l6d60h		;6d5c	30 02 	0 . 
	ex de,hl			;6d5e	eb 	. 
	and a			;6d5f	a7 	. 
l6d60h:
	ex (sp),hl			;6d60	e3 	. 
	push de			;6d61	d5 	. 
	push iy		;6d62	fd e5 	. . 
	pop de			;6d64	d1 	. 
	push hl			;6d65	e5 	. 
	sbc hl,de		;6d66	ed 52 	. R 
	pop hl			;6d68	e1 	. 
	jr c,l6d6ch		;6d69	38 01 	8 . 
	ex de,hl			;6d6b	eb 	. 
l6d6ch:
	push de			;6d6c	d5 	. 
	pop iy		;6d6d	fd e1 	. . 
	pop de			;6d6f	d1 	. 
	pop ix		;6d70	dd e1 	. . 
	push iy		;6d72	fd e5 	. . 
	push hl			;6d74	e5 	. 
	pop iy		;6d75	fd e1 	. . 
	dec ix		;6d77	dd 2b 	. + 
	push af			;6d79	f5 	. 
	push bc			;6d7a	c5 	. 
	call CMD_LINE		;6d7b	cd a7 6d 	. . m 
	pop bc			;6d7e	c1 	. 
	pop af			;6d7f	f1 	. 
	inc ix		;6d80	dd 23 	. # 
	pop hl			;6d82	e1 	. 
	dec hl			;6d83	2b 	+ 
	push iy		;6d84	fd e5 	. . 
	push de			;6d86	d5 	. 
	push ix		;6d87	dd e5 	. . 
	pop de			;6d89	d1 	. 
	push af			;6d8a	f5 	. 
	push bc			;6d8b	c5 	. 
	call CMD_LINE		;6d8c	cd a7 6d 	. . m 
	pop bc			;6d8f	c1 	. 
	pop af			;6d90	f1 	. 
	inc hl			;6d91	23 	# 
	pop ix		;6d92	dd e1 	. . 
	inc ix		;6d94	dd 23 	. # 
	push hl			;6d96	e5 	. 
	pop iy		;6d97	fd e1 	. . 
	push af			;6d99	f5 	. 
	push bc			;6d9a	c5 	. 
	call CMD_LINE		;6d9b	cd a7 6d 	. . m 
	pop bc			;6d9e	c1 	. 
	pop af			;6d9f	f1 	. 
	dec ix		;6da0	dd 2b 	. + 
	pop hl			;6da2	e1 	. 
	inc hl			;6da3	23 	# 
	push ix		;6da4	dd e5 	. . 
	pop de			;6da6	d1 	. 
CMD_LINE:
	push af			;6da7	f5 	. 
	ld a,(BIOS_SCRMOD)		;6da8	3a af fc 	: . . 
	cp 005h		;6dab	fe 05 	. . 
	jp c,l746dh		;6dad	da 6d 74 	. m t 
	pop af			;6db0	f1 	. 
	call sub_73cah		;6db1	cd ca 73 	. . s 
	push af			;6db4	f5 	. 
	call sub_7438h		;6db5	cd 38 74 	. 8 t 
	call sub_7424h		;6db8	cd 24 74 	. $ t 
	pop af			;6dbb	f1 	. 
	push de			;6dbc	d5 	. 
	push hl			;6dbd	e5 	. 
	push ix		;6dbe	dd e5 	. . 
	push iy		;6dc0	fd e5 	. . 
	push af			;6dc2	f5 	. 
	push af			;6dc3	f5 	. 
	push hl			;6dc4	e5 	. 
	call sub_7077h		;6dc5	cd 77 70 	. w p 
	inc c			;6dc8	0c 	. 
	inc c			;6dc9	0c 	. 
	push ix		;6dca	dd e5 	. . 
	pop hl			;6dcc	e1 	. 
	out (c),l		;6dcd	ed 69 	. i 
	out (c),h		;6dcf	ed 61 	. a 
	push iy		;6dd1	fd e5 	. . 
	pop hl			;6dd3	e1 	. 
	out (c),l		;6dd4	ed 69 	. i 
	ld a,(0faf6h)		;6dd6	3a f6 fa 	: . . 
	or h			;6dd9	b4 	. 
	out (c),a		;6dda	ed 79 	. y 
	ld a,00ch		;6ddc	3e 0c 	> . 
	push ix		;6dde	dd e5 	. . 
	pop hl			;6de0	e1 	. 
	sbc hl,de		;6de1	ed 52 	. R 
	jr nc,l6dedh		;6de3	30 08 	0 . 
	ex de,hl			;6de5	eb 	. 
	ld hl,00001h		;6de6	21 01 00 	! . . 
	sbc hl,de		;6de9	ed 52 	. R 
	and 0fbh		;6deb	e6 fb 	. . 
l6dedh:
	push hl			;6ded	e5 	. 
	pop ix		;6dee	dd e1 	. . 
	pop de			;6df0	d1 	. 
	push iy		;6df1	fd e5 	. . 
	pop hl			;6df3	e1 	. 
	sbc hl,de		;6df4	ed 52 	. R 
	jr nc,l6e00h		;6df6	30 08 	0 . 
	ex de,hl			;6df8	eb 	. 
	ld hl,00001h		;6df9	21 01 00 	! . . 
	sbc hl,de		;6dfc	ed 52 	. R 
	and 0f7h		;6dfe	e6 f7 	. . 
l6e00h:
	push ix		;6e00	dd e5 	. . 
	pop de			;6e02	d1 	. 
	push hl			;6e03	e5 	. 
	sbc hl,de		;6e04	ed 52 	. R 
	pop hl			;6e06	e1 	. 
	jr c,l6e0ch		;6e07	38 03 	8 . 
	ex de,hl			;6e09	eb 	. 
	or 001h		;6e0a	f6 01 	. . 
l6e0ch:
	out (c),e		;6e0c	ed 59 	. Y 
	out (c),d		;6e0e	ed 51 	. Q 
	out (c),l		;6e10	ed 69 	. i 
	out (c),h		;6e12	ed 61 	. a 
	ld e,a			;6e14	5f 	_ 
	pop af			;6e15	f1 	. 
	out (c),a		;6e16	ed 79 	. y 
	out (c),e		;6e18	ed 59 	. Y 
	ld a,b			;6e1a	78 	x 
	or 070h		;6e1b	f6 70 	. p 
	out (c),a		;6e1d	ed 79 	. y 
	pop af			;6e1f	f1 	. 
	pop iy		;6e20	fd e1 	. . 
	pop ix		;6e22	dd e1 	. . 
	pop hl			;6e24	e1 	. 
	pop de			;6e25	d1 	. 
	ret			;6e26	c9 	. 
l6e27h:
	push af			;6e27	f5 	. 
	ld a,(BIOS_SCRMOD)		;6e28	3a af fc 	: . . 
	cp 005h		;6e2b	fe 05 	. . 
	jp c,l7477h		;6e2d	da 77 74 	. w t 
	pop af			;6e30	f1 	. 
	call sub_73cah		;6e31	cd ca 73 	. . s 
sub_6e34h:
	push af			;6e34	f5 	. 
	call sub_7438h		;6e35	cd 38 74 	. 8 t 
	call sub_7424h		;6e38	cd 24 74 	. $ t 
	pop af			;6e3b	f1 	. 
	push af			;6e3c	f5 	. 
	push hl			;6e3d	e5 	. 
	push ix		;6e3e	dd e5 	. . 
	pop hl			;6e40	e1 	. 
	and a			;6e41	a7 	. 
	push hl			;6e42	e5 	. 
	sbc hl,de		;6e43	ed 52 	. R 
	pop hl			;6e45	e1 	. 
	jr nc,l6e4ah		;6e46	30 02 	0 . 
	ex de,hl			;6e48	eb 	. 
	and a			;6e49	a7 	. 
l6e4ah:
	ex (sp),hl			;6e4a	e3 	. 
	push de			;6e4b	d5 	. 
	push iy		;6e4c	fd e5 	. . 
	pop de			;6e4e	d1 	. 
	push hl			;6e4f	e5 	. 
	sbc hl,de		;6e50	ed 52 	. R 
	pop hl			;6e52	e1 	. 
	jr c,l6e56h		;6e53	38 01 	8 . 
	ex de,hl			;6e55	eb 	. 
l6e56h:
	push de			;6e56	d5 	. 
	pop iy		;6e57	fd e1 	. . 
	pop de			;6e59	d1 	. 
	pop ix		;6e5a	dd e1 	. . 
	call sub_7077h		;6e5c	cd 77 70 	. w p 
	inc c			;6e5f	0c 	. 
	inc c			;6e60	0c 	. 
	out (c),e		;6e61	ed 59 	. Y 
	out (c),d		;6e63	ed 51 	. Q 
	out (c),l		;6e65	ed 69 	. i 
	ld a,(0faf6h)		;6e67	3a f6 fa 	: . . 
	or h			;6e6a	b4 	. 
	out (c),a		;6e6b	ed 79 	. y 
	push hl			;6e6d	e5 	. 
	push ix		;6e6e	dd e5 	. . 
	pop hl			;6e70	e1 	. 
	and a			;6e71	a7 	. 
	sbc hl,de		;6e72	ed 52 	. R 
	inc hl			;6e74	23 	# 
	out (c),l		;6e75	ed 69 	. i 
	out (c),h		;6e77	ed 61 	. a 
	ld a,l			;6e79	7d 	} 
	or e			;6e7a	b3 	. 
	push iy		;6e7b	fd e5 	. . 
	pop hl			;6e7d	e1 	. 
	pop de			;6e7e	d1 	. 
	sbc hl,de		;6e7f	ed 52 	. R 
	inc hl			;6e81	23 	# 
	out (c),l		;6e82	ed 69 	. i 
	out (c),h		;6e84	ed 61 	. a 
	ld e,a			;6e86	5f 	_ 
	pop hl			;6e87	e1 	. 
	inc b			;6e88	04 	. 
	djnz l6ea5h		;6e89	10 1a 	. . 
	ld a,(BIOS_SCRMOD)		;6e8b	3a af fc 	: . . 
	cp 008h		;6e8e	fe 08 	. . 
	jr z,l6ebch		;6e90	28 2a 	( * 
	cp 007h		;6e92	fe 07 	. . 
	jr z,l6e9ah		;6e94	28 04 	( . 
	cp 006h		;6e96	fe 06 	. . 
	jr z,l6ea0h		;6e98	28 06 	( . 
l6e9ah:
	ld a,e			;6e9a	7b 	{ 
	rrca			;6e9b	0f 	. 
	jr nc,l6eb5h		;6e9c	30 17 	0 . 
	jr l6ea5h		;6e9e	18 05 	. . 
l6ea0h:
	ld a,e			;6ea0	7b 	{ 
	and 003h		;6ea1	e6 03 	. . 
	jr z,l6eb0h		;6ea3	28 0b 	( . 
l6ea5h:
	out (c),h		;6ea5	ed 61 	. a 
	xor a			;6ea7	af 	. 
	out (c),a		;6ea8	ed 79 	. y 
	ld a,b			;6eaa	78 	x 
	or 080h		;6eab	f6 80 	. . 
	out (c),a		;6ead	ed 79 	. y 
	ret			;6eaf	c9 	. 
l6eb0h:
	ld a,h			;6eb0	7c 	| 
	add a,a			;6eb1	87 	. 
	add a,a			;6eb2	87 	. 
	or h			;6eb3	b4 	. 
	ld h,a			;6eb4	67 	g 
l6eb5h:
	ld a,h			;6eb5	7c 	| 
	add a,a			;6eb6	87 	. 
	add a,a			;6eb7	87 	. 
	add a,a			;6eb8	87 	. 
	add a,a			;6eb9	87 	. 
	or h			;6eba	b4 	. 
	ld h,a			;6ebb	67 	g 
l6ebch:
	out (c),h		;6ebc	ed 61 	. a 
	xor a			;6ebe	af 	. 
	out (c),a		;6ebf	ed 79 	. y 
	ld a,0c0h		;6ec1	3e c0 	> . 
	out (c),a		;6ec3	ed 79 	. y 
	ret			;6ec5	c9 	. 
l6ec6h:
	call sub_7438h		;6ec6	cd 38 74 	. 8 t 
	call sub_7424h		;6ec9	cd 24 74 	. $ t 
	push hl			;6ecc	e5 	. 
	push de			;6ecd	d5 	. 
	ld de,(BIOS_GRPACX)		;6ece	ed 5b b7 fc 	. [ . . 
	ld hl,(BIOS_GRPACY)		;6ed2	2a b9 fc 	* . . 
	call sub_7438h		;6ed5	cd 38 74 	. 8 t 
	ld (BIOS_GRPACX),de		;6ed8	ed 53 b7 fc 	. S . . 
	ld (BIOS_GRPACY),hl		;6edc	22 b9 fc 	" . . 
	pop de			;6edf	d1 	. 
	pop hl			;6ee0	e1 	. 
	push de			;6ee1	d5 	. 
	push bc			;6ee2	c5 	. 
	ld a,020h		;6ee3	3e 20 	>   
	call sub_7079h		;6ee5	cd 79 70 	. y p 
	inc c			;6ee8	0c 	. 
	inc c			;6ee9	0c 	. 
	out (c),e		;6eea	ed 59 	. Y 
	out (c),d		;6eec	ed 51 	. Q 
	out (c),l		;6eee	ed 69 	. i 
	ld a,(0fc18h)		;6ef0	3a 18 fc 	: . . 
	or h			;6ef3	b4 	. 
	out (c),a		;6ef4	ed 79 	. y 
	push hl			;6ef6	e5 	. 
	ld hl,(BIOS_GRPACX)		;6ef7	2a b7 fc 	* . . 
	out (c),l		;6efa	ed 69 	. i 
	out (c),h		;6efc	ed 61 	. a 
	ld hl,(BIOS_GRPACY)		;6efe	2a b9 fc 	* . . 
	out (c),l		;6f01	ed 69 	. i 
	ld a,(0fc19h)		;6f03	3a 19 fc 	: . . 
	or h			;6f06	b4 	. 
	out (c),a		;6f07	ed 79 	. y 
	ld b,000h		;6f09	06 00 	. . 
	ld a,(BIOS_GRPACX)		;6f0b	3a b7 fc 	: . . 
	or e			;6f0e	b3 	. 
	push ix		;6f0f	dd e5 	. . 
	pop hl			;6f11	e1 	. 
	sbc hl,de		;6f12	ed 52 	. R 
	jr nc,l6f26h		;6f14	30 10 	0 . 
	ld a,(BIOS_GRPACX)		;6f16	3a b7 fc 	: . . 
	cpl			;6f19	2f 	/ 
	ld d,a			;6f1a	57 	W 
	ld a,e			;6f1b	7b 	{ 
	cpl			;6f1c	2f 	/ 
	or d			;6f1d	b2 	. 
	ex de,hl			;6f1e	eb 	. 
	ld hl,00000h		;6f1f	21 00 00 	! . . 
	sbc hl,de		;6f22	ed 52 	. R 
	set 2,b		;6f24	cb d0 	. . 
l6f26h:
	inc hl			;6f26	23 	# 
	out (c),l		;6f27	ed 69 	. i 
	out (c),h		;6f29	ed 61 	. a 
	or l			;6f2b	b5 	. 
	pop de			;6f2c	d1 	. 
	push iy		;6f2d	fd e5 	. . 
	pop hl			;6f2f	e1 	. 
	sbc hl,de		;6f30	ed 52 	. R 
	jr nc,l6f3ch		;6f32	30 08 	0 . 
	ex de,hl			;6f34	eb 	. 
	ld hl,00001h		;6f35	21 01 00 	! . . 
	sbc hl,de		;6f38	ed 52 	. R 
	set 3,b		;6f3a	cb d8 	. . 
l6f3ch:
	inc hl			;6f3c	23 	# 
	out (c),l		;6f3d	ed 69 	. i 
	out (c),h		;6f3f	ed 61 	. a 
	out (c),a		;6f41	ed 79 	. y 
	out (c),b		;6f43	ed 41 	. A 
	ld b,a			;6f45	47 	G 
	pop hl			;6f46	e1 	. 
	pop de			;6f47	d1 	. 
	ld a,h			;6f48	7c 	| 
	and a			;6f49	a7 	. 
	jr nz,l6f66h		;6f4a	20 1a 	  . 
	ld a,(BIOS_SCRMOD)		;6f4c	3a af fc 	: . . 
	cp 008h		;6f4f	fe 08 	. . 
	jr z,l6f6ch		;6f51	28 19 	( . 
	cp 007h		;6f53	fe 07 	. . 
	jr z,l6f5bh		;6f55	28 04 	( . 
	cp 006h		;6f57	fe 06 	. . 
	jr z,l6f61h		;6f59	28 06 	( . 
l6f5bh:
	ld a,b			;6f5b	78 	x 
	rrca			;6f5c	0f 	. 
	jr nc,l6f6ch		;6f5d	30 0d 	0 . 
	jr l6f66h		;6f5f	18 05 	. . 
l6f61h:
	ld a,b			;6f61	78 	x 
	and 003h		;6f62	e6 03 	. . 
	jr z,l6f6ch		;6f64	28 06 	( . 
l6f66h:
	ld a,h			;6f66	7c 	| 
	or 090h		;6f67	f6 90 	. . 
	out (c),a		;6f69	ed 79 	. y 
	ret			;6f6b	c9 	. 
l6f6ch:
	ld a,0d0h		;6f6c	3e d0 	> . 
	out (c),a		;6f6e	ed 79 	. y 
	ret			;6f70	c9 	. 
l6f71h:
	push af			;6f71	f5 	. 
	ld a,(BIOS_SCRMOD)		;6f72	3a af fc 	: . . 
	cp 005h		;6f75	fe 05 	. . 
	jp c,l7491h		;6f77	da 91 74 	. . t 
	pop af			;6f7a	f1 	. 
	call sub_73cah		;6f7b	cd ca 73 	. . s 
	push af			;6f7e	f5 	. 
	call sub_7438h		;6f7f	cd 38 74 	. 8 t 
	call sub_7077h		;6f82	cd 77 70 	. w p 
	inc c			;6f85	0c 	. 
	inc c			;6f86	0c 	. 
	out (c),e		;6f87	ed 59 	. Y 
	out (c),d		;6f89	ed 51 	. Q 
	out (c),l		;6f8b	ed 69 	. i 
	ld a,(0faf6h)		;6f8d	3a f6 fa 	: . . 
	or h			;6f90	b4 	. 
	out (c),a		;6f91	ed 79 	. y 
	out (c),a		;6f93	ed 79 	. y 
	out (c),a		;6f95	ed 79 	. y 
	out (c),a		;6f97	ed 79 	. y 
	out (c),a		;6f99	ed 79 	. y 
	pop af			;6f9b	f1 	. 
	out (c),a		;6f9c	ed 79 	. y 
	xor a			;6f9e	af 	. 
	out (c),a		;6f9f	ed 79 	. y 
	ld a,b			;6fa1	78 	x 
	or 050h		;6fa2	f6 50 	. P 
	out (c),a		;6fa4	ed 79 	. y 
	ret			;6fa6	c9 	. 
l6fa7h:
	ld a,(BIOS_SCRMOD)		;6fa7	3a af fc 	: . . 
	cp 005h		;6faa	fe 05 	. . 
	jp c,l749fh		;6fac	da 9f 74 	. . t 
	call sub_7438h		;6faf	cd 38 74 	. 8 t 
	ld a,020h		;6fb2	3e 20 	>   
	call sub_7079h		;6fb4	cd 79 70 	. y p 
	inc c			;6fb7	0c 	. 
	inc c			;6fb8	0c 	. 
	out (c),e		;6fb9	ed 59 	. Y 
	out (c),d		;6fbb	ed 51 	. Q 
	out (c),l		;6fbd	ed 69 	. i 
	ld a,(0faf6h)		;6fbf	3a f6 fa 	: . . 
	or h			;6fc2	b4 	. 
	out (c),a		;6fc3	ed 79 	. y 
	out (c),a		;6fc5	ed 79 	. y 
	out (c),a		;6fc7	ed 79 	. y 
	out (c),a		;6fc9	ed 79 	. y 
	out (c),a		;6fcb	ed 79 	. y 
	out (c),a		;6fcd	ed 79 	. y 
	out (c),a		;6fcf	ed 79 	. y 
	out (c),a		;6fd1	ed 79 	. y 
	out (c),a		;6fd3	ed 79 	. y 
	out (c),a		;6fd5	ed 79 	. y 
	xor a			;6fd7	af 	. 
	out (c),a		;6fd8	ed 79 	. y 
	ld a,040h		;6fda	3e 40 	> @ 
	out (c),a		;6fdc	ed 79 	. y 
	dec c			;6fde	0d 	. 
	dec c			;6fdf	0d 	. 
	ld h,000h		;6fe0	26 00 	& . 
	call sub_7088h		;6fe2	cd 88 70 	. . p 
	ld a,007h		;6fe5	3e 07 	> . 
	di			;6fe7	f3 	. 
	call sub_709ah		;6fe8	cd 9a 70 	. . p 
	in l,(c)		;6feb	ed 68 	. h 
	xor a			;6fed	af 	. 
	call sub_709ah		;6fee	cd 9a 70 	. . p 
	ei			;6ff1	fb 	. 
	call sub_73bch		;6ff2	cd bc 73 	. . s 
	ret nz			;6ff5	c0 	. 
	ld a,l			;6ff6	7d 	} 
	and 0f8h		;6ff7	e6 f8 	. . 
	rra			;6ff9	1f 	. 
	rra			;6ffa	1f 	. 
	rra			;6ffb	1f 	. 
	rra			;6ffc	1f 	. 
	ld l,a			;6ffd	6f 	o 
	ret c			;6ffe	d8 	. 
	ld l,0ffh		;6fff	2e ff 	. . 
	ret			;7001	c9 	. 
sub_7002h:
	push af			;7002	f5 	. 
	ld a,e			;7003	7b 	{ 
	and d			;7004	a2 	. 
	inc a			;7005	3c 	< 
	jr z,l7021h		;7006	28 19 	( . 
	ld c,e			;7008	4b 	K 
	dec de			;7009	1b 	. 
	rr d		;700a	cb 1a 	. . 
	rr e		;700c	cb 1b 	. . 
	srl e		;700e	cb 3b 	. ; 
	srl e		;7010	cb 3b 	. ; 
	inc e			;7012	1c 	. 
	ld a,e			;7013	7b 	{ 
	and 03fh		;7014	e6 3f 	. ? 
	ld e,a			;7016	5f 	_ 
	ld a,c			;7017	79 	y 
	neg		;7018	ed 44 	. D 
	and 007h		;701a	e6 07 	. . 
	ld d,a			;701c	57 	W 
	ld (0fffbh),de		;701d	ed 53 fb ff 	. S . . 
l7021h:
	ld a,l			;7021	7d 	} 
	and h			;7022	a4 	. 
	inc a			;7023	3c 	< 
	jr z,l702ah		;7024	28 04 	( . 
	ld a,l			;7026	7d 	} 
	ld (0fff6h),a		;7027	32 f6 ff 	2 . . 
l702ah:
	ld hl,0fffah		;702a	21 fa ff 	! . . 
	inc b			;702d	04 	. 
	jr z,l7036h		;702e	28 06 	( . 
	set 1,(hl)		;7030	cb ce 	. . 
	djnz l7036h		;7032	10 02 	. . 
	res 1,(hl)		;7034	cb 8e 	. . 
l7036h:
	pop bc			;7036	c1 	. 
	inc b			;7037	04 	. 
	jr z,l7040h		;7038	28 06 	( . 
	set 0,(hl)		;703a	cb c6 	. . 
	djnz l7040h		;703c	10 02 	. . 
	res 0,(hl)		;703e	cb 86 	. . 
l7040h:
	ld a,(00006h)		;7040	3a 06 00 	: . . 
	ld c,a			;7043	4f 	O 
	inc c			;7044	0c 	. 
	ld a,002h		;7045	3e 02 	> . 
	di			;7047	f3 	. 
	call sub_709ah		;7048	cd 9a 70 	. . p 
l704bh:
	in a,(c)		;704b	ed 78 	. x 
	add a,a			;704d	87 	. 
	jp m,l704bh		;704e	fa 4b 70 	. K p 
l7051h:
	in a,(c)		;7051	ed 78 	. x 
	add a,a			;7053	87 	. 
	jp p,l7051h		;7054	f2 51 70 	. Q p 
	xor a			;7057	af 	. 
	call sub_709ah		;7058	cd 9a 70 	. . p 
	ld a,(0fff6h)		;705b	3a f6 ff 	: . . 
	out (c),a		;705e	ed 79 	. y 
	ld a,097h		;7060	3e 97 	> . 
	out (c),a		;7062	ed 79 	. y 
	ld a,019h		;7064	3e 19 	> . 
	out (c),a		;7066	ed 79 	. y 
	ld a,091h		;7068	3e 91 	> . 
	out (c),a		;706a	ed 79 	. y 
	inc c			;706c	0c 	. 
	inc c			;706d	0c 	. 
	ld b,003h		;706e	06 03 	. . 
	ld hl,0fffah		;7070	21 fa ff 	! . . 
	otir		;7073	ed b3 	. . 
	ei			;7075	fb 	. 
	ret			;7076	c9 	. 
sub_7077h:
	ld a,024h		;7077	3e 24 	> $ 
sub_7079h:
	push af			;7079	f5 	. 
	ld a,(00006h)		;707a	3a 06 00 	: . . 
	ld c,a			;707d	4f 	O 
	inc c			;707e	0c 	. 
	pop af			;707f	f1 	. 
	di			;7080	f3 	. 
	out (c),a		;7081	ed 79 	. y 
	ld a,091h		;7083	3e 91 	> . 
	ei			;7085	fb 	. 
	out (c),a		;7086	ed 79 	. y 
sub_7088h:
	ld a,002h		;7088	3e 02 	> . 
	di			;708a	f3 	. 
	call sub_709ah		;708b	cd 9a 70 	. . p 
	in a,(c)		;708e	ed 78 	. x 
	rra			;7090	1f 	. 
	ld a,000h		;7091	3e 00 	> . 
	call sub_709ah		;7093	cd 9a 70 	. . p 
	ei			;7096	fb 	. 
	jr c,sub_7088h		;7097	38 ef 	8 . 
	ret			;7099	c9 	. 
sub_709ah:
	out (c),a		;709a	ed 79 	. y 
	ld a,08fh		;709c	3e 8f 	> . 
	out (c),a		;709e	ed 79 	. y 
	ret			;70a0	c9 	. 
l70a1h:
	call sub_70a9h		;70a1	cd a9 70 	. . p 
	ld h,000h		;70a4	26 00 	& . 
	in l,(c)		;70a6	ed 68 	. h 
	ret			;70a8	c9 	. 
sub_70a9h:
	ld a,(00006h)		;70a9	3a 06 00 	: . . 
	ld c,a			;70ac	4f 	O 
sub_70adh:
	call sub_73fah		;70ad	cd fa 73 	. . s 
	ei			;70b0	fb 	. 
	out (c),a		;70b1	ed 79 	. y 
	dec c			;70b3	0d 	. 
	ret			;70b4	c9 	. 
l70b5h:
	ld b,a			;70b5	47 	G 
	call sub_71afh		;70b6	cd af 71 	. . q 
	out (c),b		;70b9	ed 41 	. A 
	ret			;70bb	c9 	. 
l70bch:
	and 003h		;70bc	e6 03 	. . 
	ld hl,0f3e0h		;70be	21 e0 f3 	! . . 
	res 0,(hl)		;70c1	cb 86 	. . 
	res 1,(hl)		;70c3	cb 8e 	. . 
	or (hl)			;70c5	b6 	. 
	ld (hl),a			;70c6	77 	w 
	jp BIOS_CLRSPR		;70c7	c3 69 00 	. i . 
sub_70cah:
	ld (0faf5h),a		;70ca	32 f5 fa 	2 . . 
	rrca			;70cd	0f 	. 
	rrca			;70ce	0f 	. 
	rrca			;70cf	0f 	. 
	and 0e0h		;70d0	e6 e0 	. . 
	ld e,a			;70d2	5f 	_ 
	ld a,(0f3e1h)		;70d3	3a e1 f3 	: . . 
	and 01fh		;70d6	e6 1f 	. . 
	ld c,002h		;70d8	0e 02 	. . 
	call sub_7107h		;70da	cd 07 71 	. . q 
	ld a,(BIOS_SCRMOD)		;70dd	3a af fc 	: . . 
	cp 007h		;70e0	fe 07 	. . 
	ld a,(0faf5h)		;70e2	3a f5 fa 	: . . 
	ld d,0fch		;70e5	16 fc 	. . 
	jr c,l70ech		;70e7	38 03 	8 . 
	add a,a			;70e9	87 	. 
	ld d,0fdh		;70ea	16 fd 	. . 
l70ech:
	ld e,a			;70ec	5f 	_ 
	ld a,(0ffeah)		;70ed	3a ea ff 	: . . 
	ld c,00bh		;70f0	0e 0b 	. . 
	call sub_7106h		;70f2	cd 06 71 	. . q 
	ld a,d			;70f5	7a 	z 
	rlca			;70f6	07 	. 
	rlca			;70f7	07 	. 
	rlca			;70f8	07 	. 
	rlca			;70f9	07 	. 
	ld d,a			;70fa	57 	W 
	ld a,e			;70fb	7b 	{ 
	rlca			;70fc	07 	. 
	rlca			;70fd	07 	. 
	rlca			;70fe	07 	. 
	rlca			;70ff	07 	. 
	ld e,a			;7100	5f 	_ 
	ld a,(0f3e5h)		;7101	3a e5 f3 	: . . 
	ld c,006h		;7104	0e 06 	. . 
sub_7106h:
	and d			;7106	a2 	. 
sub_7107h:
	or e			;7107	b3 	. 
	ld b,a			;7108	47 	G 
	jp 00047h		;7109	c3 47 00 	. G . 
sub_710ch:
	push af			;710c	f5 	. 
	push bc			;710d	c5 	. 
	push de			;710e	d5 	. 
	ld ix,00149h		;710f	dd 21 49 01 	. ! I . 
	call BIOS_EXTROM		;7113	cd 5f 01 	. _ . 
	pop de			;7116	d1 	. 
	ld l,c			;7117	69 	i 
	ld a,b			;7118	78 	x 
	rlca			;7119	07 	. 
	rlca			;711a	07 	. 
	rlca			;711b	07 	. 
	rlca			;711c	07 	. 
	and 00fh		;711d	e6 0f 	. . 
	ld e,a			;711f	5f 	_ 
	ld a,b			;7120	78 	x 
	pop bc			;7121	c1 	. 
	and 00fh		;7122	e6 0f 	. . 
	ld c,a			;7124	4f 	O 
	ld a,d			;7125	7a 	z 
	inc a			;7126	3c 	< 
	jr nz,l712ah		;7127	20 01 	  . 
	ld d,e			;7129	53 	S 
l712ah:
	ld a,h			;712a	7c 	| 
	inc a			;712b	3c 	< 
	jr nz,l712fh		;712c	20 01 	  . 
	ld h,l			;712e	65 	e 
l712fh:
	ld a,b			;712f	78 	x 
	inc a			;7130	3c 	< 
	jr nz,l7134h		;7131	20 01 	  . 
	ld b,c			;7133	41 	A 
l7134h:
	ld a,d			;7134	7a 	z 
	add a,a			;7135	87 	. 
	add a,a			;7136	87 	. 
	add a,a			;7137	87 	. 
	add a,a			;7138	87 	. 
	or b			;7139	b0 	. 
	pop de			;713a	d1 	. 
	ld e,h			;713b	5c 	\ 
	ld ix,0014dh		;713c	dd 21 4d 01 	. ! M . 
	jp BIOS_EXTROM		;7140	c3 5f 01 	. _ . 
sub_7143h:
	push hl			;7143	e5 	. 
	call BIOS_CALPAT		;7144	cd 84 00 	. . . 
	call sub_71afh		;7147	cd af 71 	. . q 
	call BIOS_GSPSIZ		;714a	cd 8a 00 	. . . 
	ld a,008h		;714d	3e 08 	> . 
	jr nc,l7153h		;714f	30 02 	0 . 
	ld a,020h		;7151	3e 20 	>   
l7153h:
	ld b,a			;7153	47 	G 
	pop hl			;7154	e1 	. 
	sub (hl)			;7155	96 	. 
	push af			;7156	f5 	. 
	jr c,l715eh		;7157	38 05 	8 . 
	ld b,(hl)			;7159	46 	F 
	inc b			;715a	04 	. 
	dec b			;715b	05 	. 
	jr z,l7162h		;715c	28 04 	( . 
l715eh:
	inc hl			;715e	23 	# 
	call sub_71a2h		;715f	cd a2 71 	. . q 
l7162h:
	pop af			;7162	f1 	. 
	ret z			;7163	c8 	. 
	ret c			;7164	d8 	. 
	ld b,a			;7165	47 	G 
	xor a			;7166	af 	. 
l7167h:
	out (c),a		;7167	ed 79 	. y 
	djnz l7167h		;7169	10 fc 	. . 
	ret			;716b	c9 	. 
sub_716ch:
	call BIOS_CALPAT		;716c	cd 84 00 	. . . 
	call sub_70a9h		;716f	cd a9 70 	. . p 
	call BIOS_GSPSIZ		;7172	cd 8a 00 	. . . 
	ld b,008h		;7175	06 08 	. . 
	jr nc,l717bh		;7177	30 02 	0 . 
	ld b,020h		;7179	06 20 	.   
l717bh:
	ld hl,BASIC_BUF		;717b	21 5e f5 	! ^ . 
	push hl			;717e	e5 	. 
	ld (hl),b			;717f	70 	p 
	inc hl			;7180	23 	# 
l7181h:
	ini		;7181	ed a2 	. . 
	jr nz,l7181h		;7183	20 fc 	  . 
	pop hl			;7185	e1 	. 
	ret			;7186	c9 	. 
sub_7187h:
	and 07fh		;7187	e6 7f 	.  
	push af			;7189	f5 	. 
	call sub_71a7h		;718a	cd a7 71 	. . q 
	pop af			;718d	f1 	. 
	ld b,010h		;718e	06 10 	. . 
	jr l7167h		;7190	18 d5 	. . 
sub_7192h:
	push hl			;7192	e5 	. 
	call sub_71a7h		;7193	cd a7 71 	. . q 
	pop hl			;7196	e1 	. 
	ld a,(hl)			;7197	7e 	~ 
	and a			;7198	a7 	. 
	ret z			;7199	c8 	. 
	cp 011h		;719a	fe 11 	. . 
	jr c,l71a0h		;719c	38 02 	8 . 
	ld a,010h		;719e	3e 10 	> . 
l71a0h:
	ld b,a			;71a0	47 	G 
	inc hl			;71a1	23 	# 
sub_71a2h:
	outi		;71a2	ed a3 	. . 
	jr nz,sub_71a2h		;71a4	20 fc 	  . 
	ret			;71a6	c9 	. 
sub_71a7h:
	ld a,b			;71a7	78 	x 
	add a,a			;71a8	87 	. 
	add a,a			;71a9	87 	. 
	call BIOS_CALATR		;71aa	cd 87 00 	. . . 
	dec h			;71ad	25 	% 
	dec h			;71ae	25 	% 
sub_71afh:
	ld a,(00006h)		;71af	3a 06 00 	: . . 
	ld c,a			;71b2	4f 	O 
sub_71b3h:
	call sub_73fah		;71b3	cd fa 73 	. . s 
	or 040h		;71b6	f6 40 	. @ 
	ei			;71b8	fb 	. 
	out (c),a		;71b9	ed 79 	. y 
	dec c			;71bb	0d 	. 
	ret			;71bc	c9 	. 
sub_71bdh:
	push af			;71bd	f5 	. 
	ld a,(00006h)		;71be	3a 06 00 	: . . 
	ld c,a			;71c1	4f 	O 
	ld a,(BIOS_SCRMOD)		;71c2	3a af fc 	: . . 
	cp 004h		;71c5	fe 04 	. . 
	jp nc,l722ch		;71c7	d2 2c 72 	. , r 
	pop af			;71ca	f1 	. 
	push hl			;71cb	e5 	. 
	push de			;71cc	d5 	. 
	call BIOS_CALATR		;71cd	cd 87 00 	. . . 
	pop de			;71d0	d1 	. 
	push hl			;71d1	e5 	. 
	call sub_70adh		;71d2	cd ad 70 	. . p 
	ld a,b			;71d5	78 	x 
	ld hl,0fc18h		;71d6	21 18 fc 	! . . 
	ld b,004h		;71d9	06 04 	. . 
l71dbh:
	ini		;71db	ed a2 	. . 
	jr nz,l71dbh		;71dd	20 fc 	  . 
	ld b,a			;71df	47 	G 
	bit 7,b		;71e0	cb 78 	. x 
	jr z,l71ffh		;71e2	28 1b 	( . 
	ld (0fc18h),iy		;71e4	fd 22 18 fc 	. " . . 
	push ix		;71e8	dd e5 	. . 
	pop hl			;71ea	e1 	. 
	ld a,h			;71eb	7c 	| 
	and 080h		;71ec	e6 80 	. . 
	ld e,a			;71ee	5f 	_ 
	ld a,l			;71ef	7d 	} 
	ld hl,0fc1bh		;71f0	21 1b fc 	! . . 
	jr z,l71f7h		;71f3	28 02 	( . 
	add a,020h		;71f5	c6 20 	.   
l71f7h:
	ld (0fc19h),a		;71f7	32 19 fc 	2 . . 
	ld a,(hl)			;71fa	7e 	~ 
	and 07fh		;71fb	e6 7f 	.  
	or e			;71fd	b3 	. 
	ld (hl),a			;71fe	77 	w 
l71ffh:
	bit 6,b		;71ff	cb 70 	. p 
	jr z,l720fh		;7201	28 0c 	( . 
	ld hl,0fc1bh		;7203	21 1b fc 	! . . 
	ld a,d			;7206	7a 	z 
	and 00fh		;7207	e6 0f 	. . 
	ld d,a			;7209	57 	W 
	ld a,(hl)			;720a	7e 	~ 
	and 0f0h		;720b	e6 f0 	. . 
	or d			;720d	b2 	. 
	ld (hl),a			;720e	77 	w 
l720fh:
	pop hl			;720f	e1 	. 
	ex (sp),hl			;7210	e3 	. 
	bit 5,b		;7211	cb 68 	. h 
	jr z,l7220h		;7213	28 0b 	( . 
	call BIOS_GSPSIZ		;7215	cd 8a 00 	. . . 
	ld a,h			;7218	7c 	| 
	jr nc,l721dh		;7219	30 02 	0 . 
	add a,a			;721b	87 	. 
	add a,a			;721c	87 	. 
l721dh:
	ld (0fc1ah),a		;721d	32 1a fc 	2 . . 
l7220h:
	pop hl			;7220	e1 	. 
	call sub_71b3h		;7221	cd b3 71 	. . q 
	ld hl,0fc18h		;7224	21 18 fc 	! . . 
	ld b,004h		;7227	06 04 	. . 
	jp sub_71a2h		;7229	c3 a2 71 	. . q 
l722ch:
	pop af			;722c	f1 	. 
	push af			;722d	f5 	. 
	push hl			;722e	e5 	. 
	push de			;722f	d5 	. 
	push af			;7230	f5 	. 
	call BIOS_CALATR		;7231	cd 87 00 	. . . 
	pop af			;7234	f1 	. 
	push hl			;7235	e5 	. 
	push af			;7236	f5 	. 
	call sub_70adh		;7237	cd ad 70 	. . p 
	ld a,b			;723a	78 	x 
	ld hl,0fc18h		;723b	21 18 fc 	! . . 
	ld b,003h		;723e	06 03 	. . 
l7240h:
	ini		;7240	ed a2 	. . 
	jr nz,l7240h		;7242	20 fc 	  . 
	ld b,a			;7244	47 	G 
	pop af			;7245	f1 	. 
	add a,a			;7246	87 	. 
	add a,a			;7247	87 	. 
	call BIOS_CALATR		;7248	cd 87 00 	. . . 
	dec h			;724b	25 	% 
	dec h			;724c	25 	% 
	push hl			;724d	e5 	. 
	call sub_70adh		;724e	cd ad 70 	. . p 
	ld a,b			;7251	78 	x 
	ld hl,0fc1ch		;7252	21 1c fc 	! . . 
	ld b,010h		;7255	06 10 	. . 
l7257h:
	ini		;7257	ed a2 	. . 
	jr nz,l7257h		;7259	20 fc 	  . 
	ld b,a			;725b	47 	G 
	ld e,000h		;725c	1e 00 	. . 
	ld l,0ffh		;725e	2e ff 	. . 
	bit 7,b		;7260	cb 78 	. x 
	jr z,l7279h		;7262	28 15 	( . 
	ld (0fc18h),iy		;7264	fd 22 18 fc 	. " . . 
	push ix		;7268	dd e5 	. . 
	pop hl			;726a	e1 	. 
	ld a,h			;726b	7c 	| 
	and 080h		;726c	e6 80 	. . 
	ld e,a			;726e	5f 	_ 
	ld a,l			;726f	7d 	} 
	jr z,l7274h		;7270	28 02 	( . 
	add a,020h		;7272	c6 20 	.   
l7274h:
	ld (0fc19h),a		;7274	32 19 fc 	2 . . 
	ld l,07fh		;7277	2e 7f 	.  
l7279h:
	pop ix		;7279	dd e1 	. . 
	pop iy		;727b	fd e1 	. . 
	pop af			;727d	f1 	. 
	bit 6,b		;727e	cb 70 	. p 
	jr z,l728ah		;7280	28 08 	( . 
	and 00fh		;7282	e6 0f 	. . 
	or e			;7284	b3 	. 
	ld e,a			;7285	5f 	_ 
	ld a,l			;7286	7d 	} 
	and 0f0h		;7287	e6 f0 	. . 
	ld l,a			;7289	6f 	o 
l728ah:
	push bc			;728a	c5 	. 
	ld d,l			;728b	55 	U 
	ld hl,0fc1ch		;728c	21 1c fc 	! . . 
	ld b,010h		;728f	06 10 	. . 
l7291h:
	ld a,(hl)			;7291	7e 	~ 
	and d			;7292	a2 	. 
	or e			;7293	b3 	. 
	ld (hl),a			;7294	77 	w 
	inc hl			;7295	23 	# 
	djnz l7291h		;7296	10 f9 	. . 
	pop bc			;7298	c1 	. 
	pop hl			;7299	e1 	. 
	bit 5,b		;729a	cb 68 	. h 
	jr z,l72a9h		;729c	28 0b 	( . 
	call BIOS_GSPSIZ		;729e	cd 8a 00 	. . . 
	ld a,h			;72a1	7c 	| 
	jr nc,l72a6h		;72a2	30 02 	0 . 
	add a,a			;72a4	87 	. 
	add a,a			;72a5	87 	. 
l72a6h:
	ld (0fc1ah),a		;72a6	32 1a fc 	2 . . 
l72a9h:
	push ix		;72a9	dd e5 	. . 
	pop hl			;72ab	e1 	. 
	call sub_71b3h		;72ac	cd b3 71 	. . q 
	ld hl,0fc1ch		;72af	21 1c fc 	! . . 
	ld b,010h		;72b2	06 10 	. . 
	call sub_71a2h		;72b4	cd a2 71 	. . q 
	push iy		;72b7	fd e5 	. . 
	pop hl			;72b9	e1 	. 
	call sub_71b3h		;72ba	cd b3 71 	. . q 
	ld hl,0fc18h		;72bd	21 18 fc 	! . . 
	ld b,003h		;72c0	06 03 	. . 
	call sub_71a2h		;72c2	cd a2 71 	. . q 
	pop de			;72c5	d1 	. 
	ld a,01fh		;72c6	3e 1f 	> . 
	sub d			;72c8	92 	. 
	ret z			;72c9	c8 	. 
	ld e,a			;72ca	5f 	_ 
	ld d,a			;72cb	57 	W 
	push ix		;72cc	dd e5 	. . 
	pop hl			;72ce	e1 	. 
l72cfh:
	ld a,l			;72cf	7d 	} 
	add a,010h		;72d0	c6 10 	. . 
	ld l,a			;72d2	6f 	o 
	jr nc,l72d6h		;72d3	30 01 	0 . 
	inc h			;72d5	24 	$ 
l72d6h:
	call sub_70adh		;72d6	cd ad 70 	. . p 
	ld b,010h		;72d9	06 10 	. . 
l72dbh:
	in a,(c)		;72db	ed 78 	. x 
	and 040h		;72dd	e6 40 	. @ 
	jr nz,l72e5h		;72df	20 04 	  . 
	djnz l72dbh		;72e1	10 f8 	. . 
	jr l72e8h		;72e3	18 03 	. . 
l72e5h:
	dec e			;72e5	1d 	. 
	jr nz,l72cfh		;72e6	20 e7 	  . 
l72e8h:
	ld a,d			;72e8	7a 	z 
	sub e			;72e9	93 	. 
	ret z			;72ea	c8 	. 
	ld e,a			;72eb	5f 	_ 
	push ix		;72ec	dd e5 	. . 
	pop hl			;72ee	e1 	. 
	push de			;72ef	d5 	. 
	ld a,(0fc1ch)		;72f0	3a 1c fc 	: . . 
	and 080h		;72f3	e6 80 	. . 
	ld d,a			;72f5	57 	W 
l72f6h:
	ld a,l			;72f6	7d 	} 
	add a,010h		;72f7	c6 10 	. . 
	ld l,a			;72f9	6f 	o 
	jr nc,l72fdh		;72fa	30 01 	0 . 
	inc h			;72fc	24 	$ 
l72fdh:
	push hl			;72fd	e5 	. 
	call sub_70adh		;72fe	cd ad 70 	. . p 
	ld hl,0fc2ch		;7301	21 2c fc 	! , . 
	ld b,010h		;7304	06 10 	. . 
l7306h:
	in a,(c)		;7306	ed 78 	. x 
	and 07fh		;7308	e6 7f 	.  
	or d			;730a	b2 	. 
	ld (hl),a			;730b	77 	w 
	inc hl			;730c	23 	# 
	djnz l7306h		;730d	10 f7 	. . 
	pop hl			;730f	e1 	. 
	push hl			;7310	e5 	. 
	call sub_71b3h		;7311	cd b3 71 	. . q 
	ld hl,0fc2ch		;7314	21 2c fc 	! , . 
	ld b,010h		;7317	06 10 	. . 
	call sub_71a2h		;7319	cd a2 71 	. . q 
	pop hl			;731c	e1 	. 
	dec e			;731d	1d 	. 
	jr nz,l72f6h		;731e	20 d6 	  . 
	pop de			;7320	d1 	. 
	ld b,e			;7321	43 	C 
	push iy		;7322	fd e5 	. . 
	pop hl			;7324	e1 	. 
l7325h:
	inc hl			;7325	23 	# 
	inc hl			;7326	23 	# 
	inc hl			;7327	23 	# 
	inc hl			;7328	23 	# 
	call sub_71b3h		;7329	cd b3 71 	. . q 
	ld de,(0fc18h)		;732c	ed 5b 18 fc 	. [ . . 
	out (c),e		;7330	ed 59 	. Y 
	out (c),d		;7332	ed 51 	. Q 
	djnz l7325h		;7334	10 ef 	. . 
	ret			;7336	c9 	. 
l7337h:
	ld a,h			;7337	7c 	| 
	and a			;7338	a7 	. 
	jr z,l7350h		;7339	28 15 	( . 
	ld a,(00006h)		;733b	3a 06 00 	: . . 
	ld c,a			;733e	4f 	O 
	inc c			;733f	0c 	. 
	xor a			;7340	af 	. 
	sub l			;7341	95 	. 
	di			;7342	f3 	. 
	call sub_709ah		;7343	cd 9a 70 	. . p 
	in l,(c)		;7346	ed 68 	. h 
	xor a			;7348	af 	. 
	call sub_709ah		;7349	cd 9a 70 	. . p 
	ei			;734c	fb 	. 
	ld h,000h		;734d	26 00 	& . 
	ret			;734f	c9 	. 
l7350h:
	ld de,0f3dfh		;7350	11 df f3 	. . . 
	ld a,l			;7353	7d 	} 
	cp 009h		;7354	fe 09 	. . 
	jr c,l7362h		;7356	38 0a 	8 . 
	ld de,0ffdeh		;7358	11 de ff 	. . . 
	cp 019h		;735b	fe 19 	. . 
	jr c,l7362h		;735d	38 03 	8 . 
	ld de,0ffe0h		;735f	11 e0 ff 	. . . 
l7362h:
	add hl,de			;7362	19 	. 
	ld l,(hl)			;7363	6e 	n 
	ld h,000h		;7364	26 00 	& . 
	ret			;7366	c9 	. 
CMD_SCREEN:
	cp 009h		;7367	fe 09 	. . 
	jp c,BIOS_CHGMOD		;7369	da 5f 00 	. _ . 
	ld hl,0fffah		;736c	21 fa ff 	! . . 
	ld b,(hl)			;736f	46 	F 
	ld c,b			;7370	48 	H 
	set 3,b		;7371	cb d8 	. . 
	res 4,b		;7373	cb a0 	. . 
	cp 00ch		;7375	fe 0c 	. . 
	jr z,l73ach		;7377	28 33 	( 3 
	set 4,b		;7379	cb e0 	. . 
	ld hl,MODE		;737b	21 fc fa 	! . . 
	cp 00bh		;737e	fe 0b 	. . 
	set 5,(hl)		;7380	cb ee 	. . 
	jr z,l7386h		;7382	28 02 	( . 
	res 5,(hl)		;7384	cb ae 	. . 
l7386h:
	bit 3,c		;7386	cb 59 	. Y 
	jr z,l73b0h		;7388	28 26 	( & 
	bit 4,c		;738a	cb 61 	. a 
	ret nz			;738c	c0 	. 
	push bc			;738d	c5 	. 
	ld a,0f7h		;738e	3e f7 	> . 
	ld b,001h		;7390	06 01 	. . 
	ld de,00000h		;7392	11 00 00 	. . . 
	ld l,e			;7395	6b 	k 
	ld h,d			;7396	62 	b 
	ld ix,000ffh		;7397	dd 21 ff 00 	. ! . . 
	ld iy,000ffh		;739b	fd 21 ff 00 	. ! . . 
	call sub_6e34h		;739f	cd 34 6e 	. 4 n 
	ld a,(00006h)		;73a2	3a 06 00 	: . . 
	ld c,a			;73a5	4f 	O 
	inc c			;73a6	0c 	. 
	call sub_7088h		;73a7	cd 88 70 	. . p 
	jr l73b6h		;73aa	18 0a 	. . 
l73ach:
	bit 3,c		;73ac	cb 59 	. Y 
	jr nz,l73b7h		;73ae	20 07 	  . 
l73b0h:
	push bc			;73b0	c5 	. 
	ld a,008h		;73b1	3e 08 	> . 
	call BIOS_CHGMOD		;73b3	cd 5f 00 	. _ . 
l73b6h:
	pop bc			;73b6	c1 	. 
l73b7h:
	ld c,019h		;73b7	0e 19 	. . 
	jp 00047h		;73b9	c3 47 00 	. G . 
sub_73bch:
	ld a,(0fffah)		;73bc	3a fa ff 	: . . 
	and 018h		;73bf	e6 18 	. . 
	cp 018h		;73c1	fe 18 	. . 
	ret nz			;73c3	c0 	. 
	ld a,(MODE)		;73c4	3a fc fa 	: . . 
	and 020h		;73c7	e6 20 	.   
	ret			;73c9	c9 	. 
sub_73cah:
	ld c,a			;73ca	4f 	O 
	call sub_73bch		;73cb	cd bc 73 	. . s 
	ld a,c			;73ce	79 	y 
	ret nz			;73cf	c0 	. 
	ex (sp),hl			;73d0	e3 	. 
	ld (0f7c0h),hl		;73d1	22 c0 f7 	" . . 
	pop hl			;73d4	e1 	. 
	inc b			;73d5	04 	. 
	djnz l73f0h		;73d6	10 18 	. . 
	push af			;73d8	f5 	. 
	push de			;73d9	d5 	. 
	push hl			;73da	e5 	. 
	push ix		;73db	dd e5 	. . 
	push iy		;73dd	fd e5 	. . 
	ld a,00fh		;73df	3e 0f 	> . 
	ld b,001h		;73e1	06 01 	. . 
	call sub_73f4h		;73e3	cd f4 73 	. . s 
	pop iy		;73e6	fd e1 	. . 
	pop ix		;73e8	dd e1 	. . 
	pop hl			;73ea	e1 	. 
	pop de			;73eb	d1 	. 
	pop af			;73ec	f1 	. 
	ld b,002h		;73ed	06 02 	. . 
	scf			;73ef	37 	7 
l73f0h:
	rla			;73f0	17 	. 
	rla			;73f1	17 	. 
	rla			;73f2	17 	. 
	rla			;73f3	17 	. 
sub_73f4h:
	push hl			;73f4	e5 	. 
	ld hl,(0f7c0h)		;73f5	2a c0 f7 	* . . 
	ex (sp),hl			;73f8	e3 	. 
	ret			;73f9	c9 	. 
sub_73fah:
	inc c			;73fa	0c 	. 
	ld a,(BIOS_VERSION)		;73fb	3a 2d 00 	: - . 
	and a			;73fe	a7 	. 
	jr z,l741dh		;73ff	28 1c 	( . 
	push de			;7401	d5 	. 
	ld a,h			;7402	7c 	| 
	rlca			;7403	07 	. 
	rlca			;7404	07 	. 
	and 003h		;7405	e6 03 	. . 
	ld e,a			;7407	5f 	_ 
	ld a,(BIOS_SCRMOD)		;7408	3a af fc 	: . . 
	cp 007h		;740b	fe 07 	. . 
	ld a,(0faf6h)		;740d	3a f6 fa 	: . . 
	jr c,l7413h		;7410	38 01 	8 . 
	add a,a			;7412	87 	. 
l7413h:
	add a,a			;7413	87 	. 
	add a,e			;7414	83 	. 
	pop de			;7415	d1 	. 
	di			;7416	f3 	. 
	out (c),a		;7417	ed 79 	. y 
	ld a,08eh		;7419	3e 8e 	> . 
	out (c),a		;741b	ed 79 	. y 
l741dh:
	di			;741d	f3 	. 
	out (c),l		;741e	ed 69 	. i 
	ld a,h			;7420	7c 	| 
	and 03fh		;7421	e6 3f 	. ? 
	ret			;7423	c9 	. 
sub_7424h:
	push hl			;7424	e5 	. 
	push de			;7425	d5 	. 
	push ix		;7426	dd e5 	. . 
	pop de			;7428	d1 	. 
	push iy		;7429	fd e5 	. . 
	pop hl			;742b	e1 	. 
	call sub_7438h		;742c	cd 38 74 	. 8 t 
	push hl			;742f	e5 	. 
	pop iy		;7430	fd e1 	. . 
	push de			;7432	d5 	. 
	pop ix		;7433	dd e1 	. . 
	pop de			;7435	d1 	. 
	pop hl			;7436	e1 	. 
	ret			;7437	c9 	. 
sub_7438h:
	ld a,(MODE)		;7438	3a fc fa 	: . . 
	and 010h		;743b	e6 10 	. . 
	ret nz			;743d	c0 	. 
	ld a,d			;743e	7a 	z 
	add a,a			;743f	87 	. 
	jr nc,l7447h		;7440	30 05 	0 . 
	ld de,00000h		;7442	11 00 00 	. . . 
	jr l7453h		;7445	18 0c 	. . 
l7447h:
	ld a,(BIOS_SCRMOD)		;7447	3a af fc 	: . . 
	rrca			;744a	0f 	. 
	and 001h		;744b	e6 01 	. . 
	cp d			;744d	ba 	. 
	jr nc,l7453h		;744e	30 03 	0 . 
	ld e,0ffh		;7450	1e ff 	. . 
	ld d,a			;7452	57 	W 
l7453h:
	ld a,h			;7453	7c 	| 
	add a,a			;7454	87 	. 
	jr nc,l745bh		;7455	30 04 	0 . 
	ld hl,00000h		;7457	21 00 00 	! . . 
	ret			;745a	c9 	. 
l745bh:
	jr nz,l7461h		;745b	20 04 	  . 
	ld a,l			;745d	7d 	} 
	cp 0d4h		;745e	fe d4 	. . 
	ret c			;7460	d8 	. 
l7461h:
	ld hl,000d3h		;7461	21 d3 00 	! . . 
	ret			;7464	c9 	. 
l7465h:
	xor a			;7465	af 	. 
	ld ix,BIOS_CLS		;7466	dd 21 c3 00 	. ! . . 
	jp l74e5h		;746a	c3 e5 74 	. . t 
l746dh:
	ld bc,l58fch		;746d	01 fc 58 	. . X 
	jr l747ah		;7470	18 08 	. . 
l7472h:
	ld bc,l5912h		;7472	01 12 59 	. . Y 
	jr l747ah		;7475	18 03 	. . 
l7477h:
	ld bc,l58c1h		;7477	01 c1 58 	. . X 
l747ah:
	pop af			;747a	f1 	. 
	ld (BIOS_ATRBYT),a		;747b	32 f2 f3 	2 . . 
	push bc			;747e	c5 	. 
	ld (0fcb3h),de		;747f	ed 53 b3 fc 	. S . . 
	ld (0fcb5h),hl		;7483	22 b5 fc 	" . . 
	push ix		;7486	dd e5 	. . 
	pop bc			;7488	c1 	. 
	push iy		;7489	fd e5 	. . 
	pop de			;748b	d1 	. 
	pop ix		;748c	dd e1 	. . 
	jp l74e5h		;748e	c3 e5 74 	. . t 
l7491h:
	pop af			;7491	f1 	. 
	ld (BIOS_ATRBYT),a		;7492	32 f2 f3 	2 . . 
	ld c,e			;7495	4b 	K 
	ld b,d			;7496	42 	B 
	ex de,hl			;7497	eb 	. 
	ld ix,l57f5h		;7498	dd 21 f5 57 	. ! . W 
	jp l74e5h		;749c	c3 e5 74 	. . t 
l749fh:
	ld c,e			;749f	4b 	K 
	ld b,d			;74a0	42 	B 
	ex de,hl			;74a1	eb 	. 
	call BIOS_SCALXY		;74a2	cd 0e 01 	. . . 
	ld hl,0ffffh		;74a5	21 ff ff 	! . . 
	ret nc			;74a8	d0 	. 
	call BIOS_MAPXYC		;74a9	cd 11 01 	. . . 
	call BIOS_READC		;74ac	cd 1d 01 	. . . 
	ld l,a			;74af	6f 	o 
	ld h,000h		;74b0	26 00 	& . 
	ret			;74b2	c9 	. 
CMD_PAINT:
	ld (0fcb2h),a		;74b3	32 b2 fc 	2 . . 
	ld a,b			;74b6	78 	x 
	ld (BIOS_ATRBYT),a		;74b7	32 f2 f3 	2 . . 
	ld c,e			;74ba	4b 	K 
	ld b,d			;74bb	42 	B 
	ex de,hl			;74bc	eb 	. 
	ld a,(BIOS_SCRMOD)		;74bd	3a af fc 	: . . 
	cp 005h		;74c0	fe 05 	. . 
	jp c,l74cch		;74c2	da cc 74 	. . t 
	ld ix,0266eh		;74c5	dd 21 6e 26 	. ! n & 
	jp BIOS_EXTROM		;74c9	c3 5f 01 	. _ . 
l74cch:
	ld ix,059e3h		;74cc	dd 21 e3 59 	. ! . Y 
	jp l74e5h		;74d0	c3 e5 74 	. . t 
CMD_CIRCLE:
	ex de,hl			;74d3	eb 	. 
	ld hl,0fc1bh		;74d4	21 1b fc 	! . . 
	ld (hl),000h		;74d7	36 00 	6 . 
	dec hl			;74d9	2b 	+ 
	ld (hl),a			;74da	77 	w 
	dec hl			;74db	2b 	+ 
	ld (hl),00fh		;74dc	36 0f 	6 . 
	dec hl			;74de	2b 	+ 
	ld (hl),02ch		;74df	36 2c 	6 , 
	ld ix,05b19h		;74e1	dd 21 19 5b 	. ! . [ 
l74e5h:
	call BIOS_CALBAS		;74e5	cd 59 01 	. Y . 
	ei			;74e8	fb 	. 
	ret			;74e9	c9 	. 
sub_74eah:
	ld a,009h		;74ea	3e 09 	> . 
	rst 18h			;74ec	df 	. 
	ret			;74ed	c9 	. 
CMD_PRINT_CRLF:
	ld a,00dh		;74ee	3e 0d 	> . 
	rst 18h			;74f0	df 	. 
	ld a,00ah		;74f1	3e 0a 	> . 
	rst 18h			;74f3	df 	. 
	ret			;74f4	c9 	. 
CMD_PRINT_STR:
	ld b,(hl)			;74f5	46 	F 
	ld a,b			;74f6	78 	x 
	and a			;74f7	a7 	. 
	ret z			;74f8	c8 	. 
l74f9h:
	inc hl			;74f9	23 	# 
	ld a,(hl)			;74fa	7e 	~ 
	rst 18h			;74fb	df 	. 
	djnz l74f9h		;74fc	10 fb 	. . 
	ret			;74fe	c9 	. 
CMD_PRINT_INT:
	call AUX_INT_TO_STR		;74ff	cd 26 7b 	. & { 
	jr l7507h		;7502	18 03 	. . 
CMD_PRINT_FLOAT:
	call AUX_FLOAT_TO_STR		;7504	cd 80 7b 	. . { 
l7507h:
	call CMD_PRINT_STR		;7507	cd f5 74 	. . t 
	ld a,020h		;750a	3e 20 	>   
	rst 18h			;750c	df 	. 
	ret			;750d	c9 	. 
l750eh:
	call CMD_PRINT_STR		;750e	cd f5 74 	. . t 
l7511h:
	call BIOS_QINLIN		;7511	cd b4 00 	. . . 
	jp c,EXEC_RETURN_TO_BASIC		;7514	da fa 4d 	. . M 
	ex de,hl			;7517	eb 	. 
	inc de			;7518	13 	. 
	call sub_7c6dh		;7519	cd 6d 7c 	. m | 
	ld a,(de)			;751c	1a 	. 
	and a			;751d	a7 	. 
	ret z			;751e	c8 	. 
	ld hl,l7527h		;751f	21 27 75 	! ' u 
	call CMD_PRINT_STR		;7522	cd f5 74 	. . t 
	jr l7511h		;7525	18 ea 	. . 
l7527h:
	ld (de),a			;7527	12 	. 
	ccf			;7528	3f 	? 
	ld d,d			;7529	52 	R 
	ld h,l			;752a	65 	e 
	ld h,h			;752b	64 	d 
	ld l,a			;752c	6f 	o 
	jr nz,l7595h		;752d	20 66 	  f 
	ld (hl),d			;752f	72 	r 
	ld l,a			;7530	6f 	o 
l7531h:
	ld l,l			;7531	6d 	m 
	jr nz,$+117		;7532	20 73 	  s 
	ld (hl),h			;7534	74 	t 
	ld h,c			;7535	61 	a 
	ld (hl),d			;7536	72 	r 
	ld (hl),h			;7537	74 	t 
	dec c			;7538	0d 	. 
	ld a,(bc)			;7539	0a 	. 
l753ah:
	call CMD_PRINT_STR		;753a	cd f5 74 	. . t 
l753dh:
	call BIOS_QINLIN		;753d	cd b4 00 	. . . 
	jp c,EXEC_RETURN_TO_BASIC		;7540	da fa 4d 	. . M 
	inc h			;7543	24 	$ 
	ld e,l			;7544	5d 	] 
	ld d,h			;7545	54 	T 
	inc de			;7546	13 	. 
	ld bc,00100h		;7547	01 00 01 	. . . 
	lddr		;754a	ed b8 	. . 
	inc hl			;754c	23 	# 
	ld b,0ffh		;754d	06 ff 	. . 
l754fh:
	inc b			;754f	04 	. 
	inc de			;7550	13 	. 
	ld a,(de)			;7551	1a 	. 
	and a			;7552	a7 	. 
	jr nz,l754fh		;7553	20 fa 	  . 
	ld (hl),b			;7555	70 	p 
	ret			;7556	c9 	. 
l7557h:
	call sub_75b5h		;7557	cd b5 75 	. . u 
l755ah:
	ld a,(hl)			;755a	7e 	~ 
	inc hl			;755b	23 	# 
	cp 020h		;755c	fe 20 	.   
	jr z,l755ah		;755e	28 fa 	( . 
	ld de,0f55fh		;7560	11 5f f5 	. _ . 
	ld b,000h		;7563	06 00 	. . 
	cp 022h		;7565	fe 22 	. " 
	jr z,l758ah		;7567	28 21 	( ! 
	dec hl			;7569	2b 	+ 
l756ah:
	ld a,(hl)			;756a	7e 	~ 
	and a			;756b	a7 	. 
	jr z,l7582h		;756c	28 14 	( . 
	cp 03ah		;756e	fe 3a 	. : 
	jr z,l7582h		;7570	28 10 	( . 
	cp 02ch		;7572	fe 2c 	. , 
	jr z,l7582h		;7574	28 0c 	( . 
	ld (de),a			;7576	12 	. 
	inc de			;7577	13 	. 
	inc hl			;7578	23 	# 
	inc b			;7579	04 	. 
	jr l756ah		;757a	18 ee 	. . 
l757ch:
	inc hl			;757c	23 	# 
	ld a,(hl)			;757d	7e 	~ 
	cp 020h		;757e	fe 20 	.   
	jr z,l757ch		;7580	28 fa 	( . 
l7582h:
	ld (BASIC_DATPTR),hl		;7582	22 c8 f6 	" . . 
	ld hl,BASIC_BUF		;7585	21 5e f5 	! ^ . 
	ld (hl),b			;7588	70 	p 
	ret			;7589	c9 	. 
l758ah:
	ld a,(hl)			;758a	7e 	~ 
	and a			;758b	a7 	. 
	jr z,l7582h		;758c	28 f4 	( . 
	cp 022h		;758e	fe 22 	. " 
	jr z,l757ch		;7590	28 ea 	( . 
	ld (de),a			;7592	12 	. 
	inc de			;7593	13 	. 
	inc hl			;7594	23 	# 
l7595h:
	inc b			;7595	04 	. 
	jr l758ah		;7596	18 f2 	. . 
l7598h:
	call sub_75b5h		;7598	cd b5 75 	. . u 
	ex de,hl			;759b	eb 	. 
	call sub_7c6dh		;759c	cd 6d 7c 	. m | 
	ld (BASIC_DATPTR),de		;759f	ed 53 c8 f6 	. S . . 
	ld a,(de)			;75a3	1a 	. 
	and a			;75a4	a7 	. 
	ret z			;75a5	c8 	. 
	cp 03ah		;75a6	fe 3a 	. : 
	ret z			;75a8	c8 	. 
	cp 02ch		;75a9	fe 2c 	. , 
	ret z			;75ab	c8 	. 
	ld hl,(BIOS_DATLIN)		;75ac	2a a3 f6 	* . . 
	ld (BASIC_CURLIN),hl		;75af	22 1c f4 	" . . 
	jp 04e16h		;75b2	c3 16 4e 	. . N 
sub_75b5h:
	ld hl,(BASIC_DATPTR)		;75b5	2a c8 f6 	* . . 
	ld a,(hl)			;75b8	7e 	~ 
	cp 02ch		;75b9	fe 2c 	. , 
	jr nz,l75d3h		;75bb	20 16 	  . 
	inc hl			;75bd	23 	# 
	ret			;75be	c9 	. 
l75bfh:
	xor a			;75bf	af 	. 
	ld b,0ffh		;75c0	06 ff 	. . 
	cpir		;75c2	ed b1 	. . 
l75c4h:
	ld a,(hl)			;75c4	7e 	~ 
	inc hl			;75c5	23 	# 
	or (hl)			;75c6	b6 	. 
	jp z,ERROR_MSG_CODES		;75c7	ca 07 4e 	. . N 
	inc hl			;75ca	23 	# 
	ld e,(hl)			;75cb	5e 	^ 
	inc hl			;75cc	23 	# 
	ld d,(hl)			;75cd	56 	V 
	ld (BIOS_DATLIN),de		;75ce	ed 53 a3 f6 	. S . . 
	inc hl			;75d2	23 	# 
l75d3h:
	ld b,000h		;75d3	06 00 	. . 
l75d5h:
	ld a,(hl)			;75d5	7e 	~ 
	inc hl			;75d6	23 	# 
	and a			;75d7	a7 	. 
	jr z,l75c4h		;75d8	28 ea 	( . 
	cp 084h		;75da	fe 84 	. . 
	jr nz,l75e1h		;75dc	20 03 	  . 
	inc b			;75de	04 	. 
	dec b			;75df	05 	. 
	ret z			;75e0	c8 	. 
l75e1h:
	cp 022h		;75e1	fe 22 	. " 
	jr nz,l75e9h		;75e3	20 04 	  . 
	inc b			;75e5	04 	. 
	djnz l75d3h		;75e6	10 eb 	. . 
	inc b			;75e8	04 	. 
l75e9h:
	cp 0ffh		;75e9	fe ff 	. . 
	inc hl			;75eb	23 	# 
	jr z,l75d5h		;75ec	28 e7 	( . 
	dec hl			;75ee	2b 	+ 
	cp 08fh		;75ef	fe 8f 	. . 
	jr z,l75bfh		;75f1	28 cc 	( . 
	cp 020h		;75f3	fe 20 	.   
	jr nc,l75d5h		;75f5	30 de 	0 . 
	cp 00bh		;75f7	fe 0b 	. . 
	jr c,l75d5h		;75f9	38 da 	8 . 
	cp 00fh		;75fb	fe 0f 	. . 
	ld c,001h		;75fd	0e 01 	. . 
	jr z,l7613h		;75ff	28 12 	( . 
	inc c			;7601	0c 	. 
	jr c,l7613h		;7602	38 0f 	8 . 
	cp 01bh		;7604	fe 1b 	. . 
	jr c,l75d5h		;7606	38 cd 	8 . 
	sub 01ch		;7608	d6 1c 	. . 
	jr z,l7613h		;760a	28 07 	( . 
	dec a			;760c	3d 	= 
	ld c,004h		;760d	0e 04 	. . 
	jr z,l7613h		;760f	28 02 	( . 
	ld c,008h		;7611	0e 08 	. . 
l7613h:
	ld a,l			;7613	7d 	} 
	add a,c			;7614	81 	. 
	ld l,a			;7615	6f 	o 
	jr nc,l75d5h		;7616	30 bd 	0 . 
	inc h			;7618	24 	$ 
	jr l75d5h		;7619	18 ba 	. . 
sub_761bh:
	ld c,l			;761b	4d 	M 
	ld b,h			;761c	44 	D 
	ld hl,00000h		;761d	21 00 00 	! . . 
	ld a,010h		;7620	3e 10 	> . 
l7622h:
	add hl,hl			;7622	29 	) 
	ex de,hl			;7623	eb 	. 
	add hl,hl			;7624	29 	) 
	ex de,hl			;7625	eb 	. 
	jr nc,l7629h		;7626	30 01 	0 . 
	add hl,bc			;7628	09 	. 
l7629h:
	dec a			;7629	3d 	= 
	jr nz,l7622h		;762a	20 f6 	  . 
	ret			;762c	c9 	. 
l762dh:
	push de			;762d	d5 	. 
	push hl			;762e	e5 	. 
	bit 7,d		;762f	cb 7a 	. z 
	jr nz,l7639h		;7631	20 06 	  . 
	xor a			;7633	af 	. 
	sub e			;7634	93 	. 
	ld e,a			;7635	5f 	_ 
	sbc a,a			;7636	9f 	. 
	sub d			;7637	92 	. 
	ld d,a			;7638	57 	W 
l7639h:
	ld c,e			;7639	4b 	K 
l763ah:
	ld b,d			;763a	42 	B 
	bit 7,h		;763b	cb 7c 	. | 
	jr z,l7645h		;763d	28 06 	( . 
	xor a			;763f	af 	. 
	sub l			;7640	95 	. 
	ld l,a			;7641	6f 	o 
	sbc a,a			;7642	9f 	. 
	sub h			;7643	94 	. 
	ld h,a			;7644	67 	g 
l7645h:
	ex de,hl			;7645	eb 	. 
	xor a			;7646	af 	. 
	ld l,a			;7647	6f 	o 
	ld h,a			;7648	67 	g 
	ld a,e			;7649	7b 	{ 
	ld e,010h		;764a	1e 10 	. . 
	rla			;764c	17 	. 
	rl d		;764d	cb 12 	. . 
l764fh:
	adc hl,hl		;764f	ed 6a 	. j 
	push hl			;7651	e5 	. 
	add hl,bc			;7652	09 	. 
	jr nc,$+5		;7653	30 03 	0 . 
	inc sp			;7655	33 	3 
	inc sp			;7656	33 	3 
	jr nc,l763ah		;7657	30 e1 	0 . 
	rla			;7659	17 	. 
	rl d		;765a	cb 12 	. . 
	dec e			;765c	1d 	. 
	jr nz,l764fh		;765d	20 f0 	  . 
	ld e,a			;765f	5f 	_ 
	ex de,hl			;7660	eb 	. 
	pop af			;7661	f1 	. 
	and a			;7662	a7 	. 
	jp p,l766eh		;7663	f2 6e 76 	. n v 
	xor a			;7666	af 	. 
	sub e			;7667	93 	. 
	ld e,a			;7668	5f 	_ 
	sbc a,a			;7669	9f 	. 
	sub d			;766a	92 	. 
	ld d,a			;766b	57 	W 
	ld a,080h		;766c	3e 80 	> . 
l766eh:
	pop bc			;766e	c1 	. 
	xor b			;766f	a8 	. 
	ret p			;7670	f0 	. 
	xor a			;7671	af 	. 
	sub l			;7672	95 	. 
	ld l,a			;7673	6f 	o 
	sbc a,a			;7674	9f 	. 
	sub h			;7675	94 	. 
	ld h,a			;7676	67 	g 
	ret			;7677	c9 	. 
TOKEN_08_RND:
	ld a,b			;7678	78 	x 
	and a			;7679	a7 	. 
	jr z,l76b3h		;767a	28 37 	( 7 
	bit 7,h		;767c	cb 7c 	. | 
	jr z,l7689h		;767e	28 09 	( . 
	ld (0f7beh),hl		;7680	22 be f7 	" . . 
	ld h,b			;7683	60 	` 
	ld l,035h		;7684	2e 35 	. 5 
	ld (BASIC_SWPTMP),hl		;7686	22 bc f7 	" . . 
l7689h:
	ld hl,(BASIC_SWPTMP)		;7689	2a bc f7 	* . . 
	ld e,l			;768c	5d 	] 
	ld d,h			;768d	54 	T 
	add hl,hl			;768e	29 	) 
	ld c,l			;768f	4d 	M 
	ld b,h			;7690	44 	D 
	jr nc,l7694h		;7691	30 01 	0 . 
	inc bc			;7693	03 	. 
l7694h:
	add hl,de			;7694	19 	. 
	ld (BASIC_SWPTMP),hl		;7695	22 bc f7 	" . . 
	ld hl,(0f7beh)		;7698	2a be f7 	* . . 
	ld e,l			;769b	5d 	] 
	ld d,h			;769c	54 	T 
	adc hl,hl		;769d	ed 6a 	. j 
	add hl,de			;769f	19 	. 
	add hl,bc			;76a0	09 	. 
	ld (0f7beh),hl		;76a1	22 be f7 	" . . 
l76a4h:
	ld a,l			;76a4	7d 	} 
	or h			;76a5	b4 	. 
	ld b,a			;76a6	47 	G 
	ret z			;76a7	c8 	. 
	ld b,080h		;76a8	06 80 	. . 
l76aah:
	bit 7,h		;76aa	cb 7c 	. | 
	res 7,h		;76ac	cb bc 	. . 
	ret nz			;76ae	c0 	. 
	add hl,hl			;76af	29 	) 
	dec b			;76b0	05 	. 
	jr l76aah		;76b1	18 f7 	. . 
l76b3h:
	ld hl,(0f7beh)		;76b3	2a be f7 	* . . 
	jr l76a4h		;76b6	18 ec 	. . 
l76b8h:
	call l762dh		;76b8	cd 2d 76 	. - v 
	ex de,hl			;76bb	eb 	. 
	ret			;76bc	c9 	. 
l76bdh:
	ld a,d			;76bd	7a 	z 
	xor 080h		;76be	ee 80 	. . 
	ld d,a			;76c0	57 	W 
sub_76c1h:
	ld a,c			;76c1	79 	y 
	and a			;76c2	a7 	. 
	ret z			;76c3	c8 	. 
	ld a,b			;76c4	78 	x 
	and a			;76c5	a7 	. 
	jp z,l772fh		;76c6	ca 2f 77 	. / w 
	sub c			;76c9	91 	. 
	jr nc,l76d0h		;76ca	30 04 	0 . 
	neg		;76cc	ed 44 	. D 
	ex de,hl			;76ce	eb 	. 
	ld b,c			;76cf	41 	A 
l76d0h:
	cp 011h		;76d0	fe 11 	. . 
	ret nc			;76d2	d0 	. 
	ld c,a			;76d3	4f 	O 
	ld a,h			;76d4	7c 	| 
	xor d			;76d5	aa 	. 
	add a,a			;76d6	87 	. 
	ld a,h			;76d7	7c 	| 
	push af			;76d8	f5 	. 
	set 7,h		;76d9	cb fc 	. . 
	set 7,d		;76db	cb fa 	. . 
	inc c			;76dd	0c 	. 
	dec c			;76de	0d 	. 
	jr z,l76ebh		;76df	28 0a 	( . 
l76e1h:
	srl d		;76e1	cb 3a 	. : 
	rr e		;76e3	cb 1b 	. . 
	dec c			;76e5	0d 	. 
	jr nz,l76e1h		;76e6	20 f9 	  . 
	jr nc,l76ebh		;76e8	30 01 	0 . 
	inc de			;76ea	13 	. 
l76ebh:
	pop af			;76eb	f1 	. 
	jr c,l7706h		;76ec	38 18 	8 . 
	add hl,de			;76ee	19 	. 
	jr nc,l76f8h		;76ef	30 07 	0 . 
	inc b			;76f1	04 	. 
	jr z,l76ffh		;76f2	28 0b 	( . 
	rr h		;76f4	cb 1c 	. . 
	rr l		;76f6	cb 1d 	. . 
l76f8h:
	res 7,h		;76f8	cb bc 	. . 
l76fah:
	and a			;76fa	a7 	. 
	ret p			;76fb	f0 	. 
	set 7,h		;76fc	cb fc 	. . 
	ret			;76fe	c9 	. 
l76ffh:
	ld b,0ffh		;76ff	06 ff 	. . 
	ld hl,l7fffh		;7701	21 ff 7f 	! .  
	jr l76fah		;7704	18 f4 	. . 
l7706h:
	and a			;7706	a7 	. 
	sbc hl,de		;7707	ed 52 	. R 
	jr nc,l7712h		;7709	30 07 	0 . 
	ex de,hl			;770b	eb 	. 
	ld hl,00001h		;770c	21 01 00 	! . . 
	sbc hl,de		;770f	ed 52 	. R 
	cpl			;7711	2f 	/ 
l7712h:
	ld c,a			;7712	4f 	O 
l7713h:
	ld a,h			;7713	7c 	| 
	and a			;7714	a7 	. 
	jr nz,l7726h		;7715	20 0f 	  . 
	or l			;7717	b5 	. 
	jr z,l7723h		;7718	28 09 	( . 
	ld h,l			;771a	65 	e 
	ld l,000h		;771b	2e 00 	. . 
	ld a,b			;771d	78 	x 
	sub 008h		;771e	d6 08 	. . 
	ld b,a			;7720	47 	G 
	jr nc,l7726h		;7721	30 03 	0 . 
l7723h:
	ld b,000h		;7723	06 00 	. . 
	ret			;7725	c9 	. 
l7726h:
	ld a,c			;7726	79 	y 
l7727h:
	bit 7,h		;7727	cb 7c 	. | 
	jr nz,l76f8h		;7729	20 cd 	  . 
	add hl,hl			;772b	29 	) 
	djnz l7727h		;772c	10 f9 	. . 
	ret			;772e	c9 	. 
l772fh:
	ld b,c			;772f	41 	A 
	ex de,hl			;7730	eb 	. 
	ret			;7731	c9 	. 
l7732h:
	ld a,b			;7732	78 	x 
	and a			;7733	a7 	. 
	ret z			;7734	c8 	. 
	ld a,c			;7735	79 	y 
	and a			;7736	a7 	. 
	jr z,l7723h		;7737	28 ea 	( . 
	add a,b			;7739	80 	. 
	ld b,a			;773a	47 	G 
	rra			;773b	1f 	. 
	xor b			;773c	a8 	. 
	ld a,b			;773d	78 	x 
	jp p,l776ch		;773e	f2 6c 77 	. l w 
	add a,080h		;7741	c6 80 	. . 
	ld b,a			;7743	47 	G 
	ret z			;7744	c8 	. 
	ld a,h			;7745	7c 	| 
	xor d			;7746	aa 	. 
	ld c,a			;7747	4f 	O 
	push bc			;7748	c5 	. 
	set 7,h		;7749	cb fc 	. . 
	set 7,d		;774b	cb fa 	. . 
	ld c,l			;774d	4d 	M 
	ld a,h			;774e	7c 	| 
	ld hl,00000h		;774f	21 00 00 	! . . 
	ld b,010h		;7752	06 10 	. . 
	rra			;7754	1f 	. 
	rr c		;7755	cb 19 	. . 
l7757h:
	jr nc,l775ah		;7757	30 01 	0 . 
	add hl,de			;7759	19 	. 
l775ah:
	rr h		;775a	cb 1c 	. . 
	rr l		;775c	cb 1d 	. . 
	rra			;775e	1f 	. 
	rr c		;775f	cb 19 	. . 
	djnz l7757h		;7761	10 f4 	. . 
	pop bc			;7763	c1 	. 
	add a,a			;7764	87 	. 
	jp nc,l7726h		;7765	d2 26 77 	. & w 
	inc hl			;7768	23 	# 
	jp l7726h		;7769	c3 26 77 	. & w 
l776ch:
	and a			;776c	a7 	. 
	jp p,l7723h		;776d	f2 23 77 	. # w 
	ld a,h			;7770	7c 	| 
	xor d			;7771	aa 	. 
	jp l76ffh		;7772	c3 ff 76 	. . v 
l7775h:
	ld a,c			;7775	79 	y 
	and a			;7776	a7 	. 
	ld a,h			;7777	7c 	| 
	jp z,l76ffh		;7778	ca ff 76 	. . v 
	ld a,b			;777b	78 	x 
	and a			;777c	a7 	. 
	ret z			;777d	c8 	. 
	sub c			;777e	91 	. 
	ld b,a			;777f	47 	G 
	rra			;7780	1f 	. 
	xor b			;7781	a8 	. 
	ld a,b			;7782	78 	x 
	jp m,l776ch		;7783	fa 6c 77 	. l w 
	add a,080h		;7786	c6 80 	. . 
	ld b,a			;7788	47 	G 
	ret z			;7789	c8 	. 
	ld a,h			;778a	7c 	| 
	xor d			;778b	aa 	. 
	inc b			;778c	04 	. 
	jp z,l76ffh		;778d	ca ff 76 	. . v 
	ld c,a			;7790	4f 	O 
	push bc			;7791	c5 	. 
	set 7,h		;7792	cb fc 	. . 
	set 7,d		;7794	cb fa 	. . 
	xor a			;7796	af 	. 
	ld c,a			;7797	4f 	O 
	ld b,010h		;7798	06 10 	. . 
	jr l77a0h		;779a	18 04 	. . 
l779ch:
	adc hl,hl		;779c	ed 6a 	. j 
	jr c,l77bch		;779e	38 1c 	8 . 
l77a0h:
	sbc hl,de		;77a0	ed 52 	. R 
	jr nc,l77a5h		;77a2	30 01 	0 . 
	add hl,de			;77a4	19 	. 
l77a5h:
	ccf			;77a5	3f 	? 
l77a6h:
	rl c		;77a6	cb 11 	. . 
	rla			;77a8	17 	. 
	djnz l779ch		;77a9	10 f1 	. . 
	srl d		;77ab	cb 3a 	. : 
	rr e		;77ad	cb 1b 	. . 
	and a			;77af	a7 	. 
	sbc hl,de		;77b0	ed 52 	. R 
	ld l,c			;77b2	69 	i 
	ld h,a			;77b3	67 	g 
	pop bc			;77b4	c1 	. 
	jp c,l7713h		;77b5	da 13 77 	. . w 
	inc hl			;77b8	23 	# 
	jp l7713h		;77b9	c3 13 77 	. . w 
l77bch:
	or a			;77bc	b7 	. 
	sbc hl,de		;77bd	ed 52 	. R 
	jr l77a6h		;77bf	18 e5 	. . 
sub_77c1h:
	ld a,e			;77c1	7b 	{ 
	or d			;77c2	b2 	. 
	jr z,l7807h		;77c3	28 42 	( B 
	bit 7,d		;77c5	cb 7a 	. z 
	push af			;77c7	f5 	. 
	jp p,l77d1h		;77c8	f2 d1 77 	. . w 
	xor a			;77cb	af 	. 
	sub e			;77cc	93 	. 
	ld e,a			;77cd	5f 	_ 
	sbc a,a			;77ce	9f 	. 
	sub d			;77cf	92 	. 
	ld d,a			;77d0	57 	W 
l77d1h:
	push de			;77d1	d5 	. 
	ld c,b			;77d2	48 	H 
	ex de,hl			;77d3	eb 	. 
	call l7807h		;77d4	cd 07 78 	. . x 
l77d7h:
	ex (sp),hl			;77d7	e3 	. 
	srl h		;77d8	cb 3c 	. < 
	rr l		;77da	cb 1d 	. . 
	ex (sp),hl			;77dc	e3 	. 
	jr nc,l77e7h		;77dd	30 08 	0 . 
	push de			;77df	d5 	. 
	push bc			;77e0	c5 	. 
	call l7732h		;77e1	cd 32 77 	. 2 w 
	pop de			;77e4	d1 	. 
	ld c,e			;77e5	4b 	K 
	pop de			;77e6	d1 	. 
l77e7h:
	ex (sp),hl			;77e7	e3 	. 
	ld a,l			;77e8	7d 	} 
	or h			;77e9	b4 	. 
	ex (sp),hl			;77ea	e3 	. 
	jr z,l77fch		;77eb	28 0f 	( . 
	push hl			;77ed	e5 	. 
	push bc			;77ee	c5 	. 
	ld b,c			;77ef	41 	A 
	ld h,d			;77f0	62 	b 
	ld l,e			;77f1	6b 	k 
	call l7732h		;77f2	cd 32 77 	. 2 w 
	ld c,b			;77f5	48 	H 
	ex de,hl			;77f6	eb 	. 
	pop hl			;77f7	e1 	. 
	ld b,h			;77f8	44 	D 
	pop hl			;77f9	e1 	. 
	jr l77d7h		;77fa	18 db 	. . 
l77fch:
	pop af			;77fc	f1 	. 
	pop af			;77fd	f1 	. 
	ret p			;77fe	f0 	. 
sub_77ffh:
	ld c,b			;77ff	48 	H 
	ex de,hl			;7800	eb 	. 
	call l7807h		;7801	cd 07 78 	. . x 
	jp l7775h		;7804	c3 75 77 	. u w 
l7807h:
	ld b,081h		;7807	06 81 	. . 
	ld hl,00000h		;7809	21 00 00 	! . . 
	ret			;780c	c9 	. 
l780dh:
	push de			;780d	d5 	. 
	push bc			;780e	c5 	. 
	call TOKEN_0A_LOG		;780f	cd 53 7a 	. S z 
	pop de			;7812	d1 	. 
	ld c,e			;7813	4b 	K 
	pop de			;7814	d1 	. 
	call l7732h		;7815	cd 32 77 	. 2 w 
	jp TOKEN_0B_EXP		;7818	c3 fa 79 	. . y 
sub_781bh:
	bit 7,h		;781b	cb 7c 	. | 
	push af			;781d	f5 	. 
	res 7,h		;781e	cb bc 	. . 
	call TOKEN_MULTI_FUNCTION		;7820	cd 2d 78 	. - x 
	pop af			;7823	f1 	. 
	ret z			;7824	c8 	. 
	ld c,090h		;7825	0e 90 	. . 
	ld de,00000h		;7827	11 00 00 	. . . 
	jp sub_76c1h		;782a	c3 c1 76 	. . v 
TOKEN_MULTI_FUNCTION:
	ld a,l			;782d	7d 	} 
	or h			;782e	b4 	. 
	jr z,l784dh		;782f	28 1c 	( . 
	bit 7,h		;7831	cb 7c 	. | 
	push af			;7833	f5 	. 
	jr z,l783ch		;7834	28 06 	( . 
	xor a			;7836	af 	. 
	sub l			;7837	95 	. 
	ld l,a			;7838	6f 	o 
	sbc a,a			;7839	9f 	. 
	sub h			;783a	94 	. 
	ld h,a			;783b	67 	g 
l783ch:
	ld b,090h		;783c	06 90 	. . 
l783eh:
	bit 7,h		;783e	cb 7c 	. | 
	jr nz,l7846h		;7840	20 04 	  . 
	add hl,hl			;7842	29 	) 
	dec b			;7843	05 	. 
	jr l783eh		;7844	18 f8 	. . 
l7846h:
	res 7,h		;7846	cb bc 	. . 
	pop af			;7848	f1 	. 
	ret z			;7849	c8 	. 
	set 7,h		;784a	cb fc 	. . 
	ret			;784c	c9 	. 
l784dh:
	ld b,a			;784d	47 	G 
	ret			;784e	c9 	. 
sub_784fh:
	ld a,b			;784f	78 	x 
	and a			;7850	a7 	. 
	jr z,l7872h		;7851	28 1f 	( . 
	dec b			;7853	05 	. 
	jp p,l7872h		;7854	f2 72 78 	. r x 
	bit 7,h		;7857	cb 7c 	. | 
	push af			;7859	f5 	. 
	set 7,h		;785a	cb fc 	. . 
	ld de,00000h		;785c	11 00 00 	. . . 
l785fh:
	add hl,hl			;785f	29 	) 
	rl e		;7860	cb 13 	. . 
	rl d		;7862	cb 12 	. . 
	dec b			;7864	05 	. 
	jp m,l785fh		;7865	fa 5f 78 	. _ x 
	ex de,hl			;7868	eb 	. 
	pop af			;7869	f1 	. 
	ret z			;786a	c8 	. 
	xor a			;786b	af 	. 
	sub l			;786c	95 	. 
	ld l,a			;786d	6f 	o 
	sbc a,a			;786e	9f 	. 
	sub h			;786f	94 	. 
	ld h,a			;7870	67 	g 
	ret			;7871	c9 	. 
l7872h:
	ld hl,00000h		;7872	21 00 00 	! . . 
	ret			;7875	c9 	. 
	call sub_78b2h		;7876	cd b2 78 	. . x 
	ld hl,00000h		;7879	21 00 00 	! . . 
	ret nz			;787c	c0 	. 
	dec hl			;787d	2b 	+ 
	ret			;787e	c9 	. 
	call sub_78b2h		;787f	cd b2 78 	. . x 
	ld hl,00000h		;7882	21 00 00 	! . . 
	ret z			;7885	c8 	. 
	dec hl			;7886	2b 	+ 
	ret			;7887	c9 	. 
	call sub_78b2h		;7888	cd b2 78 	. . x 
	ld hl,00000h		;788b	21 00 00 	! . . 
	ret c			;788e	d8 	. 
	ret z			;788f	c8 	. 
	dec hl			;7890	2b 	+ 
	ret			;7891	c9 	. 
	call sub_78b2h		;7892	cd b2 78 	. . x 
	ld hl,00000h		;7895	21 00 00 	! . . 
	ret c			;7898	d8 	. 
	dec hl			;7899	2b 	+ 
	ret			;789a	c9 	. 
	call sub_78b2h		;789b	cd b2 78 	. . x 
	ld hl,00000h		;789e	21 00 00 	! . . 
	ret nc			;78a1	d0 	. 
	dec hl			;78a2	2b 	+ 
	ret			;78a3	c9 	. 
	call sub_78b2h		;78a4	cd b2 78 	. . x 
	ld hl,0ffffh		;78a7	21 ff ff 	! . . 
	ret c			;78aa	d8 	. 
	ret z			;78ab	c8 	. 
	inc hl			;78ac	23 	# 
	ret			;78ad	c9 	. 
	ld hl,0ffffh		;78ae	21 ff ff 	! . . 
	ret			;78b1	c9 	. 
sub_78b2h:
	ld a,c			;78b2	79 	y 
	and a			;78b3	a7 	. 
	jr z,l78d2h		;78b4	28 1c 	( . 
	ld a,b			;78b6	78 	x 
	and a			;78b7	a7 	. 
	ld a,d			;78b8	7a 	z 
	jr z,l78cdh		;78b9	28 12 	( . 
	xor h			;78bb	ac 	. 
	ld a,d			;78bc	7a 	z 
	jp m,l78cdh		;78bd	fa cd 78 	. . x 
	ld a,c			;78c0	79 	y 
	cp b			;78c1	b8 	. 
	jr nz,l78cbh		;78c2	20 07 	  . 
	ld a,d			;78c4	7a 	z 
	cp h			;78c5	bc 	. 
	jr nz,l78cbh		;78c6	20 03 	  . 
	ld a,e			;78c8	7b 	{ 
	sub l			;78c9	95 	. 
	ret z			;78ca	c8 	. 
l78cbh:
	rra			;78cb	1f 	. 
	xor d			;78cc	aa 	. 
l78cdh:
	cpl			;78cd	2f 	/ 
	or 001h		;78ce	f6 01 	. . 
	rla			;78d0	17 	. 
	ret			;78d1	c9 	. 
l78d2h:
	ld a,b			;78d2	78 	x 
	and a			;78d3	a7 	. 
	ret z			;78d4	c8 	. 
	ld a,h			;78d5	7c 	| 
	rla			;78d6	17 	. 
	ret			;78d7	c9 	. 
sub_78d8h:
	bit 7,h		;78d8	cb 7c 	. | 
	push af			;78da	f5 	. 
	res 7,h		;78db	cb bc 	. . 
	call sub_78e5h		;78dd	cd e5 78 	. . x 
	pop af			;78e0	f1 	. 
	ret z			;78e1	c8 	. 
	set 7,h		;78e2	cb fc 	. . 
	ret			;78e4	c9 	. 
sub_78e5h:
	ld a,b			;78e5	78 	x 
	and a			;78e6	a7 	. 
	ret z			;78e7	c8 	. 
	cp 091h		;78e8	fe 91 	. . 
	ret nc			;78ea	d0 	. 
	sub 081h		;78eb	d6 81 	. . 
	jr c,l7923h		;78ed	38 34 	8 4 
	ld de,parse_code_end		;78ef	11 00 80 	. . . 
	jr z,l78fbh		;78f2	28 07 	( . 
l78f4h:
	sra d		;78f4	cb 2a 	. * 
	rr e		;78f6	cb 1b 	. . 
	dec a			;78f8	3d 	= 
	jr nz,l78f4h		;78f9	20 f9 	  . 
l78fbh:
	bit 7,h		;78fb	cb 7c 	. | 
	jr nz,l7906h		;78fd	20 07 	  . 
	ld a,h			;78ff	7c 	| 
	and d			;7900	a2 	. 
	ld h,a			;7901	67 	g 
	ld a,l			;7902	7d 	} 
	and e			;7903	a3 	. 
	ld l,a			;7904	6f 	o 
	ret			;7905	c9 	. 
l7906h:
	push hl			;7906	e5 	. 
	ld a,h			;7907	7c 	| 
	and d			;7908	a2 	. 
	ld h,a			;7909	67 	g 
	ld a,l			;790a	7d 	} 
	and e			;790b	a3 	. 
	ld l,a			;790c	6f 	o 
	ex (sp),hl			;790d	e3 	. 
	ld a,d			;790e	7a 	z 
	cpl			;790f	2f 	/ 
	and h			;7910	a4 	. 
	jr nz,l791ah		;7911	20 07 	  . 
	ld a,e			;7913	7b 	{ 
	cpl			;7914	2f 	/ 
	and l			;7915	a5 	. 
	jr nz,l791ah		;7916	20 02 	  . 
	pop hl			;7918	e1 	. 
	ret			;7919	c9 	. 
l791ah:
	pop hl			;791a	e1 	. 
	ld c,081h		;791b	0e 81 	. . 
	ld de,parse_code_end		;791d	11 00 80 	. . . 
	jp sub_76c1h		;7920	c3 c1 76 	. . v 
l7923h:
	bit 7,h		;7923	cb 7c 	. | 
	ld b,000h		;7925	06 00 	. . 
	ret z			;7927	c8 	. 
	ld b,081h		;7928	06 81 	. . 
	ld hl,parse_code_end		;792a	21 00 80 	! . . 
	ret			;792d	c9 	. 
TOKEN_0C_COS:
	ld c,081h		;792e	0e 81 	. . 
	ld de,l4910h		;7930	11 10 49 	. . I 
	call sub_76c1h		;7933	cd c1 76 	. . v 
TOKEN_09_SIN:
	ld a,b			;7936	78 	x 
	cp 077h		;7937	fe 77 	. w 
	ret c			;7939	d8 	. 
	bit 7,h		;793a	cb 7c 	. | 
	jr z,l7948h		;793c	28 0a 	( . 
	res 7,h		;793e	cb bc 	. . 
	call l7948h		;7940	cd 48 79 	. H y 
	ld a,h			;7943	7c 	| 
	xor 080h		;7944	ee 80 	. . 
	ld h,a			;7946	67 	g 
	ret			;7947	c9 	. 
l7948h:
	ld c,07eh		;7948	0e 7e 	. ~ 
	ld de,022fah		;794a	11 fa 22 	. . " 
	call l7732h		;794d	cd 32 77 	. 2 w 
	push hl			;7950	e5 	. 
	push bc			;7951	c5 	. 
	call sub_78e5h		;7952	cd e5 78 	. . x 
	ld c,b			;7955	48 	H 
	ex de,hl			;7956	eb 	. 
	pop af			;7957	f1 	. 
	ld b,a			;7958	47 	G 
	pop hl			;7959	e1 	. 
	call l76bdh		;795a	cd bd 76 	. . v 
	ld c,07fh		;795d	0e 7f 	.  
	ld de,00000h		;795f	11 00 00 	. . . 
	call sub_78b2h		;7962	cd b2 78 	. . x 
	jr c,l797dh		;7965	38 16 	8 . 
	ld c,080h		;7967	0e 80 	. . 
	ld de,0c000h		;7969	11 00 c0 	. . . 
	call sub_76c1h		;796c	cd c1 76 	. . v 
	set 7,h		;796f	cb fc 	. . 
	ld c,07fh		;7971	0e 7f 	.  
	ld de,00000h		;7973	11 00 00 	. . . 
	call sub_76c1h		;7976	cd c1 76 	. . v 
	ld a,h			;7979	7c 	| 
	xor 080h		;797a	ee 80 	. . 
	ld h,a			;797c	67 	g 
l797dh:
	bit 7,h		;797d	cb 7c 	. | 
	push af			;797f	f5 	. 
	res 7,h		;7980	cb bc 	. . 
	ld ix,l79a2h		;7982	dd 21 a2 79 	. ! . y 
	call sub_7addh		;7986	cd dd 7a 	. . z 
	pop af			;7989	f1 	. 
	ret z			;798a	c8 	. 
	ld a,h			;798b	7c 	| 
	xor 080h		;798c	ee 80 	. . 
	ld h,a			;798e	67 	g 
	ret			;798f	c9 	. 
TOKEN_0D_TAN:
	push hl			;7990	e5 	. 
	push bc			;7991	c5 	. 
	call TOKEN_0C_COS		;7992	cd 2e 79 	. . y 
	ld a,b			;7995	78 	x 
	pop bc			;7996	c1 	. 
	ex (sp),hl			;7997	e3 	. 
	push af			;7998	f5 	. 
	call TOKEN_09_SIN		;7999	cd 36 79 	. 6 y 
	pop af			;799c	f1 	. 
	ld c,a			;799d	4f 	O 
	pop de			;799e	d1 	. 
	jp l7775h		;799f	c3 75 77 	. u w 
l79a2h:
	dec b			;79a2	05 	. 
	add a,(hl)			;79a3	86 	. 
	ld e,0d8h		;79a4	1e d8 	. . 
	add a,a			;79a6	87 	. 
	sbc a,c			;79a7	99 	. 
	ld h,087h		;79a8	26 87 	& . 
	inc hl			;79aa	23 	# 
	inc (hl)			;79ab	34 	4 
	add a,(hl)			;79ac	86 	. 
	and l			;79ad	a5 	. 
	ld e,(hl)			;79ae	5e 	^ 
	add a,e			;79af	83 	. 
	ld c,c			;79b0	49 	I 
	djnz $-51		;79b1	10 cb 	. . 
	ld a,h			;79b3	7c 	| 
	jr z,l79c0h		;79b4	28 0a 	( . 
	res 7,h		;79b6	cb bc 	. . 
	call l79c0h		;79b8	cd c0 79 	. . y 
	ld a,h			;79bb	7c 	| 
	xor 080h		;79bc	ee 80 	. . 
	ld h,a			;79be	67 	g 
	ret			;79bf	c9 	. 
l79c0h:
	ld a,b			;79c0	78 	x 
	cp 081h		;79c1	fe 81 	. . 
	jr c,l79d7h		;79c3	38 12 	8 . 
	call sub_77ffh		;79c5	cd ff 77 	. . w 
	call l79d7h		;79c8	cd d7 79 	. . y 
	ld a,h			;79cb	7c 	| 
	xor 080h		;79cc	ee 80 	. . 
	ld h,a			;79ce	67 	g 
	ld c,081h		;79cf	0e 81 	. . 
	ld de,l4910h		;79d1	11 10 49 	. . I 
	jp sub_76c1h		;79d4	c3 c1 76 	. . v 
l79d7h:
	ld ix,l79deh		;79d7	dd 21 de 79 	. ! . y 
	jp sub_7addh		;79db	c3 dd 7a 	. . z 
l79deh:
	add hl,bc			;79de	09 	. 
	ld a,b			;79df	78 	x 
	dec sp			;79e0	3b 	; 
	rst 10h			;79e1	d7 	. 
	ld a,e			;79e2	7b 	{ 
	add a,h			;79e3	84 	. 
	ld l,(hl)			;79e4	6e 	n 
	ld a,h			;79e5	7c 	| 
	cpl			;79e6	2f 	/ 
	jp nz,09a7dh		;79e7	c2 7d 9a 	. } . 
	ld sp,sub_5a7ch+1		;79ea	31 7d 5a 	1 } Z 
	ld a,07eh		;79ed	3e 7e 	> ~ 
	sub c			;79ef	91 	. 
	add a,b			;79f0	80 	. 
	ld a,(hl)			;79f1	7e 	~ 
	ld c,h			;79f2	4c 	L 
	cp h			;79f3	bc 	. 
	ld a,a			;79f4	7f 	 
	xor d			;79f5	aa 	. 
	xor d			;79f6	aa 	. 
	add a,c			;79f7	81 	. 
	nop			;79f8	00 	. 
	nop			;79f9	00 	. 
TOKEN_0B_EXP:
	ld c,081h		;79fa	0e 81 	. . 
l79fch:
	ld de,038aah		;79fc	11 aa 38 	. . 8 
	call l7732h		;79ff	cd 32 77 	. 2 w 
	ld a,b			;7a02	78 	x 
	cp 088h		;7a03	fe 88 	. . 
	jr nc,l7a33h		;7a05	30 2c 	0 , 
	cp 068h		;7a07	fe 68 	. h 
	jp c,l7807h		;7a09	da 07 78 	. . x 
	push hl			;7a0c	e5 	. 
	push bc			;7a0d	c5 	. 
	call sub_78e5h		;7a0e	cd e5 78 	. . x 
	push hl			;7a11	e5 	. 
	push bc			;7a12	c5 	. 
	call sub_784fh		;7a13	cd 4f 78 	. O x 
	ld a,l			;7a16	7d 	} 
	add a,081h		;7a17	c6 81 	. . 
	pop hl			;7a19	e1 	. 
	pop de			;7a1a	d1 	. 
	pop bc			;7a1b	c1 	. 
	ld c,h			;7a1c	4c 	L 
	pop hl			;7a1d	e1 	. 
	jr z,l7a33h		;7a1e	28 13 	( . 
	push af			;7a20	f5 	. 
	call l76bdh		;7a21	cd bd 76 	. . v 
	ld ix,l7a3dh		;7a24	dd 21 3d 7a 	. ! = z 
	call sub_7aeeh		;7a28	cd ee 7a 	. . z 
	pop af			;7a2b	f1 	. 
	ld c,a			;7a2c	4f 	O 
	ld de,00000h		;7a2d	11 00 00 	. . . 
	jp l7732h		;7a30	c3 32 77 	. 2 w 
l7a33h:
	bit 7,h		;7a33	cb 7c 	. | 
	ld b,000h		;7a35	06 00 	. . 
	ret nz			;7a37	c0 	. 
	dec b			;7a38	05 	. 
	ld hl,l7fffh		;7a39	21 ff 7f 	! .  
	ret			;7a3c	c9 	. 
l7a3dh:
	rlca			;7a3d	07 	. 
	ld (hl),h			;7a3e	74 	t 
	ld e,c			;7a3f	59 	Y 
	adc a,b			;7a40	88 	. 
	ld (hl),a			;7a41	77 	w 
	ld h,098h		;7a42	26 98 	& . 
	ld a,d			;7a44	7a 	z 
	ld e,01eh		;7a45	1e 1e 	. . 
	ld a,h			;7a47	7c 	| 
	ld h,e			;7a48	63 	c 
	ld d,b			;7a49	50 	P 
	ld a,(hl)			;7a4a	7e 	~ 
	ld (hl),l			;7a4b	75 	u 
	cp 080h		;7a4c	fe 80 	. . 
	ld sp,08172h		;7a4e	31 72 81 	1 r . 
	nop			;7a51	00 	. 
	nop			;7a52	00 	. 
TOKEN_0A_LOG:
	bit 7,h		;7a53	cb 7c 	. | 
	jr nz,l7a9ah		;7a55	20 43 	  C 
	ld a,b			;7a57	78 	x 
	and a			;7a58	a7 	. 
	jr z,l7a9ah		;7a59	28 3f 	( ? 
	ld c,b			;7a5b	48 	H 
	ld d,h			;7a5c	54 	T 
	ld e,l			;7a5d	5d 	] 
	ld a,080h		;7a5e	3e 80 	> . 
	ld b,a			;7a60	47 	G 
	xor c			;7a61	a9 	. 
	push af			;7a62	f5 	. 
	push hl			;7a63	e5 	. 
	push bc			;7a64	c5 	. 
	ld ix,l7a9bh		;7a65	dd 21 9b 7a 	. ! . z 
	call sub_7aeeh		;7a69	cd ee 7a 	. . z 
	pop af			;7a6c	f1 	. 
	pop de			;7a6d	d1 	. 
	push hl			;7a6e	e5 	. 
	push bc			;7a6f	c5 	. 
	ld b,a			;7a70	47 	G 
	ex de,hl			;7a71	eb 	. 
	ld ix,l7aa8h		;7a72	dd 21 a8 7a 	. ! . z 
	call sub_7aeeh		;7a76	cd ee 7a 	. . z 
	ld c,b			;7a79	48 	H 
	ex de,hl			;7a7a	eb 	. 
	pop af			;7a7b	f1 	. 
	ld b,a			;7a7c	47 	G 
	pop hl			;7a7d	e1 	. 
	call l7775h		;7a7e	cd 75 77 	. u w 
	pop af			;7a81	f1 	. 
	push hl			;7a82	e5 	. 
	push bc			;7a83	c5 	. 
	rlca			;7a84	07 	. 
	rrca			;7a85	0f 	. 
	ld l,a			;7a86	6f 	o 
	sbc a,a			;7a87	9f 	. 
	ld h,a			;7a88	67 	g 
	call TOKEN_MULTI_FUNCTION		;7a89	cd 2d 78 	. - x 
	pop af			;7a8c	f1 	. 
	ld c,a			;7a8d	4f 	O 
	pop de			;7a8e	d1 	. 
	call sub_76c1h		;7a8f	cd c1 76 	. . v 
	ld c,080h		;7a92	0e 80 	. . 
	ld de,MATH_IADD		;7a94	11 72 31 	. r 1 
	jp l7732h		;7a97	c3 32 77 	. 2 w 
l7a9ah:
	ret			;7a9a	c9 	. 
l7a9bh:
	inc b			;7a9b	04 	. 
	add a,e			;7a9c	83 	. 
	add hl,de			;7a9d	19 	. 
	ret m			;7a9e	f8 	. 
	add a,e			;7a9f	83 	. 
	ld b,e			;7aa0	43 	C 
	ld h,e			;7aa1	63 	c 
	add a,h			;7aa2	84 	. 
	adc a,l			;7aa3	8d 	. 
	call 08382h		;7aa4	cd 82 83 	. . . 
	add a,b			;7aa7	80 	. 
l7aa8h:
	inc b			;7aa8	04 	. 
	add a,c			;7aa9	81 	. 
	nop			;7aaa	00 	. 
	nop			;7aab	00 	. 
	add a,e			;7aac	83 	. 
	ld c,l			;7aad	4d 	M 
	or c			;7aae	b1 	. 
	add a,e			;7aaf	83 	. 
	ld de,l7f72h		;7ab0	11 72 7f 	. r  
	dec (hl)			;7ab3	35 	5 
	dec b			;7ab4	05 	. 
TOKEN_07_SQR:
	ld a,b			;7ab5	78 	x 
	and a			;7ab6	a7 	. 
	ret z			;7ab7	c8 	. 
	res 7,h		;7ab8	cb bc 	. . 
	rra			;7aba	1f 	. 
	adc a,040h		;7abb	ce 40 	. @ 
	ld c,a			;7abd	4f 	O 
	ld d,h			;7abe	54 	T 
	ld e,l			;7abf	5d 	] 
	ld a,003h		;7ac0	3e 03 	> . 
l7ac2h:
	push af			;7ac2	f5 	. 
	push hl			;7ac3	e5 	. 
	push bc			;7ac4	c5 	. 
	push de			;7ac5	d5 	. 
	push bc			;7ac6	c5 	. 
	call l7775h		;7ac7	cd 75 77 	. u w 
	pop de			;7aca	d1 	. 
	ld c,e			;7acb	4b 	K 
	pop de			;7acc	d1 	. 
	call sub_76c1h		;7acd	cd c1 76 	. . v 
	dec b			;7ad0	05 	. 
	ld c,b			;7ad1	48 	H 
	ex de,hl			;7ad2	eb 	. 
	pop af			;7ad3	f1 	. 
	ld b,a			;7ad4	47 	G 
	pop hl			;7ad5	e1 	. 
	pop af			;7ad6	f1 	. 
	dec a			;7ad7	3d 	= 
	jr nz,l7ac2h		;7ad8	20 e8 	  . 
	ld b,c			;7ada	41 	A 
	ex de,hl			;7adb	eb 	. 
	ret			;7adc	c9 	. 
sub_7addh:
	push hl			;7add	e5 	. 
	push bc			;7ade	c5 	. 
	ld c,b			;7adf	48 	H 
	ld d,h			;7ae0	54 	T 
	ld e,l			;7ae1	5d 	] 
	call l7732h		;7ae2	cd 32 77 	. 2 w 
	call sub_7aeeh		;7ae5	cd ee 7a 	. . z 
	pop af			;7ae8	f1 	. 
	ld c,a			;7ae9	4f 	O 
	pop de			;7aea	d1 	. 
	jp l7732h		;7aeb	c3 32 77 	. 2 w 
sub_7aeeh:
	ld a,(ix+000h)		;7aee	dd 7e 00 	. ~ . 
	inc ix		;7af1	dd 23 	. # 
	push af			;7af3	f5 	. 
	push hl			;7af4	e5 	. 
	push bc			;7af5	c5 	. 
	ld b,(ix+000h)		;7af6	dd 46 00 	. F . 
	ld h,(ix+001h)		;7af9	dd 66 01 	. f . 
	ld l,(ix+002h)		;7afc	dd 6e 02 	. n . 
	inc ix		;7aff	dd 23 	. # 
	inc ix		;7b01	dd 23 	. # 
	inc ix		;7b03	dd 23 	. # 
l7b05h:
	pop af			;7b05	f1 	. 
	ld c,a			;7b06	4f 	O 
	pop de			;7b07	d1 	. 
	pop af			;7b08	f1 	. 
	dec a			;7b09	3d 	= 
	ret z			;7b0a	c8 	. 
	push af			;7b0b	f5 	. 
	push de			;7b0c	d5 	. 
	ld a,c			;7b0d	79 	y 
	push af			;7b0e	f5 	. 
	call l7732h		;7b0f	cd 32 77 	. 2 w 
	ld c,(ix+000h)		;7b12	dd 4e 00 	. N . 
	ld d,(ix+001h)		;7b15	dd 56 01 	. V . 
	ld e,(ix+002h)		;7b18	dd 5e 02 	. ^ . 
	call sub_76c1h		;7b1b	cd c1 76 	. . v 
	inc ix		;7b1e	dd 23 	. # 
	inc ix		;7b20	dd 23 	. # 
	inc ix		;7b22	dd 23 	. # 
	jr l7b05h		;7b24	18 df 	. . 
AUX_INT_TO_STR:
	ld ix,0f7c6h		;7b26	dd 21 c6 f7 	. ! . . 
	bit 7,h		;7b2a	cb 7c 	. | 
	ld a,020h		;7b2c	3e 20 	>   
	jr z,l7b3bh		;7b2e	28 0b 	( . 
	xor a			;7b30	af 	. 
	sub l			;7b31	95 	. 
	ld l,a			;7b32	6f 	o 
	sbc a,a			;7b33	9f 	. 
	sub h			;7b34	94 	. 
	ld h,a			;7b35	67 	g 
	jp m,l7b4eh		;7b36	fa 4e 7b 	. N { 
	ld a,02dh		;7b39	3e 2d 	> - 
l7b3bh:
	ld (ix+000h),a		;7b3b	dd 77 00 	. w . 
	inc ix		;7b3e	dd 23 	. # 
	call l7b62h+1		;7b40	cd 63 7b 	. c { 
l7b43h:
	push ix		;7b43	dd e5 	. . 
	pop hl			;7b45	e1 	. 
	ld a,l			;7b46	7d 	} 
	sub 0c6h		;7b47	d6 c6 	. . 
	ld hl,BASIC_STRBUF		;7b49	21 c5 f7 	! . . 
	ld (hl),a			;7b4c	77 	w 
	ret			;7b4d	c9 	. 
l7b4eh:
	ld de,0f7cbh		;7b4e	11 cb f7 	. . . 
	ld hl,l7b62h		;7b51	21 62 7b 	! b { 
	ld bc,00007h		;7b54	01 07 00 	. . . 
	lddr		;7b57	ed b8 	. . 
	inc de			;7b59	13 	. 
	ex de,hl			;7b5a	eb 	. 
	ret			;7b5b	c9 	. 
	ld b,02dh		;7b5c	06 2d 	. - 
	inc sp			;7b5e	33 	3 
	ld (03637h),a		;7b5f	32 37 36 	2 7 6 
l7b62h:
	jr c,$+38		;7b62	38 24 	8 $ 
	dec h			;7b64	25 	% 
	jr nz,l7b6ch		;7b65	20 05 	  . 
	ld a,l			;7b67	7d 	} 
	cp 00ah		;7b68	fe 0a 	. . 
	jr c,l7b77h		;7b6a	38 0b 	8 . 
l7b6ch:
	ld de,0000ah		;7b6c	11 0a 00 	. . . 
	call l762dh		;7b6f	cd 2d 76 	. - v 
	push de			;7b72	d5 	. 
	call l7b62h+1		;7b73	cd 63 7b 	. c { 
	pop hl			;7b76	e1 	. 
l7b77h:
	ld a,l			;7b77	7d 	} 
	add a,030h		;7b78	c6 30 	. 0 
	ld (ix+000h),a		;7b7a	dd 77 00 	. w . 
	inc ix		;7b7d	dd 23 	. # 
	ret			;7b7f	c9 	. 
AUX_FLOAT_TO_STR:
	ld ix,0f7c6h		;7b80	dd 21 c6 f7 	. ! . . 
	call sub_7b97h		;7b84	cd 97 7b 	. . { 
	jp l7b43h		;7b87	c3 43 7b 	. C { 
l7b8ah:
	ld (ix+000h),020h		;7b8a	dd 36 00 20 	. 6 .   
	inc ix		;7b8e	dd 23 	. # 
	ld (ix+000h),030h		;7b90	dd 36 00 30 	. 6 . 0 
	inc ix		;7b94	dd 23 	. # 
	ret			;7b96	c9 	. 
sub_7b97h:
	ld a,b			;7b97	78 	x 
	and a			;7b98	a7 	. 
	jr z,l7b8ah		;7b99	28 ef 	( . 
	bit 7,h		;7b9b	cb 7c 	. | 
	ld a,020h		;7b9d	3e 20 	>   
	jr z,l7ba5h		;7b9f	28 04 	( . 
	res 7,h		;7ba1	cb bc 	. . 
	ld a,02dh		;7ba3	3e 2d 	> - 
l7ba5h:
	ld (ix+000h),a		;7ba5	dd 77 00 	. w . 
	inc ix		;7ba8	dd 23 	. # 
	xor a			;7baa	af 	. 
l7babh:
	push af			;7bab	f5 	. 
	ld c,08ah		;7bac	0e 8a 	. . 
	ld de,l79fch		;7bae	11 fc 79 	. . y 
	call sub_78b2h		;7bb1	cd b2 78 	. . x 
	jr nc,l7bc2h		;7bb4	30 0c 	0 . 
	ld c,084h		;7bb6	0e 84 	. . 
	ld de,02000h		;7bb8	11 00 20 	. .   
	call l7732h		;7bbb	cd 32 77 	. 2 w 
	pop af			;7bbe	f1 	. 
	dec a			;7bbf	3d 	= 
	jr l7babh		;7bc0	18 e9 	. . 
l7bc2h:
	ld c,08eh		;7bc2	0e 8e 	. . 
	ld de,01c3eh		;7bc4	11 3e 1c 	. > . 
	call sub_78b2h		;7bc7	cd b2 78 	. . x 
	jr c,l7bd8h		;7bca	38 0c 	8 . 
	ld c,084h		;7bcc	0e 84 	. . 
	ld de,02000h		;7bce	11 00 20 	. .   
	call l7775h		;7bd1	cd 75 77 	. u w 
	pop af			;7bd4	f1 	. 
	inc a			;7bd5	3c 	< 
	jr l7babh		;7bd6	18 d3 	. . 
l7bd8h:
	ld c,080h		;7bd8	0e 80 	. . 
	ld de,00000h		;7bda	11 00 00 	. . . 
	call sub_76c1h		;7bdd	cd c1 76 	. . v 
	call sub_784fh		;7be0	cd 4f 78 	. O x 
	pop af			;7be3	f1 	. 
	add a,005h		;7be4	c6 05 	. . 
	cp 006h		;7be6	fe 06 	. . 
	jr nc,l7c38h		;7be8	30 4e 	0 N 
	dec a			;7bea	3d 	= 
	jp p,l7bfch		;7beb	f2 fc 7b 	. . { 
	ld (ix+000h),02eh		;7bee	dd 36 00 2e 	. 6 . . 
	inc ix		;7bf2	dd 23 	. # 
	ld (ix+000h),030h		;7bf4	dd 36 00 30 	. 6 . 0 
	inc ix		;7bf8	dd 23 	. # 
	ld a,0ffh		;7bfa	3e ff 	> . 
l7bfch:
	ld b,a			;7bfc	47 	G 
	ld de,003e8h		;7bfd	11 e8 03 	. . . 
	call sub_7c15h		;7c00	cd 15 7c 	. . | 
	ret nc			;7c03	d0 	. 
	ld de,00064h		;7c04	11 64 00 	. d . 
	call sub_7c15h		;7c07	cd 15 7c 	. . | 
	ret nc			;7c0a	d0 	. 
	ld de,0000ah		;7c0b	11 0a 00 	. . . 
	call sub_7c15h		;7c0e	cd 15 7c 	. . | 
	ret nc			;7c11	d0 	. 
	ld de,00001h		;7c12	11 01 00 	. . . 
sub_7c15h:
	inc b			;7c15	04 	. 
	djnz l7c20h		;7c16	10 08 	. . 
	ld (ix+000h),02eh		;7c18	dd 36 00 2e 	. 6 . . 
	inc ix		;7c1c	dd 23 	. # 
	ld b,0ffh		;7c1e	06 ff 	. . 
l7c20h:
	ld a,02fh		;7c20	3e 2f 	> / 
l7c22h:
	inc a			;7c22	3c 	< 
	and a			;7c23	a7 	. 
	sbc hl,de		;7c24	ed 52 	. R 
	jr nc,l7c22h		;7c26	30 fa 	0 . 
	add hl,de			;7c28	19 	. 
	ld (ix+000h),a		;7c29	dd 77 00 	. w . 
	inc ix		;7c2c	dd 23 	. # 
	dec b			;7c2e	05 	. 
	jr z,l7c33h		;7c2f	28 02 	( . 
	scf			;7c31	37 	7 
	ret p			;7c32	f0 	. 
l7c33h:
	ld a,l			;7c33	7d 	} 
	or h			;7c34	b4 	. 
	ret z			;7c35	c8 	. 
	scf			;7c36	37 	7 
	ret			;7c37	c9 	. 
l7c38h:
	push af			;7c38	f5 	. 
	ld a,001h		;7c39	3e 01 	> . 
	call l7bfch		;7c3b	cd fc 7b 	. . { 
	ld (ix+000h),045h		;7c3e	dd 36 00 45 	. 6 . E 
	inc ix		;7c42	dd 23 	. # 
	pop af			;7c44	f1 	. 
	sub 002h		;7c45	d6 02 	. . 
	ld l,a			;7c47	6f 	o 
	ld a,02bh		;7c48	3e 2b 	> + 
	jp p,l7c53h		;7c4a	f2 53 7c 	. S | 
	ld a,l			;7c4d	7d 	} 
	neg		;7c4e	ed 44 	. D 
	ld l,a			;7c50	6f 	o 
	ld a,02dh		;7c51	3e 2d 	> - 
l7c53h:
	ld (ix+000h),a		;7c53	dd 77 00 	. w . 
	inc ix		;7c56	dd 23 	. # 
	ld a,l			;7c58	7d 	} 
	ld b,02fh		;7c59	06 2f 	. / 
l7c5bh:
	inc b			;7c5b	04 	. 
	sub 00ah		;7c5c	d6 0a 	. . 
	jr nc,l7c5bh		;7c5e	30 fb 	0 . 
	add a,03ah		;7c60	c6 3a 	. : 
	ld (ix+000h),b		;7c62	dd 70 00 	. p . 
	inc ix		;7c65	dd 23 	. # 
	ld (ix+000h),a		;7c67	dd 77 00 	. w . 
	inc ix		;7c6a	dd 23 	. # 
	ret			;7c6c	c9 	. 
sub_7c6dh:
	ld b,000h		;7c6d	06 00 	. . 
	call sub_7c8bh		;7c6f	cd 8b 7c 	. . | 
	cp 02dh		;7c72	fe 2d 	. - 
	jr nz,l7c7fh		;7c74	20 09 	  . 
	inc de			;7c76	13 	. 
	call sub_7c6dh		;7c77	cd 6d 7c 	. m | 
	ld a,h			;7c7a	7c 	| 
	xor 080h		;7c7b	ee 80 	. . 
	ld h,a			;7c7d	67 	g 
	ret			;7c7e	c9 	. 
l7c7fh:
	cp 026h		;7c7f	fe 26 	. & 
	jr nz,l7ce4h		;7c81	20 61 	  a 
	call sub_7c92h		;7c83	cd 92 7c 	. . | 
	push de			;7c86	d5 	. 
	call TOKEN_MULTI_FUNCTION		;7c87	cd 2d 78 	. - x 
	pop de			;7c8a	d1 	. 
sub_7c8bh:
	ld a,(de)			;7c8b	1a 	. 
	cp 020h		;7c8c	fe 20 	.   
	ret nz			;7c8e	c0 	. 
	inc de			;7c8f	13 	. 
	jr sub_7c8bh		;7c90	18 f9 	. . 
sub_7c92h:
	ld hl,00000h		;7c92	21 00 00 	! . . 
	inc de			;7c95	13 	. 
	call sub_7d8fh		;7c96	cd 8f 7d 	. . } 
	inc de			;7c99	13 	. 
	cp 042h		;7c9a	fe 42 	. B 
	jr z,l7cd5h		;7c9c	28 37 	( 7 
	cp 04fh		;7c9e	fe 4f 	. O 
	jr z,l7cc4h		;7ca0	28 22 	( " 
	cp 048h		;7ca2	fe 48 	. H 
	dec de			;7ca4	1b 	. 
	ret nz			;7ca5	c0 	. 
	inc de			;7ca6	13 	. 
l7ca7h:
	call sub_7d8fh		;7ca7	cd 8f 7d 	. . } 
	and a			;7caa	a7 	. 
	ret z			;7cab	c8 	. 
	sub 030h		;7cac	d6 30 	. 0 
	ret c			;7cae	d8 	. 
	cp 00ah		;7caf	fe 0a 	. . 
	jr c,l7cbbh		;7cb1	38 08 	8 . 
	cp 011h		;7cb3	fe 11 	. . 
	ret c			;7cb5	d8 	. 
	cp 017h		;7cb6	fe 17 	. . 
	ret nc			;7cb8	d0 	. 
	sub 007h		;7cb9	d6 07 	. . 
l7cbbh:
	add hl,hl			;7cbb	29 	) 
	add hl,hl			;7cbc	29 	) 
	add hl,hl			;7cbd	29 	) 
	add hl,hl			;7cbe	29 	) 
	add a,l			;7cbf	85 	. 
	ld l,a			;7cc0	6f 	o 
	inc de			;7cc1	13 	. 
	jr l7ca7h		;7cc2	18 e3 	. . 
l7cc4h:
	ld a,(de)			;7cc4	1a 	. 
	and a			;7cc5	a7 	. 
	ret z			;7cc6	c8 	. 
	sub 030h		;7cc7	d6 30 	. 0 
	ret c			;7cc9	d8 	. 
	cp 008h		;7cca	fe 08 	. . 
	ret nc			;7ccc	d0 	. 
	add hl,hl			;7ccd	29 	) 
	add hl,hl			;7cce	29 	) 
	add hl,hl			;7ccf	29 	) 
	add a,l			;7cd0	85 	. 
	ld l,a			;7cd1	6f 	o 
	inc de			;7cd2	13 	. 
	jr l7cc4h		;7cd3	18 ef 	. . 
l7cd5h:
	ld a,(de)			;7cd5	1a 	. 
	and a			;7cd6	a7 	. 
	ret z			;7cd7	c8 	. 
	sub 030h		;7cd8	d6 30 	. 0 
	ret c			;7cda	d8 	. 
	cp 002h		;7cdb	fe 02 	. . 
	ret nc			;7cdd	d0 	. 
	add hl,hl			;7cde	29 	) 
	add a,l			;7cdf	85 	. 
	ld l,a			;7ce0	6f 	o 
	inc de			;7ce1	13 	. 
	jr l7cd5h		;7ce2	18 f1 	. . 
l7ce4h:
	ld c,001h		;7ce4	0e 01 	. . 
	dec de			;7ce6	1b 	. 
l7ce7h:
	inc de			;7ce7	13 	. 
	call sub_7c8bh		;7ce8	cd 8b 7c 	. . | 
	cp 030h		;7ceb	fe 30 	. 0 
	jr c,l7d1bh		;7ced	38 2c 	8 , 
	cp 03ah		;7cef	fe 3a 	. : 
	jr nc,l7d1bh		;7cf1	30 28 	0 ( 
	push de			;7cf3	d5 	. 
	push bc			;7cf4	c5 	. 
	push af			;7cf5	f5 	. 
	ld c,084h		;7cf6	0e 84 	. . 
	ld de,02000h		;7cf8	11 00 20 	. .   
	call l7732h		;7cfb	cd 32 77 	. 2 w 
	pop af			;7cfe	f1 	. 
	sub 030h		;7cff	d6 30 	. 0 
	jr z,l7d12h		;7d01	28 0f 	( . 
	ld c,088h		;7d03	0e 88 	. . 
l7d05h:
	dec c			;7d05	0d 	. 
	add a,a			;7d06	87 	. 
	jp p,l7d05h		;7d07	f2 05 7d 	. . } 
	and 07fh		;7d0a	e6 7f 	.  
	ld d,a			;7d0c	57 	W 
	ld e,000h		;7d0d	1e 00 	. . 
	call sub_76c1h		;7d0f	cd c1 76 	. . v 
l7d12h:
	pop de			;7d12	d1 	. 
	ld c,e			;7d13	4b 	K 
	pop de			;7d14	d1 	. 
	dec c			;7d15	0d 	. 
	jr nz,l7ce7h		;7d16	20 cf 	  . 
	inc c			;7d18	0c 	. 
	jr l7ce7h		;7d19	18 cc 	. . 
l7d1bh:
	cp 02eh		;7d1b	fe 2e 	. . 
	jr nz,l7d23h		;7d1d	20 04 	  . 
	dec c			;7d1f	0d 	. 
	jr z,l7ce7h		;7d20	28 c5 	( . 
	ret			;7d22	c9 	. 
l7d23h:
	dec c			;7d23	0d 	. 
	jr z,l7d27h		;7d24	28 01 	( . 
	inc c			;7d26	0c 	. 
l7d27h:
	call sub_7d90h		;7d27	cd 90 7d 	. . } 
	cp 045h		;7d2a	fe 45 	. E 
	jr z,l7d32h		;7d2c	28 04 	( . 
	cp 044h		;7d2e	fe 44 	. D 
	jr nz,l7d4ch		;7d30	20 1a 	  . 
l7d32h:
	inc de			;7d32	13 	. 
	call sub_7c8bh		;7d33	cd 8b 7c 	. . | 
	cp 02bh		;7d36	fe 2b 	. + 
	jr z,l7d46h		;7d38	28 0c 	( . 
	cp 02dh		;7d3a	fe 2d 	. - 
	jr nz,l7d47h		;7d3c	20 09 	  . 
	inc de			;7d3e	13 	. 
	call sub_7d73h		;7d3f	cd 73 7d 	. s } 
	neg		;7d42	ed 44 	. D 
	jr l7d4ah		;7d44	18 04 	. . 
l7d46h:
	inc de			;7d46	13 	. 
l7d47h:
	call sub_7d73h		;7d47	cd 73 7d 	. s } 
l7d4ah:
	add a,c			;7d4a	81 	. 
	ld c,a			;7d4b	4f 	O 
l7d4ch:
	ld a,c			;7d4c	79 	y 
	and a			;7d4d	a7 	. 
	ret z			;7d4e	c8 	. 
	push de			;7d4f	d5 	. 
	push af			;7d50	f5 	. 
	jp p,l7d56h		;7d51	f2 56 7d 	. V } 
	neg		;7d54	ed 44 	. D 
l7d56h:
	ld e,a			;7d56	5f 	_ 
	ld d,000h		;7d57	16 00 	. . 
	push hl			;7d59	e5 	. 
	push bc			;7d5a	c5 	. 
	ld b,084h		;7d5b	06 84 	. . 
	ld hl,02000h		;7d5d	21 00 20 	! .   
	call sub_77c1h		;7d60	cd c1 77 	. . w 
	ld c,b			;7d63	48 	H 
	ex de,hl			;7d64	eb 	. 
	pop af			;7d65	f1 	. 
	ld b,a			;7d66	47 	G 
	pop hl			;7d67	e1 	. 
	pop af			;7d68	f1 	. 
	push af			;7d69	f5 	. 
	call p,l7732h		;7d6a	f4 32 77 	. 2 w 
	pop af			;7d6d	f1 	. 
	call m,l7775h		;7d6e	fc 75 77 	. u w 
	pop de			;7d71	d1 	. 
	ret			;7d72	c9 	. 
sub_7d73h:
	push hl			;7d73	e5 	. 
	ld l,000h		;7d74	2e 00 	. . 
l7d76h:
	call sub_7c8bh		;7d76	cd 8b 7c 	. . | 
	cp 03ah		;7d79	fe 3a 	. : 
	jr nc,l7d8ch		;7d7b	30 0f 	0 . 
	sub 030h		;7d7d	d6 30 	. 0 
	jr c,l7d8ch		;7d7f	38 0b 	8 . 
	ld h,a			;7d81	67 	g 
	ld a,l			;7d82	7d 	} 
	add a,a			;7d83	87 	. 
	add a,a			;7d84	87 	. 
	add a,l			;7d85	85 	. 
	add a,a			;7d86	87 	. 
	add a,h			;7d87	84 	. 
	ld l,a			;7d88	6f 	o 
	inc de			;7d89	13 	. 
	jr l7d76h		;7d8a	18 ea 	. . 
l7d8ch:
	ld a,l			;7d8c	7d 	} 
	pop hl			;7d8d	e1 	. 
	ret			;7d8e	c9 	. 
sub_7d8fh:
	ld a,(de)			;7d8f	1a 	. 
sub_7d90h:
	cp 061h		;7d90	fe 61 	. a 
	ret c			;7d92	d8 	. 
	cp 07bh		;7d93	fe 7b 	. { 
	ret nc			;7d95	d0 	. 
	and 0dfh		;7d96	e6 df 	. . 
	ret			;7d98	c9 	. 
l7d99h:
	cp (hl)			;7d99	be 	. 
	jr c,l7d9dh		;7d9a	38 01 	8 . 
	ld a,(hl)			;7d9c	7e 	~ 
l7d9dh:
	inc hl			;7d9d	23 	# 
	jr l7dc7h		;7d9e	18 27 	. ' 
l7da0h:
	cp (hl)			;7da0	be 	. 
	jr c,l7da4h		;7da1	38 01 	8 . 
	ld a,(hl)			;7da3	7e 	~ 
l7da4h:
	push af			;7da4	f5 	. 
	ld e,(hl)			;7da5	5e 	^ 
	neg		;7da6	ed 44 	. D 
	add a,e			;7da8	83 	. 
	inc a			;7da9	3c 	< 
	ld e,a			;7daa	5f 	_ 
	ld d,000h		;7dab	16 00 	. . 
	add hl,de			;7dad	19 	. 
	pop af			;7dae	f1 	. 
	jr l7dc7h		;7daf	18 16 	. . 
sub_7db1h:
	ld c,b			;7db1	48 	H 
	push af			;7db2	f5 	. 
	add a,c			;7db3	81 	. 
	jr c,l7dbah		;7db4	38 04 	8 . 
	dec a			;7db6	3d 	= 
	cp (hl)			;7db7	be 	. 
	jr c,l7dc3h		;7db8	38 09 	8 . 
l7dbah:
	pop af			;7dba	f1 	. 
	ld a,(hl)			;7dbb	7e 	~ 
	sub c			;7dbc	91 	. 
	jr nc,l7dc1h		;7dbd	30 02 	0 . 
	ld a,0ffh		;7dbf	3e ff 	> . 
l7dc1h:
	inc a			;7dc1	3c 	< 
	push af			;7dc2	f5 	. 
l7dc3h:
	ld b,000h		;7dc3	06 00 	. . 
	add hl,bc			;7dc5	09 	. 
	pop af			;7dc6	f1 	. 
l7dc7h:
	ld de,BASIC_BUF		;7dc7	11 5e f5 	. ^ . 
	ld (de),a			;7dca	12 	. 
	and a			;7dcb	a7 	. 
	jr z,l7dd4h		;7dcc	28 06 	( . 
	inc de			;7dce	13 	. 
	ld c,a			;7dcf	4f 	O 
	ld b,000h		;7dd0	06 00 	. . 
	ldir		;7dd2	ed b0 	. . 
l7dd4h:
	ld hl,BASIC_BUF		;7dd4	21 5e f5 	! ^ . 
	ret			;7dd7	c9 	. 
sub_7dd8h:
	ex de,hl			;7dd8	eb 	. 
	cp (hl)			;7dd9	be 	. 
	jr z,l7dddh		;7dda	28 01 	( . 
	ret nc			;7ddc	d0 	. 
l7dddh:
	ld c,a			;7ddd	4f 	O 
	ld a,(de)			;7dde	1a 	. 
	cp b			;7ddf	b8 	. 
	jr nc,l7de3h		;7de0	30 01 	0 . 
	ld b,a			;7de2	47 	G 
l7de3h:
	ld a,c			;7de3	79 	y 
	dec a			;7de4	3d 	= 
	add a,b			;7de5	80 	. 
	jr c,l7dedh		;7de6	38 05 	8 . 
	cp (hl)			;7de8	be 	. 
	jr c,l7df1h		;7de9	38 06 	8 . 
	jr z,l7df1h		;7deb	28 04 	( . 
l7dedh:
	ld a,(hl)			;7ded	7e 	~ 
	sub c			;7dee	91 	. 
	inc a			;7def	3c 	< 
	ld b,a			;7df0	47 	G 
l7df1h:
	ld a,l			;7df1	7d 	} 
	add a,c			;7df2	81 	. 
	ld l,a			;7df3	6f 	o 
	jr nz,l7df7h		;7df4	20 01 	  . 
	inc h			;7df6	24 	$ 
l7df7h:
	ex de,hl			;7df7	eb 	. 
	inc hl			;7df8	23 	# 
	ld c,b			;7df9	48 	H 
	ld b,000h		;7dfa	06 00 	. . 
	ldir		;7dfc	ed b0 	. . 
	ret			;7dfe	c9 	. 
TOKEN_13_STR:
	ld hl,0f55fh		;7dff	21 5f f5 	! _ . 
	ld (hl),a			;7e02	77 	w 
	dec hl			;7e03	2b 	+ 
	ld (hl),001h		;7e04	36 01 	6 . 
	ret			;7e06	c9 	. 
sub_7e07h:
	ld de,0f55fh		;7e07	11 5f f5 	. _ . 
	push de			;7e0a	d5 	. 
	ld a,(hl)			;7e0b	7e 	~ 
	and a			;7e0c	a7 	. 
	jr z,l7e15h		;7e0d	28 06 	( . 
	ld c,a			;7e0f	4f 	O 
	xor a			;7e10	af 	. 
	ld b,a			;7e11	47 	G 
	inc hl			;7e12	23 	# 
	ldir		;7e13	ed b0 	. . 
l7e15h:
	ld (de),a			;7e15	12 	. 
	pop de			;7e16	d1 	. 
	jp sub_7c6dh		;7e17	c3 6d 7c 	. m | 
l7e1ah:
	ld c,004h		;7e1a	0e 04 	. . 
	ld de,0030eh		;7e1c	11 0e 03 	. . . 
	ld de,BIOS_SCALXY		;7e1f	11 0e 01 	. . . 
	ld de,0f7d5h		;7e22	11 d5 f7 	. . . 
l7e25h:
	ld b,c			;7e25	41 	A 
	xor a			;7e26	af 	. 
l7e27h:
	srl h		;7e27	cb 3c 	. < 
	rr l		;7e29	cb 1d 	. . 
	rra			;7e2b	1f 	. 
	djnz l7e27h		;7e2c	10 f9 	. . 
	ld b,c			;7e2e	41 	A 
l7e2fh:
	rlca			;7e2f	07 	. 
	djnz l7e2fh		;7e30	10 fd 	. . 
	cp 00ah		;7e32	fe 0a 	. . 
	jr c,l7e38h		;7e34	38 02 	8 . 
	add a,007h		;7e36	c6 07 	. . 
l7e38h:
	add a,030h		;7e38	c6 30 	. 0 
	ld (de),a			;7e3a	12 	. 
	dec de			;7e3b	1b 	. 
	ld a,l			;7e3c	7d 	} 
	or h			;7e3d	b4 	. 
	jr nz,l7e25h		;7e3e	20 e5 	  . 
	ld a,0d5h		;7e40	3e d5 	> . 
	sub e			;7e42	93 	. 
	ld (de),a			;7e43	12 	. 
	ex de,hl			;7e44	eb 	. 
	ret			;7e45	c9 	. 
l7e46h:
	ld b,a			;7e46	47 	G 
	ld a,020h		;7e47	3e 20 	>   
sub_7e49h:
	ld hl,BASIC_BUF		;7e49	21 5e f5 	! ^ . 
	ld (hl),b			;7e4c	70 	p 
	inc b			;7e4d	04 	. 
	dec b			;7e4e	05 	. 
	ret z			;7e4f	c8 	. 
	push hl			;7e50	e5 	. 
	inc hl			;7e51	23 	# 
l7e52h:
	ld (hl),a			;7e52	77 	w 
	inc hl			;7e53	23 	# 
	djnz l7e52h		;7e54	10 fc 	. . 
	pop hl			;7e56	e1 	. 
	ret			;7e57	c9 	. 
sub_7e58h:
	call BIOS_CHSNS		;7e58	cd 9c 00 	. . . 
	ld hl,BASIC_BUF		;7e5b	21 5e f5 	! ^ . 
	ld a,000h		;7e5e	3e 00 	> . 
	jr z,l7e6ah		;7e60	28 08 	( . 
	call BIOS_CHGET		;7e62	cd 9f 00 	. . . 
	inc hl			;7e65	23 	# 
	ld (hl),a			;7e66	77 	w 
	dec hl			;7e67	2b 	+ 
	ld a,001h		;7e68	3e 01 	> . 
l7e6ah:
	ld (hl),a			;7e6a	77 	w 
BIOS_IOALLOC:
	ret			;7e6b	c9 	. 
sub_7e6ch:
	ld c,a			;7e6c	4f 	O 
	ld a,(de)			;7e6d	1a 	. 
	sub c			;7e6e	91 	. 
	sub (hl)			;7e6f	96 	. 
	add a,002h		;7e70	c6 02 	. . 
	ld b,a			;7e72	47 	G 
	ld a,c			;7e73	79 	y 
	dec a			;7e74	3d 	= 
	add a,e			;7e75	83 	. 
	ld e,a			;7e76	5f 	_ 
	jr nc,l7e7ah		;7e77	30 01 	0 . 
	inc d			;7e79	14 	. 
l7e7ah:
	push bc			;7e7a	c5 	. 
	push hl			;7e7b	e5 	. 
	ld b,(hl)			;7e7c	46 	F 
	inc hl			;7e7d	23 	# 
	inc de			;7e7e	13 	. 
	push de			;7e7f	d5 	. 
l7e80h:
	ld a,(de)			;7e80	1a 	. 
	cp (hl)			;7e81	be 	. 
	jr nz,l7e8fh		;7e82	20 0b 	  . 
	inc de			;7e84	13 	. 
	inc hl			;7e85	23 	# 
	djnz l7e80h		;7e86	10 f8 	. . 
	pop af			;7e88	f1 	. 
	pop af			;7e89	f1 	. 
	pop af			;7e8a	f1 	. 
	ld l,c			;7e8b	69 	i 
	ld h,000h		;7e8c	26 00 	& . 
	ret			;7e8e	c9 	. 
l7e8fh:
	pop de			;7e8f	d1 	. 
	pop hl			;7e90	e1 	. 
	pop bc			;7e91	c1 	. 
	inc c			;7e92	0c 	. 
	djnz l7e7ah		;7e93	10 e5 	. . 
	ld hl,00000h		;7e95	21 00 00 	! . . 
	ret			;7e98	c9 	. 
sub_7e99h:
	ld de,(0f862h)		;7e99	ed 5b 62 f8 	. [ b . 
	ld c,(hl)			;7e9d	4e 	N 
	ld b,000h		;7e9e	06 00 	. . 
	inc bc			;7ea0	03 	. 
	ldir		;7ea1	ed b0 	. . 
	ret			;7ea3	c9 	. 
	call sub_7ee0h		;7ea4	cd e0 7e 	. . ~ 
	ld hl,00000h		;7ea7	21 00 00 	! . . 
	ret c			;7eaa	d8 	. 
	ret z			;7eab	c8 	. 
	dec hl			;7eac	2b 	+ 
	ret			;7ead	c9 	. 
	call sub_7ee0h		;7eae	cd e0 7e 	. . ~ 
	ld hl,0ffffh		;7eb1	21 ff ff 	! . . 
	ret z			;7eb4	c8 	. 
	inc hl			;7eb5	23 	# 
	ret			;7eb6	c9 	. 
	call sub_7ee0h		;7eb7	cd e0 7e 	. . ~ 
	ld hl,0ffffh		;7eba	21 ff ff 	! . . 
	ret nc			;7ebd	d0 	. 
	inc hl			;7ebe	23 	# 
	ret			;7ebf	c9 	. 
	call sub_7ee0h		;7ec0	cd e0 7e 	. . ~ 
	ld hl,0ffffh		;7ec3	21 ff ff 	! . . 
	ret c			;7ec6	d8 	. 
	inc hl			;7ec7	23 	# 
	ret			;7ec8	c9 	. 
	call sub_7ee0h		;7ec9	cd e0 7e 	. . ~ 
	ld hl,0ffffh		;7ecc	21 ff ff 	! . . 
	ret nz			;7ecf	c0 	. 
	inc hl			;7ed0	23 	# 
	ret			;7ed1	c9 	. 
	call sub_7ee0h		;7ed2	cd e0 7e 	. . ~ 
	ld hl,0ffffh		;7ed5	21 ff ff 	! . . 
	ret c			;7ed8	d8 	. 
	ret z			;7ed9	c8 	. 
	inc hl			;7eda	23 	# 
	ret			;7edb	c9 	. 
	ld hl,0ffffh		;7edc	21 ff ff 	! . . 
	ret			;7edf	c9 	. 
sub_7ee0h:
	ld b,(hl)			;7ee0	46 	F 
	inc hl			;7ee1	23 	# 
	ld de,(0f862h)		;7ee2	ed 5b 62 f8 	. [ b . 
	ld a,(de)			;7ee6	1a 	. 
	ld c,a			;7ee7	4f 	O 
	inc de			;7ee8	13 	. 
l7ee9h:
	ld a,c			;7ee9	79 	y 
	or b			;7eea	b0 	. 
	ret z			;7eeb	c8 	. 
	ld a,c			;7eec	79 	y 
	sub 001h		;7eed	d6 01 	. . 
	ret c			;7eef	d8 	. 
	ld a,b			;7ef0	78 	x 
	sub 001h		;7ef1	d6 01 	. . 
	ccf			;7ef3	3f 	? 
	ret nc			;7ef4	d0 	. 
	dec b			;7ef5	05 	. 
	dec c			;7ef6	0d 	. 
	ld a,(de)			;7ef7	1a 	. 
	cp (hl)			;7ef8	be 	. 
	inc de			;7ef9	13 	. 
	inc hl			;7efa	23 	# 
	jr z,l7ee9h		;7efb	28 ec 	( . 
	ret			;7efd	c9 	. 
sub_7efeh:
	ld de,BASIC_BUF		;7efe	11 5e f5 	. ^ . 
	ld bc,(0f862h)		;7f01	ed 4b 62 f8 	. K b . 
	and a			;7f05	a7 	. 
	push hl			;7f06	e5 	. 
	sbc hl,de		;7f07	ed 52 	. R 
	pop hl			;7f09	e1 	. 
	jr z,l7f37h		;7f0a	28 2b 	( + 
	ld a,(bc)			;7f0c	0a 	. 
	add a,(hl)			;7f0d	86 	. 
	jr nc,l7f12h		;7f0e	30 02 	0 . 
	ld a,0ffh		;7f10	3e ff 	> . 
l7f12h:
	ld (de),a			;7f12	12 	. 
	inc de			;7f13	13 	. 
	push hl			;7f14	e5 	. 
	ld a,(bc)			;7f15	0a 	. 
	push af			;7f16	f5 	. 
	and a			;7f17	a7 	. 
	jr z,l7f22h		;7f18	28 08 	( . 
	inc bc			;7f1a	03 	. 
	ld l,c			;7f1b	69 	i 
	ld h,b			;7f1c	60 	` 
	ld c,a			;7f1d	4f 	O 
	ld b,000h		;7f1e	06 00 	. . 
	ldir		;7f20	ed b0 	. . 
l7f22h:
	pop af			;7f22	f1 	. 
	pop hl			;7f23	e1 	. 
	ld b,a			;7f24	47 	G 
	ld a,(hl)			;7f25	7e 	~ 
	jr nc,l7f2ah		;7f26	30 02 	0 . 
	ld a,b			;7f28	78 	x 
l7f29h:
	cpl			;7f29	2f 	/ 
l7f2ah:
	and a			;7f2a	a7 	. 
	jr z,l7f33h		;7f2b	28 06 	( . 
	inc hl			;7f2d	23 	# 
	ld c,a			;7f2e	4f 	O 
	ld b,000h		;7f2f	06 00 	. . 
	ldir		;7f31	ed b0 	. . 
l7f33h:
	ld hl,BASIC_BUF		;7f33	21 5e f5 	! ^ . 
	ret			;7f36	c9 	. 
l7f37h:
	push bc			;7f37	c5 	. 
	push hl			;7f38	e5 	. 
	ld e,c			;7f39	59 	Y 
	ld d,b			;7f3a	50 	P 
	ld a,(de)			;7f3b	1a 	. 
	cp (hl)			;7f3c	be 	. 
	jr nc,l7f40h		;7f3d	30 01 	0 . 
	ld a,(hl)			;7f3f	7e 	~ 
l7f40h:
	ld b,a			;7f40	47 	G 
	inc b			;7f41	04 	. 
l7f42h:
	ld c,(hl)			;7f42	4e 	N 
	ld a,(de)			;7f43	1a 	. 
	ld (hl),a			;7f44	77 	w 
	ld a,c			;7f45	79 	y 
	ld (de),a			;7f46	12 	. 
	inc de			;7f47	13 	. 
	inc hl			;7f48	23 	# 
	djnz l7f42h		;7f49	10 f7 	. . 
	pop bc			;7f4b	c1 	. 
	ld a,(bc)			;7f4c	0a 	. 
	ld l,a			;7f4d	6f 	o 
	ld h,000h		;7f4e	26 00 	& . 
	inc hl			;7f50	23 	# 
	add hl,bc			;7f51	09 	. 
	ex de,hl			;7f52	eb 	. 
	pop hl			;7f53	e1 	. 
	add a,(hl)			;7f54	86 	. 
	jr c,l7f5bh		;7f55	38 04 	8 . 
	ld (bc),a			;7f57	02 	. 
	ld a,(hl)			;7f58	7e 	~ 
	jr l7f2ah		;7f59	18 cf 	. . 
l7f5bh:
	ld a,(bc)			;7f5b	0a 	. 
	push af			;7f5c	f5 	. 
	ld a,0ffh		;7f5d	3e ff 	> . 
	ld (bc),a			;7f5f	02 	. 
	pop af			;7f60	f1 	. 
	jr l7f29h		;7f61	18 c6 	. . 
	ei			;7f63	fb 	. 
	push hl			;7f64	e5 	. 
	ld hl,(PROCNM)		;7f65	2a 89 fd 	* . . 
	ld de,l4341h		;7f68	11 41 43 	. A C 
	rst 20h			;7f6b	e7 	. 
	pop hl			;7f6c	e1 	. 
	scf			;7f6d	37 	7 
	ret nz			;7f6e	c0 	. 
	ld a,(0fd8bh)		;7f6f	3a 8b fd 	: . . 
l7f72h:
	and a			;7f72	a7 	. 
	scf			;7f73	37 	7 
	ret nz			;7f74	c0 	. 
	push hl			;7f75	e5 	. 
	call 0bf7bh		;7f76	cd 7b bf 	. { . 
	pop hl			;7f79	e1 	. 
	ret			;7f7a	c9 	. 
	call 0bfb3h		;7f7b	cd b3 bf 	. . . 
	ret c			;7f7e	d8 	. 
	push af			;7f7f	f5 	. 
	ld c,a			;7f80	4f 	O 
	add a,a			;7f81	87 	. 
	add a,a			;7f82	87 	. 
	add a,a			;7f83	87 	. 
	add a,a			;7f84	87 	. 
	and 030h		;7f85	e6 30 	. 0 
	ld b,a			;7f87	47 	G 
	ld a,c			;7f88	79 	y 
	and 00ch		;7f89	e6 0c 	. . 
	or b			;7f8b	b0 	. 
	ld c,a			;7f8c	4f 	O 
	ld b,000h		;7f8d	06 00 	. . 
	ld hl,0fccah		;7f8f	21 ca fc 	! . . 
	add hl,bc			;7f92	09 	. 
	ld (hl),020h		;7f93	36 20 	6   
	pop af			;7f95	f1 	. 
	ld h,040h		;7f96	26 40 	& @ 
	call BIOS_ENASLT		;7f98	cd 24 00 	. $ . 
	ld bc,header1_start		;7f9b	01 00 40 	. . @ 
	ld e,c			;7f9e	59 	Y 
	ld d,b			;7f9f	50 	P 
	ld hl,parse_code_end		;7fa0	21 00 80 	! . . 
	ldir		;7fa3	ed b0 	. . 
	ld hl,STATEMENT		;7fa5	21 32 40 	! 2 @ 
	ld (l4004h),hl		;7fa8	22 04 40 	" . @ 
	ld a,(BIOS_EXPTBL)		;7fab	3a c1 fc 	: . . 
	call BIOS_ENASLT		;7fae	cd 24 00 	. $ . 
	and a			;7fb1	a7 	. 
	ret			;7fb2	c9 	. 
	ld a,(0ffa7h)		;7fb3	3a a7 ff 	: . . 
	cp 0f7h		;7fb6	fe f7 	. . 
	ld a,(BIOS_RAMAD1)		;7fb8	3a 42 f3 	: B . 
	ret z			;7fbb	c8 	. 
	ld hl,BIOS_EXPTBL		;7fbc	21 c1 fc 	! . . 
	ld b,004h		;7fbf	06 04 	. . 
	xor a			;7fc1	af 	. 
l7fc2h:
	and 003h		;7fc2	e6 03 	. . 
	or (hl)			;7fc4	b6 	. 
l7fc5h:
	push bc			;7fc5	c5 	. 
	push hl			;7fc6	e5 	. 
	ld hl,header1_start		;7fc7	21 00 40 	! . @ 
	ld e,0aah		;7fca	1e aa 	. . 
	call 0bfefh		;7fcc	cd ef bf 	. . . 
	jr nz,l7fddh		;7fcf	20 0c 	  . 
	inc hl			;7fd1	23 	# 
	ld e,055h		;7fd2	1e 55 	. U 
	call 0bfefh		;7fd4	cd ef bf 	. . . 
	jr nz,l7fddh		;7fd7	20 04 	  . 
	pop hl			;7fd9	e1 	. 
	pop hl			;7fda	e1 	. 
	and a			;7fdb	a7 	. 
	ret			;7fdc	c9 	. 
l7fddh:
	pop hl			;7fdd	e1 	. 
	pop bc			;7fde	c1 	. 
	and a			;7fdf	a7 	. 
	jp p,0bfe9h		;7fe0	f2 e9 bf 	. . . 
	add a,004h		;7fe3	c6 04 	. . 
	cp 090h		;7fe5	fe 90 	. . 
	jr c,l7fc5h		;7fe7	38 dc 	8 . 
	inc hl			;7fe9	23 	# 
	inc a			;7fea	3c 	< 
	djnz l7fc2h		;7feb	10 d5 	. . 
	scf			;7fed	37 	7 
	ret			;7fee	c9 	. 
	ld d,a			;7fef	57 	W 
	push de			;7ff0	d5 	. 
	push af			;7ff1	f5 	. 
	call 00014h		;7ff2	cd 14 00 	. . . 
	pop af			;7ff5	f1 	. 
	call 0000ch		;7ff6	cd 0c 00 	. . . 
	pop de			;7ff9	d1 	. 
	cp e			;7ffa	bb 	. 
	ld a,d			;7ffb	7a 	z 
	ret			;7ffc	c9 	. 
	rst 38h			;7ffd	ff 	. 
	rst 38h			;7ffe	ff 	. 
l7fffh:
	rst 38h			;7fff	ff 	. 
parse_code_end:

*/
