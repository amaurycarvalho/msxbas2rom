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
- [refactor-resource-number-hl-protocol](openspec/changes/refactor-resource-number-hl-protocol) Resource number passing standardized to HL register protocol
- [set-page-screen4](openspec/changes/set-page-screen4) SET PAGE support for screen 4 compatibility
- [wishlist](openspec/changes/wishlist) Explore and track potential future improvements not yet scheduled for any sprint

### Fixed

- [fix-pletter-buffer-overflow](openspec/changes/fix-pletter-buffer-overflow) Fix Pletter heap overflow for small resource blobs by returning an owned output buffer and sizing the compressor from the worst-case bound

## [1.4.1.0] - 2026-10-27

### [add-rsctoram-offset-size](openspec/changes/archive/2026-10-26-add-rsctoram-offset-size) Add optional offset/size parameters to CMD RSCTORAM for partial resource copies

#### Added
- Add optional `offset` and `size` parameters to `CMD RSCTORAM`

#### Changed
- Clamp the effective copy length to `min(size, resource_size - offset)`
- Treat an omitted `size` as the full remaining resource
- Ignore `offset` and `size` when `pletter = 1` and decompress the whole resource as before
- Return `HL = dest` on success and `HL = 0` when the resource is missing or nothing is copied
- Pass command parameters through the `PARM1` work area
- Preserve existing 2- and 3-argument `CMD RSCTORAM` behavior

### [2026-10-27-harden-tile-bank-parameters](openspec/changes/archive/2026-10-27-harden-tile-bank-parameters) Normalize tile bank values above 2 to all banks and harden tile command parameter validation

#### Added
- Add deterministic VRAM probes to `test87`, `test88`, `test97`, and `tile_bank_all`, and require the affected tile programs to run in the GRAPH regression
- Add unit, kernel-build, and emulator regression coverage for bank values 3 and well above 3

#### Changed
- Normalize tile bank selection in the Z80 kernel: any bank value at or above 3 means "all banks" for SET TILE PATTERN, SET TILE COLOR, SET TILE FLIP, and SET TILE ROTATE
- Normalize GET TILE PATTERN and GET TILE COLOR bank values at or above 3 to bank 0, the documented default, so they never compute an invalid VRAM address
- **BREAKING**: require an array destination for `GET TILE PATTERN` and `GET TILE COLOR`; literals and scalar variables are rejected instead of being used as RAM pointers
- **BREAKING**: distinguish buffer arrays from scalar FC/BC values with the declared array flag, so `SET TILE COLOR 5, F%, G%` means foreground/background values rather than a buffer copy
- **BREAKING**: reject tile tuple/array parameters with more than 8 rows, bank parameters that are arrays, and inline `SET TILE PATTERN` parameter counts above 3 instead of compiling a silent no-op
- Harden tile parameter handling: mask background color values to 4 bits, validate tile FLIP/ROTATE direction and bank parameter shapes, and treat SCREEN 0/1 pattern writes as single-bank operations

#### Fixed
- Fix `SET TILE COLOR` bank forms so the bank parameter never overwrites the FC/BC operands or skips the FC array loop

### [redefine-resource-function](openspec/changes/archive/2026-10-26-redefine-resource-function) Redefine RESOURCE(n) to load the resource into free RAM and return a usable pointer

#### Changed
- Redefine `RESOURCE(n)` to copy the resource into `HEAP()` via `CMD RSCTORAM` and return a directly usable RAM pointer
- Return `0` when the resource does not exist and truncate the copy to `FRE()` bytes when the resource is larger
- Keep `RESOURCESIZE(n)` and the existing `DISP_cmd_rsctoram` dispatch unchanged

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.4.1.0...HEAD
[1.4.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.4.1.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
