## ADDED Requirements

### Requirement: ASCII16 MegaROM mapper with 16KB pages
The system SHALL support compilation of MSX-BASIC programs into ROM images targeting the ASCII16 MegaROM mapper, using 16KB pages with single-bank switching (upper bank at 0x8000-0xBFFF switched via 0x7000; lower bank at 0x4000-0x7FFF fixed to segment 0), up to 2048KB ROM size. The kernel SHALL be reused from the existing MegaROM kernel binary and patched at build time via dispatch table entries.

The ASCII16 mapper SHALL use the following memory layout:
- Page 0x4000-0x7FFF (16KB): lower bank, permanently fixed to segment 0 (kernel code). No switch writes target this page — 0x6000 is the hardware switch address but is never written.
- Page 0x8000-0xBFFF (16KB): upper bank, switched via base address 0x7000, initial segment 0.

All normal segment-switch writes SHALL target the base address 0x7000 exclusively. The lower bank SHALL remain permanently fixed — 0x6000 is never written. The OPENMSX autodetection section SHALL use two mirror writes at 0x77FF (within the 0x7000-0x77FF decoding range) to allow openMSX to distinguish ASCII16 from ASCII8 mappers. Mirror addresses (0x6001-0x67FF, 0x7001-0x77FF) SHALL NOT be used for any other purpose.

The kernel SHALL convert 8KB segment numbers emitted by the compiler to 16KB page numbers via `srl a` (logical shift right by 1) inside `MR_CHANGE_SGM`. The compiler SHALL remain unchanged, continuing to emit 8KB-pair segment numbers. The `MR_CHANGE_SGM` routine SHALL preserve the `AF` register via `push af`/`pop af` to maintain compatibility with `MR_JUMP` and other callers.

The kernel SHALL only switch the upper bank (0x8000-0xBFFF) via 0x7000 writes. The lower bank (0x4000-0x7FFF) SHALL remain fixed at segment 0, containing the kernel code.

ROM alignment SHALL pad to multiples of 8 pages (128KB), same as other MegaROM modes. Maximum ROM size SHALL be 2048KB. Resource segment mapping SHALL use the 8KB-pair convention (`resourceSegment = pages.size() * 2`), identical to other MegaROM modes.

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
