# Spec Delta

## ADDED Requirements

### Requirement: Skill supports accelerated capture

The skill SHALL accept an opt-in `MSX_DEBUG_ACCELERATE` setting that disables
openMSX real-time throttling when no host input script is configured. When an
input script is present, acceleration SHALL leave real-time throttling enabled so
input timing is preserved. An explicit `MSX_DEBUG_THROTTLE` value SHALL override
the acceleration default.

#### Scenario: Acceleration without input

- **WHEN** acceleration is enabled and no input script is configured
- **THEN** the skill launches openMSX with throttling disabled
- **AND** the capture still uses the configured emulated-time delay or breakpoint

#### Scenario: Acceleration with input

- **WHEN** acceleration is enabled and an input script is configured
- **THEN** the skill keeps real-time throttling enabled
- **AND** scripted key and joystick events are not outrun

#### Scenario: Explicit throttle wins

- **WHEN** acceleration is enabled and `MSX_DEBUG_THROTTLE` is explicitly set
- **THEN** the skill uses the explicit throttle value

### Requirement: Skill supports breakpoint conditions

The skill SHALL accept an optional condition on each breakpoint specification,
using the form `LABEL=ADDRESS@CONDITION` or `ADDRESS@CONDITION`. The condition
SHALL be evaluated before capture, and a false condition SHALL leave emulation
running. Conditions SHALL be installed before the ROM starts.

#### Scenario: Conditional breakpoint captures

- **WHEN** a breakpoint is configured with a condition that becomes true
- **THEN** the skill captures memory at that breakpoint
- **AND** the emulator exits without waiting for the fallback delay

#### Scenario: False condition is ignored

- **WHEN** a conditional breakpoint is reached while its condition is false
- **THEN** the skill does not capture
- **AND** emulation continues

#### Scenario: Breakpoint without a condition is unchanged

- **WHEN** a breakpoint is configured without a condition
- **THEN** the skill captures whenever the address is reached
- **AND** existing breakpoint behavior is preserved
