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

### [add-rsctoram-offset-size](openspec/changes/add-rsctoram-offset-size) Add optional offset/size parameters to CMD RSCTORAM for partial resource copies

#### Added
- Add optional `offset` and `size` parameters to `CMD RSCTORAM`

#### Changed
- Clamp the effective copy length to `min(size, resource_size - offset)`
- Treat an omitted `size` as the full remaining resource
- Ignore `offset` and `size` when `pletter = 1` and decompress the whole resource as before
- Return `HL = dest` on success and `HL = 0` when the resource is missing or nothing is copied
- Pass command parameters through the `PARM1` work area
- Preserve existing 2- and 3-argument `CMD RSCTORAM` behavior

### [redefine-resource-function](openspec/changes/redefine-resource-function) Redefine RESOURCE(n) to load the resource into free RAM and return a usable pointer

#### Changed
- Redefine `RESOURCE(n)` to copy the resource into `HEAP()` via `CMD RSCTORAM` and return a directly usable RAM pointer
- Return `0` when the resource does not exist and truncate the copy to `FRE()` bytes when the resource is larger
- Keep `RESOURCESIZE(n)` and the existing `DISP_cmd_rsctoram` dispatch unchanged

## [1.3.1.0] - 2026-10-21

### [improve-mutation-score](openspec/changes/archive/2026-10-22-improve-mutation-score) Raise the mutation score with exact-output, sentinel-diverse unit tests

#### Added
- Add exact-output and sentinel-diverse coverage for MegaROM relocation/layout, resource readers, compiler semantic helpers, statement strategies, parser, symbols, lexer, and builder code
- Add normalized Z80 generated-code fingerprints and kernel dispatch-layout integrity checks as non-scoring regression coverage
- Add exhaustive relocation-switch coverage, including never-emitted conditional `call`/`jp` variants

#### Changed
- Require kernel-derived expected values to be computed at test runtime and compiler/Z80 constants to be asserted literally
- Raise the Mull timeout to 45000 ms for the measured post-Round-6 baseline
- Strengthen the mutation-testing spec with fresh-report, sentinel-diverse, C++-scope, and kernel-safe requirements

### [isolate-unit-tests-from-file-io](openspec/changes/archive/2026-10-22-isolate-unit-tests-from-file-io) Move CLI E2E scenarios to integration and isolate file-adapter tests in OS temporary directories

#### Added
- Add a cross-platform `TestTempDir` RAII helper for OS temporary directories
- Add an integration CLI characterization script preserving normal, quiet, debug, symbol, missing-input, and ASCII8-retry assertions

#### Changed
- Move CLI characterization out of the unit/mutation binary into `tests/integration`
- Remove the compile-based `FS`, `GRAPH`, and `MEGAROM` suites from `tests/unit/src/test_fs.cpp`
- Convert explicit file-adapter unit tests to OS temporary directories with automatic cleanup
- Strengthen `check_test_inventory.py` to reject filesystem I/O outside adapter tests and project-relative `tmp/` paths
- Reduce the plain-ROM overflow fixture from 900 to 200 lines

### [refactor-clean-architecture-testability](openspec/changes/archive/2026-10-21-refactor-clean-architecture-testability) Refactor compilation boundaries for in-memory testability and enforce Clean Architecture dependency rules

#### Added
- Establish an automated Clean Architecture boundary check with a version-controlled, pre-approved exception list
- Introduce application ports and infrastructure adapters for BASIC source input, INCLUDE resolution, ROM/resource/symbol output, and kernel-image access

#### Changed
- Make lexer/parser/compiler workflows accept in-memory source content with a logical source name
- Keep `BuildOptions` as a domain value object by removing its dependency on filesystem utilities
- Separate pure ROM image assembly from ROM-file persistence
- Extract the CLI compile/retry orchestration into an application use case with structured results and diagnostics
- Replace public, all-purpose parser/compiler contexts with narrower collaborators and encapsulated state
- Preserve CLI behavior, ROM bytes, diagnostics, resource formats, and cross-platform C++11 support

### [replace-service-locators-with-dependency-injection](openspec/changes/archive/2026-10-21-replace-service-locators-with-dependency-injection) Replace global service-locator providers with explicit constructor and composition-root dependency injection

#### Added
- Introduce a headless compilation test harness backed by in-memory source, kernel, resource, symbol, and ROM adapters

#### Changed
- Require ports through constructor or parameter-object injection instead of resolving defaults inside application classes
- Concentrate production adapter construction in the CLI composition root
- Make `CompilationRequest` carry a complete, required set of compilation services
- Pass `ResourceContentSource` through the resource reader factory instead of setting it after construction
- Migrate ROM, symbol, resource, lexer/parser/compiler, and characterization tests to in-memory assertions
- Preserve public CLI behavior, diagnostics, ROM bytes, resource bytes, and symbol formats

#### Removed
- Remove the five global provider pairs: source resolver, kernel image, resource content source, symbol output sink, and ROM writer
- Remove the global test adapter setup and static test source registry

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.3.1.0...HEAD
[1.3.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.3.1.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
