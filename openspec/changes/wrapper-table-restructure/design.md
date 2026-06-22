## Context

The Z80 kernel at `src/infrastructure/kernel/asm/src/header/20_runtime.asm` contains a 126-entry jump table at the fixed address `0x4102` (`wrapper_routines_map_start`). Each entry is a 3-byte `jp addr` instruction, totaling 378 bytes. The C++ compiler references these entries through `#define` constants in `compiler_hooks.h` using a `+3` byte-offset chain. The compiler emits calls in two patterns:

1. **Direct `CALL`** (`cpu.addCall(def_xxx)`): Emits `CALL <table_offset>`, landing on a `jp` instruction which then jumps to the real routine. Used in ~100+ call sites across the compiler.
2. **Resolved `CALL`** (`optimizer.addKernelCall(def_xxx)`): Reads the 3-byte `jp` from the binary at the offset and emits `CALL <resolved_target>`. Used for integer comparisons, float operations, and shifts.

The previous `optimize-kernel-space` change freed ~4 bytes of filler space. The wrapper table restructure was deferred because updating 100+ C-side call sites was deemed too invasive for that change. This change focuses exclusively on that restructure.

## Goals / Non-Goals

**Goals:**
- Replace 126-entry `jp` table (378 bytes) with `dw` word-pointer table (252 bytes) + dispatcher (~9 bytes), net ~117 bytes saved
- Update all C++ code to emit the new dispatch sequence instead of direct `CALL`
- Remove the 5-byte NOP padding introduced by the previous optimization (reclaim 5 more bytes)
- Preserve 100% behavioral compatibility — every wrapper routine resolves to the same target address
- Keep incremental build working

**Non-Goals:**
- Changing the number of wrapper entries (126 entries preserved)
- Changing the order of wrapper entries
- Changing wrapper routine implementations
- Modifying `z80.cpp` opcode emission beyond adding a dispatch helper

## Decisions

### D1: Word-pointer dispatch table (not a hybrid approach)

**Decision:** Replace the `jp` table entirely with a `dw` table and a dispatcher routine at `wrapper_routines_map_start`.

**Alternatives considered:**
- *Keep `jp` table as-is*: Wastes 378 bytes with no filler space for new 1.0.1.0 routines.
- *Keep `jp` table and add entries at dispatch level*: Increases size, not decreases.
- *Use `jr` relative jumps*: Limited to ±128 bytes range; wrapper routines are scattered across the 16K ROM page.
- *Use a stub dispatch per entry*: Example: `ld hl, index / jp dispatcher` = 6 bytes per entry, worse than 3-byte `jp`.

**Rationale:** The `dw` table is the most compact representation. Each entry is 2 bytes vs 3 bytes for `jp`. A single dispatcher routine handles all lookups.

### D2: Entry index passed in HL

**Decision:** The C compiler emits `ld hl, <index*2>` followed by `call wrapper_routines_map_start`. The dispatcher adds HL to the table base address and jumps via `jp (hl)`.

**Alternatives considered:**
- *Pass index in A register*: Only 256 entries max (we have 126), but requires a lookup loop (slower).
- *Pass index in BC/DE*: Uses an extra register pair that may be live at call site.
- *Pre-multiply index by 2 at compile time*: Saves one ASM instruction at call site.

**Rationale:** Multiplying by 2 at compile time means the dispatcher doesn't need to do `add hl, hl`. The sequence `ld hl, nn / call dispatcher` is 6 bytes, same as the original `call nn`. No code size increase at call sites.

### D3: New C++ helper `addKernelDispatch(uint8_t index)`

**Decision:** Add a method to `CompilerCodeOptimizer` or `Z80OpcodeWriter` that emits the dispatch sequence: `ld hl, index*2; call wrapper_routines_map_start`.

The old direct-`CALL` pattern:
```cpp
cpu.addCall(def_cmd_clrscr);  // emits: CALL 0x4108
```
becomes:
```cpp
optimizer.addKernelDispatch(DISP_cmd_clrscr);  // emits: LD HL, 2; CALL 0x4102
```

**Alternatives considered:**
- *Emit `rst` vector*: Only 8 vectors available, not enough.
- *Use `jp` table directly in compiler output*: Would waste user ROM space.

**Rationale:** The helper centralizes the dispatch emission, making it easy to update all call sites with a simple find-and-replace refactoring.

### D4: compiler_hooks.h defines dispatch indexes instead of addresses

**Decision:** Replace the `+3` chain with sequential indexes starting from 0.

Before:
```c
#define def_wrapper_routines_map_start 0x4102
#define def_castParamFloatInt def_wrapper_routines_map_start
#define def_cmd_clrkey (def_castParamFloatInt + 3)
```

After:
```c
#define def_wrapper_routines_map_start 0x4102
#define DISP_castParamFloatInt 0
#define DISP_cmd_clrkey 1
```

**Alternatives considered:**
- *Keep old names with new values*: `#define def_cmd_clrkey 1` — could silently break code expecting address values.
- *Add `DISP_` prefix for new names*: Makes migration explicit and verifiable at compile time.

**Rationale:** The `DISP_` prefix makes the migration auditable. Old `def_*` names that remain must be updated to use the dispatch helper. The compiler will error on any missed `def_*` usage if we remove or rename them.

### D5: Preserve `wrapper_routines_map_start` at 0x4102

**Decision:** The dispatcher routine goes at address 0x4102, preserving backward compatibility with existing `compiler_hooks.h` address constants during the migration window. The 5 NOP bytes between `jp 0x8010` and the table are reclaimed by the dispatcher code.

**Rationale:** The dispatcher routine (9 bytes) fits exactly where the 5 NOPs + first 1.3 `jp` entries were. This avoids cascading address shifts.

## Risks / Trade-offs

- [All ~100+ C-side call sites must be updated atomically] → Cannot partially deploy; if any `def_*` address constant is passed to `cpu.addCall()` after the table restructure, it will execute the dispatcher with garbage HL. Mitigation: rename old constants to force compile errors on missed sites.
- [Dispatcher adds one extra layer of indirection (jump to dispatcher, then jump to target)] → Adds ~22 T-states per wrapper call (HL load + call + dispatcher vs direct call). Mitigation: the impact is negligible for an interpreted BASIC compiler; wrapper calls are not in hot paths.
- [5-byte NOP padding removal shifts addresses] → Already verified that `wrapper_routines_map_start` at 0x4102 is the only hardcoded address in the C code. All other references are relative to this base.
- [compiler_code_optimizer.getKernelCallAddr() must handle `dw` entries] → Currently checks for `0xC3` (jp) byte. Needs to be updated to read `dw` entries. Mitigation: the optimizer is only used for integer/float ops; all other calls use the dispatch helper.

## Open Questions

- **Q1:** Should the old `def_*` constants be renamed with a `DISP_` prefix, or should they keep their old names but with new integer values? The `DISP_` prefix approach is safer but more invasive.
- **Q2:** Should `getKernelCallAddr()` be updated to handle `dw` entries, or should all kernel calls go through the dispatch helper? Using the dispatch helper uniformly simplifies the approach.
