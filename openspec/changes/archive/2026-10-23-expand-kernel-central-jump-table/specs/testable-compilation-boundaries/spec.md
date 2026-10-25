# Testable Compilation Boundaries

## MODIFIED Requirements

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
