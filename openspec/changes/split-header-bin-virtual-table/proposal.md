## Why

The `wrapper_routines_map_table` (126 `dw` entries, 252 bytes) occupies precious kernel ROM space at 0x4102-0x41FD, plus 5 alignment nops to reach that boundary. As the kernel grows with new features, every byte in the 0x4000 page matters. Moving the dispatch table out of the kernel into a compile-time oracle section frees 257 bytes for future expansion and removes the table from the size pressure equation entirely.

## What Changes

- **header.bin doubles from 0x4000 to 0x8000 bytes**: First half (0x0000-0x3FFF) becomes a virtual compile-time-only section containing the dispatch table + zero-fill padding. Second half (0x4000-0x7FFF) is the actual kernel ROM image.
- **Dispatch table moves to logical address 0x0000**: `wrapper_routines_map_table` uses `org 0000h` and `DEFS 0x4000 - $` to pad to the second half boundary. No second `org` needed — the padding naturally lands at logical address 0x4000.
- **C++ address resolution adapts**: `getKernelCallAddr()` handles the 0x0000-0x3FFF range for table lookups (reading `bin_header_bin[address]` directly) and the 0x4000-0x7FFF range for kernel addresses (also using `bin_header_bin[address]` directly — no `- 0x4000` offset needed).
- **ROM builder writes only the second half**: `addKernel()` copies `bin_header_bin + 0x4000` instead of the full binary.
- **Build size check updates**: `LIMIT=16384` → `LIMIT=32768` for the full binary.
- **5 nops removed** from `20_runtime.asm` (they existed only to align the table to 0x4102).

## Capabilities

### New Capabilities

- `virtual-dispatch-table`: A compile-time-only dispatch table section occupying the first 0x4000 bytes of `header.bin`. The C++ compiler reads this section to resolve kernel routine addresses at build time, but it is never written to the output ROM. This separates the dispatch oracle from the kernel binary.

### Modified Capabilities

- `wrapper-dispatch-table`: The dispatch table base address changes from `0x4102` to `0x0000`. The address resolution logic in `getKernelCallAddr()` changes from `offset = address - 0x4000` + table-aware conditional to handling two distinct address ranges (0x0000-0x3FFF for table, 0x4000-0x7FFF for kernel). MegaROM inline resolutions in `compiler.cpp` simplify from `def_wrapper_routines_map_table - 0x4000 + N*2` to just `N*2`.
- `kernel-size-check`: The `header.bin` size limit changes from 16384 (0x4000) to 32768 (0x8000). The actual kernel portion (second half) must still fit within 0x4000 bytes.

## Impact

- **Assembly**: `src/infrastructure/kernel/asm/src/header/20_runtime.asm` — restructured with `org 0000h` + `DEFS` for table, ROM header and init code in second half, 5 nops removed
- **C++ compilation**: `compiler_hooks.h` (constant), `compiler_code_optimizer.cpp` (address resolution), `compiler.cpp` (MR targets)
- **ROM builder**: `rom.cpp` (memcpy source offset)
- **Build system**: `src/infrastructure/kernel/asm/Makefile` (size check limit)
- **Binary artifact**: `header.h` regnerated with `bin_header_bin_len = 32768`
- **No runtime impact**: ROM output remains 16KB kernel; dispatch table is not in ROM
