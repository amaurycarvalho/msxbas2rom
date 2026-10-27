# tile-commands Specification

## Purpose

Defines how MSX tile commands select VRAM banks and validate parameters, so tile pattern, color, flip, and rotate operations behave consistently and never compute invalid bank addresses.

## Requirements

### Requirement: SET tile commands treat bank values above 2 as all banks

`SET TILE PATTERN`, `SET TILE COLOR`, `SET TILE FLIP`, and `SET TILE ROTATE` SHALL interpret bank values `0`, `1`, and `2` as a single bank and any bank value at or above `3` as all three banks.

#### Scenario: Bank 3 means all banks

- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8), 3` is compiled and executed in SCREEN 2
- **THEN** the same pattern is written to banks 0, 1, and 2

#### Scenario: Bank above 3 means all banks

- **WHEN** `SET TILE COLOR 65, 1, 15, 9` is compiled and executed in SCREEN 2
- **THEN** the same color is written to banks 0, 1, and 2

#### Scenario: Very large bank means all banks

- **WHEN** `SET TILE ROTATE 65, 0, 200` is compiled and executed in SCREEN 2
- **THEN** the rotated pattern is written to banks 0, 1, and 2

#### Scenario: Banks 0 to 2 remain specific

- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8), 1` is compiled and executed in SCREEN 2
- **THEN** only bank 1 is written

### Requirement: GET tile commands normalize bank values above 2 to bank 0

`GET TILE PATTERN` and `GET TILE COLOR` SHALL read bank `0` when the requested bank value is at or above `3`, because a single read buffer cannot represent all banks.

#### Scenario: GET with bank 3 reads bank 0

- **WHEN** `GET TILE PATTERN 65, Q%(), 3` is compiled and executed after banks 0, 1, and 2 hold different patterns
- **THEN** `Q%()` contains the bank 0 pattern

#### Scenario: GET with a large bank reads bank 0

- **WHEN** `GET TILE COLOR 65, C%(), 200` is compiled and executed after banks 0, 1, and 2 hold different colors
- **THEN** `C%()` contains the bank 0 color data

#### Scenario: GET with banks 0 to 2 reads that bank

- **WHEN** `GET TILE PATTERN 65, Q%(), 2` is executed
- **THEN** `Q%()` contains the bank 2 pattern

### Requirement: Tile buffer parameters must be declared arrays

A tile buffer parameter SHALL be accepted only when it refers to an array declared with `DIM`. Literals, scalar variables, and undeclared identifiers SHALL be rejected for buffer forms.

#### Scenario: Declared array buffer is accepted

- **WHEN** `10 DIM PB%(3)` and `20 SET TILE PATTERN 65, PB%()` are compiled
- **THEN** compilation succeeds and the command uses `PB%()` as the buffer

#### Scenario: Literal destination is rejected

- **WHEN** `GET TILE PATTERN 65, 1` is compiled
- **THEN** compilation fails with a `GET TILE PATTERN` error

#### Scenario: Scalar destination is rejected

- **WHEN** `10 A=1` and `20 GET TILE COLOR 5, A` are compiled
- **THEN** compilation fails with a `GET TILE COLOR` error

### Requirement: SET TILE COLOR distinguishes scalar colors from buffers

`SET TILE COLOR` SHALL treat a non-array identifier in the color position as a scalar foreground or background value, not as a buffer pointer. Only a declared array SHALL select the buffer form.

#### Scenario: Scalar FC and BC variables

- **WHEN** `10 F%=1:G%=15` and `20 SET TILE COLOR 5, F%, G%` are compiled
- **THEN** compilation uses the scalar foreground/background path with values `F%` and `G%`

#### Scenario: Scalar FC variable only

- **WHEN** `10 F%=1` and `20 SET TILE COLOR 5, F%` are compiled
- **THEN** compilation uses the scalar foreground path instead of reading eight bytes from `F%`

#### Scenario: Array buffer still selects buffer form

- **WHEN** `10 DIM C%(3)` and `20 SET TILE COLOR 5, C%()` are compiled
- **THEN** compilation uses the buffer path

### Requirement: Bank parameters must be scalar expressions

A tile command SHALL reject an array or buffer as its bank parameter. A scalar variable used as a bank SHALL be honored in every form that accepts a bank.

#### Scenario: Array bank is rejected

- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8), (1,2)` is compiled
- **THEN** compilation fails with a `SET TILE PATTERN` error

#### Scenario: Variable bank is honored with an FC array

- **WHEN** `10 BK%=2` and `20 SET TILE COLOR 5, (1,2,3,4), BK%` are compiled
- **THEN** the generated code uses `BK%` as the bank instead of defaulting to all banks

### Requirement: SET TILE COLOR bank forms preserve their color operands

When `SET TILE COLOR` includes a bank parameter, the compiler SHALL use the supplied FC and BC scalars or arrays as color data and SHALL use only the final parameter as the bank.

#### Scenario: Simple FC/BC with bank

- **WHEN** `SET TILE COLOR 66, 2, 8, 1` is compiled and executed
- **THEN** bank 1 of tile 66 receives `0x28`, not `0x18`

#### Scenario: FC array with bank

- **WHEN** `SET TILE COLOR 69, (8,7,6,5,4,3,2,1), 2` is compiled and executed
- **THEN** bank 2 of tile 69 receives `80 70 60 50 40 30 20 10`

#### Scenario: FC and BC arrays with bank

- **WHEN** `SET TILE COLOR 68, (1,2,3,4,5,6,7,8), (15,14,13,12,11,10,9,8), 1` is compiled and executed
- **THEN** bank 1 of tile 68 receives `1F 2E 3D 4C 5B 6A 79 88`

### Requirement: Tile row data is limited to eight rows

Inline tile tuples and per-row color arrays SHALL accept at most eight rows. Supplied rows beyond the eighth SHALL be rejected so that adjacent tiles are never overwritten.

#### Scenario: Nine pattern rows are rejected

- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8,9)` is compiled
- **THEN** compilation fails with a `SET TILE PATTERN` error

#### Scenario: Nine color rows are rejected

- **WHEN** `SET TILE COLOR 5, (1,2,3,4,5,6,7,8,9)` is compiled
- **THEN** compilation fails with a `SET TILE COLOR` error

#### Scenario: Partial rows remain valid

- **WHEN** `SET TILE COLOR 5, (1,2,3,4)` is compiled
- **THEN** compilation succeeds and updates only rows 0 through 3

### Requirement: Inline SET TILE PATTERN rejects extra parameters

`SET TILE PATTERN` with an inline tuple SHALL accept only a tile number, the tuple, and an optional bank. Additional parameters SHALL be rejected instead of compiling a no-op.

#### Scenario: Extra parameter is rejected

- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8), 1, 2` is compiled
- **THEN** compilation fails with a `SET TILE PATTERN` error

#### Scenario: Valid inline form still compiles

- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8), 2` is compiled
- **THEN** compilation succeeds

### Requirement: Tile color nibbles are masked to four bits

Foreground and background color values SHALL be masked to four bits before being packed into a tile color byte, so out-of-range values cannot corrupt the neighboring nibble.

#### Scenario: Background value above 15 is masked

- **WHEN** `SET TILE COLOR 5, 1, 16` is executed
- **THEN** the written color byte is `0x10`, not `0x11`

#### Scenario: Foreground value above 15 is masked

- **WHEN** `SET TILE COLOR 5, 17, 15` is executed
- **THEN** the written color byte is `0x1F`

### Requirement: SET TILE FLIP and ROTATE validate literal directions

`SET TILE FLIP` and `SET TILE ROTATE` SHALL accept literal directions `0`, `1`, and `2`. Literal direction values outside that range SHALL be rejected at compile time.

#### Scenario: Literal direction 3 is rejected

- **WHEN** `SET TILE FLIP 65, 3` is compiled
- **THEN** compilation fails with a `SET TILE FLIP` error

#### Scenario: Literal directions 0 to 2 compile

- **WHEN** `SET TILE ROTATE 65, 2` is compiled
- **THEN** compilation succeeds

### Requirement: Single-bank screen modes ignore tile pattern bank selection

When the current screen mode is 0 or 1, tile pattern operations SHALL use a single bank and SHALL ignore any requested bank value.

#### Scenario: SCREEN 1 pattern write ignores bank

- **WHEN** `SET TILE PATTERN 65, (1,2,3,4,5,6,7,8), 2` is executed in SCREEN 1
- **THEN** the pattern is written to the single SCREEN 1 pattern table

#### Scenario: SCREEN 1 pattern read ignores bank

- **WHEN** `GET TILE PATTERN 65, Q%(), 2` is executed in SCREEN 1
- **THEN** the pattern is read from the single SCREEN 1 pattern table

#### Scenario: SCREEN 1 flip ignores bank

- **WHEN** `SET TILE FLIP 65, 0, 2` is executed in SCREEN 1
- **THEN** only the single SCREEN 1 pattern table is updated

#### Scenario: SCREEN 0 color write remains a no-op

- **WHEN** `SET TILE COLOR 65, 1, 15, 2` is executed in SCREEN 0
- **THEN** no VRAM write occurs

### Requirement: Affected tile integration programs use deterministic VRAM probes

The regression suite SHALL execute `test87`, `test88`, `test94`, `test97`, `tile_flip_rotate`, and `tile_bank_all` and SHALL validate their results with deterministic VRAM probes. These programs SHALL remain marked volatile so no baseline is required.

#### Scenario: Affected tile programs are included in GRAPH regression

- **WHEN** the GRAPH regression scenario runs
- **THEN** `test87`, `test88`, `test94`, `test97`, `tile_flip_rotate`, and `tile_bank_all` are executed
- **AND** their configured VRAM probes are evaluated

#### Scenario: Probes are evaluated without a baseline

- **WHEN** an affected tile program is marked volatile and declares VRAM probes
- **THEN** the runner evaluates the probes without loading a baseline file
