# Konami4 Mapper

## Purpose

Defines Konami4 MegaROM (without SCC) mapper support: 8KB segments with 4-bank switching.

## Requirements

### Requirement: Konami4 MegaROM without SCC mapper

The system SHALL compile MSX-BASIC programs into ROM images targeting the Konami4 MegaROM mapper (Konami without SCC), using 8KB segments with 4-bank switching and the first bank locked to segment 0 on reset. The kernel's upper-bank switch addresses SHALL be patched via the same dispatch-table binary patch used for KonamiSCC, and ROM alignment SHALL pad to multiples of 8 pages (128KB).

#### Scenario: Konami4 memory layout uses four switch addresses
- **WHEN** a Konami4 ROM is built
- **THEN** Bank 1 (0x4000-0x5FFF) SHALL use switch address 0x5000 and stay fixed to segment 0 on reset
- **AND** Bank 2 (0x6000-0x7FFF) SHALL use switch address 0x7000
- **AND** Bank 3 (0x8000-0x9FFF) SHALL use switch address 0x8000
- **AND** Bank 4 (0xA000-0xBFFF) SHALL use switch address 0xA000

#### Scenario: Compile a program in Konami4 mode
- **WHEN** a valid MSX-BASIC program is compiled with `--konami` flag
- **THEN** the output ROM file is generated with `[Konami]` suffix
- **AND** the kernel segment-switch addresses for the upper banks are patched to 0x8000 and 0xA000
- **AND** the ROM is padded to a multiple of 128KB (8 pages)

#### Scenario: Verify Konami4 ROM segment switch addresses
- **WHEN** a Konami4 ROM is built
- **THEN** the compiled kernel binary SHALL contain writes to 0x8000 (Bank 3) and 0xA000 (Bank 4) instead of 0x7000 and 0x7800
- **AND** all 14 segment-switch locations SHALL be patched via dispatch table entries

#### Scenario: Konami4 ROM size is valid
- **WHEN** a Konami4 ROM is generated
- **THEN** the ROM size SHALL be a multiple of 8KB segments
- **AND** the ROM size SHALL be between 128KB and 2MB (after MegaROM filler padding)

### Requirement: Konami boot bank mapping is unconditional

Konami4 and KonamiSCC ROMs SHALL program their boot banks regardless of the
contents of CPU address `0x8000`. The 14-byte `AB` signature guard at
`megarom_ascii8_bug_fix` SHALL NOT prevent the writes that map segments 1, 2,
and 3 before execution jumps to the user start code at `0x8010`.

#### Scenario: Konami4 ROM maps its boot banks without the AB signature

- **WHEN** a Konami4 ROM is built
- **THEN** the 14-byte guard at `megarom_ascii8_bug_fix` is NOPped
- **AND** the boot sequence still writes the `0x7000`, `0x8000`, and `0xA000`
  bank registers before jumping to `0x8010`

#### Scenario: KonamiSCC ROM maps its boot banks without the AB signature

- **WHEN** a KonamiSCC ROM is built
- **THEN** the 14-byte guard at `megarom_ascii8_bug_fix` is NOPped
- **AND** the boot sequence still writes the `0x7000`, `0x8000`, and `0xA000`
  bank registers before jumping to `0x8010`

#### Scenario: Konami ROM boots when the 0x8000 window is not segment 0

- **WHEN** a Konami4 or KonamiSCC ROM starts on an MSX turbo R machine whose
  mapper presents an empty segment at `0x8000`
- **THEN** the ROM maps the user-code segment and reaches `0x8010`
- **AND** the user program runs instead of executing empty memory

#### Scenario: Reset states that already expose the AB signature still work

- **WHEN** a Konami4 or KonamiSCC ROM starts on a machine whose mapper exposes
  segment 0 at `0x8000`
- **THEN** the unconditional bank writes select the same segments as before
- **AND** the program behavior is unchanged
