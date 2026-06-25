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

; copy resource to ram address
; CMD RSCTORAM <resource number>, <ram dest address>, <pletter: 0=no, 1=yes>
; hl = resource
; de = ram address
; a = pletter
cmd_rsctoram:
  di
    push de
    push af
      ld (DAC), hl
      call resource.open_and_get_address          ; in: DAC = resource number, out: hl = resource data, a = resource segment, bc = resource size
    pop af
    pop de
    or a
    jr nz, cmd_rsctoram.unpack
      ldir
      jr cmd_rsctoram.end
cmd_rsctoram.unpack:
    call resource.ram.unpack
cmd_rsctoram.end:
    call resource.close
  ei
  ret

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
set_tile_flip:
  ld a, (SCRMOD)
  cp 5
  ret nc                       ; return if screen mode >= 5
  ld a, b
  ld (TEMP2), a                ; save bank
  push hl                      ; save direction
  call get_tile_vram_addr      ; hl = VRAM address (bank 0)
  ld (TEMP), hl                ; save VRAM base
  call set_tile.copy           ; read 8 bytes to STRBUF
  pop hl                       ; restore direction

  ld a, l                      ; direction
  or a
  jr z, set_tile_flip.horiz
  dec a
  jr z, set_tile_flip.vert
  ; direction = 2 (both)
  call set_tile_flip.horiz.do
  call set_tile_flip.vert.do
  jr set_tile_flip.paste

set_tile_flip.horiz:
  call set_tile_flip.horiz.do
  jr set_tile_flip.paste

set_tile_flip.horiz.do:
  ld hl, STRBUF
  ld b, 8
set_tile_flip.horiz.loop:
  ld a, (hl)
  call binaryReverseA
  ld (hl), a
  inc hl
  djnz set_tile_flip.horiz.loop
  ret

set_tile_flip.vert:
  call set_tile_flip.vert.do
  jr set_tile_flip.paste

set_tile_flip.vert.do:
  ld hl, STRBUF
  ld de, STRBUF+7
  ld b, 4
set_tile_flip.vert.loop:
  ld a, (hl)
  ld c, a
  ld a, (de)
  ld (hl), a
  ld a, c
  ld (de), a
  inc hl
  dec de
  djnz set_tile_flip.vert.loop
  ret

set_tile_flip.paste:
  ld a, (TEMP2)                ; restore bank
  ld b, a
  jp set_tile.paste

; de = tile number
; hl = direction (0=left, 1=right, 2=180 degrees)
; b  = bank (0-2 specific, 3=all)
set_tile_rotate:
  ld a, (SCRMOD)
  cp 5
  ret nc                       ; return if screen mode >= 5
  ld a, b
  ld (TEMP2), a                ; save bank
  push hl                      ; save direction
  call get_tile_vram_addr      ; hl = VRAM address (bank 0)
  ld (TEMP), hl                ; save VRAM base
  call set_tile.copy           ; read 8 bytes to STRBUF
  pop hl                       ; restore direction

  ld a, l                      ; direction
  cp 2
  jr z, set_tile_rotate.flip   ; 180 degrees = flip both

  or a
  jr z, set_tile_rotate.left

set_tile_rotate.right:
  call set_tile.backup_rotate
  call blockRotateR
  jr set_tile_rotate.paste

set_tile_rotate.flip:
  call set_tile_flip.horiz.do
  call set_tile_flip.vert.do
  jr set_tile_rotate.paste

set_tile_rotate.left:
  call set_tile.backup_rotate
  call blockRotateL

set_tile_rotate.paste:
  ld a, (TEMP2)                ; restore bank
  ld b, a

; b = bank (0-2 specific, 3=all)
; (TEMP) = VRAM base address of tile (bank 0)
set_tile.paste:
  ld a, (SCRMOD)
  or a
  jr nz, set_tile.paste.normal
    ; screen 0: write to base address only (single bank)
    ld hl, (TEMP)
    ex de, hl
    ld hl, STRBUF
    ld bc, 8
    jp LDIRVM

set_tile.paste.normal:
  ld a, b
  cp 3
  jr z, set_tile.paste.all

  ; single bank
  ld hl, (TEMP)
  or a
  jr z, set_tile.paste.do_1
  push bc
    ld de, 0x0800
set_tile.paste.bank_loop:
    add hl, de
    djnz set_tile.paste.bank_loop
  pop bc
set_tile.paste.do_1:
  ex de, hl
  ld hl, STRBUF
  ld bc, 8
  jp LDIRVM

set_tile.paste.all:
  ld hl, (TEMP)
  ex de, hl
  ld hl, STRBUF
  ld bc, 8
  push de
    call LDIRVM
  pop hl
  ld de, 0x0800
  add hl, de
  ex de, hl
  ld hl, STRBUF
  ld bc, 8
  push de
    call LDIRVM
  pop hl
  ld de, 0x0800
  add hl, de
  ex de, hl
  ld hl, STRBUF
  ld bc, 8
  jp LDIRVM

; backup STRBUF[0..7] to STRBUF[8..15], HL=STRBUF+8, DE=STRBUF
set_tile.backup_rotate:
  ld hl, STRBUF
  ld de, STRBUF+8
  ld bc, 8
  ldir
  ld de, STRBUF
  ret

; in:  hl = VRAM address
; out: STRBUF = 8-byte tile pattern from VRAM
set_tile.copy:
  ld de, STRBUF
  ld bc, 8
  jp LDIRMV

; in:  de = tile number
; out: hl = VRAM address of tile (bank 0)
get_tile_vram_addr:
  ld l, e
  ld h, d                      ; hl = tile number
  add hl, hl
  add hl, hl
  add hl, hl                   ; hl = tile * 8
  ld a, (SCRMOD)
  or a
  ret nz                       ; screens 1-4: no base offset
  ld de, 0x0800
  add hl, de                   ; screen 0: base at 0x0800
  ret

; hl = tile number
; de = line number
; b = bank number (3=all)
; c = pattern data
set_tile_pattern:
  ld a, (SCRMOD)
  cp 5
  ret nc                       ; return if screen mode >= 5
  or a
  jr nz, set_tile_pattern.skip
    ld d, 0x08                 ; pattern table on screen mode 0
    ld b, 0x00                 ; only bank 0
set_tile_pattern.skip:
  add hl, hl                   ; x2
  add hl, hl                   ; x4
  add hl, hl                   ; x8
  add hl, de                   ; position in pattern table
  ld de, 0x0800
  ld a, b
  cp 3
  jr z, set_tile_pattern.all   ; put on all banks
  or a
  jr z, set_tile_pattern.do    ; put on bank 0
  add hl, de
  dec a
  jr z, set_tile_pattern.do    ; put on bank 1
  add hl, de
  jr set_tile_pattern.do       ; put on bank 2
set_tile_pattern.all:
  call set_tile_pattern.do
  add hl, de
  call set_tile_pattern.do
  add hl, de
set_tile_pattern.do:
  push af
    ld a, c
    call WRTVRM     ; hl = vram address, a = byte to write
  pop af
  ret

; hl = tile number
; de = line number (15=all)
; b = bank number (3=all)
; c = color data (FC,BC)
set_tile_color:
  ld a, (SCRMOD)
  cp 5
  ret nc                       ; return if screen mode >= 5
  or a
  ret z                        ; return if screen mode = 0
  ld d, 0x20
  cp 1
  jr nz, set_tile_color.multi
set_tile_color.uno:
    sra l                      ; /2
    sra l                      ; /4
    sra l                      ; /8
    ld h, 0x20
    jr set_tile_pattern.do
set_tile_color.multi:
  ld a, e
  cp 0xF
  jr nz, set_tile_pattern.skip
  ld a, 7
set_tile_color.multi.loop:
  call set_tile_color.multi.do
  dec a
  jr nz, set_tile_color.multi.loop
set_tile_color.multi.do:
  push af
  push hl
  push de
  push bc
    ld e, a
    call set_tile_pattern.skip
  pop bc
  pop de
  pop hl
  pop af
  ret

; a = tile number
; hl = buffer pointer to an 8 bytes buffer (to fill)
; b = bank number (0-2)
get_tile_pattern:
  push hl                    ; save buffer address
    ld l, a
    ld h, 0                  ; hl = tile number
    ld de, 0                 ; line = 0 (start of tile)
    ld a, (SCRMOD)
    cp 5
    jr nc, get_tile_pattern.exit
    or a
    jr nz, get_tile_pattern.skip
      ld d, 0x08             ; screen 0: pattern table at 0x0800
      ld b, 0x00             ; screen 0: only bank 0
get_tile_pattern.skip:
    add hl, hl
    add hl, hl
    add hl, hl               ; hl = tile * 8
    add hl, de               ; hl = tile*8 + base

    ld a, b
    or a
    jr z, get_tile_pattern.do
    ld de, 0x0800
get_tile_pattern.bank_loop:
    add hl, de
    dec a
    jr nz, get_tile_pattern.bank_loop
get_tile_pattern.do:
    ; hl = VRAM source address
  pop de                     ; de = buffer destination
  ld bc, 8                   ; 8 bytes per tile
  jp LDIRMV

get_tile_pattern.exit:
  pop hl
  ret

; a = tile number
; hl = buffer pointer to an 8 bytes buffer
; b = bank (0-2 specific, 3=all banks)
get_tile_color:
  ld (TEMP8), hl
  ld l, a
  ld h, 0
  ld a, (SCRMOD)
  cp 5
  jr nc, get_tile_color.exit
  or a
  jr z, get_tile_color.exit
  add hl, hl
  add hl, hl
  add hl, hl
  ld de, 0x2000
  add hl, de
  ld a, b
  or a
  jr z, get_tile_color.do
  ld de, 0x0800
get_tile_color.bank_loop:
  add hl, de
  dec a
  jr nz, get_tile_color.bank_loop
get_tile_color.do:
  ld de, (TEMP8)
  ld bc, 8
  call LDIRMV
get_tile_color.exit:
  ret

; a = tile number
; hl = 8-byte color buffer (packed FC<<4|BC per byte)
; b = bank (0-2 specific, 3=all banks)
set_tile_color_buf:
  ld (TEMP8), hl
  ld l, a
  ld h, 0
  ld a, (SCRMOD)
  cp 5
  ret nc
  or a
  ret z
  add hl, hl
  add hl, hl
  add hl, hl
  ld de, 0x2000
  add hl, de
  ld a, b
  cp 3
  jr z, set_tile_color_buf.all
  or a
  jr z, set_tile_color_buf.do
set_tile_color_buf.bank_loop:
  ld de, 0x0800
  add hl, de
  dec a
  jr nz, set_tile_color_buf.bank_loop
set_tile_color_buf.do:
  ex de, hl
  ld hl, (TEMP8)
  ld bc, 8
  jp LDIRVM
set_tile_color_buf.all:
  ex de, hl
  ld hl, (TEMP8)
  ld bc, 8
  push de
    call LDIRVM
  pop hl
  ld de, 0x0800
  add hl, de
  ex de, hl
  ld hl, (TEMP8)
  ld bc, 8
  push de
    call LDIRVM
  pop hl
  ld de, 0x0800
  add hl, de
  ex de, hl
  ld hl, (TEMP8)
  ld bc, 8
  jp LDIRVM

; a = tile number
; hl = 8-byte pattern buffer
; b = bank (0-2 specific, 3=all banks)
set_tile_pattern_buffer:
  ld (TEMP8), hl             ; save buffer address
  ld l, a
  ld h, 0                    ; hl = tile number
  ld de, 0                   ; line = 0 (start of tile)
  ld a, (SCRMOD)
  cp 5
  ret nc
  or a
  jr nz, set_tile_pattern_buffer.skip
    ld d, 0x08               ; screen 0: pattern table at 0x0800
    ld b, 0x00               ; screen 0: only bank 0
set_tile_pattern_buffer.skip:
  add hl, hl
  add hl, hl
  add hl, hl                 ; hl = tile * 8
  add hl, de                 ; hl = tile*8 + base

  ld a, b
  cp 3
  jr z, set_tile_pattern_buffer.all

  ; single bank
  or a
  jr z, set_tile_pattern_buffer.do
  ld de, 0x0800
set_tile_pattern_buffer.bank_loop:
  add hl, de
  dec a
  jr nz, set_tile_pattern_buffer.bank_loop
set_tile_pattern_buffer.do:
  ex de, hl                  ; de = VRAM address
  ld hl, (TEMP8)             ; hl = buffer
  ld bc, 8
  jp LDIRVM

set_tile_pattern_buffer.all:
  ; write to bank 0
  ex de, hl                  ; de = VRAM base (bank 0)
  ld hl, (TEMP8)             ; hl = buffer
  ld bc, 8
  push de                    ; save vram base
    call LDIRVM              ; write bank 0
  pop hl                     ; hl = vram base (bank 0)
  ld de, 0x0800
  add hl, de                 ; hl = vram base + 0x0800 (bank 1)
  ex de, hl                  ; de = vram address (bank 1)
  ld hl, (TEMP8)             ; hl = buffer
  ld bc, 8
  push de                    ; save vram bank 1
    call LDIRVM              ; write bank 1
  pop hl                     ; hl = vram bank 1
  ld de, 0x0800
  add hl, de                 ; hl = vram bank 2
  ex de, hl                  ; de = vram address (bank 2)
  ld hl, (TEMP8)             ; hl = buffer
  ld bc, 8
  jp LDIRVM                  ; write bank 2 and return

; de = sprite number
; hl = direction (0=horizontal, 1=vertical, 2=both)
set_sprite_flip:
  ld (TEMP2), de
  ld a, l
  or a
  jr z, set_sprite_flip.horiz
  dec a
  call nz, set_sprite_flip.horiz

set_sprite_flip.vert:
  ld a, (TEMP2)
  call set_sprite.copy
  ld hl, STRBUF
  ld de, STRBUF+32+15
  call set_sprite_flip.vert.1
  ld de, STRBUF+32+31
  call set_sprite_flip.vert.1
  ld hl, STRBUF+32
  jr set_sprite.paste
set_sprite_flip.vert.1:
  ld b, 16
set_sprite_flip.vert.2:
  ld a, (hl)
  ld (de), a
  inc hl
  dec de
  djnz set_sprite_flip.vert.2
  ret

set_sprite_flip.horiz:
  ld a, (TEMP2)
  call set_sprite.copy
  ld hl, STRBUF
  ld de, STRBUF+32
  ld b, 32
set_sprite_flip.horiz.1:
  ld a, (hl)
  call binaryReverseA
  ld (hl), a
  ld (de), a
  inc hl
  inc de
  djnz set_sprite_flip.horiz.1
  ld hl, STRBUF+16

set_sprite.paste:
  ld de, (TEMP)
  ld bc, 32
  jp LDIRVM    ; hl = ram data address, de = vram data address, bc = length (interruptions enabled)

set_sprite.copy:
  call gfxCALPAT
  ld (TEMP), hl
  ld de, STRBUF
  ld bc, 32
  jp LDIRMV      ; de = ram data address, hl = vram data address, bc = length

; reverse bits in A
; 8 bytes / 206 cycles
; http://www.retroprogramming.com/2014/01/fast-z80-bit-reversal.html
binaryReverseA:
  push bc
    ld b,8
    ld c,a
binaryReverseA.loop:
    rl c
    rra
    djnz binaryReverseA.loop
  pop bc
  ret

; de = sprite number
; hl = direction (0=left, 1=right, 2=180 degrees)
set_sprite_rotate:
  ld a, l
  cp 2
  jp z, set_sprite_flip
  push af
    ld a, e
    call set_sprite.copy
  pop af
  or a
  jr z, set_sprite_rotate.left

set_sprite_rotate.right:
  ld hl, STRBUF
  ld de, STRBUF+32+16
  call blockRotateR
  ld hl, STRBUF+8
  ld de, STRBUF+32
  call blockRotateR
  ld hl, STRBUF+16
  ld de, STRBUF+32+24
  call blockRotateR
  ld hl, STRBUF+24
  ld de, STRBUF+32+8
  call blockRotateR
  jr set_sprite_rotate.left.end

set_sprite_rotate.left:
  ld hl, STRBUF
  ld de, STRBUF+32+8
  call blockRotateL
  ld hl, STRBUF+8
  ld de, STRBUF+32+24
  call blockRotateL
  ld hl, STRBUF+16
  ld de, STRBUF+32
  call blockRotateL
  ld hl, STRBUF+24
  ld de, STRBUF+32+16
  call blockRotateL
set_sprite_rotate.left.end:
  ld hl, STRBUF+32
  jp set_sprite.paste

; binary rotate 8 bytes from HL to DE
; HL = source
; DE = destination
blockRotateL:
  ld b,8
blockRotateL.1:
    call blockRotateL.2
    ld (de), a
    inc de
  djnz blockRotateL.1
  ret
blockRotateL.2:
  push hl
  push bc
    ld b, 8
blockRotateL.loop:
    ld c, (hl)
    rr c
    rla
    ld (hl), c
    inc hl
    djnz blockRotateL.loop
  pop bc
  pop hl
  ret

blockRotateR:
  ld b,8
blockRotateR.1:
    call blockRotateR.2
    ld (de), a
    inc de
  djnz blockRotateR.1
  ret
blockRotateR.2:
  push hl
  push bc
    ld b, 8
blockRotateR.loop:
    ld c, (hl)
    rl c
    rra
    ld (hl), c
    inc hl
    djnz blockRotateR.loop
  pop bc
  pop hl
  ret

; a = sprite number
; hl = buffer pointer to a 32 bytes buffer
set_sprite_pattern:
  push hl 
    call gfxCALPAT
    ex de,hl
  pop hl
  ld bc, 32
  jp LDIRVM    ; hl = ram data address, de = vram data address, bc = length (interruptions enabled)

; a = sprite number
; hl = buffer pointer to a 16 bytes buffer
set_sprite_color:
  ld c, a
    ld a, (SCRMOD)
    cp 4
  ld a, c 
  jr nc, set_sprite_color.msx2
    push hl 
      call gfxCALATR
      inc hl
      inc hl
      inc hl
    pop de 
    ld a, (de)
    jp WRTVRM
set_sprite_color.msx2:
  push hl
    call gfxCALCOL
    ex de, hl
  pop hl
  ld bc, 16
  jp LDIRVM      ; hl = ram data address, de = vram data address, bc = length

; a = sprite number
; hl = buffer pointer to a 32 bytes buffer
get_sprite_pattern:
  push hl 
    call gfxCALPAT
  pop de 
  ld bc, 32
  jp LDIRMV      ; de = ram data address, hl = vram data address, bc = length

; a = sprite number
; hl = buffer pointer to a 32 bytes buffer
get_sprite_color:
  ld c, a
    ld a, (SCRMOD)
    cp 4
  ld a, c 
  jr nc, get_sprite_color.msx2
    push hl 
      call gfxCALATR
      inc hl
      inc hl
      inc hl
      call RDVRM 
    pop hl 
    ld (hl), a
    xor a 
    inc hl 
    ld (hl), a
    ret 
get_sprite_color.msx2:
  push hl
    call gfxCALCOL
  pop de
  ld bc, 16
  jp LDIRMV      ; de = ram data address, hl = vram data address, bc = length

; l = mode: 0=swap, 1=wave
; e = delay #0
; c = delay #1
cmd_page:
  ld a, (SCRMOD)
  cp 5
  ret c                 ; return if screen mode < 5
  ld d, c
cmd_page.mode:
  ld a, (RG1SAV)
  and 251               ; reset b2 from r1, if mode = swapping
  sla l
  sla l
  or l                  ; set b2 from r1, if mode = waving
  ld b, a               ; change mode to swapping or waving
  ld c, 1               ; vdp(1)
  call WRTVDP           ; b = data, c = register
cmd_page.delay:
  ld a, d
  add a, a
  add a, a
  add a, a
  add a, a
  or e
  ld b, a               ; b = delay #1 (low nibble) and #2 (high nibble)
  ld c, 13              ; vdp(14)
  call WRTVDP           ; b = data, c = register
cmd_page.end:
  ld a, 1
  ld (ACPAGE), a
  jp C70CC              ; XBASIC_SET_PAGE (a=display page)

; https://www.msx.org/wiki/Real_Time_Clock_Programming
; GET DATE
; Results:    HL = Year 1980...2079
;             D = Month (1=Jan...12=Dec)
;             E = Day (1...31)
;             A = Day of week (0=Sun...6=Sat)
cmd_get_date:
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  jr z, cmd_get_date.msx1
    ld c,12             ; year (tens)
    call SUB_REDCLK
    call cmd_clock_mult10
    ld c,11             ; year (units)
    call SUB_REDCLK
    ld bc,1980
    add a, c
    ld c, a
    add hl, bc
    push hl
      ld c, 10          ; month (tens)
      call SUB_REDCLK
      call cmd_clock_mult10
      ld c, 9           ; month (units)
      call SUB_REDCLK
      add a, l
      push af
        ld c, 8           ; day (tens)
        call SUB_REDCLK
        call cmd_clock_mult10
        ld c, 7           ; day (units)
        call SUB_REDCLK
        add a, l
      pop de
      ld e, a
    pop hl
    ld c, 6             ; week
    jp SUB_REDCLK
cmd_get_date.msx1:
  xor a
  ld h, a
  ld l, a
  ld e, a
  ld d, a
  ret

; GET TIME
; Results:    H = Hours
;             L = Minutes
;             A = Seconds
cmd_get_time:
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  jr z, cmd_get_date.msx1
    ld c, 5             ; hour (tens)
    call SUB_REDCLK
    call cmd_clock_mult10
    ld c, 4             ; hour (units)
    call SUB_REDCLK
    add a, l
    ld h, a
    push hl
      ld h, a
      ld c, 3           ; minutes (tens)
      call SUB_REDCLK
      call cmd_clock_mult10
      ld c, 2           ; minutes (units)
      call SUB_REDCLK
      add a, l
    pop hl
    ld l, a
    push hl
      ld c, 1           ; seconds (tens)
      call SUB_REDCLK
      call cmd_clock_mult10
      ld c, 0           ; seconds (units)
      call SUB_REDCLK
      add a, l
    pop hl
    ret

; SET DATE
; Input:      HL = Year 1980...2079
;             D = Month (1=Jan...12=Dec)
;             E = Day (1...31)
cmd_set_date:
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  ret z
  call cmd_clock_disable
  push de
    xor a
    ld bc,1980
    sbc hl, bc
    call cmd_clock_div10
    ld a, l
    ld c,12             ; year (tens)
    call SUB_WRTCLK
    ld a, e
    ld c,11             ; year (units)
    call SUB_WRTCLK
  pop de
  push de
    ld h, 0
    ld l, d
    call cmd_clock_div10
    ld a, l
    ld c, 10            ; month (tens)
    call SUB_WRTCLK
    ld a, e
    ld c, 9             ; month (units)
    call SUB_WRTCLK
  pop de
  ld h, 0
  ld l, e
  call cmd_clock_div10
  ld a, l
  ld c, 8               ; day (tens)
  call SUB_WRTCLK
  ld a, e
  ld c, 7               ; day (units)
  call SUB_WRTCLK
  jp cmd_clock_enable

; SET TIME
; Input:      H = Hours
;             L = Minutes
;             A = Seconds
cmd_set_time:
  ld e, a
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  ret z
  call cmd_clock_disable
  push de
    push hl
      ld l, h
      ld h, 0
      call cmd_clock_div10
      ld a, l
      ld c, 5           ; hour (tens)
      call SUB_WRTCLK
      ld a, e
      ld c, 4           ; hour (units)
      call SUB_WRTCLK
    pop hl
    ld h, 0
    call cmd_clock_div10
    ld a, l
    ld c, 3             ; minutes (tens)
    call SUB_WRTCLK
    ld a, e
    ld c, 2             ; minutes (units)
    call SUB_WRTCLK
  pop hl
  ld h, 0
  call cmd_clock_div10
  ld a, l
  ld c, 1               ; seconds (tens)
  call SUB_WRTCLK
  ld a, e
  ld c, 0               ; seconds (units)
  call SUB_WRTCLK
  jp cmd_clock_enable

; multiply A by 10 (result = HL)
cmd_clock_mult10:
  ld de, 10
  ld h, d
  ld l, a
  jp XBASIC_MULTIPLY_INTEGERS

; divide HL by 10 (result = HL, remainder = DE)
cmd_clock_div10:
  ld de, 10
  jp XBASIC_DIVIDE_INTEGERS

cmd_clock_disable:
  xor a
  ld c,13               ; disable Real Time Clock
  jp SUB_WRTCLK

cmd_clock_enable:
  ld a, 8
  ld c,13               ; enable Real Time Clock
  jp SUB_WRTCLK

;---------------------------------------------------------------------------------------------------------
; NEW COMMANDS
;---------------------------------------------------------------------------------------------------------

; a = scroll direction (same as STRIG)
; hl = ram destination
cmd_screen_copy:
  push af
    ex de, hl
    call cmd_screen.get_start
    push de
      call LDIRMV     ; vram to ram - hl=vram, de=ram, bc=size
    pop hl
  pop af
  or a
  ret z
  cp 4
  jr z, cmd_screen_copy.4
  jr nc, cmd_screen_copy.above
  cp 2
  jr z, cmd_screen_copy.2
  jr nc, cmd_screen_copy.3
  jr cmd_screen_copy.1

cmd_screen_copy.4:  ; scroll down-right
  push hl
    call cmd_screen_copy.5
  pop hl
  jr cmd_screen_copy.3

cmd_screen_copy.2:  ; scroll up-right
  push hl
    call cmd_screen_copy.1
  pop hl

cmd_screen_copy.3:  ; scroll right
  ld a, (hl)
  push af
    ld e, l
    ld d, h
    inc hl
    ld bc, 0x02FF
    ldir
    ex de, hl
    ld a, 23
    ld c, 0x20
cmd_screen_copy.3.loop:
    push af
      ld e, l
      ld d, h
      xor a
      sbc hl, bc
      ld a, (hl)
      ld (de), a
    pop af
    dec a
    jr nz, cmd_screen_copy.3.loop
  pop af
  ld (hl), a
  ret

cmd_screen_copy.1:  ; scroll up
  ld de, BUF
  push de
    push hl
      ld bc, 0x02E0
      push bc
        add hl, bc
        ld bc, 0x0020
        push hl
          ldir
          dec hl
          ex de, hl
        pop hl
        dec hl
      pop bc
      lddr
    pop de
  pop hl
  ld c, 0x20
  ldir
  ret

cmd_screen_copy.above:
  cp 7
  jr z, cmd_screen_copy.7
  jr nc, cmd_screen_copy.8
  cp 6
  jr z, cmd_screen_copy.6

cmd_screen_copy.5:  ; scroll down
  ld de, BUF
  ld bc, 0x0020
  push bc
    push de
      push hl
        ldir
      pop de
      ld bc, 0x02E0
      ldir
    pop hl
  pop bc
  ldir
  ret

cmd_screen_copy.8:  ; scroll left-up
  push hl
    call cmd_screen_copy.1
  pop hl
  jr cmd_screen_copy.7

cmd_screen_copy.6:  ; scroll down-left
  push hl
    call cmd_screen_copy.5
  pop hl

cmd_screen_copy.7:  ; scroll left
  ld bc, 0x02FF
  add hl, bc
  ld a, (hl)
  ld e, l
  ld d, h
  dec hl
  lddr
  ex de, hl
  push af
    ld c, 0x20
    ld a, 23
cmd_screen_copy.7.loop:
    push af
      ld e, l
      ld d, h
      add hl, bc
      ld a, (hl)
      ld (de), a
    pop af
    dec a
    jr nz, cmd_screen_copy.7.loop
  pop af
  ld (hl), a
  ret

; hl = ram source
cmd_screen_paste:
  ex de,hl
    call cmd_screen.get_start
  ex de,hl
  jp LDIRVM     ; ; hl = ram data address, de = vram data address, bc = length

cmd_screen.get_start:
  ld a, (SCRMOD)
  ld hl, 0x1800   ; screen start
  ld bc, 0x0300   ; screen size (tiled only)
  or a
  ret nz
  ld h, a
  ret

; hl = resource number
; DAC = resource number
cmd_screen_load:
  di
    call resource.open_and_get_address  ; in: DAC = resource number, out: hl = resource data, a = resource segment, bc = resource size
      ld c, (hl)
      inc hl
      ld b, (hl)
      inc hl
      ld (DAC), hl            ; first block address
      ld (DAC+2), a           ; segment (megarom)
      ld (ARG), bc            ; block count
    call resource.close
  ei
  jp XBASIC_BLOAD

; MSX Tile Forge - load resource
; cmd_mtf <resource> [,<operation>[,<parameters>]]
;      oper            Operation.
;                      0 (Palette/Tileset) - Load resource into VRAM
;                      0 (Map) - Full map copy using relative screen coordinates (default)
;                      1 (Map) - Full map copy using absolute tile coordinates
;                      2 (Map) - Partial map copy (window)
;      resn            Resource number.
;      colx            Horizontal screen position in the map / Source X coordinate in the map.
;      rowy            Vertical screen position in the map / Source Y coordinate in the map.
;      win_w           Window width in tiles.
;      win_h           Window height in tiles.
;      scr_x           Destination X coordinate on screen.
;      scr_y           Destination Y coordinate on screen.
;      page            Destination screen page (default=0, for screen 4 only).
MTF_RESN_PARM equ PARM1 
MTF_COLX_PARM equ MTF_RESN_PARM+2
MTF_ROWY_PARM equ MTF_COLX_PARM+2
MTF_OPER_PARM equ MTF_ROWY_PARM+2
MTF_WIN_W_PARM equ MTF_OPER_PARM+2
MTF_WIN_H_PARM equ MTF_WIN_W_PARM+2
MTF_SCR_X_PARM equ MTF_WIN_H_PARM+2
MTF_SCR_Y_PARM equ MTF_SCR_X_PARM+2
MTF_PAGE_PARM equ MTF_SCR_Y_PARM+2
MTF_MAP_1ST_ROW equ PARM2
MTF_MAP_WIDTH equ MTF_MAP_1ST_ROW+2
MTF_MAP_HEIGHT equ MTF_MAP_WIDTH+2
MTF_SCR_VRAM_BUF equ MTF_MAP_HEIGHT+2
MTF_SCR_RAM_BUF equ MTF_SCR_VRAM_BUF+2
MTF_SCR_SIZE equ MTF_SCR_RAM_BUF+2
cmd_mtf:
cmd_mtf.check_screen_mode:
  ld a,(SCRMOD)
  cp 2
  jr z, cmd_mtf.load_resource
  cp 4
  ret nz
cmd_mtf.load_resource:
  ld hl, (FONTADDR)
  ld (MTF_SCR_RAM_BUF), hl                    ; screen ram buffer address
  di
    ld hl, (MTF_RESN_PARM)                    ; resource number
    ld (DAC), hl 
    call resource.open_and_get_address        ; in: DAC = resource number, out: hl = resource data, a = resource segment, bc = resource size
      ld a, (hl)                              ; resource type 
      inc hl                                  ; skip resource type

cmd_mtf.check_palette:
      or a
      jr nz, cmd_mtf.check_tileset

cmd_mtf.palette:
      ld a, (VERSION)
      or a
      jr nz, cmd_mtf.palette.copy.to_buffer
        call resource.close                   ; return if MSX 1
        ei
        ret
cmd_mtf.palette.copy.to_buffer:
      inc hl
      inc hl
      inc hl 
      inc hl                                  ; point to palette data start address
      ld de, (MTF_SCR_RAM_BUF)        ; screen ram buffer address 
      ld bc, 16*3
      ldir                                    ; copy to screen buffer
    call resource.close
  ei
cmd_mtf.palette.copy.to_vram:
  ld hl, (MTF_SCR_RAM_BUF)
  ld d, 0                                     ; color number 
cmd_mtf.palette.copy.to_vram.loop:
  push de
    ld a, (hl)                                ; red 
    add a, a
    add a, a
    add a, a
    add a, a
    inc hl
    ld e, (hl)                                ; green
    inc hl
    or (hl)                                   ; blue
    inc hl
    ld ix, S.SETPLT 
    call EXTROM
    ;ei
  pop de
  inc d
  bit 4, d
  jr z, cmd_mtf.palette.copy.to_vram.loop
  ret

cmd_mtf.check_tileset:
      dec a 
      jr nz, cmd_mtf.map

cmd_mtf.set_tiled_mode:
      ld a, 2
      ld (SOMODE), a                          ; tiled mode activated

cmd_mtf.tileset:
      ld d, 0                                 ; calculate tileset data size
      ld e, (hl)                              ; tiles count
      inc hl 
      inc hl 
      inc hl 
      inc hl 
      inc hl                                  ; skip header to point to tileset data address
      ld a, e  
      or a 
      jr nz, cmd_mtf.tileset.copy.to_buffer
        ld de, 256                            ; correct tiles count
cmd_mtf.tileset.copy.to_buffer:
      ex de, hl                               ; hl=tile count, de=tileset+colorset data address
        add hl, hl
        add hl, hl
        add hl, hl                            ; tileset data size = tiles count * 8
        ld (MTF_SCR_SIZE), hl
        add hl, hl                            ; tileset data size + colorset data size
        ld c, l
        ld b, h                               ; bc = tileset data  + colorset data size
      ex de, hl                               ; hl=header address, de=tiles data size
      ld de, (MTF_SCR_RAM_BUF)
      ldir                                    ; copy to buffer
    call resource.close
  ei

cmd_mtf.tileset.copy.to_vram:
  ld de, 0                                    ; tileset bank 0
  call cmd_mtf.copy.to_vram
  ld de, 0x800                                ; tileset bank 1
  call cmd_mtf.copy.to_vram
  ld de, 0x1000                               ; tileset bank 2
  call cmd_mtf.copy.to_vram
  ld hl, (MTF_SCR_RAM_BUF)
  ld bc, (MTF_SCR_SIZE)
  add hl, bc                                  ; colorset buffer address
  ld (MTF_SCR_RAM_BUF), hl  
  ld de, 0x2000                               ; colorset bank 0
  call cmd_mtf.copy.to_vram
  ld de, 0x2800                               ; colorset bank 1
  call cmd_mtf.copy.to_vram
  ld de, 0x3000                               ; colorset bank 2

cmd_mtf.copy.to_vram:
  ld hl, (MTF_SCR_RAM_BUF)
  ld bc, (MTF_SCR_SIZE)
  jp LDIRVM                                   ; hl = ram data address, de = vram data address, bc = length

cmd_mtf.map:
      ; Read map header (common to all map operations)
      ld e, (hl)
      inc hl
      ld d, (hl)
      inc hl
      ld (MTF_MAP_WIDTH), de                  ; map width
      ld e, (hl)
      inc hl
      ld d, (hl)
      inc hl
      ld (MTF_MAP_HEIGHT), de                 ; map height
      ld (MTF_MAP_1ST_ROW), hl                ; map 1st row address
      ; Dispatch based on operation
      ld de, (MTF_COLX_PARM)                  ; col_x parameter
      ld bc, (MTF_ROWY_PARM)                  ; row_y parameter
      ld a, (MTF_OPER_PARM)                   ; map operation
      or a                                    ; relative coords?
      jr nz, cmd_mtf.map_xy                   ; skip if it's not
        ; calculate absolute x and y coords from col/row relative coords
        push hl                               ; resource header
          ex de,hl
          add hl, hl
          add hl, hl
          add hl, hl
          add hl, hl
          add hl, hl    
          ex de, hl                           ; de = col_x = col * 32 = col * 2^5
          ld l, c 
          ld h, b  
          add hl, hl
          add hl, hl
          add hl, hl                          ; row * 2^3
          ld c, l 
          ld b, h  
          add hl, hl                          ; row * 2^4
          add hl, bc 
          ld c, l 
          ld b, h                             ; bc = row_y = row * 24 = row * 2^3 + row * 2^4        
        pop hl                                ; resource header

cmd_mtf.map_xy:
cmd_mtf.map_xy.adjust_if_y_negative:
      ; adjust if y negative
      bit 7, b
      jr z, cmd_mtf.map_xy.adjust_if_x_negative
        ld l, c 
        ld h, b
        ld bc, (MTF_MAP_HEIGHT)
cmd_mtf.map_xy.adjust_if_y_negative.loop:
        add hl, bc 
        bit 7, h 
        jr nz, cmd_mtf.map_xy.adjust_if_y_negative.loop
          ld c, l 
          ld b, h
cmd_mtf.map_xy.adjust_if_x_negative:
      ; adjust if x negative 
      bit 7, d
      jr z, cmd_mtf.map_xy.adjust_if_x_gt_tilemap_width
        ex de, hl 
        ld de, (MTF_MAP_WIDTH)
cmd_mtf.map_xy.adjust_if_x_negative.loop:
        add hl, de
        bit 7, h  
        jr nz, cmd_mtf.map_xy.adjust_if_x_negative.loop 
          ex de, hl 
cmd_mtf.map_xy.adjust_if_x_gt_tilemap_width:
      ; adjust if x > tilemapWidth
      ; so: x = x % tilemapWidth
      push bc 
        ex de, hl                             ; hl = col_x
        ld de, (MTF_MAP_WIDTH)                ; de = map width
        call XBASIC_DIVIDE_INTEGERS
      pop bc 
      ld (MTF_COLX_PARM), de                  ; col_x parameter
cmd_mtf.map_xy.adjust_if_y_gt_tilemap_height:
      ; adjust if y > tilemapHeight
      ; so: y = y % tilemapHeight
      push de 
        ld l, c                               ; hl = row_y
        ld h, b 
        ld de, (MTF_MAP_HEIGHT)               ; de = map height
        call XBASIC_DIVIDE_INTEGERS
        ld c, e 
        ld b, d
      pop de
      ld (MTF_ROWY_PARM), bc                  ; row_y parameter

; real page support deferred to set-page-screen4 — will replace 0x1800 with (GRPNAM)
cmd_mtf.window_copy:
  ; Clip screen_x
  ld hl, (MTF_SCR_X_PARM)
  ld a, l
  cp 32
  jr c, .sx_ok
  ld a, 31
.sx_ok:
  ld d, a                        ; d = screen_x

  ; Clip screen_y
  ld hl, (MTF_SCR_Y_PARM)
  ld a, l
  cp 24
  jr c, .sy_ok
  ld a, 23
.sy_ok:
  ld e, a                        ; e = screen_y

  ; Clip width
  ld hl, (MTF_WIN_W_PARM)
  ld b, l                        ; b = win_w 
  ld a, 32
  sub d
  cp b
  jr nc, .w_ok
  ld b, a
.w_ok:
  ld a, b
  or a
  jp z, .window_copy_done

  ; Clip height
  ld hl, (MTF_WIN_H_PARM)
  ld c, l                        ; c = win_h 
  ld a, 24
  sub e
  cp c
  jr nc, .h_ok
  ld c, a
.h_ok:
  ld a, c
  or a
  jp z, .window_copy_done

  ; Store clipped width and height
  ld a, b
  ld (MTF_WIN_W_PARM), a
  ld a, c
  ld (MTF_WIN_H_PARM), a

  ; Compute vram_start = 0x1800 + screen_y * 32 + screen_x
  ld l, e
  ld h, 0
  add hl, hl
  add hl, hl
  add hl, hl
  add hl, hl
  add hl, hl                     ; hl = screen_y * 32
  ld a, d
  add a, l
  ld l, a
  ld a, h
  adc a, 0
  ld h, a                        ; hl = screen_y * 32 + screen_x
  push de 
    ld de, 0x1800
    add hl, de                   ; hl = 0x1800 + screen_y * 32 + screen_x
  pop de 
  ld (MTF_SCR_VRAM_BUF), hl      ; save vram_start

  ; Compute buffer_size = (height - 1) * 32 + width
  ld a, c                        ; a = win_h 
  dec a
  ld l, a
  ld h, 0
  add hl, hl
  add hl, hl
  add hl, hl
  add hl, hl
  add hl, hl                     ; hl = (height - 1) * 32
  ld a, b                        ; a = win_w 
  add a, l
  ld l, a
  ld a, h
  adc a, 0
  ld h, a                        ; hl = buffer_size
  ld (MTF_SCR_SIZE), hl          ; save buffer_size

  ; Full-width optimization: skip LDIRMV if screen_x == 0 and width == 32
  ld a, d                        ; a = screen_x
  or a
  jr nz, .do_ldirmv
  ld a, b                        ; a = win_w
  cp 32
  jr z, .navigate_source

  ; Copy from VRAM to RAM
.do_ldirmv:
  ld hl, (MTF_SCR_VRAM_BUF)
  ld de, (MTF_SCR_RAM_BUF)
  ld bc, (MTF_SCR_SIZE)
  call SUB_LDIRMV                ; hl = vram data address, de = ram data address, bc = length (do not touch interruptions version)

  ; Navigate to first source map row
.navigate_source:
  ld de, (MTF_ROWY_PARM)         ; map_y
  ld hl, (MTF_MAP_1ST_ROW)
  add hl, de
  add hl, de
  add hl, de                     ; hl = MTF_MAP_1ST_ROW + map_y * 3
  call .go_to_next_map_row
  ; hl = first source row pointer

  ld de, (MTF_SCR_RAM_BUF)       ; dest base

  ; Per-row copy loop
.copy_row:
  ld a, (MTF_WIN_H_PARM)
  or a
  jr z, .copy_done
  dec a
  ld (MTF_WIN_H_PARM), a

  push de                        ; save dest
  push hl                        ; save source row pointer

  ; Skip 3-byte linked-list header
  inc hl
  inc hl
  inc hl

  ; Advance to map_x column
  ld bc, (MTF_COLX_PARM)         ; map_x
  add hl, bc                     ; hl = source + map_x

  ; Copy width bytes: hl=source, de=dest, bc=size
  ld a, (MTF_WIN_W_PARM)
  ld c, a
  ld b, 0
  ldir

  ; Restore source and advance to next source row
  pop hl                         ; hl = saved source row pointer
  call .go_to_next_map_row

  ; Restore dest and advance by 32 for next dest row
  pop de                         ; de = saved dest
  ex de, hl                      ; hl = saved dest
    ld bc, 32
    add hl, bc                   ; hl = next row dest
  ex de, hl                      ; de = next row dest

  jr .copy_row

.go_to_next_map_row:
  ld a, (hl)       ; next row segment 
  inc hl 
  ld e, (hl) 
  inc hl 
  ld d, (hl)       ; next row address
  ex de, hl 
  jp MR_CHANGE_SGM

.copy_done:
  call resource.close
  ei
  ld hl, (MTF_SCR_RAM_BUF)       ; screen ram buffer source
  ld de, (MTF_SCR_VRAM_BUF)      ; screen vram buffer dest
  ld bc, (MTF_SCR_SIZE)          ; screen buffer size
  ;call VDP_WaitVblank
  ;call SUB_LDIRVM                ; hl = ram data address, de = vram data address, bc = length
  jp LDIRVM 

.window_copy_done:
  call resource.close
  ei
  ret

; https://www.msx.org/wiki/PAD()
; input l = pad function parameter code (mouse, trackball...)
; output hl
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


