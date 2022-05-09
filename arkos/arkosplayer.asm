  org #6481     ;This is the fixed address of the player/music
PlayerAndMusicBinary_Start
  PLY_AKM_HARDWARE_MSX = 1
  PLY_AKM_MANAGE_SOUND_EFFECTS = 1
  PLY_AKM_Rom = 1
  PLY_AKM_ROM_Buffer = #EF00
  include "PlayerAkm.asm"  ;It works for any other player.

; search for LinkerPostPt label in original arkos player
; and put a call to the code below there (after "End of song" tag)
;player_end_song:
;    AKMSTS = #C011
;    AKMLOOP = #C012
;    ld a, (AKMLOOP)
;    or #80               ; set bit 7
;    ld (AKMLOOP), a
;    and 1                ; test bit 0 (1=loop)
;    ret nz
;      ld a, 3            ; stop song
;      ld (AKMSTS), a
;      ret


PlayerAndMusicBinary_End
