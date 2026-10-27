;---------------------------------------------------------------------------------------------------------
; CMD RSCTORAM (fixed module)
;---------------------------------------------------------------------------------------------------------
; The routine reads a resource through resource.open_and_get_address, which maps
; the resource into page 2 on MegaROM. It must stay resident in the fixed kernel
; page and must never be placed in a page-2 overflow segment.
;
; Parameters are read from PARM1 (same layout as the CMD handler):
;   PARM1+0 resource number (word)
;   PARM1+2 destination address (word)
;   PARM1+4 pletter flag (word; low byte used)
;   PARM1+6 offset (word)
;   PARM1+8 size (word; 0 = full remaining)
;---------------------------------------------------------------------------------------------------------

RSC_RESOURCE equ PARM1
RSC_DEST     equ PARM1+2
RSC_PLETTER  equ PARM1+4
RSC_OFFSET   equ PARM1+6
RSC_SIZE     equ PARM1+8

cmd_rsctoram:
  di
  ld hl, (RSC_RESOURCE)
  ld (DAC), hl
  call resource.open_and_get_address
  jr c, cmd_rsctoram.fail
  ld a, (RSC_PLETTER)
  or a
  jr nz, cmd_rsctoram.unpack

  ; offset < resource_size (16-bit compare)
  ld de, (RSC_OFFSET)
  ld a, e
  sub c
  ld a, d
  sbc a, b
  jr nc, cmd_rsctoram.fail
  add hl, de

  ; remaining = resource_size - offset
  ld a, c
  sub e
  ld c, a
  ld a, b
  sbc a, d
  ld b, a

  ; effective = (size == 0) ? remaining : min(size, remaining)
  ld de, (RSC_SIZE)
  ld a, d
  or a
  jr nz, cmd_rsctoram.size_given
  ld a, e
  or a
  jr z, cmd_rsctoram.copy
cmd_rsctoram.size_given:
  ld a, e
  sub c
  ld a, d
  sbc a, b
  jr nc, cmd_rsctoram.copy
  ld b, d
  ld c, e
cmd_rsctoram.copy:
  ld de, (RSC_DEST)
  ldir
cmd_rsctoram.done:
  call resource.close
  ld hl, (RSC_DEST)
  ei
  ret
cmd_rsctoram.unpack:
  ld de, (RSC_DEST)
  call resource.ram.unpack
  jr cmd_rsctoram.done
cmd_rsctoram.fail:
  call resource.close
  ld hl, 0
  ei
  ret

; Keep the fixed addresses after this module unchanged. The tile bank
; normalization grew 31-cmd-tile-sprite by 23 bytes, so this filler is reduced
; by the same amount.
    DEFS 56, 0x00
