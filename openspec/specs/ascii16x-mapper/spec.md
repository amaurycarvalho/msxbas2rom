# ASCII16-X Mapper

## Purpose

Defines ASCII16-X MegaROM mapper support: 16KB pages with mirror address decoding and 12-bit bank registers.

## Requirements

### Requirement: ASCII16-X MegaROM mapper with 16KB pages and 12-bit bank registers

The system SHALL compile MSX-BASIC programs into ROM images targeting the ASCII16-X MegaROM mapper: 16KB pages with mirror address decoding and 12-bit bank number registers, up to 2048KB. The kernel SHALL reuse the ASCII16 dispatch-table patches plus an ASCII16X-specific boot-bugfix patch and the `ASCII16X` autodetection signature. The compiler SHALL remain unchanged for segment math. Resource segment mapping SHALL use the 8KB-pair convention (`resourceSegment = pages.size() * 2`).

#### Scenario: Upper bank bits are zero for supported ROM sizes
- **WHEN** an ASCII16-X ROM of ≤4MB (≤256 banks × 16KB) is built
- **THEN** the upper 4 bank bits (A8-A11) written with each bank number SHALL be 0

#### Scenario: Compile a program in ASCII16-X mode
- **WHEN** a valid MSX-BASIC program is compiled with `--ascii16x` flag
- **THEN** the output ROM file is generated with `[ASCII16X]` suffix
- **AND** the kernel is patched with 7 dispatch-table patches + the 8-byte signature at offset 0x0010
- **AND** the kernel's `MR_CHANGE_SGM` routine uses `push af; srl a; ld (0x7000),a; pop af; ret`
- **AND** all bank register writes target base address 0x7000 exclusively (lower bank at 0x4000-0x7FFF is fixed)
- **AND** the ROM is padded to a multiple of 128KB (8 pages)

#### Scenario: Verify ASCII16-X ROM kernel patching
- **WHEN** an ASCII16-X ROM is built
- **THEN** `MR_CHANGE_SGM` SHALL contain the byte sequence `F5 CB 3F 32 00 70 F1 C9` (push af / srl a / ld (0x7000),a / pop af / ret)
- **AND** 7 patch points SHALL be applied (same as ASCII16)
- **AND** the boot bugfix `ld (0x6800),a` SHALL be patched to `ld (0x7000),a`
- **AND** redundant bugfix writes SHALL be NOPped
- **AND** the 14-byte `AB` signature check SHALL be NOPped (`ld a,(0x8000); cp 0x41; jr nz; ld a,(0x8001); cp 0x42; jr nz` replaced with 0x00)
- **AND** the 4th and 5th OPENMSX writes SHALL target 0x77FF for mapper identification

#### Scenario: ASCII16-X signature present at ROM offset 0x0010
- **WHEN** an ASCII16-X ROM is built
- **THEN** the 8 bytes at ROM offset 0x0010-0x0017 SHALL be `41 53 43 49 49 31 36 58` ("ASCII16X")
- **AND** the ROM entry point pointer at offset 0x0002 SHALL point to 0x4018 (not 0x4010)
- **AND** INIT1 code SHALL begin at 0x4018

#### Scenario: ASCII16-X ROM size is valid
- **WHEN** an ASCII16-X ROM is generated
- **THEN** the ROM size SHALL be a multiple of 16KB pages
- **AND** the ROM size SHALL be between 128KB and 2048KB (after MegaROM filler padding)

#### Scenario: ASCII16-X uses base address 0x7000 only
- **WHEN** a bank register write occurs during ASCII16-X ROM execution
- **THEN** the write address SHALL be 0x7000 exclusively (upper bank)
- **AND** no mirror address SHALL be used for bank switching
- **AND** the lower bank (0x4000-0x7FFF) SHALL remain permanently fixed

#### Scenario: LOADER symbol address updated for all modes
- **WHEN** the kernel binary is regenerated via `make header`
- **THEN** `INIT1` SHALL be at 0x4018 (not 0x4010)
- **AND** the `"LOADER"` kernel symbol in `symbol_export_context.cpp` SHALL reference `"4018"`
- **AND** exported symbol files (.noi, .symbol, .omds, .cdb, .elf) SHALL contain `LOADER 4018H`
