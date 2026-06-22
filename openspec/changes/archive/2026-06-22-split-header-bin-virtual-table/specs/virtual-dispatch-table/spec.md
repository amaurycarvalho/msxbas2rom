# Virtual Dispatch Table

## Purpose

Defines the virtual compile-time-only dispatch table section occupying the first 0x4000 bytes of `header.bin`. The C++ compiler reads this section to resolve kernel routine addresses at build time via `bin_header_bin[]`, but it is never written to the output ROM. This separates the dispatch oracle from the kernel binary, freeing kernel ROM space.

## ADDED Requirements

### Requirement: header.bin SHALL be 0x8000 bytes with two logical halves

The assembled `header.bin` binary SHALL be 32768 bytes (0x8000). The first 16384 bytes (0x0000-0x3FFF) SHALL contain the dispatch table and zero-fill padding only. The second 16384 bytes (0x4000-0x7FFF) SHALL contain the actual Z80 kernel ROM image.

#### Scenario: Binary size is 32768 bytes

- **WHEN** the kernel is assembled via pasmo
- **THEN** the resulting `header.bin` file SHALL be exactly 32768 bytes

#### Scenario: First half contains only table and padding

- **WHEN** examining the first 16384 bytes of `header.bin`
- **THEN** bytes 0 through 251 SHALL contain the 126-entry word-pointer dispatch table (252 bytes)
- **AND** bytes 252 through 16383 SHALL be zero-fill padding (DEFS)

#### Scenario: Second half is byte-identical to the kernel ROM image

- **WHEN** the second half (bytes 16384 through 32767) is copied to ROM page 1
- **THEN** the resulting ROM page SHALL be a valid, self-contained Z80 kernel

### Requirement: Dispatch table SHALL occupy logical address 0x0000

The `wrapper_routines_map_table` SHALL be assembled at logical address `0x0000` using `org 0000h`. The table entries SHALL use `dw` directives pointing to kernel labels in the 0x4000-0x7FFF range. Pasmo SHALL resolve these as forward references during its second assembly pass.

#### Scenario: First entry at logical address 0x0000

- **WHEN** the kernel is assembled
- **THEN** the first `dw` entry (`castParamFloatInt`) SHALL be at logical address 0x0000

#### Scenario: Entries point to kernel labels

- **WHEN** a table entry is resolved at compile time
- **THEN** the 2-byte value SHALL be the logical address of the corresponding kernel label in the 0x4000-0x7FFF range

### Requirement: DEFS padding SHALL bridge the first half to logical address 0x4000

After the 126 `dw` entries, `DEFS 0x4000 - $` SHALL emit zero-fill bytes until the logical address counter reaches 0x4000. The kernel ROM header (`db "AB"`, etc.) SHALL begin immediately after the DEFS at logical address 0x4000, without requiring a second `org` directive.

#### Scenario: Logical address reaches 0x4000 after DEFS

- **WHEN** the assembler processes the DEFS directive
- **THEN** the logical address counter after DEFS SHALL be exactly 0x4000

#### Scenario: ROM header starts at logical 0x4000

- **WHEN** examining the assembled kernel
- **THEN** the ROM header bytes (`db "AB"`, `dw INIT1`, etc.) SHALL be at logical address 0x4000 and binary offset 0x4000

### Requirement: C++ compiler SHALL use first half for table lookups only

The C++ compiler SHALL read dispatch table entries from `bin_header_bin[entry_offset]` where `entry_offset = dispatch_index * 2` (since the table starts at binary offset 0). The compiler SHALL NOT write any bytes from the first half of `bin_header_bin` to the output ROM.

#### Scenario: Table entry read at index offset

- **WHEN** the compiler resolves dispatch index `N` via `bin_header_bin[N * 2] | (bin_header_bin[N * 2 + 1] << 8)`
- **THEN** the result SHALL be the kernel address of the corresponding routine

#### Scenario: First half excluded from ROM output

- **WHEN** the ROM builder writes the kernel page
- **THEN** only `bin_header_bin + 0x4000` through `bin_header_bin + 0x7FFF` SHALL be copied to ROM page 1

### Requirement: 5 alignment nops SHALL be removed

The 5 `nop` instructions at the end of the initialization code in `20_runtime.asm` SHALL be removed. They existed solely to align `wrapper_routines_map_table` to address 0x4102, and the table is now at logical address 0x0000 in the virtual section.

#### Scenario: Kernel code flows directly into next module

- **WHEN** the initialization code executes `jp 0x8010`
- **THEN** the next module (21_logic_pack.asm routines) SHALL begin immediately at the address following the `jp` instruction, with no intervening nops
