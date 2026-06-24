## ADDED Requirements

### Requirement: Konami4 MegaROM without SCC mapper

The system SHALL support compilation of MSX-BASIC programs into ROM images targeting the Konami4 MegaROM mapper (Konami without SCC), using 8KB segments with 4-bank switching and the first bank locked to segment 0 on reset.

The Konami4 mapper SHALL use the following memory layout:
- Bank 1 (0x4000-0x5FFF): switch address 0x5000, fixed to segment 0 on reset
- Bank 2 (0x6000-0x7FFF): switch address 0x7000
- Bank 3 (0x8000-0x9FFF): switch address 0x8000 (base)
- Bank 4 (0xA000-0xBFFF): switch address 0xA000 (base)

The kernel segment-switch addresses for upper banks (0x8000 for Bank 3, 0xA000 for Bank 4) SHALL be patched via the same binary patch used for KonamiSCC. The patch SHALL use dispatch table entries (DISP_KONAMI_PATCH_*) to locate exact instruction addresses, covering all 14 segment-switch points across the kernel including `MR_CHANGE_SGM`, `OPENMSX_EMULATOR_AUTODETECTION`, `megarom_ascii8_bug_fix`, and `verify.slots.test.megaram`.

ROM alignment SHALL pad to multiples of 8 pages (128KB), same as other MegaROM modes.

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
