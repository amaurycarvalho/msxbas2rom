# Proposal

## Why

`make test-emulator` launches openMSX roughly 59 times with real-time throttling
and fixed 8-12 second emulated delays, so a full run takes around eleven minutes
and feedback arrives too late. `make test-regression` already runs unthrottled and
captures at a pre-boot terminal-state breakpoint, reducing a representative
capture from about 10.8 seconds to about 1 second. The dispatcher and kernel
module runtime checks should use the same acceleration strategy.

## What Changes

- Add a shared helper for `tests/dispatcher/` runtime checks that runs openMSX
  with acceleration enabled when no host input script is present, installs
  pre-boot breakpoints, and preserves the emulated-time delay as a fallback.
- Add an explicit acceleration option to the `msx-memory-debug` skill so capture
  callers do not have to set low-level throttle state directly.
- Add optional breakpoint conditions to the skill (`LABEL=ADDRESS@CONDITION`) so
  MegaROM checks can require the correct mapper block before capturing.
- Compile BASIC runtime-check programs with `-s --noi`; resolve the terminal loop
  line (`LIN_<line>`) from the `.noi` symbols, mask addresses with `& 0xFFFF`, and
  derive the mapper block from the symbol's segment for conditional breakpoints.
- Install the terminal-state breakpoint on the first line of the marker loop,
  after the program has written its RAM marker.
- Export and parse the pasmo `loop` symbol for the hand-written dispatcher
  prototype.
- Preserve every existing check, marker address, machine selection, headless SDL
  behavior, and pass/fail result.
- Do not change `make test-regression`, product runtime behavior, or enable
  parallel emulator execution.

## Capabilities

### New Capabilities

- `emulator-runtime-checks`: accelerated, deterministic openMSX execution and
  capture for dispatcher and kernel module runtime checks.

### Modified Capabilities

- `msx-memory-debug`: adds an explicit acceleration option and optional
  breakpoint conditions for bank-aware MegaROM captures.

## Impact

- `tests/dispatcher/check_dispatcher.py`, `check_plain_overflow.py`,
  `check_megarom_overflow.py`, `check_megarom_resources.py`, and
  `check_page_safe_modules.py`
- New shared helper under `tests/dispatcher/`
- `.opencode/skills/msx-memory-debug-skill/scripts/debug-msx-memory.sh`,
  `SKILL.md`, and `README.md`
- Temporary test build outputs only; no compiler, kernel, or CLI production code
- `make test-emulator` becomes faster without changing the Makefile entry point
