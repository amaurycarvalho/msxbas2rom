# turbo-r-compatibility Specification

## Purpose

Defines how the compiled ROM and Z80 kernel remain correct when an MSX turbo R runs in Z80 mode or in R800 ROM mode.

## Requirements

### Requirement: Turbo mode selection is limited to supported modes

On MSX turbo R, `CMD TURBO` SHALL select only Z80 mode (0) or R800 ROM mode (1). Values other than 0 or 1 SHALL NOT enter R800 DRAM mode (2).

#### Scenario: R800 DRAM mode is not selected

- **WHEN** a program executes `CMD TURBO 2` on an MSX turbo R
- **THEN** `TURBO()` does not report mode 2
- **AND** the program's RAM and mapper state remain intact

#### Scenario: Supported modes still round-trip

- **WHEN** a program executes `CMD TURBO 1` and then `CMD TURBO 0`
- **THEN** `TURBO()` reports 1 and 0 respectively
