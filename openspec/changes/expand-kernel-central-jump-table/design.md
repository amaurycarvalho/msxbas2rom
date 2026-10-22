# Design

## Context

See `proposal.md` for motivation. The current kernel image occupies 16382 of 16384 bytes in `0x4000-0x7FFF`. The `wrapper_routines_map_table` is a build-time oracle at logical `0x0000`; generated code receives direct absolute addresses. `MR_CALL` already implements a save/switch/call/restore protocol for MegaROM data access. The Arkos player is a fixed `org #6481` binary that executes from the fixed page while the song resource occupies the `0x8000` window.

Constraints that shape the design:

- The BASIC compiler must remain self-contained; no external assembler, linker, or module file at BASIC compile time.
- Pasmo generates fixed-position code and cannot emit relocatable objects.
- The player must not be paged or relocated.
- Plain ROM has only two code pages: `0x4000` (kernel) and `0x8000` (start code + user code); the resource page at `0x0000` is not a code page.
- `DISP_*` values are a stable ABI used by the compiler, tests, and symbol exporters.

## Goals / Non-Goals

**Goals:**

- Free the fixed kernel page from non-essential code without losing functionality.
- Fill kernel free space first; send only overflow to the user page or MegaROM segments.
- Keep the player and interrupt-critical code page-safe.
- Make BASIC compilation self-contained and deterministic.
- Preserve the existing `DISP_*` ABI and conditional-call behavior.
- Provide actionable diagnostics for placement failures.

**Non-Goals:**

- Requiring a PIC rewrite of the existing Z80 modules.
- Using an external assembler or linker during BASIC compilation.
- Paging or relocating the Arkos player in this change.
- Increasing Plain ROM capacity beyond 48 KB.
- Building a full multi-object linker; the module library is pre-assembled and self-describing.

## Decisions

### D1: Three-tier placement with kernel-first packing

Routines are classified as:

1. **Fixed resident** - boot, dispatcher, central table, `MR_*`, resource/paging management, traps/interrupt-critical, Arkos player body, timer interrupt handler.
2. **Flex high priority** - game graphics (`PUT SPRITE`/`PUT TILE`, tile/sprite access, collision), `90_support` math/strings/print/logic, other hot routines.
3. **Flex low priority** - MTF, fonts/compression, extended math/strings, date/time, USR, cold graphics.
4. **Overflow** - used groups that do not fit in the kernel holes after fixed and flex placement.

Placement is per module group, not per routine, so shared code stays together and intra-group calls remain direct.

The packer reserves fixed regions first, computes the used dependency closure, fills kernel free holes with high-priority flex groups, then low-priority groups, and sends only the remaining groups to overflow.

### D2: Runtime central table with selective resident and paged stubs

The central table lives in the fixed kernel page and contains only used entries.

```text
central_table:
  ; resident entry (3 bytes)
  jp routine
  ; paged entry (6 bytes)
  call dispatcher
  db segment          ; 8 KB-pair segment number
  dw address          ; 0x8000-0xBFFF
```

- Resident entries avoid the dispatcher and segment switch.
- Paged entries carry their own metadata inline; no parallel index table is needed.
- Unused `DISP_*` entries emit no stub.
- The table and dispatcher are reserved blocks during packing, so flex groups never overwrite them.

Alternative considered: a full 225-entry table. Rejected because it costs ~675 bytes for resident stubs alone and violates the requirement to include only used routines and dependencies.

Alternative considered: `call dispatcher; db id` at every call site. Rejected for this codebase because it grows every call site from 3 to 4 bytes and breaks conditional calls, which would need a trampoline.

### D3: Dispatcher ABI

The stub executes `call dispatcher`. The dispatcher:

1. Pops the stub return address.
2. Reads inline segment and address.
3. Saves the caller's argument registers and the current `SGMADR`.
4. Selects the target segment through the mapper-patched `MR_CHANGE_SGM`.
5. Restores argument registers and calls the target.
6. Saves result registers, restores `SGMADR`, restores result registers.
7. Returns to the original caller.

Nested calls work because each invocation saves the current segment and restores it before returning. The return address to the caller is on the RAM stack, not in the unmapped page.

Conditional calls are safe: `CALL NZ, stub` executes the stub only when the condition is true; the inline metadata is never executed as code.

Interrupt safety: the timer handler and trap path save/restore `SGMADR`. The dispatcher only changes the segment around the target call. If a mapper switch is not interrupt-safe on a given machine, interrupt disable is limited to the switch window, never to the whole paged routine.

The existing `MR_CALL` save/restore pattern must be audited. The preferred pattern is:

```asm
ld a, (SGMADR)
push af
  ; switch ...
pop af
jp MR_CHANGE_SGM
```

Any use of `SGMADR-1` as a 16-bit save must be verified and corrected if it does not restore the segment exactly.

### D4: Deferred dispatch fixups and stable DISP indices

`addKernelCall(DISP_x)` and conditional variants continue to be the only call API. They emit a 3-byte call placeholder and record a dispatch fixup with the index. The used set is available after code generation.

The builder computes the dependency closure from the module manifest, assigns stub addresses, and patches every dispatch fixup. `DISP_*` numeric values never change during relayout.

The virtual first half of `header.bin` becomes a build-time manifest/oracle (exports, module ownership, residency, relocations). It is not emitted as runtime code.

### D5: Player and Arkos are fixed-resident

The Arkos player body (`0x6481-0x6BD5`) and `player.int.control` remain at their current addresses. Reasons:

- The player executes from the timer interrupt while the song resource is mapped at `0x8000`.
- Moving the player to a paged segment would conflict with the song data window or require nested page switches in the interrupt.
- The binary is assembled with `org #6481`; reassembling would add a rasm/wine build dependency and require updating hardcoded `PLY_AKM_*` equates.

The `cmd_ply*` wrappers and `player.initialize/unhook` are not interrupt-critical and may become flex in a later step; in this change they remain fixed to keep the placement simple. The packer treats the player region as a reserved block, so the kernel free space is fragmented into holes before and after it.

### D6: `90_support` subgroups and game-graphics priority

`90_support` is split into:

- `90-graphics`: XBASIC graphics, VDP/palette/sprite assignment.
- `90-math`: float/trig/casts/compare, `SGN`, `ABS`, `INT`, `FIX`, `RND`.
- `90-strings`: string functions, copy/concat/compare, casts.
- `90-print`: `TAB`, `INT`, `FLOAT`, `CRLF`, `WriteParamBCD`.
- `90-logic`: integer comparison, shifts, neg, bitwise logic.
- `90-traps`: trap enable/disable/check, swap, `C6C25`/`C6C26`/`C6C27` — **pinned fixed**.

Game graphics from `31_cmd` and `90-graphics` form one atomic high-priority group so shared VDP/tile code stays together and a game loop does not alternate between segments. If it must overflow, the packer emits a performance warning. An optional `--pin-game-graphics` policy forces it resident or fails.

### D7: Relocations via embedded tables, not a PIC rewrite

The Z80 has no PC-relative `CALL`, and `JR` reaches only -128..+127 bytes. A pure relative-jump module is not feasible for the existing 90_support/31_cmd code. Reserving a base register (`IX`/`IY`) conflicts with existing BIOS and XBASIC usage, and computing the base at every routine is too expensive for hot code.

Instead, modules remain absolute and carry relocation records for their internal 16-bit absolute references. Cross-module calls use the fixed central table. The module therefore knows only its own routines and fixed kernel entry points, which is the desired isolation.

Relocation tables are generated at project build time, not BASIC compile time. The recommended prototyping method is **dual-base diff**:

1. Assemble a module at base `A` and base `B` (e.g., `0x8000` and `0x8100`).
2. Diff the two binaries.
3. Every word that changes is a candidate absolute reference; record its offset.
4. Fixed references (BIOS, RAM, fixed kernel stubs) do not change and are not recorded.
5. The builder applies `value += (final_base - A)` for each record.

If the diff proves unreliable for a module, fall back to an explicit relocation list generated from the source/symbol map. A full relocatable toolchain is a long-term option, not a prerequisite.

A per-module export table is still useful: it gives the central table a stable set of entry points and localizes cross-module ABI. It does not remove the need to patch internal absolute references.

### D8: Module format and embedded library

Each module is described by embedded metadata:

```text
module:
  id
  name
  bytes[]
  assembly_base
  size
  relocations[]   ; offsets of 16-bit absolute words
  exports[]       ; name -> offset
  dependencies[]  ; module ids
  policy          ; fixed / flex-high / flex-low
  fixed_address?  ; for player and other pinned regions
  patch_points[]  ; mapper patch offsets, when applicable
```

The module library, manifest, and relocation records are generated into C++ headers at project build time (same pattern as `header.h` and `start.h`) and compiled into `msxbas2rom`. BASIC compilation reads only these embedded structures.

### D9: Plain ROM layout

```text
0x0000  page 0: resources (mapped on demand)
0x4000  page 1: fixed core + player + traps + dispatcher + table
                + flex groups that fit in kernel holes
0x8000  page 2: start code (99 bytes)
                + user code
                + overflow groups
```

The packer validates `start code + user code + overflow <= 16384`. On failure it reports code size, used group sizes, placement decisions, and suggests MegaROM. Plain ROM remains a 48 KB format; the gain is that the kernel no longer consumes the entire 16 KB page.

### D10: MegaROM layout

```text
segments 0/1  : fixed core + player + traps + dispatcher + table
                + flex groups that fit in kernel holes
segments 2..N : user code
segments N+2..: overflow groups
segments .....: resources
```

Overflow groups are allocated after user code so existing user-code/trap segment numbers remain stable. The builder adjusts `resourceSegment` and resource/trap metadata. Stubs carry 8 KB-pair segment numbers; the mapper-patched switch routine handles ASCII8/Konami and converts to 16 KB pages for ASCII16/ASCII16-X.

### D11: Packer algorithm

```text
1. Reserve fixed regions:
   boot/header, player/Arkos + int.control, pinned traps,
   dispatcher, central-table budget, mapper patch points.
2. Compute used modules = closure(user DISP set, init/end, dependencies).
3. Split used modules into groups and sort:
   fixed first, then flex-high, then flex-low.
4. Compute initial table size assuming flex groups are resident.
5. Pack groups into kernel holes (best-fit decreasing within priority).
6. Move non-fitting groups to overflow in priority order.
7. Recompute table size (paged stubs are 3 bytes larger) and repack
   until stable.
8. Apply relocations and record final export addresses.
9. Generate resident and paged stubs, patch dispatch fixups.
10. Allocate overflow (Plain user page or MegaROM segments), patch
    resource/trap metadata and mapper patch points.
11. Validate budgets and emit diagnostics.
```

The fixed-point iteration is bounded: a group can move from resident to overflow, never back, so it terminates after at most one pass per group.

### D12: Diagnostics and observability

The builder reports:

- used groups and dependency closure;
- resident/overflow decision per group;
- kernel free bytes before and after packing;
- overflow bytes per group;
- central-table size;
- Plain ROM code budget or MegaROM segment allocation;
- warnings for game-graphics overflow and for any fallback relocation method.

## Risks / Trade-offs

- **Player remains fixed (~2.2 KB).** The kernel keeps a fragmented free map around `0x6481-0x6BD5`. Mitigation: treat the player region as reserved and pack flex groups into the holes before and after it.
- **Game graphics may overflow and slow a loop.** Mitigation: atomic high-priority group, segment cache, optional pin policy, and a build warning.
- **Relocation correctness.** Mitigation: dual-base diff prototype, comparison against fixed builds, unit tests for every relocation record, and explicit-list fallback.
- **Module isolation drift.** Mitigation: a manifest check that rejects direct cross-module references; cross-module calls must use the central table.
- **Dispatcher ABI bugs around interrupts/nesting.** Mitigation: emulator tests for nested calls, timer interrupts, traps, and every mapper; audit `MR_CALL` save/restore.
- **Table-size/placement circularity.** Mitigation: bounded fixed-point iteration and deterministic ordering.
- **Plain ROM overflow still cannot exceed 48 KB.** Mitigation: clear diagnostics and MegaROM suggestion; document that Plain ROM reallocates rather than expands.
- **MegaROM minimum size/filler.** Mitigation: allocate overflow after user code, compact groups where possible, and accept filler only when required by segment granularity.
- **Pasmo cannot emit relocations.** Mitigation: dual-base diff or explicit relocation lists generated in the project build; no runtime assembler.

## Migration Plan

1. **Phase 0 - Inventory and prototype.** Measure module sizes, build the manifest, identify fixed regions and patch points, prototype the dispatcher and dual-base relocation on one module.
2. **Phase 1 - Fixed core and player.** Keep player/Arkos pinned, define the fixed core, generate a runtime table for resident routines only.
3. **Phase 2 - Fixups and packer.** Record used DISP entries, emit deferred fixups, implement placement and relocation, generate selective stubs.
4. **Phase 3 - Plain ROM.** Add overflow placement in `0x8000`, budget validation, and diagnostics.
5. **Phase 4 - MegaROM.** Add segment allocation, resource/trap metadata updates, and all-mapper tests.
6. **Phase 5 - Group migration.** Move file I/O, MTF, fonts, extended math/strings, date/time, USR, and cold graphics to flex/overflow; keep game graphics high priority.
7. **Phase 6 - Hardening.** Tests, docs, spec archive, and release `1.4.0.0`.

Rollback strategy: keep the current monolithic kernel path behind the build-time packer until the new path matches behavior on the full integration suite. Each phase can be reverted independently; the player and `DISP_*` indices remain stable throughout.

## Open Questions

- Exact group sizes and the resulting plain-ROM overflow amount will be measured in Phase 0; the placement policy does not change.
- Whether `cmd_ply*` and `player.initialize/unhook` should later join a flex group can be decided after the first measurement; the initial design keeps them fixed.
- The dual-base diff tooling details (base spacing, word detection) are a prototype concern and do not change the specs or placement approach.
