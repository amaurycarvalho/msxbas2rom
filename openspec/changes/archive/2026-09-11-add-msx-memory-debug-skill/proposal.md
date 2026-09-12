## Why

Inspecting the runtime RAM contents of a generated MSX ROM currently requires an
interactive openMSX debugger session, which a coding agent cannot drive reliably.
The `msx-memory-debug` skill automates the whole cycle — launch a ROM, wait a
configurable amount of emulated time, dump the CPU-visible RAM and a screenshot,
then exit — so an agent gets deterministic artifacts to inspect after the run.

The original skill did not work on this machine: openMSX aborts at startup when the
user's `settings.xml` contains a key binding the installed version does not
recognise (`Invalid keycode: PrintScreen`), and when settings are isolated it falls
back to C-BIOS instead of the project's target Sharp HB-8000 (Hotbit) machine.

## What Changes

- Register the `msx-memory-debug` capability and its implementation under
  `.opencode/skills/msx-memory-debug-skill/`.
- Launcher isolates openMSX from the user's interactive `settings.xml` by generating
  a minimal settings file, passing it with `-setting`, and deleting it on exit, so
  the skill no longer crashes on host-specific bindings.
- Launcher selects the Sharp HB-8000 (Hotbit) machine explicitly with
  `-machine Sharp_HB-8000_1.2`, preventing a C-BIOS fallback; override with
  `MSX_DEBUG_MACHINE`.
- Tcl capture script reads `Main RAM` at `0xC000..0xFFFF` (16384 bytes) with
  `debug read_block`, saves a PNG screenshot, and exits openMSX.
- Configuration via `OPENMSX_EXECUTABLE`, `OPENMSX_ARGS`, `OPENMSX_FLATPAK_APP`,
  `MSX_DEBUG_DELAY`, `MSX_DEBUG_MACHINE`, `MSX_DEBUG_SETTINGS`,
  `MSX_DEBUG_SCREENSHOT_MODE`.
- The skill documents reusable debugging strategies: freeze observable state with
  `POKE` to fixed RAM addresses plus a halt loop; map addresses with `-s --noi` and
  `header.symbols.asm`; use openMSX breakpoints (`debug set_bp`, `debug read_block
  "CPU regs"`, `debug read_block "Main RAM"`, `debug cont`) for CPU-level questions;
  rebuild the embedded kernel header after kernel edits.
- MSX2-oriented BASIC commands SHALL be debugged on the Panasonic FS-A1WX machine
  (`MSX_DEBUG_MACHINE=Panasonic_FS-A1WX`); the MSX1 default remains Sharp HB-8000 1.2.
- Target release `1.3.0.0`.

## Capabilities

### New Capabilities

- `msx-memory-debug`: agent-driven openMSX memory/screenshot capture skill for
  deterministic post-run inspection of MSX ROMs.

### Modified Capabilities

- *(none)*

## Impact

- `.opencode/skills/msx-memory-debug-skill/SKILL.md`
- `.opencode/skills/msx-memory-debug-skill/README.md`
- `.opencode/skills/msx-memory-debug-skill/scripts/debug-msx-memory.sh`
- `.opencode/skills/msx-memory-debug-skill/scripts/msx-memory-capture.tcl`
- No product code, build, or runtime behavior is affected.
