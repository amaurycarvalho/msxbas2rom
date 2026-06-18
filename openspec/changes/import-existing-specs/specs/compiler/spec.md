## ADDED Requirements

### Requirement: Convert parsed BASIC actions into Z80 machine code
The system SHALL convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably.

#### Scenario: Compile valid parser output
- **WHEN** semantic analysis is executed on a parser result with valid tags and actions
- **THEN** compilation succeeds
- **AND** code size is greater than zero
- **AND** RAM usage metrics are available

#### Scenario: Fail on duplicated line symbol mapping
- **WHEN** parser tags map to an already declared line symbol
- **THEN** compilation fails
- **AND** error message indicates line number already declared

#### Scenario: Register DATA and IDATA resources
- **WHEN** parser output has `has_data` and `has_idata` enabled and semantic analysis is executed
- **THEN** the compiler adds data and idata resources to the resource manager
