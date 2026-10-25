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
