## Why

msxbas2rom currently supports ASCII8 and KonamiSCC MegaROM mappers but lacks Konami4 (Konami without SCC), a mapper used by many classic Konami cartridge releases. Supporting it expands the target hardware range and allows users to generate ROMs compatible with Konami4-based cartridges (up to 2 MB, with 4 x 8KB banks and no SCC sound chip).

## What Changes

- Add `Konami4` compile mode to the `CompileMode` enum
- Add CLI flags `-4` and `--konami` for Konami4 MegaROM selection
- Reuse existing kernel segment-switch patching (Konami4 uses 0x8000/0xA000 upper-bank switch addresses, patched via dispatch table)
- Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` with updated method comment and error message text; extend condition to include `Konami4`
- Update output filename suffix to `[Konami]`
- Update status messages in `main.cpp` to distinguish Konami4 from KonamiSCC
- Add unit tests for Konami4 CLI parsing and ROM building
- Add integration test coverage using existing MEGAROM test programs compiled with `--konami`

## Capabilities

### New Capabilities

- `konami4-mapper`: Konami4 MegaROM without SCC mapper support. Provides compilation of MSX-BASIC programs into ROM images targeting Konami4 hardware (8KB segments, 4-bank switching, first bank locked to segment 0 on reset).

### Modified Capabilities

- `cli`: New `-4`/`--konami` flags for Konami4 compile mode selection. The `--history` and `--help` output SHALL list the new flags.
- `builder`: ROM builder SHALL handle `Konami4` compile mode by applying the same segment-switch address patch as KonamiSCC. The `fixIfKonamiSCC()` method SHALL be renamed to `fixKonamiMapper()` and accept both KonamiSCC and Konami4 modes.

## Impact

- **Domain layer**: `src/domain/options/build_options.h` (enum), `src/domain/options/build_options.cpp` (short/long names, output suffix)
- **Application layer**: `src/application/builder/rom.h` and `rom.cpp` (method rename, new mode check)
- **CLI layer**: `src/cli/options/build_options_setup.cpp` (new flags), `src/cli/main.cpp` (status messages), `src/cli/appinfo.h` (help text)
- **Tests**: `tests/unit/src/test_options.cpp` and `tests/unit/src/test_rom.cpp` (unit), `tests/integration/Makefile` (integration)
- **No changes** to kernel assembly, Z80 opcodes, or ROM layout logic
- Release 1.1.0.0
