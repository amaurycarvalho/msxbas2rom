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

## [1.2.2.0] - 2026-08-12

### [2026-08-12-fix-make-lint-warnings](openspec/changes/archive/2026-08-12-fix-make-lint-warnings) Fix `make lint` gate failures and indeterminate `Logger::LogEntry` fields

#### Fixed
- `src/infrastructure/logging/logger.h` / `logger.cpp`: give `LogEntry` explicit default and `(LogLevel, const string&)` constructors so all six fields are deterministically initialized, eliminating the `-Wmissing-field-initializers` errors and the indeterminate-`dummy` bug

#### Changed
- `src/application/compiler/helpers/semantic/compiler_code_helper.h` / `.cpp`: change `addBasicChar(char)` to `addBasicChar(unsigned char)`, matching its actual semantics (raw byte emission into `addLdiHL(unsigned char)`) and removing the `-Woverflow` errors on `0xC0`/`0xC5` MSX-BASIC tokens

#### Removed
- `src/application/compiler/helpers/semantic/compiler_code_optimizer.cpp`: drop the always-true `address >= def_wrapper_routines_map_table` lower-bound comparison in `getKernelCallAddr()` (the table base is `0x0000`, and the outer guard already ensures `address < 0x4000`), removing the `-Wtype-limits` error

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.2.2.0...HEAD
[1.2.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.2.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
