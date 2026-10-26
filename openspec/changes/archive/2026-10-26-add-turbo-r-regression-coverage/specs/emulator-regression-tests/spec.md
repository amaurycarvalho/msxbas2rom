# Spec Delta

## ADDED Requirements

### Requirement: Turbo R feature programs are assigned explicitly

Programs whose purpose is to exercise turbo R features SHALL declare a turbo R
machine requirement and SHALL run on `Panasonic_FS-A1GT`. Programs that use
turbo-capable commands only for performance SHALL remain on the MSX2+ machine
unless they declare a turbo R requirement.

#### Scenario: Turbo R feature test

- **WHEN** `MISC/test1`, `MISC/test64`, or `MISC/test65` is selected
- **THEN** the workflow runs it on `Panasonic_FS-A1GT`

#### Scenario: Performance-only turbo usage

- **WHEN** `GRAPH/tree`, `GRAPH/mexhat`, `GRAPH/test78`, `IO/test48`, or
  `MATH/cos` is selected without an explicit turbo R requirement
- **THEN** the workflow runs it on the configured MSX2+ machine

#### Scenario: Turbo R output is asserted

- **WHEN** a turbo R feature program runs on `Panasonic_FS-A1GT`
- **THEN** its configured text/RAM checks assert the turbo R-specific output
- **AND** timing-dependent values remain volatile
