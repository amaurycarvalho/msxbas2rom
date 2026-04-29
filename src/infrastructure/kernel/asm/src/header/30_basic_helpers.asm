;---------------------------------------------------------------------------------------------------------
; BASIC HELPER ROUTINES
;---------------------------------------------------------------------------------------------------------

; hl = heap start address
; de = temporary string start address
; ix = temporary font buffer address
XBASIC_INIT:
  ; --> set temporary buffers 
  xor a
  ld (TMPSTRIND), a       ; temporary string list current position
  ld (TMPSTRADDR), de     ; temporary string list start pointer
  ld (FONTADDR), ix       ; temporary font buffer address

  ; --> calculate heap size 
  ld (HEAPSTR), hl        ; heap start address
  ld (STREND), hl         ; address of the end of the variable area
  ex de, hl
  ld hl, HEAPEND          ; end of the heap area (HIMEM)
  sbc hl, de
  ld (HEAPSIZ), hl        ; heap size

  ; --> initialize disk mode, if supported
  ld a, (STARTUP_CFG_FILEIO)
  or a
  call nz, XBASIC_INIT.disk_mode

  ; --> variables area
  ld hl, BASMEM           ; address of the variables area
  ld (ARYTAB), hl         ; address of the array variables area

  ; --> resource maps 
  ld hl, (resource.map.address)
  ld (RSCMAPAD), hl       ; resource map address (copy on ram)
  ld a, (resource.map.segment)
  ld (RSCMAPSG), a        ; resource map segment number (copy on ram)

  ; --> character font data
  ld a, (CGPNTSLT)
  ld (FONTOLDSLT), a
  ld hl, (CGPNT)
  ld (FONTOLD), hl

  ; --> randomize data 
  ld hl, 0x3579           ; RANDOMIZE 1 - FIX
  ld (SWPTMP), hl         ; SWPTMP+0
  ld hl, 0x7531           ; RANDOMIZE 2 - FIX
  ld (SWPTMP+2), hl       ; SWPTMP+2

  ; --> usr function data 
  ld hl, usr0
  ld (USRTAB), hl
  ld hl, usr1
  ld (USRTAB+2), hl
  ld hl, usr2
  ld (USRTAB+4), hl
  ld hl, usr3
  ld (USRTAB+6), hl

  ; --> function keys data 
  call ERAFNK              ; disable function keys display
  ld hl, FNKSTR            ; configure function keys codes from 245 to 246
  ld de, FNKSTR+1
  ld a, 246
  ld b, 0
XBASIC_INIT.loop:
    ld (hl), a
    inc hl
    inc de
    ld (hl), b
    ld c, 15
    ldir
    inc a
  jr nz, XBASIC_INIT.loop

  ; --> clear variables
  ld hl, (HEAPSTR)         ; heap start address
  ld de, BASMEM            ; variables start address
  xor a
  sbc hl, de               ; hl = variables data size
  ld c, l
  ld b, h                  ; bc = hl
  ex de, hl
  ld (hl), a               ; clear variables first byte
  ld e, l
  ld d, h
  inc de                   ; de = hl + 1
  ldir                     ; clear all the rest
  ret

XBASIC_INIT.disk_mode
  ; test Disk Basic support
  ld a, (HPHYD)
	cp 0xC9
	jr z, XBASIC_INIT.non_disk_mode	  ; Jump if no disk installed 

  ; --> set BDOS I/O buffer FCBs and DTA address
  xor a
  ld (DSKDIS), a                    ; enable disks
  ld a, 1                           ; default MAXFILES for disk mode
  call cmd_maxfiles                 ; set DTA address, alloc i/o buffers and calculate heap size

  ; --> read default drive to populate disk buffers
  ld a, (DFTDRV)
  or a
  push af
    add a, a
    ld e, a
    ld d, 0
    ld hl, DPBLIST
    add hl, de
    ld a, (hl)
    inc hl
    ld h, (hl)
    ld l, a			                    ; pointer to DPB
  pop af
  inc hl
  ld c, (hl)
  ld b, 1
  ld hl, (DIRBUF)                   ; temporary use dirsector buffer
  push hl
    ld de, 0
    call HPHYD
    ld a, 0xFF
    ld (DIRDRV), a		              ; invalid dirsector buffer
  pop	hl
  ret	c

  ; --> copy from DIRBUF to DTAADDR
  ld de, (DTAADDR)
  ld bc, 0x0100
  ldir
  ret

XBASIC_INIT.non_disk_mode
  xor a
  ld (DSKDIS), a              ; disable disks
  ld (MAXFIL), a              ; no MAXFILES for disk mode
  ret 

XBASIC_END:
  ld a, (SCRMOD)
  cp 2
  jr c, XBASIC_END.1          ; skip if already in text mode
    ld a, 15
    ld (FORCLR), a
    ld a, 4
    ld (BAKCLR), a
    ld (BDRCLR), a
    call INITXT               ; screen 0
XBASIC_END.1:
  call INIFNK                 ; enable function keys
  call DSPFNK                 ; display function keys
  xor a
  ld (SUBFLG), a
  ld (FLGINP), a
  ld (DORES), a
  ld (CONSAV), a
  ;ld (MAXFIL), a              ; MAXFIL - reset max files
  ;ld (NLONLY), a              ; NLONLY - reset io buffers (warning: commented because it halts the system)
  ld (ONEFLG), a              ; error flag (0=not in ERROR handler routine)
  cpl
  ld (CURLIN), a              ; CURLIN = 0xFFFF
  ld (CURLIN+1), a
  ;ld (DSKDIS), a              ; disable disks
  ld hl, (HEAPSTR)            ; heap start address
  ld (VARTAB), hl		          ; start variable area
  ld (ARYTAB), hl             ; start arrayvariable area = start variable area (no variables)
  ld (STREND), hl             ; start free area = start variable area (no arrayvariables)
  ld de, (HEAPSIZ)
  ld bc, -256
  add hl, de
  add hl, bc 
  ld (FRETOP), hl             ; start of string area
  ld hl, 0x8047
  ld (DATPTR), hl
  ld hl, TEMPST
  ld (TEMPPT), hl
  ld hl, 0x0000
  ld (OLDTXT), hl
  ld (OLDLIN), hl
  ld a, (EXPTBL)
  ld h, 0x40
  jp ENASLT                ; enable basic page

; l = width size
XBASIC_WIDTH:
  ld a, (SCRMOD)          ; SCRMOD (current screen mode), OLDSCR (last text screen mode)
  cp 2
  ret nc
  ex de, hl
  ld hl, LINL40
  add a, l                ; screen 0 = LINL40 (F3AE), screen 1 = LINL32 (F3AF)
  ld l, a
  ld a, e                 ; copy parameter to A
  ld (hl), a
  ld (LINLEN), a          ; LINLEN
  sub 0x0E
  add a, 0x1C
  cpl
  inc a
  add a, e
  ld (CLMLST), a
  ; ld a, 0x0C             ; new page (clear the screen)
  ; rst 0x18               ; OUTDO - output to screen
  ld a, (SCRMOD)          ; SCRMOD (current screen mode), OLDSCR (last text screen mode)
  ld l, a
  call XBASIC_SCREEN      ; xbasic SCREEN mode (in: a, l = screen mode)
  jp XBASIC_CLS

; a, l = screen mode
XBASIC_SCREEN:
  cp 4
  jr c, XBASIC_SCREEN.TEXT_OR_GRAPH     ; skip if screen mode <= 3 (its safe for msx1)
    ld a, (VERSION)
    or a
    jr nz, XBASIC_SCREEN.TEXT_OR_GRAPH  ; skip if not MSX1 (screen mode its safe for msx2 and above)
       ld l, 2                          ; else, force screen mode 2
XBASIC_SCREEN.TEXT_OR_GRAPH:
  ld a, l
  cp 2
  rla                                   ; put carry on bit 0
  cpl
  and 1
  ld (SOMODE), a                        ; text mode (0), else graphical mode (1)
XBASIC_SCREEN.DO:
  ld a, l
  call C7369                            ; customized CHGMOD
  ld a, (VERSION)
  or a
  ret z                                 ; return if msx1
  ld ix, S.INIPLT                       ; initialize the pallete on msx2
  jp SUB_EXTROM

XBASIC_CLS:
  ld a, (SOMODE)
  cp 2              ; tiled output mode?
  jp z, cmd_clrscr
  jp I6CE3

; hl=y, de=x
XBASIC_LOCATE:
  ld a, (SOMODE)
  or a
  jr z, XBASIC_LOCATE.TXT  ; jump if text mode (0)
  dec a
  jr z, XBASIC_LOCATE.GRP  ; jump if graphical mode (1)
XBASIC_LOCATE.TIL:         ; tiled mode (2)
  ld h, e
  inc l
  inc h
  ld (CSRY),hl
  ld a, h
  ld (TTYPOS), a
  ret
XBASIC_LOCATE.GRP:         ; graphical mode (1)
  ld (GRPACY), hl
  ld (GRPACX), de
  ex de,hl
  ld c, l
  ld b, h
  call SCALXY ; BC = X, DE = Y
  jp MAPXYC ; BC = X, DE = Y
XBASIC_LOCATE.TXT:         ; text mode (0)
  ld h, e
  inc l
  inc h
  jp POSIT               ; h=x, l=y

; hl = string address
XBASIC_PRINT_STR:
  ld a, (hl)
  or a
  ret z
  ld b, a
  ld a, (SOMODE)
  or a
  jp z, C74FE                 ; jump if text mode (0)
  dec a
  jr z, XBASIC_PRINT_STR.GRP  ; jump if graphical mode (1)
XBASIC_PRINT_STR.TIL:         ; tiled mode (2)
  push hl
    ld hl, (CSRY)
    push hl
      call gfxTileAddress
      ex de,hl
    pop af
    add a, b
    ld (CSRX), a
  pop hl
  ld c, b
  ld b, 0
  inc hl
  jp LDIRVM
XBASIC_PRINT_STR.GRP:
  ld a, (SCRMOD)
  cp 5
  jr nc, XBASIC_PRINT_STR.MSX2
XBASIC_PRINT_STR.MSX1:
  inc hl
  ld a, (hl)
  call GRPPRT
  djnz XBASIC_PRINT_STR.MSX1
  ret
XBASIC_PRINT_STR.MSX2:
  inc hl
  ld a, (hl)
  ld ix, GRPPRT2
  call SUB_EXTROM
  djnz XBASIC_PRINT_STR.MSX2
  ret

; in: ix=x, iy=y, d=color, a=sprite number, h=pattern number, b=parameters flag (b11100000)
XBASIC_PUT_SPRITE:
  bit 7, b
  call nz, SUB_SETSPRTBL_XY
  jp C71BF

; hl = input data
; a = user function number
XBASIC_USR:
  ld (DAC+2), hl         ; input data

  ld hl, XBASIC_USR.ret
  push hl

  ld hl, USRTAB
  add a, a
  ld d, 0
  ld e, a
  add hl, de

  ld e, (hl)
  inc hl
  ld d, (hl)

  push de

  ld a, 2              ; input is integer type
  ld (VALTYP), a

  xor a
  ret                  ; indirect call to usr function

XBASIC_USR.ret:
  ld hl, (DAC+2)       ; return data
  ret

; de=x0, hl=y0, ix=x1, iy=y1, 0xFC18=srcpg, GRPACX=x2, GRPACY=y2, 0xFC19=destpg, b=operator, FAFC=mode (screen limits)
XBASIC_COPY:
  ld a, (SOMODE)
  cp 1
  jr nz, XBASIC_COPY.TILED ; jump if not graphical mode (1)

  ld a, (VERSION)
  or a
  ret z                    ; only for MSX2

  jp BASKUN_COPY           ; BASIC KUN original COPY (DE=X0, HL=Y0, IX=X1, IY=Y1, B=operator, FCB7=XD, FCB9=YD, FC18=PS, FC19=PD)

XBASIC_COPY.TILED:
  ld (SX), de
  ld (SY), hl
  push hl
    push ix
    pop hl
    xor a
    sbc hl, de
    inc hl
    ld (NX), hl
  pop de
  push iy
  pop hl
  xor a
  sbc hl, de
  inc hl
  ld (NY), hl
  ld hl, (FONTADDR)
  call XBASIC_COPY_TO.TILED

  ld hl, (GRPACX)
  ld (DX), hl
  ld hl, (GRPACY)
  ld (DY), hl
  ld hl, (FONTADDR)
  jp XBASIC_COPY_FROM.TILED

; dx=x, dy=y, hl=src address, argt=expansion/direction (0000DDEE), logop=operation
XBASIC_COPY_FROM:
  ld a, (SOMODE)
  cp 1
  jr nz, XBASIC_COPY_FROM.TILED ; jump if not graphical mode (1)

  ld a, (VERSION)
  or a
  ret z                         ; only for MSX2

  ld (VM_DPTR), hl
  ld hl, VM_DPTR
  ld ix, BLTVM
  jp SUB_EXTROM               ; XBASIC original COPY FROM

XBASIC_COPY_FROM.TILED:         ; dx=x, dy=y, hl=src address
  push hl
    ld hl, (DY)
    inc hl
    ld a, (DX)
    inc a
    ld h, a
    call gfxTileAddress
    ex de, hl
  pop hl

  ld c, (hl)                    ; nx
  ld b, 0
  inc hl
  ld a, (hl)                    ; ny
  inc hl
XBASIC_COPY_FROM.TILED.loop:
  push af
  push de
  push bc
  push hl
    call LDIRVM
  pop hl
  ld a, (LINLEN)
  ld c, a
  ld b, 0
  add hl, bc
  inc hl
  ex de, hl
  pop bc
  pop hl
  add hl, bc
  ex de, hl
  pop af

  dec a
  ret z
  jr XBASIC_COPY_FROM.TILED.loop

; sx=x, sy=y, hl=dest address, nx=width, ny=height, argt=expansion/direction (0000DDEE)
XBASIC_COPY_TO:
  ld (MV_DPTR), hl
  ld hl, (NX)
  xor a
  sbc hl, de                    ; x1 must be >= x0
  jr nc, XBASIC_COPY_TO.1
    call intNeg
XBASIC_COPY_TO.1:
  ld (NX), hl

  ld de, (SY)
  ld hl, (NY)
  xor a
  sbc hl, de                    ; y1 must be >= y0
  jr nc, XBASIC_COPY_TO.2
    call intNeg
XBASIC_COPY_TO.2:
  ld (NY), hl
  ld hl, (MV_DPTR)

  ld a, (RAMAD3)
  and 0xF0                      ; keep just expansion
  ld (ARGT), a

  ld a, (SOMODE)
  dec a
  jr nz, XBASIC_COPY_TO.TILED   ; jump if not graphical mode (1)

  ld a, (VERSION)
  or a
  ret z                         ; only for MSX2

  ld hl, VM_DPTR

  ld ix, BLTMV
  jp SUB_EXTROM               ; XBASIC original COPY TO

XBASIC_COPY_TO.TILED:           ; sx=x, sy=y, hl=dest address, nx=width, ny=height
  push hl
    ld hl, (SY)
    inc hl
    ld a, (SX)
    inc a
    ld h, a
    call gfxTileAddress
  pop de
  ld bc, (NX)
  inc bc
  ld a, c
  ld (de), a
  inc de
  ld a, (NY)
  inc a
  ld (de), a
  inc de
XBASIC_COPY_TO.TILED.loop:
  push af
  push hl
  push bc
    push de
      call LDIRMV
    pop hl
    ld a, (LINLEN)
    ld c, a
    ld b, 0
    add hl, bc
    inc hl
    ex de, hl
  pop bc
  pop hl
  add hl, bc
  pop af

  dec a
  ret z
  jr XBASIC_COPY_TO.TILED.loop

; hl, de, bc (all pointers to strings)
XBASIC_PLAY:
  call XBASIC_GET_BUFFER

  xor a
  cp d
  jr z, XBASIC_PLAY.1b
    cp b
    jr z, XBASIC_PLAY.1a
      push af
XBASIC_PLAY.1a:
    push bc
XBASIC_PLAY.1b:
  push de

XBASIC_PLAY.1:
  ld de, (TMPSTRBUF)
  jr XBASIC_PLAY.3

XBASIC_PLAY.2:
    ld a, ','                 ; label 2
    ld (de), a
    inc de

XBASIC_PLAY.3:
  ld a, '"'                   ; label 3
  ld (de), a
  inc de

  xor a
  ld b, a
  ld c, (hl)
  or c
  jr z, XBASIC_PLAY.4
    inc hl
    ldir
XBASIC_PLAY.4:
  ld a, '"'
  ld (de), a
  inc de

  xor a
  ld (de), a

  pop hl
  cp h
  jr nz, XBASIC_PLAY.2

  ld hl, (TMPSTRBUF)

  ld ix, (PLAY)
  call CALBAS
  ei
  ret

; out (TMPSTRBUF) = next temporary string address
XBASIC_GET_BUFFER:
  push af
  push hl
  push de
  push bc
    call GET_NEXT_TEMP_STRING_ADDRESS
    ld (TMPSTRBUF), hl
  pop bc
  pop de
  pop hl
  pop af
  ret

XBASIC_BASE:
  ld a, l
  cp 0x14                       ; parameter >= 20?
  jr nc, XBASIC_BASE.GE_20
XBASIC_BASE.LE_20:
    cp 0x02
    jr nz, XBASIC_BASE.GET_NAME_TABLE
      ld a, (LINLEN)
      cp 0x29
      jr c, XBASIC_BASE.GET_NAME_TABLE
        ld hl, 0x1000
        ret
XBASIC_BASE.GET_NAME_TABLE:
    ex de, hl
      ld hl, TXTNAM
      add hl, de
      add hl, de
      ld e, (hl)
      inc hl
      ld d, (hl)
    ex de, hl
    ret
XBASIC_BASE.GE_20:
    sub 0x19
    jr nc, XBASIC_BASE.GET_TABLE_2
XBASIC_BASE.GET_TABLE_1:
      add a, 0x0F
      ld l, a
      cp 0x0D
      jr nz, XBASIC_BASE.LE_20
        ld hl,0x1E00
        ret
XBASIC_BASE.GET_TABLE_2:
    ld d, 0x00
    ld e, a
    ld hl, XBASIC_BASE.DATA_TABLE
    add hl, de
    ld h, (hl)
    ld l, d
    ret
XBASIC_BASE.DATA_TABLE:
  db 0x00, 0x00, 0x00, 0x76, 0x78, 0x00, 0x00, 0x00
  db 0x76, 0x78, 0x00, 0x00, 0x00, 0xFA, 0xF0, 0x00
  db 0x00, 0x00, 0xFA, 0xF0

XBASIC_TAB:
  ld de, (TTYPOS)
  xor a
  sbc hl, de
  bit 7, h
  jr z, XBASIC_TAB.1
    ld l, a
XBASIC_TAB.1:
  ld a, 0x20   ; space
  ld b, l
  call XBASIC_GET_BUFFER
  ld hl, (TMPSTRBUF)
  jp 0x7EF5    ; STRING$ (hl=destination, b=number of chars, a=char)

XBASIC_SOUND:
  cp 007h
  jr nz, XBASIC_SOUND.1
    res 6,e
    set 7,e
XBASIC_SOUND.1:
  jp WRTPSG

XBASIC_READ:
  ld a, (DORES)                 ; DATA current segment
  or a
  jp nz, XBASIC_READ_MR
XBASIC_READ.cont:
  di
    call resource.open
      call XBASIC_GET_BUFFER
      ld hl, (DATPTR)           ; DATA current pointer
      ld de, (TMPSTRBUF)
      ld c, (hl)
      ld b, 0
      inc bc
      ldir
      ld (DATPTR), hl
    call resource.close
  ei
  ld hl, (TMPSTRBUF)
  ret

XBASIC_IREAD:
  ld a, (DORES)                 ; DATA current segment
  or a
  jp nz, XBASIC_IREAD_MR
XBASIC_IREAD.cont:
  di
    call resource.open
      ld hl, (DATPTR)          ; DATA current pointer
      ld e, (hl)
      inc hl
      ld d, (hl)
      inc hl
      ld (DATPTR), hl
      push de
    call resource.close
  ei
  pop hl
  ret

; RESTORE <number>
;   BYTE data_resource_type
;   structure for DATA (0)
;     WORD resource_items_count
;     ARRAY lines_map[resource_items_count]
;       WORD line_number
;       BYTE line_items_count
;     ARRAY lines_data[resource_items_count]
;       STRINGS item_data[line_items_count]
;   structure for CSV (1)
;     WORD resource_items_count
;     ARRAY lines_map[resource_items_count]
;       BYTE line_items_count
;     ARRAY lines_data[resource_items_count]
;       STRINGS item_data[line_items_count]
;   structure for TXT (2)
;     STRINGS item_data[...]
;   structure for IDATA (3)
;     WORD resource_items_count
;     ARRAY lines_map[resource_items_count]
;       WORD line_number
;       BYTE line_items_count
;     ARRAY lines_data[resource_items_count]
;       WORD item_data[line_items_count]
XBASIC_RESTORE:
  di
    push hl
      call resource.open
    pop bc                      ; bc = new line position
    ld hl, (DATLIN)             ; DATA map table start pointer
    ld a, (SUBFLG)              ; DATA start segment
    ld (DORES), a               ; DATA current segment
    or a
    call nz, MR_CHANGE_SGM
XBASIC_RESTORE.cont:
    ld a, (hl)
    ld (TEMP2), a               ; data resource type
    or a
    jr z, XBASIC_RESTORE.0      ; 0=DATA
    dec a
    jr z, XBASIC_RESTORE.1      ; 1=CSV
    dec a
    jr z, XBASIC_RESTORE.2      ; 2=TXT

                                ; default = IDATA

XBASIC_RESTORE.0:               ; DATA/IDATA resource
  call XBASIC_RESTORE.get_start
  push hl                       ; save data map start pointer
    add hl, de                  ; add lines_map size * resource_item_count
    add hl, de
    add hl, de
    ld (DATPTR), hl             ; save lines data start pointer
  pop hl                        ; restore data map start pointer

XBASIC_RESTORE.0.loop:          ; search on the data map for the line number required
  ld e, (hl)
  inc hl
  ld d, (hl)                    ; de = line number
  ex de, hl
  xor a
  sbc hl, bc
  ex de, hl
  jr nc, XBASIC_RESTORE.found   ; jump if current line number (hl) >= searched line number (bc)

  inc hl
  call XBASIC_RESTORE.add_item

  jr XBASIC_RESTORE.0.loop

XBASIC_RESTORE.1:               ; CSV resource
  call XBASIC_RESTORE.get_start
  push hl                       ; save data map start pointer
    add hl, de                  ; add lines_map size * resource_item_count
    ld (DATPTR), hl             ; save lines data start pointer
  pop hl                        ; restore data map start pointer

XBASIC_RESTORE.1.loop:
  ld a, b
  or c
  jr z, XBASIC_RESTORE.found

  call XBASIC_RESTORE.add_item

  dec bc

  jr XBASIC_RESTORE.1.loop

XBASIC_RESTORE.2:               ; TXT resource
  inc hl                        ; skip resource type
XBASIC_RESTORE.2.init:
  ld d, 0
XBASIC_RESTORE.2.loop:
  ld (DATPTR), hl               ; save lines data current pointer
  ld a, b
  or c
  jr z, XBASIC_RESTORE.end

  ld e, (hl)
  add hl, de                    ; add line size

  ld a, h
  cp 0xC0
  jr c, XBASIC_RESTORE.2.cont
    call XBASIC_RESTORE.next_sgm
    ld e, (hl)
    add hl, de                    ; add line size

XBASIC_RESTORE.2.cont:
  inc hl

  dec bc
  jr XBASIC_RESTORE.2.loop

XBASIC_RESTORE.next_sgm:
  ld a, (DORES)
  inc a
  inc a
  ld (DORES), a
  call MR_CHANGE_SGM
  ld hl, 0x8000
  ld (DATPTR), hl
  ret

XBASIC_RESTORE.get_start:
  ld de, 0
  ld (TEMP), de

  inc hl                        ; skip resource type

  ld e, (hl)
  inc hl
  ld d, (hl)                    ; de = lines count
  inc hl
  ret

XBASIC_RESTORE.add_item:
  ld e, (hl)                    ; a = items in the line
  ld d, 0
  inc hl
  push hl
    ld hl, (TEMP)
    add hl, de
    ld (TEMP), hl
  pop hl
  ret

XBASIC_RESTORE.found:
  ld bc, (TEMP)                 ; resource items to skip
  ld hl, (DATPTR)               ; resource items start address
  ld a, (TEMP2)
  cp 3
  jr nz, XBASIC_RESTORE.2.init

XBASIC_RESTORE.3:
  add hl, bc
  add hl, bc                    ; hl = hl + bc*2
  ld a, h
  cp 0xC0
  call nc, XBASIC_RESTORE.3.next_sgm
  ld (DATPTR), hl               ; DATA current pointer

XBASIC_RESTORE.end:
    call resource.close
  ei
  ret

XBASIC_RESTORE.3.next_sgm:
  push hl
    call XBASIC_RESTORE.next_sgm
  pop de
  ld l, e
  ret

XBASIC_READ_MR:
  ld a, (SGMADR)
  push af
    ld a, (DORES)    ; DATA current segment
    call MR_CHANGE_SGM
    call GET_NEXT_TEMP_STRING_ADDRESS
    ex de, hl
    ld hl, (DATPTR)  ; DATA current pointer
    ld b, 0
XBASIC_READ_MR.repeat:
    push de
      ld c, (hl)
      inc bc
      ldir
      ld a, h
      cp 0xC0
      jr c, XBASIC_READ_MR.cont
        call XBASIC_RESTORE.next_sgm
        pop de
        jr XBASIC_READ_MR.repeat
XBASIC_READ_MR.cont:
      ld (DATPTR), hl
    pop hl
  pop af
  jp MR_CHANGE_SGM

XBASIC_IREAD_MR:
  ld a, (SGMADR)            ; save current segment
  push af
    ld a, (DORES)           ; change to DATA segment
    call MR_CHANGE_SGM
    ld hl, (DATPTR)         ; get DATA current pointer
    ld e, (hl)
    inc hl
    ld d, (hl)
    inc hl
    ld (DATPTR), hl
    ex de,hl
  pop af                    ; restore to last segment
  jp MR_CHANGE_SGM

XBASIC_INPUT_2:
  call INLIN
  jr XBASIC_INPUT_1.do

XBASIC_INPUT_1:
  call QINLIN

XBASIC_INPUT_1.do:
  call XBASIC_GET_BUFFER
  ld de, (TMPSTRBUF)
  ld bc, 0x0000
  push de
    jr c, XBASIC_INPUT_1.end  ; (correct = jp c, end_of_program)
      ldi
      xor a
XBASIC_INPUT_1.do.1:
      cp (hl)
      jr z, XBASIC_INPUT_1.do.2
        ldi
        jr XBASIC_INPUT_1.do.1
XBASIC_INPUT_1.do.2:
      ld a, c
      cpl
      ld c, a
XBASIC_INPUT_1.end:
  pop hl
  ld (hl), c
  ret

; hl = item format string
; c:de = float
XBASIC_USING:
  push bc
  push de
    ld c, 0x80           ; format style (7=1 6=, 5=* 4=$ 3=+ 2=- 1=0  0=^)
    ld de, 0x0000        ; d=thousand digits, e=decimal digits
    ld b, (hl)           ; format string size
XBASIC_USING.loop:
      inc hl
      ld a, (hl)
      cp '0'
      jr z, XBASIC_USING.zero    ; 0 numeric format char, handle
      cp '#'
      jr z, XBASIC_USING.hash    ; # numeric format char, handle
      cp '+'
      jr z, XBASIC_USING.plus    ; +, set sign flag and continue
      cp '-'
      jr z, XBASIC_USING.minus   ; -, set sign after number
      cp '.'
      jr z, XBASIC_USING.dot     ; decimal point
      cp '*'
      jr z, XBASIC_USING.mark    ; mark symbol
      cp '$'
      jr z, XBASIC_USING.curr    ; currency symbol
      cp ','
      jr z, XBASIC_USING.thou    ; thousand separator
      cp '^'
      jr z, XBASIC_USING.expo    ; exponential representation
XBASIC_USING.next:
    djnz XBASIC_USING.loop
    ld a, c
  pop hl
  pop bc
  ld b, c
XBASIC_USING.do:     ; a=format, d=thousand digits, e=decimal digits, b:hl=number
  push af
    push af
    push de
      ex af, af'
        call FloatToBCD          ; convert number to BCD (b:hl to DAC)
      ex af, af'
      and 1                      ; exponential? if bit 0, normalize DAC
      call nz, DECNRM            ; normalize
    pop bc
    pop af
    call SUB_PUFOUT              ; a=format, b=thousand digits, c=decimal digits, DAC=value, out hl=string
  pop af
  and 2                          ; zero format char?
  jr nz, XBASIC_USING.format_zero
XBASIC_USING.string_size:
  ex de, hl                      ; calculate string size
    ld l, e
    ld h, d
    xor a
    cpir
    ld a, l
    sub e
  ex de, hl
XBASIC_USING.end:
  dec hl
  dec a
  ld (hl), a
  ret
XBASIC_USING.format_zero:
  push hl
    ld b, 0
XBASIC_USING.format_zero.loop:
    ld a, (hl)
    cp 32
    jr nz, XBASIC_USING.format_zero.cont
       ld a, '0'
       ld (hl), a
XBASIC_USING.format_zero.cont:
    inc b
    inc hl
    and a
    jr nz, XBASIC_USING.format_zero.loop
    ld a, b
  pop hl
  jr XBASIC_USING.end
XBASIC_USING.plus:             ; +, set sign flag and continue
  set 3, c
  ld a, d
  or d
  jr z, XBASIC_USING.hash
XBASIC_USING.minus:            ; -, set sign after number
  set 2, c
  jr XBASIC_USING.next
XBASIC_USING.mark:             ; *, set sign flag and continue
  set 5, c
  jr XBASIC_USING.hash
XBASIC_USING.curr:             ; currency symbol
  set 4, c
  jr XBASIC_USING.hash
XBASIC_USING.thou:             ; thousand separator
  set 6, c
  jr XBASIC_USING.hash
XBASIC_USING.zero:             ; zero format char
  set 1, c
  jr XBASIC_USING.hash
XBASIC_USING.expo:             ; exponential representation
  set 0, c
XBASIC_USING.hash:             ; # numeric format char, handle
  ld a, e
  or e
  jr z, XBASIC_USING.hash.1
    inc e
    jr XBASIC_USING.next
XBASIC_USING.hash.1:
  inc d
  jr XBASIC_USING.next
XBASIC_USING.dot:              ; decimal point
  inc e
  jr XBASIC_USING.next

; BLOAD STATEMENT
; parameters:
;   DAC   = first block start
;   DAC+2 = segment
;   ARG   = block count
; SCn file format:
;   db #fe 	;ID byte
;   dw {VRAM begin address}
;   dw {VRAM end address}
;   dw {not used when loading to VRAM}
XBASIC_BLOAD:
  call XBASIC_BLOAD.get_next_block
  ld a, c
  or b
  ret z

  inc hl
  ld e, (hl)
  inc hl
  ld d, (hl)

  inc hl
  inc hl
  inc hl
  inc hl
  inc hl

  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc

XBASIC_BLOAD.loop:
  push de
  push bc
    call LDIRVM    ; hl = ram data address, de = vram data address, bc = length (interruptions enabled)
  pop bc
  pop hl
  add hl, bc
  push hl
    call XBASIC_BLOAD.get_next_block
  pop de
  ld a, c
  or b
  ret z

  jr XBASIC_BLOAD.loop

XBASIC_BLOAD.get_next_block:
  ld bc, 0x0000
  ld (ARG+2), bc                  ; bytes read

  ld bc, (ARG)                    ; block count
  ld a, c
  or b
  ret z

  dec bc
  ld (ARG), bc

  di
    call resource.open
      ld hl, (DAC)                ; current block address
      ld a, (DAC+2)               ; current segment
XBASIC_BLOAD.get_next_block.loop:
      or a
      call nz, MR_CHANGE_SGM

      ld a, (hl)
      or a
      jr z, XBASIC_BLOAD.get_next_block.end
        inc hl
        ld e, a
        ld d, 0
        ex de, hl
          add hl, de
          ld a, h
        ex de, hl
        cp 0xC0
        jr c, XBASIC_BLOAD.get_next_block.cont
          ld a, (DAC+2)           ; current segment
          inc a
          inc a
          ld (DAC+2), a           ; current segment
          ld hl, 0x8000
          jr XBASIC_BLOAD.get_next_block.loop
XBASIC_BLOAD.get_next_block.cont:
        ld (DAC), de
        ld de, BUF
        call resource.ram.unpack  ; in hl = packed data, de = ram destination; out bc = size, hl=destination
        ld (ARG+2), bc            ; bytes read
XBASIC_BLOAD.get_next_block.end:
    call resource.close
  ei
  ld hl, BUF
  ld bc, (ARG+2)                  ; bytes read
  ret

