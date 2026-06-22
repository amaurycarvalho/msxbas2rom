## ADDED Requirements

### Requirement: Compiler emits kernel calls via addKernelCall
The compiler SHALL emit all calls to Z80 kernel routines (addresses in the range 0x4000-0x7FFF, corresponding to `bin_header_bin`) using `CompilerCodeOptimizer::addKernelCall()`, which resolves target addresses by reading the `jp` entries from the binary image of the wrapper table. The compiler SHALL NOT use `cpu.addCall()` for any kernel-internal address that has a wrapper table entry.

#### Scenario: Integer comparison operators use addKernelCall
- **WHEN** the compiler emits code for integer comparison (`=`, `<>`, `<`, `<=`, `>`, `>=`)
- **THEN** `optimizer.addKernelCall(def_intCompareEQ)` (and analogous) is used
- **AND** `cpu.addCall()` is not used for these kernel routines

#### Scenario: Float comparison operators use addKernelCall
- **WHEN** the compiler emits code for float comparison
- **THEN** `optimizer.addKernelCall(def_XBASIC_COMPARE_FLOATS_EQ)` (and analogous) is used

#### Scenario: XBASIC math functions use addKernelCall
- **WHEN** the compiler emits code for math functions (`SIN`, `COS`, `TAN`, `ATN`, `EXP`, `LOG`, `SQR`, `RND`, `INT`, `FIX`, `SGN`, `ABS`)
- **THEN** `optimizer.addKernelCall(def_XBASIC_SIN)` (and analogous) is used

#### Scenario: XBASIC string functions use addKernelCall
- **WHEN** the compiler emits code for string functions (`LEFT$`, `RIGHT$`, `MID$`, `INSTR`, `VAL`, `STRING$`, `BIN$`, `OCT$`, `HEX$`)
- **THEN** `optimizer.addKernelCall(def_XBASIC_LEFT)` (and analogous) is used

#### Scenario: XBASIC type cast routines use addKernelCall
- **WHEN** the compiler emits code for type casts (integer to float, float to integer, numeric to string, string to numeric)
- **THEN** `optimizer.addKernelCall(def_XBASIC_CAST_INTEGER_TO_FLOAT)` (and analogous) is used

#### Scenario: Arithmetic operators use addKernelCall for floats
- **WHEN** the compiler emits code for float addition, subtraction, multiplication, or division
- **THEN** `optimizer.addKernelCall(def_XBASIC_ADD_FLOATS)` (and analogous) is used

#### Scenario: String concatenation and copy use addKernelCall
- **WHEN** the compiler emits code for string concatenation or copy
- **THEN** `optimizer.addKernelCall(def_XBASIC_CONCAT_STRINGS)` or `optimizer.addKernelCall(def_XBASIC_COPY_STRING)` is used

#### Scenario: Graphics statements use addKernelCall
- **WHEN** the compiler emits code for graphics statements (`LINE`, `BOX`, `BOXF`, `PSET`, `PAINT`, `CIRCLE`, `COLOR`, `SCREEN`, `SET PAGE`, `SET SCROLL`, `PUT SPRITE`, `VPOKE`)
- **THEN** `optimizer.addKernelCall(def_XBASIC_LINE)` (and analogous) is used

#### Scenario: Trap routines use addKernelCall
- **WHEN** the compiler emits code for trap statements (`ON KEY`, `ON SPRITE`, `ON STRIG`, `ON STOP`, `ON INTERVAL`) or trap checking
- **THEN** `optimizer.addKernelCall(def_XBASIC_TRAP_ON)` and analogous is used

#### Scenario: Swap routines use addKernelCall
- **WHEN** the compiler emits code for the `SWAP` statement
- **THEN** `optimizer.addKernelCall(def_XBASIC_SWAP_INTEGER)` and analogous is used

#### Scenario: Wrapper table commands use addKernelCall
- **WHEN** the compiler emits code for CMD handlers (`_CLRSCR`, `_DRAW`, `_MTF`, `_SETFNT`, `_WRTVRAM`, etc.)
- **THEN** `optimizer.addKernelCall(def_cmd_*)` is used
- **AND** `cpu.addCall(def_cmd_*)` is NOT used

#### Scenario: Wrapper table sprite/tile/player routines use addKernelCall
- **WHEN** the compiler emits code for sprite, tile, or player operations (set/get tile color/pattern/flip/rotate, set/get sprite color/pattern/flip/rotate, player initialize/unhook, usr0-3, vdp_set)
- **THEN** `optimizer.addKernelCall(def_set_tile_color)` and analogous is used
- **AND** `cpu.addCall()` with wrapper table addresses is NOT used

#### Scenario: File I/O calls use addKernelCall for wrapper entries
- **WHEN** the compiler emits code for file operations (`OPEN`, `CLOSE`, `INPUT#`, `PRINT#`, `EOF`, `LOC`, `LOF`, `FPOS`, `DSKF`, `MAXFILES`)
- **THEN** `optimizer.addKernelCall(def_cmd_fopen)` and analogous is used

#### Scenario: BIOS-level calls use named constants but NOT addKernelCall
- **WHEN** the compiler emits code for BIOS calls (`ENASLT`, `CALBAS`, `DSKBAS`, `EXTROM`, `WRTVDP`, `CHSNS`, `CHGET`, etc.)
- **THEN** `cpu.addCall(def_ENASLT)` (and analogous) is used
- **AND** the constant is a named `#define` in `compiler_hooks.h` (not a magic number like `0x009C`)
- **AND** `addKernelCall` is NOT used for these (they are external stable BIOS addresses, not in the kernel binary)

#### Scenario: No magic number addresses in addCall
- **WHEN** the full C++ codebase is searched for `cpu.addCall(0x`
- **THEN** no matches are found (excluding `0x0000` fixup placeholders)

#### Scenario: Binary kernel header resolution works for all new entries
- **WHEN** `getKernelCallAddr` is called with any new wrapper table address
- **THEN** the byte at the address in `bin_header_bin` is `0xC3` (jp opcode)
- **AND** the resolved target address equals the actual routine address in the kernel binary
