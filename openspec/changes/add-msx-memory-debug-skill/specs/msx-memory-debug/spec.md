## Purpose

Provides coding agents a non-interactive, deterministic way to inspect the runtime
RAM contents and screen of an MSX ROM using openMSX, without requiring a manual
debugger session.

## ADDED Requirements

### Requirement: Skill runs a ROM in openMSX without interaction

The skill SHALL launch a valid ROM file in openMSX with the ROM inserted as a
cartridge and SHALL run the emulator without requiring user interaction. If the ROM
path does not exist, the skill SHALL fail with a clear error and SHALL NOT launch
openMSX.

#### Scenario: Valid ROM is launched

- **WHEN** the skill is invoked with the path to an existing ROM file
- **THEN** openMSX starts with that ROM inserted as a cartridge

#### Scenario: Missing ROM is rejected

- **WHEN** the skill is invoked with a path that does not exist
- **THEN** the skill reports an error and does not start openMSX

### Requirement: Skill captures CPU-visible RAM C000-FFFF

The skill SHALL capture the CPU-visible main RAM range `0xC000..0xFFFF` (16384
bytes) at the capture point and write it as a raw binary file. The captured block
SHALL be exactly 16384 bytes, where byte `0` corresponds to CPU address `0xC000`.
The capture SHALL be performed as a single bulk debugger read rather than repeated
single-byte reads.

#### Scenario: RAM dump has the expected size

- **WHEN** the capture completes
- **THEN** the RAM artifact exists beside the ROM and is exactly 16384 bytes

#### Scenario: RAM contents map to CPU addresses

- **WHEN** the RAM artifact is read
- **THEN** byte offset `0x0000` is the value at CPU address `0xC000` and offset `0x3FFF` is the value at `0xFFFF`

### Requirement: Skill captures a screenshot at the capture point

The skill SHALL capture a PNG screenshot of the emulated MSX screen at the same
point in emulated time as the RAM dump. The screenshot mode SHALL be selectable
between the scaled screen and the raw unscaled screen.

#### Scenario: Screenshot is written

- **WHEN** the capture completes
- **THEN** a PNG screenshot artifact exists beside the ROM

#### Scenario: Raw screenshot mode is honored

- **WHEN** the screenshot mode is configured as raw
- **THEN** the screenshot is captured at the raw MSX screen resolution

### Requirement: Capture point is configurable in emulated time

The skill SHALL delay capture by a configurable number of emulated MSX seconds
before reading memory and taking the screenshot. The delay SHALL be based on
emulated time rather than host wall-clock time.

#### Scenario: Default delay is applied

- **WHEN** no delay is configured
- **THEN** the capture occurs after the default emulated-time delay

#### Scenario: Custom delay is applied

- **WHEN** a non-negative delay is configured
- **THEN** the capture occurs after that many emulated seconds

#### Scenario: Invalid delay is rejected

- **WHEN** the configured delay is not a non-negative number
- **THEN** the skill reports an error and does not start openMSX

### Requirement: Artifacts are named deterministically beside the ROM

The skill SHALL write artifacts in the same directory as the ROM, using names
derived from the ROM base name, so repeated runs produce the same paths and can be
inspected deterministically by an agent.

#### Scenario: Artifact names derive from the ROM

- **WHEN** the skill runs on a ROM named `program.rom`
- **THEN** the RAM dump is written as `program.ram-c000-ffff.bin` and the screenshot as `program.screen.png`

### Requirement: openMSX exits automatically after capture

After both artifacts are written, the skill SHALL terminate openMSX so the calling
agent regains control. The skill SHALL return the openMSX exit status.

#### Scenario: Emulator exits after writing artifacts

- **WHEN** the capture completes successfully
- **THEN** openMSX exits and the skill returns control to the caller

### Requirement: Skill isolates openMSX from the user's settings

The skill SHALL NOT load the user's interactive `settings.xml`. openMSX aborts at
startup when that file contains a key binding the installed version does not
recognise (for example an unknown `PrintScreen` keycode), which would prevent the
capture from running. The skill SHALL instead generate a minimal settings file,
pass it to openMSX, and remove it after the run.

#### Scenario: Host settings cannot crash the capture

- **WHEN** the user's `settings.xml` contains an invalid key binding
- **THEN** the skill still launches openMSX and produces the artifacts

#### Scenario: Temporary settings file is removed

- **WHEN** the skill finishes
- **THEN** the generated settings file no longer exists beside the ROM

#### Scenario: Custom settings file can be supplied

- **WHEN** a settings file path is provided through configuration
- **THEN** the skill loads that file instead of generating one

### Requirement: Skill runs the Sharp HB-8000 BIOS by default

The skill SHALL select the Sharp HB-8000 (Hotbit) machine explicitly so the
emulator boots the Sharp BIOS instead of falling back to C-BIOS. The machine SHALL
be overridable through configuration.

#### Scenario: Default machine is Sharp HB-8000

- **WHEN** the skill runs without a machine override
- **THEN** openMSX boots the Sharp HB-8000 machine and not C-BIOS

#### Scenario: Machine can be overridden

- **WHEN** a machine name is provided through configuration
- **THEN** openMSX boots the specified machine instead of the default
