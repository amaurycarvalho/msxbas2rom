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
- **THEN** the workflow runs it on the configured MSX2 machine
- **AND** that machine is `Panasonic_FS-A1` by default

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

### Requirement: Regression supports per-program mapper and machine matrices

The workflow SHALL let a program declare one or more compile modes and one or
more machine classes. Each combination SHALL run as a distinct variant with its
own compiled ROM, label, and baseline path. When omitted, the compile mode SHALL
default to ASCII8 and the machine class SHALL follow the existing inference.

#### Scenario: Several compile modes for one program

- **WHEN** a program declares `compile_modes` with `konami` and `konamiscc`
- **THEN** the workflow compiles the program once per mode with the matching CLI
  flag and `[Konami]` or `[KonamiSCC]` ROM suffix
- **AND** each mode produces its own result and baseline

#### Scenario: Several machine classes for one program

- **WHEN** a program declares `machines` with `msx1`, `msx2`, `msx2plus`, and
  `turboR`
- **THEN** the workflow runs one variant per machine class
- **AND** the MSX2 variant uses the configured MSX2 machine and the MSX2+ variant
  uses the configured MSX2+ machine

#### Scenario: Matrix variants are distinguishable

- **WHEN** a program has more than one compile mode or machine class
- **THEN** the audit output and run report include the mode and machine class in
  each variant label
- **AND** baselines do not collide between variants

### Requirement: Konami MegaROM boot probes cover every machine class

The regression workflow SHALL include a machine-independent Konami boot probe
compiled in Konami4 and KonamiSCC modes and run on MSX1, MSX2, MSX2+, and
turbo R. It SHALL also include a turbo R Konami/KonamiSCC probe that switches
turbo off and on.

#### Scenario: Boot probe runs across the machine matrix

- **WHEN** `MISC/megaboot` is selected
- **THEN** it is compiled in Konami4 and KonamiSCC modes
- **AND** each mode runs on the configured MSX1, MSX2, MSX2+, and turbo R
  machines
- **AND** every variant expects `MEGAROM BOOT OK` and `SUM=5050`

#### Scenario: Turbo R Konami probe asserts its marker

- **WHEN** `MISC/turbo_konami` is selected
- **THEN** it is compiled in Konami4 and KonamiSCC modes
- **AND** each mode runs on `Panasonic_FS-A1GT`
- **AND** the captured text contains `TURBO KONAMI OK`

#### Scenario: Missing marker fails the probe

- **WHEN** a Konami probe does not reach its completion marker on any matrix
  variant
- **THEN** the regression reports the missing text expectation as a failure
