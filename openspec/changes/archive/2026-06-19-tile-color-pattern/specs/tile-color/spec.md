## ADDED Requirements

### Requirement: SET TILE COLOR supports 3 syntax forms
The system SHALL support SET TILE COLOR with three distinct syntax forms for writing tile color data: simple foreground/background pair, per-row arrays, and integer buffer array.

The tile color table SHALL be 8 bytes per tile (one per row), each byte storing FC in the high nibble and BC in the low nibble (`FC<<4 | BC`).

#### Scenario: Simple FC/BC form
- **WHEN** `SET TILE COLOR 5, 1, 15` is compiled and executed
- **THEN** all 8 rows of tile 5 have FC=1, BC=15 across all 3 banks

#### Scenario: Simple FC/BC form with specific bank
- **WHEN** `SET TILE COLOR 5, 1, 15, 1` is compiled and executed
- **THEN** all 8 rows of tile 5 have FC=1, BC=15 in bank 1 only

#### Scenario: Per-row FC array (no BC)
- **WHEN** `SET TILE COLOR 5, (1,2,3,4,5,6,7,8)` is compiled and executed
- **THEN** row 0 of tile 5 has FC=1 (BC unchanged from previous value), row 1 has FC=2, etc., across all 3 banks

#### Scenario: Per-row FC and BC arrays
- **WHEN** `SET TILE COLOR 5, (1,2,3,4,5,6,7,8), (15,14,13,12,11,10,9,8)` is compiled and executed
- **THEN** row 0 has FC=1, BC=15; row 1 has FC=2, BC=14; etc., across all 3 banks

#### Scenario: Per-row arrays with bank
- **WHEN** `SET TILE COLOR 5, (1,2,3,4,5,6,7,8), (15,14,13,12,11,10,9,8), 2` is compiled and executed
- **THEN** the color data is applied to bank 2 only

#### Scenario: Buffer array form
- **WHEN** `DIM C(3):C(0)=&H1F26:C(1)=&H3443:C(2)=&H5B6A:C(3)=&H7C8D:SET TILE COLOR 5, C()` is compiled and executed
- **THEN** tile 5 has colors FC=2,BC=6 (row 0); FC=1,BC=15 (row 1); FC=4,BC=3 (row 2); FC=3,BC=4 (row 3); FC=6,BC=11 (row 4); FC=5,BC=11 (row 5); FC=8,BC=13 (row 6); FC=7,BC=12 (row 7); across all 3 banks
- **AND** each 16-bit integer packs 2 rows: low byte = even row (FC<<4|BC), high byte = odd row (FC<<4|BC)

#### Scenario: Buffer array with bank
- **WHEN** `SET TILE COLOR 5, C(), 1` is compiled and executed
- **THEN** the buffer data is written to bank 1 only

### Requirement: GET TILE COLOR reads tile color data
The system SHALL support GET TILE COLOR for reading tile color data into an integer buffer array.

#### Scenario: Read tile color to buffer
- **WHEN** `DIM C(3):GET TILE COLOR 5, C()` is compiled and executed
- **THEN** the 8 bytes of tile 5 color data (bank 0) are stored in C(0)..C(3), where each 16-bit integer packs 2 rows (low byte = even row, high byte = odd row)

#### Scenario: Read tile color from specific bank
- **WHEN** `GET TILE COLOR 5, C(), 2` is compiled and executed
- **THEN** the color data is read from bank 2 (not the default bank 0)

### Requirement: Screen mode validation
The SET and GET TILE COLOR commands SHALL validate the current screen mode before operating. In screen modes where tile color tables are not defined (mode 0, mode ≥5), the operation SHALL be silently ignored (no-op).

#### Scenario: No-op in unsupported screen modes
- **WHEN** `SET TILE COLOR 5, 1, 15` is executed in screen mode 0
- **THEN** no VRAM write occurs

### Requirement: Error handling for invalid syntax
The parser SHALL reject invalid combinations of parameters with a clear syntax error message.

#### Scenario: Wrong parameter count
- **WHEN** `SET TILE COLOR 5` (missing color parameters) is parsed
- **THEN** a syntax error is raised

#### Scenario: Invalid bank value
- **WHEN** `SET TILE COLOR 5, 1, 15, 5` (bank out of range 0-3) is compiled and executed
- **THEN** the kernel SHALL treat the bank value modulo 4, or silently ignore the operation

### Requirement: Unit tests verify compiler translation
The compiler unit tests in tests/unit/src/test_compiler.cpp SHALL include test cases that verify all TILE COLOR syntax forms compile without errors and produce the expected Z80 output structure.

#### Scenario: All 6 SET syntax forms compile successfully
- **WHEN** each SET TILE COLOR syntax form (fc/bc, fc/bc+bank, fc-array, fc+bc-arrays, buffer, buffer+bank) is passed to the compiler as a complete BASIC program
- **THEN** the compiler returns success

#### Scenario: Both GET syntax forms compile successfully
- **WHEN** GET TILE COLOR with buffer (default bank) and GET TILE COLOR with buffer and explicit bank are passed to the compiler
- **THEN** the compiler returns success

### Requirement: Integration test validates end-to-end execution
A BASIC integration test program SHALL be created at tests/integration/GRAPH/test97.bas that exercises all SET and GET TILE COLOR syntax forms and prints results for visual verification in an emulator or real hardware.

#### Scenario: All syntax forms execute without runtime errors
- **WHEN** `test97.bas` is compiled and executed on MSX BASIC (screen 2, tile mode on)
- **THEN** all SET TILE COLOR and GET TILE COLOR commands complete without crashing, and printed color values are correct
