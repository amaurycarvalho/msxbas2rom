# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### Added

- [add-16bit-segments-ascii16-4mb-ascii16x-8mb] 16-bit segment support enabling ASCII16 up to 4MB and ASCII16X up to 8MB ROMs
- [implement-double-via-float-float] Double precision operations via float emulation
- [set-page-screen4] SET PAGE support for screen 4 compatibility

## [1.2.0.0] - 2026-06-24

### mtf-enhancements MTF window-copy support with extended parameter handling

#### Added
- Add MTF operation 2: window-copy (partial map copy with configurable source rectangle and screen destination)
- Add screen page parameter scaffolding to all map operations (0, 1, 2)
- Add unit tests for all parameter combinations and an integration test

#### Changed
- Migrate parameter passing from Z80 registers to RAM workarea block (DAC) for all operations, enabling up to 9 parameters
- Extend compiler handler to accept 1–9 parameters (was 1–4)
- Use incremental builds — no forced `make clean`

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.2.0.0...HEAD
[1.2.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.0.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
