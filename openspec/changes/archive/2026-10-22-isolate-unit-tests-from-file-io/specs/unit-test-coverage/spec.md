## ADDED Requirements

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

## MODIFIED Requirements

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
