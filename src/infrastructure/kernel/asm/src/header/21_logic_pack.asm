;---------------------------------------------------------------------------------------------------------
; LOGICAL / MATH INTEGER ROUTINES
;---------------------------------------------------------------------------------------------------------

intCompareEQ:
  and a
  sbc hl,de
  ld hl, 0xFFFF
  ret z
    inc hl
    ret

intCompareNE:
  and a
  sbc hl,de
  ld hl, 0xFFFF
  ret nz
    inc hl
    ret

intCompareLT:
  ex de,hl
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  sbc hl,hl
  ret

intCompareLE:
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  ccf
  sbc hl,hl
  ret

intCompareGT:
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  sbc hl,hl
  ret

intCompareGE:
  ex de,hl
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  ccf
  sbc hl,hl
  ret

intCompareNOT:
  ld a, l
  cpl
  ld l, a
  ld a, h
  cpl
  ld h, a
  ret

intCompareAND:
  ld a, l
  and e
  ld l, a
  ld a, h
  and d
  ld h, a
  ret

intCompareOR:
  ld a, l
  or e
  ld l, a
  ld a, h
  or d
  ld h, a
  ret

intCompareXOR:
  ld a, l
  xor e
  ld l, a
  ld a, h
  xor d
  ld h, a
  ret

intSHR:
  ex de, hl
  ld a, e
  or d
  jr z, intSHR.2
intSHR.1:
    xor a
    sra h
    rr l
    dec e
    jr nz, intSHR.1
intSHR.2:
  ret

intSHL:
  ex de, hl
  ld a, e
  or d
  jr z,intSHL.2
intSHL.1:
    add hl, hl     ; optimization (xor a; sla l; rl h)
    dec e
    jr nz, intSHL.1
intSHL.2:
  ret

intNeg:
  xor a
  sub l
  ld l,a
  sbc a,a
  sub h
  ld h,a
  ret

floatNeg:
  ex de, hl
  ld c, b
  xor a
  ld b, a
  ld l, a
  ld h, a
  jp XBASIC_SUBTRACT_FLOATS     ; calculate zero - float

castParamFloatInt:
  ex de,hl      ; swap old hl (float)
  pop hl        ; get return address
  ex (sp),hl    ; swap int from stack
  push de       ; save old float into stack
  push bc
  call XBASIC_CAST_INTEGER_TO_FLOAT   ; xbasic int to float (in hl, out b:hl)
  pop af
  pop de        ; restore old float
  ex (sp), hl   ; put new float into stack
  push bc
  push hl       ; put return address into stack
  ex de,hl      ; restore old float
  ld b, a
  ret

; Float to BCD
; in b:hl = float
; out DAC (math pack)
FloatToBCD:
    ld a, 8           ; double value type in DAC
    ld (VALTYP), a
    ld a, h           ; get float sign (positive/negative) in 7th bit
    and 0x80
    or 0x4C
    ld (DAC), a       ; bcd exponent = 12 digits
    ld a, h
    res 7, a
    ld h, a           ; reset sign bit
    ld de, DAC+1
    ld (TEMP), de     ; save next DAC pointer
    ld c, 0x9b
    ld de, 0x3ebc     ; 100000000.00
    call FloatToBCD.eval.divide
    ld c, 0x8e
    ld de, 0x1c40     ; 10000.00
    call FloatToBCD.eval.divide
    ld c, 0x81
    ld de, 0x0000     ; 1.00
    call FloatToBCD.eval.divide
    ld c, 0x8e
    ld de, 0x1c40     ; 10000.00
FloatToBCD.eval.multiply:
    call C7748        ; multiply floats (b:hl * c:de)
    jr FloatToBCD.add
FloatToBCD.eval.divide:
    ld a, b           ; save b:hl
    push af
    push hl
      ld a, c         ; save c:de
      push af
      push de
        call C782C    ; divide floats (b:hl / c:de)
        call FloatToBCD.add
        call XBASIC_CAST_INTEGER_TO_FLOAT   ; xbasic int to float (in hl, out b:hl)
      pop de          ; restore c:de
      pop af
      ld c, a
      call C7748      ; multiply floats
      ex de, hl
      ld c, b
    pop hl            ; restore b:hl
    pop af
    ld b, a
    jp C76D3          ; subtract floats
FloatToBCD.add:
    call C7901        ; convert to integer
    push hl
      call FloatToBCD.add.1
      push af
        call FloatToBCD.add.2
      pop af
      or e
      push af
        call FloatToBCD.add.1
        push af
         call FloatToBCD.add.2
        pop af
        or e
        ld hl, (TEMP)   ; restore next DAC pointer
        ld (hl), a
      pop af
      inc hl
      ld (hl), a
      inc hl
      ld (TEMP), hl
    pop hl
    ret
FloatToBCD.add.0:
    ld de, 10
    jp C7643      ; integer divide (HL/DE, remainder = DE)
FloatToBCD.add.1:
    call FloatToBCD.add.0
    ld a, e
    ret
FloatToBCD.add.2:
    call FloatToBCD.add.0
    sla e
    sla e
    sla e
    sla e
    ret
