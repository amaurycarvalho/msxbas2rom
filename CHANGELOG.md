# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [1.0.0.0] - 2026-06-19

### Added

- File handling support implementation (US-007).
- Spec migration to OpenSpec format.
- Changelog standardized to Keep a Changelog format.
- Windows Unicode argv support via `__wargv` to UTF-8 conversion on program startup.
- `--` (end-of-options) support in CLI parser for filenames starting with `-`.
- `SET TILE PATTERN` buffer form with 4-integer array and bank parameter.
- `GET TILE PATTERN` with bank parameter (former stub now implemented).
- Z80 kernel bulk LDIRVM/LDIRMV routines for `get_tile_pattern` / `set_tile_pattern_buffer`.
- `SET TILE COLOR` all syntax variants: inline FC/BC, per-row FC array (with/without BC array), and buffer form (`SET TILE COLOR <n>, <4-int-array> [, <bank>]`), with bank parameter support (0-2 for individual banks, 3 or omitted for all banks).
- `GET TILE COLOR` with bank parameter: `GET TILE COLOR <n>, <4-int-array> [, <bank>]` with LDIRMV read via `get_tile_color` (replaces stub).
- Z80 kernel routines for `set_tile_color_buf` (3-pass LDIRVM for bank=all) and `get_tile_color` with screen mode validation.
- Unit and integration tests for all SET/GET TILE COLOR syntax forms.
- `SET TILE FLIP <n>, <dir>[, <bank>]` with 3 directions (0=horizontal, 1=vertical, 2=both) and optional bank parameter (0-2 specific, 3=all).
- `SET TILE ROTATE <n>, <dir>[, <bank>]` with 3 directions (0=left, 1=right, 2=180) and optional bank parameter.
- Screen 2 validation (returns early on screen >= 5).
- Reuses existing sprite flip/rotate Z80 routines (`binaryReverseA`, `blockRotateL`, `blockRotateR`) for 8-byte tile blocks.
- Unit parser, compiler, and integration tests for all flip/rotate syntax forms.

### Changed

- Magic constants in RAM percentage calculation replaced with named constants.
- Platform detection normalized from `#ifdef Win` to standard `#ifdef _WIN32`.
- Removed dead includes (`<malloc.h>`, `<math.h>`) from `main.cpp`.

### Fixed

- Uninitialized `vscode` member causing "VSCode already initialized" error on Windows.
- `pathExists()` on Windows: replaced stub with `GetFileAttributesA`.
- `-?` help option removed due to glob expansion risk in POSIX shells.
- `set_tile_flip` / `set_tile_rotate`: direction in HL was clobbered by VRAM address operations (`get_tile_vram_addr` + `set_tile.copy`), causing all directions to behave as direction=2 (both). Fixed by saving HL on stack before VRAM ops.
- `SET TILE FLIP` / `SET TILE ROTATE` with 3 params: bank evaluation (`evalExpression`) overwrote HL (direction) before the call. Fixed by saving HL on stack before bank evaluation.
- `set_tile_rotate` left/right: destination buffer was `STRBUF+32` (sprite size) causing rotation output to be written past the tile data while source data was shifted to zero. Fixed by backing up tile to `STRBUF+8` and rotating from backup.

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.0.0.0...HEAD
[1.0.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.0.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
