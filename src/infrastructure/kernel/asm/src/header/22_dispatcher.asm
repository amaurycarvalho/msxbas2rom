;---------------------------------------------------------------------------------------------------------
; PAGED KERNEL DISPATCHER
; Stub ABI: call dispatcher / db segment / dw address
;
; The dispatcher saves the caller argument registers, reads the inline
; segment/address, restores the argument registers, switches segment, calls
; the target, restores the previous segment, and returns to the stub caller.
; The scratch area is safe for nesting: it is no longer needed once the
; target starts; nested calls use the stack for their own return frames.
;---------------------------------------------------------------------------------------------------------

dispatcher:
  ld (DISP_SAVE_HL), hl
  ld (DISP_SAVE_BC), bc
  ld (DISP_SAVE_DE), de
  ld (DISP_SAVE_IX), ix
  push af
  pop hl
  ld (DISP_SAVE_AF), hl

  pop hl                    ; hl = return address into the inline stub
  ld a, (hl)                ; inline segment
  inc hl
  ld e, (hl)
  inc hl
  ld d, (hl)                ; inline target address
  ld (DISP_SEG), a
  ld (DISP_TARGET), de

  ld a, (SGMADR)
  push af                   ; save caller segment for the cleanup routine
  ld hl, dispatcher_return
  push hl                   ; target return address
  ld hl, (DISP_TARGET)
  push hl                   ; target address

  ld bc, (DISP_SAVE_BC)
  ld de, (DISP_SAVE_DE)
  ld ix, (DISP_SAVE_IX)
  ld hl, (DISP_SAVE_AF)
  push hl
  pop af
  ld hl, (DISP_SAVE_HL)

  push af                   ; preserve caller A across segment selection
  ld a, (DISP_SEG)
  call MR_CHANGE_SGM
  pop af
  ret                       ; enter the paged target

dispatcher_return:
  ld (DISP_RESULT_A), a     ; preserve target A result across segment restore
  pop af                    ; caller segment
  call MR_CHANGE_SGM
  ld a, (DISP_RESULT_A)
  ret                       ; return to the stub caller
