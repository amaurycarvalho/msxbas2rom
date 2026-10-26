# Spec Delta

## ADDED Requirements

### Requirement: USR2-backed functions use kernel dispatch routing

The compiler SHALL emit calls to USR2-backed functions through the kernel
dispatch mechanism, passing the function selector in `HL`. It SHALL NOT emit a
direct absolute call to a resolved `USR2` address or rely on a skipped
instruction to supply the selector.

#### Scenario: TURBO() returns on MSX2+

- **WHEN** `TURBO()` is compiled and executed on `Panasonic_FS-A1WX`
- **THEN** it returns a numeric turbo-state value
- **AND** execution continues to the next statement

#### Scenario: TURBO() returns on turbo R

- **WHEN** `TURBO()` is compiled and executed on `Panasonic_FS-A1GT`
- **THEN** it returns a numeric CPU-mode value
- **AND** execution continues to the next statement

#### Scenario: VDP() and MAKER() remain callable

- **WHEN** `VDP()` or `MAKER()` is compiled and executed on a supported machine
- **THEN** it returns a numeric value
- **AND** does not hang or corrupt the kernel bank mapping
