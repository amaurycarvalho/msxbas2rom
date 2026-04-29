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

S.INIPLT	EQU	0141H      ; msx 2 (subrom) - initialize palette (copy from ROM to VRAM)
S.RSTPLT	EQU	0145H      ; msx 2 (subrom) - restore palette (copy from VRAM to VDP)
S.GETPLT	EQU	0149H      ; msx 2 (subrom) - get palette color (in: A color number, out: high B red, low B blue, low C green)
S.SETPLT	EQU	014DH      ; msx 2 (subrom) - set palette color (in: D color number, out: high A red, low A blue, low E green)

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
ONEFLG:       equ 0xF6BB     ; 1 - error flag (0=not in ERROR handler routine)
CURLIN:       equ 0xF41C     ; 2 - current line (BASIC interpreter, FFFF=direct mode)
PRTFLG:       equ 0xF416     ; output to screen (0=true)
PTRFLG:       equ 0xF6A9     ; line number converted to pointer (0=false)
DORES:        equ 0xF664     ; 1 - DATA flag to ASCII format
PRMFLG:       equ 0xF7B4
CONSAV:       equ 0xF668     ; numeric token used by CHRGTR

DSKDIS:       equ 0xFD99     ; disable disks (DEVICE, 0xFF=yes)
DFTDRV:       equ 0xF247     ; 1 - default drive
DIRDRV:       equ 0xF246     ; 1 - current directory drive
SUBFLG:       equ 0xF6A5     ; 1 (0=simple variable, not 0 = array)
MAXFIL:       equ 0xF85F     ; 1 - maxfiles
FILTAB:       equ 0xF860     ; 2 - address of the pointer table for the I/O buffer FCBs
NULBUF:       equ 0xF862     ; 2 - address of the first byte of the data buffer belonging to I/O buffer 0
PTRFIL:       equ 0xF864     ; 2 - address of the currently active I/O buffer FCB
FILNAM:       equ 0xF866     ; 11 - buffer to hold an user-specified filename
FILNM2:       equ 0xF871     ; 11 - buffer to hold a filename read from an I/O device for comparison with the contents of FILNAM
DRVTBL:       equ 0xFB21     ; drive table (2 bytes per drive)
BDOSBOTTOM:   equ 0xF34B     ; msxdos system bottom
SECBUF:       equ 0xF34D     ; 2 - current drive FAT copy address 
BUFFER:       equ 0xF34F     ; 2 - DTA buffer address (512 bytes)
DIRBUF:       equ 0xF351     ; 2 - disk sector transfer area address (512 bytes, used by DSKI$ and DSKO$)
SPTEMP:       equ 0xF304     ; SP register address copy
DSKERR:       equ 0xF302     ; disk error handler address copy 
FCBBASE:      equ 0xF353     ; File Control Block base
DPBLIST:      equ 0xF355     ; Drive Parameter Block list
DTAADDR:      equ 0xF23D     ; Disk Transfer Area address
FLGINP:       equ 0xF6A6     ; used by INPUT/READ (0=INPUT, not 0=READ)
STMTKTAB:     equ 0x392E     ; addresses of BASIC statement token service routines (start from 081H to 0D8H)
FNCTKTAB:     equ 0x39DE     ; addresses of BASIC function token service routines
DISK_ERROR_HANDLER:  equ 0xF323  ; disk error handler address (pointer to pointer)
ABORT_ERROR_HANDLER: equ 0xF1E6  ; abort error handler address

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
STKTOP: equ 0xF674  ; 2 - stack top address used by BASIC interpreter
SAVSTK: equ 0xF6B1  ; 2 - current stack position address (used by error handling and RESUME statement) 
MEMSIZ: equ 0xF672  ; 2 - top memory address which can be used by BASIC interpreter
FRETOP: equ 0xF69B  ; 2 - next free position address to BASIC's string area 
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

STARTUP_CFG_FILEIO:     equ 0x800A     ; disk mode flag
resource.map.address:   equ 0x800B     ; resource map address
resource.map.segment:   equ 0x800D     ; resource map segment
BASTEXT:                equ 0x800E     ; address of user basic code
