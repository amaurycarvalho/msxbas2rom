# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### Added

- [mtf-enhancements] MTF window-copy and screen page support for tile-based games
- [set-page-screen4] SET PAGE support for screen 4 compatibility
- [implement-double-via-float-float] Double precision operations via float emulation

## [1.1.0.0] - 2026-06-23

### Added

- Add `Konami4` compile mode and CLI flags `-4`/`--konami` for Konami4 MegaROM selection
- Add unit tests for Konami4 CLI parsing and ROM building
- Add integration test coverage for Konami4 mode using existing MEGAROM test programs

### Changed

- Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` and extend to support both KonamiSCC and Konami4 modes
- Update output filename suffix to `[Konami]` and status messages to distinguish Konami4 from KonamiSCC

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.1.0.0...HEAD
[1.1.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.1.0.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
