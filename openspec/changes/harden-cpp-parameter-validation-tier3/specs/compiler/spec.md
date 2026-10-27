# Spec Delta

## ADDED Requirements

### Requirement: Command parameters enforce their documented type

For command and function parameters whose contract requires a numeric value, the compiler SHALL reject string expressions with a syntax error instead of converting them implicitly through `VAL`. For parameters whose contract requires a string, numeric expressions SHALL be rejected unless the command explicitly documents a conversion. Expression operators and `LET` retain their existing conversion semantics.

#### Scenario: Numeric LOCATE rejects a string
- **WHEN** `LOCATE "1",2` is compiled
- **THEN** compilation fails with a type error

#### Scenario: Numeric LOCATE accepts a numeric expression
- **WHEN** `LOCATE 1,2` or `LOCATE A%,B%` is compiled
- **THEN** compilation succeeds

#### Scenario: String COLOR SPRITE$ rejects a number
- **WHEN** `COLOR SPRITE$(0)=1` is compiled
- **THEN** compilation fails with a type error

### Requirement: Fixed-table indices follow one policy

The compiler SHALL apply one documented policy to all fixed-table index parameters: literal values are range-checked, documented sentinels are preserved, and string or non-scalar values are rejected. This covers sprite, tile, KEY, STRIG, and resource indices. Sentinel forms such as `COLLISION(-1)` SHALL remain valid.

#### Scenario: Literal out-of-range sprite is rejected
- **WHEN** `PUT SPRITE 32,(1,1)` is compiled
- **THEN** compilation fails

#### Scenario: COLLISION sentinel remains valid
- **WHEN** `A=COLLISION(-1)` is compiled
- **THEN** compilation succeeds

#### Scenario: String index is rejected
- **WHEN** `PUT SPRITE A$,(1,1)` is compiled
- **THEN** compilation fails with a type error

### Requirement: Unsupported parameter forms are rejected

The compiler SHALL NOT silently ignore unsupported statement or function parameter forms. When a contract cannot be honored, compilation SHALL fail with a command-specific syntax error.

#### Scenario: Previously ignored extra parameter fails
- **WHEN** a statement with no supported parameters receives one
- **THEN** compilation fails with a syntax error
