# Changelog Archive

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [1.2.0.0] - 2026-06-24

### mtf-enhancements MTF window-copy support with extended parameter handling

#### Added
- Add MTF operation 2: window-copy (partial map copy with configurable source rectangle and screen destination)
- Add screen page parameter scaffolding to all map operations (0, 1, 2)
- Add unit tests for all parameter combinations and an integration test

#### Changed
- Migrate parameter passing from Z80 registers to RAM workarea block (DAC) for all operations, enabling up to 9 parameters
- Extend compiler handler to accept 1–9 parameters (was 1–4)
- Use incremental builds — no forced `make clean`

## [1.1.0.0] - 2026-06-23

### Added

- Add `Konami4`, `ASCII16`, and `ASCII16X` compile modes with CLI flags `-4`/`--konami`, `-6`/`--ascii16`, and `-7`/`--ascii16x` for MegaROM mapper selection
- Add dispatch-table-guided kernel binary patching via `fixAscii16Mapper()` method with 7 patch points for ASCII16/ASCII16X
- Write ASCII16-X autodetection signature (`ASCII16X`) at ROM offset 0x0010
- Add dispatch table entries for new kernel patch-point labels
- Add unit tests for Konami4, ASCII16, and ASCII16X CLI parsing, ROM building, and signature byte verification
- Add integration test coverage for Konami4 and ASCII16 modes using existing MEGAROM test programs

### Changed

- Expand MegaROM compile code size limit from 1MB to 2MB by increasing `COMPILE_MAX_PAGES` from 64 to 128 pages
- Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` and extend to support both KonamiSCC and Konami4; extend `fixAscii16Mapper()` guard to also apply for `ASCII16X` mode
- Reuse existing kernel segment-switch patching for Konami4 (0x8000/0xA000 switch addresses via dispatch table)
- Reserve 8 bytes in kernel ROM header shifting INIT1 from 0x4010 to 0x4018 for ASCII16-X
- Update hardcoded LOADER symbol address from 0x4010 to 0x4018 in `symbol_export_context.cpp`
- Update output filename suffixes (`[Konami]`, `[ASCII16]`, `[ASCII16X]`) and CLI status/help messages
- Regenerate header artifacts (header.h, header.symbols.asm) for shifted kernel layout

## [1.0.1.0] - 2026-06-23

### Added

- Kernel regression test infrastructure (`make test-kernel`) with ROM binary integrity validation
- Kernel ROM size build-time check (max 0x4000 bytes) with Makefile alert and doctest unit test
- Full dispatch table coverage for all XBASIC math, string, graphics, trap, cast, and I/O routines (~55 new `dw` entries, ~55 new `DISP_XBASIC_*` constants)

### Changed

- Compact player idle-state initialization with sequential HL writes in 33_player.asm
- Consolidate 4 consecutive `ld (ENDPRG+n),a` zero-stores with inc-chain in 20_runtime.asm
- Remove redundant `push de`/`pop de` guard in `cmd_preflight_disk`
- Remove duplicate `ld a, (STARTUP_CFG_FILEIO)` load in `run_user_basic_code_on_rom`
- Remove redundant `xor a` in `resource.address`
- Restructure wrapper routine jump table from 126-entry `jp` to word-pointer dispatch (saves ~125 bytes)
- Unify wrapper dispatch through single `addKernelCall` path with index auto-detection, remove `addKernelDispatch` and dispatcher ASM routine
- Replace ~117 `cpu.addCall(def_XBASIC_*)` with `optimizer.addKernelCall(DISP_XBASIC_*)` across all compiler files for indirect dispatch
- Replace ~12 magic number BIOS addresses (`0x009C`, `0x009F`, etc.) with named constants (`def_CHSNS`, `def_CHGET`, etc.)
- Double `header.bin` from 0x4000 to 0x8000 bytes: first half (0x0000-0x3FFF) becomes virtual dispatch table section, second half (0x4000-0x7FFF) is the kernel ROM image
- Move dispatch table to logical address 0x0000 with `DEFS` padding to 0x4000 boundary, removing the need for a second `org`
- Adapt `getKernelCallAddr()` for two-range addressing (0x0000-0x3FFF table lookups with direct `bin_header_bin` indexing, 0x4000-0x7FFF kernel addresses)
- Update ROM builder to write only second half (`bin_header_bin + 0x4000`)
- Update build size check from 16384 to 32768 byte limit
- Disable 58 unreachable code labels in `90_support.asm` with `DEFS` fillers (preserving binary offsets)
- Redirect 80 XBASIC_* EQUs from hardcoded hex addresses to native Z80 labels (`C`/`I`/`J` convention) in `00_constants_and_workarea.asm`
- Add `C7F44` mid-routine entry point label for `XBASIC_COPY_STRING` in `90_support.asm`
- Collapse `intCompareGT`/`intCompareGE` into delegated comparisons via operand swap (18 bytes saved)
- Replace `sla a` sequences with `add a,a` in `cmd_setfnt`, `cmd_play`, `cmd_page`, `cmd_mtf` (14 bytes saved)
- Replace `ld (MV_DPTR),hl`/`ld hl,(MV_DPTR)` with `push hl`/`pop hl` (4 bytes saved)
- Micro-optimizations in `FloatToBCD`, `intSHR`, `XBASIC_END`, `XBASIC_TAB`, `cmd_wrtfnt` (10 bytes saved)
- Remove dead commented-out code blocks

### Fixed

- Fix MegaROM mode restore in `verify.slots.test.ram` (save flags, write 0 to port 0x8E)
- Fix missing carry clear before `sbc hl,de` in `cmd_fmaxfiles.set_heap_size`
- Fix hardcoded 255-byte copy in `resource.get_data` (cap at actual resource size)
- Fix duplicate `WriteParamBCD` EQU conflict with existing label causing compilation error
- Fix compiler error messages reporting tag index instead of physical file line number (use `tag->lexerLine->lineNumber`)
- Fix integration test Makefile not stopping on compilation error (`|| exit 1` in for loop)

### Removed

- Remove fully commented-out BDOS fallback block from 34_file_handling.asm
- Remove commented-out `VDP_wait` routine from 60_bios_helpers.asm
- Remove dead `inc de` in `XBASIC_INIT` function key loop
- Remove dead `ld a, 2` in `cmd_finput` state machine
- Remove 5 alignment `nop` instructions from `20_runtime.asm` (no longer needed after dispatch table move)

## [1.0.0.0] - 2026-06-19

### Added

- File handling support implementation (US-007).
- Spec migration to OpenSpec format.
- Changelog standardized to Keep a Changelog format.
- Windows Unicode argv support via `__wargv` to UTF-8 conversion on program startup.
- `--` (end-of-options) support in CLI parser for filenames starting with `-`.
- `SET TILE PATTERN` buffer form with 4-integer array and bank parameter.
- `GET TILE PATTERN` with bank parameter (former stub now implemented).
- Z80 kernel bulk LDIRVM/LDIRMV routines for `get_tile_pattern` / `set_tile_pattern_buffer`.
- `SET TILE COLOR` all syntax variants: inline FC/BC, per-row FC array (with/without BC array), and buffer form (`SET TILE COLOR <n>, <4-int-array> [, <bank>]`), with bank parameter support (0-2 for individual banks, 3 or omitted for all banks).
- `GET TILE COLOR` with bank parameter: `GET TILE COLOR <n>, <4-int-array> [, <bank>]` with LDIRMV read via `get_tile_color` (replaces stub).
- Z80 kernel routines for `set_tile_color_buf` (3-pass LDIRVM for bank=all) and `get_tile_color` with screen mode validation.
- Unit and integration tests for all SET/GET TILE COLOR syntax forms.
- `SET TILE FLIP <n>, <dir>[, <bank>]` with 3 directions (0=horizontal, 1=vertical, 2=both) and optional bank parameter (0-2 specific, 3=all).
- `SET TILE ROTATE <n>, <dir>[, <bank>]` with 3 directions (0=left, 1=right, 2=180) and optional bank parameter.
- Screen 2 validation (returns early on screen >= 5).
- Reuses existing sprite flip/rotate Z80 routines (`binaryReverseA`, `blockRotateL`, `blockRotateR`) for 8-byte tile blocks.
- Unit parser, compiler, and integration tests for all flip/rotate syntax forms.

### Changed

- Magic constants in RAM percentage calculation replaced with named constants.
- Platform detection normalized from `#ifdef Win` to standard `#ifdef _WIN32`.
- Removed dead includes (`<malloc.h>`, `<math.h>`) from `main.cpp`.

### Fixed

- Uninitialized `vscode` member causing "VSCode already initialized" error on Windows.
- `pathExists()` on Windows: replaced stub with `GetFileAttributesA`.
- `-?` help option removed due to glob expansion risk in POSIX shells.
- `set_tile_flip` / `set_tile_rotate`: direction in HL was clobbered by VRAM address operations (`get_tile_vram_addr` + `set_tile.copy`), causing all directions to behave as direction=2 (both). Fixed by saving HL on stack before VRAM ops.
- `SET TILE FLIP` / `SET TILE ROTATE` with 3 params: bank evaluation (`evalExpression`) overwrote HL (direction) before the call. Fixed by saving HL on stack before bank evaluation.
- `set_tile_rotate` left/right: destination buffer was `STRBUF+32` (sprite size) causing rotation output to be written past the tile data while source data was shifted to zero. Fixed by backing up tile to `STRBUF+8` and rotating from backup.

## [0.3.4.2] - 2026-04-14

### Fixed

- Sprite glitch bug fix (issue #19).

### Changed

- Z80 kernel SRP refactoring.

## [0.3.4.1] - 2026-04-09

### Fixed

- ELF export bug.

## [0.3.4.0] - 2026-04-08

### Fixed

- CDB empty code and ON statements bug fixes (issue #18).

## [0.3.3.9] - 2026-03-25

### Fixed

- CDB variable name bug.

## [0.3.3.8] - 2026-03-21

### Fixed

- VSCode parameter bug.

## [0.3.3.7] - 2026-03-20

### Added

- CDB symbol export feature (issue #17).
- Plain/megarom auto mode parameter (`-a`).
- VSCode MSX-BASIC project initializer parameter (`--vscode`).

### Fixed

- MAX name bug (issue #13).

### Changed

- Extensive code refactoring and memory allocation hardenings.

## [0.3.3.6] - 2026-01-28

### Fixed

- LINE statement parameters bug (issue #16).

## [0.3.3.5] - 2026-01-16

### Added

- SET/GET SPRITE COLOR/PATTERN new commands.

## [0.3.3.4] - 2025-09-26

### Changed

- MSX Tile Forge map loading optimization.

### Fixed

- Resource file check bug.

## [0.3.3.3] - 2025-09-20

### Added

- MSX Tile Forge support (CMD MTF).

## [0.3.3.2] - 2025-09-14

### Added

- NoICE OpenMSX Debugger support.

### Changed

- Resources refactoring and limit expanded (~3200).

### Fixed

- Minor bug fixes.

## [0.3.3.1] - 2025-09-04

### Added

- OMDS support.
- CLEAR statement.

### Changed

- Symbols/resources refactoring.

### Fixed

- Minor bug fixes.

## [0.3.3.0] - 2025-08-18

### Added

- New `-i` and `-o` parameters.

### Changed

- GitHub CI/CD and RPM/DEB packaging refactoring.
- Source code ported from Code::Blocks to VSCode.

## [0.3.2.9] - 2025-07-22

### Fixed

- MegaROM emulation detection bug.

## [0.3.2.8] - 2025-07-21

### Added

- ASCII8 MegaROM format support.

## [0.3.2.7] - 2025-04-25

### Fixed

- DRAW bug.

## [0.3.2.6] - 2025-01-20

### Fixed

- LINE bug.

### Added

- SCREEN ON/OFF statement.

## [0.3.2.5] - 2024-12-18

### Added

- KonamiSCC filename suffix added to MegaROM.

## [0.3.2.4] - 2024-09-05

### Added

- GET DATE `date_format` parameter.

## [0.3.2.3] - 2024-09-04

### Added

- Custom SET DATE/TIME, GET DATE/TIME statements.

## [0.3.2.2] - 2024-06-19

### Fixed

- MegaROM size estimate bug.

## [0.3.2.1] - 2024-05-23

### Fixed

- MegaROM > 256kb bug.

### Changed

- `-c` set as default mode.

## [0.3.2.0] - 2023-11-04

### Fixed

- CMD PAGE fix (Cadari bit).

## [0.3.1.9] - 2023-11-02

### Added

- CMD PAGE new command (Cadari bit).

## [0.3.1.8] - 2023-10-29

### Changed

- WIDTH statement optimization.

## [0.3.1.7] - 2023-10-28

### Changed

- VDP function optimization (Cadari bit).

## [0.3.1.6] - 2023-10-16

### Added

- Mouse support on MSX1 via PAD function.

## [0.3.1.5] - 2023-09-24

### Fixed

- Integer multiply in MegaROM bug.

## [0.3.1.4] - 2023-09-10

### Fixed

- Multiplying constant integers bug.

## [0.3.1.3] - 2023-09-02

### Added

- CMD RSCTORAM new command.

## [0.3.1.2] - 2023-08-27

### Added

- SET SPRITE ROTATE new command.

## [0.3.1.1] - 2023-08-26

### Added

- IPEEK()/IPOKE new commands.

## [0.3.1.0] - 2023-08-19

### Added

- SET SPRITE FLIP new command.

## [0.3.0.9] - 2023-07-03

### Fixed

- LINE parameters parse bug.

## [0.3.0.8] - 2023-06-24

### Fixed

- BASE function bug.

## [0.3.0.7] - 2023-06-23

### Fixed

- DATA last element parameter bug.

## [0.3.0.6] - 2023-06-22

### Fixed

- MegaROM resource allocation bug.

## [0.3.0.5] - 2023-06-07

### Fixed

- Integer division optimization bug.

## [0.3.0.4] - 2023-05-25

### Fixed

- PAINT statement on WebMSX USA TurboR bug.

## [0.3.0.3] - 2023-05-25

### Fixed

- COPY statement operator parameter bug.

## [0.3.0.2] - 2023-05-24

### Fixed

- COPY statement and SYMBOLs file (var address) bug.

## [0.3.0.1] - 2023-05-23

### Fixed

- COPY screen data in tiled mode bug.

## [0.3.0.0] - 2023-02-17

### Fixed

- TURBO function bug.

### Changed

- Integer math optimizations.

## [0.2.9.9] - 2023-01-20

### Added

- SET FONT new command.

## [0.2.9.8] - 2022-12-21

### Added

- MegaROM mode support for CMD, RESOURCES and Arkos Tracker player.

## [0.2.9.7] - 2022-07-20

### Fixed

- COLOR NEW/RESTORE bug on MSX1.

## [0.2.9.6] - 2022-07-15

### Fixed

- Tokenized THEN/GOTO bug.

### Added

- SET ADJUST/SCREEN/BEEP/TITLE/PROMPT support.

## [0.2.9.5] - 2022-05-23

### Added

- SCREEN COPY/PASTE, SCREEN SCROLL.
- SCREEN LOAD (.SCn as resources).
- SPRITE LOAD (CMD WRTSPR alternative).

## [0.2.9.4] - 2022-04-25

### Fixed

- Several bug fixes: try..catch exceptions, tokenized source code input detection, DATA/RESTORE misbehavior.

## [0.2.9.3] - 2022-04-05

### Added

- IRESTORE new command (for IREAD binary files).
- RESOURCESIZE function.

## [0.2.9.2] - 2022-01-30

### Added

- Support for TinySprite backup files with BLOAD and CMD WRTSPR.

### Fixed

- BLOAD for screen files bug.

## [0.2.9.1] - 2022-01-05

### Fixed

- LINE negative parameter bug.

## [0.2.9.0] - 2021-12-03

### Added

- SET TILE ON/OFF/COLOR/PATTERN new commands.

## [0.2.8.9] - 2021-11-03

### Fixed

- Expression evaluator of TIME/INKEY bug.
- Undeclared array syntax error.

## [0.2.8.8] - 2021-10-14

### Fixed

- Hexa constant bug.

### Added

- RESUME support (tokenized mode only).

## [0.2.8.7] - 2021-10-13

### Fixed

- TAB function token bug (tokenized mode).
- MOD and \ bug for single and double types.
- Numeric constant type identification bug.

## [0.2.8.6] - 2021-10-12

### Added

- "0" as format string on USING$() for printing left zeros.

## [0.2.8.5] - 2021-10-11

### Fixed

- SWAP bug fix 2.

### Changed

- Math operations parameters optimization 2.

## [0.2.8.4] - 2021-10-08

### Fixed

- SWAP bug.
- CMD PLYLOOP bug.

### Changed

- Math operations parameters optimization.

### Added

- PLYSTATUS().

## [0.2.8.3] - 2021-10-05

### Fixed

- PRINT USING bug.

### Changed

- PRINT USING optimization.

## [0.2.8.2] - 2021-09-30

### Added

- CIRCLE full parameters.

## [0.2.8.1] - 2021-09-28

### Fixed

- PSET parameters bug.

### Added

- SCREEN full parameters support.
- SET VIDEO support.
- COPY SCREEN support.

## [0.2.8.0] - 2021-09-24

### Fixed

- COLOR without parameters bug.
- COPY parameter bug.
- CMD WRTVRAM hardware/int bug.

### Changed

- Sprite collision optimization.

## [0.2.7.9] - 2021-09-21

### Fixed

- Running on SofaRun bug.

## [0.2.7.8] - 2021-09-10

### Added

- PRINT USING support for numeric values.
- USING$() new function.
- F1..F10 function keys return ASCII codes 246 to 255 respectively.

### Fixed

- END statement bug.

## [0.2.7.7] - 2021-08-31

### Fixed

- PAINT bug (screen 2 border color must be equal to filler color).

### Added

- TXT/CSV files resources support (CMD RESTORE).
- DATA statements now allocated in resources page.

## [0.2.7.6] - 2021-07-20

### Changed

- Float and String array index access optimization.

### Added

- COLLISION(<n1>,<n2>).

## [0.2.7.5] - 2021-07-16

### Changed

- Integer array index access optimization.
- Integer FOR/NEXT optimization.

## [0.2.7.4] - 2021-06-17

### Fixed

- AKM fix to playing at same speed in 50hz/60hz.

## [0.2.7.3] - 2021-06-16

### Changed

- Arkos Tracker 2 player support (PT3 deprecated).

## [0.2.7.2] - 2021-05-07

### Fixed

- Bug fix pack (duplicated line error in FOR..NEXT block).

## [0.2.7.1] - 2021-05-05

### Fixed

- Bug fix pack (setfnt default color).

### Added

- CMD UPDFNTCLR.

## [0.2.7.0] - 2021-05-01

### Added

- CMD CPU, CPU(), MAKER(), VDP() without parameters.

## [0.2.6.9] - 2021-04-29

### Fixed

- Bug fix pack (LOCATE).

### Changed

- Several code size optimizations.

### Added

- IDATA/IREAD implementation.

## [0.2.6.8] - 2021-04-27

### Added

- Change font in graphical mode (CMD WRTFNT and CMD SETFNT).

### Changed

- Initialization, data/heap pointers and USR() code optimization.

## [0.2.6.7] - 2021-04-25

### Added

- NTSC() function.

### Fixed

- Bug fix pack (LINE, PRINT ';' in tile mode).

## [0.2.6.6] - 2021-04-22

### Added

- SYMBOLs file create for use on OpenMSX debugger (`-s` option).

## [0.2.6.5] - 2021-04-16

### Changed

- OPEN 'GRP' optimization.
- LOCATE/PRINT in graphical mode.

## [0.2.6.4] - 2021-04-07

### Fixed

- MegaROM mode bug (running with SofaRun with MegaRAM).

## [0.2.6.3] - 2021-04-07

### Fixed

- PT3/MegaROM mode bug (running with ExecROM with MegaRAM).

## [0.2.6.2] - 2021-04-05

### Fixed

- PT3 bug (running in 64kb RAM with ExecROM without mapper, ex: Hotbit w/disk driver, Sony HB-F500, Victor HC-95, Toshiba HX-23F...).

## [0.2.6.1] - 2021-03-30

### Fixed

- PT3 player bug (32kb RAM machines, ex: Pioneer PX7).

## [0.2.6.0] - 2021-03-29

### Fixed

- Compatibility bug fix pack (hotbit).

## [0.2.5.9] - 2021-03-12

### Added

- Precomp/MegaROM: BLOAD.

## [0.2.5.8] - 2021-03-10

### Fixed

- Bug fix pack (OPEN, PAINT, temporary strings).

### Added

- Line number option.

## [0.2.5.7] - 2021-03-09

### Added

- MegaROM: RETURN \<line\>, ON..., R800 cpu (Turbo-R) math code support.

## [0.2.5.6] - 2021-03-03

### Fixed

- Precompilation: bug fix pack (LINE INPUT, CLEAR, RAM size).

### Added

- MegaROM: DATA, READ, RESTORE.

## [0.2.5.5] - 2021-03-02

### Added

- Experimental MegaROM option (GOTO, GOSUB, RETURN, FOR/NEXT).

### Fixed

- MSX 1 BIOS bug (CALSLT to page 0: Expert XP-800).

## [0.2.5.4] - 2021-02-24

### Fixed

- Precomp: bug fix pack (PT3).

## [0.2.5.3] - 2021-02-23

### Fixed

- Precomp: bug fix pack (TAB).

### Changed

- Start compiled program at screen 0.

## [0.2.5.2] - 2021-02-22

### Fixed

- Precomp: bug fix pack (USR).

### Changed

- Code size optimization.

## [0.2.5.1] - 2021-02-18

### Fixed

- Precomp: bug fix pack (BASE, PAINT, PSET).

### Changed

- Binary code size optimization.

## [0.2.5.0] - 2021-02-16

### Fixed

- Precomp: DATA/USR bug.

### Added

- PSG, partial file support (EOF, INPUT#, OPEN).

### Changed

- Binary code size optimization.

## [0.2.4.9] - 2021-02-16

### Fixed

- Precomp: bug fix pack (DEF, DIM, functions, DATA, LINE, OPEN).

## [0.2.4.8] - 2021-02-12

### Added

- Precomp: COLOR=NEW, COLOR=RESTORE, COLOR=(), COLOR SPRITE()=, COLOR SPRITE$()=, OPEN, CLOSE, PRINT#, DEF USR, USR, MAXFILE, CMD CLIP.

## [0.2.4.7] - 2021-01-25

### Added

- Precomp: PUT TILE, TILE, MSX.

## [0.2.4.6] - 2021-01-21

### Added

- Precomp: COPY .. TO HEAP / COPY HEAP TO ... .

## [0.2.4.5] - 2021-01-21

### Added

- Precomp: SET PAGE, SET SCROLL, COPY.

## [0.2.4.4] - 2021-01-19

### Fixed

- Precomp: PUT SPRITE fix, RESTORE fix.

## [0.2.4.3] - 2021-01-18

### Changed

- Precomp: array pointers optimization.

## [0.2.4.2] - 2021-01-18

### Changed

- Precomp: logical operations size optimization.

## [0.2.4.1] - 2021-01-17

### Fixed

- Precomp: CMD and internal USR bug.

## [0.2.4.0] - 2021-01-16

### Added

- Precomp: CMD and refactoring memory scheme.

## [0.2.3.9] - 2021-01-15

### Added

- Precomp: ON INTERVAL/SPRITE/STRIG/KEY/STOP.

## [0.2.3.8] - 2021-01-14

### Added

- Precomp: PUT SPRITE, SPRITE$, ON...GOTO/GOSUB and related.

## [0.2.3.7] - 2021-01-12

### Added

- Precomp: DATA/READ and special functions.

## [0.2.3.6] - 2021-01-12

### Added

- Precomp: graphical statements (CIRCLE, LINE, PAINT, DRAW).

## [0.2.3.5] - 2021-01-11

### Added

- Precomp: graphical statements (COLOR, WIDTH, SCREEN, PSET, PRESET).

## [0.2.3.4] - 2021-01-10

### Added

- Precomp: FOR/NEXT.

## [0.2.3.3] - 2021-01-09

### Added

- Precomp: IF/THEN/ELSE, IF/GOTO/ELSE, IF/GOSUB/ELSE.

## [0.2.3.2] - 2021-01-08

### Added

- Precomp: string functions.

## [0.2.3.1] - 2021-01-06

### Added

- Precomp: logical expressions and numeric functions.

## [0.2.3.0] - 2021-01-01

### Added

- Precompilation kernel.

## [0.2.2.0] - 2020-10-28

### Fixed

- PT3 joystick bug.
- Locate/Print commands in screen mode 2 bug.

### Changed

- Strip 'remark only' lines.

### Fixed

- `?` and `_` commands.

## [0.2.1.0] - 2020-10-13

### Added

- Sprite collision support (USR3 function).
- Replay a PT3 (CMD PT3REPLAY).

## [0.2.0.0] - 2020-08-19

### Added

- Built-In fonts support implemented (CMD SETFNT, CMD PT3LOOP).

## [0.1.9.0] - 2020-08-02

### Fixed

- Emulator / Hardware ROM loading bugs.

## [0.1.8.0] - 2020-06-14

### Fixed

- CALL TURBO ON/OFF bug.

## [0.1.7.0] - 2020-06-13

### Fixed

- CMD 2nd parameter, SPC(), CHR$(), STRING$(), eol (Windows) bugs.

## [0.1.6.0] - 2020-06-12

### Added

- CMD WRTFNT to load compressed .ALF files.

## [0.1.5.0] - 2020-06-12

### Fixed

- Windows version minor bugs.

## [0.1.4.0] - 2020-06-03

### Added

- PT3TOOLS support.

## [0.1.3.0] - 2020-05-28

### Added

- INCLUDE, WRTSCR, WRTCLR, WRTCHR, WRTSPRPAT, CMD DRAW implemented.

## [0.1.2.0] - 2020-05-27

### Added

- Resources initial implementation (TEXT, CMD PLAY).

## [0.1.1.0] - 2020-05-24

### Added

- Turbo mode implemented (xbasic).

## [0.1.0.0] - 2020-05-21

### Added

- Proof of concept.

See main [CHANGELOG](CHANGELOG.md) for newer releases.

[1.1.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.1.0.0
[1.0.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.1.0
[1.0.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.0.0
[0.3.4.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.4.2
[0.3.4.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.4.1
[0.3.4.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.4.0
[0.3.3.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.9
[0.3.3.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.8
[0.3.3.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.7
[0.3.3.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.6
[0.3.3.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.5
[0.3.3.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.4
[0.3.3.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.3
[0.3.3.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.2
[0.3.3.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.1
[0.3.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.0
[0.3.2.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.9
[0.3.2.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.8
[0.3.2.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.7
[0.3.2.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.6
[0.3.2.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.5
[0.3.2.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.4
[0.3.2.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.3
[0.3.2.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.2
[0.3.2.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.1
[0.3.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.0
[0.3.1.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.9
[0.3.1.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.8
[0.3.1.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.7
[0.3.1.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.6
[0.3.1.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.5
[0.3.1.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.4
[0.3.1.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.3
[0.3.1.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.2
[0.3.1.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.1
[0.3.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.0
[0.3.0.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.9
[0.3.0.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.8
[0.3.0.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.7
[0.3.0.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.6
[0.3.0.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.5
[0.3.0.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.4
[0.3.0.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.3
[0.3.0.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.2
[0.3.0.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.1
[0.3.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.0
[0.2.9.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.9
[0.2.9.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.8
[0.2.9.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.7
[0.2.9.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.6
[0.2.9.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.5
[0.2.9.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.4
[0.2.9.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.3
[0.2.9.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.2
[0.2.9.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.1
[0.2.9.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.0
[0.2.8.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.9
[0.2.8.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.8
[0.2.8.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.7
[0.2.8.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.6
[0.2.8.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.5
[0.2.8.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.4
[0.2.8.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.3
[0.2.8.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.2
[0.2.8.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.1
[0.2.8.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.0
[0.2.7.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.9
[0.2.7.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.8
[0.2.7.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.7
[0.2.7.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.6
[0.2.7.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.5
[0.2.7.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.4
[0.2.7.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.3
[0.2.7.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.2
[0.2.7.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.1
[0.2.7.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.0
[0.2.6.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.9
[0.2.6.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.8
[0.2.6.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.7
[0.2.6.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.6
[0.2.6.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.5
[0.2.6.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.4
[0.2.6.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.3
[0.2.6.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.2
[0.2.6.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.1
[0.2.6.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.0
[0.2.5.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.9
[0.2.5.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.8
[0.2.5.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.7
[0.2.5.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.6
[0.2.5.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.5
[0.2.5.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.4
[0.2.5.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.3
[0.2.5.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.2
[0.2.5.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.1
[0.2.5.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.0
[0.2.4.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.9
[0.2.4.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.8
[0.2.4.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.7
[0.2.4.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.6
[0.2.4.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.5
[0.2.4.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.4
[0.2.4.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.3
[0.2.4.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.2
[0.2.4.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.1
[0.2.4.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.0
[0.2.3.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.9
[0.2.3.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.8
[0.2.3.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.7
[0.2.3.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.6
[0.2.3.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.5
[0.2.3.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.4
[0.2.3.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.3
[0.2.3.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.2
[0.2.3.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.1
[0.2.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.0
[0.2.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.2.0
[0.2.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.1.0
[0.2.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.0.0
[0.1.9.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.9.0
[0.1.8.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.8.0
[0.1.7.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.7.0
[0.1.6.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.6.0
[0.1.5.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.5.0
[0.1.4.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.4.0
[0.1.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.3.0
[0.1.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.2.0
[0.1.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.1.0
[0.1.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.0.0
