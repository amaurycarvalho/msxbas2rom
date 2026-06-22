## Why

The 126-entry `jp` jump table at `wrapper_routines_map_start` (address 0x4102) consumes 378 bytes of kernel ROM space using 3-byte `jp addr` instructions per entry. Replacing it with a 2-byte `dw` word-pointer dispatch table saves ~120 bytes, freeing space in the `BASIC_KUN_START_FILLER` region for new kernel routines planned for the 1.0.1.0 release.

## What Changes

- Replace the 126-entry `jp` jump table in `20_runtime.asm` with a `dw` word-pointer table and a small dispatcher routine (~5 bytes)
- Update `compiler_hooks.h` C-side `#define` constants to emit dispatch indexes instead of the `+3` byte-offset chain
- Update `compiler_code_optimizer.cpp` to resolve targets through the new dispatch mechanism
- Update all `cpu.addCall(def_xxx)` call sites (~100+ files) across the C++ compiler to use a new `addKernelDispatch()` helper that emits the dispatch sequence
- Remove the 5-byte padding introduced by the prior optimization to reclaim those bytes

## Capabilities

### New Capabilities
- `wrapper-dispatch-table`: A word-pointer dispatch table in the Z80 kernel that replaces the linear `jp` jump table, accessible from C code via an index-based dispatch call

### Modified Capabilities
<!-- No existing spec-level requirements are changing; the dispatch mechanism replaces the existing call pattern while preserving behavioral contracts -->

## Impact

- **Affected ASM files**: `20_runtime.asm` (wrapper table + dispatcher)
- **Affected C/C++ files**: `compiler_hooks.h` (dispatch index constants), `compiler_code_optimizer.h` (dispatcher helper), `compiler_code_optimizer.cpp` (dispatch resolution), `z80.h` / `z80.cpp` (dispatch emitter)
- **Affected call sites**: ~100+ C++ source files across `src/application/compiler/` that currently call `cpu.addCall(def_xxx)` — each must emit the new dispatch sequence instead
- **Breaking change**: `compiler_hooks.h` `#define` constants change from absolute addresses to dispatch indexes. All code using `def_*` constants for direct `CALL` must be updated.
