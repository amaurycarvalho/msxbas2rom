## ADDED Requirements

### Requirement: Wrapper routines are dispatched via word-pointer table
The Z80 kernel wrapper routine entry point at `wrapper_routines_map_start` (0x4102) SHALL use a word-pointer dispatch table instead of a linear `jp` jump table. A dispatcher routine SHALL accept an entry index in HL and jump to the corresponding routine address from the table.

#### Scenario: Dispatcher resolves entry by index
- **WHEN** HL contains entry index `N` (0-indexed) and the dispatcher is called
- **THEN** the dispatcher SHALL resolve the address from `wrapper_routines_map_table[N]` and jump to that routine

#### Scenario: Word-pointer table replaces jp entries
- **WHEN** the kernel is compiled after the restructure
- **THEN** the `wrapper_routines_map_start` region contains a dispatcher routine followed by 126 word-pointer (`dw`) entries, each 2 bytes

#### Scenario: Kernel ROM space is saved
- **WHEN** the kernel is compiled after the restructure
- **THEN** the total size of the wrapper dispatch region (dispatcher + 126 word pointers) SHALL be less than 378 bytes (the size of the original 126-entry `jp` table)

### Requirement: C compiler emits dispatch sequence instead of direct CALL
The C++ compiler SHALL emit a dispatch sequence that loads the entry index into HL and calls the dispatcher, instead of emitting a direct `CALL` to the old `jp` table entry address.

#### Scenario: Dispatch call sequence
- **WHEN** the compiler emits a call to a wrapper routine via `addKernelDispatch(index)`
- **THEN** it SHALL emit Z80 instructions: `ld hl, <index*2>` followed by `call wrapper_routines_map_start`

#### Scenario: Backward compatibility with optimizer
- **WHEN** `compiler_code_optimizer.getKernelCallAddr()` resolves a wrapper dispatch address
- **THEN** it SHALL correctly extract the target address from the `dw` table entry at the given offset

### Requirement: compiler_hooks.h defines dispatch indexes
The `compiler_hooks.h` header SHALL define 0-based dispatch indexes for each wrapper routine entry, replacing the previous `+3` byte-offset chain.

#### Scenario: Index constants are sequential
- **WHEN** the `def_*` constants are defined in `compiler_hooks.h`
- **THEN** each constant SHALL be a sequential 0-based index matching the order of entries in the `dw` table, with index `N` mapping to `wrapper_routines_map_table + (N * 2)`

#### Scenario: First entry is index 0
- **WHEN** the first wrapper routine `castParamFloatInt` is accessed
- **THEN** its dispatch index SHALL be `0`

### Requirement: Existing integration tests continue to pass
All existing `.bas` integration test files SHALL compile without errors and produce byte-identical ROM output (except filler bytes) compared to pre-restructure builds.

#### Scenario: Integration test compatibility
- **WHEN** the full integration test suite runs against the restructured kernel
- **THEN** all `.bas` test files SHALL compile without errors
