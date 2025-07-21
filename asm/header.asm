; ------------------------------------------------------------------------------------------------------
; MSXBAS2ROM Z80 header routines (for compiled mode)
; by: Amaury Carvalho, 2020
; Compile with:
;   pasmo header.asm header.bin
;   xxd -i header.bin header.h
; ------------------------------------------------------------------------------------------------------

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
ENASLT:       equ 0x0024   ; enable slot (a=slot indicator, hl=address to enable) - interruptions enabled after call
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
HPHYD:        equ 0xFFA7   ; physical disk input-output hook

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
PNTINI:       equ 0x0129 ; paint initialize - A = border color

PUFOUT:       equ 0x3426 ; DAC to formatted text
FRCSNG:       equ 0x2FB2 ; DAC to single
FRCDBL:       equ 0x303A ; DAC to double
FRCINT:       equ 0x2F8A ; DAC to integer
DECNRM:       equ 0x26FA ; normalize DAC
CLPRIM:       equ 0xF38C

ROMBDOS:      equ 0xF37D
REDCLK:	      equ 0x01F5 ; Reading a register of the internal clock (RTC)
                         ; Entry:  C = block number (bits 5-4) and register (bits 3-0) to read.
                         ; Output: A = 4 least significant bits content of the register read.
WRTCLK:       equ 0x01F9 ; Writing in a register of the internal clock (RTC).
                         ; Entry:  C = block number (bits 5-4) and register (bits 3-0).
                         ;         A = data to write. (4 least significant bits)

; ------------------------------------------------------------------------------------------------------
; BIOS AND BASIC WORK AREA
; ------------------------------------------------------------------------------------------------------

BASKUN_COPY:                equ I6EC8  ; BASIC KUN original COPY (DE=X0, HL=Y0, IX=X1, IY=Y1, B=OPERATION, FCB7=XD, FCB9=YD, FC18=PS, FC19=PD)
BASKUN_VDP_WAIT:            equ C708A  ; BASIC KUN vdp wait

; ------------------------------------------------------------------------------------------------------
; BIOS AND BASIC WORK AREA
; https://www.msx.org/wiki/System_variables_and_work_area
; ------------------------------------------------------------------------------------------------------

USRTAB:       equ 0xF39A   ; 20
VALTYP:       equ 0xF663   ; 1
VALDAT:       equ 0xF7F8   ; 2
DAC:          equ 0xF7F6   ; 16
ARG:          equ 0xF847   ; 16

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
NTMSXP:       equ 0xF417   ; printer type (0=default)
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
CGPBAS:       equ 0xF924  ; pattern generator table - basic
PATBAS:       equ 0xF926  ; sprite pattern table
ATRBAS:       equ 0xF928  ; sprite attribute table
CRTCNT:       equ 0xF3B1  ; line text count (default=24)
LINLEN:       equ 0xF3B0  ; line text length (default=39)
LINL40:       equ 0xF3AE  ; Screen width per line in SCREEN 0 (Default 39)
LINL32:       equ 0xF3AF  ; Screen width per line in SCREEN 1 (Default 29)
CLMLST:       equ 0xF3B2  ; X-location in the case that items are divided by commas in PRINT (LINLEN-(LINLEN MOD 14)-14)

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
CSRSW:        equ 0xFCA9  ; 0=cursor on when CHGET, 1=cursor always on
CSRY:         equ 0xF3DC  ; cursor Y pos
CSRX:         equ 0xF3DD  ; cursor X pos
TTYPOS:       equ 0xF661  ; teletype position
XSAVE:        equ 0xFAFE  ; 2, cache for used by NEWPAD to fetch coordinates from any devices it handles
YSAVE:        equ 0xFB00  ; 2, cache for used by NEWPAD to fetch coordinates from any devices it handles
PADX:         equ 0xFC9D  ; 1, coordenada X do paddle/mouse
PADY:         equ 0xFC9C  ; 1, coordenada Y do paddle/mouse

TXTTAB:       equ 0xF676     ; start of basic program
VARTAB:       equ 0xF6C2     ; start of variables area (end of basic program)
ARYTAB:       equ 0xF6C4     ; start of array area
STREND:       equ 0xF6C6     ; end of variables area
OLDTXT:       equ 0xF6C0     ; last program line
OLDLIN:       equ 0xF6BE     ; last line executed by interpreter
BASROM:       equ 0xFBB1     ; user basic code on rom? (0=RAM, not 0 = ROM)
NLONLY:       equ 0xF87C     ; loading basic program flags (bit 0=not close i/o buffer 0, bit 7=not close user i/o buffer)
LPTPOS:       equ 0xF415     ; printer head horizontal position
ONELIN:       equ 0xF6B9     ; error line number
ONEFLG:       equ 0xF6BB     ; error flag
PRTFLG:       equ 0xF416     ; output to screen (0=true)
PTRFLG:       equ 0xF6A9     ; line number converted to pointer (0=false)
DORES:        equ 0xF664     ; 1 - DATA flag to ASCII format
PRMFLG:       equ 0xF7B4
CONSAV:       equ 0xF668     ; numeric token used by CHRGTR
SUBFLG:       equ 0xF6A5     ; 1 (0=simple variable, not 0 = array)
MAXFIL:       equ 0xF85F
FLGINP:       equ 0xF6A6     ; used by INPUT/READ (0=INPUT, not 0=READ)
STMTKTAB:     equ 0x392E     ; addresses of BASIC statement token service routines (start from 081H to 0D8H)
FNCTKTAB:     equ 0x39DE     ; addresses of BASIC function token service routines

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
BASVER:       equ 0x002C       ; BASIC VERSION: >=10 = international, 00=japanese (keyboard 0=japanese, 1=international, 2=french, 3=UK, 4=DIN)

; ------------------------------------------------------------------------------------------------------
; HOMEBREW SUPPORT FUNCTIONS
; ------------------------------------------------------------------------------------------------------

XBASIC_SUBTRACT_FLOATS: equ 0x76D3
XBASIC_CAST_INTEGER_TO_FLOAT: equ 0x78DF
XBASIC_MULTIPLY_INTEGERS: equ 0x7631
XBASIC_DIVIDE_INTEGERS: equ 0x7643
XBASIC_CAST_INTEGER_TO_STRING: equ 0x7BD1

; ------------------------------------------------------------------------------------------------------
; HOMEBREW WORK AREA
; ------------------------------------------------------------------------------------------------------

WRKARE:       equ 0xC010       ; homebrew rom internal workarea start in RAM (alternatives: SLTWRK or PARM2)

SLTSTR:       equ WRKARE       ; 00 - 1 - startup slotid
PLYSTS:       equ SLTSTR+1     ; 01 - 1 - player song status flag (0=idle, 1=play, 2=mute)
PLYLOOP:      equ PLYSTS+1     ; 02 - 1 - player song loop status
PLYCOUNT:     equ PLYLOOP+1    ; 03 - 1 - player frame count (for 50hz/60hz control)
PLYADDR:      equ PLYCOUNT+1   ; 04 - 2 - player song start address
PLYHKSAV:     equ PLYADDR+2    ; 06 - 5 - HTIMI hook saved

SLTAD0:       equ PLYHKSAV+5   ; 11 - 1 - default slotid from page 0
SLTAD1:       equ SLTAD0+1     ; 12 - 1 - default slotid from page 1
SLTAD2:       equ SLTAD1+1     ; 13 - 1 - default slotid from page 2
SLTAD3:       equ SLTAD2+1     ; 14 - 1 - default slotid from page 3
MPRAD0:       equ SLTAD3+1     ; 15 - 1 - default segment of memory mapper from page 0
MPRAD2:       equ MPRAD0+1     ; 16 - 1 - default segment of memory mapper from page 2
MPRAD2N:      equ MPRAD2+1     ; 17 - 1 - new segment of memory mapper from page 2
MAPPER:       equ MPRAD2N+1    ; 18 - 1 - mapper status (bit 0=mapper on/off, bit 1=running on RAM)
SGMADR:       equ MAPPER+1     ; 19 - 1 - current MegaROM segment selected in &8000
SOMODE:       equ SGMADR+1     ; 20 - 1 - screen output mode (0=text, 1=graphical, 2=tiled)
HEAPSTR:      equ SOMODE+1     ; 21 - 2 - heap start address
HEAPSIZ:      equ HEAPSTR+2    ; 23 - 2 - heap size
TMPSTRIND:    equ HEAPSIZ+2    ; 25 - 1 - temporary string index
TMPSTRADDR:   equ TMPSTRIND+1  ; 26 - 2 - temporary string start address
TMPSTRBUF:    equ TMPSTRADDR+2 ; 28 - 2 - temporary string next pointer
FONTOLDSLT:   equ TMPSTRBUF+2  ; 30 - 1 - old bios font slot
FONTOLD:      equ FONTOLDSLT+1 ; 31 - 2 - old bios font address
FONTADDR:     equ FONTOLD+2    ; 33 - 2 - new font space address
RSCMAPAD:     equ FONTADDR+2   ; 35 - 2 - resource map address (copy on ram)
RSCMAPSG:     equ RSCMAPAD+2   ; 37 - 1 - resource map segment number (copy on ram)
RSCMAPT1:     equ RSCMAPSG+1   ; 38 - 1 - resource map temporary space for last segment number
PLYSGTM:      equ RSCMAPT1+1   ; 39 - 1 - player song segment (megarom)

BASMEM:       equ PLYSGTM+1    ; 40 - RAM starts after compiler internal variables

PLYBUF:       equ 0xEF00
SPRTBL:       equ PLYBUF - (32*5)      ; 32 sprites * (test, x0, x1, y0, y1)
SPRSIZ:       equ SPRTBL - 1
HEAPEND:      equ SPRSIZ - 1

PageSize:	  equ 4000h	               ; 16kB

BASTEXT:                equ 0x800E     ; address of user basic code
resource.map.address:   equ 0x800B     ; resource map address
resource.map.segment:   equ 0x800D     ; resource map segment

; ------------------------------------------------------------------------------------------------------
; MACROS
; ------------------------------------------------------------------------------------------------------

MACRO call_basic,CALL_PARM
    ld ix, CALL_PARM
    call CALBAS
    ei
ENDM

; ------------------------------------------------------------------------------------------------------
; INITIALIZE
; ------------------------------------------------------------------------------------------------------

	org 4000h

; ### ROM header ###

	db "AB"		    ; ID for auto-executable ROM
	dw INIT1	    ; Main program execution address.
    dw 0x0000	    ; STATEMENT
	dw 0		    ; DEVICE
	dw 0		    ; TEXT
    db 'MSXB2R'     ; MSXBAS2ROM signature
	;dw 0,0,0	    ; Reserved

INIT1:	                    ; Program code entry point label - 'c' has rom slot id

    ld a, (EXPTBL)
    ld h, 0x00
    call SUB_ENASLT         ; page 0 - enable bios rom (Victor HC-95/Sony HB-F500 bug fix on ExecROM with disk driver)

    ld h, 0x40              ; page 1 - this rom
    call page.getslt

    ld (SLTSTR), a          ; start slot

    ld hl, pre_start.hook_data
    ld de, HSTKE
    ld bc, 5
    ldir

    ld (HSTKE+1), a         ; program rom slot

    ret

pre_start.hook_data:
  db 0xF7, 0x00             ; CALLF
  dw start
  db 0xC9

start:
  call verify.slots

  call select_rom_on_page_2

megarom_ascii8_bug_fix:
  ld a, (0x8000)            ; ASCII8 MegaROM loading bug fix
  cp 0x41
  jr nz, clear_basic_environment
    ld a, (0x8001)
    cp 0x42
    jr nz, clear_basic_environment
      ld a, 1
      ld (0x6800), a
      inc a
      ld (0x7000), a
      inc a
      ld (0x7800), a

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
  ld (MAXFIL), a             ; reset max files
  ld (NLONLY), a             ; reset i/o buffers

  ld A,','
  ld (BUFMIN),A              ; dummy prefix for BUF

  ld A,":"
  ld (KBFMIN),A              ; dummy prefix for KBUF
  ld (ENDPRG),a              ; fake line

  ld a, 0xFF
  ld (PTRFLG), a             ; line number converted to pointer (0=false)
  ld (DSKDIS), a             ; disable disks

  call KILBUF
  call INITXT                ; screen 0
  call SUB_CLRSPR            ; clear sprites

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
; MAP OF XBASIC WRAPPER ROUTINES
;---------------------------------------------------------------------------------------------------------

wrapper_routines_map_start:
  jp castParamFloatInt
  jp cmd_clrkey
  jp cmd_clrscr
  jp cmd_disscr
  jp cmd_draw
  jp cmd_enascr
  jp cmd_keyclkoff
  jp cmd_mute
  jp cmd_play
  jp cmd_pad

  jp cmd_plyload
  jp cmd_plyloop
  jp cmd_plymute
  jp cmd_plyplay
  jp cmd_plyreplay
  jp cmd_plysong
  jp cmd_plysound

  jp cmd_ramtoram
  jp cmd_ramtovram
  jp cmd_rsctoram
  jp cmd_restore
  jp cmd_runasm

  jp cmd_screen_copy
  jp cmd_screen_load
  jp cmd_screen_paste

  jp cmd_setfnt
  jp cmd_turbo
  jp cmd_updfntclr
  jp cmd_vramtoram
  jp cmd_wrtchr
  jp cmd_wrtclr
  jp cmd_wrtfnt
  jp cmd_wrtscr
  jp cmd_wrtspr
  jp cmd_wrtspratr
  jp cmd_wrtsprclr
  jp cmd_wrtsprpat
  jp cmd_wrtvram
  jp cmd_page

  jp floatNeg
  jp gfxTileAddress

  jp intCompareAND
  jp intCompareEQ
  jp intCompareGE
  jp intCompareGT
  jp intCompareLE
  jp intCompareLT
  jp intCompareNE
  jp intCompareNOT
  jp intCompareOR
  jp intCompareXOR
  jp intNeg
  jp intSHL
  jp intSHR

  jp player.initialize
  jp player.unhook

  jp set_tile_flip
  jp set_tile_rotate
  jp set_tile_color
  jp set_tile_pattern
  jp set_sprite_flip
  jp set_sprite_rotate
  jp set_sprite_color
  jp set_sprite_pattern

  jp usr0
  jp usr1
  jp usr2
  jp usr2_play
  jp usr2_player_status
  jp usr3
  jp usr3.COLLISION_ALL
  jp usr3.COLLISION_COUPLE
  jp usr3.COLLISION_ONE
  jp gfxVDP.set

  jp cmd_get_date
  jp cmd_get_time
  jp cmd_set_date
  jp cmd_set_time

  jp GET_NEXT_TEMP_STRING_ADDRESS

  jp MR_CALL
  jp MR_CALL_TRAP
  jp MR_CHANGE_SGM
  jp MR_GET_BYTE
  jp MR_GET_DATA
  jp MR_JUMP

  jp XBASIC_BASE
  jp XBASIC_CLS
  jp XBASIC_COPY
  jp XBASIC_COPY_FROM
  jp XBASIC_COPY_TO
  jp XBASIC_END
  jp XBASIC_INIT
  jp XBASIC_INPUT_1
  jp XBASIC_INPUT_2
  jp XBASIC_IREAD
  jp XBASIC_LOCATE
  jp XBASIC_PLAY
  jp XBASIC_PRINT_STR
  jp XBASIC_PUT_SPRITE
  jp XBASIC_READ
  jp XBASIC_RESTORE
  jp XBASIC_SCREEN
  jp XBASIC_WIDTH
  jp XBASIC_SOUND
  jp XBASIC_TAB
  jp XBASIC_USING
  jp XBASIC_USING.do
  jp XBASIC_USR

;---------------------------------------------------------------------------------------------------------
; XBASIC WRAPPER ROUTINES
;---------------------------------------------------------------------------------------------------------

; hl = heap start address
; de = temporary string start address
; ix = temporary font buffer address
XBASIC_INIT:
  xor a
  ld (TMPSTRIND), a       ; temporary string list current position
  ld (TMPSTRADDR), de     ; temporary string list start pointer

  ld (HEAPSTR), hl        ; heap start address
  ex de, hl
  ld hl, HEAPEND          ; (HIMEM)
  sbc hl, de
  ld (HEAPSIZ), hl        ; heap size

  ld (FONTADDR), ix       ; temporary font buffer address

  ld hl, (resource.map.address)
  ld (RSCMAPAD), hl       ; resource map address (copy on ram)
  ld a, (resource.map.segment)
  ld (RSCMAPSG), a        ; resource map segment number (copy on ram)

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

  call ERAFNK              ; disable function keys display
  ld hl, FNKSTR            ; configure function keys codes from 245 to 246
  ld de, FNKSTR+1
  ld a, 246
  ld b, 0
XBASIC_INIT.1:
  ld (hl), a
  inc hl
  inc de
  ld (hl), b
  ld c, 15
  ldir
  inc a
  jr nz, XBASIC_INIT.1

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

XBASIC_END:
  ld a, (SCRMOD)
  cp 2
  jr c, XBASIC_END.1       ; skip if already in text mode
    ld a, 15
    ld (FORCLR), a
    ld a, 4
    ld (BAKCLR), a
    ld (BDRCLR), a
    call INITXT            ; screen 0
XBASIC_END.1:
  call INIFNK              ; enable function keys
  call DSPFNK              ; display function keys
  xor a
  ld (SUBFLG), a
  ld (FLGINP), a
  ld (DORES), a
  ld (CONSAV), a
  ld (MAXFIL), a           ; MAXFIL - reset max files
  ld (MAXFIL), a           ; NLONLY - reset io buffers
  ld hl, 0xC033
  ld (VARTAB), hl
  ld (ARYTAB), hl
  ld (STREND), hl
  ld hl, 0x8047
  ld (DATPTR), hl
  ld hl, TEMPST
  ld (TEMPPT), hl
  ld hl, 0x0000
  ld (OLDTXT), hl
  ld (OLDLIN), hl
  ld a, (EXPTBL)
  ld h, 0x40
  jp ENASLT                ; enable basic page

; l = width size
XBASIC_WIDTH:
  ld a, (SCRMOD)          ; SCRMOD (current screen mode), OLDSCR (last text screen mode)
  cp 2
  ret nc
  ex de, hl
  ld hl, LINL40
  add a, l                ; screen 0 = LINL40 (F3AE), screen 1 = LINL32 (F3AF)
  ld l, a
  ld a, e                 ; copy parameter to A
  ld (hl), a
  ld (LINLEN), a          ; LINLEN
  sub 0x0E
  add a, 0x1C
  cpl
  inc a
  add a, e
  ld (CLMLST), a
  ; ld a, 0x0C             ; new page (clear the screen)
  ; rst 0x18               ; OUTDO - output to screen
  ld a, (SCRMOD)          ; SCRMOD (current screen mode), OLDSCR (last text screen mode)
  ld l, a
  call XBASIC_SCREEN      ; xbasic SCREEN mode (in: a, l = screen mode)
  jp XBASIC_CLS

; a, l = screen mode
XBASIC_SCREEN:
  cp 4
  jr c, XBASIC_SCREEN.TEXT_OR_GRAPH     ; skip if screen mode <= 3 (its safe for msx1)
    ld a, (VERSION)
    or a
    jr nz, XBASIC_SCREEN.TEXT_OR_GRAPH  ; skip if not MSX1 (screen mode its safe for msx2 and above)
       ld l, 2                          ; else, force screen mode 2
XBASIC_SCREEN.TEXT_OR_GRAPH:
  ld a, l
  cp 2
  rla                                   ; put carry on bit 0
  cpl
  and 1
  ld (SOMODE), a                        ; text mode (0), else graphical mode (1)
XBASIC_SCREEN.DO:
  ld a, l
  call C7369                            ; customized CHGMOD
  ld a, (VERSION)
  or a
  ret z                                 ; return if msx1
  ld ix, S.INIPLT                       ; initialize the pallete on msx2
  jp SUB_EXTROM

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

; in: ix=x, iy=y, d=color, a=sprite number, h=pattern number, b=parameters flag (b11100000)
XBASIC_PUT_SPRITE:
  bit 7, b
  call nz, SUB_SETSPRTBL_XY
  jp C71BF

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

; de=x0, hl=y0, ix=x1, iy=y1, 0xFC18=srcpg, GRPACX=x2, GRPACY=y2, 0xFC19=destpg, b=operator, FAFC=mode (screen limits)
XBASIC_COPY:
  ld a, (SOMODE)
  cp 1
  jr nz, XBASIC_COPY.TILED ; jump if not graphical mode (1)

  ld a, (VERSION)
  or a
  ret z                    ; only for MSX2

  jp BASKUN_COPY           ; BASIC KUN original COPY (DE=X0, HL=Y0, IX=X1, IY=Y1, B=operator, FCB7=XD, FCB9=YD, FC18=PS, FC19=PD)

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
  jp SUB_EXTROM               ; XBASIC original COPY FROM

XBASIC_COPY_FROM.TILED:         ; dx=x, dy=y, hl=src address
  push hl
    ld hl, (DY)
    inc hl
    ld a, (DX)
    inc a
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
  ld hl, (MV_DPTR)

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
  jp SUB_EXTROM               ; XBASIC original COPY TO

XBASIC_COPY_TO.TILED:           ; sx=x, sy=y, hl=dest address, nx=width, ny=height
  push hl
    ld hl, (SY)
    inc hl
    ld a, (SX)
    inc a
    ld h, a
    call gfxTileAddress
  pop de
  ld bc, (NX)
  inc bc
  ld a, c
  ld (de), a
  inc de
  ld a, (NY)
  inc a
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
  call CALBAS
  ei
  ret

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
  cp 0x14                       ; parameter >= 20?
  jr nc, XBASIC_BASE.GE_20
XBASIC_BASE.LE_20:
    cp 0x02
    jr nz, XBASIC_BASE.GET_NAME_TABLE
      ld a, (LINLEN)
      cp 0x29
      jr c, XBASIC_BASE.GET_NAME_TABLE
        ld hl, 0x1000
        ret
XBASIC_BASE.GET_NAME_TABLE:
    ex de, hl
      ld hl, TXTNAM
      add hl, de
      add hl, de
      ld e, (hl)
      inc hl
      ld d, (hl)
    ex de, hl
    ret
XBASIC_BASE.GE_20:
    sub 0x19
    jr nc, XBASIC_BASE.GET_TABLE_2
XBASIC_BASE.GET_TABLE_1:
      add a, 0x0F
      ld l, a
      cp 0x0D
      jr nz, XBASIC_BASE.LE_20
        ld hl,0x1E00
        ret
XBASIC_BASE.GET_TABLE_2:
    ld d, 0x00
    ld e, a
    ld hl, XBASIC_BASE.DATA_TABLE
    add hl, de
    ld h, (hl)
    ld l, d
    ret
XBASIC_BASE.DATA_TABLE:
  db 0x00, 0x00, 0x00, 0x76, 0x78, 0x00, 0x00, 0x00
  db 0x76, 0x78, 0x00, 0x00, 0x00, 0xFA, 0xF0, 0x00
  db 0x00, 0x00, 0xFA, 0xF0

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
  ld a, (DORES)                 ; DATA current segment
  or a
  jp nz, XBASIC_READ_MR
XBASIC_READ.cont:
  di
    call resource.open
      call XBASIC_GET_BUFFER
      ld hl, (DATPTR)           ; DATA current pointer
      ld de, (TMPSTRBUF)
      ld c, (hl)
      ld b, 0
      inc bc
      ldir
      ld (DATPTR), hl
    call resource.close
  ei
  ld hl, (TMPSTRBUF)
  ret

XBASIC_IREAD:
  di
    call resource.open
      ld hl, (DATPTR)          ; DATA current pointer
      ld e, (hl)
      inc hl
      ld d, (hl)
      inc hl
      ld (DATPTR), hl
      push de
    call resource.close
  ei
  pop hl
  ret

; RESTORE <number>
;   BYTE data_resource_type
;   structure for DATA (0)
;     WORD resource_items_count
;     ARRAY lines_map[resource_items_count]
;       WORD line_number
;       BYTE line_items_count
;     ARRAY lines_data[resource_items_count]
;       STRINGS item_data[line_items_count]
;   structure for CSV (1)
;     WORD resource_items_count
;     ARRAY lines_map[resource_items_count]
;       BYTE line_items_count
;     ARRAY lines_data[resource_items_count]
;       STRINGS item_data[line_items_count]
;   structure for TXT (2)
;     STRINGS item_data[...]
;   structure for IDATA (3)
;     WORD resource_items_count
;     ARRAY lines_map[resource_items_count]
;       WORD line_number
;       BYTE line_items_count
;     ARRAY lines_data[resource_items_count]
;       WORD item_data[line_items_count]
XBASIC_RESTORE:
  di
    push hl
      call resource.open
    pop bc                      ; bc = new line position
    ld hl, (DATLIN)             ; DATA map table start pointer
    ld a, (SUBFLG)              ; DATA start segment
    ld (DORES), a               ; DATA current segment
    or a
    call nz, MR_CHANGE_SGM
XBASIC_RESTORE.cont:
    ld a, (hl)
    ld (TEMP2), a               ; data resource type
    or a
    jr z, XBASIC_RESTORE.0      ; 0=DATA
    dec a
    jr z, XBASIC_RESTORE.1      ; 1=CSV
    dec a
    jr z, XBASIC_RESTORE.2      ; 2=TXT

                                ; default = IDATA

XBASIC_RESTORE.0:               ; DATA/IDATA resource
  call XBASIC_RESTORE.get_start
  push hl                       ; save data map start pointer
    add hl, de                  ; add lines_map size * resource_item_count
    add hl, de
    add hl, de
    ld (DATPTR), hl             ; save lines data start pointer
  pop hl                        ; restore data map start pointer

XBASIC_RESTORE.0.loop:          ; search on the data map for the line number required
  ld e, (hl)
  inc hl
  ld d, (hl)                    ; de = line number
  ex de, hl
  xor a
  sbc hl, bc
  ex de, hl
  jr nc, XBASIC_RESTORE.found   ; jump if current line number (hl) >= searched line number (bc)

  inc hl
  call XBASIC_RESTORE.add_item

  jr XBASIC_RESTORE.0.loop

XBASIC_RESTORE.1:               ; CSV resource
  call XBASIC_RESTORE.get_start
  push hl                       ; save data map start pointer
    add hl, de                  ; add lines_map size * resource_item_count
    ld (DATPTR), hl             ; save lines data start pointer
  pop hl                        ; restore data map start pointer

XBASIC_RESTORE.1.loop:
  ld a, b
  or c
  jr z, XBASIC_RESTORE.found

  call XBASIC_RESTORE.add_item

  dec bc

  jr XBASIC_RESTORE.1.loop

XBASIC_RESTORE.2:               ; TXT resource
  inc hl                        ; skip resource type
XBASIC_RESTORE.2.init:
  ld d, 0
XBASIC_RESTORE.2.loop:
  ld (DATPTR), hl               ; save lines data current pointer
  ld a, b
  or c
  jr z, XBASIC_RESTORE.end

  ld e, (hl)
  add hl, de                    ; add line size

  ld a, h
  cp 0xC0
  jr c, XBASIC_RESTORE.2.cont
    call XBASIC_RESTORE.next_sgm
    ld e, (hl)
    add hl, de                    ; add line size

XBASIC_RESTORE.2.cont:
  inc hl

  dec bc
  jr XBASIC_RESTORE.2.loop

XBASIC_RESTORE.next_sgm:
  ld a, (DORES)
  inc a
  inc a
  ld (DORES), a
  call MR_CHANGE_SGM
  ld hl, 0x8000
  ld (DATPTR), hl
  ret

XBASIC_RESTORE.get_start:
  ld de, 0
  ld (TEMP), de

  inc hl                        ; skip resource type

  ld e, (hl)
  inc hl
  ld d, (hl)                    ; de = lines count
  inc hl
  ret

XBASIC_RESTORE.add_item:
  ld e, (hl)                    ; a = items in the line
  ld d, 0
  inc hl
  push hl
    ld hl, (TEMP)
    add hl, de
    ld (TEMP), hl
  pop hl
  ret

XBASIC_RESTORE.found:
  ld bc, (TEMP)                 ; resource items to skip
  ld hl, (DATPTR)               ; resource items start address
  ld a, (TEMP2)
  cp 3
  jr nz, XBASIC_RESTORE.2.init

XBASIC_RESTORE.3:
  add hl, bc
  add hl, bc                    ; hl = hl + bc*2
  ld a, h
  cp 0xC0
  call nc, XBASIC_RESTORE.3.next_sgm
  ld (DATPTR), hl               ; DATA current pointer

XBASIC_RESTORE.end:
    call resource.close
  ei
  ret

XBASIC_RESTORE.3.next_sgm:
  push hl
    call XBASIC_RESTORE.next_sgm
  pop de
  ld l, e
  ret

XBASIC_READ_MR:
  ld a, (SGMADR)
  push af
    ld a, (DORES)    ; DATA current segment
    call MR_CHANGE_SGM
    call GET_NEXT_TEMP_STRING_ADDRESS
    ex de, hl
    ld hl, (DATPTR)  ; DATA current pointer
    ld b, 0
XBASIC_READ_MR.repeat:
    push de
      ld c, (hl)
      inc bc
      ldir
      ld a, h
      cp 0xC0
      jr c, XBASIC_READ_MR.cont
        call XBASIC_RESTORE.next_sgm
        pop de
        jr XBASIC_READ_MR.repeat
XBASIC_READ_MR.cont:
      ld (DATPTR), hl
    pop hl
  pop af
  jp MR_CHANGE_SGM

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

; hl = item format string
; c:de = float
XBASIC_USING:
  push bc
  push de
    ld c, 0x80           ; format style (7=1 6=, 5=* 4=$ 3=+ 2=- 1=0  0=^)
    ld de, 0x0000        ; d=thousand digits, e=decimal digits
    ld b, (hl)           ; format string size
XBASIC_USING.loop:
      inc hl
      ld a, (hl)
      cp '0'
      jr z, XBASIC_USING.zero    ; 0 numeric format char, handle
      cp '#'
      jr z, XBASIC_USING.hash    ; # numeric format char, handle
      cp '+'
      jr z, XBASIC_USING.plus    ; +, set sign flag and continue
      cp '-'
      jr z, XBASIC_USING.minus   ; -, set sign after number
      cp '.'
      jr z, XBASIC_USING.dot     ; decimal point
      cp '*'
      jr z, XBASIC_USING.mark    ; mark symbol
      cp '$'
      jr z, XBASIC_USING.curr    ; currency symbol
      cp ','
      jr z, XBASIC_USING.thou    ; thousand separator
      cp '^'
      jr z, XBASIC_USING.expo    ; exponential representation
XBASIC_USING.next:
    djnz XBASIC_USING.loop
    ld a, c
  pop hl
  pop bc
  ld b, c
XBASIC_USING.do:     ; a=format, d=thousand digits, e=decimal digits, b:hl=number
  push af
    push af
    push de
      ex af, af'
        call FloatToBCD          ; convert number to BCD (b:hl to DAC)
      ex af, af'
      and 1                      ; exponential? if bit 0, normalize DAC
      call nz, DECNRM            ; normalize
    pop bc
    pop af
    call SUB_PUFOUT              ; a=format, b=thousand digits, c=decimal digits, DAC=value, out hl=string
  pop af
  and 2                          ; zero format char?
  jr nz, XBASIC_USING.format_zero
XBASIC_USING.string_size:
  ex de, hl                      ; calculate string size
    ld l, e
    ld h, d
    xor a
    cpir
    ld a, l
    sub e
  ex de, hl
XBASIC_USING.end:
  dec hl
  dec a
  ld (hl), a
  ret
XBASIC_USING.format_zero:
  push hl
    ld b, 0
XBASIC_USING.format_zero.loop:
    ld a, (hl)
    cp 32
    jr nz, XBASIC_USING.format_zero.cont
       ld a, '0'
       ld (hl), a
XBASIC_USING.format_zero.cont:
    inc b
    inc hl
    and a
    jr nz, XBASIC_USING.format_zero.loop
    ld a, b
  pop hl
  jr XBASIC_USING.end
XBASIC_USING.plus:             ; +, set sign flag and continue
  set 3, c
  ld a, d
  or d
  jr z, XBASIC_USING.hash
XBASIC_USING.minus:            ; -, set sign after number
  set 2, c
  jr XBASIC_USING.next
XBASIC_USING.mark:             ; *, set sign flag and continue
  set 5, c
  jr XBASIC_USING.hash
XBASIC_USING.curr:             ; currency symbol
  set 4, c
  jr XBASIC_USING.hash
XBASIC_USING.thou:             ; thousand separator
  set 6, c
  jr XBASIC_USING.hash
XBASIC_USING.zero:             ; zero format char
  set 1, c
  jr XBASIC_USING.hash
XBASIC_USING.expo:             ; exponential representation
  set 0, c
XBASIC_USING.hash:             ; # numeric format char, handle
  ld a, e
  or e
  jr z, XBASIC_USING.hash.1
    inc e
    jr XBASIC_USING.next
XBASIC_USING.hash.1:
  inc d
  jr XBASIC_USING.next
XBASIC_USING.dot:              ; decimal point
  inc e
  jr XBASIC_USING.next

;---------------------------------------------------------------------------------------------------------
; CMD
;---------------------------------------------------------------------------------------------------------

; play resource with Basic standard statement
; CMD PLAY <resource number> [, <channel C: 0=off|1=on>]
cmd_play:
  ld bc, (DAC)             ; bc = resource number

  call resource.get_data

  ld a, (ARG)
  and 1
  sla a
  sla a
  ld c, a
  push bc
    ld ix, (PLAY)
    call CALBAS
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
  call resource.get_data
  ld ix, (DRAW)
  call CALBAS
  ei
  xor a
  ret

; exec as assembly code
; CMD RUNASM <resource number>
cmd_runasm:
  ld bc, (DAC)             ; bc = resource number
  call resource.get_data
  push hl
  ret                      ; indirect call

; write resource to vram address
; CMD WRTVRAM <resource number>, <vram address>
cmd_wrtvram:
  ld a, (RAMAD2)           ; test RAM on page 2
  cp 0xFF
  jr nz, cmd_wrtvram.ram_on_page_2

cmd_wrtvram.ram_on_page_3:
    call resource.copy_to_ram_on_page_3     ; out: hl = resource data, bc = size
    ld de, (ARG)
    jp LDIRVM

cmd_wrtvram.ram_on_page_2:
  di
    call resource.copy_to_ram_on_page_2
    ld de, (ARG)
    call SUB_LDIRVM
    call select_rom_on_page_2
  ei                                        ; its important because ENASLT above (select_rom_on_page_2) has interruptions disabled
  ret

; write font resource to vram tile pattern table or graphical mode
; CMD WRTFNT <resource number>
cmd_wrtfnt:
  call resource.copy_to_ram_on_page_3   ; out: hl = resource data, bc = size

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

; write a Tiny Sprite resource to vram sprite pattern and color table
; CMD WRTSPR <resource number>
cmd_wrtspr:
  push hl
    call SUB_CLRSPR
  pop hl
  call resource.copy_to_ram_on_page_3   ; out: hl = resource data, bc = size

cmd_wrtspr.do:
  ld a, (hl)
  inc hl
  or a
  jr nz, cmd_wrtspr.do.msx2

cmd_wrtspr.do.msx1:
  ld a, (SCRMOD)
  cp 4
  ret nc
  call cmd_wrtspr.do.pattern
  push hl
    xor a
    call gfxCALATR
    inc hl
    inc hl
    inc hl
  pop de
  ld a, (DAC)
cmd_wrtspr.do.msx1.loop:
  push af
    ld a, (de)
    call WRTVRM
    inc hl
    inc hl
    inc hl
    inc hl
    inc de
  pop af
  dec a
  jr nz, cmd_wrtspr.do.msx1.loop
  ret

cmd_wrtspr.do.msx2:
  ld a, (SCRMOD)
  cp 4
  ret c
  call cmd_wrtspr.do.pattern
  push hl
    xor a
    call gfxCALCOL
    ex de, hl
    ld bc, (ARG)
  pop hl
  jp LDIRVM      ; hl = ram data address, de = vram data address, bc = length

cmd_wrtspr.do.pattern:
  ld a, (hl)
  inc hl
  push hl
    ld l, a
    ld h, 0
    ld (DAC), hl
    add hl, hl  ; x2
    add hl, hl  ; x4
    add hl, hl  ; x8
    add hl, hl  ; x16
    ld (ARG), hl
    add hl, hl  ; x32
    push hl
      xor a
      call gfxCALPAT
      ex de, hl
    pop bc
  pop hl
  push hl
  push bc
    call LDIRVM      ; hl = ram data address, de = vram data address, bc = length
  pop hl
  pop bc
  add hl, bc
  ret

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

; copy resource to ram address
; CMD RSCTORAM <resource number>, <ram dest address>, <pletter: 0=no, 1=yes>
; hl = resource
; de = ram address
; a = pletter
cmd_rsctoram:
  di
    push de
    push af
      ld (DAC), hl
      call resource.open_and_get_address          ; out: hl = resource data, a = resource segment, bc = resource size
    pop af
    pop de
    or a
    jr nz, cmd_rsctoram.unpack
      ldir
      jr cmd_rsctoram.end
cmd_rsctoram.unpack:
    call resource.ram.unpack
cmd_rsctoram.end:
    call resource.close
  ei
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
  cp 4
  jr z, cmd_setfnt.default_colors.cont
    cp 2
    ret nz
cmd_setfnt.default_colors.cont:
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

; DAC = data resource number
cmd_restore:
  di
    call resource.open_and_get_address  ; out: hl = resource data, a = resource segment, bc = resource size
    ld (DATLIN), hl          ; DATA start pointer
    ld (SUBFLG), a           ; DATA segment start number
    ld (DATPTR), hl          ; DATA current pointer
    ld (DORES), a            ; DATA current segment
    ld bc, 0
  jp XBASIC_RESTORE.cont

; de = tile number
; hl = direction (0=horizontal, 1=vertical, 2=both)
set_tile_flip:
  ret

; de = tile number
; hl = direction (0=left, 1=right, 2=180 degrees)
set_tile_rotate:
  ret

; hl = tile number
; de = line number
; b = bank number (3=all)
; c = pattern data
set_tile_pattern:
  ld a, (SCRMOD)
  cp 5
  ret nc                       ; return if screen mode >= 5
  or a
  jr nz, set_tile_pattern.skip
    ld d, 0x08                 ; pattern table on screen mode 0
    ld b, 0x00                 ; only bank 0
set_tile_pattern.skip:
  add hl, hl                   ; x2
  add hl, hl                   ; x4
  add hl, hl                   ; x8
  add hl, de                   ; position in pattern table
  ld de, 0x0800
  ld a, b
  cp 3
  jr z, set_tile_pattern.all   ; put on all banks
  or a
  jr z, set_tile_pattern.do    ; put on bank 0
  add hl, de
  dec a
  jr z, set_tile_pattern.do    ; put on bank 1
  add hl, de
  jr set_tile_pattern.do       ; put on bank 2
set_tile_pattern.all:
  call set_tile_pattern.do
  add hl, de
  call set_tile_pattern.do
  add hl, de
set_tile_pattern.do:
  push af
    ld a, c
    call WRTVRM     ; hl = vram address, a = byte to write
  pop af
  ret

; hl = tile number
; de = line number (15=all)
; b = bank number (3=all)
; c = color data (FC,BC)
set_tile_color:
  ld a, (SCRMOD)
  cp 5
  ret nc                       ; return if screen mode >= 5
  or a
  ret z                        ; return if screen mode = 0
  ld d, 0x20
  cp 1
  jr nz, set_tile_color.multi
set_tile_color.uno:
    sra l                      ; /2
    sra l                      ; /4
    sra l                      ; /8
    ld h, 0x20
    jr set_tile_pattern.do
set_tile_color.multi:
  ld a, e
  cp 0xF
  jr nz, set_tile_pattern.skip
  ld a, 7
set_tile_color.multi.loop:
  call set_tile_color.multi.do
  dec a
  jr nz, set_tile_color.multi.loop
set_tile_color.multi.do:
  push af
  push hl
  push de
  push bc
    ld e, a
    call set_tile_pattern.skip
  pop bc
  pop de
  pop hl
  pop af
  ret

; de = sprite number
; hl = direction (0=horizontal, 1=vertical, 2=both)
set_sprite_flip:
  ld (TEMP2), de
  ld a, l
  or a
  jr z, set_sprite_flip.horiz
  dec a
  call nz, set_sprite_flip.horiz

set_sprite_flip.vert:
  ld a, (TEMP2)
  call set_sprite.copy
  ld hl, STRBUF
  ld de, STRBUF+32+15
  call set_sprite_flip.vert.1
  ld de, STRBUF+32+31
  call set_sprite_flip.vert.1
  ld hl, STRBUF+32
  jr set_sprite.paste
set_sprite_flip.vert.1:
  ld b, 16
set_sprite_flip.vert.2:
  ld a, (hl)
  ld (de), a
  inc hl
  dec de
  djnz set_sprite_flip.vert.2
  ret

set_sprite_flip.horiz:
  ld a, (TEMP2)
  call set_sprite.copy
  ld hl, STRBUF
  ld de, STRBUF+32
  ld b, 32
set_sprite_flip.horiz.1:
  ld a, (hl)
  call binaryReverseA
  ld (hl), a
  ld (de), a
  inc hl
  inc de
  djnz set_sprite_flip.horiz.1
  ld hl, STRBUF+16

set_sprite.paste:
  ld de, (TEMP)
  ld bc, 32
  jp LDIRVM    ; hl = ram data address, de = vram data address, bc = length (interruptions enabled)

set_sprite.copy:
  call gfxCALPAT
  ld (TEMP), hl
  ld de, STRBUF
  ld bc, 32
  jp LDIRMV      ; de = ram data address, hl = vram data address, bc = length

; reverse bits in A
; 8 bytes / 206 cycles
; http://www.retroprogramming.com/2014/01/fast-z80-bit-reversal.html
binaryReverseA:
  push bc
    ld b,8
    ld c,a
binaryReverseA.loop:
    rl c
    rra
    djnz binaryReverseA.loop
  pop bc
  ret

; de = sprite number
; hl = direction (0=left, 1=right, 2=180 degrees)
set_sprite_rotate:
  ld a, l
  cp 2
  jp z, set_sprite_flip
  push af
    ld a, e
    call set_sprite.copy
  pop af
  or a
  jr z, set_sprite_rotate.left

set_sprite_rotate.right:
  ld hl, STRBUF
  ld de, STRBUF+32+16
  call blockRotateR
  ld hl, STRBUF+8
  ld de, STRBUF+32
  call blockRotateR
  ld hl, STRBUF+16
  ld de, STRBUF+32+24
  call blockRotateR
  ld hl, STRBUF+24
  ld de, STRBUF+32+8
  call blockRotateR
  jr set_sprite_rotate.left.end

set_sprite_rotate.left:
  ld hl, STRBUF
  ld de, STRBUF+32+8
  call blockRotateL
  ld hl, STRBUF+8
  ld de, STRBUF+32+24
  call blockRotateL
  ld hl, STRBUF+16
  ld de, STRBUF+32
  call blockRotateL
  ld hl, STRBUF+24
  ld de, STRBUF+32+16
  call blockRotateL
set_sprite_rotate.left.end:
  ld hl, STRBUF+32
  jp set_sprite.paste

; binary rotate 8 bytes from HL to DE
; HL = source
; DE = destination
blockRotateL:
  ld b,8
blockRotateL.1:
    call blockRotateL.2
    ld (de), a
    inc de
  djnz blockRotateL.1
  ret
blockRotateL.2:
  push hl
  push bc
    ld b, 8
blockRotateL.loop:
    ld c, (hl)
    rr c
    rla
    ld (hl), c
    inc hl
    djnz blockRotateL.loop
  pop bc
  pop hl
  ret

blockRotateR:
  ld b,8
blockRotateR.1:
    call blockRotateR.2
    ld (de), a
    inc de
  djnz blockRotateR.1
  ret
blockRotateR.2:
  push hl
  push bc
    ld b, 8
blockRotateR.loop:
    ld c, (hl)
    rl c
    rra
    ld (hl), c
    inc hl
    djnz blockRotateR.loop
  pop bc
  pop hl
  ret

; hl = sprite number
; de = line number
; c = pattern data
set_sprite_pattern:
  ret

; hl = sprite number
; de = line number (15=all)
; c = color data (FC,BC)
set_sprite_color:
  ret

; l = mode: 0=swap, 1=wave
; e = delay #0
; c = delay #1
cmd_page:
  ld a, (SCRMOD)
  cp 5
  ret c                 ; return if screen mode < 5
  ld d, c
cmd_page.mode:
  ld a, (RG1SAV)
  and 251               ; reset b2 from r1, if mode = swapping
  sla l
  sla l
  or l                  ; set b2 from r1, if mode = waving
  ld b, a               ; change mode to swapping or waving
  ld c, 1               ; vdp(1)
  call WRTVDP           ; b = data, c = register
cmd_page.delay:
  ld a, d
  sla a
  sla a
  sla a
  sla a
  or e
  ld b, a               ; b = delay #1 (low nibble) and #2 (high nibble)
  ld c, 13              ; vdp(14)
  call WRTVDP           ; b = data, c = register
cmd_page.end:
  ld a, 1
  ld (ACPAGE), a
  jp C70CC              ; XBASIC_SET_PAGE (a=display page)

; ------------------------------------------------------------------------------------------------------
; USR() IMPLEMENTATION
; ------------------------------------------------------------------------------------------------------

; get resource data address
; in hl = parameter
; out hl = address
usr0:
  di
    push hl
      call resource.open
    pop bc
    call resource.address    ; hl = resource start address, a = segment, bc = size
    push hl
      call resource.close
    pop bc
  ei
  jp c, usr_def
  jp usr_def.exit

; get resource data size
; in hl = parameter
; out hl = size
usr1:
  di
    push hl
      call resource.open
    pop bc
    call resource.address    ; hl = address, a = segment, bc = resource size
    push bc
      call resource.close
    pop bc
  ei
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
  jp z, usr2_player_status
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

usr2_player_status:
  ld a, (PLYLOOP)
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
; hl = parameter (h: sprite 1, l: sprite 2, hl < 0: all sprites, h = 0: sprite against all the rest)
usr3:
  call usr3.test
  call usr3.return
  ld (DAC+2), hl
  ret
usr3.return:
  ld l, a              ; collisor sprite
  cp 0xFF
  jr z, usr3.return.1
    xor a              ; collision occurred
usr3.return.1:
  ld h, a
  ret

usr3.test:
  xor a
  or h                     ; check if direct test of two sprite numbers
  jp z, SUB_SPRCOL_ONE
    bit 7, h               ; check if sprite number is negative (loop all sprites if so)
    jp nz, SUB_SPRCOL_ALL
      ld e, h              ; second sprite
      jp SUB_SPRCOL_COUPLE

; no parameters
usr3.COLLISION_ALL:
  call SUB_SPRCOL_ALL
  jr usr3.return

; hl = sprite 1, de = sprite 2
usr3.COLLISION_COUPLE:
  call SUB_SPRCOL_COUPLE
  jr usr3.return

; hl = sprite 1
usr3.COLLISION_ONE:
  call SUB_SPRCOL_ONE
  jr usr3.return

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

; https://www.msx.org/wiki/Real_Time_Clock_Programming
; GET DATE
; Results:    HL = Year 1980...2079
;             D = Month (1=Jan...12=Dec)
;             E = Day (1...31)
;             A = Day of week (0=Sun...6=Sat)
cmd_get_date:
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  jr z, cmd_get_date.msx1
    ld c,12             ; year (tens)
    call SUB_REDCLK
    call cmd_clock_mult10
    ld c,11             ; year (units)
    call SUB_REDCLK
    ld bc,1980
    add a, c
    ld c, a
    add hl, bc
    push hl
      ld c, 10          ; month (tens)
      call SUB_REDCLK
      call cmd_clock_mult10
      ld c, 9           ; month (units)
      call SUB_REDCLK
      add a, l
      push af
        ld c, 8           ; day (tens)
        call SUB_REDCLK
        call cmd_clock_mult10
        ld c, 7           ; day (units)
        call SUB_REDCLK
        add a, l
      pop de
      ld e, a
    pop hl
    ld c, 6             ; week
    jp SUB_REDCLK
cmd_get_date.msx1:
  xor a
  ld h, a
  ld l, a
  ld e, a
  ld d, a
  ret

; GET TIME
; Results:    H = Hours
;             L = Minutes
;             A = Seconds
cmd_get_time:
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  jr z, cmd_get_date.msx1
    ld c, 5             ; hour (tens)
    call SUB_REDCLK
    call cmd_clock_mult10
    ld c, 4             ; hour (units)
    call SUB_REDCLK
    add a, l
    ld h, a
    push hl
      ld h, a
      ld c, 3           ; minutes (tens)
      call SUB_REDCLK
      call cmd_clock_mult10
      ld c, 2           ; minutes (units)
      call SUB_REDCLK
      add a, l
    pop hl
    ld l, a
    push hl
      ld c, 1           ; seconds (tens)
      call SUB_REDCLK
      call cmd_clock_mult10
      ld c, 0           ; seconds (units)
      call SUB_REDCLK
      add a, l
    pop hl
    ret

; SET DATE
; Input:      HL = Year 1980...2079
;             D = Month (1=Jan...12=Dec)
;             E = Day (1...31)
cmd_set_date:
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  ret z
  call cmd_clock_disable
  push de
    xor a
    ld bc,1980
    sbc hl, bc
    call cmd_clock_div10
    ld a, l
    ld c,12             ; year (tens)
    call SUB_WRTCLK
    ld a, e
    ld c,11             ; year (units)
    call SUB_WRTCLK
  pop de
  push de
    ld h, 0
    ld l, d
    call cmd_clock_div10
    ld a, l
    ld c, 10            ; month (tens)
    call SUB_WRTCLK
    ld a, e
    ld c, 9             ; month (units)
    call SUB_WRTCLK
  pop de
  ld h, 0
  ld l, e
  call cmd_clock_div10
  ld a, l
  ld c, 8               ; day (tens)
  call SUB_WRTCLK
  ld a, e
  ld c, 7               ; day (units)
  call SUB_WRTCLK
  jp cmd_clock_enable

; SET TIME
; Input:      H = Hours
;             L = Minutes
;             A = Seconds
cmd_set_time:
  ld e, a
  ld a,(VERSION)
  or a                  ; MSX2 or above?
  ret z
  call cmd_clock_disable
  push de
    push hl
      ld l, h
      ld h, 0
      call cmd_clock_div10
      ld a, l
      ld c, 5           ; hour (tens)
      call SUB_WRTCLK
      ld a, e
      ld c, 4           ; hour (units)
      call SUB_WRTCLK
    pop hl
    ld h, 0
    call cmd_clock_div10
    ld a, l
    ld c, 3             ; minutes (tens)
    call SUB_WRTCLK
    ld a, e
    ld c, 2             ; minutes (units)
    call SUB_WRTCLK
  pop hl
  ld h, 0
  call cmd_clock_div10
  ld a, l
  ld c, 1               ; seconds (tens)
  call SUB_WRTCLK
  ld a, e
  ld c, 0               ; seconds (units)
  call SUB_WRTCLK
  jp cmd_clock_enable

; multiply A by 10 (result = HL)
cmd_clock_mult10:
  ld de, 10
  ld h, d
  ld l, a
  jp XBASIC_MULTIPLY_INTEGERS

; divide HL by 10 (result = HL, remainder = DE)
cmd_clock_div10:
  ld de, 10
  jp XBASIC_DIVIDE_INTEGERS

cmd_clock_disable:
  xor a
  ld c,13               ; disable Real Time Clock
  jr SUB_WRTCLK

cmd_clock_enable:
  ld a, 8
  ld c,13               ; enable Real Time Clock
  jr SUB_WRTCLK

; Entry:	C = block number (bits 5-4) and register (bits 3-0) to read.
; Output:	A = 4 least significant bits content of the register read.
SUB_REDCLK:
  ld ix,REDCLK
  jp EXTROM

; Entry:	C = block number (bits 5-4) and register (bits 3-0).
;           A = data to write. (4 least significant bits)
SUB_WRTCLK:
  ld ix,WRTCLK
  jp EXTROM

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

; Float to BCD
; in b:hl = float
; out DAC (math pack)
FloatToBCD:
    ld a, 8           ; double value type in DAC
    ld (VALTYP), a
    ld a, h           ; get float sign (positive/negative) in 7th bit
    and 0x80
    or 0x4C
    ld (DAC), a       ; bcd exponent = 12 digits
    ld a, h
    res 7, a
    ld h, a           ; reset sign bit
    ld de, DAC+1
    ld (TEMP), de     ; save next DAC pointer
    ld c, 0x9b
    ld de, 0x3ebc     ; 100000000.00
    call FloatToBCD.eval.divide
    ld c, 0x8e
    ld de, 0x1c40     ; 10000.00
    call FloatToBCD.eval.divide
    ld c, 0x81
    ld de, 0x0000     ; 1.00
    call FloatToBCD.eval.divide
    ld c, 0x8e
    ld de, 0x1c40     ; 10000.00
FloatToBCD.eval.multiply:
    call C7748        ; multiply floats (b:hl * c:de)
    jr FloatToBCD.add
FloatToBCD.eval.divide:
    ld a, b           ; save b:hl
    push af
    push hl
      ld a, c         ; save c:de
      push af
      push de
        call C782C    ; divide floats (b:hl / c:de)
        call FloatToBCD.add
        call XBASIC_CAST_INTEGER_TO_FLOAT   ; xbasic int to float (in hl, out b:hl)
      pop de          ; restore c:de
      pop af
      ld c, a
      call C7748      ; multiply floats
      ex de, hl
      ld c, b
    pop hl            ; restore b:hl
    pop af
    ld b, a
    jp C76D3          ; subtract floats
FloatToBCD.add:
    call C7901        ; convert to integer
    push hl
      call FloatToBCD.add.1
      push af
        call FloatToBCD.add.2
      pop af
      or e
      push af
        call FloatToBCD.add.1
        push af
         call FloatToBCD.add.2
        pop af
        or e
        ld hl, (TEMP)   ; restore next DAC pointer
        ld (hl), a
      pop af
      inc hl
      ld (hl), a
      inc hl
      ld (TEMP), hl
    pop hl
    ret
FloatToBCD.add.0:
    ld de, 10
    jp C7643      ; integer divide (HL/DE, remainder = DE)
FloatToBCD.add.1:
    call FloatToBCD.add.0
    ld a, e
    ret
FloatToBCD.add.2:
    call FloatToBCD.add.0
    sla e
    sla e
    sla e
    sla e
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

; hl = register
; bc = parameter
gfxVDP.set:
  ld b, c
  ld a, l
  cp 0x08
  adc a, 0xff       ; dec a if register >= 8 (so, a = a + (-1) + carry)
  ld c, a
  jp WRTVDP

  ;ld a, (VERSION)
  ;or a
  ;jp z, WRTVDP      ; b = data, c = register
  ;ld ix, NWRVDP
  ;jp SUB_EXTROM

;---------------------------------------------------------------------------------------------------------
; MEGAROM SUPPORT CODE
; out (0x8E), a - MEGAROM SELECT TO READ
; https://www.msx.org/wiki/MegaROM_Mappers
;   ASCII 8K
;       Page (8kB)                              Switching address
;       6000h~7FFFh (mirror: E000h~FFFFh)	    6800h (mirrors: 6801h~6FFFh)
;       8000h~9FFFh (mirror: 0000h~1FFFh)       7000h (mirrors: 7001h~77FFh)
;       A000h~BFFFh (mirror: 2000h~3FFFh)       7800h (mirrors: 7801h~7FFFh)
;   Konami MegaROMs with SCC
;       8000h~9FFFh (mirror: 0000h~1FFFh)	    9000h (mirrors: 9001h~97FFh)
;       A000h~BFFFh (mirror: 2000h~3FFFh)	    B000h (mirrors: B001h~B7FFh)
;---------------------------------------------------------------------------------------------------------

Seg_P8000_SW:	 equ 0x7000 ; Segment switch on page 8000h-9FFFh (ASCII8 or Konami with SCC Mapper)
Seg_PA000_SW:	 equ 0x7800 ; Segment switch on page A000h-BFFFh (ASCII8 or Konami with SCC Mapper)
MR_TRAP_FLAG:    equ 0xFC82
MR_TRAP_SEGMS:   equ MR_TRAP_FLAG+1

MR_JUMP:
  push hl
MR_CHANGE_SGM:
  ld (SGMADR), a
  ld (Seg_P8000_SW), a
  inc a
  ld (Seg_PA000_SW), a
  dec a
  ret                       ; indirect jump

MR_CALL:
  ld bc, (SGMADR-1)         ; salve old segment number
  push bc
    call MR_CHANGE_SGM
    ld de, MR_CALL_RET      ; restore old segment code
    push de
    push hl                 ; called segment code
    ex af, af'              ; restore old registers values (call parameters)
    exx
    ret                     ; indirect call
MR_CALL_RET:
  pop af                    ; restore old segment number
  jp MR_CHANGE_SGM

MR_GET_DATA:
  ld bc, (SGMADR-1)         ; salve old segment number
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
  pop af                    ; restore old segment number
  call MR_CHANGE_SGM
  ex af, af'                ; restore old registers values (call parameters)
  exx
  ld hl, (TMPSTRBUF)        ; data restored from segment number
  ret

MR_GET_BYTE:
  ld bc, (SGMADR-1)         ; salve old segment number
  push bc
    call MR_CHANGE_SGM
    ld c, (hl)
    inc hl
    ld b, (hl)
  pop af                    ; restore old segment number
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
; parameters:
;   DAC   = first block start
;   DAC+2 = segment
;   ARG   = block count
; SCn file format:
;   db #fe 	;ID byte
;   dw {VRAM begin address}
;   dw {VRAM end address}
;   dw {not used when loading to VRAM}
XBASIC_BLOAD:
  call XBASIC_BLOAD.get_next_block
  ld a, c
  or b
  ret z

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
    call LDIRVM    ; hl = ram data address, de = vram data address, bc = length (interruptions enabled)
  pop bc
  pop hl
  add hl, bc
  push hl
    call XBASIC_BLOAD.get_next_block
  pop de
  ld a, c
  or b
  ret z

  jr XBASIC_BLOAD.loop

XBASIC_BLOAD.get_next_block:
  ld bc, 0x0000
  ld (ARG+2), bc                  ; bytes read

  ld bc, (ARG)                    ; block count
  ld a, c
  or b
  ret z

  dec bc
  ld (ARG), bc

  di
    call resource.open
      ld hl, (DAC)                ; current block address
      ld a, (DAC+2)               ; current segment
XBASIC_BLOAD.get_next_block.loop:
      or a
      call nz, MR_CHANGE_SGM

      ld a, (hl)
      or a
      jr z, XBASIC_BLOAD.get_next_block.end
        inc hl
        ld e, a
        ld d, 0
        ex de, hl
          add hl, de
          ld a, h
        ex de, hl
        cp 0xC0
        jr c, XBASIC_BLOAD.get_next_block.cont
          ld a, (DAC+2)           ; current segment
          inc a
          inc a
          ld (DAC+2), a           ; current segment
          ld hl, 0x8000
          jr XBASIC_BLOAD.get_next_block.loop
XBASIC_BLOAD.get_next_block.cont:
        ld (DAC), de
        ld de, BUF
        call resource.ram.unpack  ; in hl = packed data, de = ram destination; out bc = size, hl=destination
        ld (ARG+2), bc            ; bytes read
XBASIC_BLOAD.get_next_block.end:
    call resource.close
  ei
  ld hl, BUF
  ld bc, (ARG+2)                  ; bytes read
  ret

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

;---------------------------------------------------------------------------------------------------------
; NEW COMMANDS
;---------------------------------------------------------------------------------------------------------

; a = scroll direction (same as STRIG)
; hl = ram destination
cmd_screen_copy:
  push af
    ex de, hl
    call cmd_screen.get_start
    push de
      call LDIRMV     ; vram to ram - hl=vram, de=ram, bc=size
    pop hl
  pop af
  or a
  ret z
  cp 4
  jr z, cmd_screen_copy.4
  jr nc, cmd_screen_copy.above
  cp 2
  jr z, cmd_screen_copy.2
  jr nc, cmd_screen_copy.3
  jr cmd_screen_copy.1

cmd_screen_copy.4:  ; scroll down-right
  push hl
    call cmd_screen_copy.5
  pop hl
  jr cmd_screen_copy.3

cmd_screen_copy.2:  ; scroll up-right
  push hl
    call cmd_screen_copy.1
  pop hl

cmd_screen_copy.3:  ; scroll right
  ld a, (hl)
  push af
    ld e, l
    ld d, h
    inc hl
    ld bc, 0x02FF
    ldir
    ex de, hl
    ld a, 23
    ld c, 0x20
cmd_screen_copy.3.loop:
    push af
      ld e, l
      ld d, h
      xor a
      sbc hl, bc
      ld a, (hl)
      ld (de), a
    pop af
    dec a
    jr nz, cmd_screen_copy.3.loop
  pop af
  ld (hl), a
  ret

cmd_screen_copy.1:  ; scroll up
  ld de, BUF
  push de
    push hl
      ld bc, 0x02E0
      push bc
        add hl, bc
        ld bc, 0x0020
        push hl
          ldir
          dec hl
          ex de, hl
        pop hl
        dec hl
      pop bc
      lddr
    pop de
  pop hl
  ld c, 0x20
  ldir
  ret

cmd_screen_copy.above:
  cp 7
  jr z, cmd_screen_copy.7
  jr nc, cmd_screen_copy.8
  cp 6
  jr z, cmd_screen_copy.6

cmd_screen_copy.5:  ; scroll down
  ld de, BUF
  ld bc, 0x0020
  push bc
    push de
      push hl
        ldir
      pop de
      ld bc, 0x02E0
      ldir
    pop hl
  pop bc
  ldir
  ret

cmd_screen_copy.8:  ; scroll left-up
  push hl
    call cmd_screen_copy.1
  pop hl
  jr cmd_screen_copy.7

cmd_screen_copy.6:  ; scroll down-left
  push hl
    call cmd_screen_copy.5
  pop hl

cmd_screen_copy.7:  ; scroll left
  ld bc, 0x02FF
  add hl, bc
  ld a, (hl)
  ld e, l
  ld d, h
  dec hl
  lddr
  ex de, hl
  push af
    ld c, 0x20
    ld a, 23
cmd_screen_copy.7.loop:
    push af
      ld e, l
      ld d, h
      add hl, bc
      ld a, (hl)
      ld (de), a
    pop af
    dec a
    jr nz, cmd_screen_copy.7.loop
  pop af
  ld (hl), a
  ret

; hl = ram source
cmd_screen_paste:
  ex de,hl
    call cmd_screen.get_start
  ex de,hl
  jp LDIRVM     ; ram to vram - hl=ram, de=vram, bc=size

cmd_screen.get_start:
  ld a, (SCRMOD)
  ld hl, 0x1800   ; screen start
  ld bc, 0x0300   ; screen size (tiled only)
  or a
  ret nz
  ld h, a
  ret

; hl = resource number
cmd_screen_load:
  di
    call resource.open_and_get_address
      ld c, (hl)
      inc hl
      ld b, (hl)
      inc hl
      ld (DAC), hl            ; first block address
      ld (DAC+2), a           ; segment (megarom)
      ld (ARG), bc            ; block count
    call resource.close
  ei
  jp XBASIC_BLOAD

; https://www.msx.org/wiki/PAD()
; input l = pad function parameter code (mouse, trackball...)
; output hl
cmd_pad:
  ld a, l
  cp 8
  jr c, cmd_pad.from_bios
    ld a, (VERSION)
    or a
    ld a, l
    jr nz, cmd_pad.from_bios
cmd_pad.mouse_on_msx1:
  cp 12
  jr z, cmd_pad.mouse_on_msx1.12
  cp 13
  jr z, cmd_pad.mouse_on_msx1.13
  cp 14
  jr z, cmd_pad.mouse_on_msx1.14
  cp 16
  jr z, cmd_pad.mouse_on_msx1.16
  cp 17
  jr z, cmd_pad.mouse_on_msx1.13
  cp 18
  jr z, cmd_pad.mouse_on_msx1.14
  xor a
  jr cmd_pad.end
cmd_pad.mouse_on_msx1.12:
  ld de, 0x1310              ; mouse on port 1
cmd_pad.mouse_on_msx1.12.cont:
  call cmd_pad.GTMOUS
  ld a, l
  neg
  ld (PADY), a
  ld a, h
  neg
  ld (PADX), a
  ld a, 0xff
  jr cmd_pad.end
cmd_pad.mouse_on_msx1.16:
  ld de, 0x6C20              ; mouse on port 2
  jr cmd_pad.mouse_on_msx1.12.cont
cmd_pad.mouse_on_msx1.13:
  ld a, (PADX)
  jr cmd_pad.end
cmd_pad.mouse_on_msx1.14:
  ld a, (PADY)
  jr cmd_pad.end
cmd_pad.from_bios:
  call GTPAD
cmd_pad.end:
  ld l, a
  and 0x80
  ld h, a
  ret z
  ld h, 255
  ret

; https://www.msx.org/wiki/Mouse/Trackball#Direct_usage_of_mouse
; Routine to read the mouse by direct accesses (works on MSX1/2/2+/turbo R)
; Input: DE = 01310h for mouse in port 1 (D = 00010011b, E = 00010000b)
;        DE = 06C20h for mouse in port 2 (D = 01101100b, E = 00100000b)
; Output: H = X-offset, L = Y-offset (H = L = 255 if no mouse)
cmd_pad.WAIT1:  equ   10                ; Short delay value
cmd_pad.WAIT2:  equ   30                ; Long delay value
cmd_pad.GTMOUS:
	ld	b, cmd_pad.WAIT2	            ; Long delay for first read
	call cmd_pad.GTOFS2	                ; Read bits 7-4 of the x-offset
	and	0x0F
	rlca
	rlca
	rlca
	rlca
	ld c,a
	call cmd_pad.GTOFST	                ; Read bits 3-0 of the x-offset
	and	0x0F
	or c
	ld h,a	                            ; Store combined x-offset
	call cmd_pad.GTOFST	                ; Read bits 7-4 of the y-offset
	and	0x0F
	rlca
	rlca
	rlca
	rlca
	ld c,a
	call cmd_pad.GTOFST	                ; Read bits 3-0 of the y-offset
	and 0x0F
	or c
	ld l,a		                        ; Store combined y-offset
	ret
cmd_pad.GTOFST:
    ld b, cmd_pad.WAIT1
cmd_pad.GTOFS2:
    ld a, 15		                    ; Read PSG register 15 for mouse
	di		                            ; DI useless if the routine is used during an interrupt
	  out (0xA0),a
	  in a, (0xA1)
	  and 0x80                          ; preserve LED code/Kana state
	  or  d                             ; mouse1 x0010011b / mouse2 x1101100b
	  out (0xA1),a
	  xor e
	  ld d,a
	  call cmd_pad.WAITMS	            ; Extra delay because the mouse is slow
  	  ld a,14
	  out (0xA0),a
	ei		                            ; EI useless if the routine is used during an interrupt
	in a,(0xA2)
	ret
cmd_pad.WAITMS:
	ld a,b
cmd_pad.WTTR:
	djnz cmd_pad.WTTR
	db 0xED,0x55	                    ; back if Z80 (RETN on Z80, NOP on R800)
	rlca
	rlca
	ld b,a
cmd_pad.WTTR2:
	djnz cmd_pad.WTTR2
	ld b,a
cmd_pad.WTTR3:
	djnz cmd_pad.WTTR3
	ret


;---------------------------------------------------------------------------------------------------------
; MEMORY / SLOT / PAGE ROUTINES
;---------------------------------------------------------------------------------------------------------

resource.open:
  ld a, (RSCMAPSG)
  or a
  jr nz, select_rsc_on_megarom

select_rsc_on_page_0:
    ld a, (SLTAD2)
    ld h,000h
    call SUB_ENASLT		            ; Select the ROM on page 0000h-3FFFh (with interruptions disabled)

    ld a, (MAPPER)
    bit 0, a                        ; mapper on?
    ret z                           ; return if no mapper

      ld a, (MPRAD0)                ; original memory mapper segment for page 0 rom
      out (0xFC), a
  ret

resource.close:
  ld a, (RSCMAPSG)
  or a
  jr nz, select_rom_on_megarom

select_rom_on_page_0:
    ld a, (SLTAD0)
    ld h,000h
  jp SUB_ENASLT		                ; Select the BIOS ROM (with interruptions disabled)

select_rom_on_megarom:
  ld a, (RSCMAPT1)
  jp MR_CHANGE_SGM

select_rsc_on_megarom:
  push af
    ld a, (SGMADR)
    ld (RSCMAPT1), a
  pop af
  jp MR_CHANGE_SGM

resource.open_and_get_address:
  call resource.open
  ld bc, (DAC)                            ; bc = resource number
  jp resource.address                     ; hl = resource start address, a = segment, bc = resource size

resource.copy_to_ram_on_page_2:           ; needs di/ei
  ld a, (RSCMAPSG)                        ; test megarom
  or a
  jr z, resource.copy_to_ram_on_page_2.no_mr
resource.copy_to_ram_on_page_2.mr:
    ; 1: copy resource from megarom to ram on page 3
    call resource.open_and_get_address    ; out: hl = resource data, a = resource segment, bc = resource size
    ld de, (FONTADDR)
    push de
    push bc
      ldir
      call resource.close
    ; 2: unpack from page 3 to 2
      call select_ram_on_page_2
    pop bc
    pop hl
    ld de, 0x8000
    jp resource.ram.unpack

resource.copy_to_ram_on_page_2.no_mr:
  call select_ram_on_page_2
  call resource.open_and_get_address            ; out: hl = resource data, a = resource segment, bc = resource size
resource.copy_to_ram_on_page_2.end:
  ld de, 0x8000
resource.copy_to_ram.unpack_and_close:
  call resource.ram.unpack
  push hl
  push bc
    call resource.close
  pop bc
  pop hl
  ret

resource.copy_to_ram_on_page_3:
  di
    call resource.open_and_get_address          ; out: hl = resource data, a = resource segment, bc = resource size
    ld de, (FONTADDR)
    call resource.copy_to_ram.unpack_and_close  ; out: hl = resource data, bc = resource size
  ei
  ret

select_ram_on_page_0:
  ld a, (RAMAD0)
  ld h,000h
  jp SUB_ENASLT		; Select RAM on page 0 (with interruptions disabled)

select_rom_on_page_2:
  ld a, (SLTAD2)
  ld h,080h
  call ENASLT	    ; Select the ROM on page 8000h-BFFFh (and disable interruptions)

  ld a, (MAPPER)
  bit 0, a          ; mapper on?
  ret z             ; return if no mapper

  ld a, (MPRAD2)    ; original memory mapper segment for page 2 rom
  out (0xFE), a
  ret

select_ram_on_page_2:
  ld a, (RAMAD2)
  ld h,080h
  call ENASLT	    ; Select the RAM on page 8000h-BFFFh

  ld a, (MAPPER)
  bit 0, a          ; mapper on?
  ret z             ; return if no mapper

  ld a, (MPRAD2N)   ; new memory mapper segment for page 2 ram
  out (0xFE), a
  ret

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

    in a, (0xFC)        ; get memory mapper segment on page 0
    and c
    ld (MPRAD0), a

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
    ; change page 2 to RAM slot....
    ld a, (RAMAD2)
    ld h, 0x80
    call SUB_ENASLT

    ld a, (Seg_P8000_SW)     ; save original byte there
    ld (TEMP), a

    ; change page 2 slot to same slot of page 1
    ld a, (SLTAD1)
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
    call SUB_ENASLT

    ld a, (TEMP)
    ld (Seg_P8000_SW), a        ; restore original byte in RAM

    ld a, 0xFF
    ld (RAMAD2), a              ; RAM on page 2 already used (not usable)

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
; ENASLT alternative for use of ram on page 0 (with interruptions disabled)
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

; VDP wait
;VDP_wait:
;    ld a, 2
;    call VDP_wait.1
;    and 1
;    jr nz, VDP_wait
;    xor a
;VDP_wait.1:
;    di
;      out (0x99), a
;      ld a, 0x8F
;      out (0x99), a
;      in a, (0x99)
;    ei
;    ret


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
; Resource map structure (starting at 0010h on rom memory):
;   word resource_count
;   array resources[]
;      word resource_pointer
;      word resource_size
;---------------------------------------------------------------------------------------------------------

; out: de = resource count
;      hl = start of resource map pointers
resource.count:
  ld hl, (RSCMAPAD)
  ld e, (hl)
  inc hl
  ld d, (hl)
  inc hl
  ret

; in:  bc = resource number
; out: hl = resource address
;      bc = resource size
;      a  = segment
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
    add hl, hl  ; x 2
    add hl, hl  ; x 4
    add hl, bc  ; x 5
  ex de, hl

  add hl, de

  xor a       ; clear carry
  ld e, (hl)  ; get resource address
  inc hl
  ld d, (hl)
  inc hl
  ld a, (hl)  ; get segment number
  inc hl
  ld c, (hl)  ; get resource size
  inc hl
  ld b, (hl)

  or a
  call nz, MR_CHANGE_SGM

  ex de, hl
  ret

; in:  bc = resource number
; out: hl = data pointer
resource.get_data:
  di
    push bc
      call resource.open
    pop bc

    call resource.address    ; hl = resource start address, a = segment, bc = size

    ld de, BUF
    ld bc, 255
    ldir

    call resource.close
  ei
  ld hl, BUF
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

;---------------------------------------------------------------------------------------------------------
; SPRITE COLLISION SUPPORT ROUTINES
; reference:
; https://www.msx.org/forum/development/msx-development/sprite-collision-detection-or-manually-calculation-sprite-coordina
;---------------------------------------------------------------------------------------------------------

;
; customized clear sprite
;
SUB_CLRSPR:
  call SUB_GetSpriteSize
  ld (SPRSIZ), a
  ld hl, SPRTBL
  xor a
  ld (hl), a
  ld e, l
  ld d, h
  inc de
  ld bc, 32*5 - 1    ; 32 sprites * (test, x0, x1, y0, y1)
  ldir
  jp CLRSPR

;
; get sprite XY
; in a = sprite number
; out hl = sprite data address
;
SUB_GETSPRTBL_ADDR:
  push de
    ld e, a
    add a, a              ; a * 4
    add a, a
    add a, e              ; + a
    ld h, 0
    ld l, a
    ld de, SPRTBL
    add hl, de
  pop de
  ret

;
; set sprite XY
; a = sprite number
; ix = x
; iy = y
;
SUB_SETSPRTBL_XY:
  push ix
  push iy
  exx
    ld c, a              ; save a
      call SUB_GETSPRTBL_ADDR
      ld a, 0xff
      ld (hl), a         ; test flag (0=notest, ff=test)
      inc hl
      pop de             ; de = ix
      ld (hl), e         ; x0
      inc hl
      ld a, (SPRSIZ)
      ld d, a            ; save a
        add a, e
        ld (hl), a       ; x1 = x0 + sprite size
        inc hl
      ld a, d            ; restore a
      pop de             ; de = iy
      ld (hl), e         ; y0
      inc hl
      add a, e
      ld (hl), a         ; y1 = y0 + sprite size
    ld a, c              ; restore a
  exx
  ret

;
; set sprite test flag
; a = sprite number
; l = flag
;
SUB_SETSPRTBL_TEST:
  ex de, hl
  call SUB_GETSPRTBL_ADDR
  ld (hl), e         ; set test flag (0=notest, ff=test)
  ret

;
; load sprite data
; in a = sprite number
;
SUB_SPRCOL_LOAD:
  call SUB_GETSPRTBL_ADDR
  ld a, (hl)
  or a                   ; sprite is active?
  ret z

  inc hl
  ld e, (hl)             ; ax0
  inc hl
  ld d, (hl)             ; ax1
  inc hl
  ld c, (hl)             ; ay0
  inc hl
  ld b, (hl)             ; ay1
  ret

;
; test a sprite collision against all the rest
; out a (0xFF = no sprite, or collisor sprite)
;
SUB_SPRCOL_ALL:
  xor a

SUB_SPRCOL_ALL.LOOP:
  exx
    ld b, a       ; save current sprite
  exx
  call SUB_SPRCOL_ONE.1
  cp 0xFF
  ret nz

  exx
    ld a, b       ; restore current sprite
  exx
  inc a
  cp 32
  jr nz, SUB_SPRCOL_ALL.LOOP

  jr SUB_SPRCOL_CHECK.false

;
; test a sprite collision against all the rest
; in  hl = sprite
; out a (0xFF = no, or sprite 1 if collided)
;
SUB_SPRCOL_ONE:
  ld a, l         ; parameter sprite
  exx
    ld b, a       ; save parameter sprite
  exx
SUB_SPRCOL_ONE.1:
  call SUB_SPRCOL_LOAD
  jr z, SUB_SPRCOL_CHECK.false

  xor a
  call SUB_GETSPRTBL_ADDR

SUB_SPRCOL_ONE.LOOP:
  exx
    cp b          ; compare current sprite against parameter sprite
  exx
  jr z, SUB_SPRCOL_ONE.SKIP    ; skip if same sprite
    exx
      ld c, a                  ; save current sprite
    exx
    call SUB_SPRCOL_CHECK
    cp 0xFF
    ret nz

    exx
      ld a, c                  ; restore current sprite
    exx
    jr SUB_SPRCOL_ONE.CONT

SUB_SPRCOL_ONE.SKIP:
  inc hl
  inc hl
  inc hl
  inc hl
  inc hl

SUB_SPRCOL_ONE.CONT:
  inc a
  cp 32
  jr nz, SUB_SPRCOL_ONE.LOOP

  jr SUB_SPRCOL_CHECK.false

;
; test two sprites collision
; in  hl = sprite 1
;     de = sprite 2
; out a (0xFF = no, or sprite 1 if collided)
;
SUB_SPRCOL_COUPLE:
  ; save sprite 2
  ld a, l
  exx
    ld c, a
  exx
  ; sprite 1
  ld a, e
  call SUB_SPRCOL_LOAD
  jr z, SUB_SPRCOL_CHECK.false

  ; restore sprite 2
  exx
    ld a, c
  exx
  call SUB_GETSPRTBL_ADDR

; e=ax0, d=ax1, c=ay0, b=ay1
SUB_SPRCOL_CHECK:
  ; test if sprite is active
  ld a, (hl)
  or a                   ; 0xFF = active
  jr z, SUB_SPRCOL_CHECK.skip_5
  inc hl

  ; test if ax1 > bx0 and ax0 < bx1 and ay1 > by0 and ay0 < by1
  ld a, (hl) ; bx0
  cp d       ; ax1
  jr nc, SUB_SPRCOL_CHECK.skip_4
  inc hl

  ld a, e    ; ax0
  cp (hl)    ; bx1
  jr nc, SUB_SPRCOL_CHECK.skip_3
  inc hl

  ld a, (hl) ; by0
  cp b       ; ay1
  jr nc, SUB_SPRCOL_CHECK.skip_2
  inc hl

  ld a, c    ; ay0
  cp (hl)    ; by1
  jr nc, SUB_SPRCOL_CHECK.skip_1
  inc hl

SUB_SPRCOL_CHECK.true:
  ; if ok, return collider sprite (c)
  exx
    ld a, c
  exx
  ret

SUB_SPRCOL_CHECK.skip_5:
  inc hl
SUB_SPRCOL_CHECK.skip_4:
  inc hl
SUB_SPRCOL_CHECK.skip_3:
  inc hl
SUB_SPRCOL_CHECK.skip_2:
  inc hl
SUB_SPRCOL_CHECK.skip_1:
  inc hl

SUB_SPRCOL_CHECK.false:
  ld a, 0xFF
  ret

;
; get sprite size
;
SUB_GetSpriteSize:
  push bc
    ld bc, 0x0808
    ld a, (RG1SAV)  		; bit 0 = double size, bit 1 = sprite size (0=8 pixels, 1=16 pixels)
    bit 1, a
    jr z, SUB_GetSpriteSize.1
      ld bc, 0x1010

SUB_GetSpriteSize.1:
    bit 0, a
    jr z, SUB_GetSpriteSize.2
      sll b

SUB_GetSpriteSize.2:
    ;ld (ARG), bc
    ld a, c
  pop bc
  ret

;---------------------------------------------------------------------------------------------------------
; RESOURCE DATA
;---------------------------------------------------------------------------------------------------------

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
FNKSTR  EQU 0F87FH   ; 160
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

;PAINT_FIX.1:              ; same as PNTINI on bios
;    LD	(BRDATR),A
;    ld a, (SCRMOD)
;    cp 2                  ; screen 2?
;    jr z, PAINT_FIX.1.1
;    cp 4                  ; screen 4?
;    ret nz
;PAINT_FIX.1.1:
;      ld a, b
;      ld (BRDATR), a      ; border color = paint color
;      ret

PAINT_FIX.2:
    push af
      ld a, (VERSION)
      cp 3                ; MSX Turbo R?
      jr c, PAINT_FIX.2.1
         ld a, (BASVER)
         and 0xF0
         or a
         jr z, PAINT_FIX.2.1    ; jump if japanese TurboR
           inc ix
           inc ix
           inc ix
           inc ix
           inc ix
PAINT_FIX.2.1:
    pop af
	;jp	SUB_EXTROM			; PAINT handler (subrom) ===> see it below

; EXTROM alternative with interruption enabled

SUB_EXTROM:
  call EXTROM
  ei
  ret

; SUB_PUFOUT - a=format, b=thousand digits, c=decimal digits, DAC=value, out hl=string

SUB_PUFOUT:
  ld hl, PUFOUT                ; a=format, b=thousand digits, c=decimal digits, DAC=value, out hl=string
  ld ix, 0x4D86                ; jp (hl) - workaround to do MATH PACK interslot call
  call CALBAS
  ei
  ret

;---------------------------------------------------------------------------------------------------------
; FIXED ADDRESS ROUTINES
;---------------------------------------------------------------------------------------------------------

BASIC_KUN_START_FILLER:
    DEFS	06BF7H-34-1876-$,000H     ; 34 = J_* functions size, 1876 = arkosplayer.bin size

ARKOS_PLAYER:
    INCBIN "arkos/arkosplayer.bin"

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

;     WriteParamBCD
;	  Subroutine write a float in BCD double parameter form (Basic Interpreter)
;	     Inputs  b:hl = float, de = start of basic line to write the parameter
;	     Outputs hl = next position in basic line

WriteParamBCD:
        push de
          call FloatToBCD
          call DECNRM  ; normalize
        pop hl
        ld (hl), 0x1D  ; BCD single marker
        inc hl
        ex de, hl
          ld hl, DAC
          ld bc, 4
          ldir
        ex de, hl
        ret            ; 22 bytes

  nop
  nop
  nop
  nop

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
;	     Inputs  DE=X0, HL=Y0, IX=X1, IY=Y1, B=operator, FCB7=XD, FCB9=YD, FC18=PS, FC19=PD, FAFC=mode (screen limits)
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
	JP	SUB_CLRSPR

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

C74BC:	CALL PNTINI   ;CALL PAINT_FIX.1
	LD	A,B
	LD	(ATRBYT),A
	LD	C,E
	LD	B,D
	EX	DE,HL
	LD	A,(SCRMOD)
	CP	5			; pattern screen mode ?
	JP	C,J74D5			; yep, use MSX1 paint handler
	LD	IX,S266E
	JP	PAINT_FIX.2         ; EXTROM			; PAINT handler (subrom)

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
J74EA:
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

C7508:	CALL	XBASIC_CAST_INTEGER_TO_STRING			; integer to text
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

;     XBASIC_CAST_INTEGER_TO_STRING
;	  Subroutine integer to text
;	     Inputs  HL (integer)
;	     Outputs HL (string pointer)

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





