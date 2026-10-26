# Spec Delta

## Purpose

Provides fast, deterministic openMSX execution and RAM capture for dispatcher and
kernel module runtime checks.

## ADDED Requirements

### Requirement: Runtime checks run unthrottled without host input

The runtime-check harness SHALL disable openMSX real-time throttling when a check
does not provide a host keyboard or joystick input script.

#### Scenario: Non-interactive check runs unthrottled

- **WHEN** a runtime check runs a ROM without a host input script
- **THEN** openMSX is launched with throttling disabled
- **AND** the capture still waits for the configured emulated-time delay

#### Scenario: Input-driven check keeps real-time throttling

- **WHEN** a runtime check provides a host input script
- **THEN** openMSX keeps real-time throttling enabled so input timing is preserved

### Requirement: Terminal capture uses a pre-boot breakpoint

The runtime-check harness SHALL install a breakpoint before the ROM starts and
SHALL capture RAM at the first instruction of the program's terminal marker loop,
after the program has written its marker value.

#### Scenario: Breakpoint fires at the marker loop

- **WHEN** a check program writes its marker and reaches its terminal loop
- **THEN** RAM is captured when that loop is reached
- **AND** the emulator exits without waiting for the full fallback delay

#### Scenario: Breakpoint address comes from exported symbols

- **WHEN** a BASIC check program is compiled with symbol export
- **THEN** the terminal loop line is resolved from the exported `.noi` symbols
- **AND** the resolved address is normalized to a 16-bit CPU address before use

#### Scenario: MegaROM bank is disambiguated

- **WHEN** a MegaROM line address can be executed from more than one mapper block
- **THEN** the breakpoint fires only when the symbol's mapper block is selected
- **AND** the captured marker reflects the program's terminal state

#### Scenario: Assembler prototype breakpoint

- **WHEN** a hand-written assembler prototype is used
- **THEN** its terminal loop address is resolved from the assembler symbol output

### Requirement: Fallback capture remains available

The runtime-check harness SHALL retain the configured emulated-time capture as a
fallback when a terminal-state breakpoint is not reached.

#### Scenario: Breakpoint is missed

- **WHEN** a breakpoint is configured but the program never reaches it
- **THEN** RAM is captured after the configured emulated-time delay
- **AND** the check result is evaluated from that capture

#### Scenario: Fallback delay is preserved

- **WHEN** a runtime check defines a fallback delay
- **THEN** the harness uses that delay and does not shorten it because throttling
  is disabled

### Requirement: Existing runtime-check behavior is preserved

The harness SHALL preserve every existing runtime-check assertion, mapper mode,
machine selection, headless behavior, and pass/fail result while adding
acceleration.

#### Scenario: Existing checks still pass

- **WHEN** the emulator runtime-check suite runs after the acceleration change
- **THEN** all previously passing checks still pass with the same marker
  assertions and mapper coverage

#### Scenario: Headless execution still works

- **WHEN** no display server is available
- **THEN** the harness uses the headless SDL driver as before
- **AND** breakpoint capture produces the same RAM artifact
