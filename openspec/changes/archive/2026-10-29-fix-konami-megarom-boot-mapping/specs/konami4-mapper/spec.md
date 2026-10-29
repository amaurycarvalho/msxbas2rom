# Spec Delta

## ADDED Requirements

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
