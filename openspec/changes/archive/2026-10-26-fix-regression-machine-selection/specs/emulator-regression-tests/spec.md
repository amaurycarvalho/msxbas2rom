# Spec Delta

## MODIFIED Requirements

### Requirement: Machine model is selected per program

The workflow SHALL select the least-capable configured machine that satisfies
each program's machine class. The class SHALL be inferred from screen modes and
turbo-capable commands, or declared explicitly. The workflow SHALL support MSX1,
MSX2, MSX2+, and MSXturboR classes.

#### Scenario: MSX1 program uses the default machine

- **WHEN** a program uses only MSX1-compatible screen modes and no turbo-capable
  command
- **THEN** the workflow runs it on the configured MSX1 machine

#### Scenario: MSX2 program uses an MSX2 machine

- **WHEN** a program uses an MSX2-only screen mode
- **THEN** the workflow runs it on the configured MSX2+ machine
- **AND** that machine is `Panasonic_FS-A1WX` by default

#### Scenario: MSX2 or MSX2+ program uses the MSX2+ machine

- **WHEN** a program uses `SCREEN 4` through `SCREEN 12`, with or without a space
  after `SCREEN`
- **THEN** the workflow runs it on the configured MSX2+ machine
- **AND** that machine is `Panasonic_FS-A1WX` by default

#### Scenario: Turbo-capable command without turbo R requirement uses MSX2+

- **WHEN** a program uses `CMD TURBO`, `CALL TURBO`, or `TURBO()` and does not
  declare a turbo R requirement
- **THEN** the workflow runs it on the configured MSX2+ machine

#### Scenario: Turbo R program uses the turbo R machine

- **WHEN** a program declares a turbo R requirement
- **THEN** the workflow runs it on the configured turbo R machine
- **AND** that machine is `Panasonic_FS-A1GT` by default

#### Scenario: Explicit machine override wins

- **WHEN** a program declares an explicit machine
- **THEN** the workflow uses that machine regardless of inferred class

#### Scenario: V9958 graphical mode is decoded

- **WHEN** a captured VDP register pair is `R0=0x06`, `R1=0x60`
- **THEN** the workflow reports screen mode 5, not an MSX1 mode

## ADDED Requirements

### Requirement: Regression exposes and verifies the selected machine

The workflow SHALL expose each program's selected machine and machine class in
the audit/list output and the run report, and SHALL reject a baseline generated
for a different machine.

#### Scenario: Audit lists the selected machine

- **WHEN** the audit/list command runs for a program
- **THEN** it reports the program, its machine class, and the selected machine

#### Scenario: Baseline machine mismatch is rejected

- **WHEN** a baseline records a different machine than the selected machine
- **THEN** the program fails before memory comparison
- **AND** the failure asks the operator to regenerate the baseline
