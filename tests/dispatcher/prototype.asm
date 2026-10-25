; Dispatcher ABI prototype for the central jump table.
; Runs as a plain 16 KiB ROM and publishes test results in RAM.

SGMADR      equ 0xC023
SWITCHES    equ 0xC008
HTIMI       equ 0xFD9F
RESULT    equ 0xC000
DISP_SCRATCH:  equ 0xC040
DISP_SAVE_AF:  equ DISP_SCRATCH
DISP_SAVE_BC:  equ DISP_SAVE_AF+2
DISP_SAVE_DE:  equ DISP_SAVE_BC+2
DISP_SAVE_HL:  equ DISP_SAVE_DE+2
DISP_SAVE_IX:  equ DISP_SAVE_HL+2
DISP_TARGET:   equ DISP_SAVE_IX+2
DISP_SEG:      equ DISP_TARGET+2
DISP_RESULT_A: equ DISP_SEG+1

org 0x4000
  db "AB"
  dw start
  dw 0
  dw 0
  dw 0
  db "DSPT"
  db 0, 0

start:
  ; clear result area
  ld hl, RESULT
  ld bc, 0x0020
  xor a
clear_results:
  ld (hl), a
  inc hl
  dec bc
  ld a, b
  or c
  ld a, 0
  jr nz, clear_results

  xor a
  ld (SGMADR), a

  ld a, 0
  ld (SWITCHES), a

  di
  ld hl, timer_isr
  ld (HTIMI+1), hl
  ld a, 0xC3
  ld (HTIMI), a
  ei

  call resident_stub

  ld hl, 0x1234
  ld de, 0x5678
  ld bc, 0x9ABC
  ld a, 0x12
  ld bc, 0x9ABC
  ld de, 0x5678
  ld hl, 0x1234
  ld ix, 0x4321
  call paged_stub
  ld (0xC010), hl
  ld (0xC012), de
  ld (0xC014), bc
  ld (0xC02A), a

  call nested_stub

  call timer_stub

  xor a
  ld (0xC006), a
  ld a, 1
  or a
  call nz, conditional_stub

  ld a, (SGMADR)
  ld (0xC005), a

  ld a, 0x5A
  ld (RESULT), a

loop:
  jr loop

resident_stub:
  jp resident_target

paged_stub:
  call dispatcher
  db 3
  dw paged_target

nested_stub:
  call dispatcher
  db 4
  dw nested_target

conditional_stub:
  call dispatcher
  db 6
  dw conditional_target

timer_stub:
  call dispatcher
  db 7
  dw timer_target

dispatcher:
  ld (DISP_SAVE_HL), hl
  ld (DISP_SAVE_BC), bc
  ld (DISP_SAVE_DE), de
  ld (DISP_SAVE_IX), ix
  push af
  pop hl
  ld (DISP_SAVE_AF), hl

  pop hl
  ld a, (hl)
  inc hl
  ld e, (hl)
  inc hl
  ld d, (hl)
  ld (DISP_SEG), a
  ld (DISP_TARGET), de

  ld a, (SGMADR)
  push af
  ld hl, dispatcher_return
  push hl
  ld hl, (DISP_TARGET)
  push hl

  ld bc, (DISP_SAVE_BC)
  ld de, (DISP_SAVE_DE)
  ld ix, (DISP_SAVE_IX)
  ld hl, (DISP_SAVE_AF)
  push hl
  pop af
  ld hl, (DISP_SAVE_HL)

  push af
  ld a, (DISP_SEG)
  call MR_CHANGE_SGM
  pop af
  ret

dispatcher_return:
  ld (DISP_RESULT_A), a
  pop af
  call MR_CHANGE_SGM
  ld a, (DISP_RESULT_A)
  ret

resident_target:
  ld hl, 0xC001
  inc (hl)
  ret

paged_target:
  ld (0xC080), a
  ld (0xC082), bc
  ld (0xC084), de
  ld (0xC086), hl
  ld (0xC088), ix
  ld hl, 0xC002
  inc (hl)
  ld a, 0x5A
  ld hl, 0xAAAA
  ld de, 0xBBBB
  ld bc, 0xCCCC
  ret

nested_target:
  ld hl, 0xC003
  inc (hl)
  call nested_stub2
  ret

nested_stub2:
  call dispatcher
  db 5
  dw nested_target2

nested_target2:
  ld hl, 0xC004
  inc (hl)
  ret

conditional_target:
  ld hl, 0xC006
  inc (hl)
  ret

timer_target:
  ld hl, 0xC00A
  inc (hl)
  ld bc, 0xFFFF
timer_target.spin:
  dec bc
  ld a, b
  or c
  jr nz, timer_target.spin
  ret

timer_isr:
  push af
  push hl
    ld a, (SGMADR)
    push af
      ld a, 0x7F
      ld (SGMADR), a
      ld hl, 0xC009
      inc (hl)
    pop af
    ld (SGMADR), a
  pop hl
  pop af
  ret

MR_CHANGE_SGM:
  ld (SGMADR), a
  push af
    ld a, (SWITCHES)
    inc a
    ld (SWITCHES), a
  pop af
  ret

  DEFS 0x8000 - $, 0
