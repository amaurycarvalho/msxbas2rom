## MODIFIED Requirements

### Requirement: Convert parsed BASIC actions into Z80 machine code

As a ROM builder user, the system SHALL convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably.

Compilation SHALL start only when parser output contains tags/actions. The compiler SHALL emit start/end program stubs and generated code for each tag. Duplicate or invalid symbols/flow states (e.g., FOR without NEXT) SHALL be reported as compile errors. DATA/IDATA parser flags SHALL produce corresponding compiler-managed resources. The compiler SHALL track generated code/RAM usage and expose metrics used by the CLI. The compiler's DIM statement strategy SHALL validate array parameters pre-computed by the parser rather than recomputing them. Array references in expressions SHALL compute correct element offsets regardless of whether they appear on source lines before or after the DIM declaration.

#### Scenario: Compile valid parser output
- **WHEN** semantic analysis is executed on a parser result with valid tags and actions
- **THEN** compilation succeeds
- **AND** code size is greater than zero
- **AND** RAM usage metrics are available

#### Scenario: Fail on duplicated line symbol mapping
- **WHEN** parser tags map to an already declared line symbol
- **THEN** compilation fails
- **AND** error message indicates line number already declared

#### Scenario: Array reference before DIM line compiles correctly
- **WHEN** a program uses an array on a line before its DIM declaration (e.g., `20 PRINT A%(1)` before `100 DIM A%(2)`)
- **THEN** the compiler emits correct Z80 code for the array access
- **AND** the emitted code computes `element_offset = index * x_factor` using the correct `x_factor` value (set by parser)
- **AND** the array element is accessed at the correct memory offset, not at offset 0

#### Scenario: DIM after array use is not an error
- **WHEN** a program has an array reference before its DIM statement
- **THEN** the compiler does NOT emit "Undeclared array or unknown function" for that reference
- **AND** compilation succeeds

#### Scenario: Compiler validates parser-computed array parameters
- **WHEN** a DIM statement is compiled
- **THEN** the compiler SHALL verify that `array_size` is non-zero
- **AND** SHALL report "Array declaration is missing" if validation fails
