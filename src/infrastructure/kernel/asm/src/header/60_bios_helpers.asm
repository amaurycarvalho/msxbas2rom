;---------------------------------------------------------------------------------------------------------
; MEMORY / SLOT / PAGE ROUTINES
;---------------------------------------------------------------------------------------------------------

resource.open:
  ld a, (RSCMAPSG)
  or a
  jr nz, select_rsc_on_megarom

select_rsc_on_page_0:
    ld a, (SLTAD2)
    ld h,000h
    call SUB_ENASLT		            ; Select the ROM on page 0000h-3FFFh (with interruptions disabled)

    ld a, (MAPPER)
    bit 0, a                        ; mapper on?
    ret z                           ; return if no mapper

      ld a, (MPRAD0)                ; original memory mapper segment for page 0 rom
      out (0xFC), a
  ret

resource.close:
  ld a, (RSCMAPSG)
  or a
  jr nz, select_rom_on_megarom

select_rom_on_page_0:
    ld a, (SLTAD0)
    ld h,000h
  jp SUB_ENASLT		                ; Select the BIOS ROM (with interruptions disabled)

select_rom_on_megarom:
  ld a, (RSCMAPT1)
  jp MR_CHANGE_SGM

select_rsc_on_megarom:
  push af
    ld a, (SGMADR)
    ld (RSCMAPT1), a
  pop af
  jp MR_CHANGE_SGM

resource.open_and_get_address:
  call resource.open
  ld bc, (DAC)                            ; bc = resource number
  jp resource.address                     ; hl = resource start address, a = segment, bc = resource size

resource.copy_to_ram_on_page_2:           ; needs di/ei
  ld a, (RSCMAPSG)                        ; test megarom
  or a
  jr z, resource.copy_to_ram_on_page_2.no_mr
resource.copy_to_ram_on_page_2.mr:
    ; 1: copy resource from megarom to ram on page 3
    call resource.open_and_get_address    ; out: hl = resource data, a = resource segment, bc = resource size
    ld de, (FONTADDR)
    push de
    push bc
      ldir
      call resource.close
    ; 2: unpack from page 3 to 2
      call select_ram_on_page_2
    pop bc
    pop hl
    ld de, 0x8000
    jp resource.ram.unpack

resource.copy_to_ram_on_page_2.no_mr:
  call select_ram_on_page_2
  call resource.open_and_get_address            ; out: hl = resource data, a = resource segment, bc = resource size
resource.copy_to_ram_on_page_2.end:
  ld de, 0x8000
resource.copy_to_ram.unpack_and_close:
  call resource.ram.unpack
  push hl
  push bc
    call resource.close
  pop bc
  pop hl
  ret

resource.copy_to_ram_on_page_3:
  di
    call resource.open_and_get_address          ; out: hl = resource data, a = resource segment, bc = resource size
    ld de, (FONTADDR)
    call resource.copy_to_ram.unpack_and_close  ; out: hl = resource data, bc = resource size
  ei
  ret

select_ram_on_page_0:
  ld a, (RAMAD0)
  ld h,000h
  jp SUB_ENASLT		; Select RAM on page 0 (with interruptions disabled)

select_rom_on_page_2:
  ld a, (SLTAD2)
  ld h,080h
  call ENASLT	    ; Select the ROM on page 8000h-BFFFh (and disable interruptions)

  ld a, (MAPPER)
  bit 0, a          ; mapper on?
  ret z             ; return if no mapper

  ld a, (MPRAD2)    ; original memory mapper segment for page 2 rom
  out (0xFE), a
  ret

select_ram_on_page_2:
  ld a, (RAMAD2)
  ld h,080h
  call ENASLT	    ; Select the RAM on page 8000h-BFFFh

  ld a, (MAPPER)
  bit 0, a          ; mapper on?
  ret z             ; return if no mapper

  ld a, (MPRAD2N)   ; new memory mapper segment for page 2 ram
  out (0xFE), a
  ret

; h = memory page
; a <- slot ID formatted FxxxSSPP
; Modifies: af, bc, de, hl
; ref: https://www.msx.org/forum/msx-talk/development/fusion-c-and-htimi#comment-366469
page.getslt:
	call RSLREG
	bit 7,h
	jr z,PrimaryShiftContinue
	rrca
	rrca
	rrca
	rrca
PrimaryShiftContinue:
	bit 6,h
	jr z,PrimaryShiftDone
	rrca
	rrca
PrimaryShiftDone:
	and 00000011B
	ld c,a
	ld b,0
	ex de,hl
	ld hl,EXPTBL
	add hl,bc
	ld c,a
	ld a,(hl)
	and 80H
	or c
	ld c,a
	inc hl  ; move to SLTTBL
	inc hl
	inc hl
	inc hl
	ld a,(hl)
	ex de,hl
	bit 7,h
	jr z,SecondaryShiftContinue
	rrca
	rrca
	rrca
	rrca
SecondaryShiftContinue:
	bit 6,h
	jr nz,SecondaryShiftDone
	rlca
	rlca
SecondaryShiftDone:
	and 00001100B
	or c
	ret

; verify default slots and memory ram
verify.slots:
    ld h, 0x00          ; page 0
    call page.getslt
    ld (SLTAD0), a

    ld h, 0x40          ; page 1
    call page.getslt
    ld (SLTAD1), a

    ld h, 0x80          ; page 2
    call page.getslt
    ld (SLTAD2), a
    ld (RAMAD2), a      ; presume RAM on page 2

    ld h, 0xC0          ; page 3
    call page.getslt
    ld (SLTAD3), a

    ld (RAMAD0), a      ; presume same slot/subslot of page 3 as RAM for pages left
    ld (RAMAD1), a
    ld (RAMAD3), a

verify.slots.test:
    xor a
    ld (MAPPER), a      ; default = mapper off, no running on RAM

    ld h, 0x41          ; test if running on RAM (SofaRun/ExecRom)
    call verify.slots.test.ram
    jp z, verify.slots.set.running.on.ram

      ld a, (SLTSTR)
      ld (SLTAD2), a    ; slot on page 2 must be the same as page 1

      ld h, 0x81        ; test RAM on page 2
      call verify.slots.test.ram
      jr z, verify.slots.test.mapper.mirror

        ld a, 0xFF
        ld (RAMAD0), a  ; no RAM on page 0
        ld (RAMAD1), a  ; no RAM on page 1
        ld (RAMAD2), a  ; no RAM on page 2
        ret

verify.slots.test.mapper.mirror:
      ld h, 0xD1        ; page 3 memory address
      ld c, (hl)        ; save original value on page 3
      ld a, 65
      ld (hl), a        ; change value on page 3 memory address
      ld h, 0x91        ; page 2 memory address
      cp (hl)           ; test page 2 value against page 3
      ld h, 0xD1
      ld (hl), c        ; restore original value on page 3

      ret nz            ; no RAM mirrored (pages 2 with 3)

      ; else, set mapper in page 2

verify.slots.set.mapper:
    ld a, (MAPPER)
    set 0, a            ; set bit 0 = mapper on
    ld (MAPPER), a

    in a, (0xFF)        ; get memory mapper segment on page 3
    cpl
    ld c, a             ; discard higher bits
    srl a
    or c
    srl a
    or c
    srl a
    or c
    ld c, a

    in a, (0xFC)        ; get memory mapper segment on page 0
    and c
    ld (MPRAD0), a

    in a, (0xFE)        ; get memory mapper segment on page 2
    and c
    ld (MPRAD2), a

    cp 0x0F
    jr z, verify.slots.set.mapper.1
       ld a, 0x10       ; force segment 15 as temporary RAM (presume 256k mapper)

verify.slots.set.mapper.1:
    dec a
    ld (MPRAD2N), a     ; new memory mapper segment on page2

    ret

verify.slots.set.running.on.ram:
    ld a, (MAPPER)
    set 1, a            ; set bit 1 = running on ram
    ld (MAPPER), a

    ld a, (SLTAD1)      ; get slot/subslot from page 1 (ram)
    ld (RAMAD0), a      ; presume RAM on page 0 in same slot/subslot as page 1 (ram)

verify.slots.test.mapper:       ; test mapper on page 2
    call verify.slots.set.mapper

    in a, (0x8E)        ; MegaROM mode 1 - block change off

    ld h, 0x80
    ld c, (hl)          ; save original value
    ld a, 65
    ld (hl), a

    ld a, (MPRAD2N)     ; new memory mapper segment for page 2 ram
    out (0xFE), a

    ld a, 65
    cp (hl)

    ld a, (MPRAD2)      ; original memory mapper segment for page 2 rom
    out (0xFE), a

    ld (hl), c          ; restore original value

    out (0x8E), a       ; MegaROM mode 0 - block change on

    jr nz, verify.slots.test.program.on_page_2   ; has mapper, so test if program allocated...

      ld a, (MAPPER)
      res 0, a            ; reset bit 0 = mapper off
      ld (MAPPER), a

verify.slots.test.megaram:      ; test megaram on page 2
    ; change page 2 to RAM slot....
    ld a, (RAMAD2)
    ld h, 0x80
    call SUB_ENASLT

    ld a, (Seg_P8000_SW)     ; save original byte there
    ld (TEMP), a

    ; change page 2 slot to same slot of page 1
    ld a, (SLTAD1)
    ld h, 0x80
    call SUB_ENASLT

    ; change page 2 block to 0 (presume megarom mode 0 - block change on)
    xor a
    ld (Seg_P8000_SW), a     ; change block

    ; get data value on block
    ld hl, 0x8010
    ld a, (hl)

    ; compare data of page 2 against same address on page 1
    ld h, 0x40
    cp (hl)

    ; jump if not the same
    jr nz, verify.slots.test.megaram.nope
        ; else, change page 2 to block 2
        ld a, 2
        ld (Seg_P8000_SW), a
        ; set page 2 slot to same slot of page 1 and return
        ld a, (SLTAD1)
        ld (SLTAD2), a
        ret

verify.slots.test.megaram.nope:
    ld a, (RAMAD2)              ; return page 2 to RAM slot....
    ld h, 0x80
    call SUB_ENASLT

    ld a, (TEMP)
    ld (Seg_P8000_SW), a        ; restore original byte in RAM

    ld a, 0xFF
    ld (RAMAD2), a              ; RAM on page 2 already used (not usable)

    ret

verify.slots.test.ram:
    in a, (0x8E)        ; MegaROM mode 1 - block change off
    ld c, (hl)          ; save original value
    ld a, 65
    ld (hl), a
    cp (hl)             ; test if value was changed in address
    ld (hl), c          ; restore original value
    out (0x8E), a       ; MegaROM mode 0 - block change on
    ret

verify.slots.test.program.on_page_2:
    xor a
    ld hl, (0x8010)
    ld de, 0x273E       ; ld a, 39 (on 0x8010)
    sbc hl, de
    ret z

    ; set page 2 slot to same slot of page 1 and return
    ld a, (SLTAD1)
    ld (SLTAD2), a
    ret

;---------------------------------------------------------------------------------------------------------
; ENASLT alternative for use of ram on page 0 (with interruptions disabled)
;---------------------------------------------------------------------------------------------------------

D.FFFF:  EQU     0FFFFH  ; secundairy slotregister

SUB_ENASLT:
        CALL    ENASLT.MASK             ; calculate masks
        JP      M,ENASLT.EXPSLT         ; expanded slot
        IN      A,(0A8H)
        AND     C
        OR      B
        OUT     (0A8H),A
        RET

ENASLT.EXPSLT:
        PUSH    HL
        CALL    ENASLT.SUBSLT           ; adjust secundairy slotregister
        LD      C,A
        LD      B,00H
        LD      A,L
        AND     H
        OR      D
        LD      HL,SLTTBL
        ADD     HL,BC
        LD      (HL),A                  ; update SLTTBL
        POP     HL
        LD      A,C
        JR      SUB_ENASLT              ; do a ENASLT on only the primary slot

ENASLT.MASK:
        DI
        PUSH    AF
        LD      A,H
        RLCA
        RLCA
        AND     03H
        LD      E,A                     ; page
        LD      A,0C0H
ENASLT.MASK.1:
        RLCA
        RLCA
        DEC     E
        JP      P,ENASLT.MASK.1
        LD      E,A                     ; page select mask
        CPL
        LD      C,A                     ; page clear mask
        POP     AF
        PUSH    AF
        AND     03H
        INC     A
        LD      B,A
        LD      A,0ABH
ENASLT.MASK.2:
        ADD     A,55H
        DJNZ    ENASLT.MASK.2
        LD      D,A                     ; PPPPPPPP
        AND     E
        LD      B,A                     ; prim. slot select
        POP     AF
        AND     A
        RET

ENASLT.SUBSLT:
        PUSH    AF
        LD      A,D
        AND     0C0H
        LD      C,A
        POP     AF
        PUSH    AF
        LD      D,A
        IN      A,(0A8H)
        LD      B,A
        AND     3FH
        OR      C
        OUT     (0A8H),A                ; set page 3 to requested primary slot (to access sec. slotreg)
        LD      A,D
        RRCA
        RRCA
        AND     03H
        LD      D,A
        LD      A,0ABH
ENASLT.SUBSLT.1:
        ADD     A,55H
        DEC     D
        JP      P,ENASLT.SUBSLT.1
        AND     E
        LD      D,A
        LD      A,E
        CPL
        LD      H,A
        LD      A,(D.FFFF)
        CPL
        LD      L,A
        AND     H
        OR      D
        LD      (D.FFFF),A
        LD      A,B
        OUT     (0A8H),A                ; restore primary slotreg
        POP     AF
        AND     03H
        RET

;---------------------------------------------------------------------------------------------------------
; RAM to VRAM alternative (needs DI/EI)
; in: BC=Length, dest DE=VRAM address, source HL=RAM address
;---------------------------------------------------------------------------------------------------------

SUB_LDIRVM:
  ex      de,hl
  ld a,l           ; initialize (msx1)
  out     (099H),a
  ld      a,h
  and     03FH
  or      040H
  out     (099H),a
SUB_LDIRVM.loop:
  ld      a,(de)
  out     (098H),a
  inc     de
  dec     bc
  ld      a,c
  or      b
  jr      nz, SUB_LDIRVM.loop
  ret

;---------------------------------------------------------------------------------------------------------
; Detect VDP version
;
; a <- 0: TMS9918A, 1: V9938, 2: V9958, x: VDP ID
; f <- z: TMS9918A, nz: other
;---------------------------------------------------------------------------------------------------------

VDP_GetVersion:
    call VDP_IsTMS9918A  ; use a different way to detect TMS9918A
    ret z
    ld a,1               ; select s#1
    di
    out (99H),a
    ld a,15 + 128
    out (99H),a
    in a,(99H)           ; read s#1
    and 00111110B        ; get VDP ID
    rrca
    ex af,af'
    xor a                ; select s#0 as required by BIOS
    out (99H),a
    ld a,15 + 128
    ei
    out (99H),a
    ex af,af'
    ret nz               ; return VDP ID for V9958 or higher
    inc a                ; return 1 for V9938
    ret

; VDP wait
;VDP_wait:
;    ld a, 2
;    call VDP_wait.1
;    and 1
;    jr nz, VDP_wait
;    xor a
;VDP_wait.1:
;    di
;      out (0x99), a
;      ld a, 0x8F
;      out (0x99), a
;      in a, (0x99)
;    ei
;    ret


;
; Test if the VDP is a TMS9918A.
;
; f <- z: TMS9918A, nz: V99X8
;
VDP_IsTMS9918A:
    in a,(99H)           ; read s#0, make sure interrupt flag is reset
    di
VDP_IsTMS9918A_Wait:
    in a,(99H)           ; read s#0
    and a                ; wait until interrupt flag is set
    jp p,VDP_IsTMS9918A_Wait
    ld a,2               ; select s#2 on V9938
    out (99H),a
    ld a,15 + 128        ; (this mirrors to r#7 on TMS9918 VDPs)
    out (99H),a
    in a,(99H)           ; read s#2 / s#0
    ex af,af'
    xor a                ; select s#0 as required by BIOS
    out (99H),a
    ld a,15 + 128
    out (99H),a
    ld a,(0F3E6H)
    out (99H),a          ; restore r#7 if it mirrored (small flash visible)
    ld a,7 + 128
    ei
    out (99H),a
    ex af,af'
    and 01000000B        ; check if bit 6 was 0 (s#0 5S) or 1 (s#2 VR)
    ret

;---------------------------------------------------------------------------------------------------------
; SPRITE COLLISION SUPPORT ROUTINES
; reference:
; https://www.msx.org/forum/development/msx-development/sprite-collision-detection-or-manually-calculation-sprite-coordina
;---------------------------------------------------------------------------------------------------------

;
; customized clear sprite
;
SUB_CLRSPR:
  call SUB_GetSpriteSize
  ld (SPRSIZ), a
  ld hl, SPRTBL
  xor a
  ld (hl), a
  ld e, l
  ld d, h
  inc de
  ld bc, 32*5 - 1    ; 32 sprites * (test, x0, x1, y0, y1)
  ldir
  jp CLRSPR

;
; get sprite XY
; in a = sprite number
; out hl = sprite data address
;
SUB_GETSPRTBL_ADDR:
  push de
    ld e, a
    add a, a              ; a * 4
    add a, a
    add a, e              ; + a
    ld h, 0
    ld l, a
    ld de, SPRTBL
    add hl, de
  pop de
  ret

;
; set sprite XY
; a = sprite number
; ix = x
; iy = y
;
SUB_SETSPRTBL_XY:
  push ix
  push iy
  exx
    ld c, a              ; save a
      call SUB_GETSPRTBL_ADDR
      ld a, 0xff
      ld (hl), a         ; test flag (0=notest, ff=test)
      inc hl
      pop de             ; de = ix
      ld (hl), e         ; x0
      inc hl
      ld a, (SPRSIZ)
      ld d, a            ; save a
        add a, e
        ld (hl), a       ; x1 = x0 + sprite size
        inc hl
      ld a, d            ; restore a
      pop de             ; de = iy
      ld (hl), e         ; y0
      inc hl
      add a, e
      ld (hl), a         ; y1 = y0 + sprite size
    ld a, c              ; restore a
  exx
  ret

;
; set sprite test flag
; a = sprite number
; l = flag
;
SUB_SETSPRTBL_TEST:
  ex de, hl
  call SUB_GETSPRTBL_ADDR
  ld (hl), e         ; set test flag (0=notest, ff=test)
  ret

;
; load sprite data
; in a = sprite number
;
SUB_SPRCOL_LOAD:
  call SUB_GETSPRTBL_ADDR
  ld a, (hl)
  or a                   ; sprite is active?
  ret z

  inc hl
  ld e, (hl)             ; ax0
  inc hl
  ld d, (hl)             ; ax1
  inc hl
  ld c, (hl)             ; ay0
  inc hl
  ld b, (hl)             ; ay1
  ret

;
; test a sprite collision against all the rest
; out a (0xFF = no sprite, or collisor sprite)
;
SUB_SPRCOL_ALL:
  xor a

SUB_SPRCOL_ALL.LOOP:
  exx
    ld b, a       ; save current sprite
  exx
  call SUB_SPRCOL_ONE.1
  cp 0xFF
  ret nz

  exx
    ld a, b       ; restore current sprite
  exx
  inc a
  cp 32
  jr nz, SUB_SPRCOL_ALL.LOOP

  jr SUB_SPRCOL_CHECK.false

;
; test a sprite collision against all the rest
; in  hl = sprite
; out a (0xFF = no, or sprite 1 if collided)
;
SUB_SPRCOL_ONE:
  ld a, l         ; parameter sprite
  exx
    ld b, a       ; save parameter sprite
  exx
SUB_SPRCOL_ONE.1:
  call SUB_SPRCOL_LOAD
  jr z, SUB_SPRCOL_CHECK.false

  xor a
  call SUB_GETSPRTBL_ADDR

SUB_SPRCOL_ONE.LOOP:
  exx
    cp b          ; compare current sprite against parameter sprite
  exx
  jr z, SUB_SPRCOL_ONE.SKIP    ; skip if same sprite
    exx
      ld c, a                  ; save current sprite
    exx
    call SUB_SPRCOL_CHECK
    cp 0xFF
    ret nz

    exx
      ld a, c                  ; restore current sprite
    exx
    jr SUB_SPRCOL_ONE.CONT

SUB_SPRCOL_ONE.SKIP:
  inc hl
  inc hl
  inc hl
  inc hl
  inc hl

SUB_SPRCOL_ONE.CONT:
  inc a
  cp 32
  jr nz, SUB_SPRCOL_ONE.LOOP

  jr SUB_SPRCOL_CHECK.false

;
; test two sprites collision
; in  hl = sprite 1
;     de = sprite 2
; out a (0xFF = no, or sprite 1 if collided)
;
SUB_SPRCOL_COUPLE:
  ; save sprite 2
  ld a, l
  exx
    ld c, a
  exx
  ; sprite 1
  ld a, e
  call SUB_SPRCOL_LOAD
  jr z, SUB_SPRCOL_CHECK.false

  ; restore sprite 2
  exx
    ld a, c
  exx
  call SUB_GETSPRTBL_ADDR

; e=ax0, d=ax1, c=ay0, b=ay1
SUB_SPRCOL_CHECK:
  ; test if sprite is active
  ld a, (hl)
  or a                   ; 0xFF = active
  jr z, SUB_SPRCOL_CHECK.skip_5
  inc hl

  ; test if ax1 > bx0 and ax0 < bx1 and ay1 > by0 and ay0 < by1
  ld a, (hl) ; bx0
  cp d       ; ax1
  jr nc, SUB_SPRCOL_CHECK.skip_4
  inc hl

  ld a, e    ; ax0
  cp (hl)    ; bx1
  jr nc, SUB_SPRCOL_CHECK.skip_3
  inc hl

  ld a, (hl) ; by0
  cp b       ; ay1
  jr nc, SUB_SPRCOL_CHECK.skip_2
  inc hl

  ld a, c    ; ay0
  cp (hl)    ; by1
  jr nc, SUB_SPRCOL_CHECK.skip_1
  inc hl

SUB_SPRCOL_CHECK.true:
  ; if ok, return collider sprite (c)
  exx
    ld a, c
  exx
  ret

SUB_SPRCOL_CHECK.skip_5:
  inc hl
SUB_SPRCOL_CHECK.skip_4:
  inc hl
SUB_SPRCOL_CHECK.skip_3:
  inc hl
SUB_SPRCOL_CHECK.skip_2:
  inc hl
SUB_SPRCOL_CHECK.skip_1:
  inc hl

SUB_SPRCOL_CHECK.false:
  ld a, 0xFF
  ret

;
; get sprite size
;
SUB_GetSpriteSize:
  push bc
    ld bc, 0x0808
    ld a, (RG1SAV)  		; bit 0 = double size, bit 1 = sprite size (0=8 pixels, 1=16 pixels)
    bit 1, a
    jr z, SUB_GetSpriteSize.1
      ld bc, 0x1010

SUB_GetSpriteSize.1:
    bit 0, a
    jr z, SUB_GetSpriteSize.2
      sll b

SUB_GetSpriteSize.2:
    ;ld (ARG), bc
    ld a, c
  pop bc
  ret

; Entry:	C = block number (bits 5-4) and register (bits 3-0) to read.
; Output:	A = 4 least significant bits content of the register read.
SUB_REDCLK:
  ld ix,REDCLK
  jp EXTROM

; Entry:	C = block number (bits 5-4) and register (bits 3-0).
;           A = data to write. (4 least significant bits)
SUB_WRTCLK:
  ld ix,WRTCLK
  jp EXTROM

;---------------------------------------------------------------------------------------------------------
; SUPPORT ROUTINES
;---------------------------------------------------------------------------------------------------------

; calculate text screen location
; in: HL = xy (bios based coord system where home = 1,1)
; out: HL = vram address
gfxTileAddress:
  ld a, (VERSION)
  and a
  jp z, CHCOORD
  jp CHCOORD2

; in: A=Sprite pattern number
; out: HL=Sprite pattern address
gfxCALPAT:
  ld iy, SCRMOD
  ld ix, CALPAT2
  bit 3, (iy)
  jp nz, SUB_EXTROM
  bit 2, (iy)
  jp nz, SUB_EXTROM
  jp CALPAT

; in: A=Sprite number
; out: HL=Sprite attribute address
gfxCALATR:
  ld iy, SCRMOD
  ld ix, CALATR2
  bit 3, (iy)
  jp nz, SUB_EXTROM
  bit 2, (iy)
  jp nz, SUB_EXTROM
  jp CALATR

; in:  A = sprite number
; out: HL = address to color table
gfxCALCOL:
  push af
  push de
    ld h, 0
    ld l, a         ; recover sprite number
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl      ; multiply by 16 (shift left 4)
    push hl
      xor a
      call gfxCALATR    ; get sprite attribute table address
    pop de
    add hl, de
    xor a
    ld de, 512
    sbc hl, de      ; address of color table from sprite multicolor
  pop de
  pop af
  ret

; hl = register
; bc = parameter
gfxVDP.set:
  ld b, c
  ld a, l
  cp 0x08
  adc a, 0xff       ; dec a if register >= 8 (so, a = a + (-1) + carry)
  ld c, a
  jp WRTVDP

  ;ld a, (VERSION)
  ;or a
  ;jp z, WRTVDP      ; b = data, c = register
  ;ld ix, NWRVDP
  ;jp SUB_EXTROM

