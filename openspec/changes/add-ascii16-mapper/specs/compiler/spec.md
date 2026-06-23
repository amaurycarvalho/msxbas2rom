## MODIFIED Requirements

### Requirement: Convert parsed BASIC actions into Z80 machine code
As a ROM builder user, the system SHALL convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably.

Compilation SHALL start only when parser output contains tags/actions. The compiler SHALL emit start/end program stubs and generated code for each tag. Duplicate or invalid symbols/flow states (e.g., FOR without NEXT) SHALL be reported as compile errors. DATA/IDATA parser flags SHALL produce corresponding compiler-managed resources. The compiler SHALL track generated code/RAM usage and expose metrics used by the CLI.

For MegaROM mode, the compiler SHALL split compiled code across segments. In ASCII8, Konami4, and KonamiSCC modes, each 16KB ROM page SHALL contain two 8KB segments, with segment numbering in pairs (0/1, 2/3, 4/5, ...). In ASCII16 mode, each 16KB ROM page SHALL contain one 16KB segment, with segment numbering sequential (0, 1, 2, 3, ...).

For ASCII16 mode:
- `segm_last` SHALL start at 1 (segment 0 is the kernel page)
- `segm_last` SHALL increment by 1 per 16KB page (not by 2)
- `segm_total` SHALL round to the next multiple of 8 segments per 128KB boundary: `((segm_last + 1) / 8 + 1) * 8`
- Each code item's `segm` SHALL always equal `segm_last` (no 0xA000 split)
- Cross-segment fixup resolution SHALL compute segment indices with `((address - 0x8000) / 0x4000) * 1 + 1` (instead of `* 2 + 2`)
- The segment-skip preamble for code crossing a 16KB boundary SHALL be the minimum sequence (`ld a,seg; jp MR_JUMP`), without the `ld hl, 0x8000` used in 8KB modes

These ASCII16-specific behaviors SHALL be implemented as inline conditional branches on `compileMode == ASCII16` at each affected site.

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

#### Scenario: ASCII16 segment numbering starts at 1
- **WHEN** a program is compiled in ASCII16 mode
- **THEN** `segm_last` SHALL be initialized to 1 (not 2)
- **AND** the first compiled code segment SHALL be segment 1

#### Scenario: ASCII16 segments increment by 1 per page
- **WHEN** compiled code exceeds the 16KB boundary in ASCII16 mode
- **THEN** `segm_last` SHALL increment by 1 (not by 2)
- **AND** the next code segment SHALL be segm_last + 1

#### Scenario: ASCII16 code skip preamble is minimal
- **WHEN** a code item crosses a segment boundary in ASCII16 mode
- **THEN** the segment-skip preamble SHALL be `{0x3E, segm, 0xC3, lo(MR_JUMP), hi(MR_JUMP)}` (5 bytes)
- **AND** the preamble SHALL NOT include `ld hl, 0x8000`

#### Scenario: ASCII16 fixup uses 1:1 segment math
- **WHEN** cross-segment fixup resolution runs in ASCII16 mode
- **THEN** `segm_from` SHALL be calculated as `((address + start_address - 0x8000) / 0x4000) * 1 + 1`
- **AND** `segm_to` SHALL use the same formula with the target address
- **AND** the multiplier SHALL be 1, not 2 as in ASCII8 mode

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
- **THEN** `optimizer.addKernelCall(DISP_XBASIC_CONCAT_STRINGS)` (and analogous) is used
