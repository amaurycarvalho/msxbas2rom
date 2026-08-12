## MODIFIED Requirements

### Requirement: Convert parsed BASIC actions into Z80 machine code
As a ROM builder user, the system SHALL convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably.

Compilation SHALL start only when parser output contains tags/actions. The compiler SHALL emit start/end program stubs and generated code for each tag. The compiler SHALL treat `subtype_double_decimal` as a 6-byte type distinct from `subtype_single_decimal` (3 bytes). Double-precision arithmetic, comparisons, and math function renormalization SHALL be emitted as calls to kernel utility routines (DISP 224-231) via `optimizer.addKernelCall()`. Type conversions and simple double operations (negation, zero-initialization, push/pop) SHALL be emitted inline. Duplicate or invalid symbols/flow states (e.g., FOR without NEXT) SHALL be reported as compile errors. DATA/IDATA parser flags SHALL produce corresponding compiler-managed resources. The compiler SHALL track generated code/RAM usage and expose metrics used by the CLI.

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

#### Scenario: Compile double-precision arithmetic via kernel calls
- **WHEN** semantic analysis encounters `A# = B# + C#` with double variables
- **THEN** the compiler emits `optimizer.addKernelCall(DISP_doubleTwoSum)` for the addition
- **AND** the result is stored as 6 bytes in variable A#

#### Scenario: Compile double-precision comparison via kernel call
- **WHEN** semantic analysis encounters `IF A# > B# THEN ...`
- **THEN** the compiler emits `optimizer.addKernelCall(DISP_doubleCompare)` for the comparison
- **AND** the comparison mode byte is set before the call
- **AND** the result follows MSX BASIC convention (0 or 0xFFFF)

#### Scenario: Compile double-precision math function
- **WHEN** semantic analysis encounters `SIN(A#)` with a double argument
- **THEN** the compiler emits inline code for the Taylor correction computation
- **AND** emits `optimizer.addKernelCall(DISP_doubleFastTwoSum)` for renormalization
- **AND** the function strategy handles both high and low parts of the double

#### Scenario: Allocate double variable with 6 bytes
- **WHEN** symbol resolution encounters a `subtype_double_decimal` variable
- **THEN** 6 bytes of RAM are allocated (not 3)
- **AND** the RAM pointer advances by 6

#### Scenario: Allocate double array with x_factor=6
- **WHEN** symbol resolution encounters a double array variable
- **THEN** `x_factor` is 6 (not 3)
- **AND** the array stride and total size reflect 6-byte elements
