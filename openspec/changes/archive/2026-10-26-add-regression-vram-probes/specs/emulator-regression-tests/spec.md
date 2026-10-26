# Spec Delta

## ADDED Requirements

### Requirement: VRAM checks support explicit byte probes

The workflow SHALL support configured VRAM byte probes that compare exact bytes
at absolute VRAM offsets. Probes SHALL be evaluated even for programs marked
volatile. A probe SHALL provide an `address` and either a `value` or `bytes`.

#### Scenario: Matching VRAM probe passes

- **WHEN** a program configures a VRAM probe at `0x1C10` expecting `6C` bytes
- **AND** the captured VRAM contains `6C` at that offset
- **THEN** the probe passes and the program remains eligible to pass

#### Scenario: Mismatching VRAM probe fails with offset and bytes

- **WHEN** a program configures a VRAM probe at `0x1C10` expecting `6C`
- **AND** the captured VRAM contains `0C` at that offset
- **THEN** the probe fails
- **AND** the report names the VRAM offset and the expected and actual bytes

#### Scenario: Volatile program still evaluates VRAM probes

- **WHEN** a program is marked volatile and has configured VRAM probes
- **THEN** the stable RAM/VRAM snapshot checks are skipped
- **AND** every configured VRAM probe is still evaluated

#### Scenario: Probe outside captured VRAM is rejected

- **WHEN** a program configures a VRAM probe whose range exceeds the captured
  VRAM size
- **THEN** the runner stops with a configuration error before validating the
  program
