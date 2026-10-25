# Kernel Size Check

## Purpose

Establishes a dual hard size ceiling for the Z80 kernel binary: `0x8000` bytes (32 KB) total for `header.bin`, with a `0x4000` byte (16 KB) sub-limit on the kernel ROM portion. The kernel occupies the second half of the binary (0x4000–0x7FFF) while the first half (0x0000–0x3FFF) holds the virtual dispatch table. A build-time verification prevents either portion from silently exceeding its budget.

## Requirements

### Requirement: Resident kernel page SHALL NOT exceed 0x4000 bytes

The build SHALL verify that the resident kernel image placed in `0x4000-0x7FFF` does not exceed `0x4000` bytes. The resident image includes boot code, the dispatcher, the central table, fixed routines, and the player/Arkos region.

#### Scenario: Resident page within limit
- **WHEN** the resident kernel image is 16384 bytes or fewer
- **THEN** no resident-size alert SHALL be emitted

#### Scenario: Resident page exceeds limit
- **WHEN** the resident kernel image exceeds 16384 bytes
- **THEN** the build SHALL emit a visible diagnostic with the actual size and excess
- **AND** the build SHALL identify the modules that must move to overflow

### Requirement: Module placement budgets SHALL be validated

The build SHALL verify that every relocatable module fits its assigned kernel hole, user-page slot, or MegaROM segment. A module that does not fit SHALL cause a build failure with the module name, required size, and available size.

#### Scenario: Module fits its assigned region
- **WHEN** a used module's size is less than or equal to the assigned region
- **THEN** placement SHALL succeed without a size diagnostic

#### Scenario: Module exceeds its region
- **WHEN** a used module's size exceeds the assigned kernel hole, user-page slot, or segment
- **THEN** the build SHALL fail
- **AND** the diagnostic SHALL name the module, its size, and the available space

### Requirement: Plain ROM total code budget SHALL be validated

For Plain ROM, the build SHALL verify that start code, user code, and overflow modules fit within `0x8000-0xBFFF`. Failure SHALL report the combined usage and suggest MegaROM.

#### Scenario: Plain ROM fits
- **WHEN** start code, user code, and overflow modules total 16384 bytes or fewer
- **THEN** the Plain ROM build SHALL succeed

#### Scenario: Plain ROM exceeds the code page
- **WHEN** start code, user code, and overflow modules exceed 16384 bytes
- **THEN** the build SHALL fail
- **AND** the diagnostic SHALL list the code size, overflow group sizes, and a MegaROM suggestion

### Requirement: MegaROM segment budget SHALL be validated

For MegaROM, the build SHALL verify that overflow modules fit in the allocated 8 KB/16 KB segments and that the final ROM remains within the mapper size limit.

#### Scenario: Overflow segments fit
- **WHEN** every overflow module fits in its allocated segment and the ROM is within the mapper limit
- **THEN** the MegaROM build SHALL succeed

#### Scenario: Overflow exceeds the mapper limit
- **WHEN** overflow allocation would exceed the mapper size limit
- **THEN** the build SHALL fail
- **AND** the diagnostic SHALL identify the overflowing module group and the mapper limit
