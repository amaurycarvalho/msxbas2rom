## Context

The project compiles MSX BASIC source code into Z80 machine code. The generated code calls into a Z80 runtime kernel stored in `bin_header_bin`. The kernel exposes an ABI through a *word-pointer dispatch table* called `wrapper_routines_map_table` at virtual address `0x0000` inside the header binary — a contiguous block of `dw <label>` entries where each entry occupies 2 bytes and encodes a Z80 address (little-endian).

The table was redesigned by `optimize-kernel-space` from the old `jp` jump-table (`0x4102`, 3 bytes/entry) to the current `dw` word-pointer dispatch (2 bytes/entry). The C++ side was also migrated: instead of a `+3` offset chain, `compiler_hooks.h` now uses `DISP_*` dispatch indexes (0-based), and `getKernelCallAddr` reads 2-byte word entries for addresses < 0x4000.

Currently, the C++ compiler uses two distinct calling patterns:

1. **Pattern A (~130 calls, correct)**: `optimizer.addKernelCall(DISP_*)` — passes a dispatch index (< DISP_ENTRIES), `getKernelCallAddr` reads the word from `bin_header_bin` at `0x0000 + index*2`, resolves the actual target, and emits a direct `CALL`. Address-stable: always resolves through the binary.

2. **Pattern B (~120 calls, fragile)**: `cpu.addCall(def_XBASIC_*)` — emits a `CALL` to a hardcoded address defined in the XBASIC section of `compiler_hooks.h` (e.g., `#define def_XBASIC_SIN 0x79EA`). Fragile: if the kernel is recompiled and targets move, the generated ROM is broken.

3. **Pattern C (~12 calls, magic numbers)**: `cpu.addCall(0x009C)` — raw BIOS address with no named constant. Poor readability.

The `addKernelCall` mechanism already works correctly with the `dw` dispatch. The gap is that the wrapper table only covers 126 routines (DISP 0–125), while the compiler calls ~90 additional XBASIC routines at hardcoded addresses that are NOT in the dispatch table.

```
CURRENT STATE:
┌────────────────────────────────────┐
│ C++ Compiler                       │
│                                    │
│ addKernelCall(DISP_*)  ──► reads bin_header_bin at 0x0000 + index*2
│   → ~130 calls ✅                  → extracts word-pointer target
│   (int ops, cmds, tiles, sprites,  → CALL <resolved_target>
│    usr, player, basic/io already   │
│    in dispatch table)              │
│                                    │
│ cpu.addCall(def_XBASIC_*) ───► CALL 0x7631 (hardcoded!)
│   → ~120 calls ❌                  → breaks if kernel recompiled
│   (math, strings, graphics,        → no indirection at all
│    traps, swaps, casts)            │
│                                    │
│ cpu.addCall(0x009C) ────────► CALL 0x009C (magic number)
│   → ~12 calls ❌                   → works but poor readability
│   (CHSNS, CHGET, BEEP, FNKSB,     → no named constants
│    ERAFNK, DSPFNK, GTSTCK,        │
│    GTTRIG, GTPDL, CHGCLR, RDPSG) │
└────────────────────────────────────┘

TARGET STATE:
┌────────────────────────────────────┐
│ C++ Compiler                       │
│                                    │
│ addKernelCall(DISP_*)  ──► reads bin_header_bin at 0x0000 + index*2
│   → ALL kernel calls ✅            → extracts word-pointer target
│   (~250 calls)                     → CALL <resolved_target>
│                                    │
│ cpu.addCall(def_BIOS)  ──────► CALL 0x0024
│   → BIOS calls only ✅             → named constants
│   (~8 calls)                       → stable MSX standard addresses
└────────────────────────────────────┘
```

## Goals / Non-Goals

**Goals:**
- Add ~80 new `dw` entries to the wrapper table for all XBASIC/BIOHELP/PRINT routines currently called with hardcoded addresses
- Add corresponding `DISP_*` dispatch index constants in `compiler_hooks.h`, incrementing from 126 to 206
- Update `DISP_ENTRIES` from 126 to 206
- Replace all `cpu.addCall(def_XBASIC_*)` with `optimizer.addKernelCall(DISP_XBASIC_*)` for kernel-internal routines
- Replace magic number BIOS calls with named `#define` constants
- Zero behavioral change in generated machine code — identical ROM output after the change

**Non-Goals:**
- Do NOT change the `dw` dispatch mechanism (that was done by `optimize-kernel-space`)
- Do NOT change `getKernelCallAddr` logic (it already handles `dw` entries correctly for addresses < 0x4000)
- Do NOT add BIOS routines to the wrapper table (they are external, stable MSX standard addresses)
- Do NOT touch `addKernelCallNZ` (already in use for `fmaxfiles` error path)
- Do NOT modify the kernel implementation code — only the wrapper table entries and C++ call sites

## Decisions

### D1: Add XBASIC entries as new `dw` lines after the existing table

**Choice**: Append new `dw XBASIC_ROUTINE_NAME` entries at the end of `wrapper_routines_map_table` in `20_runtime.asm`, immediately after the last existing entry (`dw cmd_fprint`).

**Rationale**: The existing `DISP_*` chain is positional — each `#define` is an index. Appending at the end (indices 126, 127, ...) keeps backward compatibility for all existing entries. Inserting in the middle would shift all subsequent `DISP_*` indexes and require updating every existing constant in the entire codebase.

### D2: Group new entries by functional category

**Choice**: New entries are grouped in this order:
1. XBASIC math/comparison/cast (most frequently called)
2. XBASIC graphics routines
3. XBASIC trap routines
4. XBASIC swap routines
5. BIOS helper (`WriteParamBCD`)
6. XBASIC print routines

**Rationale**: Logical grouping makes it easier to find entries during maintenance. The specific order within groups doesn't matter for correctness, only that the sequential `DISP_*` numbering in the C++ header matches.

### D3: Keep `DISP_` naming convention

**Choice**: New `#define` names follow the pattern `DISP_XBASIC_<NAME>`, matching the Z80 label name converted to uppercase with underscores. E.g., Z80 label `XBASIC_MULTIPLY_INTEGERS` → C define `DISP_XBASIC_MULTIPLY_INTEGERS`.

**Rationale**: Consistency with existing naming (e.g., `DISP_XBASIC_CLS`, `DISP_XBASIC_SCREEN` already exist). The old `def_XBASIC_*` hardcoded address defines will remain as fallback for any external consumers but will no longer be used by the compiler internally.

### D4: BIOS magic numbers get named constants

**Choice**: Add `#define def_*` constants for all BIOS entry points currently used as magic numbers: `def_CHSNS` (0x009C), `def_CHGET` (0x009F), `def_BEEP` (0x00C0), `def_FNKSB` (0x00C9), `def_ERAFNK` (0x00CC), `def_GTSTCK` (0x00D5), `def_GTTRIG` (0x00D8), `def_GTPDL` (0x00DE), `def_CHGCLR` (0x0062), and `def_RDPSG` (0x0096).

Existing defines for `def_DSPFNK` (0x00CF) and `def_INITXT` (0x00D5) already exist; add only the missing ones. Group under the existing `@name basic` section or a new `@name bios_entry_points` section. **Note**: `def_INITXT` and `def_GTSTCK` share the same address (0x00D5) — this is intentional in the MSX BIOS (INITXT=0x006C, GTSTCK=0x00D5 share the same entry address in this kernel's memory map). Add `def_GTSTCK` alongside with a comment.

**Rationale**: These are MSX standard BIOS entry points — stable addresses defined by the MSX specification. They don't need wrapper table indirection, but they deserve named constants for readability and searchability.

## Risks / Trade-offs

- **[Risk] Wrapper table grows by ~160 bytes (80 entries × 2 bytes)** → Acceptable: the `dw` table already saved significant space over the old `jp` table (252 bytes saved on 126 entries). The net space usage after adding 80 dw entries is still less than the old 126-entry jp table.
- **[Risk] Index errors if entries are misordered** → Mitigation: Each `DISP_*` constant is a sequential integer. A single misordered entry would shift all subsequent indexes. The review checklist includes verifying each table entry label exists in the kernel symbol file and matches the expected index order.
- **[Risk] Some XBASIC routines might be in a different address space (megaROM)** → Mitigation: The `getKernelCallAddr` function handles `dw` resolution for addresses in the virtual 0x0000 range, reading from `bin_header_bin`. As long as new entries are within this range (they will be), resolution works regardless of megaROM mode.
- **[Trade-off] The old `def_XBASIC_*` hardcoded defines remain in compiler_hooks.h** → They will stay as documentation of the actual kernel addresses. The compiler call sites will no longer reference them directly, but they remain useful for any external code that needs the real physical address. They can be removed in a future cleanup change.

## Migration Plan

1. Add new `dw` entries to `20_runtime.asm` → recompile kernel → update `bin_header_bin` and `header.h`
2. Add new `DISP_XBASIC_*` constants to `compiler_hooks.h` (indices 126–205) and update `DISP_ENTRIES` to 206
3. Add named BIOS constants to `compiler_hooks.h`
4. Replace all `cpu.addCall(def_XBASIC_*)` with `optimizer.addKernelCall(DISP_XBASIC_*)` across all C++ files
5. Replace all magic numbers (`cpu.addCall(0x00XX)`) with named constants (`cpu.addCall(def_*)`)
6. Build and run tests to verify identical ROM output

No rollback needed — the change is purely build-time. If tests pass, the output ROM is identical.

## Open Questions

- None — all aspects of the implementation are understood and the codebase has already been audited.
