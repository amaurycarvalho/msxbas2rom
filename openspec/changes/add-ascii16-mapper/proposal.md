## Why

msxbas2rom currently supports ASCII8 and KonamiSCC/Konami4 MegaROM mappers but lacks ASCII16, a mapper used by many classic cartridge releases (LZ93A13, M60002, BS6101 chips, up to 2048KB). Supporting it expands the target hardware range and allows users to generate ROMs compatible with ASCII16-based cartridges, which use 16KB pages instead of 8KB.

## What Changes

- Add `ASCII16` compile mode to the `CompileMode` enum
- Add CLI flags `-6` and `--ascii16` for ASCII16 MegaROM selection
- Add dispatch-table-guided kernel binary patching via new `fixAscii16Mapper()` method: NOP out the second segment-switch write (`ld (0x7800),a`) and surrounding `inc a`/`dec a` in `MR_CHANGE_SGM`; NOP out invalid ASCII8-only writes in the boot bugfix and verify routines
- Adjust compiler segment math for 16KB pages (Option B: inline conditionals at each site): `segm_last` starts at 1 instead of 2, increments by 1 instead of 2, segment-to-page mapping uses 1:1 ratio instead of 2:1
- Adjust `resourceSegment` calculation in ROM builder from `pages.size() * 2` to `pages.size()` for ASCII16
- Add dispatch table entries for new kernel patch-point labels
- Update output filename suffix to `[ASCII16]`
- Update status messages and help text in CLI
- Add unit tests for ASCII16 CLI parsing, ROM building, and kernel patching
- Add integration test coverage using existing MEGAROM test programs compiled with `--ascii16`

## Capabilities

### New Capabilities

- `ascii16-mapper`: ASCII16 MegaROM mapper support. Provides compilation of MSX-BASIC programs into ROM images targeting ASCII16 hardware (16KB pages, 2-bank switching at 0x6000 and 0x7000, up to 2048KB ROM). The kernel SHALL be reused from the existing MegaROM kernel binary and patched via dispatch table to adapt the segment-switching model from 8KB pairs to single 16KB pages.

### Modified Capabilities

- `cli`: New `-6`/`--ascii16` flags for ASCII16 compile mode selection. The `--history` and `--help` output SHALL list the new flags. The `setInputFilename()` method SHALL append `[ASCII16]` suffix for ASCII16 mode.
- `builder`: ROM builder SHALL handle `ASCII16` compile mode with a new `fixAscii16Mapper()` method that patches the kernel binary via dispatch table. The `resourceSegment` calculation SHALL use `pages.size()` (1:1 mapping) instead of `pages.size() * 2` for ASCII16. ROM padding SHALL remain at 128KB aligned.
- `compiler`: The compiler SHALL support 16KB segment pages for ASCII16. Segment layout logic SHALL branch on `compileMode == ASCII16` to use single-segment increments (`segm_last += 1`, start at 1) and 1:1 segment-to-page mapping. Cross-segment fixup math SHALL use `*1+1` factors instead of `*2+2`. Segment-skip preamble SHALL emit a 4-byte sequence (`ld a,seg; jp MR_JUMP`) instead of the 8-byte ASCII8 sequence.

## Impact

- **Domain layer**: `src/domain/options/build_options.h` (enum value), `src/domain/options/build_options.cpp` (short name, suffix, megaROM flag trigger)
- **Application/Builder layer**: `src/application/builder/rom.h` and `rom.cpp` (new `fixAscii16Mapper()`, conditional `resourceSegment` math)
- **Application/Compiler layer**: `src/application/compiler/compiler.cpp` (~8 sites with mode-branching for segment math, skip preamble, fixup resolution)
- **CLI layer**: `src/cli/options/build_options_setup.cpp` (new flags), `src/cli/main.cpp` (status messages), `src/cli/appinfo.h` (help text)
- **Kernel assembly**: `src/infrastructure/kernel/asm/src/header/61_megarom.asm` (new patch-point labels), `src/infrastructure/kernel/asm/src/header/20_runtime.asm` (new dispatch table entries)
- **Dispatch table**: `src/application/compiler/helpers/hooks/compiler_hooks.h` (new `DISP_ASCII16_PATCH_*` constants)
- **Tests**: Unit tests for CLI parsing, ROM builder patching, compiler segment math; integration tests with MEGAROM test programs
- Release 1.1.0.0
