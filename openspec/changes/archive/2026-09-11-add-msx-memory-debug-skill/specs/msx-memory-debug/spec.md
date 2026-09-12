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

### Requirement: Debug programs SHALL freeze observable state before capture

Because the capture point is a fixed amount of emulated time, a program that is still
running at capture time produces a nondeterministic snapshot. A debug build SHALL write
the values under inspection to fixed RAM addresses and then stop making progress (for
example, an infinite loop) so the captured RAM and screenshot reflect a known state.

#### Scenario: Fixed-address probe values are readable in the dump

- **WHEN** a debug program pokes an observed value to a fixed RAM address and then loops forever
- **THEN** the RAM dump contains that value at artifact offset `address - 0xC000`

#### Scenario: Program does not drift past the capture point

- **WHEN** the program reaches its final state before the configured emulated delay
- **THEN** repeated runs show that final state in the RAM dump and screenshot

### Requirement: Debug builds SHALL be compiled with debugger symbols

The debug workflow SHALL compile the BASIC program with symbol export
(`msxbas2rom -s --noi`) so BASIC variable names map to RAM addresses in a `.noi` file.
Kernel work-area symbols (`SPRTBL`, `HITBOX_TABLE`, `SPRSIZ`, etc.) SHALL be read from
`src/infrastructure/kernel/asm/src/header.symbols.asm`.

#### Scenario: BASIC variable address is known

- **WHEN** the `.noi` file is inspected
- **THEN** each BASIC variable (for example `VAR_X%`) has a RAM address that can be located in the dump

#### Scenario: Kernel work-area address is known

- **WHEN** kernel behavior is under debug
- **THEN** the relevant kernel symbol address from `header.symbols.asm` is used to inspect its bytes in the dump

### Requirement: CPU-level debugging SHALL use openMSX breakpoints

When the bulk RAM/screenshot capture is insufficient, the workflow SHALL allow a custom
openMSX Tcl script that sets breakpoints with `debug set_bp`, reads CPU registers with
`debug read_block "CPU regs" 0 26` and memory with `debug read_block "Main RAM" <addr> <len>`,
and resumes execution with `debug cont`. Breakpoint callbacks SHALL write results to a file
because Tcl `puts` output is not reliably visible on the launcher's stdout. The "CPU regs"
26-byte layout SHALL be treated as `F, A, B, C, D, E, H, L, F', A', B', C', D', E', H', L',
IXh, IXl, IYh, IYl, SPh, SPl, PCh, PCl, I, R`.

#### Scenario: Breakpoint captures register and memory state

- **WHEN** a breakpoint fires at a routine of interest
- **THEN** the callback records the CPU registers and the relevant RAM bytes to a file

#### Scenario: Breakpoint on a success path isolates the interesting case

- **WHEN** a routine is called many times during a run
- **THEN** the breakpoint is placed on the success/terminal path so only the relevant occurrence is recorded

#### Scenario: CPU is resumed after the callback

- **WHEN** a breakpoint callback has finished recording
- **THEN** it calls `debug cont` so the emulator continues instead of remaining paused

### Requirement: Kernel behavior changes SHALL rebuild the embedded kernel header

The kernel is embedded in the compiler as a generated C++ header. After editing
`src/infrastructure/kernel/asm/src/header/*.asm`, the workflow SHALL run
`make -C src/infrastructure/kernel/asm` (pasmo + `xxd`, regenerating `header.h`) and then
`make release` before recompiling the ROM under test. Referencing absolute RAM addresses
inside kernel code for debug stores can make pasmo emit a much larger `header.bin`; existing
kernel work-area symbols SHOULD be preferred.

#### Scenario: Kernel edit is reflected in the ROM

- **WHEN** a kernel source file is changed and only `make release` is run
- **THEN** the change is missing from the ROM until the kernel header is regenerated

#### Scenario: Debug stores do not inflate the kernel image

- **WHEN** temporary debug instrumentation is added to kernel code
- **THEN** it does not cause `header.bin` to grow beyond its expected size

### Requirement: MSX2 BASIC commands SHALL be debugged on an MSX2 machine

MSXBAS2ROM BASIC commands that target MSX2 hardware (for example `SCREEN 7` and MSX2-only
graphics or VDP features) SHALL be debugged on the Panasonic FS-A1WX machine
(`MSX_DEBUG_MACHINE=Panasonic_FS-A1WX`) rather than the default Sharp HB-8000 1.2, which is
an MSX1 machine. MSX1-oriented commands SHALL keep the Sharp HB-8000 default.

#### Scenario: MSX2 command uses FS-A1WX

- **WHEN** an MSX2-specific BASIC command is debugged
- **THEN** the skill is run with `MSX_DEBUG_MACHINE=Panasonic_FS-A1WX`

#### Scenario: MSX1 command uses the default Hotbit

- **WHEN** an MSX1 BASIC command is debugged
- **THEN** the default Sharp HB-8000 1.2 machine is used
