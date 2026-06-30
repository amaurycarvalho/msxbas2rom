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

## [1.2.1.0] - 2026-06-29

### [2026-06-29-fix-windows-startup-unicode-argv](openspec/changes/archive/2026-06-29-fix-windows-startup-unicode-argv) Fix Windows startup Unicode argv to ensure CLI runs reliably

#### Fixed
- Make Windows startup argument handling fail-safe before CLI parsing begins
- Avoid dangerous allocation from `WideCharToMultiByte()` failure paths, especially `len - 1` when `len == 0`
- Ensure informational invocations such as no arguments, `-h`, `--help`, `-v`, and `--version` print normally on Windows

#### Changed
- Preserve UTF-8 command-line support for Windows filenames when Unicode conversion succeeds

#### Added
- Add focused tests or build/run validation for Windows startup behavior where the project tooling allows it

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.2.1.0...HEAD
[1.2.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.1.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
