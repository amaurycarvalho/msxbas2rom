;---------------------------------------------------------------------------------------------------------
; ARKOS TRACKER 2 PLAYER
;---------------------------------------------------------------------------------------------------------

PLY_AKM_Play: equ 0x66A8
PLY_AKM_Init: equ 0x65AB
PLY_AKM_Stop: equ 0x6692
PLY_AKM_InitSoundEffects: equ 0x648A
PLY_AKM_PlaySoundEffect: equ 0x648E
PLY_AKM_StopSoundEffectFromChannel: equ 0x64B6

;---------------------------------------------------------------------------------------------------------
; ARKOS TRACKER 2 support routines
;---------------------------------------------------------------------------------------------------------

player.initialize:
  di
    xor a
    ld (PLYSTS), a                   ; idle
    ld (PLYCOUNT), a                 ; player frame count
    ld a, %10000001
    ld (PLYLOOP), a                  ; bit 0=loop on, bit 7=end song
  ei

  ; save old hook
  ld hl, HTIMI      ;OLD HOOK SAVE
  ld de, PLYHKSAV
  ld bc, 5
  ldir

  di
	; set new hook
	ld a, 0xF7          ; rst 0x30 - CALLF
    ld (HTIMI), a
	ld a, (SLTAD1)      ; a = program rom slot
    ld (HTIMI+1), a
    ld hl, player.int.control
	ld (HTIMI+2), hl
	ld a, 0xC9          ; ret
	ld (HTIMI+4), a
  ei

  ret

player.unhook:
  di
    ld hl, PLYHKSAV
    ld de, HTIMI
    ld bc, 5
    ldir
  ei

  jp GICINI

player.int.control:
    push af
      ld a, (PLYSTS)
      cp 1             ; play 60hz
      jr z, player.int.play.60hz

      cp 2             ; play 50hz
      jr z, player.int.play.50hz

      cp 3             ; mute
      jr z, player.int.mute

player.int.exit:
    pop af
    jp PLYHKSAV

player.int.play.60hz:
    ld a, (PLYCOUNT)
    inc a
    cp 6
    jr z, player.int.play.skip
      ld (PLYCOUNT), a

player.int.play.50hz:
    call resource.open
      ld a, (PLYSGTM)
      or a
      call nz, MR_CHANGE_SGM
      call PLY_AKM_Play
    call resource.close
    jr player.int.exit

player.int.play.skip:
    xor a
    ld (PLYCOUNT), a
    jr player.int.exit

player.int.mute:
    call resource.open
      ld a, (PLYSGTM)
      or a
      call nz, MR_CHANGE_SGM
      call PLY_AKM_Stop
    call resource.close
    xor a          ; idle
    ld (PLYSTS), a
    jr player.int.exit

; load song resource in memory
; CMD PLYLOAD <akm resource>, <akx resource>
; (DAC) = akm resource number (songs)
; (ARG) = akx resource number (effects)
cmd_plyload:
  di
    xor a                 ; 0 = idle
    ld (PLYSTS), a
    ld (PLYCOUNT), a
  ei
  halt

  di
    call resource.open_and_get_address  ; out: hl = resource data, a = resource segment, bc = resource size

    ld (PLYSGTM), a

    ld (PLYADDR), hl
    xor a
    call PLY_AKM_Init

    ld a, (RSCMAPSG)
    or a
    call nz, MR_CHANGE_SGM

    ld bc, (ARG)             ; bc = resource number
    call resource.address    ; hl = resource start address, a = segment, bc = size

    call PLY_AKM_InitSoundEffects

    ld a, (PLYLOOP)
    and %00000001            ; clear all flags (except loop flag)
    ld (PLYLOOP), a

    call resource.close
  ei

  halt

  xor a
  ret

; load song resource in memory
; CMD PLYSONG <subsong>
; (DAC) = subsong number
cmd_plysong:
  di
    xor a                 ; 0 = idle
    ld (PLYSTS), a
    ld (PLYCOUNT), a
  ei
  halt

  di
    call resource.open
      ld a, (PLYSGTM)
      or a
      call nz, MR_CHANGE_SGM

      ld hl, (PLYADDR)
      ld a, (DAC)
      call PLY_AKM_Init

      ld a, (PLYLOOP)
      and %00000001          ; clear all flags (except loop flag)
      ld (PLYLOOP), a
    call resource.close
  ei

  halt

  xor a
  ret

; replay song in memory
; CMD PLYREPLAY
cmd_plyreplay:
  di
    xor a                 ; 0 = idle
    ld (PLYSTS), a
    ld (PLYCOUNT), a
    ld a, (PLYLOOP)
    and %00000001          ; clear all flags (except loop flag)
    ld (PLYLOOP), a
  ei
  halt

; play song in memory
; CMD PLYPLAY
cmd_plyplay:
  ld a, (NTSC)
  bit 7, a  ; does bit 7 is off? so its a 60hz machine
  ld a, 1                ; 1 = play at 60hz
  jr z, player.status.set
     inc a               ; 2 = play at 50hz

player.status.set:
  di
    ld (PLYSTS), a
  ei
  halt

  xor a
  ret

; mute song in memory
; CMD PLYMUTE
cmd_plymute:
  ld a, 3         ; mute
  jr player.status.set

; set song loop status
; CMD PLYLOOP <0=off|1=on>
; (DAC) = loop status
cmd_plyloop:
  ld a, (DAC)
  and 1
  ld e, a
  di
    ld a, (PLYLOOP)
    and %11111110
    or e
    ld (PLYLOOP), a
  ei
  halt

  xor a
  ret

; play sound effect in memory
; CMD PLYSOUND <effect>, <channel>, <inverted volume>
; (DAC) = effect number
; (ARG) = voice channel
; (ARG+2) = inverted volume
cmd_plysound:
  di
    call resource.open
      ld a, (PLYSGTM)
      or a
      call nz, MR_CHANGE_SGM

      ld a,  (DAC)
      ld bc, (ARG)
      ld de, (ARG+2)
      ld b, e
      call PLY_AKM_PlaySoundEffect
    call resource.close
  ei
  ret
