# Spec Delta

## ADDED Requirements

### Requirement: Sprite pattern and color buffers must be declared arrays

`SET SPRITE PATTERN`, `GET SPRITE PATTERN`, `SET SPRITE COLOR`, and `GET SPRITE COLOR` SHALL accept their buffer parameter only when it refers to an array declared with `DIM`. A literal or scalar variable SHALL be rejected at compile time.

#### Scenario: Declared pattern array is accepted
- **WHEN** `DIM PB%(3,3)` and `SET SPRITE PATTERN 0, PB%` are compiled
- **THEN** compilation succeeds

#### Scenario: Scalar pattern buffer is rejected
- **WHEN** `A=1` and `SET SPRITE PATTERN 0, A` are compiled
- **THEN** compilation fails with a sprite-buffer error

#### Scenario: Literal color buffer is rejected
- **WHEN** `GET SPRITE COLOR 0, 1` is compiled
- **THEN** compilation fails with a sprite-buffer error

### Requirement: Sprite index literals use the 32-sprite range

For `PUT SPRITE`, `COLOR SPRITE`, `SET SPRITE PATTERN/COLOR/FLIP/ROTATE`, and `SET SPRITE HITBOX`, a literal sprite index SHALL be in `0..31`. `COLLISION` is exempt because it documents `-1` as the "any sprite" sentinel.

#### Scenario: Sprite index 31 is accepted
- **WHEN** `PUT SPRITE 31,(1,1)` is compiled
- **THEN** compilation succeeds

#### Scenario: Sprite index 32 is rejected
- **WHEN** `PUT SPRITE 32,(1,1)` or `SET SPRITE FLIP 32,0` is compiled
- **THEN** compilation fails with a sprite-index error

#### Scenario: COLLISION negative sentinel remains valid
- **WHEN** `A=COLLISION(-1)` is compiled
- **THEN** compilation succeeds
