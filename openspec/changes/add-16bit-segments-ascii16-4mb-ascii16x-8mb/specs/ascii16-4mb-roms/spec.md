## ADDED Requirements

### Requirement: ASCII16 mapper supports ROMs up to 4MB with 16-bit segments

The ASCII16 mapper SHALL support ROM images up to 4MB (4096KB = 256 pages of 16KB = 512 segments of 8KB). The kernel SHALL convert 16-bit ASCII8-style segment numbers (2 per 16KB page) to 16KB page numbers via a 16-bit right shift operation. The `MR_CHANGE_SGM_16` kernel routine SHALL accept the segment number in the HL register pair and write the page number to the ASCII16 bank register at address 0x7000.

#### Scenario: Kernel converts 16-bit segment to 16KB page via SRL HL

- **WHEN** `MR_CHANGE_SGM_16` is called with HL = 0x0200 (segment 512)
- **THEN** the routine divides HL by 2 via 16-bit shift producing L = 0x00 (page 256)
- **AND** the page number is written to address 0x7000
- **AND** the 16KB page at 0x8000-0xBFFF is correctly selected

#### Scenario: Kernel handles even segment numbers correctly

- **WHEN** `MR_CHANGE_SGM_16` is called with HL = 0x0002 (segment 2)
- **THEN** the routine produces L = 0x01 (page 1) after 16-bit right shift
- **AND** writes page 1 to 0x7000

#### Scenario: ASCII16 ROM up to 4MB builds successfully

- **WHEN** a BASIC program is compiled in ASCII16 mode with resources requiring 500+ segments
- **THEN** the ROM builder does not error with a 2048K limit message
- **AND** the ROM file is produced with valid ASCII16 kernel patches

#### Scenario: Resource map segment values up to 511 are stored correctly

- **WHEN** resources are placed at segment 510 (within 4MB)
- **THEN** the resource map entry stores 0x01FE as the 2-byte segment value
- **AND** the kernel correctly reads and switches to that segment

### Requirement: ASCII16 16-bit kernel patch replaces MR_CHANGE_SGM

For ASCII16 mode, the ROM builder SHALL patch the kernel by inserting the `MR_CHANGE_SGM_16` routine and redirecting segment-switch calls to it. The original `MR_CHANGE_SGM` (8-bit) routine SHALL remain in the kernel binary for non-ASCII16 mappers. The patch SHALL be applied via dispatch-table entries using the existing `fixAscii16Mapper()` mechanism, extended for 16-bit support.

#### Scenario: ASCII16 patch applies MR_CHANGE_SGM_16 replacement

- **WHEN** a ROM is built in ASCII16 mode
- **THEN** the kernel binary is patched so that segment switches use `MR_CHANGE_SGM_16`
- **AND** the 8-bit `MR_CHANGE_SGM` routine remains unchanged for ASCII8 mode

#### Scenario: ASCII16 4MB ROM has correct padding

- **WHEN** a 4MB ASCII16 ROM is written
- **THEN** the file size is padded to a multiple of 8 pages (128KB)
- **AND** is exactly 4096KB for a full 4MB build
