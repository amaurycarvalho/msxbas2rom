## Context

The project compiles MSX BASIC source code into Z80 machine code. The generated code calls into a Z80 runtime kernel (stored in `bin_header_bin`, loaded at ROM address 0x4000-0x7FFF). The kernel exposes an ABI through a jump table called `wrapper_routines_map_start` at address 0x4102 — a contiguous block of `jp <target>` instructions where each entry occupies 3 bytes.

Currently, the C++ compiler uses two distinct calling patterns:

1. **Pattern A (16 calls, 6%)**: `optimizer.addKernelCall(def_*)` — reads `bin_header_bin` at the wrapper table address, extracts the `jp` target, and emits a direct `CALL` to the resolved address. Correct: address-stable because it resolves through the binary.

2. **Pattern B+C+E (>190 calls, 94%)**: `cpu.addCall(address)` — emits a `CALL` to a hardcoded address. Fragile: if the kernel is recompiled and targets move, the generated ROM is broken.

The `addKernelCall` mechanism already works correctly — the gap is that the wrapper table only covers ~126 routines, while the compiler calls ~60 additional XBASIC routines at hardcoded addresses (0x6xxx-0x7Fxx) that are NOT in the wrapper table, plus uses magic numbers for BIOS calls (0x00xx) instead of named constants.

A separate change (`optimize-kernel-space`) plans to redesign the wrapper table from `jp` entries to word-pointer dispatch. Having all calls routed through `addKernelCall` now would make that future transition seamless — only `compiler_hooks.h` constants and `getKernelCallAddr` would need updates.

```
CURRENT STATE:
┌────────────────────────────────────┐
│ C++ Compiler                       │
│                                    │
│ addKernelCall(def_wrapper)  ──► reads bin_header_bin at wrapper addr
│   → 16 calls ✅                   → extracts jp target
│   (integer ops, shifts, neg)      → CALL <resolved_target>
│                                    │
│ cpu.addCall(def_wrapper)  ────► CALL 0x4102+offset
│   → 74 calls ⚠️                   → works today but brittle
│   (cmds, tiles, sprites, etc.)    → breaks if table restructured
│                                    │
│ cpu.addCall(def_XBASIC_*)  ───► CALL 0x7631 (hardcoded!)
│   → 117 calls ❌                  → breaks if kernel recompiled
│   (math, strings, graphics)       → no indirection at all
│                                    │
│ cpu.addCall(0x009C)  ────────► CALL 0x009C (magic number)
│   → 12 calls ❌                   → works but magic numbers
│   (CHSNS, CHGET, BEEP, ...)      → no named constants
└────────────────────────────────────┘

TARGET STATE:
┌────────────────────────────────────┐
│ C++ Compiler                       │
│                                    │
│ addKernelCall(def_*)  ───────► reads bin_header_bin at wrapper addr
│   → ALL kernel calls ✅           → extracts jp target
│   (~190 calls)                    → CALL <resolved_target>
│                                    │
│ cpu.addCall(def_BIOS)  ──────► CALL 0x0024
│   → BIOS calls only ✅            → named constants
│   (~8 calls)                      → stable MSX standard addresses
└────────────────────────────────────┘
```

## Goals / Non-Goals

**Goals:**
- Add ~60 new `jp` entries to the wrapper table for all XBASIC routines currently called with hardcoded addresses
- Extend the `#define` chain in `compiler_hooks.h` with corresponding offset constants
- Replace all `cpu.addCall(def_*)` with `optimizer.addKernelCall(def_*)` for kernel-internal routines
- Replace magic number BIOS calls with named `#define` constants
- Zero behavioral change in generated machine code — identical ROM output after the change

**Non-Goals:**
- Do NOT change the wrapper table dispatch mechanism (that's `optimize-kernel-space`)
- Do NOT change `getKernelCallAddr` logic (it already handles `jp` entries correctly)
- Do NOT add BIOS routines to the wrapper table (they are external, stable MSX standard addresses)
- Do NOT touch `addCallNZ`/`addCallZ` conditional call variants (these are for file I/O error paths with specific semantics)
- Do NOT modify the kernel implementation code — only the wrapper table entries and C++ call sites

## Decisions

### D1: Add XBASIC entries as new `jp` lines after the existing table

**Choice**: Append new `jp XBASIC_ROUTINE_NAME` entries at the end of `wrapper_routines_map_start` in `20_runtime.asm`, immediately after the last existing entry (`jp cmd_fprint`).

**Rationale**: The existing chain of `+3` offsets in `compiler_hooks.h` is positional — each `#define` depends on the previous. Appending at the end keeps backward compatibility for all existing entries. Inserting in the middle would shift all subsequent offsets and require updating every existing constant.

**Alternatives considered**:
- Insert alphabetically: Rejected — would break all existing offset calculations
- Separate XBASIC-only table: Rejected — adds complexity; single table is simpler to reason about

### D2: Group new entries by functional category

**Choice**: New entries are grouped in this order:
1. XBASIC math/comparison/cast (most frequently called)
2. XBASIC graphics routines
3. XBASIC trap routines
4. XBASIC swap routines
5. BIOS helper (WriteParamBCD)

**Rationale**: Logical grouping makes it easier to find entries during maintenance. The specific order within groups doesn't matter for correctness, only that the `+3` chain in the C++ header matches.

### D3: Keep the same define naming convention

**Choice**: New `#define` names follow the pattern `def_XBASIC_<NAME>`, matching the Z80 label name converted to uppercase with underscores. E.g., Z80 label `XBASIC_MULTIPLY_INTEGERS` → C define `def_XBASIC_MULTIPLY_INTEGERS`.

**Rationale**: Consistency with existing naming. Some of these defines already exist in `compiler_hooks.h` but with hardcoded hex values — they will be changed from direct hex to `+3` chain offsets.

**Key detail**: Currently `compiler_hooks.h` has TWO sections:
- Lines 120-262: Wrapper-based `#define`s (computed from `def_wrapper_routines_map_start` via `+3`)
- Lines 378-460: XBASIC direct hex `#define`s (hardcoded addresses like `0x7631`)

The second section's defines will be **replaced** by new entries in the first section's `+3` chain, then the second section's constants will be changed to align with the first section's computed values.

### D4: BIOS magic numbers get named constants

**Choice**: Add `#define def_CHSNS 0x009C`, `#define def_CHGET 0x009F`, etc. to a dedicated section in `compiler_hooks.h`, grouped under an MSX BIOS section.

**Rationale**: These are MSX standard BIOS entry points — stable addresses defined by the MSX specification. They don't need wrapper table indirection, but they deserve named constants for readability and searchability. The naming convention follows `def_<BIOS_ROUTINE_NAME>`.

## Risks / Trade-offs

- **[Risk] Wrapper table grows by ~180 bytes (60 entries × 3 bytes)** → Acceptable: the kernel has unused space (the `optimize-kernel-space` change exists to recover more). The future word-pointer redesign would reduce this overhead.

- **[Risk] Offset chain errors if entries are misordered** → Mitigation: The `+3` chain is mechanical; each constant depends only on the previous one. A single misordered entry would shift all subsequent defines. The review checklist includes verifying each `jp` target label exists in the kernel symbol file.

- **[Risk] Some XBASIC routines might be in a different address space (megaROM)** → Mitigation: The `getKernelCallAddr` function already handles the 0x4000-0x7FFF range correctly, reading from `bin_header_bin`. As long as the new `jp` entries are within this range (they will be, at 0x4102+new_offset), resolution works regardless of megaROM mode.

- **[Trade-off] Direct calls are slightly faster than going through wrapper dispatch** → Not applicable: `addKernelCall` currently emits direct calls to the resolved target (it reads the wrapper table at build time and skips the `jp` at runtime). This is actually faster than the current `cpu.addCall(def_wrapper)` which calls the `jp` entry at runtime.

## Migration Plan

1. Add new `jp` entries to `20_runtime.asm` → recompile kernel → update `header.symbols.asm` and `header.h` binary
2. Update `compiler_hooks.h` with new `+3` chain for XBASIC entries
3. Replace all `cpu.addCall(def_XBASIC_*)` with `optimizer.addKernelCall(def_XBASIC_*)` across all C++ files
4. Replace all `cpu.addCall(def_cmd_*)` / `cpu.addCall(def_set_*)` / etc. (wrapper-based defines) with `optimizer.addKernelCall(def_*)`
5. Replace magic numbers with named `def_` constants
6. Build and run tests to verify identical ROM output

No rollback needed — the change is purely build-time. If tests pass, the output ROM is identical.

## Open Questions

- Should `addCallNZ`/`addCallZ` conditional calls also use `addKernelCall`? These are for file I/O error handling (`MAXFILES`, `FCLOSE`) and currently use `cpu.addCallNZ(def_cmd_fmaxfiles)`. Since they bypass the wrapper, they would also be affected if the kernel is restructured. Answer: scope them in if they use wrapper table addresses; they are already in the Category B list.

- The `optimize-kernel-space` change plans to replace the `jp` table with word-pointer dispatch. Should this change coordinate with that one? Answer: This change is a prerequisite — it ensures all calls go through the wrapper table, making the future dispatch redesign a single-point change (only `compiler_hooks.h` and `getKernelCallAddr`).
