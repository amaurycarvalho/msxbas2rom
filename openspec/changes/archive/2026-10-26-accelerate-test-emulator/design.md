# Design

## Context

See `proposal.md` for motivation. The current runtime checks under
`tests/dispatcher/` launch openMSX 59 times. Each launch uses the default
real-time throttle and a fixed 8-12 emulated-second delay before capture, so the
suite is dominated by waiting in real time. The scripts duplicate their emulator
environment setup and never export or consume debugger symbols.

The regression workflow already solves the same problem in
`tests/integration/regression/runner.py`: it disables throttling when no host input
is present, compiles with `-s --noi`, resolves terminal addresses from the `.noi`
file with a 16-bit mask, installs breakpoints before boot, and keeps the emulated
delay as a fallback.

Implementation testing exposed a MegaROM constraint: openMSX breakpoints match the
CPU address, not the mapper block. The same address can be executed from another
bank during boot, so a line breakpoint can fire before the marker write. The
`.noi` symbol contains the segment in its high bits, which can be used with an
openMSX `pc_in_slot` condition to require the correct mapper block.

## Goals / Non-Goals

**Goals:**

- Reuse the regression acceleration strategy in all five dispatcher runtime
  checks without changing their assertions or mapper coverage.
- Expose acceleration and conditional breakpoints through the
  `msx-memory-debug` skill instead of keeping the policy only in test helpers.
- Capture at the program's terminal marker loop instead of waiting for the full
  fallback delay, including in MegaROM mappers where CPU addresses are reused.
- Keep the fallback path working when symbols are missing or a breakpoint never
  fires.
- Centralize the emulator environment and symbol handling so the five scripts do
  not drift.

**Non-Goals:**

- Do not change the compiler, kernel, product CLI, or `make test-regression`.
- Do not add parallel emulator execution.
- Do not add marker watchpoints; conditional breakpoints are sufficient.
- Do not refresh or change regression baselines.
- Do not change existing skill behavior when the new options are absent.

## Decisions

### D1: Add an explicit acceleration mode to the skill

**Choice:** The skill accepts `MSX_DEBUG_ACCELERATE=1`. When enabled and no host
input script is configured, the skill disables throttling. If an input script is
present, it keeps real-time throttling so input timing is preserved. An explicit
`MSX_DEBUG_THROTTLE` value still wins.

**Why:** This keeps the acceleration policy in the skill, where the capture
lifecycle lives, and lets any caller opt in without duplicating the throttle
rules. The runtime-check helper enables acceleration rather than setting throttle
directly.

**Alternative considered:** Setting `MSX_DEBUG_THROTTLE=off` from the helper.
Rejected because the user asked for the capability to live in the skill, and
because the skill already knows whether an input script is present.

### D2: Breakpoint on the terminal marker loop with a mapper-block condition

**Choice:** Compile BASIC checks with `-s --noi`, resolve `LIN_<loop-line>` from
the exported symbols, mask the CPU address with `& 0xFFFF`, and install that
address as a pre-boot breakpoint. For MegaROM targets, also derive the mapper
block from the symbol's segment and add an openMSX condition
`[pc_in_slot X X <block>]`. For the pasmo prototype, resolve the `loop` symbol
from pasmo's symbol output.

**Why:** The dispatcher programs are deliberately non-terminating: they write a
marker and then spin in a `GOTO` loop. `END_STMT+2` and `END_PGM` never fire.
Breaking at the first instruction of the loop after the marker captures the same
terminal state the fallback would observe, without the wait. The condition is
required because MegaROM mappers reuse CPU addresses across banks, and boot code
can execute the same address in a different bank before the program starts.

The `.noi` segment is expressed in 8 KiB units. For 8 KiB mappers (ASCII8,
Konami, Konami SCC), the mapper block equals the segment. For 16 KiB mappers
(ASCII16 and ASCII16-X), the mapper block is the segment divided by two. Plain
ROMs have no mapper and use no condition.

**Alternative considered:** Keep waiting for the fallback. Rejected because it
dominates suite time and is less deterministic for programs that finish early.
**Alternative considered:** Break on a marker write watchpoint. Rejected because
conditional breakpoints solve the bank ambiguity with a smaller skill change.

### D3: Shared helper under `tests/dispatcher/`

**Choice:** Add one helper module that owns the machine name, headless SDL setup,
throttle rule, `.noi`/pasmo symbol parsing, breakpoint formatting, and skill
invocation. The five checks call it and keep only their own compile commands,
marker addresses, and program-specific break lines.

**Why:** The five scripts already duplicate the environment. Centralizing the
acceleration rule makes it harder for one check to silently keep the old slow
path and gives a single place to document the marker-loop convention.

**Alternative considered:** Import the regression runner. Rejected because it
couples kernel/dispatcher tests to the integration regression CLI and its
manifest/artifact assumptions.

### D4: Preserve fallback delays as safety nets

**Choice:** Keep the existing per-check fallback delays (8-12 emulated seconds)
and the `MSX_DEBUG_DELAY` environment variable. Only use the breakpoint when
symbols resolve successfully.

**Why:** A missing `.noi` file, a future program that never reaches the loop, or
an assembler change must still produce a capture rather than fail the check.
Because throttling is off, the fallback is cheap in wall-clock terms.

### D5: Add explicit break lines to BASIC check cases

**Choice:** Extend each page-safe and overflow case with the line number of its
terminal loop, rather than deriving it from the marker line or program text.

**Why:** Explicit line numbers keep the convention visible and avoid fragile
source parsing. The current convention is: write the marker, then immediately
enter a self-loop; the breakpoint line is the self-loop.

### D6: Extend the skill with acceleration and conditional breakpoints

**Choice:** Add two opt-in skill features:

- `MSX_DEBUG_ACCELERATE` disables throttling when no host input script is present;
- `MSX_DEBUG_BREAKPOINTS` entries accept an optional `@CONDITION` suffix, for
  example `LIN_50=0x8152@[pc_in_slot X X 2]`.

**Why:** Conditional breakpoints are required for bank-aware MegaROM capture, and
acceleration belongs with the capture lifecycle so all callers share one policy.
Both features are opt-in, so existing invocations remain unchanged.

**Alternative considered:** Inject a custom `-command` through `OPENMSX_ARGS` or a
wrapper executable. Rejected because the quoting is fragile and the Flatpak path
would need separate handling.

## Risks / Trade-offs

- **[Risk] A future check performs side effects after writing its marker but
  before the loop** → Document the convention and require the break line to be
  the first line after all observable work.
- **[Risk] MegaROM breakpoints fire from the wrong bank during boot** → Require a
  `pc_in_slot` condition for MegaROM captures and derive the mapper block from the
  `.noi` segment.
- **[Risk] The `.noi` segment is not the mapper block for every mapper** → Map
  8 KiB segments to 8 KiB or 16 KiB blocks explicitly; verify all five mappers.
- **[Risk] `.noi` addresses can exceed 16 bits in the exported text** → The helper
  masks every resolved address with `& 0xFFFF`, matching the regression runner.
- **[Risk] Unthrottled emulation changes timing-sensitive behavior** → Apply it
  only to checks without host input; the checks assert RAM marker bytes, not
  wall-clock timing.
- **[Risk] Missing or renamed symbols silently disable acceleration** → The
  helper falls back to the existing delay and the check still runs; tests compare
  results rather than asserting that a breakpoint fired.
- **[Risk] Headless rendering behaves differently at high speed** → Headless
  capture with acceleration and a conditional breakpoint is verified locally; the
  existing SDL dummy setup is preserved.
- **[Risk] A Tcl condition contains a comma or brace and breaks parsing** → Keep
  conditions simple and document that they cannot contain the item separator.

## Migration Plan

1. Extend the skill with acceleration and conditional breakpoints, and document
   both options.
2. Extend the shared helper to emit conditions and map segment to mapper block.
3. Convert one check first and compare its result with the current behavior.
4. Convert the remaining checks and the pasmo prototype.
5. Run every dispatcher check individually, then `make test-emulator`.
6. Roll back by reverting the conversion; the fallback path keeps the old capture
   semantics available at every step.

## Open Questions

None.
