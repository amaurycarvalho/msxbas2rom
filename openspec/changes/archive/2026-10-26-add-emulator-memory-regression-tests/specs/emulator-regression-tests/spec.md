# Spec Delta

## Purpose

Provides a local, scenario-based regression workflow that compiles MSX BASIC
programs, runs them in openMSX, and validates runtime behavior through RAM and
VRAM memory checks instead of screenshot comparison.

## ADDED Requirements

### Requirement: Regression entry point is independent from integration compilation

The integration test Makefile SHALL expose a regression entry point that runs
selected programs in openMSX and SHALL NOT be invoked by the existing integration
compile target.

#### Scenario: Regression runs alone

- **WHEN** the regression entry point is invoked
- **THEN** only the selected regression programs are compiled and executed
- **AND** the existing four-mapper integration compile matrix is not executed

#### Scenario: Existing integration target is unchanged

- **WHEN** the existing integration target is invoked
- **THEN** it compiles programs as before
- **AND** it does not start openMSX or run regression validation

### Requirement: Programs are organized into selectable scenarios

The workflow SHALL use the existing feature folders as regression scenarios and
SHALL allow execution of all scenarios, one scenario, or one individual program.

#### Scenario: Run all scenarios

- **WHEN** the regression target is invoked without a selector
- **THEN** every discovered scenario and program is executed

#### Scenario: Run one scenario

- **WHEN** the regression target is invoked with a scenario selector
- **THEN** only programs in that feature folder are executed

#### Scenario: Run one program

- **WHEN** the regression target is invoked with a program selector
- **THEN** only that program is compiled and executed

### Requirement: Regression compilation uses ASCII8 and isolated output

The workflow SHALL compile selected programs in ASCII8 mode by default, export
`.noi` debug symbols, and write ROMs and symbols to an isolated output location.

#### Scenario: Default regression compilation

- **WHEN** a program is selected for regression
- **THEN** it is compiled in ASCII8 mode with NoICE symbols
- **AND** unrelated mapper variants are not compiled

#### Scenario: Output isolation

- **WHEN** regression compilation runs
- **THEN** generated ROM and symbol files are written under the regression output directory
- **AND** existing integration build artifacts are not modified or deleted

### Requirement: Terminal state is captured deterministically

The workflow SHALL capture the program state when execution reaches the program
end, and SHALL use a configurable emulated-time fallback for programs that do not
terminate.

#### Scenario: Terminating program captured at end

- **WHEN** a program reaches its implicit or explicit end
- **THEN** RAM and VRAM are captured at that point before control returns to BASIC

#### Scenario: Non-terminating program captured by fallback

- **WHEN** a program loops or waits for input without reaching its end
- **THEN** RAM and VRAM are captured after the configured emulated-time fallback

### Requirement: Interactive programs receive automated input

The workflow SHALL support per-program keyboard and joystick input scripts so
interactive programs progress deterministically during regression.

#### Scenario: Keyboard input is injected

- **WHEN** a program waits for keyboard input and an input script is configured
- **THEN** the configured keys are injected in order at the configured times
- **AND** the program advances past the input prompt

#### Scenario: Joystick input is injected

- **WHEN** a program polls a joystick and a joystick script is configured
- **THEN** the configured directions and buttons are applied during emulation

#### Scenario: Program without an input script

- **WHEN** a program is interactive but no input script is configured
- **THEN** the workflow captures the waiting state using the fallback capture point

### Requirement: Validation is memory-first

The workflow SHALL validate RAM and VRAM content before considering a screenshot,
and screenshots SHALL be used only for diagnostics or when no memory check exists.

#### Scenario: RAM validation decides the result

- **WHEN** a program has configured RAM checks
- **THEN** pass or fail is determined from RAM content
- **AND** a screenshot is not required to decide the result

#### Scenario: VRAM validation decides the result

- **WHEN** a program has configured VRAM checks and no screenshot baseline
- **THEN** pass or fail is determined from VRAM content

#### Scenario: Screenshot used as fallback

- **WHEN** a program has no configured memory checks
- **THEN** the workflow MAY use a screenshot comparison as a last resort
- **AND** it records that the decision was visual

### Requirement: RAM checks support probes and stable snapshots

The workflow SHALL support explicit RAM probes and stable RAM snapshot comparison,
including masking of ranges known to vary between runs.

#### Scenario: Explicit probe matches

- **WHEN** a configured RAM address contains the expected value at capture time
- **THEN** the probe passes

#### Scenario: Stable snapshot differs

- **WHEN** a byte in the stable portion of the RAM snapshot differs from the baseline
- **THEN** the program fails and the differing ranges are reported

#### Scenario: Volatile range is masked

- **WHEN** a RAM range is configured as volatile
- **THEN** differences in that range do not fail the program

### Requirement: VRAM checks support text and region hashes

The workflow SHALL support text extraction for text screen modes and region hashes
for graphical screen modes, using VDP or BIOS-derived addresses.

#### Scenario: Text mode validation

- **WHEN** a program captures a text screen mode
- **THEN** the workflow extracts characters from the name table
- **AND** compares them with configured expected text

#### Scenario: Graphical mode validation

- **WHEN** a program captures a graphical screen mode
- **THEN** the workflow compares hashes of configured VRAM regions with the baseline

### Requirement: Baselines can be generated and refreshed

The workflow SHALL generate and refresh memory baselines from a known-good run and
SHALL report differences per program and per region.

#### Scenario: Generate a baseline

- **WHEN** baseline generation is requested
- **THEN** RAM and VRAM captures are stored as the reference for the selected programs

#### Scenario: Refresh a baseline

- **WHEN** baseline refresh is requested for selected programs
- **THEN** only those programs' baselines are replaced

#### Scenario: Report baseline differences

- **WHEN** a regression run finds a difference
- **THEN** the report names the program, memory kind, and differing range or byte

### Requirement: Volatile programs are explicitly marked

Programs whose output depends on randomness or clock values SHALL be marked as
volatile and SHALL fail only on their configured stable checks.

#### Scenario: Volatile program does not fail on random bytes

- **WHEN** a volatile program produces different random or clock-dependent bytes
- **THEN** those bytes do not cause a regression failure
- **AND** any configured stable probes are still evaluated

### Requirement: Machine model is selected per program

The workflow SHALL select an MSX1 or MSX2 machine per program according to the
screen modes the program uses.

#### Scenario: MSX1 program uses the default machine

- **WHEN** a program uses only MSX1-compatible screen modes
- **THEN** the workflow runs it on the configured MSX1 machine

#### Scenario: MSX2 program uses an MSX2 machine

- **WHEN** a program uses an MSX2-only screen mode
- **THEN** the workflow runs it on the configured MSX2 machine

### Requirement: Regression artifacts and report are written per run

The workflow SHALL write per-program RAM, VRAM, optional screenshot, and a summary
report containing pass/fail status and failure cause.

#### Scenario: Successful program artifacts

- **WHEN** a program passes
- **THEN** its memory captures and status are recorded in the run output

#### Scenario: Failed program artifacts

- **WHEN** a program fails
- **THEN** its captures and the failure cause are preserved for inspection

### Requirement: Regression is local-only

The regression workflow SHALL NOT be added to CI/CD automation.

#### Scenario: CI pipeline remains unchanged

- **WHEN** the project CI pipeline runs
- **THEN** it does not invoke the regression workflow

#### Scenario: Regression is available locally

- **WHEN** a developer or an agent invokes the regression target locally
- **THEN** the workflow runs without requiring CI configuration
