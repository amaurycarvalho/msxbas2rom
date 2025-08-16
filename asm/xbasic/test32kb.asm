LF:	equ	0Ah
CR:	equ	0Dh

CHPUT:	equ	00A2h	; Address of character output routine of BIOS
ENASLT:	equ	0024h
INIT32:	equ	006Fh
RSLREG:	equ	0138h

PageSize:	equ 4000h	; 16kB

LINL32:	equ	0F3AFh
EXPTBL:	equ	0FCC1h		; Extended slot flags table (4 bytes)
 
	org 4000h
 
	ld	hl,Page4000hTXT	; Text pointer into HL
	call	Print		; Call the routine Print
 
Finished:
	jr	Finished	; Jump to itself endlessly.

; Message data
Page4000hTXT:			; Text pointer label
	db "Text from page 4000h-7FFFh",0	; Zero indicates the end of text

; Padding with 255 to make a fixed page of 16K size
; (Alternatively, include macros.asm and use ALIGN 4000H)

	ds PageSize - ($ - 4000h),255	; Fill the unused aera in page with 0FFh

; Begin of page 8000h-BFFFh

; ### ROM header ###

	db "AB"		; ID for auto-executable ROM
	dw INIT		; Main program execution address.
	dw 0		; STATEMENT
	dw 0		; DEVICE
	dw 0		; TEXT
	dw 0,0,0	; Reserved

INIT:	; Program code entry point label

	ld	a,32
	ld	(LINL32),a	; 32 columns
	call	INIT32		; SCREEN 1

; Typical routine to select the ROM on page 4000h-7FFFh from page 8000h-BFFFh

	call	RSLREG
	rrca
	rrca
	rrca
	rrca
	and	3	;Keep bits corresponding to the page 4000h-7FFFh
	ld	c,a
	ld	b,0
	ld	hl,EXPTBL
	add	hl,bc
	ld	a,(hl)
	and	80h
	or	c
	ld	c,a
	inc	hl
	inc	hl
	inc	hl
	inc	hl
	ld	a,(hl)
	and	0Ch
	or	c
	ld	h,040h
	call	ENASLT		; Select the ROM on page 4000h-7FFFh

	ld	hl,Page8000hTXT	; Text pointer
	call	Print		; Call the routine Print below
 
	jp	04000h	; Jump to below page.
 
Print:
	ld	a,(hl)		; Load the byte from memory at address indicated by HL to A.
	and	a		; Same as CP 0 but faster.
	ret	z		; Back behind the call print if A = 0
	call	CHPUT		; Call the routine to display a character.
	inc	hl		; Increment the HL value.
	jr	Print		; Relative jump to the address in the label Print.

Page8000hTXT:			; Text pointer label
	db "Text from page 8000h-BFFFh",LF,CR,0	; Zero indicates the end of text.

	ds PageSize - ($ - 8000h),255	; Fill the unused aera with 0FFh
