;---------------------------------------------------------------------------------------------------------
; Pletter v0.5c1 adapted from XL2S Entertainment 2008
; https://github.com/nanochess/Pletter
; PLETTER UNPACKED RAM TO RAM
; HL = packed data in RAM, DE = destination in RAM
; define lengthindata when the original size is written in the pletter data
; define LENGTHINDATA
;---------------------------------------------------------------------------------------------------------

  macro PLETTER.GETBIT
    add a,a
    call z,pletter.getbit
  endm

  macro PLETTER.GETBITEXX
    add a,a
    call z,pletter.getbitexx
  endm

pletter.modes
  dw pletter.offsok
  dw pletter.mode2
  dw pletter.mode3
  dw pletter.mode4
  dw pletter.mode5
  dw pletter.mode6

pletter.unpack:

  if defined LENGTHINDATA
    inc hl
    inc hl
  endif

  ld a,(hl)
  inc hl
  exx
  ld de,0
  add a,a
  inc a
  rl e
  add a,a
  rl e
  add a,a
  rl e
  rl e
  ld hl,pletter.modes
  add hl,de
  ld e,(hl)
  ld ixl,e
  inc hl
  ld e,(hl)
  ld ixh,e
  ld e,1
  exx
  ld iy,pletter.loop
pletter.literal:
  ldi
pletter.loop:
  PLETTER.GETBIT
  jr nc,pletter.literal
  exx
  ld h,d
  ld l,e
pletter.getlen:
  PLETTER.GETBITEXX
  jr nc,pletter.lenok
pletter.lus:
  PLETTER.GETBITEXX
  adc hl,hl
  ret c
  PLETTER.GETBITEXX
  jr nc,pletter.lenok
  PLETTER.GETBITEXX
  adc hl,hl
  ret c
  PLETTER.GETBITEXX
  jp c,pletter.lus
pletter.lenok
  inc hl
  exx
  ld c,(hl)
  inc hl
  ld b,0
  bit 7,c
  jp z,pletter.offsok
  push ix
  ret       ;jp ix

pletter.mode6
  PLETTER.GETBIT
  rl b
pletter.mode5
  PLETTER.GETBIT
  rl b
pletter.mode4
  PLETTER.GETBIT
  rl b
pletter.mode3
  PLETTER.GETBIT
  rl b
pletter.mode2
  PLETTER.GETBIT
  rl b
  PLETTER.GETBIT
  jr nc,pletter.offsok
  or a
  inc b
  res 7,c
pletter.offsok
  inc bc
  push hl
  exx
  push hl
  exx
  ld l,e
  ld h,d
  sbc hl,bc
  pop bc
  ldir
  pop hl
  ;jp iy
  push iy
  ret

pletter.getbit
  ld a,(hl)
  inc hl
  rla
  ret

pletter.getbitexx
  exx
  ld a,(hl)
  inc hl
  exx
  rla
  ret

