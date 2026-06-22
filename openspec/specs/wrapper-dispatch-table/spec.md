# Wrapper Dispatch Table

## Purpose

Defines the word-pointer dispatch table mechanism in the Z80 kernel that replaces the linear `jp` jump table at `wrapper_routines_map_table` (0x0000 in the virtual first half of header.bin). This enables index-based dispatch from C compiler code to kernel wrapper routines, saving ROM space for new kernel features.

## Requirements

### Requirement: Wrapper routines are dispatched via word-pointer table

The Z80 kernel wrapper routine entry point at `wrapper_routines_map_table` (0x0000 in the virtual first half of header.bin) SHALL use a word-pointer dispatch table instead of a linear `jp` jump table. A dispatcher routine SHALL accept an entry index in HL and jump to the corresponding routine address from the table.

#### Scenario: Dispatcher resolves entry by index

- **WHEN** HL contains entry index `N` (0-indexed) and the dispatcher is called
- **THEN** the dispatcher SHALL resolve the address from `wrapper_routines_map_table[N]` and jump to that routine

#### Scenario: Word-pointer table replaces jp entries

- **WHEN** the kernel is compiled after the restructure
- **THEN** the `wrapper_routines_map_table` region contains 126 word-pointer (`dw`) entries, each 2 bytes

### Requirement: compiler_hooks.h SHALL define dispatch table at logical address 0x0000

The `compiler_hooks.h` header SHALL define `def_wrapper_routines_map_table` as `0x0000` (previously `0x4102`), reflecting the table's new location in the virtual first half of `header.bin`.

#### Scenario: Table base address is 0x0000

- **WHEN** `def_wrapper_routines_map_table` is referenced in C++ code
- **THEN** its value SHALL be `0x0000`

### Requirement: getKernelCallAddr SHALL handle two distinct address ranges

The `getKernelCallAddr()` function SHALL handle two separate ranges:

- **0x0000-0x3FFF (virtual table range)**: When the address falls within the dispatch table bounds (`def_wrapper_routines_map_table` to `def_wrapper_routines_map_table + DISP_ENTRIES * 2`), the function SHALL read 2 bytes from `bin_header_bin[address]` (no `- 0x4000` offset) and return the resulting 16-bit kernel address.

- **0x4000-0x7FFF (kernel range)**: When the address is within the kernel page, the function SHALL use `bin_header_bin[address]` directly (since the kernel starts at binary offset 0x4000, which equals its logical address). If the byte at that position is `0xC3` (JP opcode), the function SHALL follow the jump target.

#### Scenario: Table entry resolved from 0x0000-0x3FFF range

- **WHEN** `getKernelCallAddr()` receives an address in the range `0x0000` to `0x0000 + DISP_ENTRIES * 2 - 1`
- **THEN** it SHALL compute `address` directly as the `bin_header_bin` index (no offset subtraction) and return `bin_header_bin[address] | (bin_header_bin[address + 1] << 8)`

#### Scenario: Kernel address resolved from 0x4000-0x7FFF range

- **WHEN** `getKernelCallAddr()` receives an address in the range `0x4000` to `0x7FFF`
- **THEN** it SHALL use `bin_header_bin[address]` directly (no `- 0x4000` offset) to read kernel bytes

#### Scenario: Non-table, non-kernel addresses pass through unchanged

- **WHEN** `getKernelCallAddr()` receives an address outside both the table range and the kernel range
- **THEN** it SHALL return the address unchanged

### Requirement: MegaROM addresses SHALL resolve via direct index

In `compiler.cpp`, the MR_CALL, MR_JUMP, and MR_GET_DATA target addresses SHALL be resolved directly from the dispatch table using `bin_header_bin[DISP_MR_xxx * 2]`, without the `def_wrapper_routines_map_table - 0x4000` offset prefix.

#### Scenario: MR target resolved by index

- **WHEN** `compiler.cpp` resolves an MR target address (e.g., `DISP_MR_CALL`)
- **THEN** the expression SHALL be `bin_header_bin[DISP_MR_CALL * 2] | (bin_header_bin[DISP_MR_CALL * 2 + 1] << 8)` with no additional offset

### Requirement: C compiler emits dispatch sequence via unified addKernelCall

The C++ compiler SHALL emit all wrapper routine calls through the unified `addKernelCall(DISP_xxx)` helper, which resolves the target address by calling `getKernelCallAddr()` to look up the `dw` dispatch table entry. There SHALL be no separate `addKernelDispatch()` emission method.

For conditional calls (e.g., `CALL NZ`), the compiler SHALL provide `addKernelCallNZ(uint8_t index)` and equivalent variants (`addKernelCallZ`, `addKernelCallC`, `addKernelCallNC`) that resolve the target through the `dw` table and emit the corresponding conditional `CALL` instruction.

#### Scenario: Unified dispatch call

- **WHEN** the compiler emits a call to a wrapper routine via `addKernelCall(DISP_xxx)`
- **THEN** `getKernelCallAddr()` SHALL read the 2-byte word pointer from the `dw` table at the entry offset and the compiler SHALL emit a direct `CALL` to the resolved target address

#### Scenario: Conditional dispatch call

- **WHEN** the compiler emits a conditional call to a wrapper routine via `addKernelCallNZ(DISP_xxx)`
- **THEN** `getKernelCallAddr()` SHALL resolve the target from the `dw` table and the compiler SHALL emit `CALL NZ, <resolved_target>`

### Requirement: compiler_hooks.h defines DISP_ prefix dispatch indexes

The `compiler_hooks.h` header SHALL define 0-based dispatch indexes for each wrapper routine entry using the `DISP_` prefix, replacing the previous `def_*` address chain constants.

#### Scenario: Constants use DISP_ prefix

- **WHEN** the dispatch constants are defined in `compiler_hooks.h`
- **THEN** every wrapper routine constant SHALL use the `DISP_` prefix (e.g., `#define DISP_castParamFloatInt 0`, `#define DISP_cmd_clrkey 1`)

#### Scenario: Index constants are sequential

- **WHEN** the `DISP_*` constants are defined in `compiler_hooks.h`
- **THEN** each constant SHALL be a sequential 0-based index matching the order of entries in the `dw` table, with index `N` mapping to `wrapper_routines_map_table + (N * 2)`

#### Scenario: First entry is index 0

- **WHEN** the first wrapper routine `castParamFloatInt` is accessed
- **THEN** its dispatch index SHALL be `0`

### Requirement: Existing integration tests continue to pass

All existing `.bas` integration test files SHALL compile without errors and produce byte-identical ROM output (except filler bytes) compared to pre-restructure builds.

#### Scenario: Integration test compatibility

- **WHEN** the full integration test suite runs against the restructured kernel
- **THEN** all `.bas` test files SHALL compile without errors

