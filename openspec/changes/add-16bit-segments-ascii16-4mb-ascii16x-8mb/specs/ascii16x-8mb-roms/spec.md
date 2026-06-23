## ADDED Requirements

### Requirement: ASCII16-X mapper supports ROMs up to 8MB with address-encoded bank register

The ASCII16-X mapper SHALL support ROM images up to 8MB (8192KB = 512 pages of 16KB = 1024 segments of 8KB). The kernel SHALL convert 16-bit ASCII8-style segment numbers to 16KB page numbers and write them to the ASCII16-X bank register using address encoding: data bits D0-D7 carry the low 8 bank bits, and address bits A8-A11 carry the upper 4 bank bits. The `MR_CHANGE_SGM_16X` kernel routine SHALL encode bank bits 8 and above into the write address.

#### Scenario: Kernel writes page 0-255 to base address 0x7000

- **WHEN** `MR_CHANGE_SGM_16X` is called with HL producing page number 0-255
- **THEN** the page number is written to address 0x7000 (A8-A11 = 0)
- **AND** the correct 16KB page at 0x8000-0xBFFF is selected

#### Scenario: Kernel writes page 256-511 to address 0x7100

- **WHEN** `MR_CHANGE_SGM_16X` is called with HL producing page number 256
- **THEN** the low 8 bits of the page number are written to address 0x7100 (A8 = 1)
- **AND** the combined bank number (0x100 from A8 + data) selects the correct page

#### Scenario: ASCII16-X ROM up to 8MB builds successfully

- **WHEN** a BASIC program is compiled in ASCII16X mode with resources requiring 1000+ segments
- **THEN** the ROM builder does not error with a size limit message
- **AND** the ROM file is produced with "ASCII16X" signature at offset 0x0010

#### Scenario: ASCII16-X bank register handles 9-bit bank numbers correctly

- **WHEN** the highest page (511) needs to be selected
- **THEN** the kernel writes data 0xFF to address 0x7100 (A8=1)
- **AND** the bank register receives 0x1FF (9 bits) selecting the correct page

### Requirement: ASCII16-X 16-bit kernel patch uses address encoding

For ASCII16-X mode, the ROM builder SHALL patch the kernel by inserting the `MR_CHANGE_SGM_16X` routine. This routine SHALL perform 16-bit right shift on HL to obtain the page number, then conditionally select the write address (0x7000 for pages 0-255, 0x7100 for pages 256-511, etc.) based on the upper bits of the page number.

#### Scenario: ASCII16-X patch includes address encoding logic

- **WHEN** a ROM is built in ASCII16X mode
- **THEN** the kernel binary contains the `MR_CHANGE_SGM_16X` routine with address encoding
- **AND** the routine correctly branches on the high byte of the page number

#### Scenario: ASCII16-X page 0 uses base address

- **WHEN** page 0 is selected (kernel page)
- **THEN** the write targets address 0x7000
- **AND** no address encoding is applied (upper bits = 0)
