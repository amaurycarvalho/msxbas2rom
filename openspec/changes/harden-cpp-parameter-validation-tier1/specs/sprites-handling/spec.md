# Spec Delta

## ADDED Requirements

### Requirement: COLOR SPRITE$ requires a string value

The compiler SHALL require the value expression of `COLOR SPRITE$(<sprite>) = <value>` to be a string. A numeric value SHALL be rejected with a syntax error instead of being emitted as a string pointer. The sprite index expression SHALL be numeric and present.

#### Scenario: String sprite color definition is accepted
- **WHEN** `COLOR SPRITE$(0)=CHR$(1)+CHR$(2)` is compiled
- **THEN** compilation succeeds

#### Scenario: Numeric sprite color definition is rejected
- **WHEN** `COLOR SPRITE$(0)=1` is compiled
- **THEN** compilation fails with a syntax error

#### Scenario: Missing sprite index is rejected
- **WHEN** `COLOR SPRITE ,1` is compiled
- **THEN** compilation fails with a syntax error

### Requirement: SET SPRITE flip and rotate directions are validated

The compiler SHALL accept literal directions `0`, `1`, and `2` for `SET SPRITE FLIP` and `SET SPRITE ROTATE`, and SHALL reject other literal directions, including signed literals such as `-1`.

#### Scenario: Valid sprite direction compiles
- **WHEN** `SET SPRITE FLIP 0,2` or `SET SPRITE ROTATE 0,1` is compiled
- **THEN** compilation succeeds

#### Scenario: Literal direction 3 is rejected
- **WHEN** `SET SPRITE FLIP 0,3` is compiled
- **THEN** compilation fails

#### Scenario: Signed negative direction is rejected
- **WHEN** `SET SPRITE ROTATE 0,-1` is compiled
- **THEN** compilation fails
