## ADDED Requirements

### Requirement: ASCII16 MegaROM mapper with 16KB pages
The system SHALL support compilation of MSX-BASIC programs into ROM images targeting the ASCII16 MegaROM mapper, using 16KB pages with 2-bank switching (0x6000 for 0x4000-0x7FFF, 0x7000 for 0x8000-0xBFFF), up to 2048KB ROM size. The kernel SHALL be reused from the existing MegaROM kernel binary and patched at build time via dispatch table entries to adapt the segment-switching model from 8KB pairs to single 16KB pages.

The ASCII16 mapper SHALL use the following memory layout:
- Page 0x4000-0x7FFF (16KB): base switch address 0x6000, initial segment 0, kernel fixed (never switched)
- Page 0x8000-0xBFFF (16KB): base switch address 0x7000, initial segment 0

All segment-switch writes SHALL target the base addresses (0x6000, 0x7000) exclusively. Mirror addresses (0x6001-0x67FF, 0x7001-0x77FF) SHALL NOT be used.

The kernel SHALL only switch the upper bank (0x8000-0xBFFF) via 0x7000 writes. The lower bank (0x4000-0x7FFF) SHALL remain fixed at segment 0, containing the kernel code.

ROM alignment SHALL pad to multiples of 8 pages (128KB), same as other MegaROM modes. Maximum ROM size SHALL be 2048KB.

#### Scenario: Compile a program in ASCII16 mode
- **WHEN** a valid MSX-BASIC program is compiled with `--ascii16` flag
- **THEN** the output ROM file is generated with `[ASCII16]` suffix
- **AND** the kernel's `MR_CHANGE_SGM` routine writes only to 0x7000 (not 0x7800)
- **AND** the `inc a`, `ld (0x7800),a`, and `dec a` in MR_CHANGE_SGM are replaced with NOPs
- **AND** the ROM is padded to a multiple of 128KB (8 pages)

#### Scenario: Verify ASCII16 ROM kernel patching
- **WHEN** an ASCII16 ROM is built
- **THEN** the compiled kernel binary SHALL contain NOPs (0x00) in place of the `inc a`, `ld (0x7800),a`, and `dec a` sequence in MR_CHANGE_SGM
- **AND** all ASCII16 patch locations SHALL be applied via dispatch table entries (DISP_ASCII16_PATCH_*)
- **AND** writes to 0x7000 SHALL remain present and unmodified
- **AND** all segment-switch port writes SHALL target the base address 0x7000, never mirror addresses (0x7001-0x77FF)

#### Scenario: ASCII16 ROM size is valid
- **WHEN** an ASCII16 ROM is generated
- **THEN** the ROM size SHALL be a multiple of 16KB pages
- **AND** the ROM size SHALL be between 64KB and 2048KB

#### Scenario: ASCII16 segment numbers are 1:1 with pages
- **WHEN** a ROM is built in ASCII16 mode
- **THEN** the resource map segment SHALL equal the number of ROM pages (pages.size())
- **AND** compiled code segments SHALL be numbered 1, 2, 3, ... (not 2, 4, 6, ...)
- **AND** segment 0 SHALL correspond to the kernel page

#### Scenario: ASCII8 bugfix is patched for ASCII16
- **WHEN** an ASCII16 ROM is built
- **THEN** the boot bugfix `ld (0x6800),a` SHALL be patched to `ld (0x7000),a`
- **AND** the `inc a` and `ld (0x7800),a` following the valid 0x7000 write SHALL be NOPped
- **AND** after initialization, the 0x7000 register SHALL be set to segment 1
