## Why

msxbas2rom now supports ASCII16 (implemented in release 1.1.0.0) but lacks ASCII16-X, the extended variant that adds mirror address decoding and 12-bit bank number support via address lines A8-A11. ASCII16-X is defined by the openMSX/grauw project and includes an autodetection signature at ROM offset 0x0010. Adding ASCII16-X with minimal code changes is feasible because for ROMs ≤4MB (the typical msxbas2rom output range), the upper 4 bank bits are always 0, making the kernel behavior identical to ASCII16's base-address-only writes (0x6000/0x7000).

## What Changes

- Add `ASCII16X` compile mode to the `CompileMode` enum
- Add CLI flags `-7` and `--ascii16x` for ASCII16-X MegaROM selection
- Reserve 8 bytes in the kernel ROM header (via `ds 8` between `db 'MSXB2R'` and `INIT1:`) shifting the entry point from 0x4010 to 0x4018; the `KERNEL_END_FILLER` absorbs the size delta keeping `BIN_HEADER_BIN_LEN` at 32768
- Write the ASCII16-X autodetection signature `ASCII16X` at ROM offset 0x0010 in `fixAscii16Mapper()` for `ASCII16X` mode only
- Update the hardcoded `"LOADER"` kernel symbol address from `"4010"` to `"4018"` in `symbol_export_context.cpp`
- Extend `fixAscii16Mapper()` guard to also apply kernel patching for `ASCII16X` mode (7 identical patch points: 1 SeqReplace + 3 NOP + 1 ByteReplace + 2 OMSX SeqReplace)
- Update output filename suffix to `[ASCII16X]`
- Update status messages and help text in CLI
- Regenerate `header.h` and `header.symbols.asm` via `make header`
- Add unit tests for ASCII16X CLI parsing, ROM building, and signature byte verification
- The compiler's segment math requires zero changes — kernel-side `srl a` conversion handles all MegaROM modes uniformly (only a code size overflow guard was added to `ICpuOpcodeWriter::addCodeByte()`)

## Capabilities

### New Capabilities

- `ascii16x-mapper`: ASCII16-X MegaROM mapper support. Provides compilation of MSX-BASIC programs into ROM images targeting ASCII16-X hardware. The kernel is based on ASCII16 (same dispatch-table-guided patches in `fixAscii16Mapper()`) since ROMs ≤4MB use only base addresses (0x6000/0x7000) where upper bank bits A8-A11 are 0. The ASCII16-X autodetection signature `ASCII16X` is written at ROM offset 0x0010 (MSX 0x4018–0x401F, with INIT1 shifted to 0x4018). The `KERNEL_END_FILLER` absorbs the 8-byte displacement, keeping the binary size unchanged.

### Modified Capabilities

- `cli`: New `-7`/`--ascii16x` flags for ASCII16-X compile mode selection. The `--help` output SHALL list the new flags. The `setInputFilename()` method SHALL append `[ASCII16X]` suffix for ASCII16-X mode.
- `builder`: ROM builder SHALL extend the `fixAscii16Mapper()` guard to also run for `ASCII16X` mode (the same 7 kernel patch points apply). For `ASCII16X` mode only, the builder SHALL write the 8-byte signature `ASCII16X` at ROM offset 0x0010 in page[0].

## Impact

- **Domain layer**: `src/domain/options/build_options.h` (enum value), `src/domain/options/build_options.cpp` (short name, long name, suffix, megaROM flag trigger)
- **Application/Builder layer**: `src/application/builder/rom.cpp` (extend `fixAscii16Mapper()` guard `ASCII16` → `ASCII16 || ASCII16X`; write `"ASCII16X"` at offset 0x0010 for ASCII16X mode)
- **Application/Symbols layer**: `src/application/symbols/strategies/context/symbol_export_context.cpp` (hardcoded `"LOADER"` address `"4010"` → `"4018"`)
- **CLI layer**: `src/cli/options/build_options_setup.cpp` (new flags), `src/cli/main.cpp` (status/summary messages), `src/cli/appinfo.h` (help text)
- **Kernel assembly**: `src/infrastructure/kernel/asm/src/header/20_runtime.asm` (add `ds 8` between `db 'MSXB2R'` and `INIT1:`); `src/infrastructure/kernel/header.h` and `header.symbols.asm` auto-regenerated via `make header`
- **Dispatch table**: `src/application/compiler/helpers/hooks/compiler_hooks.h` (new `DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK` constant; `DISP_ENTRIES` 223→224)
- **Compiler**: No changes (zero — kernel handles 8KB→16KB conversion)
- **Tests**: Unit tests for CLI parsing, ROM building, and signature byte verification
- Release 1.1.0.0

> **Note**: This is a partial implementation of the ASCII16-X format. The mapper hardware specification supports 12-bit bank numbers (D0-D7 + A8-A11) with 4-bit upper bank encoding via the write address, mirror page decoding, and ROM sizes up to 8MB (512 banks on XL cartridge). This implementation targets ROMs ≤4MB (≤256 banks) where the upper 4 bank bits (A8-A11) are always 0. All bank writes target base addresses 0x6000 and 0x7000 exclusively — no address-line encoding is used. Mirror addresses (0x2000-0x2FFF, 0xA000-0xAFFF, etc.) are never used for bank switching. The 12-bit bank number, full mirror decoding, and ROM sizes >4MB are not supported.
