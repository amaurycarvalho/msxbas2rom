# Spec Delta

## ADDED Requirements

### Requirement: Regression applies machine-specific boot delays

The workflow SHALL apply a configured minimum boot delay per machine class
before a fallback capture. The effective fallback SHALL be the greater of the
program's configured fallback and the selected machine's boot delay. MSX2,
MSX2+, and turbo R SHALL default to at least 15 emulated seconds.

#### Scenario: MSX2+ fallback respects the boot delay

- **WHEN** an MSX2+ program has a configured fallback below the machine boot
  delay
- **THEN** the workflow waits until the machine boot delay before capturing
- **AND** the capture artifact records the effective time

#### Scenario: Program-specific fallback longer than the boot delay wins

- **WHEN** a program configures a fallback longer than the machine boot delay
- **THEN** the workflow uses the program's longer fallback

#### Scenario: Machine boot delay can be overridden

- **WHEN** the manifest configures a machine boot delay
- **THEN** the workflow uses that value for the selected machine class
