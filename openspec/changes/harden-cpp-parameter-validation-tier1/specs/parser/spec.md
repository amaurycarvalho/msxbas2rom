# Spec Delta

## ADDED Requirements

### Requirement: Argument-less statements reject trailing tokens

The parser SHALL reject extra tokens after statements whose supported form is parameterless, instead of discarding them and compiling a no-op. This applies to `BEEP`, `CLS`, `END`, and the parameterless Arkos commands `PLYMUTE`, `PLYPLAY`, and `PLYREPLAY`. A trailing comment or statement separator remains valid.

#### Scenario: BEEP with a parameter is rejected
- **WHEN** `BEEP 1` is parsed
- **THEN** parsing fails with a syntax error

#### Scenario: CLS with a parameter is rejected
- **WHEN** `CLS 1` is parsed
- **THEN** parsing fails with a syntax error

#### Scenario: Valid parameterless statements still parse
- **WHEN** `BEEP`, `CLS`, `END`, `CMD PLYMUTE`, `CMD PLYPLAY`, and `CMD PLYREPLAY` are parsed without arguments
- **THEN** parsing succeeds

#### Scenario: Trailing comment remains valid
- **WHEN** `BEEP ' comment` is parsed
- **THEN** parsing succeeds
