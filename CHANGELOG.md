# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### Added

- [add-16bit-segments-ascii16-4mb-ascii16x-8mb] 16-bit segment support enabling ASCII16 up to 4MB and ASCII16X up to 8MB ROMs
- [implement-double-via-float-float] Double precision operations via float emulation
- [mtf-enhancements] MTF window-copy and screen page support for tile-based games
- [set-page-screen4] SET PAGE support for screen 4 compatibility

## [1.1.0.0] - 2026-06-23

### Added

- Add `Konami4`, `ASCII16`, and `ASCII16X` compile modes with CLI flags `-4`/`--konami`, `-6`/`--ascii16`, and `-7`/`--ascii16x` for MegaROM mapper selection
- Add dispatch-table-guided kernel binary patching via `fixAscii16Mapper()` method with 7 patch points for ASCII16/ASCII16X
- Write ASCII16-X autodetection signature (`ASCII16X`) at ROM offset 0x0010
- Add dispatch table entries for new kernel patch-point labels
- Add unit tests for Konami4, ASCII16, and ASCII16X CLI parsing, ROM building, and signature byte verification
- Add integration test coverage for Konami4 and ASCII16 modes using existing MEGAROM test programs

### Changed

- Expand MegaROM compile code size limit from 1MB to 2MB by increasing `COMPILE_MAX_PAGES` from 64 to 128 pages
- Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` and extend to support both KonamiSCC and Konami4; extend `fixAscii16Mapper()` guard to also apply for `ASCII16X` mode
- Reuse existing kernel segment-switch patching for Konami4 (0x8000/0xA000 switch addresses via dispatch table)
- Reserve 8 bytes in kernel ROM header shifting INIT1 from 0x4010 to 0x4018 for ASCII16-X
- Update hardcoded LOADER symbol address from 0x4010 to 0x4018 in `symbol_export_context.cpp`
- Update output filename suffixes (`[Konami]`, `[ASCII16]`, `[ASCII16X]`) and CLI status/help messages
- Regenerate header artifacts (header.h, header.symbols.asm) for shifted kernel layout

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.1.0.0...HEAD
[1.1.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.1.0.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
