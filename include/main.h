#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include "lex.h"
#include "tokenizer.h"
#include "rom.h"
#include "compiler.h"
#include "compiler_pt3.h"

#define app_version "0.3.1.9"

char inputFilename[255]="",
     outputFilename[255]="",
     symbolFilename[255]="";

bool parmHelp=false,  parmDebug=false,
     parmQuiet=false, parmError=false,
     parmVer=false, parmTurbo=false,
     parmDoc=false, parmXtd=false,
     parmNoStripRemLines=false, parmSymbols=false,
     parmCompile=false, parmLineNumber=false;

bool FileExists(char *filename);
bool SaveSymbolFile(Compiler *compiler, int code_start);
bool SaveSymbolFile(CompilerPT3 *compiler, int code_start);

const char * info_splash = R"(MSXBAS2ROM - MSX BASIC TO ROM COMPILER
Created by Amaury Carvalho (2020-2023)
Version:)";

const char * info_support = R"(
Help us to maintain this project, support us on Patreon:
https://www.patreon.com/msxbas2rom)";

const char * info_usage = R"(
Usage: msxbas2rom [options] <filename.bas>
Options:
       -q  = quiet (no verbose)
       -d  = debug mode (show details)
       -t  = turbo mode (or use CALL TURBO instructions)
       -c  = compile mode
       -x  = extended memory scheme mode (MegaROM, compile mode)
       -s  = generate symbols for OpenMSX debugger (compile mode)
       -h or -? = help
       --doc = display documentation
       --ver = display version history
       --nsr = no strip remark lines (tokenized/turbo mode)
       --lin = register line numbers (compile mode)
Output: <filename.rom>

See more information at:
https://github.com/amaurycarvalho/msxbas2rom )";

/*
0.0.0.0 – (2023/xx/xx) RUN support to call sub-programs;
0.0.0.0 – (2023/xx/xx) Disk file support;
0.0.0.0 – (2023/xx/xx) Better support to double precision type (BCD);
0.0.0.0 – (2023/xx/xx) SET TILE FLIP/ROTATE new command;
0.0.0.0 – (2023/xx/xx) SET SPRITE COLOR/PATTERN new command;
*/

const char * info_history = R"(
Version history

0.3.1.9 – (2023/11/02) CMD PAGE new command (Cadari bit);
0.3.1.8 – (2023/10/29) WIDTH statement optimization;
0.3.1.7 – (2023/10/28) VDP function optimization (Cadari bit);
0.3.1.6 – (2023/10/16) Mouse support on MSX1 via PAD function;
0.3.1.5 – (2023/09/24) Integer multiply in MegaROM bug fix;
0.3.1.4 – (2023/09/10) Multipling constant integers bug fix;
0.3.1.3 – (2023/09/02) CMD RSCTORAM new command;
0.3.1.2 – (2023/08/27) SET SPRITE ROTATE new command;
0.3.1.1 – (2023/08/26) IPEEK()/IPOKE new commands;
0.3.1.0 – (2023/08/19) SET SPRITE FLIP new command;
0.3.0.9 – (2023/07/03) LINE parameters parse bug fix;
0.3.0.8 – (2023/06/24) BASE function bug fix;
0.3.0.7 – (2023/06/23) DATA last element parameter bug fix;
0.3.0.6 – (2023/06/22) MegaROM resource allocation bug fix;
0.3.0.5 – (2023/06/07) Integer division optimization bug fix;
0.3.0.4 – (2023/05/25) PAINT stmt on WebMSX USA TurboR bug fix;
0.3.0.3 – (2023/05/25) COPY stmt operator parameter bug fix;
0.3.0.2 – (2023/05/24) COPY stmt and SYMBOLs file (var address) bug fix;
0.3.0.1 – (2023/05/23) COPY screen data in tiled mode bug fix;
0.3.0.0 – (2023/02/17) TURBO function bug fix, integer math optimizations;
0.2.9.9 – (2023/01/20) SET FONT new command;
0.2.9.8 – (2022/12/21) MegaROM mode support for CMD, RESOURCES and
                       Arkos Tracker player;
0.2.9.7 – (2022/07/20) COLOR NEW/RESTORE bug fix on MSX1;
0.2.9.6 – (2022/07/15) Tokenized THEN/GOTO bug fix,
                       SET ADJUST/SCREEN/BEEP/TITLE/PROMPT support;
0.2.9.5 – (2022/05/23) SCREEN COPY/PASTE, SCREEN SCROLL,
                       SCREEN LOAD (.SCn as resources),
                       SPRITE LOAD (CMD WRTSPR alternative);
0.2.9.4 – (2022/04/25) Several bug fixes: try..catch exceptions,
                       tokenized source code input detection,
                       DATA/RESTORE misbehaviour;
0.2.9.3 – (2022/04/05) IRESTORE new command (for IREAD binary files),
                       RESOURCESIZE function;
0.2.9.2 – (2022/01/30) Support for TinySprite backup files with BLOAD
                       and CMD WRTSPR, BLOAD for screen files bug fix;
0.2.9.1 – (2022/01/05) LINE negative parameter bug fix;
0.2.9.0 – (2021/12/03) SET TILE ON/OFF/COLOR/PATTERN new commands;
0.2.8.9 – (2021/11/03) Expression evaluator of TIME/INKEY bug fix,
                       undeclared array syntax error;
0.2.8.8 – (2021/10/14) Hexa constant bug fix, RESUME support (tokenized
                       mode only);
0.2.8.7 – (2021/10/13) TAB function token bug fix (tokenized mode), MOD
                       and \ bug fix for single and double types,
                       numeric constant type identification bug fix;
0.2.8.6 – (2021/10/12) "0" as format string on USING$() for printing
                       left zeros;
0.2.8.5 – (2021/10/11) SWAP bug fix 2, math operations parameters
                       optimization 2;
0.2.8.4 – (2021/10/08) SWAP bug fix, math operations parameters
                       optimization, CMD PLYLOOP bug fix, PLYSTATUS();
0.2.8.3 – (2021/10/05) PRINT USING bug fix and optimization;
0.2.8.2 – (2021/09/30) CIRCLE full parameters;
0.2.8.1 – (2021/09/28) PSET parameters bug fix, SCREEN full parameters
                       support, SET VIDEO support, COPY SCREEN support;
0.2.8.0 – (2021/09/24) Sprite collision optimization, COLOR without
                       parameters bug fix, COPY parameter bug fix,
                       CMD WRTVRAM hdw/int bug fix;
0.2.7.9 – (2021/09/21) Running on SofaRun bug fix;
0.2.7.8 – (2021/09/10) PRINT USING support for numeric values, USING$()
                       new function, F1..F10 function keys return ASCII
                       codes 246 to 255 respectively, END stmt bug fix;
0.2.7.7 – (2021/08/31) PAINT bug fix (screen 2 border color must be equal
                       to filler color), TXT/CSV files resources support
                       (CMD RESTORE), DATA statements now allocated in
                       resources page;
0.2.7.6 – (2021/07/20) Float and String array index access optimization,
                       COLLISION(<n1>,<n2>);
0.2.7.5 – (2021/07/16) Integer array index access optimization and integer
                       FOR/NEXT optimization;
0.2.7.4 – (2021/06/17) AKM fix to playing at same speed in 50hz/60hz;
0.2.7.3 – (2021/06/16) Arkos Tracker 2 player support (PT3 deprecated);
0.2.7.2 – (2021/05/07) Bug fix pack (duplicated line error in FOR..NEXT block);
0.2.7.1 – (2021/05/05) Bug fix pack (setfnt default color), CMD UPDFNTCLR;
0.2.7.0 – (2021/05/01) CMD CPU, CPU(), MAKER(), VDP() without parameters;
0.2.6.9 – (2021/04/29) Bug fix pack (LOCATE), several code size optimizations,
                       IDATA/IREAD implementation;
0.2.6.8 – (2021/04/27) Change font in graphical mode (CMD WRTFNT and
                       CMD SETFNT); initialization, data/heap pointers and
                       USR() code otimization;
0.2.6.7 – (2021/04/25) NTSC() function, buf fix pack (LINE, PRINT ';' in tile
                       mode);
0.2.6.6 – (2021/04/22) SYMBOLs file create for use on OpenMSX debugger
                       (-s option);
0.2.6.5 – (2021/04/16) OPEN 'GRP' otimization and LOCATE/PRINT in graphical
                       mode;
0.2.6.4 – (2021/04/07) MegaROM mode bug fix (running with SofaRun with
                       MegaRAM);
0.2.6.3 – (2021/04/07) PT3/MegaROM mode bug fix (running with ExecROM with
                       MegaRAM);
0.2.6.2 – (2021/04/05) PT3 bug fix (running in 64kb RAM with ExecROM without
                       mapper, ex: Hotbit w/disk driver, Sony HB-F500,
                       Victor HC-95, Toshiba HX-23F...);
0.2.6.1 – (2021/03/30) PT3 player bug fix (32kb RAM machines, ex: Pioneer PX7);
0.2.6.0 – (2021/03/29) Compatibility bug fix pack (hotbit);
0.2.5.9 – (2021/03/12) Precomp/MegaROM: BLOAD;
0.2.5.8 – (2021/03/10) Bug fix pack (OPEN, PAINT, temporary strings),
                       line number option;
0.2.5.7 – (2021/03/09) MegaROM: RETURN <line>, ON..., R800 cpu (Turbo-R)
                       math code support;
0.2.5.6 – (2021/03/03) Precompilation: bug fix pack (LINE INPUT, CLEAR,
                       RAM size), MegaROM: DATA, READ, RESTORE;
0.2.5.5 – (2021/03/02) Experimental MegaROM option (GOTO, GOSUB, RETURN,
                       FOR/NEXT), MSX 1 BIOS bug fix (CALSLT to
                       page 0: Expert XP-800);
0.2.5.4 – (2021/02/24) Precomp: bug fix pack (PT3);
0.2.5.3 – (2021/02/23) Precomp: bug fix pack (TAB), start compiled program
                                at screen 0;
0.2.5.2 – (2021/02/22) Precomp: bug fix pack (USR), code size optimization;
0.2.5.1 – (2021/02/18) Precomp: bug fix pack (BASE, PAINT, PSET), binary
                                code size optimization;
0.2.5.0 – (2021/02/16) Precomp: DATA/USR bug fix, PSG, partial file support
                                (EOF, INPUT#, OPEN);
0.2.4.9 – (2021/02/16) Precomp: bug fix pack (DEF, DIM, functions, DATA, LINE,
                                OPEN);
0.2.4.8 – (2021/02/12) Precomp: COLOR=NEW, COLOR=RESTORE, COLOR=(),
                                COLOR SPRITE()=, COLOR SPRITE$()=, OPEN, CLOSE,
                                PRINT#, DEF USR, USR, MAXFILE, CMD CLIP;
0.2.4.7 – (2021/01/25) Precomp: PUT TILE, TILE, MSX;
0.2.4.6 – (2021/01/21) Precomp: COPY .. TO HEAP / COPY HEAP TO ...;
0.2.4.5 – (2021/01/21) Precomp: SET PAGE, SET SCROLL, COPY;
0.2.4.4 – (2021/01/19) Precomp: PUT SPRITE fix, RESTORE fix;
0.2.4.3 – (2021/01/18) Precomp: array pointers optimization;
0.2.4.2 – (2021/01/18) Precomp: logical operations size optimization;
0.2.4.1 – (2021/01/17) Precomp: CMD and internal USR bug fix;
0.2.4.0 – (2021/01/16) Precomp: CMD and refactoring memory scheme;
0.2.3.9 – (2021/01/15) Precomp: ON INTERVAL/SPRITE/STRIG/KEY/STOP;
0.2.3.8 – (2021/01/14) Precomp: PUT SPRITE, SPRITE$, ON...GOTO/GOSUB and
                                related;
0.2.3.7 – (2021/01/12) Precomp: DATA/READ and special functions;
0.2.3.6 – (2021/01/12) Precomp: graphical stmts (CIRCLE, LINE, PAINT, DRAW);
0.2.3.5 – (2021/01/11) Precomp: graphical stmts (COLOR, WIDTH, SCREEN,
                                PSET, PRESET);
0.2.3.4 – (2021/01/10) Precomp: FOR/NEXT;
0.2.3.3 – (2021/01/09) Precomp: IF/THEN/ELSE, IF/GOTO/ELSE, IF/GOSUB/ELSE;
0.2.3.2 – (2021/01/08) Precomp: string functions;
0.2.3.1 – (2021/01/06) Precomp: logical expressions and numeric functions;
0.2.3.0 – (2021/01/01) Precompilation kernel;
0.2.2.0 – (2020/10/28) PT3 joystick bug fixed; Locate/Print commands in screen
                       mode 2 bug fixed; strip 'remark only' lines;
                       ? and _ commands fixed;
0.2.1.0 – (2020/10/13) Sprite collision support (USR3 function), replay a PT3
                       (CMD PT3REPLAY);
0.2.0.0 – (2020/08/19) Built-In fonts support implemented (CMD SETFNT,
                       CMD PT3LOOP);
0.1.9.0 – (2020/08/02) Emulator / Hardware ROM loading bugs fixed;
0.1.8.0 – (2020/06/14) CALL TURBO ON/OFF bug fixed;
0.1.7.0 – (2020/06/13) CMD 2th parameter, SPC(), CHR$(), STRING$(),
                       eol (Windows) bugs fixed;
0.1.6.0 – (2020/06/12) CMD WRTFNT to load compressed .ALF files;
0.1.5.0 – (2020/06/12) Windows version minor bugs fixed;
0.1.4.0 – (2020/06/03) PT3TOOLS support;
0.1.3.0 – (2020/05/28) INCLUDE, WRTSCR, WRTCLR, WRTCHR, WRTSPRPAT, CMD DRAW
                       implemented;
0.1.2.0 – (2020/05/27) Resources initial implementation (TEXT, CMD PLAY);
0.1.1.0 – (2020/05/24) Turbo mode implemented (xbasic);
0.1.0.0 – (2020/05/21) Proof of concept.
)";

const char * info_documentation = R"(
COMPILING CODE

   Use -c parameter to compile your BASIC code into ROM.

   Source code must be in plain text (ASCII) format.

   Statements supported:

     DEFDBL, DEFINT, DEFSNG, DEFSTR, DIM,
     REM, END, CLS, BEEP, PRINT, INPUT, LINE INPUT,
     GOTO, GOSUB, RETURN, SOUND, PLAY,
     RANDOMIZE, LET, FOR/NEXT, DATA, READ, RESTORE,
     IF/THEN/ELSE, IF/GOTO/ELSE, IF/GOSUB/ELSE,
     LOCATE, COLOR, SCREEN, WIDTH, PSET, PRESET,
     LINE, CIRCLE, PAINT, DRAW, OUT, POKE, VPOKE
     SPRITE$, PUT SPRITE, ON...GOSUB, ON...GOTO,
     SWAP, WAIT, ON INTERVAL GOSUB, INTERVAL ON/OFF,
     ON KEY GOSUB, KEY ON/OFF, KEY() ON/OFF,
     ON STRIG GOSUB, STRIG() ON/OFF,
     ON SPRITE GOSUB, SPRITE ON/OFF,
     STOP, STOP ON/OFF, ON STOP GOSUB, CMD,
     SET SCROLL, SET PAGE, COPY, PUT TILE,
     SET TILE COLOR, SET TILE PATTERN,
     OPEN, CLOSE, MAXFILES, DEF USR, BLOAD,
     PRINT USING, SET VIDEO, COPY SCREEN

     [RUN]

   Functions supported:
     INT, FIX, RND, SIN, COS, TAN, ATN, EXP,
     LOG, SQR, VAL, TIME, SGN, ABS, ASC, INKEY,
     LEN, CSNG, CDBL, CINT, CHR$, INKEY$, INPUT$,
     SPACE$, SPC, TAB, STRING$, STR$, LEFT$, RIGHT$, MID$,
     INSTR, OCT$, HEX$, BIN$, PEEK, VPEEK, INP, POS, LPOS,
     CSRLIN, STRIG, STICK, PDL, PAD, BASE, VARPTR,
     PLAY, VDP, POINT, COLLISION, FRE, HEAP, TILE,
     RESOURCE, USR, PSG, MSX, NTSC, TURBO, MAKER,
     USING$, PLYSTATUS

     TIME and MID$ assignments
     VDP assignment

   Data types, operators and operations supported:

     Integer, Float, String, Fixed Bidimensional Arrays
     Math expressions and basic operators (+-*/^)
     OR, AND, XOR, EQV, IMP, NOT, MOD, SHL, SHR
     &h0000, &o0000, &b00000000

   Limitations and differences:

     - Minimum hardware requirements for compiled programs:
       * MSX1 machine (with ROM BIOS and ROM BASIC);
       * 16kb of RAM for running as Cartridge without resources;
       * 32kb of RAM for running as Cartridge with resources;
       * 64kb of RAM for running with ExecROM (for 32kb ROMs) or ODO
         (for 48kb ROMs) via disk driver or Caslink3/CASDuino/MSX2CAS
         via cassete;
       * 64kb Memory Mapper and/or MegaRAM for running with SofaRun;
       * MegaROM compiled mode needs a Memory Mapper and/or MegaRAM with
         at least the same size as the final ROM for running it in memory
         with ExecROM (note: not required for running as Konami SCC
         Cartridge in emulators);
     - Partial file support (OPEN "GRP:", PRINT #...);
     - No dynamic arrays (REDIM);
     - No variant data type (only string, integer and single);
     - All variables has an unique data type (dont change in runtime);
     - Variables are not initialized at startup (do it yourself);
     - Default data type is single (use DEFINT A-Z to modify it to integer);
     - Printing numerical decimal values above 9999 will result in
       scientific notation;
     - CIRCLE tracing and aspect parameters not supported;
     - STOP works like END;
     - CMD WRTSCR/CMD WRTCHR activate the tiled mode in screen 2, forcing text
       mode coords;
     - LOCATE/PRINT works in graphical mode (screen 2+) without use of
       OPEN 'GRP:'. When in screen 2 tiled mode, it works similar as text
       mode coords (faster output);
     - PUT TILE <char>, (x,y) - for tiled mode used with screens 0-2;
     - COPY works only with graphical parameters (no files support) just
       like: COPY (x0,y0)-(x1,y1) TO (x2,y2);
       COPY ... TO <ram address> / COPY <ram address> TO ... is accepted
       instead of COPY <array>... (try HEAP() as <ram address>);
       When in screen 2 tiled mode, it works with text mode coords;
     - IDATA (integer data) and IREAD (integer read) works similar as DATA/READ
       but use less ROM memory and its faster. Use IRESTORE <position> to
       change the pointer position on binary files resources;
     - BLOAD works only to load standard screen files (.SCn) and TinySprite
       compatible files (.SPR). You need to inform a file name as literal (no
       via variables) because it will be loaded into ROM (it will activate
       MegaROM mode). Ex: BLOAD "file.sc5", S;
     - RANDOMIZE statement (without parameters) works like a RND(-TIME)
       function;
     - FRE() return free RAM size only (doesnt accept parameters);
     - HEAP() return first free RAM address;
     - TILE(x,y) return character from position (screens mode 0-2);
     - MSX() return current machine version (0: MSX1, 1: MSX2,
       2: MSX2+, 3: MSXturboR);
     - NTSC() return true to a NTSC (or PAL-M) machine and false to a PAL one;
     - TURBO() return true to cpu turbo mode (R800 or 5.37mhz) or false
       to standard mode (Z80/3.57mhz). Use with CMD TURBO...
     - VDP() without parameters return VDP version (0: TMS9918A, 1: V9938,
       2: V9958, x: VDP ID);
     - MAKER() return the manufacturer ID [1:ASCII/Microsoft, 2:Canon, 3:Casio,
       4:Fujitsu, 5:General, 6:Hitachi, 7:Kyocera, 8:Matsushita (Panasonic),
       9:Mitsubishi, 10:NEC, 11:Nippon Gakki (Yamaha), 12:JVC, 13:Philips,
       14:Pioneer, 15:Sanyo, 16:Sharp, 17:SONY, 18:Spectravideo, 19:Toshiba,
       20:Mitsumi, 21:Telematica, 22:Gradiente, 23:Sharp Brazil,
       24:GoldStar(LG), 25:Daewoo, 26:Samsung,
       212:1chipMSX/Zemmix Neo(KdL firmware)];
       Works only in some MSX2 machines;
     - RESOURCE(number) return resource start address (use with COPY);
     - COLLISION() return if any sprite collided with another sprite, else
       return -1;
     - COLLISION(<n>) return if a sprite <n> collided with another sprite,
       else return -1;
     - COLLISION(<n1>,<n2>) return n2 if sprite n1 collided with n2, else
       return -1;
     - INKEY() is an alternative to INKEY$, but returning an integer instead;
     - IPEEK()/IPOKE is similar to PEEK()/POKE, but applied for integer data;
     - DATA statements allocate space in resources page and will be compiled to
       a 48kb ROM;
     - PRINT USING support only numerical formatting symbols (#+-.*$,^0) and
       works the same way as USING$() function. For exponential display,
       include more # in the format string until the correct value is
       displayed. Use 0 char symbol to fill the value with left zeros;
     - USING$(format$, number) works just like PRINT USING statement;
     - F1..F10 function keys return ASCII codes 246 to 255 respectively;
     - Spaces on your line code, REMs and blank lines not affect compiled code
       size, but if your include it on your program it can help the compilation
       process (not need for strip spaces from your code anymore);
     - Resources total size are limited to 16kb (for FILE directive), and
       when compiled generates 48kb ROMs thats can run as cartridge or in RAM
       with SofaRUN or ODO loaders (ExecROM do not support this ROM size);
     - Arkos Tracker 2 player use the minimalist binary file (AKM and AKX),
       with songs composed to play at 50hz, exported at 0100 address, and must
       be included as the first resource in the list (AKM as 1st and AKX as 2nd);
     - PT3 player support is now deprecated, dont use it. Also, it cannot be
       used in concurrence with Arkos Tracker 2 player too.

   Statements not supported:

     AUTO, BLOAD, BSAVE, CALL, CLOAD,
     CONT, CSAVE, DEF FN, DELETE, EOF, ERASE,
     ERL, ERR, ERROR, FILES, FN, FPOS, FIELD,
     GET, GET DATE, GET TIME, IPL, LIST, LLIST,
     LOAD, LOC, LSET, LPRINT, MERGE, RSET,
     MOTOR, NEW, PUT, PUT KANJI, RENUM, RUN, SAVE,
     SET ADJUST, SET BEEP, SET DATE, SET PASSWORD,
     SET PROMPT, SET SCREEN, SET TIME,
     SET TITLE, TROFF, TRON,
     RESUME, ON ERROR GOTO, CLEAR

RESOURCES DIRECTIVES

  TEXT <text>
  FILE <file name>
  INCLUDE <file name>

EXTENDED COMMANDS

  Play a text resource with Basic (xbasic compatible, channel C disabled
  to use with SOUND command)
    CMD PLAY <rsn>[, <channel C: 0=off|1=on>]

  Mute the PSG (for PLAY use)
    CMD MUTE

  Draw a text resource with Basic (xbasic compatible)
    CMD DRAW <rsn>

  Exec a binary uncompressed resource as an assembly code
    CMD RUNASM <rsn>

  Exec a text resource as a plain text basic code (must start with ':')
    CMD RUNBAS <rsn>

  Write a compressed resource (pletter) to VRAM address
    CMD WRTVRAM <rsn>, <VRAM address>

  Write a compressed .ALF resource (pletter) to VRAM font pattern table
    CMD WRTFNT <rsn>

  Write a compressed resource (pletter) to VRAM tile pattern table
    CMD WRTCHR <rsn>

  Write a compressed resource (pletter) to VRAM tile color table
    CMD WRTCLR <rsn>

  Write a compressed resource (pletter) to VRAM screen table
    CMD WRTSCR <rsn>

  Write a TinySprite compatible resource (.SPR) to VRAM sprite pattern
  and color tables
    CMD WRTSPR <rsn>
    - See Jannone's online MSX Sprite Editor on:
      http://msx.jannone.org/tinysprite/tinysprite.html
    - Export your sprites as a Backup and save it as .SPR text file;
    - Use with sprite parameter size 2 or 3 on SCREEN statement;
    - You can use, alternativally: BLOAD "file.spr",S

  Write a compressed resource (pletter) to VRAM sprite pattern table
    CMD WRTSPRPAT <rsn>

  Write a compressed resource (pletter) to VRAM sprite color table
    CMD WRTSPRCLR <rsn>

  Write a compressed resource (pletter) to VRAM sprite attribute table
    CMD WRTSPRATR <rsn>

  Write RAM to VRAM address
    CMD RAMTOVRAM <RAM address>, <VRAM address>, <size>

  Write VRAM to RAM address
    CMD VRAMTORAM <VRAM address>, <RAM address>, <size>

  Write RAM to RAM address
    CMD RAMTORAM <RAM source address>, <RAM dest address>, <size>

  Write RESOURCE to RAM address
    CMD RSCTORAM <resource>, <dest address> [, <pletter? 0=no, 1=yes>]

  Inhibits the screen display
    CMD DISSCR

  Display the screen
    CMD ENASCR

  Turn off keyboard clicks (for xbasic use)
    CMD KEYCLKOFF

  Clear keyboard buffer and reset joysticks ports
    CMD CLRKEY

  Fill screen 2 with spaces to use with SETFNT/PRINT
    CMD CLRSCR

  Load internal FONTs to VRAM font pattern table (screen mode >= 1)
    CMD SETFNT <style number>[, <bank:0-2|empty=all>]
      0 = BIOS default font
      1 = Compile Zanac Style
      2 = Konami Gradius Style 1
      3 = Konami Gradius Style 2

  Update tiled font color with current fore/background COLOR
    CMD UPDFNTCLR

  Set clipping on and off (screen modes 5 through 8,
  and same as '#C of xbasic)
    CMD CLIP <0=off | 1=on>

  Activate cpu turbo mode (R800 or 5.37mhz)
    CMD TURBO <0=off | 1=on>

  Set READ statement to get lines from a text resource file (*.TXT or
  *.CSV) informed on FILE directive
    CMD RESTORE <resource number>
    - Use normal RESTORE <line number> and READ statements
      to read any specific line from the resource text;
    --- Line number zero is the first line on the resource text;
    --- Each line on TXT files will be treated like an unique string;
    --- Each line on CSV files will be treated like a DATA content;
    - Use IRESTORE <byte number> and IREAD to read binary resources.

  Enable/disable a page alternating effect on screen mode 5 and above
  via VDP registers 1 (Cadari Bit) and 14
    CMD PAGE <mode: 0=default, 1=swap, 2=wave>, <speed: 0=stop, 1=slow, 2=fast>

  Put a tile character into screen position (for tiled mode, screens 0-2)
    PUT TILE <n>, (<x>,<y>)

  Set tile color (for tiled mode, screens 1, 2 and 4)
    SET TILE COLOR <n>, <forecolor>, <backcolor>
                                          [, <bank:0-2, default=3=all>]
    SET TILE COLOR <n>, (<fc0>,...,<fc7>), (<bc0>,...,<bc7>)
                                          [, <bank:0-2, default=3=all>]

  Set tile pattern (for tiled mode, screens 0, 1, 2 and 4)
    SET TILE PATTERN <n>, (<l0>,...,<l7>) [, <bank:0-2, default=3=all>]

  Enable/Disable tiled mode (screens 2 and 4, for use with PRINT)
    SET TILE <ON|OFF>

  Set sprite color (msx2)
    SET SPRITE COLOR <n>, (<fc0>,...,<fc15>)

  Set sprite pattern for a quadrant
    SET SPRITE PATTERN <n>, <q:0-3>, (<l0>,...,<l7>)

  Flip a sprite pattern
    SET SPRITE FLIP <n>, <dir: 0=horizontal, 1=vertical, 2=both>

  Rotate a sprite pattern
    SET SPRITE ROTATE <n>, <dir: 0=left, 1=right, 2=180 degrees>

  Copy screen to array (only for modes 1, 2 and 4)
    SCREEN COPY TO <array> [SCROLL <direction>]
    *direction = same as STRIG

  Copy array to screen (only for modes 1, 2, and 4)
    SCREEN PASTE FROM <array>

  Do a screen scroll (only for modes 1, 2 and 4)
    SCREEN SCROLL <direction>
    *direction = same as STRIG

  Load a screen resource
    SCREEN LOAD <resource>

  Load a sprite resource
    SPRITE LOAD <resource>

  Load internal FONTs to VRAM font pattern table (screen mode >= 1)
    SET FONT <style number>[, <bank:0-2|empty=all>]
      0 = BIOS default font
      1 = Compile Zanac Style
      2 = Konami Gradius Style 1
      3 = Konami Gradius Style 2

EXTENDED FUNCTIONS

  Get resource data address
    <address> = RESOURCE(<rsn>)
    tokenized mode: <address> = USR0(<rsn>)

  Get resource data size
    <size> = RESOURCESIZE(<rsn>)
    tokenized mode: <size> = USR1(<rsn>)

  Multifunction alternatives
    PLAY() function alternative (for tokenized mode)
       <0=false> = USR2(0)

    INKEY$ function alternative (for tokenized mode)
       <ASC> = USR2(1)

    INPUT$(1) function alternative (for tokenized mode)
       <ASC> = USR2(2)

    AKM/PT3 PLAYER STATUS (bit 7 = end of song reached, bit 0 = loop status)
       <STATUS> = PLYSTATUS()
       tokenized mode: <STATUS> = USR2(3)

  Read PSG register
     n = PSG( <register number> )

  Sprite Collision Detection
     <-1=no collision|collided sprite number> = COLLISION( <-1=any sprite |
                                        sprite1> [, <sprite2> ] )
     tokenized mode:
          <-1=no collision|collided sprite number> = USR3( <-1=any sprite |
                                        sprite1 | &h1122> )
     Examples
       Beep if any sprite collided with each other:
          SN# = COLLISION(-1)
          IF SN# >= 0 THEN BEEP
       Beep if any sprite collided with sprite 2:
          SN# = COLLISION(2)
          IF SN# >= 0 THEN BEEP
       Beep if sprite 4 collided with sprite 5:
          SN# = COLLISION(5)
          IF SN# = 4 THEN BEEP
       Beep if sprite 5 collided with sprite 4 (direct test):
          SN# = COLLISION( 5, 4 )
          IF SN# >= 0 THEN BEEP
       Beep if sprite 0 collided with sprite 1 (direct test):
          SN# = COLLISION( 0, 1 )
          IF SN# >= 0 THEN BEEP
     Notes
       Sprites with same X and Y position are considered the same object, thus
       there's no collision in this case.

ARKOS TRACKER 2 PLAYER SUPPORT

  Load an AKM (songs) and AKX (effects) uncompressed files resources in memory
  setting up the first song to play
    CMD PLYLOAD <AKM resource number:0-n> [, <AKX resource number:0-n>]

  Initialize a song from the previous AKM resource loaded
    CMD PLYSONG [<subsong number:0-n>]

  Play/continue a song in memory
    CMD PLYPLAY

  Mute/pause a song in memory
    CMD PLYMUTE

  Play a sound effect from the previous AKX resource loaded
    CMD PLYSOUND <sound effect number:1-n> [, <channel number:0-2> [, <volume:0-16>] ]

  Set the song loop status
    CMD PLYLOOP <0=off | 1=on>

  Restart a song in memory (after stopped by loop turned off)
    CMD PLYREPLAY

PT3 PLAYER SUPPORT (DEPRECATED)

  Load a PT3 compressed resource (pletter) in memory
    CMD PT3LOAD <rsn>

  Play/continue a PT3 in memory
    CMD PT3PLAY

  Mute/pause a PT3 in memory
    CMD PT3MUTE

  Set PT3 loop status
    CMD PT3LOOP <0=off | 1=on>

  Restart a PT3 in memory
    CMD PT3REPLAY

FOOTNOTES

  <rsn> = Resource Sequence Number (start with zero)

SUPPORTERS SPECIAL THANKS

  Luciano Cadari (iplay.com.br)
  Mario Cavalcanti (clubemsx.com.br)
  Pedro de Medeiros
  Claudio Rodrigues
  Gilberto Taborda
  Ronaldo Prado
  Julio Berrincha
)";

#endif // MAIN_H_INCLUDED
