CHPUT equ 0x00A2
DAC equ 0xF7F8

start:
  ld hl,(DAC) ; parameter (string address)
  ld b, (hl)  ; string size
  inc hl      ; go to the first character on the string
loop:
  ld a, (hl)  ; print character
  call CHPUT
  inc hl      ; next character
  djnz loop
  ret

