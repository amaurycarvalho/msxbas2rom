cmd_pad:
  ld a, l
  cp 8
  jr c, cmd_pad.from_bios
    ld a, (VERSION)
    or a
    ld a, l
    jr nz, cmd_pad.from_bios
cmd_pad.mouse_on_msx1:
  cp 12
  jr z, cmd_pad.mouse_on_msx1.12
  cp 13
  jr z, cmd_pad.mouse_on_msx1.13
  cp 14
  jr z, cmd_pad.mouse_on_msx1.14
  cp 16
  jr z, cmd_pad.mouse_on_msx1.16
  cp 17
  jr z, cmd_pad.mouse_on_msx1.13
  cp 18
  jr z, cmd_pad.mouse_on_msx1.14
  xor a
  jr cmd_pad.end
cmd_pad.mouse_on_msx1.12:
  ld de, 0x1310              ; mouse on port 1
cmd_pad.mouse_on_msx1.12.cont:
  call cmd_pad.GTMOUS
  ld a, l
  neg
  ld (PADY), a
  ld a, h
  neg
  ld (PADX), a
  ld a, 0xff
  jr cmd_pad.end
cmd_pad.mouse_on_msx1.16:
  ld de, 0x6C20              ; mouse on port 2
  jr cmd_pad.mouse_on_msx1.12.cont
cmd_pad.mouse_on_msx1.13:
  ld a, (PADX)
  jr cmd_pad.end
cmd_pad.mouse_on_msx1.14:
  ld a, (PADY)
  jr cmd_pad.end
cmd_pad.from_bios:
  call GTPAD
cmd_pad.end:
  ld l, a
  and 0x80
  ld h, a
  ret z
  ld h, 255
  ret

; https://www.msx.org/wiki/Mouse/Trackball#Direct_usage_of_mouse
; Routine to read the mouse by direct accesses (works on MSX1/2/2+/turbo R)
; Input: DE = 01310h for mouse in port 1 (D = 00010011b, E = 00010000b)
;        DE = 06C20h for mouse in port 2 (D = 01101100b, E = 00100000b)
; Output: H = X-offset, L = Y-offset (H = L = 255 if no mouse)
cmd_pad.WAIT1:  equ   10                ; Short delay value
cmd_pad.WAIT2:  equ   30                ; Long delay value
cmd_pad.GTMOUS:
	ld	b, cmd_pad.WAIT2	            ; Long delay for first read
	call cmd_pad.GTOFS2	                ; Read bits 7-4 of the x-offset
	and	0x0F
	rlca
	rlca
	rlca
	rlca
	ld c,a
	call cmd_pad.GTOFST	                ; Read bits 3-0 of the x-offset
	and	0x0F
	or c
	ld h,a	                            ; Store combined x-offset
	call cmd_pad.GTOFST	                ; Read bits 7-4 of the y-offset
	and	0x0F
	rlca
	rlca
	rlca
	rlca
	ld c,a
	call cmd_pad.GTOFST	                ; Read bits 3-0 of the y-offset
	and 0x0F
	or c
	ld l,a		                        ; Store combined y-offset
	ret
cmd_pad.GTOFST:
    ld b, cmd_pad.WAIT1
cmd_pad.GTOFS2:
    ld a, 15		                    ; Read PSG register 15 for mouse
	di		                            ; DI useless if the routine is used during an interrupt
	  out (0xA0),a
	  in a, (0xA1)
	  and 0x80                          ; preserve LED code/Kana state
	  or  d                             ; mouse1 x0010011b / mouse2 x1101100b
	  out (0xA1),a
	  xor e
	  ld d,a
	  call cmd_pad.WAITMS	            ; Extra delay because the mouse is slow
  	  ld a,14
	  out (0xA0),a
	ei		                            ; EI useless if the routine is used during an interrupt
	in a,(0xA2)
	ret
cmd_pad.WAITMS:
	ld a,b
cmd_pad.WTTR:
	djnz cmd_pad.WTTR
	db 0xED,0x55	                    ; back if Z80 (RETN on Z80, NOP on R800)
	rlca
	rlca
	ld b,a
cmd_pad.WTTR2:
	djnz cmd_pad.WTTR2
	ld b,a
cmd_pad.WTTR3:
	djnz cmd_pad.WTTR3
	ret


