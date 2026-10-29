# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### [add-16bit-segments-ascii16-4mb-ascii16x-8mb](openspec/changes/add-16bit-segments-ascii16-4mb-ascii16x-8mb) 16-bit segment support enabling ASCII16 up to 4MB and ASCII16X up to 8MB

### [add-define-def-fn-preprocessor](openspec/changes/add-define-def-fn-preprocessor) Compile-time macro support via DEFINE and DEF FN preprocessor

### [fix-dim-position-order](openspec/changes/fix-dim-position-order) Fix DIM position order so array references before DIM work correctly

### [harden-cpp-parameter-validation-tier1](openspec/changes/harden-cpp-parameter-validation-tier1) Add compile-time parameter contracts for Arkos commands, FRE, RANDOMIZE, literal ranges, and function arity

### [harden-cpp-parameter-validation-tier2](openspec/changes/harden-cpp-parameter-validation-tier2) Add resource-aware, MTF, and fixed-table parameter validation in the compiler

### [harden-cpp-parameter-validation-tier3](openspec/changes/harden-cpp-parameter-validation-tier3) Introduce strict command/function parameter typing and a unified fixed-table index policy

### [harden-turbo-r-r800-optional](openspec/changes/harden-turbo-r-r800-optional) Validate and harden remaining turbo R PSG, MTF, mouse, and R800 instruction-parity candidates

### [harden-z80-parameter-guards-tier1](openspec/changes/harden-z80-parameter-guards-tier1) Add runtime guards for sprite tables, resource metadata, RUNASM, and PLYSOUND

### [harden-z80-parameter-guards-tier2](openspec/changes/harden-z80-parameter-guards-tier2) Extend runtime guards to MTF headers/clipping, pattern/color indices, resource sizes, font/bank, and mode/channel values

### [harden-z80-parameter-guards-tier3](openspec/changes/harden-z80-parameter-guards-tier3) Introduce a shared kernel guard layer and malformed-resource regression suite

### [implement-double-via-float-float](openspec/changes/implement-double-via-float-float) Double precision operations via float emulation

### [refactor-resource-number-hl-protocol](openspec/changes/refactor-resource-number-hl-protocol) Resource number passing standardized to HL register protocol

### [set-page-screen4](openspec/changes/set-page-screen4) SET PAGE support for screen 4 compatibility

### [wishlist](openspec/changes/wishlist) Explore and track potential future improvements not yet scheduled for any sprint

## [1.4.3.0] - 2026-10-29

### [2026-10-29-fix-konami-megarom-boot-mapping](openspec/changes/archive/2026-10-29-fix-konami-megarom-boot-mapping) Fix Konami4/KonamiSCC MegaROM boot mapping on MSX turbo R

#### Added
- Add a mapper-neutral `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` alias for the shared guard patch point.
- Add `MISC/megaboot.bas`, a machine-independent boot probe run on MSX1, MSX2, MSX2+, and turbo R.
- Add `MISC/turbo_konami.bas`, a turbo R-focused Konami/KonamiSCC probe that switches turbo off and on.
- Add builder unit tests for both Konami modes and non-regression checks for ASCII8/ASCII16.

#### Changed
- Make the Konami4/KonamiSCC boot bank mapping unconditional by NOPing the 14-byte `AB` guard at `megarom_ascii8_bug_fix`.
- Keep ASCII8 and ASCII16 behavior unchanged; only Konami4/KonamiSCC get the unconditional mapping.
- Extend the regression runner with per-program `compile_modes` and `machines` matrices.

#### Fixed
- Correct the regression runner's true MSX2 default to `Panasonic_FS-A1` while keeping `Panasonic_FS-A1WX` for MSX2+.

### [2026-10-29-harden-turbo-r-r800-priority](openspec/changes/archive/2026-10-29-harden-turbo-r-r800-priority) Clamp CMD TURBO on turbo R to supported Z80 and R800 ROM modes

#### Added
- Promote the `CMD TURBO 2` diagnostic into the permanent `MISC/turbo_mode_clamp` regression program with `requires: turboR`, `volatile: true`, and stable text expectations.

#### Changed
- Clamp `CMD TURBO` on turbo R to Z80 mode (0) and R800 ROM mode (1) before calling `CHGCPU`, so values outside 0/1 never enter R800 DRAM mode.
- Keep the RAM canary and mapper state intact and document R800 DRAM mode as unsupported.

### [2026-10-29-spike-turbo-r-r800-fragility](openspec/changes/archive/2026-10-29-spike-turbo-r-r800-fragility) Spike the R800/turbo R failure candidates and record the reproducible breakage matrix

#### Added
- Add a temporary diagnostic harness under `tests/integration/MISC/` covering VDP resource copy, VDP graphics/scroll, ARKOS/direct PSG, mouse PSG read, integer math, and `CMD TURBO 2`.
- Produce a breakage matrix/report under the change directory with per-path evidence and suspected mechanisms.

#### Changed
- Run the diagnostics on openMSX `Panasonic_FS-A1GT` and on real hardware when available, capturing RAM/VRAM artifacts, screenshots, logs, and audio observations.
- Rewrite the compatibility work into `harden-turbo-r-r800-priority` and `harden-turbo-r-r800-optional`.

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.4.3.0...HEAD
[1.4.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.4.3.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
