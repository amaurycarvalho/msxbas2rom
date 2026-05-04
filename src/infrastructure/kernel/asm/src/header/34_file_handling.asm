; ======================================================================================================
; FILE HANDLING SUPPORT IMPLEMENTATIONS
; ======================================================================================================

; ------------------------------------------------------------------------------------------------------
; preflight disk availability for file i/o
; in : a = drive number (0=A:, 1=B:, ...)
; out: a = 0 if available, otherwise is unavailable
; ------------------------------------------------------------------------------------------------------
cmd_preflight_disk:
  push hl
    ld l, a
    ld h, 0
    add hl, hl                          ; offset = drive * 2
    push de
      ld de, DRVTBL
      add hl, de
    pop de 
    ld a, (hl)
    inc hl
    or (hl)
  pop hl
  ret z                             ; disk unavailable
  xor a                             ; disk available
  ret

; ------------------------------------------------------------------------------------------------------
; BDOS with error handling
; ------------------------------------------------------------------------------------------------------
cmd_bdos_we:
  ex af, af'
  exx
    ; --> copy error handler to RAM
    ld de, HOLD2
    ld hl, cmd_bdos_we.error_handler
    ld bc, cmd_bdos_we.error_handler.end - cmd_bdos_we.error_handler
    ldir
    ; --> copy abort handler to RAM
    ld de, HOLD8 
    ld hl, cmd_bdos_we.abort_handler
    ld bc, cmd_bdos_we.abort_handler.end - cmd_bdos_we.abort_handler
    ldir 
    ; --> set BDOS error/abort handler addresses
    ld hl, (0xF323)             ; save error handler address
    ld (ARG), hl
    ld hl, (0xF1E6)             ; save abort handler address
    ld (ARG + 2), hl
    ld hl, HOLD2                ; error handler address in RAM
    ld (HOLD), hl               ; save error handler address
    ld hl, HOLD                 ; error handler pointer address
    ld (0xF323), hl             ; set error handler pointer address
    ld hl, HOLD8                ; abort handler address in RAM 
    ld (0xF1E6), hl             ; set abort handler address
  exx
  ex af, af'

  ld (DAC), sp                  ; save stack pointer
  call ROMBDOS

cmd_bdos_we.done: 
  ; --> restore default BDOS error/abort handler addresses
  push hl 
    ld hl, (ARG) 
    ld (0xF323), hl             ; error handler address
    ld hl, (ARG + 2)
    ld (0xF1E6), hl             ; abort handler address
  pop hl
  ret 

cmd_bdos_we.error_handler:
  ld a, c                       ; get error code
  ld c, 2                       ; reply = abort
  ret
cmd_bdos_we.error_handler.end:

cmd_bdos_we.abort_handler: 
  and 0x7F                      ; clear bit 7
  neg                           ; turn error code value to negative (A = -A)
  ld sp, (DAC)                  ; restore stack pointer
  ex af, af'
  exx
    ld a, (SLTSTR)                ; kernel slot
    ld h, 0x40                    ; select the ROM on page 4000h
    call ENASLT                   ; restore page 1 to kernel slot
  exx
  ex af, af'
  ei
  jp cmd_bdos_we.done           ; warning: required, because it will run in RAM 
cmd_bdos_we.abort_handler.end: 

; ------------------------------------------------------------------------------------------------------
; GET FCB
; in : a = file number
; out: hl = FCB address, 0 if error
; ------------------------------------------------------------------------------------------------------
cmd_fget_fcb:
  ld ix, BDOS_GET_FCB
  jp CALBAS 

; ======================================================================================================
; FILE HANDLING STATEMENTS/FUNCTIONS IMPLEMENTATIONS
; ======================================================================================================

; ------------------------------------------------------------------------------------------------------
; MAXFILES statement
; in : a = number of user i/o channels
; out: none
; ------------------------------------------------------------------------------------------------------
cmd_fmaxfiles:
  cp 16
  ret nc 
  ld (MAXFIL), a
  push af
cmd_fmaxfiles.set_heap_end:
    ;ld hl, HEAPEND                      ; end of the heap area (HIMEM)
    ld hl, (FCBBASE)
cmd_fmaxfiles.set_himem:
    dec hl 
    ld (HIMEM), hl                      ; highest memory adddress according to CLEAR stmt
cmd_fmaxfiles.set_filtab:
    ld de, -(256+9+2)
cmd_fmaxfiles.set_filtab.loop:
      add hl, de 
      dec a 
    jp p, cmd_fmaxfiles.set_filtab.loop
    ld (FILTAB), hl                     ; start of i/o channel pointers
cmd_fmaxfiles.set_heap_size:
    push hl 
      dec hl 
      ld (MEMSIZ), hl                   ; highest memory address according to BASIC
      ld de, (HEAPSTR)                  ; heap start address
      sbc hl, de                        ; heap size = memsiz - heap start
      ld (HEAPSIZ), hl                  ; new heap size
    pop de 
cmd_fmaxfiles.populate_filtab:
  pop af
  ld l, a
  inc l                                 ; number of i/o channels
  ld h, 0
  add hl, hl                            ; *2
  add hl, de                            ; +start of i/o channel pointers
  ex de, hl 
  push de                               ; start of i/o channel buffers
    ld bc, -2+256+9+2
cmd_fmaxfiles.populate_filtab.loop:
      ld (hl), e 
      inc hl 
      ld (hl), d 
      inc hl                            ; pointer to i/o channel buffer
      ex de, hl 
        ld (hl), 0                      ; i/o channel closed
        add hl, bc                      ; to the next i/o channel buffer
      ex de, hl 
      dec a 
    jp p, cmd_fmaxfiles.populate_filtab.loop ; next i/o channel
  pop hl                                ; start of i/o channel buffer
  ld bc, 9
  add hl, bc
  ld (NULBUF), hl                       ; pointer to the i/o channel 0 buffer
  ret

; ------------------------------------------------------------------------------------------------------
; DSKF function
; in : a = disk number (0 = default drive, 1=A, 2=B...)
; out: hl = free clusters (negative = disk error)
; ------------------------------------------------------------------------------------------------------
cmd_fdskf:
  ; --> check disk number parameter
  cp 9
  jr nc, cmd_fdskf.error                ; error if a >= 9
  ; --> check disk
  call cmd_preflight_disk
  or a
  jr nz, cmd_fdskf.error                ; error if a != 0
  ; --> check drivers
  ld c, 0x18                            ; BDOS GetLoginVector (return drivers flag in L)
  push hl 
    call ROMBDOS
  pop de
  ld a, l 
  and a 
  jr z, cmd_fdskf.error                 ; error if A = 0
  ; ---> get disk information
  ld c, 0x1B                            ; BDOS GetAllocationInfo (e = drive number)
  call cmd_bdos_we                      ; out: a = sectors per cluster (0xFF if error)
                                        ;      hl = free clusters
  cp 0x80
  ret c                                 ; return if success

cmd_fdskf.error:
  ; --> disk error fallback
  ld l, a
  ld h, 0xFF
  ret

; ------------------------------------------------------------------------------------------------------
; OPEN statement
; in : a = file number
;      hl = filename (pascal string)
;      e = file mode (1=input, 2=output, 4=random, 8=append)
;      bc = record length
; out: hl = true if success, false if error
; reference: BDOS 0x0F (OpenFile) and 0x16 (CreateFile)
; ------------------------------------------------------------------------------------------------------
cmd_fopen:
  push af
  push de
  push bc
    ld a, (hl)        
    inc hl 
    ld (ARG), a        ; string size 
    ld (ARG+1), hl     ; string pointer
    ld hl, ARG         ; string descriptor
    ld a, 3
    ld (VALTYP), a     ; DAC type = string 
    ld (DAC+2), hl     ; DAC = string descriptor
    ld hl, BDOS_EMPTY_LINE
    ld ix, BDOS_FILEVL ; in hl=BASIC pointer, DAC = string descriptor; out d = device
    call CALBAS
  pop bc 
  pop hl 
  pop af 
  ld e, l 
  ld hl, BDOS_EMPTY_LINE
  ld ix, BDOS_OPEN ; in: a = i/o number, e = filemode, d = devicecode, hl = BASIC pointer
  jp CALBAS 

; ------------------------------------------------------------------------------------------------------
; EOF function
; in : a = file number
; out: hl = true or false
; ------------------------------------------------------------------------------------------------------
cmd_feof:
  call cmd_fget_fcb 
  call HEOF              ; in: hl=FCB, out: DAC+2
  ld hl, (DAC+2)         ; true or false
  ret

; ------------------------------------------------------------------------------------------------------
; CLOSE statement
; in : a = file number (0xFF = all)
; out: a = 0 if success, otherwise error
; reference: BDOS 0x10 (CloseFile)
; ------------------------------------------------------------------------------------------------------
cmd_fclose:
  ld bc, BDOS_CLOSE  ; in: a = file number
  push bc 
  pop ix
  cp 0xFF
  jr nz, cmd_fclose.exec
    ld hl, BDOS_EMPTY_LINE
    ld a, (MAXFIL)
    ld ix, BDOS_CLOSE_ALL  ; in: hl = BASIC pointer, a = (MAXFIL), bc = BDOS_CLOSE
cmd_fclose.exec:
  jp CALBAS 

; ------------------------------------------------------------------------------------------------------
; INPUT# statement
; in : a = file number
; out: hl = true if success, false if error
; reference: BDOS 0x14 (SequentialReadFile) and 0x21 (RandomReadFile)
; ------------------------------------------------------------------------------------------------------
cmd_finput:
  ret

; input from CON:
cmd_finput.con:
  ret

; input from COMn:
cmd_finput.com:
  ret

; input from CAS:
cmd_finput.cas:
  ret

; ------------------------------------------------------------------------------------------------------
; PRINT# statement
; in : a = file number
; out: hl = true if success, false if error
; reference: BDOS 0x15 (SequentialWriteFile) and 0x22 (RandomWriteFile)
; ------------------------------------------------------------------------------------------------------
cmd_fprint:
  ret

; print to GRP:
cmd_fprint.grp:
  ret

; print to CRT: / CON:
cmd_fprint.con:
  ret

; print to COMn:
cmd_fprint.com:
  ret

; print to LPT: / PRN: / LST:
cmd_fprint.lpt:
  ret

; print to CAS:
cmd_fprint.cas:
  ret
