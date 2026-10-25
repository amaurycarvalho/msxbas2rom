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

