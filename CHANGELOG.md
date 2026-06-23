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
- [implement-double-via-float-float] Implement double precision operations via float emulation
- [wishlist] Collect and track feature requests and future enhancements

## [1.0.1.0] - 2026-06-23

### Added

- Kernel regression test infrastructure (`make test-kernel`) with ROM binary integrity validation
- Kernel ROM size build-time check (max 0x4000 bytes) with Makefile alert and doctest unit test
- Full dispatch table coverage for all XBASIC math, string, graphics, trap, cast, and I/O routines (~55 new `dw` entries, ~55 new `DISP_XBASIC_*` constants)

### Changed

- Compact player idle-state initialization with sequential HL writes in 33_player.asm
- Consolidate 4 consecutive `ld (ENDPRG+n),a` zero-stores with inc-chain in 20_runtime.asm
- Remove redundant `push de`/`pop de` guard in `cmd_preflight_disk`
- Remove duplicate `ld a, (STARTUP_CFG_FILEIO)` load in `run_user_basic_code_on_rom`
- Remove redundant `xor a` in `resource.address`
- Restructure wrapper routine jump table from 126-entry `jp` to word-pointer dispatch (saves ~125 bytes)
- Unify wrapper dispatch through single `addKernelCall` path with index auto-detection, remove `addKernelDispatch` and dispatcher ASM routine
- Replace ~117 `cpu.addCall(def_XBASIC_*)` with `optimizer.addKernelCall(DISP_XBASIC_*)` across all compiler files for indirect dispatch
- Replace ~12 magic number BIOS addresses (`0x009C`, `0x009F`, etc.) with named constants (`def_CHSNS`, `def_CHGET`, etc.)
- Double `header.bin` from 0x4000 to 0x8000 bytes: first half (0x0000-0x3FFF) becomes virtual dispatch table section, second half (0x4000-0x7FFF) is the kernel ROM image
- Move dispatch table to logical address 0x0000 with `DEFS` padding to 0x4000 boundary, removing the need for a second `org`
- Adapt `getKernelCallAddr()` for two-range addressing (0x0000-0x3FFF table lookups with direct `bin_header_bin` indexing, 0x4000-0x7FFF kernel addresses)
- Update ROM builder to write only second half (`bin_header_bin + 0x4000`)
- Update build size check from 16384 to 32768 byte limit
- Disable 58 unreachable code labels in `90_support.asm` with `DEFS` fillers (preserving binary offsets)
- Redirect 80 XBASIC_* EQUs from hardcoded hex addresses to native Z80 labels (`C`/`I`/`J` convention) in `00_constants_and_workarea.asm`
- Add `C7F44` mid-routine entry point label for `XBASIC_COPY_STRING` in `90_support.asm`
- Collapse `intCompareGT`/`intCompareGE` into delegated comparisons via operand swap (18 bytes saved)
- Replace `sla a` sequences with `add a,a` in `cmd_setfnt`, `cmd_play`, `cmd_page`, `cmd_mtf` (14 bytes saved)
- Replace `ld (MV_DPTR),hl`/`ld hl,(MV_DPTR)` with `push hl`/`pop hl` (4 bytes saved)
- Micro-optimizations in `FloatToBCD`, `intSHR`, `XBASIC_END`, `XBASIC_TAB`, `cmd_wrtfnt` (10 bytes saved)
- Remove dead commented-out code blocks

### Fixed

- Fix MegaROM mode restore in `verify.slots.test.ram` (save flags, write 0 to port 0x8E)
- Fix missing carry clear before `sbc hl,de` in `cmd_fmaxfiles.set_heap_size`
- Fix hardcoded 255-byte copy in `resource.get_data` (cap at actual resource size)
- Fix duplicate `WriteParamBCD` EQU conflict with existing label causing compilation error

### Removed

- Remove fully commented-out BDOS fallback block from 34_file_handling.asm
- Remove commented-out `VDP_wait` routine from 60_bios_helpers.asm
- Remove dead `inc de` in `XBASIC_INIT` function key loop
- Remove dead `ld a, 2` in `cmd_finput` state machine
- Remove 5 alignment `nop` instructions from `20_runtime.asm` (no longer needed after dispatch table move)

[Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v1.0.1.0...HEAD
[1.0.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.1.0

See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
