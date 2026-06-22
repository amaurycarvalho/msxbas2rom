## Context

The MSXBAS2ROM kernel occupies MSX ROM page 1 (0x4000-0x7FFF, 16KB). At logical address 0x4102 sits `wrapper_routines_map_table` — a 126-entry word-pointer dispatch table that maps compiler dispatch indices to actual kernel routine addresses. Along with 5 `nop` alignment bytes that precede it, the table consumes 257 bytes of kernel space.

The C++ compiler resolves dispatch calls at build time by reading the compiled kernel binary (`bin_header_bin[]`, auto-generated via `xxd -i` from `header.bin`). It looks up the 2-byte pointer from the table entry and emits a direct `CALL` to the resolved address. The table itself is never executed — it's metadata consumed by the C++ build pipeline.

`pasmo` (the Z80 assembler) uses `org` directives only to set the logical address counter for label resolution. Its flat binary output is contiguous — no gaps are filled between `org` sections. This makes it possible to have a 0x8000-byte binary where the first 0x4000 is virtual metadata and the second 0x4000 is the actual kernel, with natural alignment via `DEFS` padding.

## Goals / Non-Goals

**Goals:**
- Move the dispatch table out of the kernel ROM page into a compile-time-only virtual section
- Free 257 bytes (252 table + 5 nops) for kernel expansion
- Maintain identical ROM output (16KB kernel on page 1)
- Keep all existing kernel label addresses unchanged relative to the kernel start
- Simplify the address resolution math where possible

**Non-Goals:**
- Growing the actual ROM kernel beyond 16KB
- Changing the dispatch table entry format (stays `dw`)
- Modifying how the kernel initializes or runs at runtime
- Adding new entries to the dispatch table (just relocating it)

## Decisions

### Decision 1: Single `org 0000h` with DEFS padding, no second `org`

The table section uses `org 0000h` followed by `DEFS 0x4000 - $`. After the padding, the logical address counter is naturally 0x4000 — no redundant `org 4000h` is needed. The ROM header (`db "AB"`, `dw INIT1`, etc.) starts immediately after the DEFS.

**Rationale**: Minimal assembly changes. The DEFS both pads the binary AND advances the logical address. A second `org` would be redundant and could introduce subtle issues if the DEFS calculation were off.

**Alternative considered**: Explicit `org 4000h` after DEFS. Rejected as redundant — the DEFS already guarantees the address counter reaches 0x4000.

### Decision 2: Table at logical address 0x0000 in virtual section

The dispatch table occupies logical addresses 0x0000-0x00FB (126 × 2 = 252 bytes). `def_wrapper_routines_map_table` changes from `0x4102` to `0x0000`. The table entries still contain word pointers to kernel labels in the 0x4000-0x7FFF range — pasmo resolves these as forward references during its second pass.

**Rationale**: Starting at 0x0000 gives the simplest index math. The C++ side can use `bin_header_bin[DISP_index * 2]` directly without an offset.

**Alternative considered**: Keeping the table at 0x4102 in the virtual section. Rejected — it would require subtracting an offset in C++ and waste 0x102 bytes of the virtual section on unused space before the table.

### Decision 3: Address resolution handles two distinct ranges

`getKernelCallAddr()` currently works with a single range (0x4000-0x7FFF) and subtracts 0x4000 to convert ROM addresses into `bin_header_bin` indices. After the split, it must handle:

- **0x0000-0x3FFF**: Virtual table range. Read 2 bytes from `bin_header_bin[address]` to get the kernel target address.
- **0x4000-0x7FFF**: Kernel range. `bin_header_bin[address]` directly maps to the kernel byte (since the kernel starts at binary offset 0x4000, which equals its logical address).

The `- 0x4000` offset is eliminated entirely — `bin_header_bin` is 0x8000 bytes now and indexed by logical address.

**Rationale**: The new indexing is actually simpler. The dual-range check replaces the old table-range check (+ `- 0x4000` offset) with cleaner, more direct logic.

### Decision 4: MegaROM resolution simplifies to direct index

In `compiler.cpp`, the MR target resolution currently computes:
```cpp
bin_header_bin[def_wrapper_routines_map_table - 0x4000 + DISP_MR_CALL * 2]
```

With the table at logical 0x0000, this becomes:
```cpp
bin_header_bin[DISP_MR_CALL * 2]
```

**Rationale**: The `def_wrapper_routines_map_table - 0x4000` was the offset of the table within the old binary. Since the table now starts at byte 0, no offset is needed.

### Decision 5: 5 nops removed

The 5 `nop` instructions at lines 157-161 of `20_runtime.asm` existed solely to align the table start to 0x4102. Since the table is no longer in the kernel section, these nops are removed. Kernel routines in subsequent includes (21_logic_pack.asm onward) start 257 bytes earlier than before.

**Rationale**: Eliminates dead padding. The 257-byte shift of subsequent labels is transparent because all kernel addresses are resolved from the binary at compile time — no hardcoded kernel routine addresses exist in C++ (only the table base address, which we're changing).

## Risks / Trade-offs

- **[Risk] Pasmo DEFS behavior**: If pasmo's `DEFS` in flat binary mode doesn't emit zero-fill bytes, the second half won't start at offset 0x4000. → **Mitigation**: Verify with a test build before implementing C++ changes. Pasmo documentation and prior usage confirm DEFS emits bytes in binary output mode.

- **[Risk] Shifted kernel labels**: Removing 257 bytes from the kernel section shifts all subsequent labels (21_logic_pack.asm through 90_support.asm) by -257 bytes. If any C++ code hardcodes a kernel routine address beyond the dispatch table, it would break. → **Mitigation**: Audit confirms only `def_wrapper_routines_map_table = 0x4102` is hardcoded to a kernel address. All other kernel access goes through the dispatch table or `getKernelCallAddr()` which reads the binary at compile time.

- **[Risk] fixIfKonamiSCC scan**: This function scans `pages[0]` bytes 0xDB-0x3FFF for mapper instruction patterns. Since kernel code shifts, the bytes at a given offset change. → **Mitigation**: The scan is pattern-based (searches for `LD (0x7000),A` and `LD A,(0x7000)`), not offset-based. The 11 matching locations still exist in the shifted kernel code.

- **[Trade-off] Binary bloat**: `bin_header_bin[]` grows from 16KB to 32KB in the compiled C++ binary. This is a compile-time cost only — the ROM output stays at 16KB kernel. The ~16KB of zero-fill padding is negligible in a modern build environment.
