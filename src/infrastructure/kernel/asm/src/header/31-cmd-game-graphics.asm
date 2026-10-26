; ------------------------------------------------------------------------------------------------------
; STATEMENTS / FUNCTIONS IMPLEMENTATIONS
; ------------------------------------------------------------------------------------------------------

; play resource with Basic standard statement
; CMD PLAY <resource number> [, <channel C: 0=off|1=on>]
cmd_play:
  ld bc, (DAC)             ; bc = resource number

  call resource.get_data

  ld a, (ARG)
  and 1
  add a, a
  add a, a
  ld c, a
  push bc
    ld ix, (PLAY)
    call CALBAS
  pop bc
  di
    ld a, (MUSICF)
    and 3                  ; disable channel C
    or c                   ; enable/disable channel C
    ld (MUSICF), a
    ld a, 0xFF
    ld (QUETAB + 12), a
    ld (QUETAB + 13), a    ; clear channel C buffer index (in and out)
  ei
  xor a
  ret

; mute PSG
; CMD MUTE
; Calling GICINI twice as a workaround to fix the "envelope period reset bug" during PSG initialization.
; Because of a race condition in the MSX BIOS, the first call triggers the envelope (initializes PSG and buffers), 
; while the second resets the envelope period properly (ensures envelope period resets).
cmd_mute:
  halt
  di
    call GICINI
  ei
  jp GICINI  ; "envelope period reset bug" fix

; draw resource with Basic standard statement
; CMD DRAW <resource number>
cmd_draw:
  ld bc, (DAC)             ; bc = resource number
  call resource.get_data
  ld ix, (DRAW)
  call CALBAS
  ei
  xor a
  ret

; exec as assembly code
; CMD RUNASM <resource number>
cmd_runasm:
  ld bc, (DAC)             ; bc = resource number
  call resource.get_data
  push hl
  ret                      ; indirect call

; write resource to vram address
; CMD WRTVRAM <resource number>, <vram address>
cmd_wrtvram:
  ld a, (RAMAD2)           ; test RAM on page 2
  cp 0xFF
  jr nz, cmd_wrtvram.ram_on_page_2

cmd_wrtvram.ram_on_page_3:
    call resource.copy_to_ram_on_page_3     ; out: hl = resource data, bc = size
    ld de, (ARG)
    jp LDIRVM

cmd_wrtvram.ram_on_page_2:
  di
    call resource.copy_to_ram_on_page_2
    ld de, (ARG)
    call SUB_LDIRVM
    call select_rom_on_page_2
  ei                                        ; its important because ENASLT above (select_rom_on_page_2) has interruptions disabled
  ret

; write font resource to vram tile pattern table or graphical mode
; CMD WRTFNT <resource number>
cmd_wrtfnt:
  call resource.copy_to_ram_on_page_3   ; out: hl = resource data, bc = size

  ld de, 7
  add hl, de
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc

  ld a, (SOMODE)
  cp 1                     ; graphical screen output mode?
  jr z, cmd_wrtfnt.no_tiled

cmd_wrtfnt.tiled:
  push hl
    call cmd_setfnt.get_vram_address
    ex de, hl
  pop hl

  jp LDIRVM

cmd_wrtfnt.no_tiled:
  ld a, (RAMAD3)
  ld (CGPNTSLT), a
  ld (CGPNT), hl
  ret

; write resource to vram tile pattern table
; CMD WRTCHR <resource number>
cmd_wrtchr:
  ld a, 2                  ; tiled screen output mode
  ld (SOMODE), a
  ld bc, (GRPCGP)
  ld (ARG), bc
  call cmd_wrtvram
  jp cmd_updfntclr         ; set default color to font range on all bank numbers

; write resource to vram tile color table
; CMD WRTCLR <resource number>
cmd_wrtclr:
  ld a, 2                  ; tiled screen output mode
  ld (SOMODE), a
  ld bc, (GRPCOL)
  ld (ARG), bc
  jp cmd_wrtvram

; write resource to vram tile screen table
; CMD WRTSCR <resource number>
cmd_wrtscr:
  ld a, 2                  ; tiled screen output mode
  ld (SOMODE), a
  ld bc, (GRPNAM)
  ld (ARG), bc
  jp cmd_wrtvram

; write a Tiny Sprite resource to vram sprite pattern and color table
; CMD WRTSPR <resource number>
WS.SPRITE.COUNT: EQU DAC              ; sprite count
WS.SPRITE.COLOR_TABLE_SIZE: EQU ARG   ; sprite color table size
cmd_wrtspr:
  push hl
    call SUB_CLRSPR
  pop hl
  call resource.copy_to_ram_on_page_3   ; out: hl = resource data, bc = size

cmd_wrtspr.do:
  ld a, (hl)
  inc hl
  or a
  jr nz, cmd_wrtspr.do.msx2

cmd_wrtspr.do.msx1:
  ld a, (SCRMOD)
  cp 4
  ret nc
  call cmd_wrtspr.do.pattern
  push hl
    xor a
    call gfxCALATR
    inc hl  ; from x to y field
    inc hl  ; from y to pattern field
    inc hl  ; from pattern to color field
  pop de
  ld a, (WS.SPRITE.COUNT)
  cp 32
  jr c, cmd_wrtspr.do.msx1.ok
  jr z, cmd_wrtspr.do.msx1.ok
  ld a, 32
cmd_wrtspr.do.msx1.ok:
cmd_wrtspr.do.msx1.loop:
  push af
    ld a, (de)  ; copy sprite color to attribute table
    call WRTVRM
    inc hl  ; from color to next x field
    inc hl  ; from x to y field
    inc hl  ; from y to pattern field
    inc hl  ; from pattern to color field
    inc de
  pop af
  dec a
  jr nz, cmd_wrtspr.do.msx1.loop
  ret

cmd_wrtspr.do.msx2:
  ld a, (SCRMOD)
  cp 4
  ret c
  call cmd_wrtspr.do.pattern
cmd_wrtspr.do.msx2.color:
  push hl
    xor a
    call gfxCALCOL
    ex de, hl
    ld bc, (WS.SPRITE.COLOR_TABLE_SIZE)  ; sprite color table size
  pop hl
  jp LDIRVM      ; hl = ram data address, de = vram data address, bc = length

cmd_wrtspr.do.pattern:
  ld a, (hl)
  cp 64
  jr c, cmd_wrtspr.do.pattern.ok
  jr z, cmd_wrtspr.do.pattern.ok
  ld a, 64
cmd_wrtspr.do.pattern.ok:
  inc hl
  push hl
    ld l, a
    ld h, 0
    ld (WS.SPRITE.COUNT), hl         ; sprite count
    add hl, hl  ; x2
    add hl, hl  ; x4
    add hl, hl  ; x8
    add hl, hl  ; x16
    call cmd_wrtspr.set_color_table_size
    add hl, hl  ; x32
    push hl
      xor a
      call gfxCALPAT
      ex de, hl
    pop bc
  pop hl
  push hl
  push bc
    call LDIRVM      ; hl = ram data address, de = vram data address, bc = length
  pop hl
  pop bc
  add hl, bc
  ret

cmd_wrtspr.set_color_table_size:
  ld (WS.SPRITE.COLOR_TABLE_SIZE), hl         ; sprite color table size
  ld a, (WS.SPRITE.COUNT)
  cp 32
  ret c
  ld bc, 0x200
  ld (WS.SPRITE.COLOR_TABLE_SIZE), bc
  ret

; write resource to vram sprite pattern table
; CMD WRTSPRPAT <resource number>
cmd_wrtsprpat:
  xor a
  call gfxCALPAT
  ld (ARG), hl
  jp cmd_wrtvram

; write resource to vram sprite color table
; CMD WRTSPRCLR <resource number>
cmd_wrtsprclr:
  xor a
  call gfxCALCOL
  ld (ARG), hl
  jp cmd_wrtvram

; write resource to vram sprite attribute table
; CMD WRTSPRATR <resource number>
cmd_wrtspratr:
  xor a
  call gfxCALATR
  ld (ARG), hl
  jp cmd_wrtvram

; write ram to vram address
; CMD RAMTOVRAM <ram address>, <vram address>, <size>
cmd_ramtovram:
  ld hl, (DAC)
  ld de, (ARG)
  ld bc, (ARG+2)
  jp LDIRVM      ; hl = ram data address, de = vram data address, bc = length

; write vram to ram address
; CMD VRAMTORAM <vram address>, <ram address>, <size>
cmd_vramtoram:
  ld hl, (DAC)
  ld de, (ARG)
  ld bc, (ARG+2)
  jp LDIRMV      ; de = ram data address, hl = vram data address, bc = length

; copy ram to ram address
; CMD RAMTORAM <ram source address>, <ram dest address>, <size>
cmd_ramtoram:
  ld hl, (DAC)
  ld de, (ARG)
  ld bc, (ARG+2)
  ldir      ; hl = source ram address, de = dest ram address, bc = length
  ret

; cmd_rsctoram moved to the fixed 31-cmd-rsctoram module.
; This filler preserves the fixed layout (player.int.control / ARKOS_PLAYER).
    DEFS 26, 0x00

; disable screen
cmd_disscr:
  jp DISSCR

; enable screen
cmd_enascr:
  jp ENASCR

cmd_keyclkoff:
  xor a
  ld (CLIKSW), a
  ret

; enable built in fonts
; CMD SETFNT <number> [,<bank>]
cmd_setfnt:
  ld a, (SCRMOD)
  or a
  ret z                         ; no default fonts to screen 0

  ld a, (DAC)                   ; a = font number
  or a                          ; 0
  jr z, cmd_setfnt.bios

  ld hl, font.resource.data.1
  dec a                         ; 1
  jr z, cmd_setfnt.unpack

  ld hl, font.resource.data.2
  dec a                         ; 2
  jr z, cmd_setfnt.unpack

  ld hl, font.resource.data.3   ; 3

cmd_setfnt.unpack:
  ld de, (FONTADDR)
  call resource.ram.unpack

  ld a, (RAMAD3)
  ld (CGPNTSLT), a

  jr cmd_setfnt.do

cmd_setfnt.bios:
  ld a, (FONTOLDSLT)
  ld (CGPNTSLT), a
  ld hl, (FONTOLD)
  ld (CGPNT), hl

  ex de, hl
  ld bc, 95 * 8                   ; bytes to be copied (chars * 8)
  ld hl, 32 * 8                   ; pattern generator in bios start position * 8
  add hl, de                      ; ram font source in bios

cmd_setfnt.do:
  ld a, (SOMODE)
  cp 1                            ; graphical screen output mode?
  jr z, cmd_setfnt.no_tiled

cmd_setfnt.tiled:
  ld a, (ARG)                     ; bank number (0xFF = all)
  push af
    call cmd_setfnt.cpy_to_bank   ; hl=source address, a=bank number in vram
  pop af
  jp cmd_setfnt.default_colors

cmd_setfnt.no_tiled:
  xor a
  ld bc, 32 * 8
  sbc hl, bc
  ld (CGPNT), hl
  ret

; a = bank number (0xFF = all)
; hl = ram address
cmd_setfnt.cpy_to_bank:
  cp 0xFF
  jr nz, cmd_setfnt.cpy_to_bank.do
     push hl
       xor a
       call cmd_setfnt.cpy_to_bank.do
     pop hl
     push hl
       ld a, 1
       call cmd_setfnt.cpy_to_bank.do
     pop hl
     ld a, 2

cmd_setfnt.cpy_to_bank.do:
  push hl
    call cmd_setfnt.get_vram_address
    ld de, 32 * 8    ; pattern generator in vram start position * 8
    add hl, de
    or a
    jr z, cmd_setfnt.cpy_to_bank.cont
      ld de, 0x0800
      add hl, de         ; next bank
      cp 1
      jr z, cmd_setfnt.cpy_to_bank.cont
        add hl, de       ; next bank
cmd_setfnt.cpy_to_bank.cont:
    ex de, hl
  pop hl
  ld bc, 95 * 8    ; bytes to be copied (chars * 8)
  jp LDIRVM

; out hl = pattern generator vram address
cmd_setfnt.get_vram_address:
  push af
    ld hl, (GRPCGP)
    ld a, (SCRMOD)
    cp 2
    jr nc, cmd_setfnt.get_address.exit
    ld hl, (T32CGP)
    cp 1
    jr z, cmd_setfnt.get_address.exit
    ld hl, (TXTCGP)
cmd_setfnt.get_address.exit:
  pop af
  ret

; a = bank number (0xFF = all)
cmd_setfnt.default_colors:
  ld (ARG), a
  ld a, (SCRMOD)
  cp 4
  jr z, cmd_setfnt.default_colors.cont
    cp 2
    ret nz
cmd_setfnt.default_colors.cont:
  push hl
  push de
  push bc
    ld bc, (GRPATR)
    ld (ATRBAS), bc
    ld bc, (GRPNAM)
    ld (NAMBAS), bc
    ld bc, (GRPCGP)
    ld (CGPBAS), bc
    ld a, 32  ;31
    ld (LINLEN), a
    ld a, 24
    ld (CRTCNT), a
    ld hl, (GRPCOL)
    ld de, 32*8
    add hl, de
    ld a, (ARG)      ; bank number in vram (0xFF = all)
    cp 0xFF
    jr nz, cmd_setfnt.default_colors.end
      xor a
      call cmd_setfnt.default_colors.0
      ld a, 1
      call cmd_setfnt.default_colors.0
      ld a, 2
cmd_setfnt.default_colors.end:
    call cmd_setfnt.default_colors.0
  pop bc
  pop de
  pop hl
  ret

cmd_setfnt.default_colors.0:
    push hl
      or a
      jr z, cmd_setfnt.default_colors.1
        ld de, 0x0800
        add hl, de
        cp 1
        jr z, cmd_setfnt.default_colors.1
          add hl, de
cmd_setfnt.default_colors.1:
      ld a, (FORCLR)
      add a, a ; x2
      add a, a ; x4
      add a, a ; x8
      add a, a ; x16
      ex de, hl
        ld hl, BAKCLR
        or (hl)
      ex de, hl
      ld bc, 95*8
      call FILVRM
    pop hl
    ret

cmd_updfntclr:
  ld a, 0xFF
  jp cmd_setfnt.default_colors

cmd_clrkey:
  di
    push hl
      call KILBUF
    pop hl
    xor a
    ld (TRGFLG), a   ; clear joysticks
  ei
  ret

cmd_clrscr:
  ld hl, (GRPNAM)
  ld bc, 28*32
  ld a, 32
  jp FILVRM

; DAC = 0 (off), 1 (on)
cmd_turbo:
  ld a, (VERSION)
  cp 2              ; msx2+ or turbo R
  ret c             ; not a turbo capable machine
  jr z, cmd_turbo.msx2p
cmd_turbo.turboR:   ; R800 processor type
  ld a, (CHGCPU)
  cp 0xC3
  ret nz            ; BIOS function CHGCPU doesnt exist
  ld a, (DAC)
  or 0x80           ; update turbo led
  jp CHGCPU
cmd_turbo.msx2p:    ; msx2+ machine type
  ld a, 8
  out (64), a
  ld a, (DAC)
  and 1
  xor 1
  out (65), a
  ret

; DAC = data resource number
cmd_restore:
  di
    call resource.open_and_get_address  ; in: DAC = resource number, out: hl = resource data, a = resource segment, bc = resource size
    ld (DATLIN), hl          ; DATA start pointer
    ld (SUBFLG), a           ; DATA segment start number
    ld (DATPTR), hl          ; DATA current pointer
    ld (DORES), a            ; DATA current segment
    ld bc, 0
  jp XBASIC_RESTORE.cont

; de = tile number
; hl = direction (0=horizontal, 1=vertical, 2=both)
; b  = bank (0-2 specific, 3=all)
