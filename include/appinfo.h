/***
 * @file appinfo.h
 * @brief Application information header
 * @author Amaury Carvalho (2025)
 */

#ifndef APPINFO_H
#define APPINFO_H

/***
 * @brief Compiler current version
 * @warning Dont forget to update CHANGELOG.md, debian/changelog and
 * rpmbuild/SPECS/msxbas2rom.spec files
 * @remark Run the code below to create a new release on github:
 * git tag -a v0.0.0.0 -m "Release 0.0.0.0" && git push msxbas2rom --tags
 * git checkout -b release/v0.0.0.0
 * git push msxbas2rom release/v0.0.0.0
 * git switch master
 */
#define app_version "0.3.3.3"

//! @brief Header information
const char *info_header = R"(MSXBAS2ROM - MSX BASIC TO ROM COMPILER
Created by Amaury Carvalho (2020-2025)
Version:)";

//! @brief Footer information
const char *info_footer = R"(
Help us to maintain this project, learn how:
https://github.com/amaurycarvalho/msxbas2rom/wiki/Contributing )";

//! @brief Help information (-h parameter)
const char *info_help = R"(
Usage: msxbas2rom [options] <filename.bas>

General options:
    -h or --help or -? = help
    -q or --quiet = quiet (no verbose)
    -d or --debug = debug mode (show details)
    -D or --doc = display quick reference guide
    -H or --history = display app history
    -v or --version = display app version

Compile options (optional):
    -c  = plain ROM compile mode (default)
    -x  = ASCII8 MegaROM compile mode
    -x --scc = Konami SCC MegaROM compile mode
    -s  = generate symbols for OpenMSX debugger
    --lin = write the MSX BASIC line numbers in the binary code

Path options (optional)
    -i  = input path (default=source file path)
    -o  = output path (default=source file path)

Output: <filename.rom>

See more information at:
https://github.com/amaurycarvalho/msxbas2rom/wiki/Usage
)";

/***
 * @var
 * @brief History information (--ver parameter)
 * @todo
 *   0.0.0.0 – (2023/xx/xx) RUN support to call sub-programs;
 *   0.0.0.0 – (2023/xx/xx) FM/SCC music support (furnace? Trillo Tracker?);
 *   0.0.0.0 – (2023/xx/xx) AKG music file support (Arkos Tracker);
 *   0.0.0.0 – (2023/xx/xx) Disk file support;
 *   0.0.0.0 – (2023/xx/xx) Better support to double precision type (BCD);
 *   0.0.0.0 – (2023/xx/xx) SET TILE FLIP/ROTATE new command;
 *   0.0.0.0 – (2023/xx/xx) SET SPRITE COLOR/PATTERN new command;
 */
const char *info_history = R"(
Version history

0.3.3.3 – (2025/09/20) MSX Tile Forge support (CMD MTF);
0.3.3.2 – (2025/09/14) Resources refactoring and limit expanded (~3200), 
                       NoICE OpenMSX Debugger support and minor bug fixes;
0.3.3.1 – (2025/09/03) Symbols/resources refactoring, OMDS support,
                       CLEAR statement and minor bug fixes;
0.3.3.0 – (2025/08/18) New -i and -o parameters;
0.3.2.9 – (2025/07/22) MegaROM emulation detection bug fix;
0.3.2.8 – (2025/07/21) ASCII8 MegaROM format support;
0.3.2.7 – (2025/04/25) DRAW bug fix;
0.3.2.6 – (2025/01/20) LINE bug fix and SCREEN ON/OFF stmt;
0.3.2.5 – (2024/12/18) KonamiSCC filename sufix added to MegaROM;
0.3.2.4 – (2024/09/05) GET DATE date_format parameter;
0.3.2.3 – (2024/09/04) Custom SET DATE/TIME, GET DATE/TIME stmts;
0.3.2.2 – (2024/06/19) MegaROM size estimate bug fix;
0.3.2.1 – (2024/05/23) MegaROM > 256kb bug fix and -c as default;
0.3.2.0 – (2023/11/04) CMD PAGE fix (Cadari bit);
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

Know more about the project:
https://github.com/amaurycarvalho/msxbas2rom
)";

/***
 * @brief Documentation information (--doc parameter)
 */
const char *info_documentation = R"(
See a complete reference guide on project wiki page:
https://github.com/amaurycarvalho/msxbas2rom/wiki/Documentation

SUPPORTERS SPECIAL THANKS

  - Luciano Cadari (iplay.com.br);
  - Mario Cavalcanti (clubemsx.com.br);
  - Pedro de Medeiros;
  - Claudio Pinheiro;
  - Diogo Patrao;
  - Paulo Goncalves;
  - Gilberto Taborda;
  - Ronaldo Prado;
  - Julio Berrincha.

Know more at:
https://github.com/amaurycarvalho/msxbas2rom/wiki
)";

#endif  // APPINFO_H