# Spec Delta

## Purpose

Defines how the compiled ROM and Z80 kernel remain correct when an MSX turbo R runs in Z80 mode or in R800 ROM mode.

## ADDED Requirements

### Requirement: Slow I/O writes are CPU-aware

Kernel routines that write devices without hardware wait states, including the PSG, SHALL ensure the device receives every register-select and data write when the CPU is an R800. The ARKOS player SHALL NOT drop PSG register writes in R800 mode.

#### Scenario: ARKOS playback writes every PSG register

- **WHEN** a program enables turbo mode and plays an AKM song on an MSX turbo R
- **THEN** the player's PSG register-select and data writes complete in the intended order
- **AND** playback continues without dropped notes caused by missing PSG writes

#### Scenario: PSG probe reports the last written register value

- **WHEN** a diagnostic writes PSG registers in a tight loop with turbo enabled and reads the values back
- **THEN** the read-back values match the values written

### Requirement: VDP block transfers preserve data in R800 mode

Kernel VDP block-transfer paths used by `CMD WRTVRAM`, partial `CMD MTF` windows, and scroll setup SHALL produce the same VRAM contents in Z80 and R800 modes.

#### Scenario: Resource copy round-trips in both CPU modes

- **WHEN** a program writes a known pattern with `CMD WRTVRAM`, reads it back with `CMD VRAMTORAM` with turbo off, and repeats with turbo on
- **THEN** both read-backs equal the original pattern

#### Scenario: Graphics and scroll remain stable in R800 mode

- **WHEN** a program draws with `PSET`, `LINE`, and `CIRCLE` and uses `SET SCROLL` with turbo off, and repeats with turbo on
- **THEN** the pixels and scroll state match the Z80-mode result

### Requirement: R800 instruction differences do not change results

Kernel code SHALL produce the same observable results on Z80 and R800, including where the R800 replaces `SLL` with `SLA` or uses its `MULUW` multiply path.

#### Scenario: Integer arithmetic matches across CPU modes

- **WHEN** the same integer multiply, divide, and modulo operations run with turbo off and turbo on
- **THEN** all results are equal

#### Scenario: Portable shift is used where the result is observable

- **WHEN** kernel code performs a left shift whose result is consumed
- **THEN** the code uses a documented shift that has the same result on Z80 and R800
