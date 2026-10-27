# Spec Delta

## ADDED Requirements

### Requirement: FRE requires exactly one argument

The `FRE` function SHALL require exactly one argument, matching the MSX BASIC function form. The compiler SHALL accept `FRE(<expression>)` and SHALL reject both the zero-argument and multi-argument forms with a syntax error. The argument value is evaluated for compatibility but does not change the returned free-memory value.

#### Scenario: FRE with one argument is accepted
- **WHEN** `A=FRE(0)` is compiled
- **THEN** compilation succeeds and the generated code reads the free-heap size

#### Scenario: Bare FRE is rejected
- **WHEN** `A=FRE()` is compiled
- **THEN** compilation fails with a syntax error

#### Scenario: FRE with extra arguments is rejected
- **WHEN** `A=FRE(0,1)` is compiled
- **THEN** compilation fails with a syntax error
