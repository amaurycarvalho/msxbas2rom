ENASLT:	equ	0024h
RSLREG:	equ	0138h
EXPTBL:	equ	0FCC1h		; Extended slot flags table (4 bytes)

PageSize:	equ 4000h	; 16kB

	org 4000h
 
; ### ROM header ###

	db "AB"		; ID for auto-executable ROM
	dw INIT1	; Main program execution address.
	dw 0		; STATEMENT
	dw 0		; DEVICE
	dw 0		; TEXT
	dw 0,0,0	; Reserved

INIT1:	; Program code entry point label
	call	RSLREG
	rrca
	rrca
	and	3	;Keep bits corresponding to the page 8000h-BFFFh
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
	ld	h,080h
	call	ENASLT		; Select the ROM on page 8000h-BFFFh

	jp	INIT2	; Jump to above page.
 

; Padding with 255 to make a fixed page of 16K size
; (Alternatively, include macros.asm and use ALIGN 4000H)

	ds PageSize - ($ - 4000h),255	; Fill the unused aera in page with 0FFh

; Begin of page 8000h-BFFFh

INIT2:
    ret
 
	ds PageSize - ($ - 8000h),255	; Fill the unused aera with 0FFh
