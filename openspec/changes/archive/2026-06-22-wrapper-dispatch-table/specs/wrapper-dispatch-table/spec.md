## MODIFIED Requirements

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

### Requirement: C compiler emits dispatch sequence via unified addKernelCall

The C++ compiler SHALL emit all wrapper routine calls through the unified `addKernelCall(DISP_xxx)` helper, which resolves the target address by calling `getKernelCallAddr()` to look up the `dw` dispatch table entry. There SHALL be no separate `addKernelDispatch()` emission method.

For conditional calls (e.g., `CALL NZ`), the compiler SHALL provide `addKernelCallNZ(uint8_t index)` and equivalent variants (`addKernelCallZ`, `addKernelCallC`, `addKernelCallNC`) that resolve the target through the `dw` table and emit the corresponding conditional `CALL` instruction.

For MegaROM inline byte-patching in `compiler.cpp` (MR_CALL, MR_JUMP, MR_GET_DATA), the target addresses SHALL be resolved directly from the `dw` table via `bin_header_bin` lookup at compile time — no dedicated `addMRKernelCall` helper is needed because the byte-patching code writes raw address values, not opcodes.

#### Scenario: Unified dispatch call

- **WHEN** the compiler emits a call to a wrapper routine via `addKernelCall(DISP_xxx)`
- **THEN** `getKernelCallAddr()` SHALL read the 2-byte word pointer from the `dw` table at the entry offset and the compiler SHALL emit a direct `CALL` to the resolved target address

#### Scenario: Conditional dispatch call

- **WHEN** the compiler emits a conditional call to a wrapper routine via `addKernelCallNZ(DISP_xxx)`
- **THEN** `getKernelCallAddr()` SHALL resolve the target from the `dw` table and the compiler SHALL emit `CALL NZ, <resolved_target>`

#### Scenario: getKernelCallAddr resolves through dw table

- **WHEN** `compiler_code_optimizer.getKernelCallAddr()` resolves a wrapper dispatch address
- **THEN** it SHALL read the 2-byte word pointer from the `dw` table entry at the given offset (instead of checking for the `0xC3` `jp` opcode), compute `offset = address - 0x4000`, and return `bin_header_bin[offset] | (bin_header_bin[offset + 1] << 8)`

#### Scenario: MegaROM addresses resolved inline

- **WHEN** `compiler.cpp` needs the target address of MR_CALL, MR_JUMP, or MR_GET_DATA for binary patching
- **THEN** the address SHALL be resolved directly from `bin_header_bin[def_wrapper_routines_map_table - 0x4000 + DISP_MR_xxx * 2]` without a dedicated helper method

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

## REMOVED Requirements

### Requirement: C compiler emits dispatch sequence instead of direct CALL

The previous requirement for a separate `addKernelDispatch()` emission method SHALL be removed. All wrapper calls go through the unified `addKernelCall()` path, which resolves targets at compile time and emits a direct `CALL` — matching the optimization strategy already used for integer/float operations.
