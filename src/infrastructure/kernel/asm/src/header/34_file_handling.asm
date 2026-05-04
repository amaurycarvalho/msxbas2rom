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
  ret z                                 ; disk unavailable
  xor a                                 ; disk available
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
    ld hl, (0xF323)                     ; save error handler address
    ld (ARG), hl
    ld hl, (0xF1E6)                     ; save abort handler address
    ld (ARG + 2), hl
    ld hl, HOLD2                        ; error handler address in RAM
    ld (HOLD), hl                       ; save error handler address
    ld hl, HOLD                         ; error handler pointer address
    ld (0xF323), hl                     ; set error handler pointer address
    ld hl, HOLD8                        ; abort handler address in RAM 
    ld (0xF1E6), hl                     ; set abort handler address
  exx
  ex af, af'

  ld (DAC), sp                          ; save stack pointer
  call ROMBDOS  

cmd_bdos_we.done: 
  ; --> restore default BDOS error/abort handler addresses
  push hl 
    ld hl, (ARG) 
    ld (0xF323), hl                     ; error handler address
    ld hl, (ARG + 2)
    ld (0xF1E6), hl                     ; abort handler address
  pop hl
  ret 

cmd_bdos_we.error_handler:
  ld a, c                               ; get error code
  ld c, 2                               ; reply = abort
  ret
cmd_bdos_we.error_handler.end:

cmd_bdos_we.abort_handler: 
  and 0x7F                              ; clear bit 7
  neg                                   ; turn error code value to negative (A = -A)
  ld sp, (DAC)                          ; restore stack pointer
  ex af, af'
  exx
    ld a, (SLTSTR)                      ; kernel slot
    ld h, 0x40                          ; select the ROM on page 4000h
    call ENASLT                         ; restore page 1 to kernel slot
  exx
  ex af, af'
  ei
  jp cmd_bdos_we.done                   ; warning: required, because it will run in RAM 
cmd_bdos_we.abort_handler.end: 

; ------------------------------------------------------------------------------------------------------
; RESET BASIC INTERPRETER REDIRECT
; ------------------------------------------------------------------------------------------------------
cmd_freset_fil:
  ld hl, 0 
  ld (PTRFIL), hl  
  ret

; ------------------------------------------------------------------------------------------------------
; Sequential output
; in : a = character
; ------------------------------------------------------------------------------------------------------
cmd_ffilout:
  ld ix, BDOS_FILOUT
  jp cmd_fsetfil.calbas

; ------------------------------------------------------------------------------------------------------
; Sequential input
; out: a = character
; ------------------------------------------------------------------------------------------------------
cmd_findskc:
  ld ix, BDOS_INDSKC
  jp cmd_fsetfil.calbas

; ------------------------------------------------------------------------------------------------------
; Set BASIC interpreter i/o channel
; in : a = i/o channel
; ------------------------------------------------------------------------------------------------------
cmd_fsetfil:
  ld ix, BDOS_SETFIL
cmd_fsetfil.calbas:
  push bc
  push de
  push hl
    call CALBAS 
  pop hl
  pop de
  pop bc
  ret

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
    ;ld hl, HEAPEND                     ; end of the heap area (HIMEM)
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
    ld (ARG), a                         ; string size 
    ld (ARG+1), hl                      ; string pointer
    ld hl, ARG                          ; string descriptor
    ld a, 3
    ld (VALTYP), a                      ; DAC type=string 
    ld (DAC+2), hl                      ; DAC=string descriptor
    ld hl, BDOS_EMPTY_LINE
    ld ix, BDOS_FILEVL                  ; in hl=BASIC pointer, DAC=string descriptor; out d=device id
    call CALBAS
  pop bc 
  pop hl 
  pop af 
  ld e, l 
  ld hl, BDOS_EMPTY_LINE
  ld ix, BDOS_OPNFIL                    ; in: a=i/o number, e=file mode, d=device id, hl=BASIC pointer
  jp CALBAS 

; ------------------------------------------------------------------------------------------------------
; EOF function
; in : hl = file number
; out: hl = true or false
; ------------------------------------------------------------------------------------------------------
cmd_feof:
  ld ix, BDOS_EOF 
cmd_fcall.function:
  ld (DAC+2), hl 
  jp CALBAS

; ------------------------------------------------------------------------------------------------------
; LOC function
; in : a = file number
; out: hl = number of bytes that have been read (sequential) or record number (random)
; ------------------------------------------------------------------------------------------------------
cmd_floc:
  ld ix, BDOS_LOC
  jr cmd_fcall.function

; ------------------------------------------------------------------------------------------------------
; LOF function
; in : a = file number
; out: hl = size of a file on disk in bytes
; ------------------------------------------------------------------------------------------------------
cmd_flof:
  ld ix, BDOS_LOF
  jr cmd_fcall.function

; ------------------------------------------------------------------------------------------------------
; FPOS function
; in : a = file number
; out: hl = current position of the file pointer within the specified file
; ------------------------------------------------------------------------------------------------------
cmd_fpos:
  ld ix, BDOS_FPOS
  jr cmd_fcall.function

; ------------------------------------------------------------------------------------------------------
; CLOSE statement
; in : a = file number (0xFF = all)
; out: a = 0 if success, otherwise error
; reference: BDOS 0x10 (CloseFile)
; ------------------------------------------------------------------------------------------------------
cmd_fclose:
  ld bc, BDOS_CLSFIL                    ; in: a = file number
  push bc 
  pop ix
  cp 0xFF
  jr nz, cmd_fclose.exec
    ld hl, BDOS_EMPTY_LINE
    ld a, (MAXFIL)
    ld ix, BDOS_CLSALL                  ; in: hl=BASIC pointer, a=(MAXFIL), bc=BDOS_CLOSE
cmd_fclose.exec:
  jp CALBAS 

; ------------------------------------------------------------------------------------------------------
; INPUT# statement
; in : a=file number, hl=string address (pascal style), e=mode (0=INPUT#, 1=LINE INPUT#)
; out: hl=string address (pascal style)
; reference: BDOS 0x14 (SequentialReadFile) and 0x21 (RandomReadFile)
; ------------------------------------------------------------------------------------------------------
cmd_finput:
  call cmd_fsetfil
  ld a, e
  ld (ARG), a                           ; keep read mode
  xor a 
  ld (hl), a
  ld e, l 
  ld d, h 
  ld bc, cmd_finput.begin 
cmd_finput.begin:
  call cmd_findskc
  cp BDOS_EOF_FLAG
  jr z, cmd_finput.end
  cp 0x0D                               ; CR
  jr z, cmd_finput.exec
  cp 0x0A                               ; LF 
  jr z, cmd_finput.exec
  push af
  ld a, (ARG)
  and a
  jr nz, cmd_finput.keep_char           ; LINE INPUT#: only CR/LF are delimiters
  pop af
  cp 0x2C                               ; , 
  jr z, cmd_finput.exec
  cp 0x22                               ; "
  jr z, cmd_finput.exec
  jr cmd_finput.append
cmd_finput.keep_char:
  pop af
cmd_finput.append:
  ld (hl), a                            ; next character
  inc hl
  ld a, (de)                            ; string length
  inc a 
  ld (de), a
  cp 0xFF
  jr z, cmd_finput.end                  ; return if maximum string size reached
  ld bc, cmd_finput.end
  jr cmd_finput.begin 
cmd_finput.exec:
  push bc 
  ret
cmd_finput.end:
  ex de, hl                             ; return string address (hl)
  ret 

; ------------------------------------------------------------------------------------------------------
; PRINT# statement
; in : a=file number, hl=string address (pascal style), e=prefix, d=suffix
; reference: BDOS 0x15 (SequentialWriteFile) and 0x22 (RandomWriteFile)
; ------------------------------------------------------------------------------------------------------
cmd_fprint:
  call cmd_fsetfil
  push de 
    ld a, e
    or a                                ; prefix? i.e: 0x2C (comma)
    call nz, cmd_ffilout
    ld a, 0x22                          ; "
    call cmd_ffilout
    ld a, (hl)
    or a 
    jr z, cmd_fprint.end
    ld b, a
    inc hl 
cmd_fprint.loop:
      ld a, (hl)
      call cmd_ffilout                  ; print string next character
      inc hl
    djnz cmd_fprint.loop
cmd_fprint.end:
    ld a, 0x22                          ; "
    call cmd_ffilout 
  pop af 
  or a                                  ; suffix? e.i: 0x0A (LF)
  ret z 
  jp cmd_ffilout
