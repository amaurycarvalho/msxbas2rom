# Spec Delta

## ADDED Requirements

### Requirement: PUT SPRITE preserves OR and collision-ignore color bits

On MSX2 and later, the `PUT SPRITE` color parameter SHALL preserve bit 6 (OR)
and bit 5 (collision ignore) when writing the sprite color table. Bit 7 SHALL be
ignored. MSX1 SHALL remain limited to the 4-bit sprite color.

#### Scenario: OR and collision-ignore bits are written

- **WHEN** `PUT SPRITE 1, (103, 100), 12 + 96, 0` executes on SCREEN 4
- **THEN** all 16 color-table bytes for sprite 1 contain `0x6C`
- **AND** sprite 0 keeps its plain color bytes

#### Scenario: Plain color remains unchanged

- **WHEN** `PUT SPRITE 0, (100, 100), 1, 0` executes on SCREEN 4
- **THEN** all 16 color-table bytes for sprite 0 contain `0x01`

#### Scenario: MSX1 ignores unsupported bits

- **WHEN** `PUT SPRITE` receives a color above 15 on an MSX1 screen mode
- **THEN** only the low four color bits are applied
- **AND** no OR or collision-ignore behavior is introduced
