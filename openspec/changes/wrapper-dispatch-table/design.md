## Context

The parent change `wrapper-table-restructure` replaces the 126-entry `jp` jump table at `wrapper_routines_map_start` (0x4102) with a `dw` word-pointer dispatch table and dispatcher routine. Its design document identified two open questions:

- **Q1**: Should old `def_*` constants in `compiler_hooks.h` be renamed with a `DISP_` prefix, or keep old names with new integer values?
- **Q2**: Should `getKernelCallAddr()` be updated to handle `dw` entries, or should all kernel calls go through a separate dispatch helper? Using a unified approach simplifies the design.

This change resolves both questions and establishes a hard kernel ROM size ceiling of `0x4000` bytes (16 KB) — the address space available between `0x4000` (ROM start) and `0x8000` (next page boundary).

The current `getKernelCallAddr()` at `compiler_code_optimizer.cpp:231` checks the `header.bin` binary for the `0xC3` (`jp`) opcode byte to resolve targets. After the table restructure, this function must read `dw` word pointers instead.

## Goals / Non-Goals

**Goals:**
- Resolve Q1: All dispatch index constants use `DISP_` prefix, forcing compile-time errors on any missed migration site
- Resolve Q2: Update `getKernelCallAddr()` to read 2-byte word pointers from the `dw` table, and funnel all C-side wrapper calls through the existing `addKernelCall()` optimization path — no separate `addKernelDispatch()` emitter needed
- Establish a 0x4000-byte kernel ROM size limit with a build-time check
- Preserve `def_wrapper_routines_map_start` at 0x4102

**Non-Goals:**
- Adding `addKernelDispatch()` emission method (unnecessary — `addKernelCall` handles all dispatch)
- Reordering wrapper table entries
- Changing the number of entries (126 preserved)
- Modifying the dispatcher routine in ASM (already defined by parent change)

## Decisions

### D1: `DISP_` prefix for all dispatch constants

**Decision:** Rename all wrapper table `def_*` constants in `compiler_hooks.h` to use the `DISP_` prefix with sequential 0-based integer index values.

Before:
```c
#define def_castParamFloatInt def_wrapper_routines_map_start
#define def_cmd_clrkey (def_castParamFloatInt + 3)
```

After:
```c
#define DISP_castParamFloatInt 0
#define DISP_cmd_clrkey 1
```

**Alternatives considered:**
- *Keep `def_` names with new index values*: Silent behavioral change — old code expecting address values would compile but produce wrong calls.
- *Keep `def_` names as deprecated aliases*: Migration completeness is unverifiable; stale address-based calls could persist.

**Rationale:** The `DISP_` prefix makes the migration auditable and auditable. Any code still passing a `def_*` value to `cpu.addCall()` will fail at compile time once the old constants are removed. This is the safer, more explicit approach identified in the parent design's Q1 analysis.

### D2: Unified dispatch through `addKernelCall` / `getKernelCallAddr`

**Decision:** Instead of adding a new `addKernelDispatch(uint8_t index)` method that emits `LD HL, index*2; CALL 0x4102`, update `getKernelCallAddr()` to resolve through the `dw` table. All wrapper calls go through the existing `addKernelCall(DISP_xxx)` path, which internally calls `getKernelCallAddr()` and emits a direct `CALL`.

The updated `getKernelCallAddr()`:
```cpp
int CompilerCodeOptimizer::getKernelCallAddr(unsigned int address) {
    if (address >= 0x4000 && address < 0x8000) {
        int i = address - 0x4000;
        // Read 2-byte word pointer from dw table
        return bin_header_bin[i] | (bin_header_bin[i + 1] << 8);
    }
    return address;
}
```

This means call sites use:
```cpp
optimizer.addKernelCall(DISP_cmd_clrscr);
// getKernelCallAddr reads the dw table at offset DISP_cmd_clrscr*2
// Emits: CALL <resolved_target>
```

**Alternatives considered:**
- *Separate `addKernelDispatch()` + `addKernelCall()` paths*: Two code paths for wrapper calls, one emitting dispatch sequence and one resolving targets. Unnecessary duplication.
- *Only `addKernelDispatch()` (emit dispatch sequence on every call):* Wastes user ROM space ($LD HL, nn$ + $CALL dispatcher$ = 6 bytes vs $CALL target$ = 3 bytes). `addKernelCall` saves 3 bytes per call site.

**Rationale:** `addKernelCall` already optimizes by resolving the target at compile time and emitting a direct `CALL`. The only change needed is teaching `getKernelCallAddr()` to read `dw` entries instead of `jp` entries. This eliminates ~3 bytes per call site in user ROM compared to emitting the dispatch sequence every time.

### D3: Kernel ROM size ceiling at 0x4000 bytes

**Decision:** Add a build-time check (Makefile target or post-build step) that verifies `header.bin` size ≤ 16384 bytes. On breach, emit a visible alert.

The kernel occupies ROM page 1 (`0x4000`–`0x7FFF`). Exceeding 16 KB would spill into unallocated address space. This check ensures the combined dispatcher + table + all kernel routines fit within budget.

**Implementation:** A simple `stat`/`wc` check in the Makefile after `header.bin` is assembled. If size > 16384, print a warning with the actual size and excess bytes.

## Risks / Trade-offs

- [Old `def_*` constants removed unconditionally] → Any code still using `def_*` values will fail at compile time. Mitigation: this is intentional — it forces identification of all remaining non-migrated sites.
- [Direct `CALL` to resolved target skips the dispatcher] → If the dispatcher logic ever needs to change (e.g., add a hook), call sites using the optimized `addKernelCall` path won't see the change. Mitigation: This matches current behavior where `addKernelCall` already resolved `jp` targets directly. No regression.
- [Kernel size check is soft (alert only)] → The build does not abort on breach. Mitigation: An alert is sufficient for CI visibility; a hard failure could block unrelated work. The check can be hardened to an error later.

## Open Questions

None remaining — Q1 and Q2 are resolved by the decisions above.
