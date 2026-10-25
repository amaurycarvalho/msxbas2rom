# Testable Compilation Boundaries Specification

## Purpose

Defines testable compilation boundaries so compiler behavior can run with
in-memory collaborators while operating-system and kernel details stay outside
the application core.

## Requirements

### Requirement: BASIC source is accepted independently of filesystem storage

The compilation workflow SHALL accept BASIC source content with a logical
source name independently of how that content is stored. File-backed input
SHALL be supplied by an outer adapter. Unit tests for lexing, parsing, and
compilation SHALL be able to use in-memory source without creating a
temporary `.bas` file.

#### Scenario: Compile an in-memory BASIC program
- **WHEN** a caller provides BASIC text and a logical source name
- **THEN** lexical analysis, parsing, compilation, and diagnostics SHALL
  behave as they do for the equivalent file-backed program
- **AND** no source file SHALL be created or read by that scenario

### Requirement: Include resolution is supplied through a source boundary

When a BASIC source requests another source through `INCLUDE`, the workflow
SHALL resolve it through the same source boundary as the primary source.
The file-backed implementation SHALL remain available at the outer layer, and
an in-memory implementation SHALL support unit tests.

#### Scenario: Resolve an in-memory include
- **WHEN** an in-memory BASIC program includes a named in-memory source
- **THEN** the included content SHALL be evaluated with the same observable
  parser behavior as a file-backed include
- **AND** no filesystem access SHALL be required

### Requirement: ROM and export assembly are observable before persistence

The workflow SHALL produce assembled ROM pages and symbol-export content in
memory before an outer adapter persists them. Failure to persist an artifact
SHALL be reported without changing the already assembled result.

#### Scenario: Build a ROM with an in-memory output adapter
- **WHEN** a compiled program is built using an in-memory output adapter
- **THEN** the resulting ROM bytes, sizes, resource map, and diagnostics SHALL
  be observable without creating an output file

### Requirement: Kernel bytes and dispatch addresses are supplied at a boundary

Code generation and ROM mapping SHALL obtain kernel bytes, module manifests, relocation records, and dispatch metadata through a read-only kernel boundary. The production boundary SHALL preserve behavior of the embedded generated kernel, while tests SHALL be able to provide deterministic images.

#### Scenario: Compile against a deterministic kernel image

- **WHEN** a test supplies a kernel image with known dispatch entries, modules, and relocation records
- **THEN** generated dispatch fixups, module placement, relocations, and mapper patches SHALL use those entries
- **AND** the test SHALL not depend on a global generated-kernel array

#### Scenario: Deterministic module placement
- **WHEN** a test supplies a fixed set of used modules and a deterministic free-space map
- **THEN** the builder SHALL produce deterministic placement, stub addresses, and overflow decisions

#### Scenario: Relocation data is read-only
- **WHEN** the builder applies relocations
- **THEN** it SHALL read relocation records through the kernel boundary
- **AND** it SHALL NOT mutate the embedded module library

### Requirement: Compilation is available as an application workflow

The application SHALL expose compilation as a workflow that returns structured
success/failure, diagnostics, and generated artifacts independently of CLI
printing and process exit handling. The CLI SHALL remain responsible for
argument parsing, adapter composition, presentation, and exit status.

#### Scenario: Retry policy is exercised without the CLI
- **WHEN** automatic ROM-mode retry is enabled and the first build exceeds the
  plain-ROM limit
- **THEN** the application workflow SHALL retry with the configured mapped ROM
  mode and return the resulting diagnostics and artifact
- **AND** the behavior SHALL be testable without standard-output capture or
  filesystem mutation
