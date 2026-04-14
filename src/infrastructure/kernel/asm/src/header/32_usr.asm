; ------------------------------------------------------------------------------------------------------
; USR() IMPLEMENTATION
; ------------------------------------------------------------------------------------------------------

; get resource data address
; in hl = parameter
; out hl = address
usr0:
  di
    push hl
      call resource.open
    pop bc
    call resource.address    ; hl = resource start address, a = segment, bc = size
    push hl
      call resource.close
    pop bc
  ei
  jp c, usr_def
  jp usr_def.exit

; get resource data size
; in hl = parameter
; out hl = size
usr1:
  di
    push hl
      call resource.open
    pop bc
    call resource.address    ; hl = address, a = segment, bc = resource size
    push bc
      call resource.close
    pop bc
  ei
  jp c, usr_def
  jp usr_def.exit

; multi function root
; hl = parameter
usr2:
  ld a, l
  or a
  jr z, usr2_play
  dec a ; 1
  jp z, usr2_inkey
  dec a ; 2
  jp z, usr2_input
  dec a ; 3
  jp z, usr2_player_status
  dec a ; 4
  jp z, usr2_vdp
  dec a ; 5
  jp z, usr2_turbo
  dec a ; 6
  jp z, usr2_maker
  jp usr_def


; PLAY() function alternative
; hl = parameter
usr2_play:
  ld e, h
  ld a, (MUSICF)
  dec e
  jp m, usr2.3

usr2.0:
  rrca
  dec e
  jp p, usr2.0
  ld a, 0
  jr nc, usr2.2

usr2.1:
  dec a

usr2.2:
  ld c, a
  rla
  sbc a, a
  ld b, a
  jp usr_def.exit

usr2.3:
  and 7
  jr z, usr2.2
  ld a, 0xff
  jr usr2.2

; INKEY() function alternative
usr2_inkey:
  call CHSNS
  jp z, usr_def

; INPUT() function alternative
usr2_input:
  call CHGET
  jp usr_def.ok

usr2_player_status:
  ld a, (PLYLOOP)
  jp usr_def.ok

usr2_vdp:
  call VDP_GetVersion
  jp usr_def.ok

usr2_turbo:
  ;in a, (64)
  ;cp 247            ; 8 - panasonic
  ;jp nz, usr_def    ; turbo works only on panasonic machines
  ld a, (VERSION)
  cp 2              ; msx2+ or turbo R
  jp c, usr_def     ; not a turbo capable machine
  jr z, usr2_turbo.msx2p
usr2_turbo.turboR:  ; R800 processor type
    ld a, (GETCPU)
	cp 0xC3
	jp nz, usr_def  ; BIOS function GETCPU doesnt exist
	call GETCPU     ; 0=z80, 1=R800 rom, 2=R800 dram *just only at boot time
    jp usr_def.ok
usr2_turbo.msx2p:   ; msx2+ machine type
  in a, (65)
  and 1
  xor 1
  jp usr_def.ok

usr2_maker:
  in a, (64)
  cpl
  jp usr_def.ok

; sprite collision detection algorithm
; hl = parameter (h: sprite 1, l: sprite 2, hl < 0: all sprites, h = 0: sprite against all the rest)
usr3:
  call usr3.test
  call usr3.return
  ld (DAC+2), hl
  ret
usr3.return:
  ld l, a              ; collisor sprite
  cp 0xFF
  jr z, usr3.return.1
    xor a              ; collision occurred
usr3.return.1:
  ld h, a
  ret

usr3.test:
  xor a
  or h                     ; check if direct test of two sprite numbers
  jp z, SUB_SPRCOL_ONE
    bit 7, h               ; check if sprite number is negative (loop all sprites if so)
    jp nz, SUB_SPRCOL_ALL
      ld e, h              ; second sprite
      jp SUB_SPRCOL_COUPLE

; no parameters
usr3.COLLISION_ALL:
  call SUB_SPRCOL_ALL
  jr usr3.return

; hl = sprite 1, de = sprite 2
usr3.COLLISION_COUPLE:
  call SUB_SPRCOL_COUPLE
  jr usr3.return

; hl = sprite 1
usr3.COLLISION_ONE:
  call SUB_SPRCOL_ONE
  jr usr3.return

; default
usr_def:
  xor a
usr_def.ok:
  ld b, 0
  ld c, a
usr_def.exit:
  ld l, c
  ld h, b
  ld (DAC+2), hl
  ret

