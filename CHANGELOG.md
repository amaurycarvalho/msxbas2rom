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

### [harden-z80-parameter-guards-tier1](openspec/changes/harden-z80-parameter-guards-tier1) Add runtime guards for sprite tables, resource metadata, RUNASM, and PLYSOUND

### [harden-z80-parameter-guards-tier2](openspec/changes/harden-z80-parameter-guards-tier2) Extend runtime guards to MTF headers/clipping, pattern/color indices, resource sizes, font/bank, and mode/channel values

### [harden-z80-parameter-guards-tier3](openspec/changes/harden-z80-parameter-guards-tier3) Introduce a shared kernel guard layer and malformed-resource regression suite

### [implement-double-via-float-float](openspec/changes/implement-double-via-float-float) Double precision operations via float emulation

### [refactor-resource-number-hl-protocol](openspec/changes/refactor-resource-number-hl-protocol) Resource number passing standardized to HL register protocol

### [set-page-screen4](openspec/changes/set-page-screen4) SET PAGE support for screen 4 compatibility

### [wishlist](openspec/changes/wishlist) Explore and track potential future improvements not yet scheduled for any sprint

## [1.4.2.0] - 2026-10-28

### [fix-pletter-buffer-overflow](openspec/changes/archive/2026-10-28-fix-pletter-buffer-overflow) Fix Pletter heap overflow for small resource blobs by returning an owned output buffer and sizing the compressor from the worst-case bound

#### Added
- Add a single `Pletter::maxPackedSize(n)` bound (the literal worst case) for tests, reservations, and remaining capacity checks.
- Add regression tests for 1, 2, 3, 4, 5, and 6-byte blobs, the 8K boundary, empty input, and the `packedSize <= maxPackedSize(n)` contract.

#### Changed
- Replace `Pletter::pack`'s raw `unsigned char*` destination with a `std::vector<unsigned char>` output so the compressor returns an owned buffer with the exact compressed size. **BREAKING** for the two internal C++ call sites; no change to the BASIC language, kernel ABI, or ROM format.
- Make `Pletter::saves` grow dynamically (`clear`/`reserve`/`push_back`) instead of assuming `length * 2` bytes; this removes the fixed-buffer overflow.
- Update `ResourceBlobPackedReader` and `ResourceBlobChunkPackedReader` to use the vector output. Preserve the chunked format (1-byte block size, 200-byte chunks) and the 8K resource limit.
- Keep compressed payloads byte-identical for inputs that already worked; blobs of 1..5 bytes now produce valid output instead of corrupting memory.
- Harden the unit-test build and CI with `_GLIBCXX_ASSERTIONS` and ASan/UBSan (or Valgrind), so this class of error fails on every Linux toolchain rather than only on the newest Ubuntu.
- Document and validate the chunk reader's `200`/`255`/`1024` relationship against the new bound.

### [fix-string-temp-buffer-concat](openspec/changes/archive/2026-10-28-fix-string-temp-buffer-concat) Fix temporary string buffer concatenation and normalize startup to SCREEN 0 40 columns

#### Added
- Add probes for both split programs:
  - `PRINT/test5` for trailing `;`/`,` and tab/separator layout.
  - `STRING/test1` for literal and variable `STRING$` concatenation.

#### Changed
- Align `STRING$`, `SPACE$`, and `INKEY$` with the existing `BUF` to temporary-string copy pattern used by `LEFT$`, `RIGHT$`, `MID$`, `HEX$`, and `USING$`, so each result survives until concatenation.
- Normalise startup display explicitly to `SCREEN 0` with `WIDTH 40`, matching `LINL40`/`LINLEN` and producing one deterministic 40-column layout on MSX1, MSX2, MSX2+, and turbo R.
- Review and regenerate the regression probes/baselines of every program that uses `PRINT`, `?`, or `STRING$`. Existing baselines encode the current 32-column mismatch and must be treated as stale.
- Document the initial-screen normalization decision in `doc/adr/ADR-002 Initial text screen normalization.md` and update the start-code section of `doc/misc/architecture.md`.
- Apply the tier-1 hardening for bounded function arity to `STRING$` (reject five or more arguments), reusing `harden-cpp-parameter-validation-tier1` task 4.4.
- **BREAKING**: the default text layout changes from the inconsistent 32-column display to `SCREEN 0` 40-column text; programs that relied on the old shifted layout render differently.

#### Fixed
- Fix `GET_NEXT_TEMP_STRING_ADDRESS` to advance by one 256-byte slot per temporary string instead of one byte.

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.4.2.0...HEAD
[1.4.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.4.2.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
