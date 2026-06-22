# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [Unreleased]

### Added
- [add-konami4-mapper] Add Konami4 MegaROM mapper support with CLI flags and ROM building
- [mtf-enhancements] Add MTF window-copy and screen page support for tile-based games
- [set-page-screen4] Add SET PAGE support for screen 4 compatibility
- [extend-kernel-optimizations] Extend kernel space optimizations for additional byte savings
- [implement-double-via-float-float] Implement double precision operations via float emulation
- [kernel-call-indirection] Add kernel call indirection support for cross-page dispatch
- [disable-unused-90-support-code] Disable unused 90_support.asm code paths for kernel size reduction
- [wishlist] Collect and track feature requests and future enhancements

## [1.0.1.0] - 2026-06-22

### Added
- Kernel regression test infrastructure (`make test-kernel`) with ROM binary integrity validation

### Changed
- Compact player idle-state initialization with sequential HL writes in 33_player.asm
- Consolidate 4 consecutive `ld (ENDPRG+n),a` zero-stores with inc-chain in 20_runtime.asm
- Remove redundant `push de`/`pop de` guard in `cmd_preflight_disk`
- Remove duplicate `ld a, (STARTUP_CFG_FILEIO)` load in `run_user_basic_code_on_rom`
- Remove redundant `xor a` in `resource.address`
- Restructure wrapper routine jump table from 126-entry `jp` to word-pointer dispatch (saves ~120 bytes)

### Fixed
- Fix double-GICINI call in `cmd_mute` (replace `jp GICINI` with `ret`)
- Fix MegaROM mode restore in `verify.slots.test.ram` (save flags, write 0 to port 0x8E)
- Fix missing carry clear before `sbc hl,de` in `cmd_fmaxfiles.set_heap_size`
- Fix hardcoded 255-byte copy in `resource.get_data` (cap at actual resource size)

### Removed
- Remove fully commented-out BDOS fallback block from 34_file_handling.asm
- Remove commented-out `VDP_wait` routine from 60_bios_helpers.asm
- Remove dead `inc de` in `XBASIC_INIT` function key loop
- Remove dead `ld a, 2` in `cmd_finput` state machine

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.0.1.0...HEAD
[1.0.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.1.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
