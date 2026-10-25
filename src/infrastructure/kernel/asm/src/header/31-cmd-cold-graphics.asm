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
