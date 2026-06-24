## Why

msxbas2rom currently supports ASCII8 and KonamiSCC/Konami4 MegaROM mappers but lacks ASCII16, a mapper used by many classic cartridge releases (LZ93A13, M60002, BS6101 chips, up to 2048KB). Supporting it expands the target hardware range and allows users to generate ROMs compatible with ASCII16-based cartridges, which use 16KB pages instead of 8KB.

## What Changes

- Add `ASCII16` compile mode to the `CompileMode` enum
- Add CLI flags `-6` and `--ascii16` for ASCII16 MegaROM selection
- Add dispatch-table-guided kernel binary patching via new `fixAscii16Mapper()` method: rewrites `MR_CHANGE_SGM` with `push af; srl a; ld (0x7000),a; pop af; ret` (SeqReplace, 9 bytes), patches boot bugfix to single `ld (0x7000),1` write (1 ByteReplace + 3 NOPs), patches 4th/5th OPENMSX autodetection writes to 0x77FF (2× SeqReplace, 3 bytes each). Total: 7 patch points.
- The compiler is unchanged — the kernel converts 8KB segment numbers to 16KB page numbers via `srl a` at runtime
- `resourceSegment` stays at `pages.size() * 2` (8KB-pair convention unchanged across all MegaROM modes)
- Add dispatch table entries for new kernel patch-point labels (`ascii16_patch_bugfix_inc1`, `ascii16_patch_bugfix_nopseq`)
- Update output filename suffix to `[ASCII16]`
- Update status messages and help text in CLI
- Add unit tests for ASCII16 CLI parsing and ROM building
- Add integration test coverage using existing MEGAROM test programs compiled with `--ascii16`

## Capabilities

### New Capabilities

- `ascii16-mapper`: ASCII16 MegaROM mapper support. Provides compilation of MSX-BASIC programs into ROM images targeting ASCII16 hardware (16KB pages, upper bank switched via 0x7000, lower bank fixed to segment 0, up to 2048KB ROM). The kernel SHALL be reused from the existing MegaROM kernel binary and patched via dispatch table to insert `srl a` in `MR_CHANGE_SGM`, converting 8KB segment numbers to 16KB page numbers at runtime. The compiler SHALL remain unchanged.

### Modified Capabilities

- `cli`: New `-6`/`--ascii16` flags for ASCII16 compile mode selection. The `--history` and `--help` output SHALL list the new flags. The `setInputFilename()` method SHALL append `[ASCII16]` suffix for ASCII16 mode.
- `builder`: ROM builder SHALL handle `ASCII16` compile mode with a new `fixAscii16Mapper()` method that patches the kernel binary via dispatch table (7 patch points: 1 SeqReplace + 3 NOP + 1 ByteReplace + 2 OMSX SeqReplace). `resourceSegment` stays at `pages.size() * 2` for all MegaROM modes including ASCII16. ROM padding SHALL remain at 128KB aligned.

## Impact

- **Domain layer**: `src/domain/options/build_options.h` (enum value), `src/domain/options/build_options.cpp` (short name, suffix, megaROM flag trigger)
- **Application/Builder layer**: `src/application/builder/rom.h` and `rom.cpp` (new `fixAscii16Mapper()` method)
- **CLI layer**: `src/cli/options/build_options_setup.cpp` (new flags), `src/cli/main.cpp` (status messages), `src/cli/appinfo.h` (help text)
- **Kernel assembly**: `src/infrastructure/kernel/asm/src/header/20_runtime.asm` (new labels `ascii16_patch_bugfix_inc1`, `ascii16_patch_bugfix_nopseq` and dispatch table entries)
- **Dispatch table**: `src/application/compiler/helpers/hooks/compiler_hooks.h` (new `DISP_ASCII16_PATCH_BUGFIX_INC1`, `DISP_ASCII16_PATCH_BUGFIX_NOPSEQ` constants; `DISP_ENTRIES` 221→223)
- **Tests**: Unit tests for CLI parsing and ROM building; integration tests with MEGAROM test programs
- Release 1.1.0.0

> **Note**: This is a partial implementation of the ASCII16 format. The mapper hardware specification supports 8-bit bank registers at addresses 0x6000 and 0x7000 with mirror ranges (0x6001-0x6FFF and 0x7001-0x77FF) and up to 256 banks (2048KB). This implementation uses only the base addresses 0x6000 and 0x7000, never mirror addresses. All bank writes target the base address 0x7000 exclusively for segment switching. The compiler emits 8KB segment pairs; the kernel converts to 16KB pages via `srl a`.
