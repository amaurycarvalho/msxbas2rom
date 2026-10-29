# Compiler

## Purpose

Defines the compiler contract: converting parsed BASIC actions into Z80 machine code, kernel calls, and symbol/resource metadata.

## Requirements

### Requirement: Convert parsed BASIC actions into Z80 machine code
The system SHALL convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably. Compilation SHALL start only when parser output contains tags/actions, emit start/end program stubs and per-tag code, and report duplicate or invalid symbols/flow states as compile errors. DATA/IDATA flags SHALL produce compiler-managed resources, and the compiler SHALL track code/RAM usage metrics for the CLI.

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

The compiler SHALL emit all calls to Z80 kernel routines using `CompilerCodeOptimizer::addKernelCall()` and its conditional variants, passing a `DISP_*` dispatch index. The compiler SHALL record the index in the used dispatch set and emit a deferred dispatch fixup; it SHALL NOT embed a hardcoded absolute address for a kernel-internal routine. The compiler SHALL NOT use `cpu.addCall()` for any kernel-internal routine that has a dispatch table entry.

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

### Requirement: USR2-backed functions use kernel dispatch routing

The compiler SHALL emit calls to USR2-backed functions through the kernel
dispatch mechanism, passing the function selector in `HL`. It SHALL NOT emit a
direct absolute call to a resolved `USR2` address or rely on a skipped
instruction to supply the selector.

#### Scenario: TURBO() returns on MSX2+

- **WHEN** `TURBO()` is compiled and executed on `Panasonic_FS-A1WX`
- **THEN** it returns a numeric turbo-state value
- **AND** execution continues to the next statement

#### Scenario: TURBO() returns on turbo R

- **WHEN** `TURBO()` is compiled and executed on `Panasonic_FS-A1GT`
- **THEN** it returns a numeric CPU-mode value
- **AND** execution continues to the next statement

#### Scenario: VDP() and MAKER() remain callable

- **WHEN** `VDP()` or `MAKER()` is compiled and executed on a supported machine
- **THEN** it returns a numeric value
- **AND** does not hang or corrupt the kernel bank mapping

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

### Requirement: Compiler SHALL record used DISP entries and defer dispatch resolution

The compiler SHALL maintain a set of used `DISP_*` indices populated by every kernel-call emission, and SHALL expose that set to the builder so it can compute the dependency closure and generate only the required stubs.

#### Scenario: Used entry is recorded once
- **WHEN** the compiler emits any call to a given `DISP_*` entry
- **THEN** that index SHALL be present in the used dispatch set
- **AND** repeated calls SHALL NOT change the set semantics

#### Scenario: Conditional calls contribute to the used set
- **WHEN** the compiler emits a conditional kernel call
- **THEN** its `DISP_*` index SHALL be recorded in the used dispatch set

#### Scenario: Unused entries are absent
- **WHEN** a program never emits a call to a `DISP_*` entry and no used module depends on it
- **THEN** that index SHALL NOT be present in the used dispatch set

### Requirement: Compiler SHALL emit deferred dispatch fixups

The compiler SHALL emit a 3-byte call placeholder for each kernel call and record a dispatch fixup containing the `DISP_*` index. The builder SHALL resolve the placeholder after module placement.

#### Scenario: Dispatch fixup is recorded
- **WHEN** the compiler emits a kernel call
- **THEN** the call target SHALL be a placeholder
- **AND** a dispatch fixup with the `DISP_*` index SHALL be recorded

#### Scenario: Fixup is applied after packing
- **WHEN** the builder has assigned the stub address for a used `DISP_*` entry
- **THEN** the compiler's dispatch fixup SHALL patch the call placeholder to that stub address

### Requirement: CMD RSCTORAM handler accepts optional offset and size

The `CompilerCmdRscToRamHandler` SHALL accept between 2 and 5 arguments for `CMD RSCTORAM`: `<resource>, <dest>[, <pletter>[, <offset>[, <size>]]]`. It SHALL store all arguments in the `PARM1` work area for the kernel routine and SHALL resolve omitted `pletter` (default `0`), omitted `offset` (default `0`), and omitted `size` (default `0`, meaning the full remaining resource). The `PARM1` offsets SHALL be shared with the `RESOURCE(n)` compiler strategy so both call sites use the same layout.

#### Scenario: Two-argument form uses defaults

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>`
- **THEN** the emitted code SHALL store `pletter = 0`, `offset = 0`, and `size = 0` in the `PARM1` work area
- **AND** the emitted code SHALL call `cmd_rsctoram`

#### Scenario: Three-argument form uses offset and size defaults

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>, <pletter>`
- **THEN** the emitted code SHALL store the provided `pletter`, `offset = 0`, and `size = 0`

#### Scenario: Four-argument form uses the size default

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>, <pletter>, <offset>`
- **THEN** the emitted code SHALL store the provided `pletter` and `offset`, and `size = 0`

#### Scenario: Five-argument form passes offset and size

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>, <pletter>, <offset>, <size>`
- **THEN** the emitted code SHALL store the provided `pletter`, `offset`, and `size` values in the `PARM1` work area

#### Scenario: Too many arguments is a syntax error

- **WHEN** the compiler processes `CMD RSCTORAM` with more than five arguments
- **THEN** compilation SHALL fail with a `CMD RSCTORAM syntax error`

#### Scenario: Existing three-argument form is unchanged

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>, <pletter>`
- **THEN** the emitted code SHALL use `offset = 0` and `size = 0` (full remaining resource)
- **AND** the generated call SHALL be compatible with the previous behavior

### Requirement: RESOURCE(n) compiler strategy delegates to CMD RSCTORAM

The C++ `RESOURCE(n)` function strategy SHALL emit code that stores the `CMD RSCTORAM` parameters and calls `DISP_cmd_rsctoram`. It SHALL evaluate and cast `n`, store it as the resource number, then use `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` (`HEAPSIZ`). It SHALL NOT emit a call to `DISP_usr0`. The `PARM1` offsets SHALL be shared with `CompilerCmdRscToRamHandler`.

#### Scenario: RESOURCE(n) does not call usr0

- **WHEN** the compiler processes an expression containing `RESOURCE(<expr>)`
- **THEN** the generated code SHALL contain a `call` to the `cmd_rsctoram` dispatch target
- **AND** the generated code SHALL NOT contain a `call` to `usr0`

#### Scenario: RESOURCE(n) parameter setup

- **WHEN** the compiler processes `RESOURCE(<expr>)`
- **THEN** the generated code SHALL store `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` in the `PARM1` work area before the `cmd_rsctoram` call
- **AND** the argument `<expr>` SHALL be evaluated and cast to numeric before being stored as the resource number
- **AND** storing the resource number SHALL happen before `HL` is overwritten by the `HEAP()`/`FRE()` setup

#### Scenario: RESOURCE(n) uses the shared PARM1 layout

- **WHEN** the compiler emits `RESOURCE(<expr>)` and `CMD RSCTORAM`
- **THEN** both SHALL use the same resource, destination, pletter, offset, and size offsets

#### Scenario: RESOURCE(n) result type

- **WHEN** the compiler processes `RESOURCE(<expr>)`
- **THEN** the function result type SHALL be numeric

### Requirement: String-function results use distinct temporary storage

When a string expression evaluates more than one string-function result, each result SHALL remain valid until the enclosing expression finishes. Concatenating two `STRING$` results SHALL preserve both operands in evaluation order, even when both calls use the same character and lengths differ.

#### Scenario: Two STRING$ results are concatenated

- **WHEN** `PRINT STRING$(5,65)+STRING$(6,66)` is compiled and executed
- **THEN** the output contains five `A` characters followed by six `B` characters
- **AND** it does not contain the second result duplicated in place of the first

#### Scenario: STRING$ results from variables are concatenated

- **WHEN** `X%=5`, `Y%=1`, and `PRINT STRING$(X%,65)+STRING$(6-Y%,66)` is executed
- **THEN** the output contains five `A` characters followed by five `B` characters

#### Scenario: STRING$ results survive nested concatenation

- **WHEN** a concatenation combines string-function results with literals or other string functions
- **THEN** every operand contributes its own characters in evaluation order
- **AND** no earlier result is overwritten by a later function call

### Requirement: Startup selects a deterministic 40-column text mode

The generated ROM startup SHALL explicitly select `SCREEN 0` and `WIDTH 40`, regardless of the machine's boot default. A `PRINT` statement without a trailing `;` or `,` SHALL leave the cursor at column 0 of the following row.

#### Scenario: Consecutive PRINT lines start at the left margin

- **WHEN** a program executes `PRINT "HELLO"` followed by `PRINT "WORLD"`
- **THEN** `HELLO` and `WORLD` appear on consecutive rows
- **AND** both strings begin at the screen's left margin
- **AND** the visible text does not drift or wrap because of a width mismatch

#### Scenario: Startup screen layout matches the text width

- **WHEN** a program captures the terminal text state without issuing `SCREEN` or `WIDTH`
- **THEN** the captured name table and text width correspond to the standard 40-column `SCREEN 0` layout
- **AND** the ROM does not select a 32-column VDP layout while writing 40-column text

#### Scenario: The same 40-column layout is used on every machine class

- **WHEN** the same program runs on MSX1, MSX2, MSX2+, and turbo R machines
- **THEN** it uses the same 40-column `SCREEN 0` text layout
- **AND** the captured name table starts at `0x0000`
- **AND** the machine's non-Japanese/Japanese boot default does not change the output columns

### Requirement: STRING$ enforces the shared function arity bound

`STRING$` SHALL reject five or more arguments with a syntax error before evaluating or storing argument subtypes, consistent with the shared function arity policy. Its valid two-argument form SHALL continue to compile.

#### Scenario: Valid STRING$ compiles

- **WHEN** `A$=STRING$(3,65)` is compiled
- **THEN** compilation succeeds

#### Scenario: STRING$ with five arguments is rejected

- **WHEN** `A$=STRING$(3,65,1,2,3)` is compiled
- **THEN** compilation fails with a syntax error
- **AND** the compiler does not evaluate or store the excess arguments
