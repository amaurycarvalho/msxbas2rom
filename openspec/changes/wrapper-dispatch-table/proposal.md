## Why

The `wrapper-table-restructure` change left two architectural decisions unresolved: the naming convention for dispatch index constants (Q1) and how `getKernelCallAddr()` should interact with the new `dw` dispatch table (Q2). This change resolves both by adopting the `DISP_` prefix for all dispatch constants and unifying the C-side call mechanism through `addKernelCall()` — eliminating the separate `addKernelDispatch()` path proposed earlier. It also establishes a hard kernel ROM size limit of `0x4000` bytes.

## What Changes

- **Q1 resolution**: All dispatch index constants in `compiler_hooks.h` use the `DISP_` prefix (e.g., `DISP_cmd_clrkey`, `DISP_castParamFloatInt`) — **BREAKING** renaming from `def_*` address constants
- **Q2 resolution**: `getKernelCallAddr()` is updated to resolve targets through the `dw` dispatch table instead of checking for `0xC3` (`jp`) opcode bytes. All C-side wrapper calls go through a single helper (`addKernelCall()`) using the dispatch index and calculating the real call address at compile time instead of runtime. Also, it eliminates the need for a separate `addKernelDispatch()` emission method and a wrapper_routines_map_start routine (20_runtime.asm).
- **Kernel size constraint**: The final kernel ROM SHALL NOT exceed `0x4000` bytes; a build-time check alerts if this limit is breached and an unit test must be written to test it.
- `def_wrapper_routines_map_start` must be removed.

## Capabilities

### New Capabilities

- `kernel-size-check`: Build-time verification that the Z80 kernel ROM does not exceed 0x4000 bytes, with an alert on breach

### Modified Capabilities

- `wrapper-dispatch-table`: Updated to require `DISP_` prefix for all constants, `getKernelCallAddr()` resolving through `dw` entries, and unified dispatch via `addKernelCall()`

## Impact

- **Affected ASM files**: `20_runtime.asm` (kernel size assertion)
- **Affected C/C++ files**: `compiler_hooks.h` (DISP\_ constants), `compiler_code_optimizer.h/.cpp` (getKernelCallAddr update, addKernelCall unification)
- **Affected Makefile**: Build step for kernel size verification
- **Breaking change**: All `def_*` wrapper constants renamed to `DISP_*` in `compiler_hooks.h`, forcing compile-time detection of missed migration sites
- **Target release**: 1.0.1.0
