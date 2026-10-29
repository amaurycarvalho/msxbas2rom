# Proposal

## Why

Konami4 (`-4`) and KonamiSCC (`-k`) MegaROMs crash on MSX turbo R machines
(for example, `Panasonic_FS-A1GT`) before their BASIC code runs. The kernel's
boot routine only programs the Konami mapper banks when it finds the ROM header
signature `AB` at `0x8000`. On turbo R the mapper's `0x8000` window can point to
an empty segment at that moment, so the guard is skipped, the user-code segment
is never mapped, and execution falls into empty memory. ASCII8, ASCII16, and
ASCII16X are unaffected because their boot paths or builder patches do not rely
on that guard.

## What Changes

- Make the Konami4/KonamiSCC boot bank mapping unconditional: the builder SHALL
  NOP the 14-byte `AB` guard at `megarom_ascii8_bug_fix` so the writes to
  `0x7000`, `0x8000`, and `0xA000` always execute.
- Add a mapper-neutral local alias `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` for the
  shared guard patch point, replacing the misleading ASCII16X-only name at the
  call site.
- Keep ASCII8 and ASCII16 behavior unchanged; only Konami4/KonamiSCC get the
  unconditional mapping.
- Extend the regression runner with per-program `compile_modes` and `machines`
  matrices, so one probe can run on MSX1, MSX2, MSX2+, and turbo R, and in both
  Konami modes.
- Correct the runner's true MSX2 default to `Panasonic_FS-A1` and keep
  `Panasonic_FS-A1WX` for MSX2+.
- Add two new regression programs (not `turbo_test1`):
  - `MISC/megaboot.bas`: machine-independent boot probe (`MEGAROM BOOT OK`,
    `SUM=5050`) run on MSX1, MSX2, MSX2+, and turbo R.
  - `MISC/turbo_konami.bas`: turboR-focused Konami/KonamiSCC probe that switches
    turbo off/on and prints `TURBO KONAMI OK`.
- Add builder unit tests for both Konami modes and non-regression checks for
  ASCII8/ASCII16.

## Capabilities

### New Capabilities

- (none; the fix and its coverage extend existing capabilities)

### Modified Capabilities

- `konami4-mapper`: Konami4 and KonamiSCC boot bank mapping becomes
  unconditional and no longer depends on the `AB` header being visible at
  `0x8000`.
- `builder`: `fixKonamiMapper()` NOPs the shared 14-byte `AB` guard for both
  Konami modes, using a mapper-neutral alias for the dispatch entry.
- `emulator-regression-tests`: per-program mapper and machine matrices, a true
  MSX2 default machine, and new Konami/KonamiSCC regression probes.

## Impact

- Builder: `src/application/builder/rom.cpp` (`Rom::fixKonamiMapper`).
- Unit tests: `tests/unit/src/test_rom.cpp`.
- Regression runner: `tests/integration/regression/runner.py`,
  `manifest.schema.json`, `manifest.json`, `test_runner.py`.
- New regression programs: `tests/integration/MISC/megaboot.bas`,
  `tests/integration/MISC/turbo_konami.bas`.
- Specs: `openspec/specs/konami4-mapper/spec.md`,
  `openspec/specs/builder/spec.md`,
  `openspec/specs/emulator-regression-tests/spec.md`.
- No kernel source or generated kernel artifact changes are required; the fix is
  a build-time binary patch. Kernel size and architecture checks still run as
  gates.
