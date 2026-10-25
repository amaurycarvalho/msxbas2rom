# Spec Delta

## ADDED Requirements

### Requirement: Skill captures VDP VRAM

The skill SHALL capture VDP VRAM content alongside the existing RAM and screenshot
artifacts, and SHALL write it as a raw binary artifact named deterministically
beside the ROM.

#### Scenario: VRAM artifact is written

- **WHEN** the skill completes a capture
- **THEN** a raw VRAM artifact exists beside the ROM
- **AND** its size matches the emulated VDP VRAM size

#### Scenario: VRAM range is configurable

- **WHEN** a VRAM start address and length are configured
- **THEN** the artifact contains exactly that range

### Requirement: Skill sets breakpoints before ROM execution

The skill SHALL accept breakpoint specifications and install them before the ROM
starts executing, so short-lived programs are captured at their terminal state.

#### Scenario: Breakpoint fires at program end

- **WHEN** a breakpoint is configured at a program-end address
- **THEN** the skill captures memory at that breakpoint
- **AND** does not wait for the full emulated-time delay

#### Scenario: Breakpoint never fires

- **WHEN** a configured breakpoint is not reached
- **THEN** the skill falls back to the configured emulated-time capture

### Requirement: Skill injects scripted input

The skill SHALL accept a timed input script and SHALL inject keyboard and joystick
input during emulation so interactive programs can advance deterministically.

#### Scenario: Keyboard input is injected

- **WHEN** the input script contains keyboard events
- **THEN** the skill applies each key at its configured emulated time

#### Scenario: Joystick input is injected

- **WHEN** the input script contains joystick events
- **THEN** the skill applies the configured direction and button state

### Requirement: Skill supports fallback capture

The skill SHALL support a maximum emulated-time capture that runs when no
breakpoint fires, preserving the existing fixed-delay behavior as the fallback.

#### Scenario: Fallback capture completes

- **WHEN** no breakpoint is reached and the fallback time elapses
- **THEN** the skill captures RAM, VRAM, and screenshot artifacts

### Requirement: Existing capture behavior remains compatible

The skill SHALL preserve its existing RAM and screenshot artifact behavior when the
new VRAM, breakpoint, and input options are not configured.

#### Scenario: Legacy invocation is unchanged

- **WHEN** the skill is invoked with only a ROM path
- **THEN** it produces the same RAM and screenshot artifacts as before
- **AND** the default delay and machine behavior are unchanged
