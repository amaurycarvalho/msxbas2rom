# ASCII16 Mapper

## Purpose

Defines ASCII16 MegaROM mapper support: 16KB pages with single-bank switching, up to 2048KB ROM size.

## Requirements

### Requirement: ASCII16 MegaROM mapper with 16KB pages

The system SHALL support compilation of MSX-BASIC programs into ROM images targeting the ASCII16 MegaROM mapper, using 16KB pages with single-bank switching, up to 2048KB ROM size. The kernel SHALL be reused from the existing MegaROM kernel binary and patched at build time via dispatch table entries. Resource segment mapping SHALL use the 8KB-pair convention (`resourceSegment = pages.size() * 2`).

#### Scenario: Lower bank remains fixed at segment 0
- **WHEN** an ASCII16 ROM executes
- **THEN** the lower bank at 0x4000-0x7FFF SHALL remain fixed to segment 0 (kernel code)
- **AND** no segment-switch write SHALL target 0x6000

#### Scenario: 8KB segment numbers are converted to 16KB pages
- **WHEN** the compiler emits an 8KB-pair segment number
- **THEN** `MR_CHANGE_SGM` SHALL convert it to a 16KB page number via `srl a` before writing 0x7000
- **AND** the compiler SHALL remain unchanged, continuing to emit 8KB-pair segment numbers
- **AND** `MR_CHANGE_SGM` SHALL preserve `AF` via `push af`/`pop af` for `MR_JUMP` and other callers

#### Scenario: Normal switch writes use the base address exclusively
- **WHEN** an ASCII16 ROM performs a normal segment switch
- **THEN** the write SHALL target the base address 0x7000 exclusively
- **AND** mirror addresses SHALL NOT be used except for the OPENMSX autodetection writes at 0x77FF

#### Scenario: Compile a program in ASCII16 mode
- **WHEN** a valid MSX-BASIC program is compiled with `--ascii16` flag
- **THEN** the output ROM file is generated with `[ASCII16]` suffix
- **AND** the kernel's `MR_CHANGE_SGM` routine uses `push af; srl a; ld (0x7000),a; pop af; ret` instead of the original dual-write sequence
- **AND** the ROM is padded to a multiple of 128KB (8 pages)

#### Scenario: Verify ASCII16 ROM kernel patching
- **WHEN** an ASCII16 ROM is built
- **THEN** `MR_CHANGE_SGM` SHALL contain the byte sequence `F5 CB 3F 32 00 70 F1 C9` (push af / srl a / ld (0x7000),a / pop af / ret)
- **AND** the first 3 OPENMSX autodetection writes SHALL target 0x7000 (base address)
- **AND** the 4th and 5th OPENMSX autodetection writes SHALL target 0x77FF (mirror for mapper identification)
- **AND** all normal segment-switch writes SHALL target the base address 0x7000, never mirror addresses
- **AND** all ASCII16 patch locations SHALL be applied via dispatch table entries

#### Scenario: ASCII16 ROM size is valid
- **WHEN** an ASCII16 ROM is generated
- **THEN** the ROM size SHALL be a multiple of 16KB pages
- **AND** the ROM size SHALL be between 128KB and 2048KB (after MegaROM filler padding)

#### Scenario: ASCII8 bugfix is patched for ASCII16
- **WHEN** an ASCII16 ROM is built
- **THEN** the boot bugfix `ld (0x6800),a` SHALL be patched to `ld (0x7000),a`
- **AND** the `inc a` and `ld (0x7800),a` following the first write SHALL be NOPped
- **AND** the redundant second `ld (0x7000),a` SHALL be NOPped
- **AND** after initialization, the 0x7000 register SHALL be set to segment 1 via a single write
