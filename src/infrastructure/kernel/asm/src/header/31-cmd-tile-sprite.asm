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
  cp 2
  jr nc, set_tile.paste.normal
    ; screens 0 and 1: write to base address only (single bank)
    ld hl, (TEMP)
    ex de, hl
    ld hl, STRBUF
    ld bc, 8
    jp LDIRVM

set_tile.paste.normal:
  ld a, b
  cp 3
  jr nc, set_tile.paste.all

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
  cp 2
  jr nc, set_tile_pattern.skip
    or a
    jr nz, set_tile_pattern.force_bank0
    ld d, 0x08                 ; pattern table on screen mode 0
set_tile_pattern.force_bank0:
    ld b, 0x00                 ; screens 0 and 1 have a single bank
set_tile_pattern.skip:
  add hl, hl                   ; x2
  add hl, hl                   ; x4
  add hl, hl                   ; x8
  add hl, de                   ; position in pattern table
  ld de, 0x0800
  ld a, b
  cp 3
  jr nc, set_tile_pattern.all  ; bank >= 3 means all banks
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
    cp 2
    jr nc, get_tile_pattern.skip
      or a
      jr nz, get_tile_pattern.force_bank0
      ld d, 0x08             ; screen 0: pattern table at 0x0800
get_tile_pattern.force_bank0:
      ld b, 0x00             ; screens 0 and 1 have a single bank
get_tile_pattern.skip:
    add hl, hl
    add hl, hl
    add hl, hl               ; hl = tile * 8
    add hl, de               ; hl = tile*8 + base

    ld a, b
    cp 3
    jr c, get_tile_pattern.bank_valid
      xor a                  ; bank >= 3 reads the default bank
get_tile_pattern.bank_valid:
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
  cp 3
  jr c, get_tile_color.bank_valid
    xor a                      ; bank >= 3 reads the default bank
get_tile_color.bank_valid:
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
  jr nc, set_tile_color_buf.all
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
  cp 2
  jr nc, set_tile_pattern_buffer.skip
    or a
    jr nz, set_tile_pattern_buffer.force_bank0
    ld d, 0x08               ; screen 0: pattern table at 0x0800
set_tile_pattern_buffer.force_bank0:
    ld b, 0x00               ; screens 0 and 1 have a single bank
set_tile_pattern_buffer.skip:
  add hl, hl
  add hl, hl
  add hl, hl                 ; hl = tile * 8
  add hl, de                 ; hl = tile*8 + base

  ld a, b
  cp 3
  jr nc, set_tile_pattern_buffer.all

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

;---------------------------------------------------------------------------------------------------------
; SET SPRITE HITBOX
; a = mode: 0=ON all, 1=OFF all, 2=AUTO all,
;           3=ON one (l=sprite), 4=margins (l=sprite; stack left,top,right,bottom),
;           5=OFF one (l=sprite), 6=AUTO one (l=sprite)
;---------------------------------------------------------------------------------------------------------
set_sprite_hitbox:
  or a
  jr z, set_sprite_hitbox.on_all
  dec a
  jr z, set_sprite_hitbox.off_all
  dec a
  jr z, set_sprite_hitbox.auto_all
  dec a
  jr z, set_sprite_hitbox.on_one
  dec a
  jr z, set_sprite_hitbox.margins
  dec a
  jr z, set_sprite_hitbox.off_one
  ; mode 6: AUTO one
  jp set_sprite_hitbox.auto

set_sprite_hitbox.on_all:
  xor a
set_sprite_hitbox.on_all.loop:
  push af
    call set_sprite_hitbox.default
  pop af
  inc a
  cp 32
  jr nz, set_sprite_hitbox.on_all.loop
  ret

set_sprite_hitbox.off_all:
  xor a
set_sprite_hitbox.off_all.loop:
  push af
    call SUB_GET_HITBOX_ADDR
    inc hl
    inc hl
    inc hl
    inc hl
    ld (hl), 0
  pop af
  inc a
  cp 32
  jr nz, set_sprite_hitbox.off_all.loop
  ret

set_sprite_hitbox.auto_all:
  xor a
set_sprite_hitbox.auto_all.loop:
  push af
    ld l, a
    call set_sprite_hitbox.auto
  pop af
  inc a
  cp 32
  jr nz, set_sprite_hitbox.auto_all.loop
  ret

set_sprite_hitbox.on_one:
  ld a, l
  call set_sprite_hitbox.default
  ret

set_sprite_hitbox.off_one:
  ld a, l
  call SUB_GET_HITBOX_ADDR
  inc hl
  inc hl
  inc hl
  inc hl
  ld (hl), 0
  ret

; default hitbox for sprite in a: zero margins, enabled
set_sprite_hitbox.default:
  call SUB_GET_HITBOX_ADDR
  ld (hl), 0
  inc hl
  ld a, (SPRSIZ)
  ld (hl), a
  inc hl
  ld (hl), 0
  inc hl
  ld (hl), a
  inc hl
  ld (hl), 0xff
  ret

; set margins
; l = sprite, stack (top to bottom): left, top, right, bottom
set_sprite_hitbox.margins:
  ld a, l
  ld (STRBUF+8), a         ; save sprite number
  pop de                   ; return address
  pop hl
  ld (STRBUF+0), hl        ; left
  pop hl
  ld (STRBUF+2), hl        ; top
  pop hl
  ld (STRBUF+4), hl        ; right
  pop hl
  ld (STRBUF+6), hl        ; bottom
  push de                  ; restore return address

  ; hitbox address
  ld a, (STRBUF+8)
  call SUB_GET_HITBOX_ADDR
  push hl
  pop ix

  ; left
  ld hl, (STRBUF+0)
  call clamp_margin
  ld (ix+0), a
  ld (STRBUF+0), a

  ; top
  ld hl, (STRBUF+2)
  call clamp_margin
  ld (ix+2), a
  ld (STRBUF+2), a

  ; right, limited to SPRSIZ - left
  ld a, (SPRSIZ)
  ld b, a
  ld a, (STRBUF+0)
  ld c, a
  ld a, b
  sub c
  ld b, a
  ld hl, (STRBUF+4)
  call clamp_margin_limit
  ld c, a
  ld a, (SPRSIZ)
  sub c
  ld (ix+1), a

  ; bottom, limited to SPRSIZ - top
  ld a, (SPRSIZ)
  ld b, a
  ld a, (STRBUF+2)
  ld c, a
  ld a, b
  sub c
  ld b, a
  ld hl, (STRBUF+6)
  call clamp_margin_limit
  ld c, a
  ld a, (SPRSIZ)
  sub c
  ld (ix+3), a

  ld (ix+4), 0xff
  ret

; clamp 16-bit value in hl to [0, SPRSIZ]; result in a
clamp_margin:
  ld a, h
  or a
  jr nz, clamp_margin.max
  ld a, (SPRSIZ)
  cp l
  jr nc, clamp_margin.use_l
  ret
clamp_margin.use_l:
  ld a, l
  ret
clamp_margin.max:
  ld a, (SPRSIZ)
  ret

; clamp 16-bit value in hl to [0, b]; result in a
clamp_margin_limit:
  ld a, h
  or a
  jr nz, clamp_margin_limit.max
  ld a, b
  cp l
  jr nc, clamp_margin_limit.use_l
  ret
clamp_margin_limit.use_l:
  ld a, l
  ret
clamp_margin_limit.max:
  ld a, b
  ret

; derive hitbox from sprite pattern (one-shot)
; l = sprite number
set_sprite_hitbox.auto:
  ld a, l
  ld (STRBUF+40), a        ; save sprite number

  ; read pattern into STRBUF
  ld a, l
  call gfxCALPAT           ; hl = vram pattern address
  ld de, STRBUF
  ld a, (SPRSIZ)
  cp 16
  jr z, set_sprite_hitbox.auto.read32
  ld bc, 8
  jr set_sprite_hitbox.auto.read
set_sprite_hitbox.auto.read32:
  ld bc, 32
set_sprite_hitbox.auto.read:
  call LDIRMV              ; de = ram data address, hl = vram data address, bc = length

  ; init scratch
  ld a, 0xff
  ld (STRBUF+34), a        ; miny
  xor a
  ld (STRBUF+35), a        ; maxy
  ld (STRBUF+36), a        ; column mask low (x0..x7)
  ld (STRBUF+37), a        ; column mask high (x8..x15)

  ld a, (SPRSIZ)
  cp 16
  jr z, set_sprite_hitbox.auto.scan16
  ld b, 8
  ld c, 1
  jr set_sprite_hitbox.auto.scan
set_sprite_hitbox.auto.scan16:
  ld b, 16
  ld c, 2

set_sprite_hitbox.auto.scan:
  ld iy, STRBUF
  ld d, 0                  ; y
set_sprite_hitbox.auto.loop:
  ld a, (iy+0)             ; b0
  ld e, a                  ; row mask
  ld hl, STRBUF+36
  or (hl)
  ld (hl), a               ; coll |= b0
  ld a, c
  cp 2
  jr nz, set_sprite_hitbox.auto.rowmask
  ld a, (iy+1)             ; b1
  or e
  ld e, a                  ; row mask |= b1
  ld a, (iy+1)
  ld hl, STRBUF+37
  or (hl)
  ld (hl), a               ; colh |= b1
set_sprite_hitbox.auto.rowmask:
  ld a, e
  or a
  jr z, set_sprite_hitbox.auto.nextrow
  ld a, (STRBUF+34)
  cp 0xff
  jr nz, set_sprite_hitbox.auto.sety_max
  ld a, d
  ld (STRBUF+34), a        ; miny = y
set_sprite_hitbox.auto.sety_max:
  ld a, d
  ld (STRBUF+35), a        ; maxy = y
set_sprite_hitbox.auto.nextrow:
  inc iy
  ld a, c
  cp 2
  jr nz, set_sprite_hitbox.auto.no2
  inc iy
set_sprite_hitbox.auto.no2:
  inc d
  dec b
  jr nz, set_sprite_hitbox.auto.loop

  ; hitbox address
  ld a, (STRBUF+40)
  call SUB_GET_HITBOX_ADDR
  push hl
  pop ix

  ; any visible pixel?
  ld a, (STRBUF+36)
  ld hl, STRBUF+37
  or (hl)
  jr z, set_sprite_hitbox.auto.disabled

  ; minx
  ld a, (STRBUF+36)
  or a
  jr z, set_sprite_hitbox.auto.minx_hi
  call firstSetBitPos
  jr set_sprite_hitbox.auto.minx_store
set_sprite_hitbox.auto.minx_hi:
  ld a, (STRBUF+37)
  call firstSetBitPos
  add a, 8
set_sprite_hitbox.auto.minx_store:
  ld (STRBUF+32), a

  ; maxx
  ld a, (STRBUF+37)
  or a
  jr z, set_sprite_hitbox.auto.maxx_lo
  call lastSetBitPos
  add a, 8
  jr set_sprite_hitbox.auto.maxx_store
set_sprite_hitbox.auto.maxx_lo:
  ld a, (STRBUF+36)
  call lastSetBitPos
set_sprite_hitbox.auto.maxx_store:
  ld (STRBUF+33), a

  ; write hitbox
  ld a, (STRBUF+32)
  ld (ix+0), a             ; X0 = minx
  ld a, (STRBUF+33)
  inc a
  ld (ix+1), a             ; X1 = maxx + 1
  ld a, (STRBUF+34)
  ld (ix+2), a             ; Y0 = miny
  ld a, (STRBUF+35)
  inc a
  ld (ix+3), a             ; Y1 = maxy + 1
  ld (ix+4), 0xff          ; enabled
  ret

set_sprite_hitbox.auto.disabled:
  ld (ix+4), 0
  ret

; input a, output a = leftmost set bit position (0=MSB) or 8 if none
firstSetBitPos:
  ld b, 0
firstSetBitPos.loop:
  bit 7, a
  jr nz, firstSetBitPos.found
  add a, a
  inc b
  ld c, a
  ld a, b
  cp 8
  ld a, c
  jr nz, firstSetBitPos.loop
  ld a, 8
  ret
firstSetBitPos.found:
  ld a, b
  ret

; input a, output a = rightmost set bit position (0=MSB) or 0xFF if none
lastSetBitPos:
  ld c, 0
lastSetBitPos.loop:
  bit 0, a
  jr nz, lastSetBitPos.found
  srl a
  inc c
  ld d, a
  ld a, c
  cp 8
  ld a, d
  jr nz, lastSetBitPos.loop
  ld a, 0xff
  ret
lastSetBitPos.found:
  ld a, 7
  sub c
  ret

; l = mode: 0=swap, 1=wave
; e = delay #0
; c = delay #1
