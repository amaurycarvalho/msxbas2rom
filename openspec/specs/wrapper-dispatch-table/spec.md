# Wrapper Dispatch Table

## Purpose

Defines the word-pointer dispatch table mechanism in the Z80 kernel that replaces the linear `jp` jump table at `wrapper_routines_map_table` (0x0000 in the virtual first half of header.bin). This enables index-based dispatch from C compiler code to kernel wrapper routines, saving ROM space for new kernel features.

## Requirements

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

### Requirement: Runtime central table contains selective dispatch stubs

The runtime central table SHALL contain a stub only for each used `DISP_*` entry and its required dependencies. Resident entries SHALL be `jp` stubs; overflow entries SHALL be dispatcher stubs carrying segment and address metadata.

#### Scenario: Resident stub is emitted
- **WHEN** a used dispatch entry resolves to a routine in the fixed kernel page
- **THEN** the table SHALL contain a 3-byte `jp` stub to that routine

#### Scenario: Paged stub is emitted
- **WHEN** a used dispatch entry resolves to an overflow routine
- **THEN** the table SHALL contain a stub that passes the target segment and address to the dispatcher

#### Scenario: Unused entry has no stub
- **WHEN** a `DISP_*` entry is not used by generated code or its dependency closure
- **THEN** the table SHALL NOT contain a stub for that entry

### Requirement: Compiler resolves kernel calls through build-time dispatch fixups

The compiler SHALL keep emitting kernel calls through `addKernelCall(DISP_*)` and conditional variants, but it SHALL resolve them through dispatch fixups after placement instead of reading a direct routine address during code generation.

#### Scenario: Call site stays a 3-byte placeholder
- **WHEN** the compiler emits a kernel call
- **THEN** the call instruction SHALL remain a 3-byte call placeholder
- **AND** the target SHALL be recorded as a dispatch fixup

#### Scenario: Conditional call uses the same fixup
- **WHEN** the compiler emits `addKernelCallNZ(DISP_*)` or another conditional variant
- **THEN** the conditional call target SHALL be resolved by the same dispatch fixup mechanism

#### Scenario: Fixup is applied after packing
- **WHEN** the packer has assigned all stub addresses
- **THEN** every dispatch fixup SHALL be patched to its resolved stub address

### Requirement: DISP indices remain stable across kernel relayouts

The numeric value of every `DISP_*` constant SHALL remain stable when kernel modules are moved, packed, or sent to overflow.

#### Scenario: Relayout does not renumber DISP constants
- **WHEN** the kernel module layout changes between two builds
- **THEN** the `DISP_*` values SHALL be identical
- **AND** only the resolved stub addresses SHALL change
