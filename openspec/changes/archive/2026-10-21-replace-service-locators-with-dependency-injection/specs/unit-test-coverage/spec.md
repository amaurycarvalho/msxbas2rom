## MODIFIED Requirements

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

## ADDED Requirements

### Requirement: Unit tests exercise application behavior headlessly

Unit tests that lex, parse, compile, build, or export SHALL use in-memory
adapters for source, kernel, resource, symbol, and ROM persistence. They SHALL
NOT create, read, or delete temporary `.bas`, `.rom`, symbol, or resource files
unless the test explicitly targets an infrastructure or CLI adapter. Tests SHALL
NOT depend on global adapter setup.

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

#### Scenario: Tests do not require global composition

- **WHEN** the unit test binary starts
- **THEN** it SHALL NOT install process-wide default adapters
- **AND** each test SHALL construct the collaborators it needs
