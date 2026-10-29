# Spec Delta

## ADDED Requirements

### Requirement: Konami mapper patch makes boot mapping unconditional

`fixKonamiMapper()` SHALL NOP the 14-byte `AB` signature guard at
`megarom_ascii8_bug_fix` for both Konami4 and KonamiSCC builds, so the boot bank
mapping always runs. The patch SHALL use a mapper-neutral dispatch alias named
`DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` that resolves to the shared guard address.
ASCII8 and ASCII16 builds SHALL keep the guard unchanged.

#### Scenario: Konami4 build NOPs the shared AB guard

- **WHEN** a compilation result is built in Konami4 mode
- **THEN** the 14 bytes starting at `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` are
  `0x00`
- **AND** the existing Konami4 segment-switch patches remain applied

#### Scenario: KonamiSCC build NOPs the shared AB guard

- **WHEN** a compilation result is built in KonamiSCC mode
- **THEN** the 14 bytes starting at `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` are
  `0x00`
- **AND** the existing KonamiSCC segment-switch patches remain applied

#### Scenario: ASCII8 build keeps the AB guard

- **WHEN** a compilation result is built in ASCII8 mode
- **THEN** the 14-byte `AB` guard remains present in the kernel page

#### Scenario: ASCII16 build keeps the AB guard

- **WHEN** a compilation result is built in ASCII16 mode
- **THEN** the 14-byte `AB` guard remains present in the kernel page

#### Scenario: Invalid patch offset fails the build

- **WHEN** the guard dispatch address resolves outside the kernel page
- **THEN** the build fails with a Konami ROM format adjustment error
- **AND** no partial ROM is reported as valid
