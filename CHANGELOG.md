# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### Added

- [add-16bit-segments-ascii16-4mb-ascii16x-8mb](openspec/changes/add-16bit-segments-ascii16-4mb-ascii16x-8mb) 16-bit segment support enabling ASCII16 up to 4MB and ASCII16X up to 8MB ROMs
- [add-define-def-fn-preprocessor](openspec/changes/add-define-def-fn-preprocessor) Compile-time macro support via DEFINE and DEF FN preprocessor
- [fix-dim-position-order](openspec/changes/fix-dim-position-order) Fix DIM position order so array references before DIM work correctly
- [implement-double-via-float-float](openspec/changes/implement-double-via-float-float) Double precision operations via float emulation
- [improve-mutation-score](openspec/changes/improve-mutation-score) Raise the mutation score toward the 85% target with exact-output, sentinel-diverse unit tests
- [refactor-resource-number-hl-protocol](openspec/changes/refactor-resource-number-hl-protocol) Resource number passing standardized to HL register protocol
- [set-page-screen4](openspec/changes/set-page-screen4) SET PAGE support for screen 4 compatibility
- [wishlist](openspec/changes/wishlist) Explore and track potential future improvements not yet scheduled for any sprint

## [1.3.0.0] - 2026-09-14

### [2026-09-11-add-msx-memory-debug-skill](openspec/changes/archive/2026-09-11-add-msx-memory-debug-skill) Add an agent-driven openMSX memory/screenshot capture skill for deterministic post-run inspection of MSX ROMs

#### Added
- Register the `msx-memory-debug` capability and its implementation under `.opencode/skills/msx-memory-debug-skill/`
- Tcl capture script reads `Main RAM` at `0xC000..0xFFFF` (16384 bytes) with `debug read_block`, saves a PNG screenshot, and exits openMSX
- Configuration via `OPENMSX_EXECUTABLE`, `OPENMSX_ARGS`, `OPENMSX_FLATPAK_APP`, `MSX_DEBUG_DELAY`, `MSX_DEBUG_MACHINE`, `MSX_DEBUG_SETTINGS`, `MSX_DEBUG_SCREENSHOT_MODE`

#### Changed
- Isolate openMSX from the user's interactive `settings.xml` by generating a minimal settings file, passing it with `-setting`, and deleting it on exit, so the skill no longer crashes on host-specific bindings
- Select the Sharp HB-8000 (Hotbit) machine explicitly with `-machine Sharp_HB-8000_1.2`, preventing a C-BIOS fallback
- Document reusable debugging strategies: freeze observable state with `POKE` plus a halt loop, map addresses with `-s --noi` and `header.symbols.asm`, use openMSX breakpoints, and rebuild the embedded kernel header after kernel edits
- Debug MSX2-oriented BASIC commands on the Panasonic FS-A1WX machine; the MSX1 default remains Sharp HB-8000 1.2

### [add-sprite-hitbox-margins](openspec/changes/archive/2026-09-14-add-sprite-hitbox-margins) Add per-sprite configurable collision hitbox margins via SET SPRITE HITBOX

#### Added
- Add the `SET SPRITE HITBOX` BASIC command family with global and per-sprite `ON`/`OFF`/`AUTO` forms plus explicit `<sprite>, <left>, <top>, <right>, <bottom>` margins
- Add the `HITBOX_TABLE` runtime table (32 × 5 bytes) storing pre-normalized relative bounds (`X0`, `X1`, `Y0`, `Y1`) and a per-sprite enabled flag
- Add the single kernel routine `set_sprite_hitbox` with a mode byte implementing all command forms
- Add the `HITBOX` and `AUTO` lexer keywords

#### Changed
- Make `COLLISION()` combine each sprite's position from `SPRTBL` with its relative bounds from `HITBOX_TABLE`, keeping the public `COLLISION()`/`COLLISION(n)`/`COLLISION(n1,n2)` API unchanged
- Reset `HITBOX_TABLE` to the default full-rectangle state on sprite clear (`SUB_CLRSPR`), covering the SCREEN sprite-size path

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.3.0.0...HEAD
[1.3.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.3.0.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
