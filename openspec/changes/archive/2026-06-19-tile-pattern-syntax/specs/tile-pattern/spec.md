## ADDED Requirements

### Requirement: SET TILE PATTERN supports inline and array syntax forms
The system SHALL support SET TILE PATTERN with two syntax forms for writing tile pattern data: inline 8-byte list and 4-integer buffer array. Both SHALL accept an optional bank parameter (0-2, default 3=all banks).

The tile pattern table SHALL be 8 bytes per tile, one byte per row, each byte defining the 8-pixel bitmap for that row.

#### Scenario: Inline form without bank
- **WHEN** `SET TILE PATTERN 65, (255,129,129,129,129,129,129,255)` is compiled and executed in screen 2
- **THEN** tile 65 has pattern: row0=%11111111, row1=%10000001, ..., row7=%11111111 across all 3 banks

#### Scenario: Inline form with specific bank
- **WHEN** `SET TILE PATTERN 66, (255,0,255,0,255,0,255,0), 1` is compiled and executed
- **THEN** the pattern data is written to bank 1 only (rows 0-3 of bank 1 pattern table)

#### Scenario: Inline form in screen 0
- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8)` is compiled and executed in screen 0
- **THEN** tile 65 in the screen 0 pattern table (at 0x0800) has the specified rows; bank parameter is ignored

#### Scenario: 4-integer array form without bank
- **WHEN** `DIM P%(3):P%(0)=&H0100:P%(1)=&H0302:P%(2)=&H0504:P%(3)=&H0706` and `SET TILE PATTERN 65, P%()` is compiled and executed
- **THEN** tile 65 has row0=0, row1=1, row2=2, row3=3, row4=4, row5=5, row6=6, row7=7 across all 3 banks
- **AND** each 16-bit integer packs 2 rows: low byte = even row, high byte = odd row

#### Scenario: 4-integer array form with specific bank
- **WHEN** `SET TILE PATTERN 65, P%(), 2` is compiled and executed
- **THEN** the pattern data is written to bank 2 only

### Requirement: GET TILE PATTERN reads tile pattern data
The system SHALL support GET TILE PATTERN for reading tile pattern data into a 4-integer buffer array, with optional bank select (default 0).

#### Scenario: Read tile pattern to buffer
- **WHEN** `DIM Q%(3):GET TILE PATTERN 65, Q%()` is compiled and executed after writing known pattern data
- **THEN** the 8 bytes of tile 65 pattern data (bank 0) are stored in Q%(0)..Q%(3), where each 16-bit integer packs 2 rows (low byte = even row, high byte = odd row)

#### Scenario: Read tile pattern from specific bank
- **WHEN** `GET TILE PATTERN 65, Q%(), 1` is compiled and executed
- **THEN** the pattern data is read from bank 1 (not the default bank 0)

### Requirement: Screen mode validation
The SET and GET TILE PATTERN commands SHALL validate the current screen mode before operating. In screen modes where pattern tables are not applicable (mode ≥ 5), the operation SHALL be silently ignored.

#### Scenario: No-op in unsupported screen modes
- **WHEN** `SET TILE PATTERN 65, (255,0,255,0,255,0,255,0)` is executed in screen mode 5
- **THEN** no VRAM write occurs

#### Scenario: Screen mode 0 handling
- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8), 2` is executed in screen mode 0
- **THEN** the bank parameter is ignored; pattern is written to screen 0's single pattern table

### Requirement: Error handling for invalid syntax
The parser SHALL reject invalid parameter combinations with a clear syntax error message.

#### Scenario: Wrong parameter count
- **WHEN** `SET TILE PATTERN 65` (missing pattern data) is parsed
- **THEN** a syntax error is raised

#### Scenario: Invalid bank value
- **WHEN** `SET TILE PATTERN 65, (255), 5` (bank out of range) is compiled and executed
- **THEN** the kernel SHALL treat the bank value as 3 (all banks) or silently ignore, matching behavior of SET TILE COLOR

### Requirement: Backward compatibility with inline list form
The system SHALL support the existing inline list syntax `SET TILE PATTERN <n>, (<l0>,...<l7>)` without requiring a bank parameter (defaults to all banks).

#### Scenario: Existing inline syntax still works
- **WHEN** an existing program uses `SET TILE PATTERN 65, (255,129,129,129,129,129,129,255)` (no bank)
- **THEN** it compiles and executes identically to previous behavior (writes to all banks)
