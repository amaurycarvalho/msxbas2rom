# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### Added

- [add-16bit-segments-ascii16-4mb-ascii16x-8mb] 16-bit segment support enabling ASCII16 up to 4MB and ASCII16X up to 8MB ROMs
- [implement-double-via-float-float] Double precision operations via float emulation
- [refactor-resource-number-hl-protocol] Resource number passing standardized to HL register protocol
- [set-page-screen4] SET PAGE support for screen 4 compatibility

## [1.2.0.0] - 2026-06-25

### mtf-enhancements MTF window-copy support with extended parameter handling

#### Added
- Add MTF operation 2: window-copy (partial map copy with configurable source rectangle and screen destination — preserves tiles outside the window via LDIRMV+LDIRVM, only 2 VDP transfers regardless of window size)
- Add screen page parameter scaffolding to all map operations (0, 1, 2) — compiler accepts and stores the parameter, kernel uses placeholder (always page 0 at 0x1800)
- Add unit tests for all parameter combinations and an integration test (`tests/integration/MTF/mtf5.bas`)

#### Changed
- Migrate parameter passing from Z80 registers to RAM workarea block (PARM1) for all operations, enabling up to 9 parameters
- Unify map copy code path: operations 0 (relative) and 1 (absolute) delegate to the same `window_copy` kernel routine by setting full-screen window parameters, eliminating ~70 bytes of duplicate copy-loop code
- Full-width optimization: when screen_x=0 and width=32, skip LDIRMV (1 VDP transfer) — ensures ops 0/1 maintain current performance
- Extend compiler handler to accept 1–9 parameters (was 1–4)
- Use incremental builds — no forced `make clean`

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.2.0.0...HEAD
[1.2.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.0.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
