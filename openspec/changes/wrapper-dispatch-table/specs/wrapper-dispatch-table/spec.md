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
The C++ compiler SHALL emit all wrapper routine calls through the single unified `addKernelCall(DISP_xxx)` helper, which resolves the target address by calling `getKernelCallAddr()` to look up the `dw` dispatch table entry. There SHALL be no separate `addKernelDispatch()` emission path.

#### Scenario: Unified dispatch call
- **WHEN** the compiler emits a call to a wrapper routine via `addKernelCall(DISP_xxx)`
- **THEN** `getKernelCallAddr()` SHALL read the 2-byte word pointer from the `dw` table at the entry offset and the compiler SHALL emit a direct `CALL` to the resolved target address — matching the optimization strategy already used by `addKernelCall()`

#### Scenario: getKernelCallAddr resolves through dw table
- **WHEN** `compiler_code_optimizer.getKernelCallAddr()` resolves a wrapper dispatch address
- **THEN** it SHALL read the 2-byte word pointer from the `dw` table entry at the given offset (instead of checking for the `0xC3` `jp` opcode), compute `offset = wrapAddress - 0x4000`, and return `bin_header_bin[offset] | (bin_header_bin[offset + 1] << 8)`

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

## ADDED Requirements

### Requirement: Kernel ROM size SHALL NOT exceed 0x4000 bytes
The build system SHALL verify at build time that the Z80 kernel ROM binary does not exceed `0x4000` bytes. If the kernel exceeds this limit, the build SHALL emit an alert identifying the actual size and the excess amount.

#### Scenario: Kernel within limit builds silently
- **WHEN** the kernel is compiled and the binary is 16384 bytes or fewer
- **THEN** no alert is emitted and the build proceeds normally

#### Scenario: Kernel exceeds limit triggers alert
- **WHEN** the kernel is compiled and the binary exceeds 16384 bytes
- **THEN** the build SHALL emit a visible alert message stating the actual size and how many bytes over the limit the kernel is
