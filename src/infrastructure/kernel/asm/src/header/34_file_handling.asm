; ======================================================================================================
; FILE HANDLING SUPPORT IMPLEMENTATIONS
; ======================================================================================================

; ------------------------------------------------------------------------------------------------------
; preflight disk availability for file i/o
; in : a = drive number (0=A:, 1=B:, ...)
; out: a = 0 if available, otherwise is unavailable
; ------------------------------------------------------------------------------------------------------
cmd_preflight_disk:
  ; --> check disk number parameter
  cp 9
  ret nc                                ; return error if a >= 9
  ; --> check disk availability
  push hl
    ld l, a
    ld h, 0
    add hl, hl                          ; offset = drive * 2
    push de                             ; save caller's DE
      ld de, DRVTBL
      add hl, de
    pop de                              ; restore caller's DE
    ld a, (hl)
    inc hl
    or (hl)
  pop hl
  ret z                                 ; disk unavailable
  xor a                                 ; disk available
  ret

; ------------------------------------------------------------------------------------------------------
; Call BASIC with error handling
; ------------------------------------------------------------------------------------------------------
cmd_fcalbas_we:
  ex af, af'
  exx
    ; --> reset error code
    xor a 
    ld (ERRFLG), a                      ; reset error code 
    ; --> set Disk BASIC error handler
    ld a, 0xC3                          ; Z80 jp 
    ld (HERRO), a 
    ld de, PARM2
    ld (HERRO+1), de 
    ; --> copy error handler to RAM
    ld hl, cmd_fcalbas_we.error_handler
    ld bc, cmd_fcalbas_we.error_handler.end - cmd_fcalbas_we.error_handler
    ldir
  exx
  ex af, af'
  ld (PARM2+50), sp                     ; save stack pointer
  call CALBAS  
cmd_fcalbas_we.done: 
  ; --> restore default BASIC error handler
  push af 
    ld a, 0xC9                          ; Z80 ret
    ld (HERRO), a 
  pop af
  ret 

cmd_fcalbas_we.error_handler:
  ld sp, (PARM2+50)                     ; restore stack pointer
  ex af, af'
    ld a, e                             ; copy error code 
    ld (ERRFLG), a
    exx
      ld a, (SLTSTR)                    ; kernel slot
      ld h, 0x40                        ; select the ROM on page 4000h
      call ENASLT                       ; restore page 1 to kernel slot
    exx
  ex af, af'
  ei
  jp cmd_fcalbas_we.done                 ; warning: required, because it will run in RAM 
cmd_fcalbas_we.error_handler.end:

; ------------------------------------------------------------------------------------------------------
; Call BASIC saving registers
; ------------------------------------------------------------------------------------------------------
cmd_fcalbas:
  push bc
  push de
  push hl
    call CALBAS 
  pop hl
  pop de
  pop bc
  ret

; ------------------------------------------------------------------------------------------------------
; Turn off BASIC interpreter i/o redirect
; ------------------------------------------------------------------------------------------------------
cmd_freset_fil:
  ld ix, BDOS_FINPRT
  jr cmd_fcalbas

; ------------------------------------------------------------------------------------------------------
; Sequential output
; in : a = character
; ------------------------------------------------------------------------------------------------------
cmd_ffilout:
  ld ix, BDOS_FILOUT
  jr cmd_fcalbas

; ------------------------------------------------------------------------------------------------------
; Sequential input
; out: a = character
; ------------------------------------------------------------------------------------------------------
cmd_findskc:
  ld ix, BDOS_INDSKC
  jr cmd_fcalbas

; ------------------------------------------------------------------------------------------------------
; Set BASIC interpreter i/o channel
; in : a = i/o channel
; ------------------------------------------------------------------------------------------------------
cmd_fsetfil:
  ld ix, BDOS_SETFIL
  jr cmd_fcalbas

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
      or a                              ; clear carry
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
; OPEN statement
; in : a = file number
;      hl = filename (pascal string)
;      e = file mode (1=input, 2=output, 4=random, 8=append)
;      bc = record length
; out: hl = true if success, false if error
; reference: BDOS 0x0F (OpenFile) and 0x16 (CreateFile)
; ------------------------------------------------------------------------------------------------------
FOPEN_STRING_DESCRIPTOR: equ PARM1
cmd_fopen:
  or a 
  jr z, cmd_fopen.error
  push af
  push de
  push bc
    ld a, (hl)        
    inc hl 
    ld (FOPEN_STRING_DESCRIPTOR), a                       ; string size 
    ld (FOPEN_STRING_DESCRIPTOR+1), hl                    ; string pointer
    ld hl, FOPEN_STRING_DESCRIPTOR                        ; string descriptor
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
  call cmd_fcalbas_we                   ; call BASIC with error handling
  jp cmd_freset_fil                     ; turn off BASIC interpreter i/o redirect
cmd_fopen.error:
  ld a, 52                              ; bad file number error
  ld (ERRFLG), a
  ret 

; ------------------------------------------------------------------------------------------------------
; EOF function
; in : hl = file number
; out: hl = true (FFFF) or false (0)
;      DAC+2 = integer value
; ------------------------------------------------------------------------------------------------------
cmd_feof:
  ld ix, BDOS_EOF 
  call cmd_fcall.function
  or a 
  ret z                                 ; return if no error
  ld hl, 0xFFFF                         ; return true if error                                        
  ret 

cmd_fcall.function:
  ld (DAC+2), hl                        ; file number
  ld a, 2                               ; integer value
  ld (VALTYP), a
  call cmd_fcalbas_we
  ld a, (ERRFLG) 
  ld hl, (DAC+2)
  ret

; ------------------------------------------------------------------------------------------------------
; LOC function
; in : a = file number
; out: hl = number of bytes that have been read (sequential) or record number (random)
;      DAC = BCD value
; ------------------------------------------------------------------------------------------------------
cmd_floc:
  ld ix, BDOS_LOC
  jr cmd_fcall.function

; ------------------------------------------------------------------------------------------------------
; LOF function
; in : a = file number
; out: hl = size of a file on disk in bytes 
;      DAC = BCD value
; ------------------------------------------------------------------------------------------------------
cmd_flof:
  ld ix, BDOS_LOF
  jr cmd_fcall.function

; ------------------------------------------------------------------------------------------------------
; FPOS function
; in : a = file number
; out: hl = current position of the file pointer within the specified file
;      DAC = BCD value
; ------------------------------------------------------------------------------------------------------
cmd_fpos:
  ld ix, BDOS_FPOS
  jr cmd_fcall.function

; ------------------------------------------------------------------------------------------------------
; DSKF function
; in : hl and a = disk number (0 = default drive, 1=A, 2=B...)
; out: hl = free clusters (negative = disk error)
; ------------------------------------------------------------------------------------------------------
cmd_fdskf:
  ; --> check disk
  call cmd_preflight_disk
  or a
  jr nz, cmd_fdskf.error                ; error if a != 0
  ; --> call DSKF function
  ld ix, BDOS_DSKF
  call cmd_fcall.function
  or a 
  ret z                                 ; return if no error
cmd_fdskf.error:
  ; --> disk error fallback
  and 0x7F                              ; clear bit 7
  neg                                   ; turn error code value to negative (A = -A)
  ld l, a
  ld h, 0xFF
  ret
  
; ------------------------------------------------------------------------------------------------------
; CLOSE statement
; in : a = file number (0xFF = all)
; out: a = 0 if success, otherwise error
; reference: BDOS 0x10 (CloseFile)
; ------------------------------------------------------------------------------------------------------
cmd_fclose:
  ld hl, BDOS_EMPTY_LINE
  ld ix, BDOS_CLSFIL                    ; in: a = file number, hl=BASIC pointer
  cp 0xFF
  jr z, cmd_fclose.all
  call cmd_fcalbas_we
  jp cmd_freset_fil
cmd_fclose.all:
    push ix
    pop bc 
    ld a, (MAXFIL)
    ld ix, BDOS_CLSALL                  ; in: a=(MAXFIL), bc=BDOS_CLSFIL, hl=BASIC pointer
    call cmd_fcalbas_we
    jp cmd_freset_fil

; ------------------------------------------------------------------------------------------------------
; INPUT# statement
; in : a=file number, hl=string address (pascal style), e=mode (0=INPUT#, 1=LINE INPUT#)
; out: hl=string address (pascal style)
; reference: BDOS 0x14 (SequentialReadFile) and 0x21 (RandomReadFile)
; ------------------------------------------------------------------------------------------------------
cmd_finput:
  call cmd_fsetfil
  ld a, e                               ; save INPUT# / LINE INPUT# mode
  or a                                  ; FLGINP states:
  jr nz, cmd_finput.line_mode           ; 1 = LINE INPUT#
  ld a, 2                               ; 2 = INPUT# first char pending
  jr cmd_finput.mode_saved
cmd_finput.line_mode:
  ld a, 1
cmd_finput.mode_saved:
  ld (FLGINP), a
  xor a 
  ld (hl), a
  ld e, l 
  ld d, h 
  inc hl                                ; first character into string
cmd_finput.begin:
  call cmd_findskc
  cp BDOS_EOF_FLAG
  jp z, cmd_finput.end
  ld b, a
  ld a, (FLGINP)
  ld c, a
  cp 4                                  ; pending LF after previous CR (INPUT#)
  jr z, cmd_finput.pending_lf
  cp 5                                  ; pending LF after previous CR (LINE INPUT#)
  jr z, cmd_finput.pending_lf
  ld a, b
  jr cmd_finput.check_delimiter
cmd_finput.pending_lf:
  ld a, b
  cp 0x0A                               ; LF
  jr nz, cmd_finput.pending_no_lf
  ld a, c
  cp 5
  jr nz, cmd_finput.pending_lf_save
  ld a, 1                               ; restore LINE INPUT# mode
  jr cmd_finput.pending_lf_store
cmd_finput.pending_lf_save:
  ld a, 2
cmd_finput.pending_lf_store:
  ld (FLGINP), a
  jr cmd_finput.begin
cmd_finput.pending_no_lf:
  ld a, c
  cp 5
  jr nz, cmd_finput.pending_no_lf_save
  ld a, 1                               ; restore LINE INPUT# mode
  jr cmd_finput.pending_no_lf_store
cmd_finput.pending_no_lf_save:
  ld a, 2                               ; continue as regular INPUT#
cmd_finput.pending_no_lf_store:
  ld (FLGINP), a
  ld a, b
cmd_finput.check_delimiter:
  cp 0x0D                               ; CR
  jr z, cmd_finput.exec
  cp 0x0A                               ; LF 
  jr z, cmd_finput.exec
  ld b, a
  ld a, (FLGINP)
  cp 1                                  ; LINE INPUT# mode?
  jr z, cmd_finput.append_b
  cp 3                                  ; INPUT# quoted mode?
  jr z, cmd_finput.quoted
  cp 2                                  ; first char pending?
  jr nz, cmd_finput.normal
  ld a, b
  cp 0x22
  jr nz, cmd_finput.first_done
  ld a, 3                               ; enter quoted mode
  ld (FLGINP), a
  jr cmd_finput.begin                   ; skip opening quote
cmd_finput.first_done:
  xor a                                 ; FLGINP=0 regular INPUT# mode
  ld (FLGINP), a
cmd_finput.normal:
  ld a, b
  cp 0x2C                               ; , 
  jr z, cmd_finput.exec
  cp 0x09                               ; TAB
  jr z, cmd_finput.exec
cmd_finput.append_b:
  ld a, b
  jr cmd_finput.append
cmd_finput.quoted:
  ld a, b
  cp 0x22                               ; closing quote
  jr z, cmd_finput.end
cmd_finput.append:
  ld (hl), a                            ; next character
  inc hl
  ld a, (de)                            ; string length
  inc a 
  ld (de), a
  cp 0xFF
  jr z, cmd_finput.end                  ; return if maximum string size reached
  jp cmd_finput.begin 
cmd_finput.exec:
  cp 0x0D                               ; when CR is delimiter, ignore immediate next LF (CR+LF files)
  jr nz, cmd_finput.exec_ret
  ld a, (FLGINP)
  cp 1
  ld a, 4
  jr nz, cmd_finput.exec_set_pending
  ld a, 5                               ; pending LF after CR for LINE INPUT#
cmd_finput.exec_set_pending:
  ld (FLGINP), a
cmd_finput.exec_ret:
  ld a, (FLGINP)
  cp 1                                  ; LINE INPUT#: delimiter always finishes current read
  jr z, cmd_finput.end
  cp 5                                  ; LINE INPUT# pending LF after CR
  jr z, cmd_finput.end
  ;cp 4                                  ; INPUT# pending LF after CR
  ;jr z, cmd_finput.exec_check_started
cmd_finput.exec_check_started:
  ld a, (de)                            ; string length
  or a
  jr nz, cmd_finput.end
  jp cmd_finput.begin                   ; ignore leading delimiters in INPUT#
cmd_finput.end:
  ex de, hl                             ; return string address (hl)
  xor a 
  ld (FLGINP), a
  jp cmd_freset_fil                     ; turn off BASIC interpreter i/o redirect

; ------------------------------------------------------------------------------------------------------
; PRINT# statement
; in : a=file number, hl=string address (pascal style), e=sufix1, d=suffix2
; reference: BDOS 0x15 (SequentialWriteFile) and 0x22 (RandomWriteFile)
; ------------------------------------------------------------------------------------------------------
cmd_fprint:
  call cmd_fsetfil
  push de 
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
  pop de
  ld a, e 
  or a                                  ; suffix1? i.e: 0x09 (TAB) or 0x0D (CR)
  call nz, cmd_ffilout
  ld a, d
  or a                                  ; suffix2? e.i: 0x0A (LF)
  call nz, cmd_ffilout
  jp cmd_freset_fil                     ; turn off BASIC interpreter i/o redirect
