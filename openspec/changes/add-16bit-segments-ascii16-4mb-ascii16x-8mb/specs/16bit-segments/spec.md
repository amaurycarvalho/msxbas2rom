## ADDED Requirements

### Requirement: Segments are treated as 16-bit values throughout the pipeline

The system SHALL treat segment numbers as 16-bit values in internal storage (C++ `int`, Z80 16-bit registers) for all MegaROM compilation and building. The compiler SHALL emit 16-bit segment values in generated code for mappers that require them, while maintaining 8-bit emission for legacy mappers (ASCII8, Konami4, KonamiSCC). The ROM builder SHALL store 16-bit segment values in the ROM header and resource map. The kernel SHALL read and write 16-bit segment values from ROM structures.

#### Scenario: Compiler uses 16-bit segments for ASCII16/ASCII16X modes

- **WHEN** the compiler writes segment bridges in ASCII16 or ASCII16X mode
- **THEN** the bridge emits a 2-byte segment value using `LD HL, <segm16>`
- **AND** the bridge occupies 9 bytes (was 8 bytes for 8-bit)

#### Scenario: Compiler uses 8-bit segments for ASCII8/Konami modes

- **WHEN** the compiler writes segment bridges in ASCII8, Konami4, or KonamiSCC mode
- **THEN** the bridge emits a 1-byte segment value using `LD A, <segm8>`
- **AND** the bridge occupies 8 bytes

#### Scenario: Cross-segment fixup writes 2-byte segment for 16-bit modes

- **WHEN** a cross-segment fixup is resolved in ASCII16 or ASCII16X mode
- **THEN** two bytes of the 16-bit segment number are written at the fixup preamble location
- **AND** the preamble contains `LD HL, <segm16>` instead of `LD A, <segm8>`

#### Scenario: Resource map stores 2-byte segment per entry

- **WHEN** a resource map is built
- **THEN** each resource entry contains a WORD (2 bytes) for the segment number
- **AND** the entry occupies 6 bytes total (WORD offset + WORD segment + WORD size)

#### Scenario: ROM header stores 2-byte resource map segment

- **WHEN** a ROM is built
- **THEN** the resource map segment at ROM offset 0x800D occupies 2 bytes (WORD)
- **AND** `BASTEXT` is shifted from 0x800E to 0x800F

#### Scenario: Kernel workarea has 2-byte RSCMAPSG

- **WHEN** the kernel initializes resource handling
- **THEN** `RSCMAPSG` (resource map segment in RAM) is a 2-byte variable
- **AND** subsequent workarea variables (`RSCMAPT1`, `PLYSGTM`, `BASMEM`) are shifted by 1 byte

#### Scenario: resource.address reads 2-byte segment from map

- **WHEN** the kernel routine `resource.address` reads a resource entry from the resource map
- **THEN** it reads 2 bytes for the segment number using 16-bit register operations
- **AND** passes the full 16-bit value to the segment switch routine

### Requirement: Helper method identifies 16-bit segment mappers

The `BuildOptions` class SHALL provide a method `uses16BitSegments()` that returns true for `ASCII16` and `ASCII16X` compile modes and false for all other modes. The compiler and builder SHALL use this method to conditionally emit 16-bit or 8-bit segment values.

#### Scenario: uses16BitSegments returns true for ASCII16

- **WHEN** `compileMode == CompileMode::ASCII16`
- **THEN** `uses16BitSegments()` returns true

#### Scenario: uses16BitSegments returns true for ASCII16X

- **WHEN** `compileMode == CompileMode::ASCII16X`
- **THEN** `uses16BitSegments()` returns true

#### Scenario: uses16BitSegments returns false for ASCII8

- **WHEN** `compileMode == CompileMode::ASCII8`
- **THEN** `uses16BitSegments()` returns false

#### Scenario: uses16BitSegments returns false for Konami modes

- **WHEN** `compileMode` is `Konami4` or `KonamiSCC`
- **THEN** `uses16BitSegments()` returns false
