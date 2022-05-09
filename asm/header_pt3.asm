; ------------------------------------------------------------------------------------------------------
; MSXBAS2ROM Z80 header routines
; by: Amaury Carvalho, 2020
; Compile with:
;   pasmo header.asm header.bin
;   xxd -i header.bin header.h
; ------------------------------------------------------------------------------------------------------

; ------------------------------------------------------------------------------------------------------
; PLETTER AND PT3TOOLS
; ------------------------------------------------------------------------------------------------------

PT3.LOAD:         equ 0x8000
PT3.LOAD_FROM_HL: equ PT3.LOAD + 3
PT3.PLAY:         equ PT3.LOAD + 5
PT3.MUTE:         equ PT3.LOAD + 8
PT3.DATA.STATUS:  equ PT3.LOAD + 10    ; setup and status flags
PT3.DATA.INDEX:   equ PT3.LOAD + 11    ; pointer to current position value in PT3 module;
PT3.DATA.ADDR:    equ PT3.LOAD + 0x0900
PT3.DATA.TEMP:    equ PT3.DATA.ADDR + 0x1800
MEM.DATA.TEMP:    equ 0xDA00

; ------------------------------------------------------------------------------------------------------
; BIOS AND BASIC FUNCTIONS
; ------------------------------------------------------------------------------------------------------

CALBAS:       equ 0x0159
CALSLT:       equ 0x001C
EXTROM:       equ 0x015F
BASINIT:      equ 0x629A   ; initialize interpreter, basic pointer at start of program
RDSLT:        equ 0x000C   ; Read a byte in a Slot
RSLREG:       equ 0x0138   ; Read primary Slot REGister
WRSLT:        equ 0x0014   ; Write a byte in a Slot
WSLREG:       equ 0x013B   ; Write primary Slot REGister
ENASLT:       equ 0x0024   ; enable slot (a=slot indicator, hl=address to enable)
RDHDR:        equ 0x7E1A   ; read header data
XFER:         equ 0xF36E   ; LDIR of RAM page 1 from/to RAM page 3 (BC=512 bytes, HL=source address, DE=dest address)
IOALLOC:      equ 0x7E6B   ; IO allocation (CLEAR stmt)

CRUNCH:       equ 0x42B2
NEWSTT:       equ 0x4601   ; execute a text (hl=text pointer; text must start with a ":")
CHRGTR:       equ 0x4666   ; extract one character from command line text (hl=text pointer; hl=char address, a=char, Z=end of line)
FRMEVL:       equ 0x4C64   ; evaluate an expression in text (hl=expression pointer; hl=after expression, VALTYP/DAC)
FRMQNT:       equ 0x542F   ; evaluate an expression and return as integer (hl=expression; hl=after, DE=result)
GETBYT:       equ 0x521C   ; evaluate an expression and return as byte (hl=expression; hl=after, A=result)
FRESTR:       equ 0x67D0   ; register a string (VALTYP=3, DAC=string descriptor; HL=string descriptor)
PTRGET:       equ 0x5EA4   ; obtain the address for the storage of a variable (HL=variable name address, SUBFLG; HL=after name, DE=content address)
BEEP:         equ 0x00C0
CHGET:        equ 0x009F
CHPUT:        equ 0x00A2
CHSNS:        equ 0x009C
GICINI:       equ 0x0090   ; initialize PSG voices
GTTRIG:       equ 0x00D8   ; joy trigger status
CHCOORD:      equ 0x0BF2   ; in h=x, l=y; out hl = VDP address
CHCOORD2:     equ 0x0B98   ; same above for MSX2
BLTVM:        equ 0x0195
BLTMV:        equ 0x0199
INLIN:        equ 0x00B1
QINLIN:       equ 0x00B4

PLAY:         equ 0x39AE
DRAW:         equ 0x39A8
ERRORHANDLER: equ 0x406F
RUN:          equ 0x3940

HCMD:         equ 0xFE0D   ; cmd hook
HPRGE:        equ 0xFEF8   ; end of program hook
HTIMI:        equ 0xFD9F   ; timer interrupt hook
HREAD:        equ 0xFF07   ; ready prompt hook
HSTKE:        equ 0xFEDA   ; stack hook (for after msx initialization)
HCHPU:        equ 0xFDA4   ; char put hook
HKEYI:        equ 0xFD9A

WRTVDP:       equ 0x0047
RDVRM:        equ 0x004A
WRTVRM:       equ 0x004D
LDIRVM:       equ 0x005C
LDIRMV:       equ 0x0059
FILVRM:       equ 0x0056 ; fill VRAM with value
CALPAT:       equ 0x0084
CALATR:       equ 0x0087
GSPSIZ:       equ 0x008A
GRPPRT:       equ 0x008D
CLRSPR:       equ 0x0069

DISSCR:       equ 0x0041
ENASCR:       equ 0x0044

BIGFIL:       equ 0x016B ; msx 2
NRDVRM:       equ 0x0174 ; msx 2
NWRVRM:       equ 0x0177 ; msx 2
NRDVDP:       equ 0x013E ; msx 2
VDPSTA:       equ 0x0131 ; msx 2
NWRVDP:       equ 0x012D ; msx 2 (0x0647)
CALPAT2:      equ 0x00F9 ; msx 2
CALATR2:      equ 0x00FD ; msx 2
GSPSIZ2:      equ 0x0101 ; msx 2
CLRSPR2:      equ 0x00F5 ; msx 2
GRPPRT2:      equ 0x0089 ; msx 2 - a = character

CHGCPU:       equ 0x0180 ; turbo R - a = cpu mode (0=z80, 1=R800 rom, 2=R800 dram *just only at boot time)
GETCPU:       equ 0x0183 ; turbo R - return a with cpu mode

INIFNK:       equ 0x003E
INITIO:       equ 0x003B
INITXT:       equ 0x00D5 ; init screen 0
INIT32:       equ 0x00D9 ; init screen 1
INITXTWP:     equ 0x006C ; init screen 0 without pallete
INIT32WP:     equ 0x006F ; init screen 1 without pallete
KILBUF:       equ 0x0156
POSIT:        equ 0x00C6 ; locate (h=y, l=x, home position=1,1)
SCALXY:       equ 0x010E ; bc=x, de=y
MAPXYC:       equ 0x0111 ; bc=x, de=y

; ------------------------------------------------------------------------------------------------------
; BIOS AND BASIC WORK AREA
; ------------------------------------------------------------------------------------------------------

USRTAB:       equ 0xF39A   ; 20
VALTYP:       equ 0xF663   ; 1
VALDAT:       equ 0xF7F8   ; 2
DAC:          equ 0xF7F6   ; 16
ARG:          equ 0xF847   ; 16
SUBFLG:       equ 0xF6A5   ; 1 (0=simple variable, not 0 = array)
STMTKTAB:     equ 0x392E   ; addresses of BASIC statement token service routines (start from 081H to 0D8H)
FNCTKTAB:     equ 0x39DE   ; addresses of BASIC function token service routines

TEMP:         equ 0xF6A7   ; 2
TEMP2:        equ 0xF6BC   ; 2
TEMP3:        equ 0xF69D   ; 2
TEMP8:        equ 0xF69F   ; 2
TEMP9:        equ 0xF7B8   ; 2
TEMPPT:       equ 0xF678   ; 2 - pointer to next free position into TEMPST
TEMPST:       equ 0xF67A   ; 30 - temporary string descriptors buffer
DSCTMP:       equ 0xF698   ; 3 - index and start address of temporary strings
RAWPRT:       equ 0xF41F   ; 1
PARM1:        equ 0xF6E8   ; 100
PARM2:        equ 0xF750   ; 100
CMDP0:        equ PARM1    ; 1
CMDP1:        equ CMDP0+1  ; 2
CMDP2:        equ CMDP1+2  ; 2
CMDP3:        equ CMDP2+2  ; 2
BUF:          equ 0xF55E   ; 258
KBUF:         equ 0xF41F   ; 318
SWPTMP:       equ 0xF7BC   ; 8
STRBUF:       equ 0xF7C5   ; 43
JIFFY:        equ 0xFC9E   ; timer counter
VARWRK:       equ 0xF380   ; BASIC variables workspace start
TRCFLG:       equ 0xF7C4   ; BASIC line number trace on/off (0=off)
CLKFLG:       equ 0xF338   ; ask for clock flag (0=yes, 1=no)
WRMBOOT:      equ 0xF340   ; warm boot? (1=yes)
DSKDIS:       equ 0xFD99   ; disable disks (DEVICE, 0xFF=yes)

ROMSLT:       equ 0xFFF7   ; Main-ROM slot
EXPTBL:       equ 0xFCC1   ; Expanded Slot Table
SLTTBL:       equ 0xFCC5   ; Slot Table
RAMAD0:       equ 0xF341   ; slotid ram page 0 (0xFF = no ram)
RAMAD1:       equ 0xF342   ; slotid ram page 1
RAMAD2:       equ 0xF343   ; slotid ram page 2 (0xFF = no ram)
RAMAD3:       equ 0xF344   ; slotid ram page 3
RAMSLT:       equ KBUF+3   ; temporary searched ram slot
SLTWRK:       equ 0xFD09   ; 128 - variable array used to reserve a RAM work area in Main-RAM for ROM applications
SLTATR:       equ 0xFCC9   ; 64 - slots attributes (bit 5 = statement, bit 7 = basic code at page)

SCRMOD:       equ 0xFCAF   ; 0=40x24 Text Mode, 1=32x24 Text Mode, 2=Graphics Mode, 3=Multicolour Mode.
OLDSCR:       equ 0xFCB0   ; old text screen mode
NTMSXP:       equ 0xF417   ; last screen mode
RG0SAV:       equ 0xF3DF
RG1SAV:       equ 0xF3E0
RG8SAV:       equ 0xFFE7
STATFL:       equ 0xF3E7  ; VDP status register

TRGFLG:       equ 0xF3E8  ; joysticks trigger ports flag (b0-space, b4-1A, b5-1B, b6-2A, b7-2B)
SCNCNT:       equ 0xF3F6  ; joystick test counter (1 time for each 3 cycles)
QUETAB:       equ 0xF959  ; play queue tab (6 bytes per channel: index in, index out, flag, size, address)

FORCLR:       equ 0xF3E9  ; foreground color
BAKCLR:       equ 0xF3EA  ; background color
BDRCLR:       equ 0xF3EB  ; border color
ATRBYT:       equ 0xF3F2  ; char color

TXTNAM:       equ 0xF3B3
NAMBAS:       equ 0xF922  ; pattern name table - basic
PATBAS:       equ 0xF926  ; sprite pattern table
ATRBAS:       equ 0xF928  ; sprite attribute table
CGPBAS:       equ 0xF924  ; pattern generator table - basic
LINLEN:       equ 0xF3B0  ; line text length (default=39)
CRTCNT:       equ 0xF3B1  ; line text count (default=24)

GRPNAM:       equ 0xF3C7  ; pattern name table
GRPCOL:       equ 0xF3C9  ; colour table
GRPCGP:       equ 0xF3CB  ; pattern generator table (screen 2)
GRPATR:       equ 0xF3CD  ; sprite attribute table
GRPPAT:       equ 0xF3CF  ; sprite generator table
CGPNTSLT:     equ 0xF91F  ; 1 - current MSX Font slot location (default BIOS slot: 0x00)
CGPNT:        equ 0xF920  ; 2 - current MSX Font address location (default BIOS address: 0x1BBF)

TXTCGP:       equ 0xF3B7  ; pattern generator table (screen 0)
T32CGP:       equ 0xF3C1  ; pattern generator table (screen 1)

MLTNAM:       equ 0xF3D1  ; pattern name table (screen 3, multicolor)
MLTCOL:       equ 0xF3D3  ; colour table (screen 3, multicolor)
MLTCGP:       equ 0xF3D5  ; pattern generator table (screen 3, multicolor)
MLTATR:       equ 0xF3D7  ; sprite attribute table (screen 3, multicolor)
MLTPAT:       equ 0xF3D9  ; sprite generator table (screen 3, multicolor)

MUSICF:       equ 0xFB3F  ; contains 3 bit flags set by the STRTMS. Bits 0, 1 and 2 correspond to VOICAQ, VOICBQ and VOICCQ.
CLIKSW:       equ 0xF3DB  ; 0=keyboard click off, 1=keyboard click on
CSRY:         equ 0xF3DC  ; cursor Y pos
CSRX:         equ 0xF3DD  ; cursor X pos
TTYPOS:       equ 0xF661  ; teletype position

TXTTAB:       equ 0xF676     ; start of basic program
VARTAB:       equ 0xF6C2     ; start of variables area (end of basic program)
ARYTAB:       equ 0xF6C4     ; start of array area
STREND:       equ 0xF6C6     ; end of variables area
BASROM:       equ 0xFBB1     ; user basic code on rom? (0=RAM, not 0 = ROM)
NLONLY:       equ 0xF87C     ; loading basic program flags (bit 0=not close i/o buffer 0, bit 7=not close user i/o buffer)
LPTPOS:       equ 0xF415     ; printer head horizontal position
ONELIN:       equ 0xF6B9
ONEFLG:       equ 0xF6BB
PRTFLG:       equ 0xF416     ; output to screen (0=true)
PTRFLG:       equ 0xF6A9     ; line number converted to pointer (0=false)
DORES:        equ 0xF664     ; 1 - DATA flag to ASCII format
PRMFLG:       equ 0xF7B4

MV_DPTR:      equ 0xF566     ; 2
VM_DPTR:      equ 0xF562     ; 2
SX:           equ 0xF562     ; 2
SY:           equ 0xF564     ; 2
DX:           equ 0xF566     ; 2
DY:           equ 0xF568     ; 2
NX:           equ 0xF56A     ; 2
NY:           equ 0xF56C     ; 2
ARGT:         equ 0xF56F     ; 1
LOGOP:        equ 0xF570     ; 1

BOTTOM: equ 0xFC48
HIMEM:  equ 0xFC4A
ENDBUF: equ 0xF660
AUTFLG: equ 0xF6AA
BUFMIN: equ 0xF55D
KBFMIN: equ 0xF41E
CGTABL: equ 0x0004
PRMSTK: equ 0xF6E4
PRMPRV: equ 0xF74C
STKTOP: equ 0xF674
SAVSTK: equ 0xF6B1
MEMSIZ: equ 0xF672
ENDPRG: equ 0xF40F

VERSION:      equ 0x002D       ; BIOS VERSION - 0 = MSX1, 1 = MSX2, 2 = MSX2+, 3 = MSXturboR
NTSC:         equ 0x002B       ; bit 7 on = PAL, off = NTSC

WRKARE:       equ 0xC010       ; homebrew rom internal workarea start in RAM (alternatives: SLTWRK or PARM2)

SLTSTR:       equ WRKARE       ; 1 - startup slotid
PT3STS:       equ SLTSTR+1     ; 1 - PT3 status flag (0=idle, 1=play, 2=mute) - RAWPRT
PT3HKSAV:     equ PT3STS+1     ; 5 - HTIMI hook saved

SLTAD0:       equ PT3HKSAV+5   ; 1 - default slotid from page 0
SLTAD1:       equ SLTAD0+1     ; 1 - default slotid from page 1
SLTAD2:       equ SLTAD1+1     ; 1 - default slotid from page 2
SLTAD3:       equ SLTAD2+1     ; 1 - default slotid from page 3
MPRAD2:       equ SLTAD3+1     ; 1 - default segment of memory mapper from page 2
MPRAD2N:      equ MPRAD2+1     ; 1 - new segment of memory mapper from page 2
MAPPER:       equ MPRAD2N+1    ; 1 - mapper status (bit 0=mapper on/off, bit 1=running on RAM)
SGMADR:       equ MAPPER+1     ; 1 - current MegaROM segment selected in &8000
SOMODE:       equ SGMADR+1     ; 1 - screen output mode (0=text, 1=graphical, 2=tiled)
HEAPSTR:      equ SOMODE+1     ; 2 - heap start address
HEAPSIZ:      equ HEAPSTR+2    ; 2 - heap size
TMPSTRIND:    equ HEAPSIZ+2    ; 1 - temporary string index
TMPSTRADDR:   equ TMPSTRIND+1  ; 2 - temporary string start address
TMPSTRBUF:    equ TMPSTRADDR+2 ; 2 - temporary string next pointer
RESADDR:      equ TMPSTRBUF+2  ; 2 - resource address pointer
FONTOLDSLT:   equ RESADDR+2    ; 1 - old bios font slot
FONTOLD:      equ FONTOLDSLT+1 ; 2 - old bios font address
FONTADDR:     equ FONTOLD+2    ; 2 - new font space address

BASMEM:       equ FONTADDR+2
BASTEXT:      equ 0x800E       ; 0x8008 - address of user basic code

PageSize:	  equ 4000h	       ; 16kB

PT3FLG:         equ 0x800D     ; PT3 flag: 0=disabled, 1=enabled
resource.data:  equ PT3FLG - 2

XBASIC_SUBTRACT_FLOATS: equ 0x76D3
XBASIC_CAST_INTEGER_TO_FLOAT: equ 0x78DF
XBASIC_MULTIPLY_INTEGERS: equ 0x7631
XBASIC_DIVIDE_INTEGERS: equ 0x7643

; ------------------------------------------------------------------------------------------------------
; MACROS
; ------------------------------------------------------------------------------------------------------

MACRO call_basic,CALL_PARM
    ld ix, CALL_PARM
    call SUB_CALBAS
ENDM

; ------------------------------------------------------------------------------------------------------
; INITIALIZE
; ------------------------------------------------------------------------------------------------------

	org 4000h

; ### ROM header ###

	db "AB"		; ID for auto-executable ROM
	dw INIT1	; Main program execution address.
    dw 0x0000	; STATEMENT
	dw 0		; DEVICE
	dw 0		; TEXT
	dw 0,0,0	; Reserved

INIT1:	; Program code entry point label - 'c' has rom slot id

    ld a, (EXPTBL)
    ld h, 0x00
    call SUB_ENASLT       ; page 0 - enable bios rom (Victor HC-95/Sony HB-F500 bug fix on ExecROM with disk driver)

    ld h, 0x40            ; page 1 - this rom
    call page.getslt

    ld (SLTSTR), a        ; start slot

    ld hl, pre_start.hook_data
    ld de, HSTKE
    ld bc, 5
    ldir

    ld (HSTKE+1), a     ; program rom slot

    ret

pre_start.hook_data:
  db 0xF7, 0x00         ; CALLF
  dw start
  db 0xC9

start:
  call verify.slots

  call select_rom_on_page_2

clear_basic_environment:
  ld a, 0xC9
  ld (HSTKE), a
  ld (HSTKE+1), a
  ld (HSTKE+2), a
  ld (HSTKE+3), a
  ld (HSTKE+4), a

  xor a
  ld (BUF), a                ; clear BUF (keyboard)
  ld (ENDBUF), a             ; endmarker for BUF
  ld (KBUF), a               ; clear KBUF (interpreter command line)
  ld (AUTFLG), a             ; quit auto linenumber mode
  ld (LPTPOS), a             ; printer position
  ld (TRCFLG), a             ; disable trace
  ld (PRTFLG), a             ; output to screen
  ld (ENDPRG+1),a            ; fake line
  ld (ENDPRG+2),a            ; fake line
  ld (ENDPRG+3),a            ; fake line
  ld (ENDPRG+4),a            ; fake line
  ld (TRGFLG), a             ; joysticks ports
  ld (SOMODE), a             ; default screen output mode = text

  ld A,','
  ld (BUFMIN),A              ; dummy prefix for BUF

  ld A,":"
  ld (KBFMIN),A              ; dummy prefix for KBUF
  ld (ENDPRG),a              ; fake line

  ld a, 0x81                 ; 10000001b
  ld (NLONLY), a             ; dont close i/o buffers

  ld a, 0xFF
  ld (PTRFLG), a             ; line number converted to pointer (0=false)
  ld (DSKDIS), a             ; disable disks

  call KILBUF
  call INITXT                ; screen 0
  call CLRSPR                ; clear sprites

run_user_basic_code_on_rom:
  ld hl, BASMEM              ; variable starts at page 3 (0xC000...)
  ld (VARTAB), hl
  ld (BOTTOM), hl

  ld hl, (BASTEXT)           ; start of user basic code
  inc hl
  ld (TXTTAB), hl
  ld a, h
  ld (BASROM), a             ; basic code location (0=RAM, not 0 = ROM)

  ld HL,VARWRK
  ld (HIMEM),HL              ; highest BASIC RAM address
  ld (MEMSIZ),hl
  xor a                      ; user i/o channels (FILES number=0)
  ld bc, 200                 ; string buffer size
  sbc hl, bc
  ld (STKTOP), hl

  jp 0x8010      	         ; Jump to above page (start code)

;---------------------------------------------------------------------------------------------------------
; XBASIC WRAPPER ROUTINES
;---------------------------------------------------------------------------------------------------------

; hl = heap start address
; de = temporary string start address
; bc = data address
; ix = temporary font buffer address
; a = data start segment number
XBASIC_INIT:
  ld (SUBFLG), a          ; DATA start segment number
  ld (DORES), a           ; DATA current segment number
  ld (DATPTR), bc         ; DATA current pointer
  ld (DATLIN), bc         ; DATA start pointer

  xor a
  ld (TMPSTRIND), a       ; temporary string list current position
  ld (TMPSTRADDR), de     ; temporary string list start pointer

  ld (HEAPSTR), hl        ; heap start address
  ex de, hl
  ld hl, (HIMEM)
  sbc hl, de
  ld (HEAPSIZ), hl        ; heap size

  ld (FONTADDR), ix       ; temporary font buffer address

  ld a, (CGPNTSLT)
  ld (FONTOLDSLT), a
  ld hl, (CGPNT)
  ld (FONTOLD), hl

  ld hl, 0x3579           ; RANDOMIZE 1 - FIX
  ld (SWPTMP), hl         ; SWPTMP+0
  ld hl, 0x7531           ; RANDOMIZE 2 - FIX
  ld (SWPTMP+2), hl       ; SWPTMP+2

  ld hl, usr0
  ld (USRTAB), hl
  ld hl, usr1
  ld (USRTAB+2), hl
  ld hl, usr2
  ld (USRTAB+4), hl
  ld hl, usr3
  ld (USRTAB+6), hl

  ld hl, (HEAPSTR)         ; heap start address
  ld de, BASMEM            ; variables start address
  xor a
  sbc hl, de               ; hl = variables data size
  ld c, l
  ld b, h                  ; bc = hl
  ex de, hl
  ld (hl), a               ; clear variables first byte
  ld e, l
  ld d, h
  inc de                   ; de = hl + 1
  ldir                     ; clear all the rest

  ret

; a = screen mode
XBASIC_SCREEN:
  ld c, 0
  cp 2
  jr c, XBASIC_SCREEN.DO   ; text mode (0), else graphical mode (1)
     inc c
XBASIC_SCREEN.DO:
  push af
    ld a, c
    ld (SOMODE), a
  pop af
  jp C7369

XBASIC_CLS:
  ld a, (SOMODE)
  cp 2              ; tiled output mode?
  jp z, cmd_clrscr
  jp I6CE3

; hl=y, de=x
XBASIC_LOCATE:
  ld a, (SOMODE)
  or a
  jr z, XBASIC_LOCATE.TXT  ; jump if text mode (0)
  dec a
  jr z, XBASIC_LOCATE.GRP  ; jump if graphical mode (1)
XBASIC_LOCATE.TIL:         ; tiled mode (2)
  ld h, e
  inc l
  inc h
  ld (CSRY),hl
  ld a, h
  ld (TTYPOS), a
  ret
XBASIC_LOCATE.GRP:         ; graphical mode (1)
  ld (GRPACY), hl
  ld (GRPACX), de
  ex de,hl
  ld c, l
  ld b, h
  call SCALXY ; BC = X, DE = Y
  jp MAPXYC ; BC = X, DE = Y
XBASIC_LOCATE.TXT:         ; text mode (0)
  ld h, e
  inc l
  inc h
  jp POSIT               ; h=x, l=y

; hl = string address
XBASIC_PRINT_STR:
  ld a, (hl)
  or a
  ret z
  ld b, a
  ld a, (SOMODE)
  or a
  jp z, C74FE                 ; jump if text mode (0)
  dec a
  jr z, XBASIC_PRINT_STR.GRP  ; jump if graphical mode (1)
XBASIC_PRINT_STR.TIL:         ; tiled mode (2)
  push hl
    ld hl, (CSRY)
    push hl
      call gfxTileAddress
      ex de,hl
    pop af
    add a, b
    ld (CSRX), a
  pop hl
  ld c, b
  ld b, 0
  inc hl
  jp LDIRVM
XBASIC_PRINT_STR.GRP:
  ld a, (SCRMOD)
  cp 5
  jr nc, XBASIC_PRINT_STR.MSX2
XBASIC_PRINT_STR.MSX1:
  inc hl
  ld a, (hl)
  call GRPPRT
  djnz XBASIC_PRINT_STR.MSX1
  ret
XBASIC_PRINT_STR.MSX2:
  inc hl
  ld a, (hl)
  ld ix, GRPPRT2
  call SUB_EXTROM
  djnz XBASIC_PRINT_STR.MSX2
  ret

; hl = input data
; a = user function number
XBASIC_USR:
  ld (DAC+2), hl         ; input data

  ld hl, XBASIC_USR.ret
  push hl

  ld hl, USRTAB
  add a, a
  ld d, 0
  ld e, a
  add hl, de

  ld e, (hl)
  inc hl
  ld d, (hl)

  push de

  ld a, 2              ; input is integer type
  ld (VALTYP), a

  xor a
  ret                  ; indirect call to usr function

XBASIC_USR.ret:
  ld hl, (DAC+2)       ; return data
  ret

; de=x0, hl=y0, ix=x1, iy=y1, 0xFC18=srcpg, GRPACX=x2, GRPACY=y2, 0xFC19=destpg, b=operator
XBASIC_COPY:
  ld a, (SOMODE)
  cp 1
  jr nz, XBASIC_COPY.TILED ; jump if not graphical mode (1)

  ld a, (VERSION)
  or a
  ret z                    ; only for MSX2

  jp 0x6EC8                ; XBASIC original COPY

XBASIC_COPY.TILED:
  ld (SX), de
  ld (SY), hl
  push hl
    push ix
    pop hl
    xor a
    sbc hl, de
    inc hl
    ld (NX), hl
  pop de
  push iy
  pop hl
  xor a
  sbc hl, de
  inc hl
  ld (NY), hl
  ld hl, (FONTADDR)
  call XBASIC_COPY_TO.TILED

  ld hl, (GRPACX)
  ld (DX), hl
  ld hl, (GRPACY)
  ld (DY), hl
  ld hl, (FONTADDR)
  jp XBASIC_COPY_FROM.TILED

; dx=x, dy=y, hl=src address, argt=expansion/direction (0000DDEE), logop=operation
XBASIC_COPY_FROM:
  ld a, (SOMODE)
  cp 1
  jr nz, XBASIC_COPY_FROM.TILED ; jump if not graphical mode (1)

  ld a, (VERSION)
  or a
  ret z                         ; only for MSX2

  ld (VM_DPTR), hl
  ld hl, VM_DPTR
  ld ix, BLTVM
  jp SUB_EXTROM                     ; XBASIC original COPY FROM

XBASIC_COPY_FROM.TILED:         ; dx=x, dy=y, hl=src address
  push hl
    ld a, (DY)
    ld l, a
    ld a, (DX)
    ld h, a
    call gfxTileAddress
    ex de, hl
  pop hl

  ld c, (hl)                    ; nx
  ld b, 0
  inc hl
  ld a, (hl)                    ; ny
  inc hl
XBASIC_COPY_FROM.TILED.loop:
  push af
  push de
  push bc
  push hl
    call LDIRVM
  pop hl
  ld a, (LINLEN)
  ld c, a
  ld b, 0
  add hl, bc
  inc hl
  ex de, hl
  pop bc
  pop hl
  add hl, bc
  ex de, hl
  pop af

  dec a
  ret z
  jr XBASIC_COPY_FROM.TILED.loop

; sx=x, sy=y, hl=dest address, nx=width, ny=height, argt=expansion/direction (0000DDEE)
XBASIC_COPY_TO:
  ld (MV_DPTR), hl
  ld hl, (NX)
  xor a
  sbc hl, de                    ; x1 must be >= x0
  jr nc, XBASIC_COPY_TO.1
    call intNeg
XBASIC_COPY_TO.1:
  ld (NX), hl

  ld de, (SY)
  ld hl, (NY)
  xor a
  sbc hl, de                    ; y1 must be >= y0
  jr nc, XBASIC_COPY_TO.2
    call intNeg
XBASIC_COPY_TO.2:
  ld (NY), hl

  ld a, (RAMAD3)
  and 0xF0                      ; keep just expansion
  ld (ARGT), a

  ld a, (SOMODE)
  dec a
  jr nz, XBASIC_COPY_TO.TILED   ; jump if not graphical mode (1)

  ld a, (VERSION)
  or a
  ret z                         ; only for MSX2

  ld hl, VM_DPTR

  ld ix, BLTMV
  jp SUB_EXTROM                     ; XBASIC original COPY FROM

XBASIC_COPY_TO.TILED:           ; sx=x, sy=y, hl=dest address, nx=width, ny=height
  ld hl, (SY)
  ld a, (SX)
  ld h, a
  call gfxTileAddress
  ld de, (MV_DPTR)
  ld bc, (NX)
  ld a, c
  ld (de), a
  inc de
  ld a, (NY)
  ld (de), a
  inc de
XBASIC_COPY_TO.TILED.loop:
  push af
  push hl
  push bc
  push de
    call LDIRMV
  pop hl
  ld a, (LINLEN)
  ld c, a
  ld b, 0
  add hl, bc
  inc hl
  ex de, hl
  pop bc
  pop hl
  add hl, bc
  pop af

  dec a
  ret z
  jr XBASIC_COPY_TO.TILED.loop

; hl, de, bc (all pointers to strings)
XBASIC_PLAY:
  call XBASIC_GET_BUFFER

  xor a
  cp d
  jr z, XBASIC_PLAY.1b
    cp b
    jr z, XBASIC_PLAY.1a
      push af
XBASIC_PLAY.1a:
    push bc
XBASIC_PLAY.1b:
  push de

XBASIC_PLAY.1:
  ld de, (TMPSTRBUF)
  jr XBASIC_PLAY.3

XBASIC_PLAY.2:
    ld a, ','                 ; label 2
    ld (de), a
    inc de

XBASIC_PLAY.3:
  ld a, '"'                   ; label 3
  ld (de), a
  inc de

  xor a
  ld b, a
  ld c, (hl)
  or c
  jr z, XBASIC_PLAY.4
    inc hl
    ldir
XBASIC_PLAY.4:
  ld a, '"'
  ld (de), a
  inc de

  xor a
  ld (de), a

  pop hl
  cp h
  jr nz, XBASIC_PLAY.2

  ld hl, (TMPSTRBUF)

  ld ix, (PLAY)
  jp SUB_CALBAS

; out (TMPSTRBUF) = next temporary string address
XBASIC_GET_BUFFER:
  push af
  push hl
  push de
  push bc
    call GET_NEXT_TEMP_STRING_ADDRESS
    ld (TMPSTRBUF), hl
  pop bc
  pop de
  pop hl
  pop af
  ret

XBASIC_BASE:
  ld a, l
  cp 0x14
  jr c, XBASIC_BASE.1      ; a < 20? goto label 1
    sub 0x19
    jr nc, XBASIC_BASE.3    ; a >= 0x19? goto label 3
      add a, 0x0F
      ld e,a
      cp 0x0D
      jr nz, XBASIC_BASE.1  ; a <> 0x0D? goto label 1
        ld hl,0x1E00
        ret
XBASIC_BASE.3:
    ld d, 0x00    ; label 3
    ld e, a
    ld hl, 0x22E0
    add hl,de
    ld h, (hl)
    ld l, d
    ret
XBASIC_BASE.1:
    cp 0x02         ; label 1
    jr nz, XBASIC_BASE.2     ; a <> 2? goto label 2
      ld a, (LINLEN)
      cp 0x41
      jr c, XBASIC_BASE.2     ; a < 41? goto label 2
        ld hl, 0x1000
        ret
XBASIC_BASE.2:
    ex de, hl       ; label 2
    ld hl, TXTNAM
    add hl, de
    add hl, de
    ld e, (hl)
    inc hl
    ld d, (hl)
    ex de, hl
    ret

XBASIC_TAB:
  ld de, (TTYPOS)
  xor a
  sbc hl, de
  bit 7, h
  jr z, XBASIC_TAB.1
    ld l, a
XBASIC_TAB.1:
  ld a, 0x20   ; space
  ld b, l
  call XBASIC_GET_BUFFER
  ld hl, (TMPSTRBUF)
  jp 0x7EF5    ; STRING$ (hl=destination, b=number of chars, a=char)

XBASIC_SOUND:
  cp 007h
  jr nz, XBASIC_SOUND.1
    res 6,e
    set 7,e
XBASIC_SOUND.1:
  jp WRTPSG

XBASIC_READ:
  ld hl, (DATPTR)          ; DATA current pointer
  push hl
    ld e, (hl)
    ld d, 0
    add hl, de
    inc hl
    ld (DATPTR), hl
  pop hl
  ret

XBASIC_RESTORE:
  ld hl, (DATLIN)        ; DATA start pointer
  add hl, de             ; add data size
  ld (DATPTR), hl        ; DATA current pointer
  ret

XBASIC_READ_MR:
  ld hl, (DATPTR)  ; DATA current pointer
  ld a, (DORES)    ; DATA current segment
XBASIC_READ_MR.start:
  call MR_GET_DATA
  ld a, (hl)
  cp 0xFF
  jr nz, XBASIC_READ_MR.move_to_next
    inc hl
    ld a, (hl)
    dec hl
    cp 0xFF
    jr nz, XBASIC_READ_MR.move_to_next
      ld a, (DORES)
      inc a
      inc a
      ld (DORES), a
      ld hl, 0x8000
      ld (DATPTR), hl
      jr XBASIC_READ_MR.start
XBASIC_READ_MR.move_to_next:
  push hl
    ld e, a
    ld d, 0
    ld hl, (DATPTR)  ; DATA current pointer
    add hl, de
    inc hl
    ld (DATPTR), hl
  pop hl
  ret

XBASIC_RESTORE_MR:
  ld hl, (DATLIN)        ; DATA start pointer
  ld (DATPTR), hl        ; DATA current pointer
  ld a, (SUBFLG)    ; DATA start segment
  ld (DORES), a     ; DATA current segment
XBASIC_RESTORE_MR.verify:
  ld a, d
  or e
  ret z
  push de
    ld hl, (DATPTR)  ; DATA current pointer
    ld a, (DORES)    ; DATA current segment
XBASIC_RESTORE_MR.start:
    call MR_GET_BYTE
    cp 0xFF
    jr nz, XBASIC_RESTORE_MR.move_to_next
      ld a, b
      cp 0xFF
      jr nz, XBASIC_RESTORE_MR.move_to_next
        ld a, (DORES)
        inc a
        inc a
        ld (DORES), a
        ld hl, 0x8000
        ld (DATPTR), hl
        jr XBASIC_RESTORE_MR.start
XBASIC_RESTORE_MR.move_to_next:
    ld e, a
    ld d, 0
    ld hl, (DATPTR)  ; DATA current pointer
    add hl, de
    inc hl
    ld (DATPTR), hl
  pop de
  dec de
  jr XBASIC_RESTORE_MR.verify

XBASIC_IREAD:
  ld hl, (DATPTR)          ; DATA current pointer
  ld e, (hl)
  inc hl
  ld d, (hl)
  inc hl
  ld (DATPTR), hl
  ex de, hl
  ret

XBASIC_INPUT_2:
  call INLIN
  jr XBASIC_INPUT_1.do

XBASIC_INPUT_1:
  call QINLIN

XBASIC_INPUT_1.do:
  call XBASIC_GET_BUFFER
  ld de, (TMPSTRBUF)
  ld bc, 0x0000
  push de
    jr c, XBASIC_INPUT_1.end  ; (correct = jp c, end_of_program)
      ldi
      xor a
XBASIC_INPUT_1.do.1:
      cp (hl)
      jr z, XBASIC_INPUT_1.do.2
        ldi
        jr XBASIC_INPUT_1.do.1
XBASIC_INPUT_1.do.2:
      ld a, c
      cpl
      ld c, a
XBASIC_INPUT_1.end:
  pop hl
  ld (hl), c
  ret

;---------------------------------------------------------------------------------------------------------
; CMD
;---------------------------------------------------------------------------------------------------------

; play resource with Basic standard statement
; CMD PLAY <resource number> [, <channel C: 0=off|1=on>]
cmd_play:
  ld bc, (DAC)             ; bc = resource number
  call resource.address    ; hl = resource start address
  ret c
  ld a, (ARG)
  and 1
  sla a
  sla a
  ld c, a
  push bc
    ld ix, (PLAY)
    call SUB_CALBAS
  pop bc
  di
    ld a, (MUSICF)
    and 3                  ; disable channel C
    or c                   ; enable/disable channel C
    ld (MUSICF), a
    ld a, 0xFF
    ld (QUETAB + 12), a
    ld (QUETAB + 13), a    ; clear channel C buffer index (in and out)
  ei
  xor a
  ret

; mute PSG
; CMD MUTE
cmd_mute:
  halt
  di
    call GICINI
  ei
  jp GICINI

; draw resource with Basic standard statement
; CMD DRAW <resource number>
cmd_draw:
  ld bc, (DAC)             ; bc = resource number
  call resource.address    ; hl = resource start address
  ret c
  ld ix, (DRAW)
  call SUB_CALBAS
  xor a
  ret

; exec as assembly code
; CMD RUNASM <resource number>
cmd_runasm:
  ld bc, (DAC)             ; bc = resource number
  call resource.address    ; hl = resource start address
  ret c
  push hl
  ret                      ; indirect call

; write resource to vram address
; CMD WRTVRAM <resource number>, <vram address>
cmd_wrtvram:
  ld bc, (DAC)             ; bc = resource number

cmd_wrtvram.patch.1:
  ld a, (RAMAD2)           ; test RAM on page 2
  cp 0xFF
  jr z, cmd_wrtvram.no.ram.page.2

cmd_wrtvram.patch.2:
  ld de, PT3.DATA.TEMP
  call resource.copy_from_rom_to_ram
  ret c

  di
cmd_wrtvram.patch.3:
    call select_ram_on_page_2

cmd_wrtvram.patch.3.1:
    ld hl, PT3.DATA.TEMP
    ld de, (RESADDR)
    call resource.ram.unpack

    ld hl, (RESADDR)
    ld de, (ARG)
    call SUB_LDIRVM

cmd_wrtvram.patch.4:
    call select_rom_on_page_2
  ei
  ret

cmd_wrtvram.no.ram.page.2:
  call resource.address    ; hl = resource start address
  ret c

  ld de, (FONTADDR)        ; MEM.DATA.TEMP
  call resource.ram.unpack

  ld hl, (FONTADDR)        ; MEM.DATA.TEMP
  ld de, (ARG)
  jp LDIRVM

; write font resource to vram tile pattern table or graphical mode
; CMD WRTFNT <resource number>
cmd_wrtfnt:
  ld bc, (DAC)             ; bc = resource number
  call resource.address    ; in bc = resource number, out hl = resource start address

  ld de, (FONTADDR)
  call resource.ram.unpack

  inc hl
  inc hl
  inc hl
  inc hl
  inc hl
  inc hl
  inc hl
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc

  ld a, (SOMODE)
  cp 1                     ; graphical screen output mode?
  jr z, cmd_wrtfnt.no_tiled

cmd_wrtfnt.tiled:
  push hl
    call cmd_setfnt.get_vram_address
    ex de, hl
  pop hl

  jp LDIRVM

cmd_wrtfnt.no_tiled:
  ld a, (RAMAD3)
  ld (CGPNTSLT), a
  ld (CGPNT), hl
  ret

; write resource to vram tile pattern table
; CMD WRTCHR <resource number>
cmd_wrtchr:
  ld a, 2                  ; tiled screen output mode
  ld (SOMODE), a
  ld bc, (GRPCGP)
  ld (ARG), bc
  call cmd_wrtvram
  jp cmd_updfntclr         ; set default color to font range on all bank numbers

; write resource to vram tile color table
; CMD WRTCLR <resource number>
cmd_wrtclr:
  ld a, 2                  ; tiled screen output mode
  ld (SOMODE), a
  ld bc, (GRPCOL)
  ld (ARG), bc
  jp cmd_wrtvram

; write resource to vram tile screen table
; CMD WRTSCR <resource number>
cmd_wrtscr:
  ld a, 2                  ; tiled screen output mode
  ld (SOMODE), a
  ld bc, (GRPNAM)
  ld (ARG), bc
  jp cmd_wrtvram

; write resource to vram sprite pattern table
; CMD WRTSPRPAT <resource number>
cmd_wrtsprpat:
  xor a
  call gfxCALPAT
  ld (ARG), hl
  jp cmd_wrtvram

; write resource to vram sprite color table
; CMD WRTSPRCLR <resource number>
cmd_wrtsprclr:
  xor a
  call gfxCALCOL
  ld (ARG), hl
  jp cmd_wrtvram

; write resource to vram sprite attribute table
; CMD WRTSPRATR <resource number>
cmd_wrtspratr:
  xor a
  call gfxCALATR
  ld (ARG), hl
  jp cmd_wrtvram

; write ram to vram address
; CMD RAMTOVRAM <ram address>, <vram address>, <size>
cmd_ramtovram:
  ld hl, (DAC)
  ld de, (ARG)
  ld bc, (ARG+2)
  jp LDIRVM      ; hl = ram data address, de = vram data address, bc = length

; write vram to ram address
; CMD VRAMTORAM <vram address>, <ram address>, <size>
cmd_vramtoram:
  ld hl, (DAC)
  ld de, (ARG)
  ld bc, (ARG+2)
  jp LDIRMV      ; de = ram data address, hl = vram data address, bc = length

; copy ram to ram address
; CMD RAMTORAM <ram source address>, <ram dest address>, <size>
cmd_ramtoram:
  ld hl, (DAC)
  ld de, (ARG)
  ld bc, (ARG+2)
  ldir      ; hl = source ram address, de = dest ram address, bc = length
  ret

; disable screen
cmd_disscr:
  jp DISSCR

; enable screen
cmd_enascr:
  jp ENASCR

cmd_keyclkoff:
  xor a
  ld (CLIKSW), a
  ret

; load PT3 resource in memory
; CMD PT3LOAD <resource number>
cmd_pt3load:
  ld a, (RAMAD2)          ; test RAM on page 2
  cp 0xFF
  ret z                   ; return if no RAM

  di
    xor a                 ; 0 = idle
    ld (PT3STS), a
  ei
  halt

  ld bc, (DAC)            ; bc = resource number
cmd_pt3load.patch.1:
  ld de, PT3.DATA.TEMP
  call resource.copy_from_rom_to_ram
  ret c

  halt

  di

cmd_pt3load.patch.2:
    call select_ram_on_page_2

cmd_pt3load.patch.3:
    ld hl, PT3.DATA.TEMP
cmd_pt3load.patch.4:
    ld de, PT3.DATA.ADDR
    call pletter.unpack

cmd_pt3load.patch.5:
    call select_rom_on_page_2

  ei

  halt

cmd_pt3load.do:
  di

cmd_pt3load.patch.6:
    call select_ram_on_page_2

cmd_pt3load.patch.7:
    ld a, (PT3.DATA.STATUS)
    and %00000001          ; clear all flags (except loop flag)
cmd_pt3load.patch.8:
    ld (PT3.DATA.STATUS), a

cmd_pt3load.patch.9:
    ld hl, PT3.DATA.ADDR
cmd_pt3load.patch.10:
    call PT3.LOAD_FROM_HL

cmd_pt3load.patch.11:
    call select_rom_on_page_2

  ei

  halt

  xor a
  ret

; replay PT3 in memory
; CMD PT3REPLAY
cmd_pt3replay:
  ld a, (RAMAD2)          ; test RAM on page 2
  cp 0xFF
  ret z                   ; return if no RAM

  di
    xor a                 ; 0 = idle
    ld (PT3STS), a
  ei

  halt

  call cmd_pt3load.do

; play PT3 in memory
; CMD PT3PLAY
cmd_pt3play:
  di
    ld a, 1                ; play
    ld (PT3STS), a
  ei

  halt

  xor a
  ret

; mute PT3 in memory
; CMD PT3MUTE
cmd_pt3mute:
  di
    ld a, 2         ; mute
    ld (PT3STS), a
  ei

  halt

  xor a
  ret

; set PT3 loop status
; CMD PT3LOOP <0=off|1=on>
cmd_pt3loop:
  ld a, (RAMAD2)          ; test RAM on page 2
  cp 0xFF
  ret z                   ; return if no RAM

  ld a, (DAC)             ; a = loop status
  ld e, 1
  and e
  xor e
  ld e, a
  di
    push de
cmd_pt3loop.patch.1:
      call select_ram_on_page_2
    pop de
cmd_pt3loop.patch.2:
    ld a, (PT3.DATA.STATUS)
    and %11111110
    or e
cmd_pt3loop.patch.3:
    ld (PT3.DATA.STATUS), a
cmd_pt3loop.patch.4:
    call select_rom_on_page_2
  ei

  halt

  xor a
  ret

; enable built in fonts
; CMD SETFNT <number> [,<bank>]
cmd_setfnt:
  ld a, (SCRMOD)
  or a
  ret z                         ; no default fonts to screen 0

  ld a, (DAC)                   ; a = font number
  or a                          ; 0
  jr z, cmd_setfnt.bios

  ld hl, font.resource.data.1
  dec a                         ; 1
  jr z, cmd_setfnt.unpack

  ld hl, font.resource.data.2
  dec a                         ; 2
  jr z, cmd_setfnt.unpack

  ld hl, font.resource.data.3   ; 3

cmd_setfnt.unpack:
  ld de, (FONTADDR)
  call resource.ram.unpack

  ld a, (RAMAD3)
  ld (CGPNTSLT), a

  jr cmd_setfnt.do

cmd_setfnt.bios:
  ld a, (FONTOLDSLT)
  ld (CGPNTSLT), a
  ld hl, (FONTOLD)
  ld (CGPNT), hl

  ex de, hl
  ld bc, 95 * 8                   ; bytes to be copied (chars * 8)
  ld hl, 32 * 8                   ; pattern generator in bios start position * 8
  add hl, de                      ; ram font source in bios

cmd_setfnt.do:
  ld a, (SOMODE)
  cp 1                            ; graphical screen output mode?
  jr z, cmd_setfnt.no_tiled

cmd_setfnt.tiled:
  ld a, (ARG)                     ; bank number (0xFF = all)
  push af
    call cmd_setfnt.cpy_to_bank   ; hl=source address, a=bank number in vram
  pop af
  jp cmd_setfnt.default_colors

cmd_setfnt.no_tiled:
  xor a
  ld bc, 32 * 8
  sbc hl, bc
  ld (CGPNT), hl
  ret

; a = bank number (0xFF = all)
; hl = ram address
cmd_setfnt.cpy_to_bank:
  cp 0xFF
  jr nz, cmd_setfnt.cpy_to_bank.do
     push hl
       xor a
       call cmd_setfnt.cpy_to_bank.do
     pop hl
     push hl
       ld a, 1
       call cmd_setfnt.cpy_to_bank.do
     pop hl
     ld a, 2

cmd_setfnt.cpy_to_bank.do:
  push hl
    call cmd_setfnt.get_vram_address
    ld de, 32 * 8    ; pattern generator in vram start position * 8
    add hl, de
    or a
    jr z, cmd_setfnt.cpy_to_bank.cont
      ld de, 0x0800
      add hl, de         ; next bank
      cp 1
      jr z, cmd_setfnt.cpy_to_bank.cont
        add hl, de       ; next bank
cmd_setfnt.cpy_to_bank.cont:
    ex de, hl
  pop hl
  ld bc, 95 * 8    ; bytes to be copied (chars * 8)
  jp LDIRVM

; out hl = pattern generator vram address
cmd_setfnt.get_vram_address:
  push af
    ld hl, (GRPCGP)
    ld a, (SCRMOD)
    cp 2
    jr nc, cmd_setfnt.get_address.exit
    ld hl, (T32CGP)
    cp 1
    jr z, cmd_setfnt.get_address.exit
    ld hl, (TXTCGP)
cmd_setfnt.get_address.exit:
  pop af
  ret

; a = bank number (0xFF = all)
cmd_setfnt.default_colors:
  ld (ARG), a
  ld a, (SCRMOD)
  cp 2
  ret nz
    push hl
    push de
    push bc
      ld bc, (GRPATR)
      ld (ATRBAS), bc
      ld bc, (GRPNAM)
      ld (NAMBAS), bc
      ld bc, (GRPCGP)
      ld (CGPBAS), bc
      ld a, 32  ;31
      ld (LINLEN), a
      ld a, 24
      ld (CRTCNT), a
      ld hl, (GRPCOL)
      ld de, 32*8
      add hl, de
      ld a, (ARG)      ; bank number in vram (0xFF = all)
      cp 0xFF
      jr nz, cmd_setfnt.default_colors.end
        xor a
        call cmd_setfnt.default_colors.0
        ld a, 1
        call cmd_setfnt.default_colors.0
        ld a, 2
cmd_setfnt.default_colors.end:
      call cmd_setfnt.default_colors.0
    pop bc
    pop de
    pop hl
    ret

cmd_setfnt.default_colors.0:
    push hl
      or a
      jr z, cmd_setfnt.default_colors.1
        ld de, 0x0800
        add hl, de
        cp 1
        jr z, cmd_setfnt.default_colors.1
          add hl, de
cmd_setfnt.default_colors.1:
      ld a, (FORCLR)
      sla a ; x2
      sla a ; x4
      sla a ; x8
      sla a ; x16
      ex de, hl
        ld hl, BAKCLR
        or (hl)
      ex de, hl
      ld bc, 95*8
      call FILVRM
    pop hl
    ret

cmd_updfntclr:
  ld a, 0xFF
  jp cmd_setfnt.default_colors

cmd_clrkey:
  di
    push hl
      call KILBUF
    pop hl
    xor a
    ld (TRGFLG), a   ; clear joysticks
  ei
  ret

cmd_clrscr:
  ld hl, (GRPNAM)
  ld bc, 28*32
  ld a, 32
  jp FILVRM

; DAC = 0 (off), 1 (on)
cmd_turbo:
  ;in a, (64)
  ;cp 247            ; 8 - panasonic
  ;ret nz            ; turbo works only on panasonic machines
  ld a, (VERSION)
  cp 2              ; msx2+ or turbo R
  ret c             ; not a turbo capable machine
  jr z, cmd_turbo.msx2p
cmd_turbo.turboR:   ; R800 processor type
  ld a, (CHGCPU)
  cp 0xC3
  ret nz            ; BIOS function CHGCPU doesnt exist
  ld a, (DAC)
  or 0x80           ; update turbo led
  jp CHGCPU
cmd_turbo.msx2p:    ; msx2+ machine type
  ld a, 8
  out (64), a
  ld a, (DAC)
  and 1
  xor 1
  out (65), a
  ret

; ------------------------------------------------------------------------------------------------------
; USR() IMPLEMENTATION
; ------------------------------------------------------------------------------------------------------

; get resource data address
; in hl = parameter
; out hl = address
usr0:
  ld c, l
  ld b, h
  call resource.address
  jp c, usr_def
  jp usr_def.exit

; get resource data size
; in hl = parameter
; out hl = size
usr1:
  ld c, l
  ld b, h
  call resource.size
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
  jp z, usr2_pt3status
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

usr2_pt3status:
  di
usr2_pt3status.patch.1:
    call select_ram_on_page_2
usr2_pt3status.patch.2:
    ld a, (PT3.DATA.STATUS)
    push af
usr2_pt3status.patch.3:
      call select_rom_on_page_2
    pop af
  ei
  xor 1                               ; invert loop flag
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
; reference:
; https://www.msx.org/forum/development/msx-development/sprite-collision-detection-or-manually-calculation-sprite-coordina
; hl = parameter
usr3:
  ld (DAC), hl
  ld a, (STATFL)
  bit 5, a             ; hardware collision test
  jr usr3.init         ; if collision detected, process collision...
    nop
    nop 
    nop
    bit 5, a           ; hardware collision test
    jp z, usr3.false   ; if collision not detected, go out
    nop
    nop 
    nop
    nop

usr3.init:
    ; copy sprite attribute table to ram
    ld hl, (ATRBAS)        ; source: attribute table
    ld de, BUF             ; dest: ram
    ld bc, 128             ; 32*4 = size of attribute table
    call LDIRMV

    ; pre-calculate each sprite width
    call usr3.GetSpriteSize
	ld c, a                     ; sprite size
    ld b, 32                    ; sprite count
	ld d, 208                   ; Y no-display flag
    ld hl, BUF                  ; start of sprites attributes

    ; test screen mode
    ld a, (SCRMOD)
    cp 3           ; above screen 3?
    jp c, usr3.init.next
      ld d, 216    ; Y no-display flag

usr3.init.next:
    ; test IC flag (no collision) in color table
    ; test EC flag (early clock, shift 32 dots to the left) in color table
    ; set x1 = x0 + size, y1 = y0 + size
    ld a, (hl)   ; y0
    cp d         ; test if sprite will not be displayed
    jr z, usr3.init.clear_all_sprite_data
      inc l
      ld e, (hl)   ; x0
      inc l

      ex af, af'
      ld a, (hl)   ; sprite pattern
      cp 32        ; test if pattern = 32
      jr z, usr3.init.clear_sprite_data

      ex af, af'
      add a, c     ; y0 + sprite_size
      ld (hl), a   ; y1
      inc l

      ld a, e      ; x0
      add a, c     ; x0 + sprite_size
      ld (hl), a   ; x1
      inc l

      djnz usr3.init.next
      jp usr3.check.collision

usr3.init.clear_sprite_data:
    dec l
    dec l
    ld (hl), 0xff  ; clear sprite data
    inc l
    inc l
    inc l
    inc l
    djnz usr3.init.next
    jp usr3.check.collision

usr3.init.clear_all_sprite_data:
    ld (hl), 0xff  ; clear sprite data
    inc l
    inc l
    inc l
    inc l
    djnz usr3.init.clear_all_sprite_data

usr3.check.collision:      ; DAC = sprite number
    ld hl, (DAC)

    xor a
    or h                 ; check if direct test of two sprite numbers
    jp z, usr3.check.collision.target      ; test all sprites against this sprite number
      bit 7, h               ; check if sprite number is negative (loop all sprites if so)
      jr z, usr3.check.collision.couple

usr3.check.collision.loop_all:
      ld hl, 0
      ld (DAC), hl

usr3.check.collision.loop.1:
      call usr3.check.collision.target
      ld a, l
      cp 0xFF
      ret nz

      ld hl, DAC
      inc (hl)
      ld a, (hl)
      cp 32
      jp nz, usr3.check.collision.loop.1   ; check next if current sprite < max sprite number

      jp usr3.false

usr3.check.collision.couple:
    ; calculate first sprite address (iy = hl)
    ld a, (DAC)
    ld l, a
    ld h, 0
    ld de, BUF
    add hl, hl            ; x4
    add hl, hl
    add hl, de
    push hl
    pop iy

    ; calculate second sprite address (iy = hl)
    ld a, (DAC+1)
    ld l, a
    ld h, 0
    ld de, BUF
    add hl, hl            ; x4
    add hl, hl
    add hl, de
    push hl
    pop ix

    ; test if x1 > nx and x < nx1 and y1 > ny and y < ny1
    ld a, (ix)   ; ny
    cp 0xFF      ; test if sprite will not be displayed
    jp z, usr3.false

    cp (iy+2)    ; y1
    jp nc, usr3.false

    ld a, (ix+1) ; nx
    cp (iy+3)    ; x1
    jp nc, usr3.false

    ld a, (iy+1) ; x
    cp (ix+3)    ; nx1
    jp nc, usr3.false

    ld a, (iy)   ; y
    cp (ix+2)    ; ny1
    jp nc, usr3.false

    ; if so, return collider sprite (bc)
    ld a, (DAC)
    jp usr3.true     ; return true

usr3.check.collision.target:      ; DAC = sprite number
    ; calculate target sprite address (iy = hl)
    ld hl, (DAC)
    ld de, BUF
    add hl, hl            ; sprite number * 4 + BUF
    add hl, hl
    add hl, de

    ld a, (hl)            ; y0
    cp 0xff               ; check if y0 = 0xFF
    jp z, usr3.false      ; if so, its a no visible sprite

    ld b, a               ; y0
    inc l
    ld c, (hl)            ; x0
    inc l
    ld d, (hl)            ; y1
    inc l
    ld e, (hl)            ; x1

    ; start test against others sprites
    ld hl, BUF
    xor a                 ; initialize sprite counter
    ex af, af'            ; save sprite counter

usr3.check.next:
    ; load next nx0, ny0, nx1, ny1
    ; skip if ny0 = 0xFF
    ; or test if x1 > nx0 and x0 < nx1 and y1 > ny0 and y0 < ny1
    ld a, (hl)            ; ny0
    cp 0xff
    jr z, usr3.check.skip.4

    cp d                  ; jump if ny0 >= y1
    jr nc, usr3.check.skip.4

    inc l

    ; skip if ny0 = y0 and nx0 = x0
    cp b                  ; jump if ny0 <> y0
    ld a, (hl)            ; nx0
    jr nz, usr3.check.next.cont
      cp c                ; jump if nx0 = x0
      jr z, usr3.check.skip.3

usr3.check.next.cont:
    cp e                  ; jump if nx0 >= x1?
    jr nc, usr3.check.skip.3

    inc l
    ld a, b               ; y0
    cp (hl)               ; jump if y0 >= ny
    jr nc, usr3.check.skip.2

    inc l
    ld a, c               ; x0
    cp (hl)               ; jump if  x0 >= nx1
    jr nc, usr3.check.skip.1

    ex af,af'             ; restore sprite counter
    jp usr3.true          ; return true

usr3.check.skip.4:
  inc l
usr3.check.skip.3:
  inc l
usr3.check.skip.2:
  inc l
usr3.check.skip.1:
  inc l
  ex af, af'            ; restore sprite counter
    inc a
    cp 32               ; jump if end of sprite list
    jr z, usr3.false
  ex af, af'            ; save sprite counter
  jp usr3.check.next

  ; else return false
usr3.false:
  ld hl, 0xFFFF    ; return false
  ld (DAC+2), hl
  ret

usr3.true:
  ld h, 0
  ld l, a
  ld (DAC+2), hl
  ret

usr3.GetSpriteSize:
  push bc
    ld bc, 0x0808
    ld a, (RG1SAV)  		; bit 0 = double size, bit 1 = sprite size (0=8 pixels, 1=16 pixels)
    bit 1, a
    jr z, usr3.GetSpriteSize.1
      ld bc, 0x1010

usr3.GetSpriteSize.1:
    bit 0, a
    jr z, usr3.GetSpriteSize.2
      sll b

usr3.GetSpriteSize.2:
    ld (ARG), bc
    ld a, c
  pop bc
  ret

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

;---------------------------------------------------------------------------------------------------------
; LOGICAL / MATH INTEGER ROUTINES
;---------------------------------------------------------------------------------------------------------

intCompareEQ:
  and a
  sbc hl,de
  ld hl, 0xFFFF
  ret z
    inc hl
    ret

intCompareNE:
  and a
  sbc hl,de
  ld hl, 0xFFFF
  ret nz
    inc hl
    ret

intCompareLT:
  ex de,hl
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  sbc hl,hl
  ret

intCompareLE:
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  ccf
  sbc hl,hl
  ret

intCompareGT:
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  sbc hl,hl
  ret

intCompareGE:
  ex de,hl
  ld a, h
  xor 0x80
  ld h, a
  ld a, d
  xor 0x80
  ld d, a
  sbc hl,de
  ccf
  sbc hl,hl
  ret

intCompareNOT:
  ld a, l
  cpl
  ld l, a
  ld a, h
  cpl
  ld h, a
  ret

intCompareAND:
  ld a, l
  and e
  ld l, a
  ld a, h
  and d
  ld h, a
  ret

intCompareOR:
  ld a, l
  or e
  ld l, a
  ld a, h
  or d
  ld h, a
  ret

intCompareXOR:
  ld a, l
  xor e
  ld l, a
  ld a, h
  xor d
  ld h, a
  ret

intSHR:
  ex de, hl
  ld a, e
  or d
  jr z, intSHR.2
intSHR.1:
    xor a
    sra h
    rr l
    dec e
    jr nz, intSHR.1
intSHR.2:
  ret

intSHL:
  ex de, hl
  ld a, e
  or d
  jr z,intSHL.2
intSHL.1:
    add hl, hl     ; optimization (xor a; sla l; rl h)
    dec e
    jr nz, intSHL.1
intSHL.2:
  ret

intNeg:
  xor a
  sub l
  ld l,a
  sbc a,a
  sub h
  ld h,a
  ret

floatNeg:
  ex de, hl
  ld c, b
  xor a
  ld b, a
  ld l, a
  ld h, a
  jp XBASIC_SUBTRACT_FLOATS     ; calculate zero - float

castParamFloatInt:
  ex de,hl      ; swap old hl (float)
  pop hl        ; get return address
  ex (sp),hl    ; swap int from stack
  push de       ; save old float into stack
  push bc
  call XBASIC_CAST_INTEGER_TO_FLOAT   ; xbasic int to float (in hl, out b:hl)
  pop af
  pop de        ; restore old float
  ex (sp), hl   ; put new float into stack
  push bc
  push hl       ; put return address into stack
  ex de,hl      ; restore old float
  ld b, a
  ret


;---------------------------------------------------------------------------------------------------------
; SUPPORT ROUTINES
;---------------------------------------------------------------------------------------------------------

; calculate text screen location
; in: HL = xy (bios based coord system where home = 1,1)
; out: HL = vram address
gfxTileAddress:
  ld a, (VERSION)
  and a
  jp z, CHCOORD
  jp CHCOORD2

; in: A=Sprite pattern number
; out: HL=Sprite pattern address
gfxCALPAT:
  ld iy, SCRMOD
  ld ix, CALPAT2
  bit 3, (iy)
  jp nz, SUB_EXTROM
  bit 2, (iy)
  jp nz, SUB_EXTROM
  jp CALPAT

; in: A=Sprite number
; out: HL=Sprite attribute address
gfxCALATR:
  ld iy, SCRMOD
  ld ix, CALATR2
  bit 3, (iy)
  jp nz, SUB_EXTROM
  bit 2, (iy)
  jp nz, SUB_EXTROM
  jp CALATR

; in:  A = sprite number
; out: HL = address to color table
gfxCALCOL:
  push af
  push de
    ld h, 0
    ld l, a         ; recover sprite number
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl      ; multiply by 16 (shift left 4)
    push hl
      xor a
      call gfxCALATR    ; get sprite attribute table address
    pop de
    add hl, de
    xor a
    ld de, 512
    sbc hl, de      ; address of color table from sprite multicolor
  pop de
  pop af
  ret

;---------------------------------------------------------------------------------------------------------
; MEGAROM SUPPORT CODE
; out (0x8E), a - MEGAROM SELECT TO READ
;---------------------------------------------------------------------------------------------------------

Seg_P8000_SW:	 equ 0x9000 ; Segment switch on page 8000h-9FFFh (Konami with SCC Mapper)
Seg_PA000_SW:	 equ 0xB000 ; Segment switch on page A000h-BFFFh (Konami with SCC Mapper)
MR_TRAP_FLAG:    equ 0xFC82
MR_TRAP_SEGMS:   equ MR_TRAP_FLAG+1

MR_JUMP:
  push hl
MR_CHANGE_SGM:
  ld (SGMADR), a
  ld (Seg_P8000_SW), a
  inc a
  ld (Seg_PA000_SW), a
  ret                     ; indirect jump

MR_CALL:
  ld bc, (SGMADR-1)       ; salve old segment number
  push bc
  call MR_CHANGE_SGM
  ld de, MR_CALL_RET      ; restore old segment code
  push de
  push hl                 ; called segment code
  ex af, af'              ; restore old registers values (call parameters)
  exx
  ret                     ; indirect call
MR_CALL_RET:
  pop af                  ; restore old segment number
  jp MR_CHANGE_SGM

MR_GET_DATA:
  ld bc, (SGMADR-1)       ; salve old segment number
  push bc
    call MR_CHANGE_SGM
    ; copy string from hl to TEMPORARY STRING BUFFER
    ld b, 0
    ld c, (hl)
    inc bc
    ex de, hl
      call GET_NEXT_TEMP_STRING_ADDRESS
      ld (TMPSTRBUF), hl
    ex de, hl
    ldir
  pop af                  ; restore old segment number
  call MR_CHANGE_SGM
  ex af, af'              ; restore old registers values (call parameters)
  exx
  ld hl, (TMPSTRBUF)      ; data restored from segment number
  ret

MR_GET_BYTE:
  ld bc, (SGMADR-1)       ; salve old segment number
  push bc
    call MR_CHANGE_SGM
    ld c, (hl)
    inc hl
    ld b, (hl)
  pop af                  ; restore old segment number
  call MR_CHANGE_SGM
  ld a, c
  ret

MR_CALL_TRAP:               ; call trap in DE (wrapper to C6C25 in J6C5F at 6C6F)
  ld a, (MR_TRAP_FLAG)      ; start of TRPTBL reserved area (megaROM flag)
  or a
  jp z, C6C25               ; jump if not megaROM trappers
    push de
      ld de, TRPTBL         ; start of TRPTBL
      xor a
      sbc hl, de
      ld de, 3
      call XBASIC_DIVIDE_INTEGERS
      ld de, MR_TRAP_SEGMS  ; start of TRPTBL reserved area (segments list)
      add hl, de
      ld a, (hl)            ; segment to call
    pop hl
    jp MR_CALL

GET_NEXT_TEMP_STRING_ADDRESS:
    push af
    push bc
      ld a, (TMPSTRIND)      ; temporary string list current position
      ld b, a
      ld c, 0
      ld hl, (TMPSTRADDR)    ; temporary string list start address
      add hl, bc
      inc a
      cp 5
      jr nz, GET_NEXT_TEMP_STRING_ADDRESS.1
        xor a
GET_NEXT_TEMP_STRING_ADDRESS.1:
      ld (TMPSTRIND), a
    pop bc
    pop af
    ret

; BLOAD STATEMENT
; parameters: hl = block start, bc = block count
; SCn file format:
;   db #fe 	;ID byte
;   dw {VRAM begin address}
;   dw {VRAM end address}
;   dw {not used when loading to VRAM}
XBASIC_BLOAD:
  ld a, (hl)
  and a
  ret z

  push bc
  push hl

  inc hl
  ld de, BUF

  call resource.ram.unpack ; in hl = packed data, de = ram destination; out bc = size, hl=destination

  inc hl
  ld e, (hl)
  inc hl
  ld d, (hl)

  inc hl
  inc hl
  inc hl
  inc hl
  inc hl

  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc
  dec bc

XBASIC_BLOAD.loop:
  push de
  push bc
    call LDIRVM    ; hl = ram data address, de = vram data address, bc = length
  pop bc
  pop hl
  add hl, bc
  ex de, hl

  pop hl
  pop bc
  dec bc
  ld a, c
  or b
  ;jr z, XBASIC_BLOAD.end
  ret z

  push bc

  ld c, (hl)
  ld b, 0
  add hl, bc
  inc hl

  push hl
    ld c, (hl)
    add hl, bc
    ld a, h
  pop hl
  cp 0xC0
  jr c, XBASIC_BLOAD.cont
    ld a, (SGMADR)
    inc a
    inc a
    ld hl, 0x8000
    call MR_CHANGE_SGM

XBASIC_BLOAD.cont:
  push hl
  push de

  inc hl
  ld de, BUF

  call resource.ram.unpack ; in hl = packed data, de = ram destination; out bc = size, hl=destination

  pop de

  jr XBASIC_BLOAD.loop

;XBASIC_BLOAD.end:
;  ld ix, 0x0145   ; color restore
;  jp EXTROM

;---------------------------------------------------------------------------------------------------------
; PT3TOOLS support routines
;---------------------------------------------------------------------------------------------------------

initialize.pt3:
  ld a, (PT3FLG)
  or a                               ; pt3 tools disabled?
  ret z

  ld a, (RAMAD2)
  cp 0xFF
  jr z, initialize.pt3.on_page_0     ; jump if no RAM on page 2

  di
    call select_ram_on_page_2

    ld hl, pt3.resource.data
    ld de, PT3.LOAD
    call pletter.unpack              ; hl = packed data, de = ram destination

    call select_rom_on_page_2

    jr hook.pt3

initialize.pt3.on_page_0:
  ld a, (RAMAD0)
  cp 0xFF
  ret z                              ; return if no RAM on page 0

  di
    call select_ram_on_page_0

    ld hl, pt3.resource.data
    ld de, 0x0000
    call pletter.unpack              ; hl = packed data, de = ram destination

    call patch.pt3.on_page_0

    call select_rom_on_page_0

hook.pt3:
    xor a
    ld (PT3STS), a                   ; idle
  ei

  ; save old hook
  ld hl, HTIMI      ;OLD HOOK SAVE
  ld de, PT3HKSAV
  ld bc, 5
  ldir

  di
	; set new hook
	ld a, 0xF7          ; rst 0x30 - CALLF
    ld (HTIMI), a
	ld a, (SLTAD1)      ; a = program rom slot
    ld (HTIMI+1), a
    ld hl, int.pt3
	ld (HTIMI+2), hl
	ld a, 0xC9          ; ret
	ld (HTIMI+4), a
  ei

  ret

unhook.pt3:
  ld a, (PT3FLG)
  or a                  ; pt3 tools disabled?
  ret z

  ld a, (RAMAD2)
  cp 0xFF
  jr nz, unhook.pt3.1   ; unhook if RAM on page 2

    ld a, (RAMAD0)
    cp 0xFF
    ret z               ; return if no RAM on page 0

unhook.pt3.1:
  di
    ld hl, PT3HKSAV
    ld de, HTIMI
    ld bc, 5
    ldir
  ei
  jp GICINI

int.pt3:
    push af
      ld a, (PT3STS)
      cp 1             ; play
int.pt3.patch.1:
      jp z, int.pt3.play.on_page_2

      cp 0             ; idle
      jp z, int.pt3.exit

      cp 2             ; mute
int.pt3.patch.2:
      jp z, int.pt3.mute.on_page_2

int.pt3.exit:
    pop af
    jp PT3HKSAV

int.pt3.play.on_page_0:
    ;ld iy, (RAMAD0-1)
    ;ld ix, PT3.PLAY - PT3.LOAD
    ;call CALSLT
    call select_ram_on_page_0
    call PT3.PLAY - PT3.LOAD
    call select_rom_on_page_0
    jr int.pt3.exit

int.pt3.play.on_page_2:
    call select_ram_on_page_2
    call PT3.PLAY
    call select_rom_on_page_2
    jr int.pt3.exit

int.pt3.mute.on_page_0:
    ;ld iy, (RAMAD0-1)
    ;ld ix, PT3.MUTE - PT3.LOAD
    ;call CALSLT
    call select_ram_on_page_0
    call PT3.MUTE - PT3.LOAD
    call select_rom_on_page_0
    jr int.pt3.mute.on_page_2.cont

int.pt3.mute.on_page_2:
    call select_ram_on_page_2
    call PT3.MUTE
    call select_rom_on_page_2

int.pt3.mute.on_page_2.cont:
    xor a          ; idle
    ld (PT3STS), a
    jr int.pt3.exit


;---------------------------------------------------------------------------------------------------------
; PATCH PT3/CMD TO USE PAGE 0 AS TEMPORARY BUFFER
;---------------------------------------------------------------------------------------------------------

patch.pt3.on_page_0:
  ld de, patch.pt3.data.1     ;de=source array
  ld b, 144                   ;b=items count
  ld hl, 0x0000               ;hl=dest memory

patch.pt3.on_page_0.do:
  ld a, (de)
  inc de

  push de
    ld e, a
    ld d, 0x00
    add hl, de
  pop de

  ld c, (hl)
  res 7, c
  ld (hl), c

  djnz patch.pt3.on_page_0.do
  ret

patch.cmd.on_page_1:
  in a, (0x8E)      ; MegaROM mode 1 - block change off

  ld hl, patch.cmd.data.1
  ld bc, select_ram_on_page_0
  ld a, 6
  call patch.cmd.on_page_1.do

  ld bc, select_rom_on_page_0
  ld a, 6
  call patch.cmd.on_page_1.do

  ld bc, RAMAD0
  ld a, 4
  call patch.cmd.on_page_1.do

  ld bc, PT3.DATA.STATUS - PT3.LOAD
  ld a, 5
  call patch.cmd.on_page_1.do

  ld bc, PT3.DATA.ADDR - PT3.LOAD
  ld a, 2
  call patch.cmd.on_page_1.do

  ld bc, PT3.DATA.TEMP - PT3.LOAD
  ld a, 4
  call patch.cmd.on_page_1.do

  ld bc, PT3.LOAD_FROM_HL - PT3.LOAD
  ld a, 1
  call patch.cmd.on_page_1.do

  ld bc, int.pt3.play.on_page_0
  ld a, 1
  call patch.cmd.on_page_1.do

  ld bc, int.pt3.mute.on_page_0
  ld a, 1
  call patch.cmd.on_page_1.do

  out (0x8E), a     ; MegaROM mode 0 - block change on

  ret

patch.cmd.on_page_1.do:      ;hl=dest array, bc=source data, a=items count
  ld e, (hl)
  inc hl
  ld d, (hl)
  inc hl

  ex de, hl

  ld (hl), c
  inc hl
  ld (hl), b

  ex de, hl

  dec a
  jr nz, patch.cmd.on_page_1.do
  ret

patch.cmd.data.1:   ;select_ram_on_page_0
  dw cmd_wrtvram.patch.3+1
  dw cmd_pt3load.patch.2+1
  dw cmd_pt3load.patch.6+1
  dw cmd_pt3loop.patch.1+1
  dw usr2_pt3status.patch.1+1
  dw memory.copy_from_rom_to_ram.1.patch.1+1

patch.cmd.data.2:   ;select_rom_on_page_0
  dw cmd_wrtvram.patch.4+1
  dw cmd_pt3load.patch.5+1
  dw cmd_pt3load.patch.11+1
  dw cmd_pt3loop.patch.4+1
  dw usr2_pt3status.patch.3+1
  dw memory.copy_from_rom_to_ram.1.patch.2+1

patch.cmd.data.3:   ;RAMAD0
  dw cmd_wrtvram.patch.1+1
  dw cmd_pt3load+1
  dw cmd_pt3replay+1
  dw cmd_pt3loop+1

patch.cmd.data.4:   ;PT3.DATA.STATUS - PT3.LOAD
  dw cmd_pt3load.patch.7+1
  dw cmd_pt3load.patch.8+1
  dw cmd_pt3loop.patch.2+1
  dw cmd_pt3loop.patch.3+1
  dw usr2_pt3status.patch.2+1

patch.cmd.data.5:   ;PT3.DATA.ADDR - PT3.LOAD
  dw cmd_pt3load.patch.4+1
  dw cmd_pt3load.patch.9+1

patch.cmd.data.6:   ;PT3.DATA.TEMP - PT3.LOAD
  dw cmd_wrtvram.patch.2+1
  dw cmd_pt3load.patch.1+1
  dw cmd_pt3load.patch.3+1
  dw cmd_wrtvram.patch.3.1+1

patch.cmd.data.7:   ;PT3.LOAD_FROM_HL - PT3.LOAD
  dw cmd_pt3load.patch.10+1

patch.cmd.data.8:   ;PT3.PLAY
  dw int.pt3.patch.1+1

patch.cmd.data.9:   ;PT3.MUTE
  dw int.pt3.patch.2+1

patch.pt3.data.1:   ;pt3 on page 0
  db 2,   7-2,  15-7,  24-15,  28-24,  32-28,  37-32,  43-37,  46-43
  db 49-46,  52-49,  55-52,  64-55,  71-64,  79-71,  90-79,  97-90
  db 104-97, 107-104, 112-107, 115-112, 147-115, 151-147, 160-151
  db 166-160, 169-166, 172-169, 175-172, 178-175, 181-178, 184-181, 187-184, 190-187, 193-190
  db 196-193, 212-196, 224-212, 239-224, 246-239, 255-246, 303-255, 312-303, 351-312, 355-351
  db 393-355, 400-393, 456-400, 461-456, 472-461, 482-472, 491-482
  db 536-491, 548-536, 568-548, 605-568, 678-605, 757-678, 760-757, 769-760, 775-769, 782-775
  db 791-782, 798-791, 803-798, 831-803, 833-831, 835-833, 837-835
  db 839-837, 841-839, 843-841, 845-843, 847-845, 849-847, 851-849, 853-851, 855-853, 857-855
  db 859-857, 861-859, 865-861, 873-865, 877-873, 906-877, 970-906
  db 1037-970,1087-1037,1103-1087,1115-1103,1141-1115,1152-1141,1166-1152,1206-1166,1209-1206
  db 1213-1209,1216-1213,1222-1216,1236-1222,1239-1236,1247-1239,1255-1247,1268-1255
  db 1272-1268,1282-1272,1287-1282,1294-1287,1297-1294,1301-1297,1304-1301,1311-1304,1317-1311
  db 1321-1317,1324-1321,1331-1324,1337-1331,1341-1337,1346-1341,1350-1346,1353-1350
  db 1356-1353,1359-1356,1362-1359,1365-1362,1369-1365,1372-1369,1375-1372,1378-1375,1381-1378
  db 1384-1381,1388-1384,1391-1388,1394-1391,1397-1394,1400-1397,1405-1400,1414-1405
  db 1419-1414,1423-1419,1427-1423,1443-1427,1449-1443,1465-1449,1485-1465,1505-1485

;---------------------------------------------------------------------------------------------------------
; MEMORY / SLOT / PAGE ROUTINES
;---------------------------------------------------------------------------------------------------------

select_rom_on_page_2:
  ld a, (SLTAD2)
  ld h,080h
  call ENASLT		; Select the ROM on page 8000h-BFFFh

  ld a, (MAPPER)
  bit 0, a          ; mapper on?
  ret z             ; return if no mapper

  ld a, (MPRAD2)    ; original memory mapper segment for page 2 rom
  out (0xFE), a
  ret

select_ram_on_page_2:
  ld a, (RAMAD2)
  ld h,080h
  call ENASLT		; Select the RAM on page 8000h-BFFFh

  ld a, (MAPPER)
  bit 0, a          ; mapper on?
  ret z             ; return if no mapper

  ld a, (MPRAD2N)   ; new memory mapper segment for page 2 ram
  out (0xFE), a
  ret

select_rom_on_page_0:
  ld a, (SLTAD0)
  ld h,000h
  jp SUB_ENASLT		; Select the BIOS ROM

select_ram_on_page_0:
  ld a, (RAMAD0)
  ld h,000h
  jp SUB_ENASLT		; Select RAM on page 0

; h = memory page
; a <- slot ID formatted FxxxSSPP
; Modifies: af, bc, de, hl
; ref: https://www.msx.org/forum/msx-talk/development/fusion-c-and-htimi#comment-366469
page.getslt:
	call RSLREG
	bit 7,h
	jr z,PrimaryShiftContinue
	rrca
	rrca
	rrca
	rrca
PrimaryShiftContinue:
	bit 6,h
	jr z,PrimaryShiftDone
	rrca
	rrca
PrimaryShiftDone:
	and 00000011B
	ld c,a
	ld b,0
	ex de,hl
	ld hl,EXPTBL
	add hl,bc
	ld c,a
	ld a,(hl)
	and 80H
	or c
	ld c,a
	inc hl  ; move to SLTTBL
	inc hl
	inc hl
	inc hl
	ld a,(hl)
	ex de,hl
	bit 7,h
	jr z,SecondaryShiftContinue
	rrca
	rrca
	rrca
	rrca
SecondaryShiftContinue:
	bit 6,h
	jr nz,SecondaryShiftDone
	rlca
	rlca
SecondaryShiftDone:
	and 00001100B
	or c
	ret

; verify default slots and memory ram
verify.slots:
    ld h, 0x00          ; page 0
    call page.getslt
    ld (SLTAD0), a

    ld h, 0x40          ; page 1
    call page.getslt
    ld (SLTAD1), a

    ld h, 0x80          ; page 2
    call page.getslt
    ld (SLTAD2), a
    ld (RAMAD2), a      ; presume RAM on page 2

    ld h, 0xC0          ; page 3
    call page.getslt
    ld (SLTAD3), a

    ld (RAMAD0), a      ; presume same slot/subslot of page 3 as RAM for pages left
    ld (RAMAD1), a
    ld (RAMAD3), a

verify.slots.test:
    xor a
    ld (MAPPER), a      ; default = mapper off, no running on RAM

    ld h, 0x41          ; test if running on RAM (SofaRun/ExecRom)
    call verify.slots.test.ram
    jp z, verify.slots.set.running.on.ram

      ld a, (SLTSTR)
      ld (SLTAD2), a    ; slot on page 2 must be the same as page 1

      ld h, 0x81        ; test RAM on page 2
      call verify.slots.test.ram
      jr z, verify.slots.test.mapper.mirror

        ld a, 0xFF
        ld (RAMAD0), a  ; no RAM on page 0
        ld (RAMAD1), a  ; no RAM on page 1
        ld (RAMAD2), a  ; no RAM on page 2
        ret

verify.slots.test.mapper.mirror:
      ld h, 0xD1        ; page 3 memory address
      ld c, (hl)        ; save original value on page 3
      ld a, 65
      ld (hl), a        ; change value on page 3 memory address
      ld h, 0x91        ; page 2 memory address
      cp (hl)           ; test page 2 value against page 3
      ld h, 0xD1
      ld (hl), c        ; restore original value on page 3

      ret nz            ; no RAM mirrored (pages 2 with 3)

      ; else, set mapper in page 2

verify.slots.set.mapper:
    ld a, (MAPPER)
    set 0, a            ; set bit 0 = mapper on
    ld (MAPPER), a

    in a, (0xFF)        ; get memory mapper segment on page 3
    cpl
    ld c, a             ; discard higher bits
    srl a
    or c
    srl a
    or c
    srl a
    or c
    ld c, a

    in a, (0xFE)        ; get memory mapper segment on page 2
    and c
    ld (MPRAD2), a

    cp 0x0F
    jr z, verify.slots.set.mapper.1
       ld a, 0x10       ; force segment 15 as temporary RAM (presume 256k mapper)

verify.slots.set.mapper.1:
    dec a
    ld (MPRAD2N), a     ; new memory mapper segment on page2

    ret

verify.slots.set.running.on.ram:
    ld a, (MAPPER)
    set 1, a            ; set bit 1 = running on ram
    ld (MAPPER), a

    ld a, (SLTAD1)      ; get slot/subslot from page 1 (ram)
    ld (RAMAD0), a      ; presume RAM on page 0 in same slot/subslot as page 1 (ram)

verify.slots.test.mapper:       ; test mapper on page 2
    call verify.slots.set.mapper

    in a, (0x8E)        ; MegaROM mode 1 - block change off

    ld h, 0x80
    ld c, (hl)          ; save original value
    ld a, 65
    ld (hl), a

    ld a, (MPRAD2N)     ; new memory mapper segment for page 2 ram
    out (0xFE), a

    ld a, 65
    cp (hl)

    ld a, (MPRAD2)      ; original memory mapper segment for page 2 rom
    out (0xFE), a

    ld (hl), c          ; restore original value

    out (0x8E), a       ; MegaROM mode 0 - block change on

    jr nz, verify.slots.test.program.on_page_2   ; has mapper, so test if program allocated...

      ld a, (MAPPER)
      res 0, a            ; reset bit 0 = mapper off
      ld (MAPPER), a

verify.slots.test.megaram:      ; test megaram on page 2

    call verify.slots.test.megaram.fix.1
    ld h, 0x80
    call SUB_ENASLT

    ; change page 2 block to 0 (presume megarom mode 0 - block change on)
    xor a
    ld (Seg_P8000_SW), a     ; change block

    ; get data value on block
    ld hl, 0x8010
    ld a, (hl)

    ; compare data of page 2 against same address on page 1
    ld h, 0x40
    cp (hl)

    ; jump if not the same
    jr nz, verify.slots.test.megaram.nope
        ; else, change page 2 to block 2
        ld a, 2
        ld (Seg_P8000_SW), a
        ; set page 2 slot to same slot of page 1 and return
        ld a, (SLTAD1)
        ld (SLTAD2), a
        ret

verify.slots.test.megaram.nope:
    ld a, (RAMAD2)              ; return page 2 to RAM slot....
    ld h, 0x80
    call verify.slots.test.megaram.fix.2

    ld a, 0xFF
    ld (RAMAD2), a              ; RAM on page 2 already used (not usable)

    call patch.cmd.on_page_1    ; patch page 1 to use page 0 as temporary memory buffer

    ret

verify.slots.test.ram:
    in a, (0x8E)        ; MegaROM mode 1 - block change off
    ld c, (hl)          ; save original value
    ld a, 65
    ld (hl), a
    cp (hl)             ; test if value was changed in address
    ld (hl), c          ; restore original value
    out (0x8E), a       ; MegaROM mode 0 - block change on
    ret

verify.slots.test.program.on_page_2:
    xor a
    ld hl, (0x8010)
    ld de, 0x273E       ; ld a, 39 (on 0x8010)
    sbc hl, de
    ret z

    ; set page 2 slot to same slot of page 1 and return
    ld a, (SLTAD1)
    ld (SLTAD2), a
    ret


;---------------------------------------------------------------------------------------------------------
; ENASLT alternative for use of ram on page 0
;---------------------------------------------------------------------------------------------------------

D.FFFF:  EQU     0FFFFH  ; secundairy slotregister

SUB_ENASLT:
        CALL    ENASLT.MASK             ; calculate masks
        JP      M,ENASLT.EXPSLT         ; expanded slot
        IN      A,(0A8H)
        AND     C
        OR      B
        OUT     (0A8H),A
        RET

ENASLT.EXPSLT:
        PUSH    HL
        CALL    ENASLT.SUBSLT           ; adjust secundairy slotregister
        LD      C,A
        LD      B,00H
        LD      A,L
        AND     H
        OR      D
        LD      HL,SLTTBL
        ADD     HL,BC
        LD      (HL),A                  ; update SLTTBL
        POP     HL
        LD      A,C
        JR      SUB_ENASLT              ; do a ENASLT on only the primary slot

ENASLT.MASK:
        DI
        PUSH    AF
        LD      A,H
        RLCA
        RLCA
        AND     03H
        LD      E,A                     ; page
        LD      A,0C0H
ENASLT.MASK.1:
        RLCA
        RLCA
        DEC     E
        JP      P,ENASLT.MASK.1
        LD      E,A                     ; page select mask
        CPL
        LD      C,A                     ; page clear mask
        POP     AF
        PUSH    AF
        AND     03H
        INC     A
        LD      B,A
        LD      A,0ABH
ENASLT.MASK.2:
        ADD     A,55H
        DJNZ    ENASLT.MASK.2
        LD      D,A                     ; PPPPPPPP
        AND     E
        LD      B,A                     ; prim. slot select
        POP     AF
        AND     A
        RET

ENASLT.SUBSLT:
        PUSH    AF
        LD      A,D
        AND     0C0H
        LD      C,A
        POP     AF
        PUSH    AF
        LD      D,A
        IN      A,(0A8H)
        LD      B,A
        AND     3FH
        OR      C
        OUT     (0A8H),A                ; set page 3 to requested primary slot (to access sec. slotreg)
        LD      A,D
        RRCA
        RRCA
        AND     03H
        LD      D,A
        LD      A,0ABH
ENASLT.SUBSLT.1:
        ADD     A,55H
        DEC     D
        JP      P,ENASLT.SUBSLT.1
        AND     E
        LD      D,A
        LD      A,E
        CPL
        LD      H,A
        LD      A,(D.FFFF)
        CPL
        LD      L,A
        AND     H
        OR      D
        LD      (D.FFFF),A
        LD      A,B
        OUT     (0A8H),A                ; restore primary slotreg
        POP     AF
        AND     03H
        RET

;---------------------------------------------------------------------------------------------------------
; RAM to VRAM alternative (needs DI/EI)
; in: BC=Length, dest DE=VRAM address, source HL=RAM address
;---------------------------------------------------------------------------------------------------------

SUB_LDIRVM:
  ex      de,hl
  ld a,l           ; initialize (msx1)
  out     (099H),a
  ld      a,h
  and     03FH
  or      040H
  out     (099H),a
SUB_LDIRVM.loop:
  ld      a,(de)
  out     (098H),a
  inc     de
  dec     bc
  ld      a,c
  or      b
  jr      nz, SUB_LDIRVM.loop
  ret

;---------------------------------------------------------------------------------------------------------
; Detect VDP version
;
; a <- 0: TMS9918A, 1: V9938, 2: V9958, x: VDP ID
; f <- z: TMS9918A, nz: other
;---------------------------------------------------------------------------------------------------------

VDP_GetVersion:
    call VDP_IsTMS9918A  ; use a different way to detect TMS9918A
    ret z
    ld a,1               ; select s#1
    di
    out (99H),a
    ld a,15 + 128
    out (99H),a
    in a,(99H)           ; read s#1
    and 00111110B        ; get VDP ID
    rrca
    ex af,af'
    xor a                ; select s#0 as required by BIOS
    out (99H),a
    ld a,15 + 128
    ei
    out (99H),a
    ex af,af'
    ret nz               ; return VDP ID for V9958 or higher
    inc a                ; return 1 for V9938
    ret

;
; Test if the VDP is a TMS9918A.
;
; f <- z: TMS9918A, nz: V99X8
;
VDP_IsTMS9918A:
    in a,(99H)           ; read s#0, make sure interrupt flag is reset
    di
VDP_IsTMS9918A_Wait:
    in a,(99H)           ; read s#0
    and a                ; wait until interrupt flag is set
    jp p,VDP_IsTMS9918A_Wait
    ld a,2               ; select s#2 on V9938
    out (99H),a
    ld a,15 + 128        ; (this mirrors to r#7 on TMS9918 VDPs)
    out (99H),a
    in a,(99H)           ; read s#2 / s#0
    ex af,af'
    xor a                ; select s#0 as required by BIOS
    out (99H),a
    ld a,15 + 128
    out (99H),a
    ld a,(0F3E6H)
    out (99H),a          ; restore r#7 if it mirrored (small flash visible)
    ld a,7 + 128
    ei
    out (99H),a
    ex af,af'
    and 01000000B        ; check if bit 6 was 0 (s#0 5S) or 1 (s#2 VR)
    ret

;---------------------------------------------------------------------------------------------------------
; Pletter v0.5c1 adapted from XL2S Entertainment 2008
; https://github.com/nanochess/Pletter
; PLETTER UNPACKED RAM TO RAM
; HL = packed data in RAM, DE = destination in RAM
; define lengthindata when the original size is written in the pletter data
; define LENGTHINDATA
;---------------------------------------------------------------------------------------------------------

  macro PLETTER.GETBIT
    add a,a
    call z,pletter.getbit
  endm

  macro PLETTER.GETBITEXX
    add a,a
    call z,pletter.getbitexx
  endm

pletter.modes
  dw pletter.offsok
  dw pletter.mode2
  dw pletter.mode3
  dw pletter.mode4
  dw pletter.mode5
  dw pletter.mode6

pletter.unpack:

  if defined LENGTHINDATA
    inc hl
    inc hl
  endif

  ld a,(hl)
  inc hl
  exx
  ld de,0
  add a,a
  inc a
  rl e
  add a,a
  rl e
  add a,a
  rl e
  rl e
  ld hl,pletter.modes
  add hl,de
  ld e,(hl)
  ld ixl,e
  inc hl
  ld e,(hl)
  ld ixh,e
  ld e,1
  exx
  ld iy,pletter.loop
pletter.literal:
  ldi
pletter.loop:
  PLETTER.GETBIT
  jr nc,pletter.literal
  exx
  ld h,d
  ld l,e
pletter.getlen:
  PLETTER.GETBITEXX
  jr nc,pletter.lenok
pletter.lus:
  PLETTER.GETBITEXX
  adc hl,hl
  ret c
  PLETTER.GETBITEXX
  jr nc,pletter.lenok
  PLETTER.GETBITEXX
  adc hl,hl
  ret c
  PLETTER.GETBITEXX
  jp c,pletter.lus
pletter.lenok
  inc hl
  exx
  ld c,(hl)
  inc hl
  ld b,0
  bit 7,c
  jp z,pletter.offsok
  push ix
  ret       ;jp ix

pletter.mode6
  PLETTER.GETBIT
  rl b
pletter.mode5
  PLETTER.GETBIT
  rl b
pletter.mode4
  PLETTER.GETBIT
  rl b
pletter.mode3
  PLETTER.GETBIT
  rl b
pletter.mode2
  PLETTER.GETBIT
  rl b
  PLETTER.GETBIT
  jr nc,pletter.offsok
  or a
  inc b
  res 7,c
pletter.offsok
  inc bc
  push hl
  exx
  push hl
  exx
  ld l,e
  ld h,d
  sbc hl,bc
  pop bc
  ldir
  pop hl
  ;jp iy
  push iy
  ret

pletter.getbit
  ld a,(hl)
  inc hl
  rla
  ret

pletter.getbitexx
  exx
  ld a,(hl)
  inc hl
  exx
  rla
  ret

;---------------------------------------------------------------------------------------------------------
; RESOURCES CODE
;---------------------------------------------------------------------------------------------------------

; out: de = resource count
resource.count:
  ld hl, (resource.data)

  inc hl
  inc hl       ; skip pt3tools address

  ld e, (hl)   ; resource count
  inc hl
  ld d, (hl)
  inc hl

  ret

; in:  bc = resource number
; out: hl = resource address
resource.address:
  call resource.count

  dec de

  ld a, d
  cp b
  ret c       ; return if resource number > resource count
  jr nz, resource.address.next

  ld a, e
  cp c
  ret c       ; return if resource number > resource count

resource.address.next:
  ex de, hl
  ld l, c
  ld h, b
  add hl, hl  ; x2
  add hl, hl  ; x4
  ex de, hl

  add hl, de

  ld e, (hl)  ; get resource address
  inc hl
  ld d, (hl)
  inc hl

  ex de, hl
  xor a
  ret

; in:  bc = resource number
; out: bc = resource size
resource.size:
  call resource.address
  ret c
  ex de, hl
  ld c, (hl)
  inc hl
  ld b, (hl)
  xor a
  ret

; in:  hl = pletter data
;      de = ram address
; out: hl = ram address
;      bc = size
resource.ram.unpack:
  push de
    call pletter.unpack
    exx       ; calculate decoded length (de' point to end of decoded data)
    ex de, hl
  pop de
  push de
    xor a
    sbc hl, de
    ld b, h
    ld c, l                 ; bc = decoded length
  pop hl                    ; hl = decoded data
  ret

; in: bc = resource number, de = dest RAM address
; out: (RESADDR) = next byte in RAM address
resource.copy_from_rom_to_ram:
  push bc
  push de
    call resource.address    ; hl = resource start address
  pop de
  pop bc
  ret c

  push hl
  push de
    call resource.size       ; bc = resource size
  pop de
  pop hl

; in: hl = source ROM address, de = dest RAM address, bc = size
; out: (RESADDR) = next byte in RAM address
memory.copy_from_rom_to_ram:
  ld (RESADDR), de

memory.copy_from_rom_to_ram.1:
  push bc

    ld de, BUF
    ld bc, 256
    ldir

    push hl

      di
memory.copy_from_rom_to_ram.1.patch.1:
        call select_ram_on_page_2
        ld hl, BUF
        ld de, (RESADDR)
        ld bc, 256
        ldir
        ld (RESADDR), de
memory.copy_from_rom_to_ram.1.patch.2:
        call select_rom_on_page_2
      ei

    pop hl

  pop bc

  xor a
  cp b
  ret z

  dec b
  jp memory.copy_from_rom_to_ram.1

;---------------------------------------------------------------------------------------------------------
; RESOURCE DATA
;---------------------------------------------------------------------------------------------------------

pt3.resource.data:
  db 0x20, 0x21, 0xaf, 0x88, 0x18, 0x2d, 0xc3, 0x04, 0xb3, 0x84, 0x18, 0x14
  db 0x00, 0x00, 0x21, 0x00, 0x0a, 0x80, 0xcb, 0xfe, 0xcb, 0x46, 0xc8, 0xe1
  db 0x02, 0x21, 0xe9, 0x86, 0x34, 0x21, 0xad, 0x03, 0x42, 0x3a, 0x10, 0xe6
  db 0x81, 0x32, 0x04, 0x00, 0xaf, 0x67, 0x6f, 0x32, 0xf7, 0x86, 0x22, 0xf8
  db 0x00, 0x86, 0xc3, 0xa5, 0x85, 0x22, 0xa0, 0x81, 0x22, 0x00, 0x34, 0x83
  db 0xe5, 0x11, 0x64, 0x00, 0x19, 0x7e, 0x00, 0x32, 0x3f, 0x85, 0xe5, 0xdd
  db 0xe1, 0x19, 0x22, 0x00, 0x0b, 0x80, 0xdd, 0x5e, 0x02, 0x19, 0x23, 0x22
  db 0x00, 0xe1, 0x84, 0xd1, 0xdd, 0x6e, 0x03, 0xdd, 0x66, 0x40, 0x04, 0x12
  db 0xee, 0x84, 0x21, 0xa9, 0x00, 0x83, 0x06, 0x2d, 0x83, 0x21, 0x69, 0x0e
  db 0x06, 0x99, 0x81, 0x5b, 0x00, 0xbe, 0x11, 0x5d, 0x86, 0x01, 0x60, 0x87
  db 0x1a, 0x00, 0x13, 0xfe, 0x1e, 0x30, 0x06, 0x67, 0x1a, 0x6f, 0x00, 0x13
  db 0x18, 0x07, 0xd5, 0x16, 0x00, 0x5f, 0x19, 0x02, 0x19, 0xd1, 0x7c, 0x02
  db 0x0b, 0x7d, 0x02, 0x00, 0xd6, 0xf0, 0x20, 0xe3, 0x21, 0x92, 0x86, 0x77
  db 0x20, 0x11, 0x93, 0x26, 0x6c, 0x00, 0xed, 0xb0, 0x20, 0x3c, 0x32, 0x87
  db 0x21, 0x01, 0x12, 0xf0, 0x22, 0xad, 0x7a, 0xca, 0x02, 0x48, 0xe7, 0x0b
  db 0x59, 0x05, 0xcb, 0x84, 0x24, 0x22, 0x9f, 0x05, 0xbc, 0x02, 0xd9, 0x92
  db 0x02, 0xa1, 0x02, 0xbe, 0x02, 0x00, 0xdb, 0x86, 0xdd, 0x7e, 0x9f, 0xd6
  db 0x30, 0x38, 0x00, 0x04, 0xfe, 0x0a, 0x38, 0x02, 0x3e, 0x06, 0x32, 0x04
  db 0x8b, 0x82, 0xf5, 0xfe, 0x04, 0x12, 0xff, 0x00, 0x17, 0xe6, 0x07, 0x21
  db 0x09, 0x86, 0xd5, 0x50, 0x40, 0x87, 0x60, 0x5e, 0x23, 0xcb, 0x3b, 0x9f
  db 0x00, 0xe6, 0xa7, 0x32, 0x14, 0x81, 0xeb, 0xc1, 0x09, 0x00, 0x1a, 0x01
  db 0x19, 0x86, 0x81, 0x4f, 0x88, 0x91, 0x00, 0x47, 0xc5, 0x11, 0xef, 0x87
  db 0xd5, 0x06, 0x0c, 0x00, 0xc5, 0x4e, 0x23, 0xe5, 0x46, 0xd5, 0xeb, 0x11
  db 0x00, 0x17, 0x00, 0xdd, 0x26, 0x08, 0xcb, 0x38, 0xcb, 0x80, 0x8e, 0x79
  db 0x8a, 0x77, 0x23, 0x40, 0x78, 0x03, 0x19, 0xdd, 0x25, 0x20, 0xef, 0x00
  db 0xd1, 0x13, 0x13, 0xe1, 0x23, 0xc1, 0x10, 0xda, 0x00, 0xe1, 0xd1, 0x7b
  db 0xd5, 0x11, 0x25, 0x86, 0xbb, 0x00, 0xd1, 0x20, 0x05, 0x3e, 0xfd, 0x32
  db 0x1d, 0x88, 0x00, 0x1a, 0xa7, 0x28, 0x11, 0x1f, 0xf5, 0x87, 0x4f, 0x00
  db 0x09, 0xf1, 0x30, 0x02, 0x35, 0x35, 0x34, 0xa7, 0x20, 0xed, 0x42, 0xcd
  db 0xeb, 0xf1, 0x00, 0xfe, 0x05, 0x21, 0x11, 0x00, 0x54, 0x5c, 0x3e, 0x00
  db 0x17, 0x30, 0x03, 0x2d, 0x5d, 0xaf, 0x32, 0x6c, 0x00, 0x81, 0xdd, 0x21
  db 0xff, 0x86, 0x0e, 0x10, 0xe5, 0x00, 0x19, 0xeb, 0xed, 0x62, 0x7d, 0x7d
  db 0x7c, 0xce, 0x90, 0x62, 0x77, 0x02, 0x23, 0x19, 0x0c, 0x10, 0x79, 0xe6
  db 0x0f, 0x5a, 0xe1, 0x7b, 0xfe, 0x00, 0x77, 0x20, 0x01, 0x1c, 0x79, 0xa7
  db 0x20, 0xdf, 0xc4, 0xd7, 0xdd, 0x36, 0x00, 0x08, 0x00, 0xcd, 0x25, 0x83
  db 0x0a, 0x03, 0x0f, 0x85, 0xb0, 0x16, 0x8a, 0x58, 0x00, 0xb5, 0x38, 0x56
  db 0x06, 0xdd, 0x75, 0x90, 0xd1, 0x74, 0x04, 0x18, 0x41, 0x64, 0x07, 0x00
  db 0x3e, 0x10, 0x24, 0x18, 0x3b, 0x04, 0x08, 0x02, 0xf4, 0x05, 0x18, 0x33
  db 0x3d, 0x20, 0x07, 0x2d, 0x00, 0x09, 0x05, 0x18, 0x29, 0xcd, 0x09, 0x83
  db 0x18, 0x53, 0x24, 0x3c, 0x04, 0x1c, 0x65, 0x1b, 0xc4, 0x0f, 0x13, 0x19
  db 0x18, 0xb8, 0x84, 0x25, 0x8d, 0x6f, 0x82, 0x91, 0x28, 0xfa, 0x91, 0xfb
  db 0x22, 0x04, 0x91, 0x82, 0x11, 0x10, 0x20, 0x15, 0x83, 0x00, 0x38, 0x96
  db 0x82, 0x28, 0x4f, 0x38, 0x9b, 0x83, 0x08, 0x28, 0x2b, 0x38, 0xad, 0x04
  db 0xb3, 0x38, 0x04, 0xb9, 0xc6, 0x60, 0x38, 0x26, 0x15, 0xc1, 0x12, 0x82
  db 0x38, 0x15, 0x05, 0xc0, 0x7b, 0x88, 0xe5, 0x8e, 0x15, 0x21, 0x3e, 0x83
  db 0x93, 0xc0, 0x7f, 0xd5, 0x18, 0xca, 0x32, 0xed, 0x02, 0x86, 0x18, 0xc8
  db 0xdd, 0xcb, 0x09, 0x05, 0xc6, 0x59, 0x06, 0x02, 0x08, 0xc6, 0xaf, 0xed
  db 0x73, 0x44, 0x55, 0x5b, 0xf9, 0x93, 0xe5, 0x45, 0x00, 0x31, 0x00, 0x23
  db 0x68, 0x05, 0x1b, 0x0f, 0xc9, 0x15, 0x1c, 0x96, 0x61, 0x00, 0x26, 0x0b
  db 0x0a, 0x02, 0xf9, 0x38, 0xdd, 0x80, 0x41, 0x08, 0x07, 0x87, 0x6f, 0x26
  db 0x99, 0xab, 0x23, 0x66, 0x19, 0x2f, 0x3e, 0x3e, 0x42, 0xdc, 0x0e, 0x5b
  db 0xe1, 0x21, 0xed, 0x52, 0xda, 0x0d, 0x0b, 0xda, 0x0e, 0xbb, 0x82, 0xa0
  db 0x56, 0xfb, 0x1b, 0x04, 0xfe, 0x06, 0x38, 0x09, 0x11, 0x00, 0xdd, 0x44
  db 0x73, 0x0e, 0x72, 0xfb, 0x47, 0x08, 0xa4, 0x02, 0xe3, 0x01, 0xeb, 0x80
  db 0x25, 0xf2, 0xac, 0x82, 0x2f, 0x08, 0xed, 0x63, 0x44, 0x7e, 0x0c, 0x04
  db 0x03, 0x0b, 0xf5, 0xaf, 0x04, 0xfe, 0x71, 0xf1, 0x6c, 0xd6, 0x0a, 0xe6
  db 0x6a, 0x87, 0x2c, 0x18, 0xdf, 0x1c, 0x0e, 0xf5, 0xc9, 0x05, 0xf4, 0xd1
  db 0x05, 0xff, 0x9c, 0x28, 0x07, 0x00, 0xa9, 0x31, 0x23, 0x02, 0xfa, 0x30
  db 0x02, 0xfb, 0x17, 0x32, 0x9b, 0x09, 0x85, 0x32, 0xec, 0x86, 0x07, 0x6f
  db 0x07, 0x02, 0x67, 0x22, 0x9e, 0x85, 0x28, 0x11, 0x3f, 0x05, 0x75, 0x08
  db 0x32, 0x56, 0xfc, 0x15, 0x12, 0x0a, 0x18, 0x22, 0xfd, 0x86, 0x31, 0x43
  db 0xc4, 0x25, 0x67, 0x0b, 0xea, 0x3a, 0x86, 0xc9, 0x90, 0x87, 0x92, 0xf4
  db 0xa5, 0x93, 0x00, 0x5b, 0x74, 0xfe, 0xc9, 0x24, 0x83, 0xba, 0x82, 0x00
  db 0x4d, 0x82, 0xcd, 0x82, 0xd3, 0x82, 0xd9, 0x82, 0xa1, 0x0b, 0x01, 0xf1
  db 0x82, 0x03, 0x6e, 0x05, 0x59, 0x01, 0x81, 0xf9, 0x86, 0x00, 0xa7, 0x15
  db 0x46, 0xe5, 0xca, 0x8c, 0x49, 0x84, 0xb4, 0xd7, 0x83, 0x4a, 0x8a, 0x0d
  db 0x8a, 0x09, 0x0e, 0xf9, 0xd1, 0x67, 0x98, 0x00, 0x00, 0x6f, 0x39, 0x3c
  db 0xba, 0x38, 0x01, 0x7b, 0xec, 0x90, 0x7e, 0x12, 0x12, 0x86, 0xf2, 0x8c
  db 0x2c, 0xfe, 0x88, 0x81, 0x02, 0x3e, 0x5f, 0x87, 0x08, 0x24, 0x25, 0x0f
  db 0x25, 0x10, 0x25, 0x26, 0xc0, 0x19, 0x01, 0x47, 0x87, 0x87, 0x8f, 0x29
  db 0xf9, 0x78, 0x04, 0x2b, 0x01, 0xc1, 0xe1, 0xad, 0xc0, 0x20, 0x56, 0x09
  db 0x19, 0xcb, 0x70, 0x22, 0x28, 0x06, 0x85, 0x0a, 0x0c, 0x74, 0x09, 0xeb
  db 0x08, 0xd0, 0x53, 0x31, 0xec, 0x12, 0x25, 0xe1, 0x19, 0x1c, 0x06, 0x1c
  db 0x32, 0x07, 0x19, 0x92, 0x40, 0xe3, 0xc1, 0xb6, 0x05, 0x28, 0x90, 0xef
  db 0x35, 0x05, 0x20, 0x39, 0x99, 0x46, 0x16, 0xa6, 0xc8, 0x56, 0x17, 0x56
  db 0x18, 0x7c, 0xc1, 0xbb, 0x06, 0x30, 0x35, 0x07, 0x96, 0x12, 0x56, 0x20
  db 0x1f, 0x2f, 0x19, 0x2f, 0x6c, 0x1a, 0xe6, 0x89, 0xfa, 0x1e, 0x84, 0x28
  db 0x13, 0x30, 0x28, 0x12, 0xfb, 0xcb, 0xce, 0xa8, 0xb9, 0x0f, 0x02, 0xcb
  db 0x00, 0x79, 0x28, 0x13, 0xcb, 0x71, 0x28, 0x07, 0xfe, 0x02, 0x0f, 0x28
  db 0x0b, 0x3c, 0x18, 0x05, 0xf8, 0x44, 0x28, 0x04, 0x3d, 0x19, 0x02, 0x00
  db 0x6f, 0x78, 0xe6, 0x0f, 0x85, 0xf2, 0x41, 0x84, 0x86, 0xb4, 0x10, 0x09
  db 0xb4, 0x0f, 0x6b, 0x1c, 0x85, 0x84, 0xf1, 0x28, 0x86, 0xe7, 0xcb, 0x41
  db 0x26, 0x20, 0x03, 0x0e, 0x14, 0x04, 0xf9, 0xcb, 0x78, 0x3b, 0x19, 0x0e
  db 0x17, 0x17, 0xcb, 0x2f, 0x01, 0x03, 0xdd, 0x86, 0x04, 0xcb, 0x68, 0x28
  db 0x20, 0x8c, 0x04, 0x21, 0x7f, 0x85, 0x05, 0x86, 0x77, 0x18, 0x0e, 0x1f
  db 0x11, 0x26, 0xf8, 0xee, 0x86, 0xc0, 0x14, 0x03, 0x78, 0x1f, 0xe6, 0x48
  db 0x00, 0x21, 0xf6, 0x86, 0xb6, 0x0f, 0xf6, 0x80, 0xe6, 0x18, 0xbf, 0x77
  db 0xe1, 0xbc, 0x10, 0x0a, 0xc8, 0xbb, 0x0a, 0x09, 0xc0, 0xdd, 0xae, 0x15
  db 0x1d, 0x15, 0x04, 0x2b, 0x7e, 0x0b, 0x38, 0x03, 0x04, 0x0c, 0xc9, 0xee
  db 0xc9, 0x32, 0xd4, 0x40, 0x32, 0x2a, 0x64, 0x3d, 0xae, 0x21, 0x01, 0xe9
  db 0x86, 0x35, 0x20, 0x7f, 0x21, 0xad, 0x84, 0x05, 0x4c, 0x01, 0x00, 0x0a
  db 0x00, 0xa7, 0x20, 0x3a, 0x57, 0x32, 0xed, 0x86, 0x2a, 0x00, 0x0b, 0x80
  db 0x23, 0x7e, 0x3c, 0x20, 0x08, 0xcd, 0x32, 0x0d, 0x80, 0xac, 0x92, 0x09
  db 0x22, 0x0f, 0x3d, 0xc3, 0x8e, 0xcb, 0x12, 0xb9, 0x82, 0xed, 0x5b, 0xa0
  db 0x81, 0x8a, 0x8c, 0x09, 0x85, 0xac, 0x48, 0x44, 0x4d, 0x03, 0x22, 0x21
  db 0x61, 0x85, 0x04, 0x35, 0x85, 0x88, 0xb1, 0xdd, 0x21, 0x9e, 0x10, 0x86
  db 0xcd, 0xdd, 0x1c, 0x43, 0xcb, 0x84, 0x31, 0x21, 0xca, 0x51, 0x0e, 0x19
  db 0x10, 0xbb, 0x86, 0x55, 0xa4, 0x13, 0x26, 0x21, 0x6c, 0xe7, 0x13, 0xd8
  db 0xd4, 0x13, 0x84, 0x35, 0x3e, 0x3e, 0x32, 0x81, 0x49, 0x12, 0x92, 0x71
  db 0xef, 0x09, 0x3a, 0x5e, 0x83, 0x22, 0x05, 0x3a, 0x06, 0xf6, 0x32, 0xf7
  db 0x26, 0x12, 0xaf, 0x12, 0xf1, 0x97, 0x12, 0x05, 0x31, 0x12, 0xf8, 0x12
  db 0xcc, 0x34, 0x12, 0xf3, 0x12, 0x50, 0xf3, 0x08, 0xa3, 0x7c, 0x82, 0xc3
  db 0xf5, 0x86, 0x3f, 0xa1, 0xee, 0x9f, 0x57, 0x2a, 0x78, 0xee, 0x96, 0x5c
  db 0xe7, 0x87, 0x2c, 0xfa, 0xf7, 0x21, 0xec, 0x92, 0x84, 0x28, 0x0e, 0x69
  db 0x4e, 0x0a, 0x1b, 0x77, 0xaf, 0x2e, 0xff, 0x13, 0x10, 0x0e, 0xa0, 0x21
  db 0x59, 0xed, 0x79, 0x0c, 0x00, 0xed, 0xa3, 0x0d, 0x3c, 0xfe, 0x08, 0x20
  db 0xf5, 0x09, 0xd3, 0xa0, 0x3a, 0x0a, 0xa6, 0x00, 0x02, 0x28, 0x05, 0xaf
  db 0xd3, 0xa1, 0x18, 0x03, 0x43, 0x7e, 0x04, 0x3e, 0x09, 0x23, 0x66, 0x13
  db 0x04, 0xe3, 0x13, 0x0a, 0xc6, 0x13, 0x08, 0xe3, 0x13, 0x0b, 0x39, 0x13
  db 0x07, 0x0c, 0xe3, 0x07, 0x0d, 0x88, 0x07, 0xa7, 0xf8, 0x09, 0xc9, 0x64
  db 0x04, 0x2a, 0x65, 0x00, 0x01, 0x0c, 0x01, 0x94, 0x00, 0x35, 0x30, 0x0e
  db 0x60, 0x20, 0x60, 0x21, 0x01, 0x00, 0x05, 0x09, 0x0b, 0x0d, 0x0f, 0x13
  db 0x15, 0x19, 0x0b, 0x25, 0x3d, 0x00, 0x5d, 0xde, 0x00, 0x37, 0x4d, 0x53
  db 0x5f, 0x71, 0x82, 0x8c, 0x9c, 0x00, 0x9e, 0xa0, 0xa6, 0xa8, 0xaa, 0xac
  db 0xae, 0xae, 0x00, 0x00, 0x57, 0x1f, 0x23, 0x25, 0x29, 0x2d, 0x2f, 0x00
  db 0x33, 0xbf, 0x00, 0x1d, 0x21, 0x23, 0x27, 0x2b, 0x08, 0x2d, 0x31, 0x55
  db 0xbd, 0x0a, 0x1b, 0x21, 0x81, 0x13, 0x2b, 0x3b, 0x4d, 0x5f, 0xbb, 0x40
  db 0x0b, 0x4c, 0x00, 0x90, 0x0d, 0xd8, 0x69, 0x00, 0x70, 0x76, 0x7d, 0x85
  db 0x8d, 0x95, 0x9d, 0xa8, 0x00, 0xb1, 0xbb, 0x0c, 0xda, 0x62, 0x68, 0x6d
  db 0x75, 0x00, 0x7b, 0x83, 0x8a, 0x92, 0x9c, 0xa4, 0xaf, 0xb8, 0x00, 0x0e
  db 0x08, 0x6a, 0x72, 0x78, 0x7e, 0x86, 0x90, 0x00, 0x96, 0xa0, 0xaa, 0xb4
  db 0xbe, 0x0f, 0xc0, 0x78, 0x00, 0x88, 0x80, 0x90, 0x98, 0xa0, 0xb0, 0xa8
  db 0xe0, 0x1a, 0xb0, 0xe8, 0x00, 0xaf, 0xbd, 0x00, 0xff, 0xff, 0xff, 0xff
  db 0x80

font.resource.data.1:
  db 0x3e,0x00,0x32,0x00,0x20,0x00,0x05,0x96,0x02,0x50,0x00,0x92,0x0f,0x06,0xf8,0x01
  db 0xa0,0x0b,0x11,0x78,0xa0,0x70,0x28,0x40,0xf0,0x17,0xc0,0xc8,0x10,0x20,0x40,0x00
  db 0x98,0x18,0x00,0x40,0xa0,0x40,0xa8,0x90,0x19,0x98,0x60,0x00,0x0d,0xdd,0x27,0x07
  db 0x14,0x00,0x20,0x10,0x17,0x03,0x10,0xe0,0x13,0x20,0xa8,0x70,0x20,0x1c,0x70,0xa8
  db 0x20,0x50,0xf8,0xef,0x51,0x38,0x5c,0x2c,0x78,0x00,0xf9,0x00,0x60,0x4c,0x3f,0x00
  db 0x08,0x2e,0x00,0x80,0x00,0x3c,0x66,0x66,0x00,0xcc,0xcc,0x80,0x1a,0x04,0x06,0x06
  db 0x00,0x0c,0x0c,0x50,0x08,0x0f,0x07,0x38,0xc0,0xc0,0xba,0x0f,0x07,0x0f,0x95,0x07
  db 0x24,0x1f,0x07,0xd6,0x17,0x33,0x9d,0x0f,0x07,0xcf,0x2f,0x3c,0xad,0x2f,0x1f,0xb4
  db 0x0f,0x27,0xee,0x62,0x7b,0xd3,0x74,0x40,0x77,0x18,0x30,0x60,0xc0,0x18,0x60,0x30
  db 0x18,0x0f,0xf8,0xcb,0x01,0x00,0xb0,0x0c,0x12,0x00,0x70,0x65,0x88,0x7f,0x23,0xc4
  db 0x07,0x48,0xa8,0xb3,0x44,0x18,0x3e,0x71,0x00,0x7f,0x45,0x71,0x07,0x7e,0x71,0x01
  db 0x70,0x02,0x0f,0x70,0x70,0x71,0x75,0x3e,0x0f,0x00,0xc2,0x0f,0x7f,0x70,0x7e,0x10
  db 0x75,0x7f,0x07,0x05,0x68,0x02,0x1f,0x77,0x67,0x73,0x1f,0x53,0x35,0x00,0x07,0x1c
  db 0x49,0x00,0x07,0x0e,0x86,0x00,0x4e,0x3c,0x05,0x17,0x72,0x7c,0x74,0x72,0x17,0x75
  db 0x60,0x00,0x37,0x0e,0x0f,0x7b,0x7f,0x75,0x27,0x13,0x71,0x79,0x7d,0x37,0x71,0x6f
  db 0x6f,0x1b,0x5f,0x7e,0x06,0x4f,0x75,0x75,0x72,0x3d,0xc7,0x0f,0x73,0x73,0x0a,0x0f
  db 0x3c,0x0e,0x43,0x1f,0x6f,0xf1,0x57,0x71,0xe6,0x2f,0x0e,0x05,0x73,0x76,0x7c,0x07
  db 0x1c,0x75,0x7f,0x7b,0x4f,0x3a,0x9e,0x1d,0x3a,0x70,0x07,0x27,0x7f,0x0e,0x1c,0x53
  db 0x38,0x6f,0x76,0x40,0xa9,0x00,0x57,0x00,0x80,0x92,0x9a,0x08,0x0f,0x74,0x10,0x00
  db 0x0f,0x20,0x3e,0x50,0x88,0x86,0xb9,0x93,0xef,0x1d,0x24,0x0b,0x89,0xf8,0x88,0xe0
  db 0x1b,0xf0,0x48,0x70,0x48,0x74,0xf0,0xae,0x80,0x9a,0x00,0x78,0x3a,0x0f,0x48,0x0f
  db 0x2b,0x0f,0x80,0xf0,0x80,0xf8,0x1a,0x07,0x80,0x1c,0x1f,0xb8,0x88,0x5f,0x88,0xf2
  db 0x37,0xee,0xae,0x00,0x1f,0x1b,0x38,0x08,0x08,0x0e,0x17,0x90,0xe0,0x90,0x17,0xce
  db 0x46,0x37,0x8e,0x0f,0xd8,0xa8,0x27,0x86,0x07,0xc8,0xa8,0x98,0x95,0x2f,0x0d,0x39
  db 0xcd,0x5f,0x88,0xab,0x4f,0x0f,0xd8,0x74,0xe8,0x07,0x15,0xa0,0x68,0x90,0x5f,0x70
  db 0x3c,0x08,0xf8,0x57,0xe3,0xa3,0x3c,0x2e,0x6f,0x1d,0x90,0xa0,0x40,0x07,0x34,0x2f
  db 0xd8,0x4f,0x47,0x50,0xd3,0x9d,0x07,0x27,0x67,0x2f,0xd7,0x32,0x37,0x18,0xfb,0xab
  db 0x02,0xf7,0xb9,0x13,0x03,0xc0,0x2a,0x03,0x10,0x02,0xf7,0x1a,0x40,0xa8,0x10,0x1f
  db 0xed,0xff,0xff,0xff,0xf8
font.resource.data.2:
  db 0x3e, 0x00, 0x00, 0x00, 0x06, 0x0e, 0x0c, 0x18, 0x00, 0x60, 0x60, 0x2d, 0x00, 0x6c, 0x00, 0x26
  db 0x0f, 0x06, 0xfe, 0x40, 0x01, 0x0b, 0x10, 0x7c, 0xd0, 0x7c, 0x16, 0x20, 0x7c, 0x10, 0x10, 0x62
  db 0x64, 0x08, 0x10, 0x00, 0x26, 0x46, 0x00, 0x70, 0x88, 0x50, 0x20, 0x54, 0x23, 0x88, 0x76, 0x0f
  db 0x18, 0x30, 0x4b, 0x27, 0x06, 0xab, 0x00, 0x3a, 0x02, 0xb0, 0x00, 0x13, 0x24, 0x18, 0x1d, 0x7e
  db 0x18, 0x24, 0x18, 0x2d, 0x07, 0x7e, 0x03, 0x96, 0x23, 0x18, 0x3a, 0x5e, 0x06, 0x0f, 0x74, 0x6b
  db 0x16, 0x6f, 0x1c, 0x01, 0x38, 0x70, 0xe0, 0xc0, 0x00, 0x7c, 0xc6, 0xc9, 0x00, 0x7c, 0x13, 0x38
  db 0xd4, 0x3f, 0x21, 0x0f, 0x0e, 0x0a, 0x3c, 0x78, 0xe0, 0xfe, 0x2c, 0x8e, 0x0c, 0x3c, 0x06, 0x17
  db 0x00, 0x1c, 0x2c, 0x4c, 0x8c, 0xfe, 0x0c, 0x0c, 0x00, 0x0e, 0xfe, 0xc0, 0xfc, 0x06, 0x0f, 0x2e
  db 0x3c, 0x60, 0x08, 0x2f, 0x1c, 0xfe, 0xc6, 0x0c, 0x7a, 0x00, 0x01, 0x78, 0xc4, 0xe4, 0x7c, 0x9e
  db 0x86, 0x7c, 0xc1, 0x47, 0x7e, 0x06, 0x0c, 0x78, 0xdd, 0x5c, 0x5f, 0xe5, 0x07, 0x77, 0x88, 0x25
  db 0x60, 0x9b, 0xbb, 0x3f, 0x7d, 0x8c, 0x80, 0x0b, 0xa8, 0x26, 0x13, 0x47, 0x66, 0x66, 0xac, 0x06
  db 0x21, 0x37, 0x48, 0xde, 0x00, 0xc0, 0x3f, 0x38, 0x6c, 0x93, 0x40, 0xfe, 0x02, 0x00, 0x3b, 0x5c
  db 0x02, 0x10, 0x1f, 0xc0, 0x00, 0x66, 0x3c, 0x00, 0x30, 0xf8, 0xcc, 0x9e, 0x32, 0xcc, 0xf8, 0x7f
  db 0x77, 0x9e, 0x02, 0xfe, 0x42, 0x07, 0xb7, 0x3e, 0x87, 0x00, 0xce, 0xc6, 0x66, 0x3e, 0x00, 0xc6
  db 0xea, 0x36, 0x37, 0x2f, 0xd0, 0xff, 0x91, 0x2f, 0x0e, 0xa5, 0xa1, 0xa7, 0x00, 0x33, 0xd8, 0xf0
  db 0xf8, 0xdc, 0xce, 0x00, 0xc0, 0xea, 0x00, 0x37, 0x0f, 0x0a, 0xee, 0xfe, 0xfe, 0xd6, 0x27, 0x07
  db 0x0b, 0xe6, 0xf6, 0xfe, 0xde, 0x39, 0xe3, 0xf7, 0xac, 0x6f, 0x6d, 0x4f, 0xe1, 0x0f, 0xde, 0xcc
  db 0x7a, 0xa7, 0x0f, 0x51, 0x37, 0x0f, 0x94, 0xd8, 0x47, 0xc3, 0xf7, 0x00, 0x77, 0x67, 0x2f, 0x06
  db 0x07, 0x44, 0x6c, 0x38, 0x10, 0x25, 0x07, 0xd6, 0x50, 0xee, 0x4f, 0x0c, 0x57, 0x7c, 0x38, 0x7c
  db 0x07, 0x87, 0xce, 0x24, 0x11, 0x82, 0x9f, 0xa3, 0xd7, 0x34, 0x6f, 0x3c, 0x97, 0x90, 0x3c, 0x7f
  db 0xe0, 0x70, 0x38, 0x44, 0x1c, 0x94, 0x0f, 0x0c, 0xe9, 0x00, 0x0f, 0x18, 0x0f, 0xd8, 0x00, 0xbc
  db 0x00, 0x90, 0x74, 0x0a, 0xff, 0xff, 0xff, 0xff, 0xc0
font.resource.data.3:
  db 0x3f, 0x00, 0x01, 0x00, 0x0c, 0x18, 0x30, 0x20, 0x00, 0x40, 0x0d, 0x07, 0x66, 0x66, 0x22, 0x06
  db 0x0f, 0x24, 0x7e, 0x24, 0x00, 0x02, 0x00, 0x10, 0x7c, 0xd0, 0x7c, 0x16, 0x7c, 0x40, 0x10, 0x0f
  db 0x62, 0x64, 0x08, 0x10, 0x26, 0x00, 0x46, 0x00, 0x70, 0x88, 0x50, 0x20, 0x54, 0x88, 0x56, 0x76
  db 0x0f, 0x2e, 0x99, 0x26, 0x35, 0x09, 0x00, 0x30, 0x18, 0x00, 0x02, 0x08, 0x70, 0x00, 0x13, 0x24
  db 0x18, 0x7e, 0x29, 0x18, 0x24, 0x18, 0x38, 0x10, 0x5b, 0x35, 0x36, 0x5c, 0x23, 0x1b, 0x09, 0x3c
  db 0xfa, 0x6a, 0x0f, 0x98, 0x03, 0x06, 0x6f, 0x60, 0x00, 0xc0, 0x80, 0x00, 0xfe, 0x86, 0x8a, 0x92
  db 0xa2, 0x23, 0xc2, 0xfe, 0x2e, 0x30, 0x10, 0xa0, 0x00, 0x0f, 0x82, 0x02, 0xfe, 0x80, 0x38, 0x80
  db 0xfe, 0x07, 0x3e, 0x02, 0x52, 0x82, 0x07, 0x0b, 0x88, 0x00, 0x5d, 0xfe, 0x5a, 0x14, 0xdc, 0x0f
  db 0x07, 0x82, 0xe1, 0x07, 0x02, 0x02, 0x04, 0x47, 0x55, 0xb1, 0x77, 0x0c, 0x0f, 0x2e, 0x04, 0x02
  db 0x3a, 0x5c, 0xef, 0x5f, 0x2e, 0x07, 0x21, 0x66, 0x86, 0x9b, 0x0c, 0x34, 0x10, 0x7e, 0x02, 0xce
  db 0x0b, 0x7a, 0xe5, 0x2f, 0x0c, 0x6d, 0x08, 0x01, 0x7c, 0xc4, 0x9c, 0x00, 0xc0, 0x7c, 0x00, 0x00
  db 0x02, 0x06, 0x0a, 0x12, 0x3e, 0x42, 0x82, 0x06, 0x00, 0xf8, 0x84, 0x82, 0xfc, 0x76, 0x02, 0x67
  db 0x44, 0x00, 0x4f, 0xf0, 0x88, 0x0d, 0x84, 0x39, 0x88, 0xf0, 0x0f, 0xfc, 0xdb, 0x0f, 0x39, 0x07
  db 0x80, 0x07, 0x9e, 0xd7, 0x77, 0x03, 0x51, 0x7f, 0x37, 0xbd, 0xe0, 0xbf, 0x08, 0xe5, 0x00, 0xf8
  db 0x17, 0x32, 0x34, 0x90, 0x3a, 0xbf, 0x3c, 0x47, 0x0e, 0x82, 0xc6, 0xaa, 0x92, 0x27, 0x02, 0x82
  db 0xc2, 0xa2, 0x92, 0x8a, 0x86, 0x07, 0xef, 0x34, 0x07, 0xb7, 0x9e, 0x74, 0x4f, 0x0f, 0x18, 0xfa
  db 0xd3, 0x0f, 0xb3, 0x37, 0x5f, 0x8b, 0xcf, 0x07, 0xc7, 0x57, 0x82, 0x9c, 0x2f, 0x57, 0xa0, 0xc4
  db 0xb7, 0x89, 0x99, 0x04, 0xaa, 0xaa, 0xcc, 0xcc, 0x88, 0x0f, 0x44, 0x07, 0x28, 0x10, 0x28, 0x44
  db 0x82, 0x6d, 0x07, 0x7f, 0xca, 0x95, 0x40, 0x27, 0x68, 0x1c, 0x36, 0x1c, 0x1c, 0x00, 0xc0, 0x60
  db 0xf1, 0x0d, 0x02, 0x00, 0x38, 0x04, 0x97, 0x38, 0x93, 0x9a, 0x3c, 0x66, 0x97, 0xf2, 0x8e, 0x90
  db 0x9f, 0x09, 0xff, 0xff, 0xff, 0xf8

;----------------------------------------------------------------------------------
; BASIC KUN (version 2.1) SUPPORT ROUTINES
;----------------------------------------------------------------------------------

VDP.DR	EQU	0006H
VDP.DW	EQU	0007H
IDBYT2	EQU	002DH
CHGMOD	EQU	005FH
CHGCLR	EQU	0062H
WRTPSG	EQU	0093H
;QINLIN	EQU	00B4H
CLS	EQU	00C3H
ERAFNK	EQU	00CCH
DSPFNK	EQU	00CFH
GTSTCK	EQU	00D5H
GTPAD	EQU	00DBH
GTPDL	EQU	00DEH
READC	EQU	011DH

S.INIPLT	EQU	0141H
S.RSTPLT	EQU	0145H
S.GETPLT	EQU	0149H
S.SETPLT	EQU	014DH

M406F	EQU	406FH			; BASIC error
M57F1	EQU	57F1H			; PSET
M58C1	EQU	58C1H			; LINE boxfill
M58FC	EQU	58FCH			; LINE
M5912	EQU	5912H			; LINE box
M59E3	EQU	59E3H			; PAINT MSX1
M5B19	EQU	5B19H			; CIRCLE
M5EA4	EQU	5EA4H			; locate variable
M5F5D	EQU	5F5DH			; locate variable (search only)
M7810	EQU	7810H			; check ON <trap> GOTO

S266E	EQU	266EH			; PAINT MSX2 (subrom)


RG2SAV	EQU	0F3E1H
RG6SAV	EQU	0F3E5H
CURLIN	EQU	0F41CH
DATLIN	EQU	0F6A3H
SAVTXT	EQU	0F6AFH
DATPTR	EQU	0F6C8H
DEFTBL	EQU	0F6CAH
FBUFFR	EQU	0F7C5H
NULBUF	EQU	0F862H
DPPAGE	EQU	0FAF5H
ACPAGE	EQU	0FAF6H
MODE	EQU	0FAFCH
FNKFLG	EQU	0FBCEH
ONGSBF	EQU	0FBD8H
LINWRK	EQU	0FC18H
TRPTBL	EQU	0FC4CH
INTFLG	EQU	0FC9BH
BRDATR	EQU	0FCB2H
GXPOS	EQU	0FCB3H
GYPOS	EQU	0FCB5H
GRPACX	EQU	0FCB7H
GRPACY	EQU	0FCB9H
PROCNM	EQU	0FD89H
RG11SAV	EQU	0FFEAH
RG23SAV	EQU	0FFF6H
RG25SAV	EQU	0FFFAH
RG26SAV	EQU	0FFFBH

J4DFA:
J4E07:	LD	E,4
	DEFB	001H
J4E0A:	LD	E,12
	DEFB	001H
J4E0D:	LD	E,5
	DEFB	001H
J4E10:	LD	E,13
	DEFB	001H
J4E13:	LD	E,16
	DEFB	001H
J4E16:	LD	E,2
	DEFB	001H
J4E19:	LD	E,8
	DEFB	001H
J4E1C:	LD	E,1
	DEFB	001H
J4E1F:	LD	E,7
	DEFB	001H
J4E22:	LD	E,10
	DEFB	001H
J4E25:	LD	E,9
	DEFB	001H
I4E28:	LD	E,3
	LD	HL,MODE
	RES	4,(HL)			; 212 lines mode
	LD	IX,M406F
	JP	CALBAS			; BASIC error

PAINT_FIX:
    push af
      ld a, (VERSION)
      cp 3                ; MSX Turbo R?
      jr c, PAINT_FIX.1
         inc ix
         inc ix
         inc ix
         inc ix
         inc ix
PAINT_FIX.1:
    pop af
	JP	SUB_EXTROM			; PAINT handler (subrom)

verify.slots.test.megaram.fix.1:
    ; change page 2 to RAM slot....
    ld a, (RAMAD2)
    ld h, 0x80
    call SUB_ENASLT

    ld a, (Seg_P8000_SW)     ; save original byte there
    ld (TEMP), a

    ; change page 2 slot to same slot of page 1
    ld a, (SLTAD1)
    ret

verify.slots.test.megaram.fix.2:
    call SUB_ENASLT

    ld a, (TEMP)
    ld (Seg_P8000_SW), a        ; restore original byte in RAM
    ret

SUB_EXTROM:
  call EXTROM
  ei
  ret                  ; 5 bytes

SUB_CALBAS:
  call CALBAS
  ei
  ret

BASIC_KUN_START_FILLER:
    DEFS	06BF7H-34-$,000H

J_SGN_INT:
    ld a, l
	or h
	ret z
	  bit 7,h
	  ld hl,0ffffh
	  ret nz
	    inc hl
	    inc hl
        ret

J_SGN_FLOAT:
    ld a, b
    and a
    ret z
      ld b, 0x81
      ld l, 0
      ld a, h
      and 0x80
      ld h, a
      ret

J_ABS_INT:
    ld a, h
	add a, a
	ret nc
	  xor a
	  sub l
	  ld l, a
	  sbc a, a
	  sub h
	  ld h, a
	  ret

;	  Subroutine swap integer
;	     Inputs  ________________________
;	     Outputs ________________________

I6BF7:	LD	B,2			; integer is 2 bytes
	JR	J6C01

;	  Subroutine swap string
;	     Inputs  ________________________
;	     Outputs ________________________

I6BFB:	LD	B,0			; string is 256 bytes
	JR	J6C01

;	  Subroutine swap float
;	     Inputs  ________________________
;	     Outputs ________________________

I6BFF:	LD	B,3			; float is 3 bytes

J6C01:	LD	C,(HL)
	LD	A,(DE)
	LD	(HL),A
	LD	A,C
	LD	(DE),A
	INC	DE
	INC	HL
	DJNZ	J6C01
	RET

;	  Subroutine put in DAC, execute routine DE, get result from DAC
;	     Inputs  ________________________
;	     Outputs ________________________

C6C0B:	LD	(DAC+2),HL
	LD	A,2
	LD	(VALTYP),A
	LD	HL,DAC
	CALL	C6C25			; CALL DE
	LD	A,(VALTYP)
	CP	2
	JP	NZ, J4E10		; BASIC error: type mismatch
	LD	HL,(DAC+2)
	RET

;	  Subroutine CALL DE
;	     Inputs  ________________________
;	     Outputs ________________________

C6C25:	PUSH	DE
	RET

;	  Subroutine check for traps handler
;	     Inputs  ________________________
;	     Outputs ________________________

C6C27:	LD	HL,TRPTBL+10*3		; STOP trap
	BIT	0,(HL)			; trap enabled ?
	JR	Z,J6C4A			; nope, skip STOP trap
	EX	DE,HL
	LD	HL,INTFLG
	DI
	LD	A,(HL)			; get STOP or CTRL-STOP status
	EI
	LD	(HL),0			; reset STOP status
	CP	3			; CTRL-STOP pressed ?
	JR	NZ,J6C4A		; nope, skip STOP trap
	EX	DE,HL
	LD	A,(HL)
	SET	2,(HL)			; trap raised
	CP	(HL)			; trap already raised ?
	JR	Z,J6C4A			; yep, skip STOP trap
	AND	02H			; trap paused ?
	JR	NZ,J6C4A		; yep, skip STOP trap
	LD	HL,ONGSBF
	INC	(HL)			; increase trap counter
J6C4A:	LD	A,(ONGSBF)
	AND	A			; trap occured ?
	RET	Z			; nope, quit
	LD	HL,TRPTBL		; trap table
	LD	B,26
J6C54:	LD	A,(HL)
	CP	05H			; trap enabled AND trap not paused AND trap occured ?
	INC	HL
	JR	Z,J6C5F			; yep, invoke trap handler
	INC	HL
J6C5B:	INC	HL
	DJNZ	J6C54			; next trap
	RET

J6C5F:	LD	E,(HL)
	INC	HL
	LD	D,(HL)			; trap handler
	LD	A,E
	OR	D			; trap handler defined ?
	JR	Z,J6C5B			; nope, next trap
	DEC	HL
	DEC	HL
	CALL	C6CB1			; decrease trap counter
	CALL	C6CA7			; pause trap
	PUSH	HL
	;CALL	C6C25			; call trap handler
	CALL MR_CALL_TRAP
	POP	HL
	BIT	0,(HL)			; trap enabled ?
	RET	Z			; nope, quit
	DI
	LD	A,(HL)
	AND	05H			; ignore trap paused
	CP	(HL)			; trap enabled AND trap occured AND trap not paused ?
	LD	(HL),A			; resume trap
	JR	Z,J6CBE			; yep, quit
	CP	05H			; trap enabled AND trap occured ?
	JR	NZ,J6CBE		; yep, quit
J6C82:	LD	A,(ONGSBF)
	INC	A
	LD	(ONGSBF),A		; increase trap counter
	EI
	RET

;	  Subroutine enable trap
;	     Inputs  ________________________
;	     Outputs ________________________

I6C8B:	CALL	C6CC0			; disable normal function key behaviour if function key trap
	DI
	LD	A,(HL)
	AND	04H			; trap raised status
	OR	01H
	CP	(HL)			; trap already enabled AND trap not paused ?
	LD	(HL),A			; enable trap, resume trap
	JR	Z,J6CBE			; yep, quit
	AND	04H			; trap raised ?
	JR	NZ,J6C82		; yep, increase trap counter and quit
	EI
	RET

;	  Subroutine disable trap
;	     Inputs  ________________________
;	     Outputs ________________________

I6C9E:	CALL	C6CC8			; enable normal function key behaviour if function key trap
	DI
	LD	A,(HL)			; current trap status
	LD	(HL),0			; trap disabled, trap not paused, trap not occured
	JR	J6CAB			; if trap occured, reset trap occured and decrease trap counter

;	  Subroutine pause trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CA7:	DI
	LD	A,(HL)			; current trap status
	SET	1,(HL)			; pause trap

; if trap occured, reset trap occured and decrease trap counter

J6CAB:	CP	05H			; was trap enabled AND trap not paused AND trap occured ?
	JR	Z,J6CB4			; yep, reset trap occured and decrease trap counter
	EI
	RET

;	  Subroutine decrease trap counter
;	     Inputs  ________________________
;	     Outputs ________________________

C6CB1:	DI
	RES	2,(HL)			; trap not occured
J6CB4:	LD	A,(ONGSBF)
	SUB	1			; decrease trap counter
	JR	C,J6CBE			; already zero, quit
	LD	(ONGSBF),A
J6CBE:	EI
	RET

;	  Subroutine disable normal function key behaviour if function key trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CC0:	CALL	C6CCF			; get FNKFLG entry if trap is a function key trap
	RET	NC			; not a function key trap, quit
	LD	A,1
	LD	(DE),A			; function key trap enabled, disable normal function key behavior
	RET

;	  Subroutine enable normal function key behaviour if function key trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CC8:	CALL	C6CCF			; get FNKFLG entry if trap is a function key trap
	RET	NC			; not a function key trap, quit
	XOR	A
	LD	(DE),A			; function key trap disabled, enable normal function key behavior
	RET

;	  Subroutine get FNKFLG entry if trap is a function key trap
;	     Inputs  ________________________
;	     Outputs ________________________

C6CCF:	PUSH	HL
	LD	DE,TRPTBL+10*3
	AND	A
	SBC	HL,DE			; function key trap ?
	LD	C,L
	POP	HL
	RET	NC			; nope, quit
	LD	DE,FNKFLG+9+1
J6CDC:	DEC	DE
	INC	C
	INC	C
	INC	C
	JR	NZ,J6CDC
	RET

;	  Subroutine clear screen
;	     Inputs  ________________________
;	     Outputs ________________________

I6CE3:	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J746E			; yep, use CLS BIOS handler
	CALL	C73D3			; ?? screen mode 10 action
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	XOR	A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	LD	A,(ACPAGE)
	OUT	(C),A
	XOR	A
	OUT	(C),A
	LD	A,(SCRMOD)
	RRCA
	AND	01H
	INC	A
	OUT	(C),A
	LD	A,(MODE)
	AND	10H			; 212 lines mode ?
	JR	Z,J6D18			; yep,
	XOR	A
	OUT	(C),A
	INC	A
	JR	J6D1D

J6D18:	LD	A,0D4H
	OUT	(C),A
	XOR	A
J6D1D:	OUT	(C),A
	LD	A,(BAKCLR)
	CALL	C6D2F			; convert background color to background color byte
	OUT	(C),H
	XOR	A
	OUT	(C),A
	LD	A,0C0H
	OUT	(C),A
	RET

;	  Subroutine convert background color to background color byte
;	     Inputs  ________________________
;	     Outputs ________________________

C6D2F:	LD	H,A
	LD	A,(SCRMOD)
	CP	8			; screen mode 8 ?
	RET	Z			; yep, quit
	CP	7			; screen mode 7 ?
	JR	Z,J6D43			; yep, b7-b4 = b3-b0
	CP	5			; screen mode 5 ?
	JR	Z,J6D43			; yep, b7-b4 = b3-b0
					; b7-b6 = b5-b4 = b3-b2 = b1-b0
	LD	A,H
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
J6D43:	LD	A,H
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
	RET

;	  Subroutine draw box
;	     Inputs  ________________________
;	     Outputs ________________________

I6D4B:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J747B			; yep, use MSX1 draw box handler
	POP	AF
	PUSH	HL
	PUSH	IX
	POP	HL
	AND	A
	PUSH	HL
	SBC	HL,DE
	POP	HL
	JR	NC,J6D62
	EX	DE,HL
	AND	A
J6D62:	EX	(SP),HL
	PUSH	DE
	PUSH	IY
	POP	DE
	PUSH	HL
	SBC	HL,DE
	POP	HL
	JR	C,J6D6E
	EX	DE,HL
J6D6E:	PUSH	DE
	POP	IY
	POP	DE
	POP	IX
	PUSH	IY
	PUSH	HL
	POP	IY
	DEC	IX
	PUSH	AF
	PUSH	BC
	CALL	C6DA9			; draw line
	POP	BC
	POP	AF
	INC	IX
	POP	HL
	DEC	HL
	PUSH	IY
	PUSH	DE
	PUSH	IX
	POP	DE
	PUSH	AF
	PUSH	BC
	CALL	C6DA9			; draw line
	POP	BC
	POP	AF
	INC	HL
	POP	IX
	INC	IX
	PUSH	HL
	POP	IY
	PUSH	AF
	PUSH	BC
	CALL	C6DA9			; draw line
	POP	BC
	POP	AF
	DEC	IX
	POP	HL
	INC	HL
	PUSH	IX
	POP	DE
					; draw line and quit

;	  Subroutine draw line
;	     Inputs  ________________________
;	     Outputs ________________________

C6DA9:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J7476			; yep,use MSX1 draw line handler
	POP	AF
	CALL	C73D3			; ?? screen mode 10 action
	PUSH	AF
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled (DE,HL)
	CALL	C742D			; force X and Y coordinate within screen limits, when limits enabled (IX,IY)
	POP	AF
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	PUSH	AF
	PUSH	AF
	PUSH	HL
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	PUSH	IX
	POP	HL
	OUT	(C),L
	OUT	(C),H
	PUSH	IY
	POP	HL
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A
	LD	A,0CH	; 12
	PUSH	IX
	POP	HL
	SBC	HL,DE
	JR	NC,J6DEF
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	AND	0FBH
J6DEF:	PUSH	HL
	POP	IX
	POP	DE
	PUSH	IY
	POP	HL
	SBC	HL,DE
	JR	NC,J6E02
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	AND	0F7H
J6E02:	PUSH	IX
	POP	DE
	PUSH	HL
	SBC	HL,DE
	POP	HL
	JR	C,J6E0E
	EX	DE,HL
	OR	01H
J6E0E:	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	OUT	(C),H
	LD	E,A
	POP	AF
	OUT	(C),A
	OUT	(C),E
	LD	A,B
	OR	70H	; "p"
	OUT	(C),A
	POP	AF
	POP	IY
	POP	IX
	POP	HL
	POP	DE
	RET

;	  Subroutine draw filled box
;	     Inputs  ________________________
;	     Outputs ________________________

I6E29:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J7480			; yep, use MSX1 draw filled box handler
	POP	AF
	CALL	C73D3			; ?? screen mode 10 action

;	  Subroutine filled box
;	     Inputs  ________________________
;	     Outputs ________________________

C6E36:	PUSH	AF
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled (DE,HL)
	CALL	C742D			; force X and Y coordinate within screen limits, when limits enabled (IX,IY)
	POP	AF
	PUSH	AF
	PUSH	HL			; save HL
	PUSH	IX
	POP	HL			; HL = second X
	AND	A
	PUSH	HL			; save second X
	SBC	HL,DE			; second X smaller as first X ?
	POP	HL
	JR	NC,J6E4C
	EX	DE,HL			; yep, swap
	AND	A
J6E4C:	EX	(SP),HL
	PUSH	DE			; save DE
	PUSH	IY
	POP	DE			; DE = second Y
	PUSH	HL
	SBC	HL,DE			; second Y smaller as first Y ?
	POP	HL
	JR	C,J6E58
	EX	DE,HL			; yep, swap
J6E58:	PUSH	DE
	POP	IY			; IY = DE
	POP	DE
	POP	IX
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D			; DX = DE
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A			; DY = HL
	PUSH	HL
	PUSH	IX
	POP	HL
	AND	A
	SBC	HL,DE
	INC	HL
	OUT	(C),L
	OUT	(C),H			; NX
	LD	A,L
	OR	E
	PUSH	IY
	POP	HL
	POP	DE
	SBC	HL,DE
	INC	HL
	OUT	(C),L
	OUT	(C),H			; NY
	LD	E,A
	POP	HL
	INC	B
	DJNZ	J6EA7
	LD	A,(SCRMOD)
	CP	8
	JR	Z,J6EBE
	CP	7
	JR	Z,J6E9C
	CP	6
	JR	Z,J6EA2
J6E9C:	LD	A,E
	RRCA
	JR	NC,J6EB7
	JR	J6EA7

J6EA2:	LD	A,E
	AND	3
	JR	Z,J6EB2
J6EA7:	OUT	(C),H
	XOR	A
	OUT	(C),A
	LD	A,B
	OR	80H
	OUT	(C),A
	RET

J6EB2:	LD	A,H
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
J6EB7:	LD	A,H
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	H
	LD	H,A
J6EBE:	OUT	(C),H
	XOR	A
	OUT	(C),A
	LD	A,0C0H
	OUT	(C),A
	RET

;	  Subroutine copy
;	     Inputs  ________________________
;	     Outputs ________________________

I6EC8:	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled (DE,HL)
	CALL	C742D			; force X and Y coordinate within screen limits, when limits enabled (IX,IY)
	PUSH	HL
	PUSH	DE
	LD	DE,(GRPACX)
	LD	HL,(GRPACY)
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	LD	(GRPACX),DE
	LD	(GRPACY),HL
	POP	DE
	POP	HL
	PUSH	DE
	PUSH	BC
	LD	A,32			; VDP register SX, auto increment
	CALL	C707B			; set VDP controlregister
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	LD	A,(LINWRK+0)
	OR	H
	OUT	(C),A
	PUSH	HL
	LD	HL,(GRPACX)
	OUT	(C),L
	OUT	(C),H
	LD	HL,(GRPACY)
	OUT	(C),L
	LD	A,(LINWRK+1)
	OR	H
	OUT	(C),A
	LD	B,0
	LD	A,(GRPACX)
	OR	E
	PUSH	IX
	POP	HL
	SBC	HL,DE
	JR	NC,J6F28
	LD	A,(GRPACX)
	CPL
	LD	D,A
	LD	A,E
	CPL
	OR	D
	EX	DE,HL
	LD	HL,0
	SBC	HL,DE
	SET	2,B
J6F28:	INC	HL
	OUT	(C),L
	OUT	(C),H
	OR	L
	POP	DE
	PUSH	IY
	POP	HL
	SBC	HL,DE
	JR	NC,J6F3E
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	SET	3,B
J6F3E:	INC	HL
	OUT	(C),L
	OUT	(C),H
	OUT	(C),A
	OUT	(C),B
	LD	B,A
	POP	HL
	POP	DE
	LD	A,H
	AND	A
	JR	NZ,J6F68
	LD	A,(SCRMOD)
	CP	8
	JR	Z,J6F6E
	CP	7
	JR	Z,J6F5D
	CP	6
	JR	Z,J6F63
J6F5D:	LD	A,B
	RRCA
	JR	NC,J6F6E
	JR	J6F68

J6F63:	LD	A,B
	AND	3
	JR	Z,J6F6E
J6F68:	LD	A,H
	OR	90H
	OUT	(C),A
	RET

J6F6E:	LD	A,0D0H
	OUT	(C),A
	RET

;	  Subroutine draw point
;	     Inputs  ________________________
;	     Outputs ________________________

I6F73:	PUSH	AF
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J749A			; yep, use MSX1 draw point handler
	POP	AF
	CALL	C73D3			; ?? screen mode 10 action
	PUSH	AF
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	CALL	C7079			; set VDP controlregister = DX, auto increment
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	POP	AF
	OUT	(C),A
	XOR	A
	OUT	(C),A
	LD	A,B
	OR	50H	; "P"
	OUT	(C),A
	RET

;	  Subroutine point
;	     Inputs  ________________________
;	     Outputs ________________________

I6FA9:	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J74A8			; yep, use MSX1 point handler
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	LD	A,32			; VDP register SX, auto increment
	CALL	C707B			; set VDP controlregister
	INC	C
	INC	C
	OUT	(C),E
	OUT	(C),D
	OUT	(C),L
	LD	A,(ACPAGE)
	OR	H
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	OUT	(C),A
	XOR	A
	OUT	(C),A
	LD	A,40H
	OUT	(C),A
	DEC	C
	DEC	C
	LD	H,0
	CALL	C708A			; wait until VDP command is done
	LD	A,7			; VDP status register 7
	DI
	CALL	C709C			; read VDP status register
	IN	L,(C)
	XOR	A			; VDP status register 0
	CALL	C709C			; read VDP status register
	EI
	CALL	C73BE			; check if screen mode 10
	RET	NZ			; nope, quit
	LD	A,L
	AND	0F8H
	RRA
	RRA
	RRA
	RRA
	LD	L,A
	RET	C
	LD	L,0FFH
	RET

;	  Subroutine setup scroll
;	     Inputs  ________________________
;	     Outputs ________________________

C7004:	PUSH	AF
	LD	A,E
	AND	D
	INC	A
	JR	Z,J7023
	LD	C,E
	DEC	DE
	RR	D
	RR	E
	SRL	E
	SRL	E
	INC	E
	LD	A,E
	AND	3FH
	LD	E,A
	LD	A,C
	NEG
	AND	07H
	LD	D,A
	LD	(RG26SAV),DE
J7023:	LD	A,L
	AND	H
	INC	A
	JR	Z,J702C
	LD	A,L
	LD	(RG23SAV),A
J702C:	LD	HL,RG25SAV
	INC	B
	JR	Z,J7038
	SET	1,(HL)
	DJNZ	J7038
	RES	1,(HL)
J7038:	POP	BC
	INC	B
	JR	Z,J7042
	SET	0,(HL)
	DJNZ	J7042
	RES	0,(HL)
J7042:	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	LD	A,2			; VDP status register 2
	DI
	CALL	C709C			; read VDP status register
J704D:	IN	A,(C)
	ADD	A,A
	JP	M,J704D
J7053:	IN	A,(C)
	ADD	A,A
	JP	P,J7053
	XOR	A			; VDP status register 0
	CALL	C709C			; read VDP status register
	LD	A,(RG23SAV)
	OUT	(C),A
	LD	A,128+23
	OUT	(C),A
	LD	A,25
	OUT	(C),A
	LD	A,128+17
	OUT	(C),A
	INC	C
	INC	C
	LD	B,3
	LD	HL,RG25SAV
	OTIR
	EI
	RET

;	  Subroutine set VDP controlregister = DX, auto increment
;	     Inputs  ________________________
;	     Outputs ________________________

C7079:	LD	A,36			; VDP register DX, auto increment

;	  Subroutine set VDP controlregister
;	     Inputs  ________________________
;	     Outputs ________________________

C707B:	PUSH	AF
	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	POP	AF
	DI
	OUT	(C),A
	LD	A,128+17
	EI
	OUT	(C),A

;	  Subroutine wait until VDP command is done
;	     Inputs  ________________________
;	     Outputs ________________________

C708A:	LD	A,2			; VDP status register 2
	DI
	CALL	C709C			; read VDP status register
	IN	A,(C)
	RRA
	LD	A,0			; VDP status register 0
	CALL	C709C			; read VDP status register
	EI
	JR	C,C708A
	RET

;	  Subroutine read VDP status register
;	     Inputs  ________________________
;	     Outputs ________________________

C709C:	OUT	(C),A
	LD	A,128+15
	OUT	(C),A
	RET

;	  Subroutine read VRAM
;	     Inputs  HL = VRAM address
;	     Outputs HL = VRAM value

I70A3:	CALL	C70AB			; setup VDP for VRAM read
	LD	H,0
	IN	L,(C)
	RET

;	  Subroutine setup VDP for VRAM read
;	     Inputs  HL = VRAM address
;	     Outputs ________________________

C70AB:	LD	A,(VDP.DR)
	LD	C,A

;	  Subroutine setup VDP for VRAM read
;	     Inputs  HL = VRAM address, C = VDP data read port
;	     Outputs ________________________

C70AF:	CALL	C7403			; setup VDP active screen page and low byte VRAM address
	EI
	OUT	(C),A			; high byte VRAM address, read VRAM operation
	DEC	C			; data port
	RET

;	  Subroutine write VRAM
;	     Inputs  HL = VRAM address, A = byte
;	     Outputs ________________________

I70B7:	LD	B,A
	CALL	C71B1			; setup VDP for VRAM write
	OUT	(C),B
	RET

;	  Subroutine set sprite mode
;	     Inputs  ________________________
;	     Outputs ________________________

I70BE:	AND	3
	LD	HL,RG1SAV
	RES	0,(HL)
	RES	1,(HL)
	OR	(HL)
	LD	(HL),A
	JP	CLRSPR

;	  Subroutine change display page
;	     Inputs  ________________________
;	     Outputs ________________________

C70CC:	LD	(DPPAGE),A
	RRCA
	RRCA
	RRCA
	AND	0E0H
	LD	E,A			; OR mask
	LD	A,(RG2SAV)
	AND	1FH
	LD	C,2			; register 2
	CALL	C7109			; write VDP register with OR mask
	LD	A,(SCRMOD)
	CP	7
	LD	A,(DPPAGE)
	LD	D,0FCH			; AND mask
	JR	C,J70EE
	ADD	A,A
	LD	D,0FDH			; AND mask
J70EE:	LD	E,A			; OR mask
	LD	A,(RG11SAV)
	LD	C,11			; register 11
	CALL	C7108			; write VDP register with AND and OR mask
	LD	A,D
	RLCA
	RLCA
	RLCA
	RLCA
	LD	D,A			; AND mask
	LD	A,E
	RLCA
	RLCA
	RLCA
	RLCA
	LD	E,A			; OR mask
	LD	A,(RG6SAV)
	LD	C,6			; register 6

;	  Subroutine write VDP register with AND and OR mask
;	     Inputs  ________________________
;	     Outputs ________________________

C7108:	AND	D

;	  Subroutine write VDP register with OR mask
;	     Inputs  ________________________
;	     Outputs ________________________

C7109:	OR	E
	LD	B,A
	JP	WRTVDP

;	  Subroutine change color definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C710E:	PUSH	AF
	PUSH	BC
	PUSH	DE
	LD	IX,S.GETPLT
	CALL	EXTROM
	POP	DE
	LD	L,C
	LD	A,B
	RLCA
	RLCA
	RLCA
	RLCA
	AND	0FH
	LD	E,A
	LD	A,B
	POP	BC
	AND	0FH
	LD	C,A
	LD	A,D
	INC	A
	JR	NZ,J712C
	LD	D,E
J712C:	LD	A,H
	INC	A
	JR	NZ,J7131
	LD	H,L
J7131:	LD	A,B
	INC	A
	JR	NZ,J7136
	LD	B,C
J7136:	LD	A,D
	ADD	A,A
	ADD	A,A
	ADD	A,A
	ADD	A,A
	OR	B
	POP	DE
	LD	E,H
	LD	IX,S.SETPLT
	JP	EXTROM

;	  Subroutine set sprite definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7145:	PUSH	HL
	CALL	CALPAT
	CALL	C71B1			; setup VDP for VRAM write
	CALL	GSPSIZ
	LD	A,8
	JR	NC,J7155
	LD	A,32
J7155:	LD	B,A			; size of sprite definition
	POP	HL
	SUB	(HL)			; string length > size of sprite definition ?
	PUSH	AF
	JR	C,J7160			; yep, use only the size of sprite definition
	LD	B,(HL)			; string length
	INC	B
	DEC	B			; empty string ?
	JR	Z,J7164			; yep, skip changing the sprite definition
J7160:	INC	HL
	CALL	C71A4			; write bytes to VRAM
J7164:	POP	AF
	RET	Z			; complete sprite definition, quit
	RET	C			; complete sprite definition, quit
	LD	B,A
	XOR	A
J7169:	OUT	(C),A
	DJNZ	J7169			; clear remainer of sprite defintion
	RET

;	  Subroutine get sprite definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C716E:	CALL	CALPAT
	CALL	C70AB			; setup VDP for VRAM read
	CALL	GSPSIZ
	LD	B,8
	JR	NC,J717D
	LD	B,32
J717D:	LD	HL,BUF
	PUSH	HL
	LD	(HL),B			; string length
	INC	HL
J7183:	INI
	JR	NZ,J7183
	POP	HL
	RET

;	  Subroutine change sprite color handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7189:	AND	7FH
	PUSH	AF
	CALL	C71A9			; setup VDP for VRAM write of sprite color table
	POP	AF
	LD	B,16
	JR	J7169

;	  Subroutine change sprite color definition handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7194:	PUSH	HL
	CALL	C71A9			; setup VDP for VRAM write of sprite color table
	POP	HL
	LD	A,(HL)
	AND	A
	RET	Z
	CP	16+1
	JR	C,J71A2
	LD	A,16
J71A2:	LD	B,A
	INC	HL

;	  Subroutine write bytes to VRAM
;	     Inputs  ________________________
;	     Outputs ________________________

C71A4:	OUTI
	JR	NZ,C71A4
	RET

;	  Subroutine setup VDP for VRAM write of sprite color table
;	     Inputs  ________________________
;	     Outputs ________________________

C71A9:	LD	A,B
	ADD	A,A			; *2
	ADD	A,A			; *4
	CALL	CALATR
	DEC	H
	DEC	H

;	  Subroutine setup VDP for VRAM write
;	     Inputs  HL = VRAM address
;	     Outputs ________________________

C71B1:	LD	A,(VDP.DR)
	LD	C,A

;	  Subroutine setup VDP for VRAM write
;	     Inputs  HL = VRAM address, C = VDP data write port
;	     Outputs ________________________

C71B5:	CALL	C7403			; setup VDP active screen page and low byte VRAM address
	OR	40H
	EI
	OUT	(C),A			; high byte VRAM address, write VRAM operation
	DEC	C			; data port
	RET

;	  Subroutine set sprite attributes handler
;	     Inputs  ________________________
;	     Outputs ________________________

C71BF:	PUSH	AF
	LD	A,(VDP.DR)
	LD	C,A
	LD	A,(SCRMOD)
	CP	4			; TMS9918 screen mode ?
	JP	NC,J722E		; nope,

; set sprite attributes handler sprite mode 0

	POP	AF
	PUSH	HL
	PUSH	DE
	CALL	CALATR
	POP	DE
	PUSH	HL
	CALL	C70AF			; setup VDP for VRAM read
	LD	A,B
	LD	HL,LINWRK
	LD	B,4
J71DD:	INI
	JR	NZ,J71DD
	LD	B,A
	BIT	7,B
	JR	Z,J7201
	LD	(LINWRK+0),IY
	PUSH	IX
	POP	HL
	LD	A,H
	AND	80H
	LD	E,A
	LD	A,L
	LD	HL,LINWRK+3
	JR	Z,J71F9
	ADD	A,20H	; " "
J71F9:	LD	(LINWRK+1),A
	LD	A,(HL)
	AND	7FH
	OR	E
	LD	(HL),A
J7201:	BIT	6,B
	JR	Z,J7211
	LD	HL,LINWRK+3
	LD	A,D
	AND	0FH	; 15
	LD	D,A
	LD	A,(HL)
	AND	0F0H
	OR	D
	LD	(HL),A
J7211:	POP	HL
	EX	(SP),HL
	BIT	5,B
	JR	Z,J7222
	CALL	GSPSIZ
	LD	A,H
	JR	NC,J721F
	ADD	A,A
	ADD	A,A
J721F:	LD	(LINWRK+2),A
J7222:	POP	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK
	LD	B,4			; 4 bytes
	JP	C71A4			; write bytes to VRAM

; set sprite attributes handler sprite mode 1

J722E:	POP	AF
	PUSH	AF
	PUSH	HL
	PUSH	DE
	PUSH	AF
	CALL	CALATR
	POP	AF
	PUSH	HL
	PUSH	AF
	CALL	C70AF			; setup VDP for VRAM read
	LD	A,B
	LD	HL,LINWRK
	LD	B,3
J7242:	INI
	JR	NZ,J7242
	LD	B,A
	POP	AF
	ADD	A,A
	ADD	A,A
	CALL	CALATR
	DEC	H
	DEC	H
	PUSH	HL
	CALL	C70AF			; setup VDP for VRAM read
	LD	A,B
	LD	HL,LINWRK+4
	LD	B,16
J7259:	INI
	JR	NZ,J7259
	LD	B,A
	LD	E,0
	LD	L,0FFH
	BIT	7,B
	JR	Z,J727B
	LD	(LINWRK+0),IY
	PUSH	IX
	POP	HL
	LD	A,H
	AND	80H
	LD	E,A
	LD	A,L
	JR	Z,J7276
	ADD	A,20H	; " "
J7276:	LD	(LINWRK+1),A
	LD	L,7FH
J727B:	POP	IX
	POP	IY
	POP	AF
	BIT	6,B
	JR	Z,J728C
	AND	0FH	; 15
	OR	E
	LD	E,A
	LD	A,L
	AND	0F0H
	LD	L,A
J728C:	PUSH	BC
	LD	D,L
	LD	HL,LINWRK+4
	LD	B,16
J7293:	LD	A,(HL)
	AND	D
	OR	E
	LD	(HL),A
	INC	HL
	DJNZ	J7293
	POP	BC
	POP	HL
	BIT	5,B
	JR	Z,J72AB
	CALL	GSPSIZ
	LD	A,H
	JR	NC,J72A8
	ADD	A,A
	ADD	A,A
J72A8:	LD	(LINWRK+2),A
J72AB:	PUSH	IX
	POP	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK+4
	LD	B,16			; 16 bytes
	CALL	C71A4			; write bytes to VRAM
	PUSH	IY
	POP	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK
	LD	B,3			; 3 bytes
	CALL	C71A4			; write bytes to VRAM
	POP	DE
	LD	A,1FH
	SUB	D
	RET	Z
	LD	E,A
	LD	D,A
	PUSH	IX
	POP	HL
J72D1:	LD	A,L
	ADD	A,10H	; 16
	LD	L,A
	JR	NC,J72D8
	INC	H
J72D8:	CALL	C70AF			; setup VDP for VRAM read
	LD	B,16
J72DD:	IN	A,(C)
	AND	40H	; "@"
	JR	NZ,J72E7
	DJNZ	J72DD
	JR	J72EA

J72E7:	DEC	E
	JR	NZ,J72D1
J72EA:	LD	A,D
	SUB	E
	RET	Z
	LD	E,A
	PUSH	IX
	POP	HL
	PUSH	DE
	LD	A,(LINWRK+4)
	AND	80H
	LD	D,A
J72F8:	LD	A,L
	ADD	A,10H	; 16
	LD	L,A
	JR	NC,J72FF
	INC	H
J72FF:	PUSH	HL
	CALL	C70AF			; setup VDP for VRAM read
	LD	HL,LINWRK+20
	LD	B,16
J7308:	IN	A,(C)
	AND	7FH
	OR	D
	LD	(HL),A
	INC	HL
	DJNZ	J7308
	POP	HL
	PUSH	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	HL,LINWRK+20
	LD	B,16			; 16 bytes
	CALL	C71A4			; write bytes to VRAM
	POP	HL
	DEC	E
	JR	NZ,J72F8
	POP	DE
	LD	B,E
	PUSH	IY
	POP	HL
J7327:	INC	HL
	INC	HL
	INC	HL
	INC	HL
	CALL	C71B5			; setup VDP for VRAM write
	LD	DE,(LINWRK+0)
	OUT	(C),E
	OUT	(C),D
	DJNZ	J7327
	RET

;	  Subroutine read vdp register
;	     Inputs  ________________________
;	     Outputs ________________________

I7339:	LD	A,H
	AND	A
	JR	Z,J7352
	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	XOR	A
	SUB	L
	DI
	CALL	C709C			; read VDP status register
	IN	L,(C)
	XOR	A			; VDP status register 0
	CALL	C709C			; read VDP status register
	EI
	LD	H,0
	RET

J7352:	LD	DE,RG0SAV
	LD	A,L
	CP	9
	JR	C,J7364
	LD	DE,RG8SAV-9
	CP	25
	JR	C,J7364
	LD	DE,RG25SAV-26
J7364:	ADD	HL,DE
	LD	L,(HL)
	LD	H,0
	RET

;	  Subroutine change screen mode handler (with MSX2+ screen mode support)
;	     Inputs  A = screen mode (0-12)
;	     Outputs ________________________

C7369:	CP	8+1			; special MSX2+ screen mode ?
	JP	C,CHGMOD		; nope, use CHGMOD BIOS
	LD	HL,RG25SAV
	LD	B,(HL)
	LD	C,B			; save current VDP register 25
	SET	3,B
	RES	4,B			; assume YAE = 0, YJK = 1
	CP	12			; screen mode 12 ?
	JR	Z,J73AE			; yep, setup screen mode 8 if needed, setup YAE/YJK and quit
	SET	4,B			; assume YAE = 1, YJK = 1
	LD	HL,MODE
	CP	11			; screen mode 11 ?
	SET	5,(HL)			; assume flag screen mode 11
	JR	Z,J7388			; yep,
	RES	5,(HL)			; flag not screen mode 11
J7388:	BIT	3,C			; screen already in YJK mode ?
	JR	Z,J73B2			; nope, setup screen mode 8, setup YAE/YJK and quit
	BIT	4,C			; screen already in YAE mode ?
	RET	NZ			; yep, quit
	PUSH	BC
	LD	A,0F7H			; YAE attribute
	LD	B,1
	LD	DE,0			; X1 = 0
	LD	L,E
	LD	H,D			; Y1 = 0
	LD	IX,255			; X2 = 255
	LD	IY,255			; Y2 = 255
	CALL	C6E36			; filled box
	LD	A,(VDP.DR)
	LD	C,A
	INC	C
	CALL	C708A			; wait until VDP command is done
	JR	J73B8			; setup YAE/YJK and quit

J73AE:	BIT	3,C			; screen already in YJK mode ?
	JR	NZ,J73B9		; yep, setup YAE/YJK and quit
J73B2:	PUSH	BC
	LD	A,8
	CALL	CHGMOD			; screen mode 8
J73B8:	POP	BC
J73B9:	LD	C,25
	JP	WRTVDP			; setup YAE/YJK

;	  Subroutine check if screen mode 10
;	     Inputs  ________________________
;	     Outputs ________________________

C73BE:	LD	A,(IDBYT2)
	CP	2			; MSX2+ or higher ?
	SBC	A,A
	RET	NZ			; nope, quit
	LD	A,(RG25SAV)
	AND	18H			; YJK/YAE
	CP	18H			; color via color palette ?
	RET	NZ			; nope, quit
	LD	A,(MODE)
	AND	20H			; screen 11
	RET

;	  Subroutine ?? screen mode 10 action
;	     Inputs  ________________________
;	     Outputs ________________________

C73D3:	LD	C,A
	CALL	C73BE			; check if screen mode 10
	LD	A,C
	RET	NZ			; nope, quit
	EX	(SP),HL
	LD	(SWPTMP+4),HL		; save return address
	POP	HL
	INC	B
	DJNZ	J73F9

; B=0

	PUSH	AF
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	LD	A,0FH
	LD	B,1
	CALL	C73FD			; invoke saved return address
	POP	IY
	POP	IX
	POP	HL
	POP	DE
	POP	AF
	LD	B,2
	SCF

J73F9:	RLA
	RLA
	RLA
	RLA

;	  Subroutine invoke saved return address
;	     Inputs  ________________________
;	     Outputs ________________________

C73FD:	PUSH	HL
	LD	HL,(SWPTMP+4)
	EX	(SP),HL
	RET

;	  Subroutine setup VDP active screen page and low byte VRAM address
;	     Inputs  HL = VRAM address
;	     Outputs ________________________

C7403:	INC	C
	LD	A,(IDBYT2)
	AND	A			; MSX1 ?
	JR	Z,J7426			; yep, skip active screen page support
	PUSH	DE
	LD	A,H
	RLCA
	RLCA
	AND	03H
	LD	E,A
	LD	A,(SCRMOD)
	CP	7
	LD	A,(ACPAGE)
	JR	C,J741C
	ADD	A,A
J741C:	ADD	A,A
	ADD	A,E
	POP	DE
	DI
	OUT	(C),A
	LD	A,128+14
	OUT	(C),A			; setup page
J7426:	DI
	OUT	(C),L			; low byte VRAM address
	LD	A,H
	AND	3FH
	RET

;	  Subroutine force X and Y coordinate within screen limits, when limits enabled
;	     Inputs  IX = X value, IY = Y value
;	     Outputs ________________________

C742D:	PUSH	HL
	PUSH	DE
	PUSH	IX
	POP	DE
	PUSH	IY
	POP	HL
	CALL	C7441			; force X and Y coordinate within screen limits, when limits enabled
	PUSH	HL
	POP	IY
	PUSH	DE
	POP	IX
	POP	DE
	POP	HL
	RET

;	  Subroutine force X and Y coordinate within screen limits, when limits enabled
;	     Inputs  DE = X value, HL = Y value
;	     Outputs ________________________

C7441:	LD	A,(MODE)
	AND	10H			; 212 lines mode ?
	RET	NZ			; nope, quit
	LD	A,D
	ADD	A,A			; X value negative ?
	JR	NC,J7450		; nope,
	LD	DE,0			; yep, use X = 0
	JR	J745C

J7450:	LD	A,(SCRMOD)
	RRCA
	AND	01H			; screen mode 5 & 7 512, screen mode 6 & 8 256
	CP	D			; X value bigger or equal to max X ?
	JR	NC,J745C		; nope,
	LD	E,0FFH
	LD	D,A			; yep, use X = 255 or 511
J745C:	LD	A,H
	ADD	A,A			; Y value negative ?
	JR	NC,J7464		; nope,
	LD	HL,0			; yep, use Y = 0
	RET

J7464:	JR	NZ,J746A		; Y > 255, use Y = 211
	LD	A,L
	CP	211+1
	RET	C
J746A:	LD	HL,211
	RET

J746E:	XOR	A
	LD	IX,CLS
	JP	J74EE

J7476:	LD	BC,M58FC		; line handler
	JR	J7483

J747B:	LD	BC,M5912		; line box handler
	JR	J7483

J7480:	LD	BC,M58C1		; line boxfill handler
J7483:	POP	AF
	LD	(ATRBYT),A
	PUSH	BC
	LD	(GXPOS),DE
	LD	(GYPOS),HL
	PUSH	IX
	POP	BC
	PUSH	IY
	POP	DE
	POP	IX
	JP	J74EE

J749A:	POP	AF
	LD	(ATRBYT),A
	LD	C,E
	LD	B,D
	EX	DE,HL
	LD	IX,M57F1+4
	JP	J74EE			; PSET handler

J74A8:	LD	C,E
	LD	B,D
	EX	DE,HL
	CALL	SCALXY
	LD	HL,-1
	RET	NC
	CALL	MAPXYC
	CALL	READC
	LD	L,A
	LD	H,0
	RET

;	  Subroutine paint handler
;	     Inputs  ________________________
;	     Outputs ________________________

C74BC:	LD	(BRDATR),A
	LD	A,B
	LD	(ATRBYT),A
	LD	C,E
	LD	B,D
	EX	DE,HL
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J74D5			; yep, use MSX1 paint handler
	LD	IX,S266E
	JP	PAINT_FIX          ; EXTROM			; PAINT handler (subrom)

J74D5:	LD	IX,M59E3		; PAINT handler
	JP	J74EE

;	  Subroutine circle
;	     Inputs  A = color
;	     Outputs ________________________

I74DC:	EX	DE,HL
	LD	HL,LINWRK+3
	LD	(HL),0
	DEC	HL
	LD	(HL),A
	DEC	HL
	LD	(HL),0FH
	DEC	HL
	LD	(HL),","		; ,<int constant>
	LD	IX,M5B19		; CIRCLE handler
J74EE:	CALL	CALBAS
	EI
	RET

;	  Subroutine print tab
;	     Inputs  ________________________
;	     Outputs ________________________

C74F3:	LD	A,9
	RST	18H
	RET

;	  Subroutine print new line
;	     Inputs  ________________________
;	     Outputs ________________________

C74F7:	LD	A,13
	RST	18H
	LD	A,10
	RST	18H
	RET

;	  Subroutine print string
;	     Inputs  ________________________
;	     Outputs ________________________

C74FE:	LD	B,(HL)
	LD	A,B
	AND	A
	RET	Z
J7502:	INC	HL
	LD	A,(HL)
	RST	18H
	DJNZ	J7502
	RET

;	  Subroutine print integer
;	     Inputs  ________________________
;	     Outputs ________________________

C7508:	CALL	C7BD1			; integer to text
	JR	J7510

;	  Subroutine print float
;	     Inputs  ________________________
;	     Outputs ________________________

C750D:	CALL	C7C2B			; float to text
J7510:	CALL	XBASIC_PRINT_STR			; print string
	LD	A," "
	RST	18H
	RET

;	  Subroutine print string and get numeric input
;	     Inputs  ________________________
;	     Outputs ________________________

I7517:	CALL	XBASIC_PRINT_STR			; print string

;	  Subroutine get numeric input
;	     Inputs  ________________________
;	     Outputs ________________________

J751A:	CALL	QINLIN
	JP	C,J4DFA			; abort, end program
	EX	DE,HL
	INC	DE
	CALL	C7D18			; convert numeric string to float
	LD	A,(DE)
	AND	A			; at end of string ?
	RET	Z			; yep, quit
	LD	HL,I7530
	CALL	XBASIC_PRINT_STR			; print string
	JR	J751A			; try again

I7530:	DEFB	18
	DEFB	"?Redo from start",13,10

;	  Subroutine print string and get string input
;	     Inputs  ________________________
;	     Outputs ________________________

I7543:	CALL	XBASIC_PRINT_STR			; print string

;	  Subroutine get string input
;	     Inputs  ________________________
;	     Outputs ________________________

I7546:	CALL	QINLIN
	JP	C,J4DFA			; abort, end program
	INC	H
	LD	E,L
	LD	D,H
	INC	DE
	LD	BC,256
	LDDR
	INC	HL
	LD	B,0FFH
J7558:	INC	B
	INC	DE
	LD	A,(DE)
	AND	A
	JR	NZ,J7558
	LD	(HL),B
	RET

; read string input handler

I7560:	CALL	C75BE			; skip to the start of DATA
J7563:	LD	A,(HL)
	INC	HL
	CP	" "
	JR	Z,J7563			; skip spaces
	LD	DE,BUF+1
	LD	B,0			; current string length = 0
	CP	'"'			; string indicator ?
	JR	Z,J7593			; yep,
	DEC	HL
J7573:	LD	A,(HL)
	AND	A			; end of line ?
	JR	Z,J758B			; yep, end of input
	CP	":"			; statement seperator ?
	JR	Z,J758B			; yep, end of input
	CP	","			; DATA item seperator ?
	JR	Z,J758B			; yep, end of input
	LD	(DE),A
	INC	DE
	INC	HL
	INC	B
	JR	J7573			; next character

J7585:	INC	HL
	LD	A,(HL)
	CP	" "
	JR	Z,J7585
J758B:	LD	(DATPTR),HL		; update current DATA pointer
	LD	HL,BUF
	LD	(HL),B			; set string length
	RET

J7593:	LD	A,(HL)
	AND	A			; end of line ?
	JR	Z,J758B			; yep, end of input
	CP	'"'			; string indicator ?
	JR	Z,J7585			; yep, end of string
	LD	(DE),A
	INC	DE
	INC	HL
J759E:	INC	B
	JR	J7593			; next character

; read numeric input handler

I75A1:	CALL	C75BE			; skip to the start of DATA
	EX	DE,HL
	CALL	C7D18			; convert numeric string to float
	LD	(DATPTR),DE		; save DATA pointer
	LD	A,(DE)
	AND	A			; end of line ?
	RET	Z			; yep, quit
	CP	":"			; end of statement ?
	RET	Z			; yep, quit
	CP	","			; next data item ?
	RET	Z			; yep, quit
	LD	HL,(DATLIN)
	LD	(CURLIN),HL		; update current BASIC line number (generated error contains the current line number)
	JP	J4E16			; BASIC error: syntax error

;	  Subroutine skip to the start of DATA
;	     Inputs  ________________________
;	     Outputs ________________________

C75BE:	LD	HL,(DATPTR)		; current DATA pointer
	LD	A,(HL)
	CP	","
	JR	NZ,J75DC
	INC	HL
	RET

J75C8:	XOR	A
	LD	B,0FFH
	CPIR				; skip to end of line
J75CD:	LD	A,(HL)
	INC	HL
	OR	(HL)			; end of program ?
	JP	Z,J4E07			; yep, BASIC error: out of data
	INC	HL
	LD	E,(HL)
	INC	HL
	LD	D,(HL)			; line number
	LD	(DATLIN),DE		; update current DATA line number
	INC	HL

J75DC:	LD	B,0			; reset in a string flag
J75DE:	LD	A,(HL)
	INC	HL
	AND	A			; end of line ?
	JR	Z,J75CD			; yep, to the next line
	CP	84H			; DATA token ?
	JR	NZ,J75EA		; nope,
	INC	B
	DEC	B			; string indicator ?
	RET	Z			; nope, quit
J75EA:	CP	'"'			; start of DATA string ?
	JR	NZ,J75F2		; nope,
	INC	B
	DJNZ	J75DC			; in a string, continue to next character
	INC	B			; B=1, set in a sting flag
J75F2:	CP	0FFH			; function token ?
	INC	HL
	JR	Z,J75DE			; yep, continue to next character
	DEC	HL
	CP	8FH			; REM token ?
	JR	Z,J75C8			; yep, skip line and continue
	CP	1FH+1			; special number token ?
	JR	NC,J75DE		; nope, continue to next character
	CP	0BH			; unknown token ?
	JR	C,J75DE			; yep, continue to next character
	CP	0FH			; integer 11-255 ?
	LD	C,1
	JR	Z,J761C			; yep, skip next byte
	INC	C
	JR	C,J761C			; octal number, hexadecimal number, linepointer or linenumber, skip next 2 bytes
	CP	1BH			; integer 0 - integer 9 ? (?? BUG: should be CP 1CH, because of integer 10)
	JR	C,J75DE			; yep, continue to next character
	SUB	1CH			; integer ?
	JR	Z,J761C			; yep, skip next 2 bytes
	DEC	A			; single real ?
	LD	C,4
	JR	Z,J761C			; yep, skip next 4 bytes
	LD	C,8			; double real, skip next 8 bytes
J761C:	LD	A,L
	ADD	A,C
	LD	L,A
	JR	NC,J75DE
	INC	H
	JR	J75DE			; continue to next character

;	  Subroutine multiply
;	     Inputs  ________________________
;	     Outputs ________________________

C7624:	LD	A,40H
	DEFB	0CBH,037H		; SLL A (Z80), TST A (R800)
	CP	81H
	JR	Z,J7631			; Z80,
	LD	C,E
	LD	B,D
	DEFB	0EDH,0C3H		; MULUW HL,BC
	RET

J7631:	LD	C,L
	LD	B,H
	LD	HL,0
	LD	A,16
J7638:	ADD	HL,HL
	EX	DE,HL
	ADD	HL,HL
	EX	DE,HL
	JR	NC,J763F
	ADD	HL,BC
J763F:	DEC	A
	JR	NZ,J7638
	RET

;	  Subroutine divide
;	     Inputs  ________________________
;	     Outputs ________________________

C7643:	PUSH	DE
	PUSH	HL
	BIT	7,D
	JR	NZ,J764F
	XOR	A
	SUB	E
	LD	E,A
	SBC	A,A
	SUB	D
	LD	D,A
J764F:	LD	C,E
J7650:	LD	B,D
	BIT	7,H
	JR	Z,J765B
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A
J765B:	EX	DE,HL
	XOR	A
	LD	L,A
	LD	H,A
	LD	A,E
	LD	E,16
	RLA
	RL	D
J7665:	ADC	HL,HL
	PUSH	HL
	ADD	HL,BC
	JR	NC,J766E
	INC	SP
	INC	SP
	DEFB	030H			; pseudo JR NC,xx (skip next instruction)
J766E:	POP	HL
	RLA
	RL	D
	DEC	E
	JR	NZ,J7665
	LD	E,A
	EX	DE,HL
	POP	AF
	AND	A
	JP	P,J7684
	XOR	A
	SUB	E
	LD	E,A
	SBC	A,A
	SUB	D
	LD	D,A
	LD	A,80H
J7684:	POP	BC
	XOR	B
	RET	P
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A
	RET

;	  Subroutine RND handler
;	     Inputs  ________________________
;	     Outputs ________________________

I768E:	LD	A,B
	AND	A
	JR	Z,J76C9
	BIT	7,H
	JR	Z,J769F
	LD	(SWPTMP+2),HL
	LD	H,B
	LD	L,"5"
	LD	(SWPTMP+0),HL
J769F:	LD	HL,(SWPTMP+0)
	LD	E,L
	LD	D,H
	ADD	HL,HL
	LD	C,L
	LD	B,H
	JR	NC,J76AA
	INC	BC
J76AA:	ADD	HL,DE
	LD	(SWPTMP+0),HL
	LD	HL,(SWPTMP+2)
	LD	E,L
	LD	D,H
	ADC	HL,HL
	ADD	HL,DE
	ADD	HL,BC
	LD	(SWPTMP+2),HL
J76BA:	LD	A,L
	OR	H
	LD	B,A
	RET	Z
	LD	B,80H
J76C0:	BIT	7,H
	RES	7,H
	RET	NZ
	ADD	HL,HL
	DEC	B
	JR	J76C0

J76C9:	LD	HL,(SWPTMP+2)
	JR	J76BA

;	  Subroutine remainer of divide integer
;	     Inputs  ________________________
;	     Outputs ________________________

C76CE:	CALL	C7643			; divide
	EX	DE,HL
	RET

;	  Subroutine subtract floats
;	     Inputs  ________________________
;	     Outputs ________________________

C76D3:	LD	A,D
	XOR	80H
	LD	D,A

;	  Subroutine add floats
;	     Inputs  ________________________
;	     Outputs ________________________

C76D7:	LD	A,C
	AND	A
	RET	Z
	LD	A,B
	AND	A
	JP	Z,J7745
	SUB	C
	JR	NC,J76E6
	NEG
	EX	DE,HL
	LD	B,C
J76E6:	CP	11H
	RET	NC
	LD	C,A
	LD	A,H
	XOR	D
	ADD	A,A
	LD	A,H
	PUSH	AF
	SET	7,H
	SET	7,D
	INC	C
	DEC	C
	JR	Z,J7701
J76F7:	SRL	D
	RR	E
	DEC	C
	JR	NZ,J76F7
	JR	NC,J7701
	INC	DE
J7701:	POP	AF
	JR	C,J771C
	ADD	HL,DE
	JR	NC,J770E
	INC	B
	JR	Z,J7715
	RR	H
	RR	L
J770E:	RES	7,H
J7710:	AND	A
	RET	P
	SET	7,H
	RET

J7715:	LD	B,0FFH
	LD	HL,7FFFH		; (max positive float) 1.701E+38
	JR	J7710

J771C:	AND	A
	SBC	HL,DE
	JR	NC,J7728
	EX	DE,HL
	LD	HL,1
	SBC	HL,DE
	CPL
J7728:	LD	C,A
J7729:	LD	A,H
	AND	A
	JR	NZ,J773C
	OR	L
	JR	Z,J7739
	LD	H,L
	LD	L,0
	LD	A,B
	SUB	08H	; 8
	LD	B,A
	JR	NC,J773C
J7739:	LD	B,0
	RET

J773C:	LD	A,C
J773D:	BIT	7,H
	JR	NZ,J770E
	ADD	HL,HL
	DJNZ	J773D
	RET

J7745:	LD	B,C
	EX	DE,HL
	RET

;	  Subroutine multipy floats
;	     Inputs  ________________________
;	     Outputs ________________________

C7748:	LD	A,B
	AND	A
	RET	Z
	LD	A,C
	AND	A
	JR	Z,J7739
	ADD	A,B
	LD	B,A
	RRA
	XOR	B
	LD	A,B
	JP	P,J7823
	ADD	A,80H
	LD	B,A
	RET	Z
	LD	A,H
	XOR	D
	LD	C,A
	PUSH	BC
	SET	7,H
	SET	7,D
	LD	A,40H
	DEFB	0CBH,037H		; SLL A (Z80), TST A (R800)
	CP	81H
	JR	Z,J7774			; Z80,
	LD	C,E
	LD	B,D
	DEFB	0EDH,0C3H		; MULUW HL,BC
	LD	A,H
	EX	DE,HL
	JP	J781A

J7774:	LD	C,L
	LD	A,H
	LD	HL,0
	RRA
	RR	C
	JR	NC,J777F
	ADD	HL,DE
J777F:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7789
	ADD	HL,DE
J7789:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7793
	ADD	HL,DE
J7793:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J779D
	ADD	HL,DE
J779D:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77A7
	ADD	HL,DE
J77A7:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77B1
	ADD	HL,DE
J77B1:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77BB
	ADD	HL,DE
J77BB:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77C5
	ADD	HL,DE
J77C5:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77CF
	ADD	HL,DE
J77CF:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77D9
	ADD	HL,DE
J77D9:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77E3
	ADD	HL,DE
J77E3:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77ED
	ADD	HL,DE
J77ED:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J77F7
	ADD	HL,DE
J77F7:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7801
	ADD	HL,DE
J7801:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J780B
	ADD	HL,DE
J780B:	RR	H
	RR	L
	RRA
	RR	C
	JR	NC,J7815
	ADD	HL,DE
J7815:	RR	H
	RR	L
	RRA
J781A:	POP	BC
	ADD	A,A
	JP	NC,J773C
	INC	HL
	JP	J773C

J7823:	AND	A
	JP	P,J7739
	LD	A,H
	XOR	D
	JP	J7715

;	  Subroutine divide floats
;	     Inputs  ________________________
;	     Outputs ________________________

C782C:	LD	A,C
	AND	A
	LD	A,H
	JP	Z,J7715
	LD	A,B
	AND	A
	RET	Z
	SUB	C
	LD	B,A
	RRA
	XOR	B
	LD	A,B
	JP	M,J7823
	ADD	A,80H
	LD	B,A
	RET	Z
	LD	A,H
	XOR	D
	INC	B
	JP	Z,J7715
	LD	C,A
	PUSH	BC
	SET	7,H
	SET	7,D
	XOR	A
	LD	C,A
	LD	B,16
	JR	J7857

J7853:	ADC	HL,HL
	JR	C,J7873
J7857:	SBC	HL,DE
	JR	NC,J785C
	ADD	HL,DE
J785C:	CCF
J785D:	RL	C
	RLA
	DJNZ	J7853
	SRL	D
	RR	E
	AND	A
	SBC	HL,DE
	LD	L,C
	LD	H,A
	POP	BC
	JP	C,J7729
	INC	HL
	JP	J7729

J7873:	OR	A
	SBC	HL,DE
	JR	J785D

;	  Subroutine ^ handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7878:	LD	A,E
	OR	D
	JP	Z,C7924			; quit with 1.0
	BIT	7,D
	PUSH	AF
	JP	P,J7889
	XOR	A
	SUB	E
	LD	E,A
	SBC	A,A
	SUB	D
	LD	D,A
J7889:	PUSH	DE
	LD	C,B
	EX	DE,HL
	CALL	C7924			; 1.0
J788F:	EX	(SP),HL
	SRL	H
	RR	L
	EX	(SP),HL
	JR	NC,J789F
	PUSH	DE
	PUSH	BC
	CALL	C7748			; multipy floats
	POP	DE
	LD	C,E
	POP	DE
J789F:	EX	(SP),HL
	LD	A,L
	OR	H
	EX	(SP),HL
	JR	Z,J78B4
	PUSH	HL
	PUSH	BC
	LD	B,C
	LD	H,D
	LD	L,E
	CALL	C7748			; multipy floats
	LD	C,B
	EX	DE,HL
	POP	HL
	LD	B,H
	POP	HL
	JR	J788F

J78B4:	POP	AF
	POP	AF
	RET	P

;	  Subroutine 1/x
;	     Inputs  B:HL = float
;	     Outputs ________________________

C78B7:	LD	C,B
	EX	DE,HL			; C:DE = float
	CALL	C7924			; 1.0
	JP	C782C			; divide floats

;	  Subroutine ^
;	     Inputs  ________________________
;	     Outputs ________________________

I78BF:	PUSH	DE
	PUSH	BC
	CALL	C7B01			; LOG handler
	POP	DE
	LD	C,E
	POP	DE			; C:DE = float
	CALL	C7748			; multipy floats
	JP	J7AA8			; EXP handler

;	  Subroutine convert unsigned integer to float
;	     Inputs  HL = unsigned integer
;	     Outputs B:HL = float

C78CD:	BIT	7,H			; positive integer ?
	PUSH	AF
	RES	7,H			; to postive
	CALL	C78DF			; convert integer to float
	POP	AF
	RET	Z			; positive, quit
	LD	C,90H
	LD	DE,0			; 3.277E+04
	JP	C76D7			; add floats

;	  Subroutine convert integer to float
;	     Inputs  HL = integer
;	     Outputs B:HL = float

C78DF:	LD	A,L
	OR	H
	JR	Z,J78FF
	BIT	7,H
	PUSH	AF
	JR	Z,J78EE
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A
J78EE:	LD	B,90H
J78F0:	BIT	7,H
	JR	NZ,J78F8
	ADD	HL,HL
	DEC	B
	JR	J78F0

J78F8:	RES	7,H
	POP	AF
	RET	Z
	SET	7,H
	RET

J78FF:	LD	B,A			; 0.0
	RET

;	  Subroutine convert float to integer
;	     Inputs  ________________________
;	     Outputs ________________________

C7901:	LD	A,B
	AND	A			; B=0 ?
	JR	Z,J7926			; yep, return float 0.0
	DEC	B			; B=1 ?
	JP	P,J7926			; yep, return float 0.0
	BIT	7,H
	PUSH	AF			; save sign
	SET	7,H
	LD	DE,0
J7911:	ADD	HL,HL
	RL	E
	RL	D
	DEC	B
	JP	M,J7911
	EX	DE,HL
	POP	AF			; sign set ?
	RET	Z			; nope, quit
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A			; yep, negate
	RET

;	  Subroutine float 1.0
;	     Inputs  ________________________
;	     Outputs ________________________

C7924:	LD	B,81H
J7926:	LD	HL,0
	RET

;	  Subroutine float =
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C792A:	CALL	C7966			; compare floats
	LD	HL,0
	RET	NZ			; not equal, return false
	DEC	HL			; equal, return true
	RET

;	  Subroutine float <>
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7933:	CALL	C7966			; compare floats
	LD	HL,0
	RET	Z			; equal, return false
	DEC	HL			; not equal, return true
	RET

;	  Subroutine float >
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C793C:	CALL	C7966			; compare floats
	LD	HL,0
	RET	C			; smaller, return false
	RET	Z			; equal, return false
	DEC	HL			; bigger, return true
	RET

;	  Subroutine float >=
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7946:	CALL	C7966			; compare floats
	LD	HL,0
	RET	C			; smaller, return false
	DEC	HL			; bigger or equal, return true
	RET

;	  Subroutine float <
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C794F:	CALL	C7966			; compare floats
	LD	HL,0
	RET	NC			; bigger or equal, return false
	DEC	HL			; smaller, return true
	RET

;	  Subroutine float <=
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7958:	CALL	C7966			; compare floats
	LD	HL,-1
	RET	C			; smaller, return true
	RET	Z			; equal, return true
	INC	HL			; bigger, return false
	RET

;	  Subroutine float <=>
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs HL = result

C7962:	LD	HL,-1			; return true
	RET

;	  Subroutine compare floats
;	     Inputs  B:HL = float1, C:DE = float2
;	     Outputs ________________________

C7966:	LD	A,C
	AND	A			; float2 0.0 ?
	JR	Z,J7986			; yep,
	LD	A,B
	AND	A			; float1 0.0 ?
	LD	A,D
	JR	Z,J7981			; yep,
	XOR	H			; float1 has same sign as float2 ?
	LD	A,D
	JP	M,J7981			; nope,
	LD	A,C
	CP	B
	JR	NZ,J797F
	LD	A,D
	CP	H
	JR	NZ,J797F
	LD	A,E
	SUB	L
	RET	Z
J797F:	RRA
	XOR	D
J7981:	CPL
	OR	01H
	RLA
	RET

J7986:	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit
	LD	A,H
	RLA
	RET

;	  Subroutine FIX handler
;	     Inputs  ________________________
;	     Outputs ________________________

C798C:	BIT	7,H
	PUSH	AF			; save sign
	RES	7,H			; force positive
	CALL	C7999			; INT handler
	POP	AF			; restore sign
	RET	Z			; was a positive, quit
	SET	7,H			; force negative
	RET

;	  Subroutine INT handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7999:	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit
	CP	91H			; > float 65540 ?
	RET	NC			; yep, quit
	SUB	81H			; < 1 or < -1 ?
	JR	C,J79D7			; yep,
	LD	DE,8000H
	JR	Z,J79AF
J79A8:	SRA	D
	RR	E
	DEC	A
	JR	NZ,J79A8
J79AF:	BIT	7,H
	JR	NZ,J79BA
	LD	A,H
	AND	D
	LD	H,A
	LD	A,L
	AND	E
	LD	L,A
	RET

J79BA:	PUSH	HL
	LD	A,H
	AND	D
	LD	H,A
	LD	A,L
	AND	E
	LD	L,A
	EX	(SP),HL
	LD	A,D
	CPL
	AND	H
	JR	NZ,J79CE
	LD	A,E
	CPL
	AND	L
	JR	NZ,J79CE
	POP	HL
	RET

J79CE:	POP	HL
	LD	C,81H
	LD	DE,8000H		; float -1.0
	JP	C76D7			; add floats

J79D7:	BIT	7,H
	LD	B,0			; float 0.0
	RET	Z
	LD	B,81H
	LD	HL,8000H		; float -1.0
	RET

;	  Subroutine COS handler
;	     Inputs  ________________________
;	     Outputs ________________________

C79E2:	LD	C,81H
	LD	DE,4910H		; float 1.571 (PI/2)
	CALL	C76D7			; add floats

;	  Subroutine SIN handler
;	     Inputs  ________________________
;	     Outputs ________________________

C79EA:	LD	A,B
	CP	77H			; < 0.0004883 or < -0.0004883 ?
	RET	C			; yep, quit
	BIT	7,H			; positive float ?
	JR	Z,C79FC			; yep, calculate sinus
	RES	7,H			; force positive
	CALL	C79FC			; calculate sinus

;	  Subroutine change sign
;	     Inputs  ________________________
;	     Outputs ________________________

J79F7:	LD	A,H
	XOR	80H
	LD	H,A
	RET

;	  Subroutine calculate sinus
;	     Inputs  ________________________
;	     Outputs ________________________

C79FC:	LD	C,7EH
	LD	DE,22FAH		; float 0.1592
	CALL	C7748			; multipy floats
	PUSH	HL
	PUSH	BC
	CALL	C7999			; INT handler
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	CALL	C76D3			; subtract floats
	LD	C,7FH
	LD	DE,0			; float 0.25
	CALL	C7966			; compare floats
	JR	C,J7A31			; smaller,
	LD	C,80H
	LD	DE,0C000H		; float -0.75
	CALL	C76D7			; add floats
	SET	7,H			; force negative
	LD	C,7FH
	LD	DE,0			; float 0.25
	CALL	C76D7			; add floats
	LD	A,H
	XOR	80H
	LD	H,A			; change sign
J7A31:	BIT	7,H
	PUSH	AF			; save sign
	RES	7,H			; force positive
	LD	IX,I7A53
	CALL	C7B88			; polynomial approximation odd series
	POP	AF			; restore sign
	RET	Z			; positive float, quit
	JR	J79F7			; change sign and quit

;	  Subroutine TAN handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7A41:	PUSH	HL
	PUSH	BC
	CALL	C79E2			; COS handler
	LD	A,B
	POP	BC
	EX	(SP),HL
	PUSH	AF
	CALL	C79EA			; SIN handler
	POP	AF
	LD	C,A
	POP	DE
	JP	C782C			; divide floats

I7A53:	DEFB	5
	DEFB	086H,01EH,0D8H		; float  39.71
	DEFB	087H,099H,026H		; float -76.58
	DEFB	087H,023H,034H		; float  81.60
	DEFB	086H,0A5H,05EH		; float -41.34
	DEFB	083H,049H,010H		; float  6.283

;	  Subroutine ATN handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7A63:	BIT	7,H			; postive float ?
	JR	Z,C7A6E			; yep, calculate arc tangent and quit
	RES	7,H			; force positive
	CALL	C7A6E			; calculate arc tangent
	JR	J79F7			; change sign and quit

;	  Subroutine calculate arc tangent
;	     Inputs  ________________________
;	     Outputs ________________________

C7A6E:	LD	A,B
	CP	81H			; < 1.0 or < -1.0 ?
	JR	C,C7A85			; yep,
	CALL	C78B7			; 1/x
	CALL	C7A85			; calculate ATN
	LD	A,H
	XOR	80H
	LD	H,A			; change sign
	LD	C,81H
	LD	DE,4910H		; float 1.571 (PI/2)
	JP	C76D7			; add floats

;	  Subroutine calculate ATN
;	     Inputs  ________________________
;	     Outputs ________________________

C7A85:	LD	IX,I7A8C		; ATN table
	JP	C7B88			; polynomial approximation odd series

I7A8C:	DEFB	9
	DEFB	078H,03BH,0D7H		; float  0.002866
	DEFB	07BH,084H,06EH		; float -0.01617
	DEFB	07CH,02FH,0C2H		; float  0.04291
	DEFB	07DH,09AH,031H		; float -0.07529
	DEFB	07DH,05AH,03EH		; float  0.1066
	DEFB	07EH,091H,080H		; float -0.1421
	DEFB	07EH,04CH,0BCH		; float  0.1999
	DEFB	07FH,0AAH,0AAH		; float -0.3333
	DEFB	081H,000H,000H		; float  1.0

;	  Subroutine EXP handler
;	     Inputs  ________________________
;	     Outputs ________________________

J7AA8:	LD	C,81H
	LD	DE,38AAH		; float 1.443
	CALL	C7748			; multipy floats
	LD	A,B
	CP	88H			; > float 128 ?
	JR	NC,J7AE1		; yep,
	CP	68H			; < float -0.0000000298
	JP	C,C7924			; yep, uit with 1.0
	PUSH	HL
	PUSH	BC
	CALL	C7999			; INT handler
	PUSH	HL
	PUSH	BC
	CALL	C7901			; convert float to integer
	LD	A,L
	ADD	A,81H
	POP	HL
	POP	DE
	POP	BC
	LD	C,H
	POP	HL
	JR	Z,J7AE1
	PUSH	AF
	CALL	C76D3			; subtract floats
	LD	IX,I7AEB
	CALL	C7B99			; polynomial approximation even series
	POP	AF
	LD	C,A
	LD	DE,0
	JP	C7748			; multipy floats

J7AE1:	BIT	7,H
	LD	B,0			; float 0.0
	RET	NZ
	DEC	B
	LD	HL,7FFFH		; float 1.701E+38 (max float)
	RET

I7AEB:	DEFB	7
	DEFB	074H,059H,088H		; float 0.0002075
	DEFB	077H,026H,098H		; float 0.001271
	DEFB	07AH,01EH,01EH		; float 0.009651
	DEFB	07CH,063H,050H		; float 0.0555
	DEFB	07EH,075H,0FEH		; float 0.2402
	DEFB	080H,031H,072H		; float 0.6932
	DEFB	081H,000H,000H		; float 1.0

;	  Subroutine LOG handler
;	     Inputs  B:HL = float
;	     Outputs ________________________

C7B01:	BIT	7,H			; sign negative ?
	RET	NZ			; yep, quit (error)
	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit (error)
	LD	C,B
	LD	D,H
	LD	E,L			; C:DE = float
	LD	A,80H
	LD	B,A
	XOR	C
	PUSH	AF
	PUSH	HL
	PUSH	BC
	LD	IX,I7B46
	CALL	C7B99			; polynomial approximation even series
	POP	AF
	POP	DE
	PUSH	HL
	PUSH	BC
	LD	B,A
	EX	DE,HL
	LD	IX,I7B53
	CALL	C7B99			; polynomial approximation even series
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	CALL	C782C			; divide floats
	POP	AF
	PUSH	HL
	PUSH	BC
	RLCA
	RRCA
	LD	L,A
	SBC	A,A
	LD	H,A
	CALL	C78DF			; convert integer to float
	POP	AF
	LD	C,A
	POP	DE
	CALL	C76D7			; add floats
	LD	C,80H
	LD	DE,3172H		; float 0.6932
	JP	C7748			; multipy floats and quit

I7B46:	DEFB	4
	DEFB	083H,019H,0F8H		; float  4.812
	DEFB	083H,043H,063H		; float  6.106
	DEFB	084H,08DH,0CDH		; float -8.863
	DEFB	082H,083H,080H		; float -4.109

I7B53:	DEFB	4
	DEFB	081H,000H,000H		; float 1.0
	DEFB	083H,04DH,0B1H		; float 6.428
	DEFB	083H,011H,072H		; float 4.545
	DEFB	07FH,035H,005H		; float 0.3536

;	  Subroutine SQR handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7B60:	LD	A,B
	AND	A			; float 0.0 ?
	RET	Z			; yep, quit
	RES	7,H			; force positive
	RRA
	ADC	A,40H
	LD	C,A
	LD	D,H
	LD	E,L
	LD	A,3
J7B6D:	PUSH	AF
	PUSH	HL
	PUSH	BC
	PUSH	DE
	PUSH	BC
	CALL	C782C			; divide floats
	POP	DE
	LD	C,E
	POP	DE
	CALL	C76D7			; add floats
	DEC	B
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	POP	AF
	DEC	A
	JR	NZ,J7B6D
	LD	B,C
	EX	DE,HL
	RET

;	  Subroutine polynomial approximation odd series
;	     Inputs  ________________________
;	     Outputs ________________________

C7B88:	PUSH	HL
	PUSH	BC			; save float
	LD	C,B
	LD	D,H
	LD	E,L			; C:DE = float
	CALL	C7748			; multipy floats
	CALL	C7B99			; polynomial approximation even series
	POP	AF
	LD	C,A
	POP	DE			; restore float in C:DE
	JP	C7748			; multipy floats

;	  Subroutine polynomial approximation even series
;	     Inputs  B:HL = float, IX = pointer to serie of floats
;	     Outputs ________________________
; X*N1+N2

C7B99:	LD	A,(IX+0)		; number of items in serie
	INC	IX
	PUSH	AF			; save number of items in serie
	PUSH	HL
	PUSH	BC			; save float
	LD	B,(IX+0)
	LD	H,(IX+1)
	LD	L,(IX+2)		; B:HL = first float of serie
	INC	IX
	INC	IX
	INC	IX			; update pointer to next float in serie
J7BB0:	POP	AF
	LD	C,A
	POP	DE			; restore float in C:DE
	POP	AF			; restore number of items in serie
	DEC	A			; more items in serie ?
	RET	Z			; nope, quit
	PUSH	AF			; save number of items in serie
	PUSH	DE
	LD	A,C
	PUSH	AF			; save float in C:DE
	CALL	C7748			; multipy floats
	LD	C,(IX+0)
	LD	D,(IX+1)
	LD	E,(IX+2)
	CALL	C76D7			; add floats
	INC	IX
	INC	IX
	INC	IX			; update pointer to next float in serie
	JR	J7BB0			; next

;	  Subroutine integer to text
;	     Inputs  ________________________
;	     Outputs ________________________

C7BD1:	LD	IX,FBUFFR+1
	BIT	7,H			; postive integer ?
	LD	A," "
	JR	Z,J7BE6			; yep, use space as first character
	XOR	A
	SUB	L
	LD	L,A
	SBC	A,A
	SUB	H
	LD	H,A			; negate
	JP	M,J7BF9			; still negative (was -32768), -32768 string and quit
	LD	A,"-"			; - as first character
J7BE6:	LD	(IX+0),A
	INC	IX
	CALL	C7C0E			; generate string from integer
J7BEE:	PUSH	IX
	POP	HL
	LD	A,L
	SUB	LOW (FBUFFR+1)		; calculate string length
	LD	HL,FBUFFR+0
	LD	(HL),A			; string length
	RET

J7BF9:	LD	DE,FBUFFR+6
	LD	HL,I7C07+6
	LD	BC,7
	LDDR
	INC	DE
	EX	DE,HL
	RET

I7C07:	DEFB	6
	DEFB	"-32768"

;	  Subroutine generate string from integer
;	     Inputs  ________________________
;	     Outputs ________________________

C7C0E:	INC	H
	DEC	H
	JR	NZ,J7C17
	LD	A,L
	CP	9+1
	JR	C,J7C22
J7C17:	LD	DE,10
	CALL	C7643			; / 10
	PUSH	DE
	CALL	C7C0E
	POP	HL
J7C22:	LD	A,L
	ADD	A,"0"
	LD	(IX+0),A
	INC	IX
	RET

;	  Subroutine float to text
;	     Inputs  ________________________
;	     Outputs ________________________

C7C2B:	LD	IX,FBUFFR+1
	CALL	C7C42			; generate string from float
	JP	J7BEE			; set string length and quit

J7C35:	LD	(IX+0)," "
	INC	IX
	LD	(IX+0),"0"
	INC	IX
	RET

;	  Subroutine generate string from float
;	     Inputs  ________________________
;	     Outputs ________________________

C7C42:	LD	A,B
	AND	A			; float 0.0 ?
	JR	Z,J7C35			; yep, string 0 and quit
	BIT	7,H			; positive float ?
	LD	A," "
	JR	Z,J7C50			; yep, use space as first character
	RES	7,H			; negate
	LD	A,"-"			; use - as first character
J7C50:	LD	(IX+0),A
	INC	IX
	XOR	A
J7C56:	PUSH	AF
	LD	C,8AH
	LD	DE,79FCH		; float 1000
	CALL	C7966			; compare floats
	JR	NC,J7C6D		; > 1000,
	LD	C,84H
	LD	DE,2000H		; float 10
	CALL	C7748			; multipy floats
	POP	AF
	DEC	A
	JR	J7C56

J7C6D:	LD	C,8EH
	LD	DE,1C3EH		; float 10000
	CALL	C7966			; compare floats
	JR	C,J7C83			; < 10000,
	LD	C,84H
	LD	DE,2000H		; float 10
	CALL	C782C			; divide floats
	POP	AF
	INC	A
	JR	J7C56

J7C83:	LD	C,80H
	LD	DE,0			; float 0.5
	CALL	C76D7			; add floats
	CALL	C7901			; convert float to integer
	POP	AF
	ADD	A,5
	CP	6
	JR	NC,J7CE3
	DEC	A
	JP	P,C7CA7			; convert integer to digits
	LD	(IX+0),"."
	INC	IX
	LD	(IX+0),"0"
	INC	IX
	LD	A,0FFH			; force remainer check

;	  Subroutine convert integer to digits
;	     Inputs  HL = integer, A = flag
;	     Outputs ________________________

C7CA7:	LD	B,A
	LD	DE,1000
	CALL	C7CC0			; divide and add digit
	RET	NC			; remainer = 0, quit
	LD	DE,100
	CALL	C7CC0			; divide and add digit
	RET	NC			; remainer = 0, quit
	LD	DE,10
	CALL	C7CC0			; divide and add digit
	RET	NC			; remainer = 0, quit
	LD	DE,1

;	  Subroutine divide and add digit
;	     Inputs  HL = integer, DE = divider
;	     Outputs ________________________

C7CC0:	INC	B
	DJNZ	J7CCB			; B<>0, skip
	LD	(IX+0),"."
	INC	IX
	LD	B,0FFH			; force remainer check
J7CCB:	LD	A,"0"-1
J7CCD:	INC	A
	AND	A
	SBC	HL,DE
	JR	NC,J7CCD		; again
	ADD	HL,DE
	LD	(IX+0),A		; add digit
	INC	IX
	DEC	B			; B=1 ?
	JR	Z,J7CDE			; yep,
	SCF
	RET	P			; b7=0, quit with Cx set
J7CDE:	LD	A,L
	OR	H			; integer left = 0 ?
	RET	Z			; yep, quit with Cx reset, Zx set
	SCF				; quit with Cx set
	RET

J7CE3:	PUSH	AF
	LD	A,1			; force 1 digit
	CALL	C7CA7			; convert integer to digits
	LD	(IX+0),"E"
	INC	IX
	POP	AF
	SUB	2
	LD	L,A
	LD	A,"+"
	JP	P,J7CFE
	LD	A,L
	NEG
	LD	L,A
	LD	A,"-"
J7CFE:	LD	(IX+0),A
	INC	IX
	LD	A,L
	LD	B,"0"-1
J7D06:	INC	B
	SUB	10
	JR	NC,J7D06
	ADD	A,'0'+10
	LD	(IX+0),B
	INC	IX
	LD	(IX+0),A
	INC	IX
	RET

;	  Subroutine convert numeric string to float
;	     Inputs  ________________________
;	     Outputs ________________________

C7D18:	LD	B,0
	CALL	C7D34			; skip spaces
	CP	"-"
	JR	NZ,J7D28
	INC	DE
	CALL	C7D18			; convert numeric string to float
	JP	J79F7			; change sign and quit

J7D28:	CP	"&"
	JR	NZ,J7D8D		; nope, convert numeric string
	CALL	C7D3B			; convert &B,&O,&H string
	PUSH	DE
	CALL	C78DF			; convert integer to float
	POP	DE

;	  Subroutine skip spaces
;	     Inputs  ________________________
;	     Outputs ________________________

C7D34:	LD	A,(DE)
	CP	" "
	RET	NZ
	INC	DE
	JR	C7D34

;	  Subroutine convert &B,&O,&H string
;	     Inputs  ________________________
;	     Outputs ________________________

C7D3B:	LD	HL,0
	INC	DE
	CALL	C7E38		; get character and to upper
	INC	DE
	CP	"B"
	JR	Z,J7D7E
	CP	"O"
	JR	Z,J7D6D
	CP	"H"
	DEC	DE
	RET	NZ

; convert &H

	INC	DE
J7D50:	CALL	C7E38		; get character and to upper
	AND	A
	RET	Z
	SUB	"0"
	RET	C
	CP	9+1
	JR	C,J7D64
	CP	11H
	RET	C
	CP	17H
	RET	NC
	SUB	07H
J7D64:	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	A,L
	LD	L,A
	INC	DE
	JR	J7D50

; convert &O

J7D6D:	LD	A,(DE)
	AND	A
	RET	Z
	SUB	"0"
	RET	C
	CP	7+1
	RET	NC
	ADD	HL,HL
	ADD	HL,HL
	ADD	HL,HL
	ADD	A,L
	LD	L,A
	INC	DE
	JR	J7D6D

; convert &B

J7D7E:	LD	A,(DE)
	AND	A
	RET	Z
	SUB	"0"
	RET	C
	CP	1+1
	RET	NC
	ADD	HL,HL
	ADD	A,L
	LD	L,A
	INC	DE
	JR	J7D7E

; convert numeric string

J7D8D:	LD	C,1
	DEC	DE
J7D90:	INC	DE
	CALL	C7D34			; skip spaces
	CP	"0"
	JR	C,J7DC4
	CP	"9"+1
	JR	NC,J7DC4
	PUSH	DE
	PUSH	BC
	PUSH	AF
	LD	C,84H
	LD	DE,2000H		; float 10
	CALL	C7748			; multipy floats
	POP	AF
	SUB	"0"
	JR	Z,J7DBB
	LD	C,88H
J7DAE:	DEC	C
	ADD	A,A
	JP	P,J7DAE
	AND	7FH
	LD	D,A
	LD	E,0
	CALL	C76D7			; add floats
J7DBB:	POP	DE
	LD	C,E
	POP	DE
	DEC	C
	JR	NZ,J7D90
	INC	C
	JR	J7D90

J7DC4:	CP	"."
	JR	NZ,J7DCC
	DEC	C
	JR	Z,J7D90
	RET

J7DCC:	DEC	C
	JR	Z,J7DD0
	INC	C
J7DD0:	CALL	C7E39			; to upper
	CP	"E"			; exponent indicator ?
	JR	Z,J7DDB			; yep,
	CP	"D"			; exponent indicator (double real) ?
	JR	NZ,J7DF5		; nope, skip exponent
J7DDB:	INC	DE
	CALL	C7D34			; skip spaces
	CP	"+"
	JR	Z,J7DEF
	CP	"-"
	JR	NZ,J7DF0
	INC	DE			; to next character
	CALL	C7E1C			; get exponent
	NEG				; negate
	JR	J7DF3

J7DEF:	INC	DE			; to next character
J7DF0:	CALL	C7E1C			; get exponent
J7DF3:	ADD	A,C
	LD	C,A
J7DF5:	LD	A,C
	AND	A			;
	RET	Z
	PUSH	DE
	PUSH	AF
	JP	P,J7DFF
	NEG
J7DFF:	LD	E,A
	LD	D,0
	PUSH	HL
	PUSH	BC
	LD	B,84H
	LD	HL,2000H		; float 10.0
	CALL	C7878			; ^ handler
	LD	C,B
	EX	DE,HL
	POP	AF
	LD	B,A
	POP	HL
	POP	AF
	PUSH	AF
	CALL	P,C7748			; multipy floats
	POP	AF
	CALL	M,C782C			; divide floats
	POP	DE
	RET

;	  Subroutine get exponent
;	     Inputs  ________________________
;	     Outputs ________________________

C7E1C:	PUSH	HL
	LD	L,0
J7E1F:	CALL	C7D34			; skip spaces
	CP	"9"+1
	JR	NC,J7E35
	SUB	"0"
	JR	C,J7E35
	LD	H,A
	LD	A,L
	ADD	A,A
	ADD	A,A
	ADD	A,L
	ADD	A,A			; *10
	ADD	A,H
	LD	L,A
	INC	DE
	JR	J7E1F

J7E35:	LD	A,L
	POP	HL
	RET

;	  Subroutine get character and to upper
;	     Inputs  ________________________
;	     Outputs ________________________

C7E38:	LD	A,(DE)

;	  Subroutine to upper
;	     Inputs  ________________________
;	     Outputs ________________________

C7E39:	CP	"a"
	RET	C
	CP	"z"+1
	RET	NC
	AND	0DFH
	RET

;	  Subroutine left of string
;	     Inputs  ________________________
;	     Outputs ________________________

I7E42:	CP	(HL)
	JR	C,J7E46
	LD	A,(HL)
J7E46:	INC	HL
	JR	J7E70

;	  Subroutine right of string
;	     Inputs  ________________________
;	     Outputs ________________________

I7E49:	CP	(HL)
	JR	C,J7E4D
	LD	A,(HL)
J7E4D:	PUSH	AF
	LD	E,(HL)
	NEG
	ADD	A,E
	INC	A
	LD	E,A
	LD	D,0
	ADD	HL,DE
	POP	AF
	JR	J7E70

;	  Subroutine part of string
;	     Inputs  ________________________
;	     Outputs ________________________

C7E5A:	LD	C,B
	PUSH	AF
	ADD	A,C
	JR	C,J7E63
	DEC	A
	CP	(HL)
	JR	C,J7E6C
J7E63:	POP	AF
	LD	A,(HL)
	SUB	C
	JR	NC,J7E6A
	LD	A,0FFH
J7E6A:	INC	A
	PUSH	AF
J7E6C:	LD	B,0
	ADD	HL,BC
	POP	AF
J7E70:	LD	DE,BUF
	LD	(DE),A
	AND	A
	JR	Z,J7E7D
	INC	DE
	LD	C,A
	LD	B,0
	LDIR
J7E7D:	LD	HL,BUF
	RET

;	  Subroutine replace in string
;	     Inputs  ________________________
;	     Outputs ________________________

C7E81:	EX	DE,HL
	CP	(HL)
	JR	Z,J7E86
	RET	NC
J7E86:	LD	C,A
	LD	A,(DE)
	CP	B
	JR	NC,J7E8C
	LD	B,A
J7E8C:	LD	A,C
	DEC	A
	ADD	A,B
	JR	C,J7E96
	CP	(HL)
	JR	C,J7E9A
	JR	Z,J7E9A
J7E96:	LD	A,(HL)
	SUB	C
	INC	A
	LD	B,A
J7E9A:	LD	A,L
	ADD	A,C
	LD	L,A
	JR	NZ,J7EA0
	INC	H
J7EA0:	EX	DE,HL
	INC	HL
	LD	C,B
	LD	B,0
	LDIR
	RET

;	  Subroutine CHR$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EA8:	LD	HL,BUF+1
	LD	(HL),A			; character
	DEC	HL
	LD	(HL),1			; string length = 1
	RET

;	  Subroutine VAL handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7EB0:	LD	DE,BUF+1
	PUSH	DE
	LD	A,(HL)
	AND	A			; string length
	JR	Z,J7EBE			; empty string,
	LD	C,A
	XOR	A
	LD	B,A			; string length
	INC	HL
	LDIR				; copy string to BUF+1
J7EBE:	LD	(DE),A			; string end marker
	POP	DE
	JP	C7D18			; convert numeric string to float

;	  Subroutine HEX$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EC3:	LD	C,4
	DEFB	011H

;	  Subroutine OCT$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EC6:	LD	C,3
	DEFB	011H

;	  Subroutine BIN$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EC9:	LD	C,1
	LD	DE,FBUFFR+16
J7ECE:	LD	B,C
	XOR	A
J7ED0:	SRL	H
	RR	L
	RRA
	DJNZ	J7ED0
	LD	B,C
J7ED8:	RLCA
	DJNZ	J7ED8
	CP	9+1
	JR	C,J7EE1
	ADD	A,7
J7EE1:	ADD	A,"0"
	LD	(DE),A
	DEC	DE
	LD	A,L
	OR	H
	JR	NZ,J7ECE
	LD	A,0D5H
	SUB	E
	LD	(DE),A
	EX	DE,HL
	RET

;	  Subroutine SPACE$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

I7EEF:	LD	B,A
	LD	A," "

;	  Subroutine STRING$ handler
;	     Inputs  B = number of characters, A = character
;	     Outputs ________________________

C7EF2:	LD	HL,BUF
	LD	(HL),B			; string length
	INC	B
	DEC	B			; length = 0 ?
	RET	Z			; yep, quit
	PUSH	HL
	INC	HL
J7EFB:	LD	(HL),A			; character
	INC	HL
	DJNZ	J7EFB
	POP	HL
	RET

;	  Subroutine INKEY$ handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7F01:	CALL	CHSNS			; key in buffer ?
	LD	HL,BUF
	LD	A,0
	JR	Z,J7F13			; nope, quit with empty string
	CALL	CHGET			; get key from buffer
	INC	HL
	LD	(HL),A			; character (key)
	DEC	HL
	LD	A,1			; string length = 1
J7F13:	LD	(HL),A
	RET

;	  Subroutine INSTR handler
;	     Inputs  ________________________
;	     Outputs ________________________

C7F15:	LD	C,A
	LD	A,(DE)
	SUB	C
	SUB	(HL)
	ADD	A,2
	LD	B,A
	LD	A,C
	DEC	A
	ADD	A,E
	LD	E,A
	JR	NC,J7F23
	INC	D
J7F23:	PUSH	BC
	PUSH	HL
	LD	B,(HL)
	INC	HL
	INC	DE
	PUSH	DE
J7F29:	LD	A,(DE)
	CP	(HL)
	JR	NZ,J7F37
	INC	DE
	INC	HL
	DJNZ	J7F29
	POP	AF
	POP	AF
	POP	AF
	LD	L,C
	LD	H,B
	RET

J7F37:	POP	DE
	POP	HL
	POP	BC
	INC	C
	DJNZ	J7F23
	LD	L,B
	LD	H,B
	RET

;	  Subroutine copy string to NULBUF
;	     Inputs  HL = pointer to string
;	     Outputs ________________________

C7F40:	LD	DE,(NULBUF)
	LD	C,(HL)			; size of string
	LD	B,0
	INC	BC			; include size byte
	LDIR
	RET

;	  Subroutine string1 > string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F4B:	CALL	C7F7B			; compare strings
	LD	HL,0
	RET	C			; smaller, return false
	RET	Z			; equal, return false
	DEC	HL			; bigger, return true
	RET

;	  Subroutine string1 = string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F55:	CALL	C7F74			; compare strings, assume true
	RET	Z			; equal, return true
	INC	HL			; not equal, return false
	RET

;	  Subroutine string1 >= string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F5B:	CALL	C7F74			; compare strings, assume true
	RET	NC			; bigger or equal, return true
	INC	HL			; smaller, return false
	RET

;	  Subroutine string1 < string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F61:	CALL	C7F74			; compare strings, assume true
	RET	C			; smaller, return true
	INC	HL			; bigger or equal, return false
	RET

;	  Subroutine string1 <> string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F67:	CALL	C7F74			; compare strings, assume true
	RET	NZ			; not equal, return true
	INC	HL			; equal, return false
	RET

;	  Subroutine string1 <= string 2
;	     Inputs  NULBUF = string 1, HL = string2
;	     Outputs ________________________

C7F6D:	CALL	C7F74			; compare strings, assume true
	RET	C			; smaller, return true
	RET	Z			; equal, return true
	INC	HL			; bigger, return false
	RET

;	  Subroutine compare strings, assume true
;	     Inputs  ________________________
;	     Outputs ________________________

C7F74:	CALL	C7F7B			; compare strings

;	  Subroutine string1 <=> string2
;	     Inputs  ________________________
;	     Outputs ________________________

C7F77:	LD	HL,-1			; return true
	RET

;	  Subroutine compare strings
;	     Inputs  NULBUF = first string, HL = second string
;	     Outputs ________________________

C7F7B:	LD	B,(HL)			; size second string
	INC	HL
	LD	DE,(NULBUF)
	LD	A,(DE)
	LD	C,A			; size first string
	INC	DE
J7F84:	LD	A,C
	OR	B			; at the end of both strings ?
	RET	Z			; yep, quit with Zx set
	LD	A,C
	SUB	1			; at the end of first string ?
	RET	C			; yep, quit with Cx set, Zx reset
	LD	A,B
	SUB	1			; at the end of second string
	CCF
	RET	NC			; yep, quit with Cx reset, Zx reset
	DEC	B
	DEC	C
	LD	A,(DE)
	CP	(HL)			; compare character
	INC	DE
	INC	HL
	JR	Z,J7F84			; equal, next character
	RET

;	  Subroutine concat strings
;	     Inputs  NULBUF = string1, HL = string2
;	     Outputs BUF = result string

C7F99:	LD	DE,BUF
	LD	BC,(NULBUF)
	AND	A
	PUSH	HL
	SBC	HL,DE			; string2 in BUF ?
	POP	HL
	JR	Z,J7FD2			; yep,
	LD	A,(BC)			; size of string1
	ADD	A,(HL)			; size of string2
	JR	NC,J7FAD
	LD	A,255			; concat string to 255 characters
J7FAD:	LD	(DE),A			; result string size
	INC	DE
	PUSH	HL
	LD	A,(BC)			; size of string1
	PUSH	AF
	AND	A			; empty string1 ?
	JR	Z,J7FBD			; yep, skip copy string1
	INC	BC
	LD	L,C
	LD	H,B
	LD	C,A
	LD	B,0
	LDIR				; copy string1
J7FBD:	POP	AF			; size of string1
	POP	HL			; string2
	LD	B,A
	LD	A,(HL)			; size string2
	JR	NC,J7FC5		; string2 not limited
	LD	A,B
J7FC4:	CPL
J7FC5:	AND	A			; empty string2 ?
	JR	Z,J7FCE			; yep, skip copy string2
	INC	HL
	LD	C,A
	LD	B,0
	LDIR				; copy string2
J7FCE:	LD	HL,BUF			; string in BUF
	RET

J7FD2:	PUSH	BC
	PUSH	HL
	LD	E,C
	LD	D,B
	LD	A,(DE)
	CP	(HL)
	JR	NC,J7FDB
	LD	A,(HL)
J7FDB:	LD	B,A
	INC	B
J7FDD:	LD	C,(HL)
	LD	A,(DE)
	LD	(HL),A
	LD	A,C
	LD	(DE),A
	INC	DE
	INC	HL
	DJNZ	J7FDD
	POP	BC
	LD	A,(BC)
	LD	L,A
	LD	H,0
	INC	HL
	ADD	HL,BC
	EX	DE,HL
	POP	HL
	ADD	A,(HL)
	JR	C,J7FF6
	LD	(BC),A
	LD	A,(HL)
	JR	J7FC5

J7FF6:	LD	A,(BC)
	PUSH	AF
	LD	A,0FFH
	LD	(BC),A
	POP	AF
	JR	J7FC4

BASIC_KUN_END_FILLER:
	DEFS	08000H-$,0FFH





