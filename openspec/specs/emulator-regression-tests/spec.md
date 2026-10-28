# emulator-regression-tests Specification

## Purpose

Provides a local, scenario-based regression workflow that compiles MSX BASIC
programs, runs them in openMSX, and validates runtime behavior through RAM and
VRAM memory checks instead of screenshot comparison.

## Requirements

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
terminate. Terminal capture SHALL happen before the end routine restores text
mode or otherwise cleans up the display state.

#### Scenario: Terminating program captured at end

- **WHEN** a program reaches its implicit or explicit end
- **THEN** RAM and VRAM are captured at the pre-cleanup END entry
- **AND** the captured state still reflects the program's graphics mode

#### Scenario: Non-terminating program captured by fallback

- **WHEN** a program loops or waits for input without reaching its end
- **THEN** RAM and VRAM are captured after the configured emulated-time fallback

#### Scenario: Implicit end is captured before text restore

- **WHEN** a program falls off its last line and terminates implicitly
- **THEN** the capture breakpoint is the pre-cleanup END entry
- **AND** `END_PGM` is used only as a fallback when that entry is unavailable

#### Scenario: Captured mode conflicts with source mode

- **WHEN** the captured VDP registers indicate text mode but the program's last
  literal SCREEN is a graphics mode
- **THEN** the report records the conflict
- **AND** the workflow does not silently select text-mode VRAM regions

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

The workflow SHALL select the least-capable configured machine that satisfies
each program's machine class. The class SHALL be inferred from screen modes and
turbo-capable commands, or declared explicitly. The workflow SHALL support MSX1,
MSX2, MSX2+, and MSXturboR classes.

#### Scenario: MSX1 program uses the default machine

- **WHEN** a program uses only MSX1-compatible screen modes and no turbo-capable
  command
- **THEN** the workflow runs it on the configured MSX1 machine

#### Scenario: MSX2 program uses an MSX2 machine

- **WHEN** a program uses an MSX2-only screen mode
- **THEN** the workflow runs it on the configured MSX2+ machine
- **AND** that machine is `Panasonic_FS-A1WX` by default

#### Scenario: MSX2 or MSX2+ program uses the MSX2+ machine

- **WHEN** a program uses `SCREEN 4` through `SCREEN 12`, with or without a space
  after `SCREEN`
- **THEN** the workflow runs it on the configured MSX2+ machine
- **AND** that machine is `Panasonic_FS-A1WX` by default

#### Scenario: Turbo-capable command without turbo R requirement uses MSX2+

- **WHEN** a program uses `CMD TURBO`, `CALL TURBO`, or `TURBO()` and does not
  declare a turbo R requirement
- **THEN** the workflow runs it on the configured MSX2+ machine

#### Scenario: Turbo R program uses the turbo R machine

- **WHEN** a program declares a turbo R requirement
- **THEN** the workflow runs it on the configured turbo R machine
- **AND** that machine is `Panasonic_FS-A1GT` by default

#### Scenario: Explicit machine override wins

- **WHEN** a program declares an explicit machine
- **THEN** the workflow uses that machine regardless of inferred class

#### Scenario: V9958 graphical mode is decoded

- **WHEN** a captured VDP register pair is `R0=0x06`, `R1=0x60`
- **THEN** the workflow reports screen mode 5, not an MSX1 mode

### Requirement: Regression exposes and verifies the selected machine

The workflow SHALL expose each program's selected machine and machine class in
the audit/list output and the run report, and SHALL reject a baseline generated
for a different machine.

#### Scenario: Audit lists the selected machine

- **WHEN** the audit/list command runs for a program
- **THEN** it reports the program, its machine class, and the selected machine

#### Scenario: Baseline machine mismatch is rejected

- **WHEN** a baseline records a different machine than the selected machine
- **THEN** the program fails before memory comparison
- **AND** the failure asks the operator to regenerate the baseline

### Requirement: Turbo R feature programs are assigned explicitly

Programs whose purpose is to exercise turbo R features SHALL declare a turbo R
machine requirement and SHALL run on `Panasonic_FS-A1GT`. Programs that use
turbo-capable commands only for performance SHALL remain on the MSX2+ machine
unless they declare a turbo R requirement.

#### Scenario: Turbo R feature test

- **WHEN** `MISC/test1`, `MISC/test64`, or `MISC/test65` is selected
- **THEN** the workflow runs it on `Panasonic_FS-A1GT`

#### Scenario: Performance-only turbo usage

- **WHEN** `GRAPH/tree`, `GRAPH/mexhat`, `GRAPH/test78`, `IO/test48`, or
  `MATH/cos` is selected without an explicit turbo R requirement
- **THEN** the workflow runs it on the configured MSX2+ machine

#### Scenario: Turbo R output is asserted

- **WHEN** a turbo R feature program runs on `Panasonic_FS-A1GT`
- **THEN** its configured text/RAM checks assert the turbo R-specific output
- **AND** timing-dependent values remain volatile

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

### Requirement: PRINT and STRING scenarios validate the 40-column text layout

The regression workflow SHALL include the split `PRINT/test5` and `STRING/test1` programs with explicit 40-column text extraction, expected text for separator and concatenation forms, and VRAM byte probes for rows whose spacing or content is significant. Probes SHALL be positioned on rows that no later statement overwrites.

#### Scenario: PRINT separator layout is validated

- **WHEN** `PRINT/test5` is run through the regression workflow
- **THEN** the captured text contains the expected rows for trailing `;`, trailing `,`, repeated commas, and leading separators
- **AND** byte probes confirm the exact tab/space layout for those rows
- **AND** each `PRINT` without a trailing separator advances to its own row

#### Scenario: Concatenated STRING$ result is validated

- **WHEN** `STRING/test1` is run through the regression workflow
- **THEN** the captured name table contains `AAAAABBBBBB` for `STRING$(5,65)+STRING$(6,66)`
- **AND** it contains `AAAAABBBBBB!!!` for the literal concatenation with a suffix
- **AND** it contains `AAAAABBBBB` for each variable-based concatenation
- **AND** a VRAM probe at the first character of each result matches the expected bytes

#### Scenario: PRINT line layout is validated

- **WHEN** either program reaches its terminal capture point
- **THEN** the captured rows begin at column 0 of the configured 40-column name table
- **AND** a regression using the pre-fix 32-column layout fails the configured text or probe checks

### Requirement: All PRINT and STRING$ regression expectations are reviewed

The change SHALL audit every regression program whose source uses `PRINT`, `?`, or `STRING$`, and SHALL review or regenerate its baseline and manifest checks after the default screen layout changes.

#### Scenario: PRINT and STRING baselines are refreshed

- **WHEN** the startup layout changes to 40 columns
- **THEN** every existing `PRINT/*` and `STRING/*` baseline is regenerated and reviewed
- **AND** the new `PRINT/test5` and `STRING/test1` baselines are added

#### Scenario: Programs with explicit SCREEN statements are checked

- **WHEN** a regression program using `PRINT`, `?`, or `STRING$` sets its own `SCREEN`
- **THEN** its captured mode is verified against that statement
- **AND** its text or VRAM probes are updated if `STRING$` results changed

#### Scenario: Manifest text checks use the fixed layout

- **WHEN** a manifest entry configures `vram.text` for a program that does not set `SCREEN`
- **THEN** its `name_table` and `width` match the fixed 40-column `SCREEN 0` layout
- **AND** the baseline records the corrected screen mode
