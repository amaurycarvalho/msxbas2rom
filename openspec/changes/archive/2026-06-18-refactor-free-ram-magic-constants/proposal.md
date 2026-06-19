## Why

Lines 314-319 of `compiler.cpp` use three undocumented magic constants (5931, 10534, 0x538) to compute the `ram_memory_perc` metric, with an existing `//! @todo` comment calling for refactoring. These values were empirically calibrated during the file-handling feature's development without recorded derivation, making them fragile against kernel memory layout changes and opaque to maintainers.

## What Changes

- Replace inline magic constants in `compiler.cpp:314-319` with named constants defined in `compiler_hooks.h`
- Document each constant's purpose and relationship to the kernel's assembly-level memory layout
- Preserve all existing numeric values exactly (no behavioral change to the percentage calculation)

## Capabilities

### New Capabilities

None. This is a pure implementation refactoring — no new capabilities or user-facing behavior changes.

### Modified Capabilities

None. No spec-level requirements change.

## Impact

- **`src/application/compiler/helpers/hooks/compiler_hooks.h`**: 4–6 new `#define` constants with documentation
- **`src/application/compiler/compiler.cpp`**: Replace 3 inline magic constants with the new named constants (lines 314–319)
- **No behavioral changes**: the computed `ram_memory_perc` value stays identical for both disk and non-disk modes
