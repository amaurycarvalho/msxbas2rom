## ADDED Requirements

### Requirement: Convert parsed BASIC actions into Z80 machine code
As a ROM builder user, the system SHALL convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably.

Compilation SHALL start only when parser output contains tags/actions. The compiler SHALL emit start/end program stubs and generated code for each tag. Duplicate or invalid symbols/flow states (e.g., FOR without NEXT) SHALL be reported as compile errors. DATA/IDATA parser flags SHALL produce corresponding compiler-managed resources. The compiler SHALL track generated code/RAM usage and expose metrics used by the CLI.

#### Scenario: Compile valid parser output
- **WHEN** semantic analysis is executed on a parser result with valid tags and actions
- **THEN** compilation succeeds
- **AND** code size is greater than zero
- **AND** RAM usage metrics are available

#### Scenario: Fail on duplicated line symbol mapping
- **WHEN** parser tags map to an already declared line symbol
- **THEN** compilation fails
- **AND** error message indicates line number already declared
#### Scenario: Register DATA and IDATA resources

- **WHEN** parser output has `has_data` and `has_idata` enabled and semantic analysis is executed
- **THEN** the compiler adds data and idata resources to the resource manager

### Requirement: Compiler emits kernel calls via addKernelCall

The compiler SHALL emit all calls to Z80 kernel routines (addresses in the range 0x4000-0x7FFF, corresponding to `bin_header_bin`) using `CompilerCodeOptimizer::addKernelCall()`, which resolves target addresses by reading the `dw` word-pointer entries from `bin_header_bin` at `0x0000 + index * 2`. The compiler SHALL pass a `DISP_*` dispatch index to `addKernelCall`. The compiler SHALL NOT use `cpu.addCall()` for any kernel-internal routine that has a dispatch table entry.

#### Scenario: Integer comparison operators use addKernelCall

- **WHEN** the compiler emits code for integer comparison (`=`, `<>`, `<`, `<=`, `>`, `>=`)
- **THEN** `optimizer.addKernelCall(DISP_intCompareEQ)` (and analogous) is used
- **AND** `cpu.addCall()` is not used for these kernel routines

#### Scenario: Float comparison operators use addKernelCall

- **WHEN** the compiler emits code for float comparison
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_COMPARE_FLOATS_EQ)` (and analogous) is used

#### Scenario: XBASIC math functions use addKernelCall

- **WHEN** the compiler emits code for math functions (`SIN`, `COS`, `TAN`, `ATN`, `EXP`, `LOG`, `SQR`, `RND`, `INT`, `FIX`, `SGN`, `ABS`)
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_SIN)` (and analogous) is used

#### Scenario: XBASIC string functions use addKernelCall

- **WHEN** the compiler emits code for string functions (`LEFT$`, `RIGHT$`, `MID$`, `INSTR`, `VAL`, `STRING$`, `BIN$`, `OCT$`, `HEX$`)
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_LEFT)` (and analogous) is used

#### Scenario: XBASIC type cast routines use addKernelCall

- **WHEN** the compiler emits code for type casts (integer to float, float to integer, numeric to string, string to numeric)
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_CAST_INTEGER_TO_FLOAT)` (and analogous) is used

#### Scenario: Arithmetic operators use addKernelCall for floats

- **WHEN** the compiler emits code for float addition, subtraction, multiplication, or division
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_ADD_FLOATS)` (and analogous) is used

#### Scenario: String concatenation and copy use addKernelCall

- **WHEN** the compiler emits code for string concatenation or copy
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_CONCAT_STRINGS)` or `optimizer.addKernelCall(DISP_XBASIC_COPY_STRING)` is used

#### Scenario: Graphics statements use addKernelCall

- **WHEN** the compiler emits code for graphics statements (`LINE`, `BOX`, `BOXF`, `PSET`, `PAINT`, `CIRCLE`, `COLOR`, `SCREEN`, `SET PAGE`, `SET SCROLL`, `PUT SPRITE`, `VPOKE`, `SPRITE ASSIGN`)
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_LINE)` (and analogous) is used

#### Scenario: Trap routines use addKernelCall

- **WHEN** the compiler emits code for trap statements (`ON KEY`, `ON SPRITE`, `ON STRIG`, `ON STOP`, `ON INTERVAL`) or trap checking
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_TRAP_ON)` and analogous is used

#### Scenario: Swap routines use addKernelCall

- **WHEN** the compiler emits code for the `SWAP` statement
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_SWAP_INTEGER)` and analogous is used

#### Scenario: Print routines use addKernelCall

- **WHEN** the compiler emits code for PRINT statements (`TAB`, `USING`, comma, semicolon, integer/float/string output)
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_PRINT_TAB)` and analogous is used

#### Scenario: Wrapper table commands use addKernelCall

- **WHEN** the compiler emits code for CMD handlers (`_CLRSCR`, `_DRAW`, `_MTF`, `_SETFNT`, `_WRTVRAM`, etc.)
- **THEN** `optimizer.addKernelCall(DISP_cmd_*)` is used
- **AND** `cpu.addCall(def_cmd_*)` is NOT used

#### Scenario: Wrapper table sprite/tile/player routines use addKernelCall

- **WHEN** the compiler emits code for sprite, tile, or player operations (set/get tile color/pattern/flip/rotate, set/get sprite color/pattern/flip/rotate, player initialize/unhook, usr0-3, vdp_set)
- **THEN** `optimizer.addKernelCall(DISP_set_tile_color)` and analogous is used
- **AND** `cpu.addCall()` with wrapper table addresses is NOT used

#### Scenario: File I/O calls use addKernelCall for dispatch entries

- **WHEN** the compiler emits code for file operations (`OPEN`, `CLOSE`, `INPUT#`, `PRINT#`, `EOF`, `LOC`, `LOF`, `FPOS`, `DSKF`, `MAXFILES`)
- **THEN** `optimizer.addKernelCall(DISP_cmd_fopen)` and analogous is used

#### Scenario: XBASIC core routines use addKernelCall for dispatch entries

- **WHEN** the compiler emits code for XBASIC core routines already in the dispatch table (`INIT`, `END`, `CLS`, `WIDTH`, `LOCATE`, `SCREEN`, `COPY`, `RESTORE`, `READ`, `IREAD`, `PLAY`, `SOUND`, `INPUT`, `TAB`, `USING`, `USR`, `BASE`, `PUT SPRITE`, `PRINT_STR`)
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_INIT)` and analogous is used

#### Scenario: BIOS-level calls use named constants but NOT addKernelCall

- **WHEN** the compiler emits code for BIOS calls (`ENASLT`, `CALBAS`, `DSKBAS`, `EXTROM`, `WRTVDP`, `CHSNS`, `CHGET`, etc.)
- **THEN** `cpu.addCall(def_ENASLT)` (and analogous) is used
- **AND** the constant is a named `#define` in `compiler_hooks.h` (not a magic number like `0x009C`)
- **AND** `addKernelCall` is NOT used for these (they are external stable BIOS addresses, not in the kernel binary)

#### Scenario: No magic number addresses in addCall

- **WHEN** the full C++ codebase is searched for `cpu.addCall(0x`
- **THEN** no matches are found (excluding `0x0000` fixup placeholders)

### Requirement: Compiler emits 8KB-pair segment numbers compatible with all MegaROM mappers
The compiler SHALL emit 8KB-pair segment numbers (2, 4, 6, ...) for all MegaROM modes including ASCII16 and ASCII16X. The ASCII16/ASCII16X kernel SHALL handle the 8KB→16KB conversion at runtime via `srl a` in the patched `MR_CHANGE_SGM` routine. No compiler changes are required for ASCII16/ASCII16X support.

#### Scenario: ASCII16 uses same segment math as ASCII8
- **WHEN** a program is compiled in ASCII16 mode
- **THEN** `segm_last` SHALL start at 2 (same as ASCII8)
- **AND** `segm_last` SHALL increment by 2 per 16KB page (same as ASCII8)
- **AND** the segment-skip preamble SHALL be the standard 8-byte sequence (same as ASCII8)
- **AND** fixup `segm_from`/`segm_to` SHALL use `* 2 + 2` multipliers (same as ASCII8)

#### Scenario: Binary kernel header resolution works for all dispatch entries

- **WHEN** `getKernelCallAddr` is called with `0x0000 + index * 2` for any `DISP_XBASIC_*` index
- **THEN** the 2-byte word at the address in `bin_header_bin` encodes a valid kernel routine address (0x4000-0x7FFF)
- **AND** the resolved target address equals the actual routine address in the kernel binary
