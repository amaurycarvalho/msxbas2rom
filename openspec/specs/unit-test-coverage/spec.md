# Unit Test Coverage

## Purpose

Defines the unit test coverage requirements for the project.

## Requirements

### Requirement: Global line coverage must reach 85%

The project SHALL achieve line coverage of at least 85% across all source files compiled in the unit test suite, as verified by `gcovr` executed via `make coverage` in `tests/unit/`.

#### Scenario: Coverage report passes the threshold
- **WHEN** `make coverage` is executed in `tests/unit/`
- **THEN** gcovr SHALL report line coverage ≥ 85.0%
- **THEN** the command SHALL exit with code 0 (pass --fail-under-line 85)

### Requirement: Compiler SET statement has adequate test coverage

The file `compiler_set_statement_strategy.cpp` SHALL have line coverage ≥ 70% (currently 44.9%, 316/704 lines). Tests MUST cover at least: BGM, ADJUST, BEEP, DISPLAY, PAGE, TITLE, VIDEO, CLIP, PASSWORD, INTERVAL, SCREEN, and SEARCH sub-commands.

#### Scenario: SET BGM compiles correctly
- **WHEN** a .bas program containing `SET BGM,1` is compiled
- **THEN** the compiler SHALL generate valid Z80 opcodes
- **THEN** no error SHALL be reported

#### Scenario: SET VIDEO mode compiles correctly
- **WHEN** a .bas program containing `SET VIDEO,0` is compiled
- **THEN** the compiler SHALL select the correct video mode handler
- **THEN** valid opcodes SHALL be emitted

#### Scenario: SET SCREEN compiles without errors
- **WHEN** a .bas program containing `SET SCREEN,1` is compiled
- **THEN** the compiler SHALL process the screen mode change
- **THEN** valid opcodes SHALL be emitted

### Requirement: Compiler ON statement path coverage expanded

The file `compiler_on_statement_strategy.cpp` SHALL have line coverage ≥ 50% (currently 20.8%, 64/307 lines). Tests MUST cover ON ERROR, ON STOP, ON SPRITE, ON INTERVAL, and ON KEY variants.

#### Scenario: ON ERROR GOTO compiles correctly
- **WHEN** a .bas program containing `ON ERROR GOTO 1000` is compiled
- **THEN** the compiler SHALL generate error handler setup opcodes
- **THEN** the generated code SHALL route to the specified line on error

### Requirement: Compiler graphics statements have expanded coverage

The files `compiler_copy_statement_strategy.cpp`, `compiler_put_statement_strategy.cpp`, `compiler_screen_statement_strategy.cpp`, `compiler_circle_statement_strategy.cpp`, and `compiler_paint_statement_strategy.cpp` SHALL each have line coverage ≥ 65%.

#### Scenario: COPY statement with source-destination compiles
- **WHEN** a .bas program containing `COPY(0,0)-(31,23) TO (0,0)` is compiled
- **THEN** the compiler SHALL generate VRAM-to-VRAM copy opcodes
- **THEN** coordinates SHALL be correctly encoded

#### Scenario: PUT SPRITE statement compiles
- **WHEN** a .bas program containing `PUT SPRITE 0,(10,10),15,0` is compiled
- **THEN** the compiler SHALL generate sprite attribute write opcodes
- **THEN** the sprite pattern and color SHALL be correctly set

### Requirement: Expression evaluator edge cases covered

The file `compiler_expression_evaluator.cpp` SHALL have line coverage ≥ 80% (currently 66%, 464/703 lines). Tests MUST cover: type coercion between integer/float/string, operator precedence chains, function calls within expressions, and unary operators.

#### Scenario: Mixed-type expression coerces correctly
- **WHEN** an expression mixing integer and float operands (e.g., `A% + 3.14`) is compiled
- **THEN** the expression evaluator SHALL correctly promote types
- **THEN** valid opcodes for float addition SHALL be generated

### Requirement: Compiler variable emitter path coverage

The file `compiler_variable_emitter.cpp` SHALL have line coverage ≥ 65% (currently 43.8%, 88/201 lines). Tests MUST cover: scalar assignment, array element assignment, DEFUSR function assignment, and type-checking paths.

#### Scenario: Array element assignment works
- **WHEN** a .bas program containing `A(5) = 42` is compiled
- **THEN** the variable emitter SHALL calculate the correct array offset
- **THEN** the value SHALL be stored at the correct memory address

### Requirement: Resource readers have basic validation coverage

The file `resource_akm_reader.cpp` SHALL have line coverage ≥ 30% (currently 2.5%, 4/159 lines). The file `resource_mtf_map_reader.cpp` SHALL have line coverage ≥ 50% (currently 22.6%, 21/93 lines).

#### Scenario: AKM reader accepts valid minimal file
- **WHEN** a valid minimal AKM binary file is provided
- **THEN** the reader SHALL correctly parse the header and metadata
- **THEN** no error SHALL be returned

#### Scenario: MTF map reader parses tile data
- **WHEN** a valid MTF map file is provided
- **THEN** the reader SHALL extract tile indices correctly
- **THEN** the tile count SHALL match the header specification

### Requirement: All existing tests continue to pass

The system SHALL preserve the behavior and coverage intent of existing test
scenarios across refactors. Tests MAY be migrated to equivalent headless
fixtures, but no test scenario or assertion SHALL be removed without
replacement coverage. All previously passing behaviors SHALL continue to pass,
and the total number of covered behaviors SHALL not decrease.

#### Scenario: Existing test suite unaffected

- **WHEN** `make run` is executed in `tests/unit/` after a test refactor
- **THEN** all previously covered behaviors SHALL continue to be exercised
- **AND** all tests SHALL pass
- **AND** total behavioral coverage SHALL not decrease

#### Scenario: Migrated test preserves its assertions

- **WHEN** a test is migrated to an in-memory fixture
- **THEN** its original success, failure, boundary, and diagnostic assertions
  SHALL be retained or replaced by equivalent assertions

### Requirement: Unit tests exercise application behavior headlessly

Unit tests that lex, parse, compile, build, or export SHALL use in-memory
adapters for source, kernel, resource, symbol, and ROM persistence. They SHALL
NOT create, read, or delete temporary `.bas`, `.rom`, symbol, or resource files
unless the test explicitly targets an infrastructure or CLI adapter. Explicit
file-adapter tests SHALL be limited to the adapter behavior and SHALL use the OS
temporary directory. Tests SHALL NOT depend on global adapter setup or on the
current working directory.

#### Scenario: Compilation test uses no temporary source file

- **WHEN** a unit test compiles a BASIC program
- **THEN** it SHALL supply the program text through an in-memory source
  resolver
- **AND** it SHALL NOT create or delete a temporary `.bas` file

#### Scenario: ROM and symbol assertions are in memory

- **WHEN** a unit test inspects generated ROM bytes or symbol text
- **THEN** it SHALL observe the in-memory ROM artifact or symbol sink
- **AND** it SHALL NOT read or clean up an output file

#### Scenario: File adapters are tested explicitly

- **WHEN** a test targets file-backed source, ROM, resource, or symbol adapters
- **THEN** it SHALL be identified as an adapter test
- **AND** file use SHALL be limited to that explicit adapter scenario
- **AND** it SHALL use an OS temporary directory with automatic cleanup

#### Scenario: Tests do not require global composition

- **WHEN** the unit test binary starts
- **THEN** it SHALL NOT install process-wide default adapters
- **AND** each test SHALL construct the collaborators it needs

### Requirement: End-to-end CLI scenarios live outside the unit binary

Tests that invoke the CLI composition root and exercise real source, ROM, and
symbol files SHALL live in the integration suite, not in the unit-test binary
used for mutation testing. They SHALL retain their exit-code, diagnostic, and
output-file assertions in the integration suite.

#### Scenario: CLI characterization does not run per mutant
- **WHEN** the unit-test binary is executed by the mutation runner
- **THEN** it SHALL NOT invoke the CLI composition root against real files
- **AND** the CLI characterization scenarios SHALL execute in the integration suite

#### Scenario: CLI assertions are preserved after migration
- **WHEN** the CLI characterization scenarios run in the integration suite
- **THEN** normal, quiet, debug, symbol, missing-input, and ASCII8-retry behavior SHALL retain their exit-code and output assertions

### Requirement: File-adapter unit tests use an OS temporary directory

Unit tests that explicitly target a file-backed adapter SHALL create their
files in a unique directory under the operating system temporary directory.
They SHALL NOT create files under the repository or under a path relative to the
current working directory, and they SHALL clean the directory automatically.

#### Scenario: Adapter test leaves no project files
- **WHEN** an explicit file-adapter unit test completes, including after a failed assertion
- **THEN** its temporary directory SHALL be removed
- **AND** no file SHALL remain in the repository tree

#### Scenario: Unit binary is independent of the working directory
- **WHEN** the unit-test binary is executed from the repository root or from `tests/unit`
- **THEN** file-adapter tests SHALL use the same OS temporary location
- **AND** they SHALL NOT create a project-relative `tmp/` directory

### Requirement: Test inventory enforces the unit/integration boundary

The test inventory check SHALL reject filesystem access in unit tests that are
not marked as explicit adapter tests. It SHALL also reject project-relative
temporary paths in adapter tests.

#### Scenario: Non-adapter test performing file I/O is rejected
- **WHEN** a unit test without the file-adapter marker reads or writes a file
- **THEN** the test inventory check SHALL fail and identify the file

#### Scenario: Adapter test using a project-relative temporary path is rejected
- **WHEN** a marked file-adapter test performs file I/O through a project-relative `tmp/` path
- **THEN** the test inventory check SHALL fail and identify the file
