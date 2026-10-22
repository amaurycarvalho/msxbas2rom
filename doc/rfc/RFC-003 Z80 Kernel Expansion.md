# RFC-003: Z80 Kernel Expansion via Central Jump Table and Paged Runtime Dispatch

| Field | Value |
|---|---|
| Status | Draft |
| Date | 2026-10-08 |
| Scope | Z80 kernel space, Plain ROM, MegaROM, dispatcher, ABI, build/runtime packaging |

## 1. Problem

There is currently no space in the Z80 kernel for routine expansion. We need to evaluate strategies to free kernel space without losing functionality.

In particular, evaluate the adoption of a central jump table that resides in the Z80 kernel segment/page, with routines allocated and distributed both in the kernel segment/page and in user-code segments/pages, depending on whether the compilation target is Plain ROM or MegaROM.

The intended model is:

- user code calls addresses in the central jump table;
- the jump table automatically allocates the segment of the called routine;
- the routine is called;
- control returns to the original caller segment.

Only routines actually called by user code and their dependencies should be inserted into the jump table, together with initialization/finalization routines, page/segment management, jump-table management, and related infrastructure.

The goal is to allow the kernel to grow without the current 16 KB restriction.

## 2. Root Cause

The current kernel is constrained by the fixed resident kernel page. Even if routines are moved into paged areas, the resident core must still contain bootstrap, mapper selection, the dispatcher, the compact table, traps, and high-frequency or critical routines.

Two existing mechanisms are decisive:

1. `wrapper_routines_map_table` already lists approximately 230 destinations, but it is only a build-time oracle. It does not go into the ROM.
2. `MR_CALL` already implements almost the required protocol:
   - save the current segment;
   - select the target segment at `0x8000–0xBFFF`;
   - call indirectly;
   - restore the caller segment.

Therefore, the central jump table proposal is technically viable for MegaROM, but it must reuse and evolve the existing `MR_CALL` protocol rather than create a second, competing mechanism.

A key architectural constraint is that “automatic allocation” cannot happen at runtime, because ROM is immutable. Allocation must occur at build time. The linker/runtime packager must compute the dependency closure, distribute modules into pages, and write the dispatch table. At runtime, the dispatcher only selects the correct bank.

## 3. Options

| Strategy                                 |                 Gain | Risk / Effort | Recommendation                                                                 |
| ---------------------------------------- | -------------------: | ------------: | ------------------------------------------------------------------------------ |
| Micro-optimization and dead-code removal |           Low/medium |           Low | Do first as baseline                                                           |
| Reorganize Arkos/assets and fillers      |           Low/medium |        Medium | Do as quick win, after measuring                                               |
| Optional modules per feature             |          Medium/high |        Medium | Do before the complete dispatcher                                              |
| Routine compression                      |               Medium |          High | Only for RAM-loadable modules; compressed ROM is not executable                |
| Central table + paged runtime            | Very high in MegaROM |          High | Main strategy                                                                  |
| Move everything to code pages only       |               Medium |          High | Insufficient without table/ABI; loses performance and does not solve Plain ROM |

There is an indication of immediate structural gain in the assembly itself: `90_support.asm` already records the intention to move the Arkos player to the end of the kernel and remove intermediate fillers. This may reduce fragmentation, but it does not replace modularization.

## 4. Better Solution

Adopt a hybrid architecture:

- minimal resident core;
- modules selected per program;
- a single dispatcher in MegaROM;
- selection/direct linking in Plain ROM.

### 4.1 Limits by Mode

| Mode          | Result of the proposal                                                                                                                                                            |
| ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Plain ROM     | No dynamic page switching. Routines can leave the fixed page and enter the code area, but they compete for the same total ROM space as the program. There is no unlimited growth. |
| MegaROM       | Routines can be distributed across 16 KB pages in additional banks. There is real capacity gain, up to the mapper/format limit.                                                   |
| Resident core | Still limited to 16 KB. It must contain only boot, ABI, dispatcher, compact table, mapper/trap management, and hot/critical routines.                                             |

Therefore, “the kernel can grow freely” is correct only for paged modules in MegaROM. For Plain ROM, the best strategy is module selection and dead stripping per program.

### 4.2 Recommended Architecture

```text
PAGE 1: 0x4000-0x7fff  (resident core)
+----------------------------------------------+
| bootstrap / BIOS / mapper / traps            |
| central dispatcher                           |
| dispatch directory of used services          |
| hot and non-pageable routines                |
+----------------------------------------------+

PAGE 2: 0x8000-0xbfff  (banked window)
+--------------------+-------------------------+
| user code          | runtime kernel modules  |
| current segment    | selected by build       |
+--------------------+-------------------------+
```

### 4.3 Call Flow

```text
user code
  |
  | CALL dispatcher + local id
  v
resident core
  |
  | save caller segment
  | locate {segment, address, flags}
  | select target bank
  v
paged routine
  |
  | RET
  v
resident core
  |
  | restore caller segment
  v
user code
```

### 4.4 Recommended Call Format

Do not use a table of 230 independent stubs. That would waste resident space.

Prefer a single dispatcher with a compacted local ID in the generated code:

```asm
call runtime_dispatch
db service_id
```

The dispatcher reads the ID, advances the return address, preserves the call ABI, selects the segment, calls the routine, and restores the previous segment.

For conditional calls, the C++ generator must emit a small conditional trampoline. `CALL NZ, dispatcher` followed by an ID is not safe, because the ID byte would be executed if the condition fails.

Very frequently called routines — arithmetic, strings, comparison, graphics primitives — should remain resident or co-located. Paging every call inside inner loops can severely degrade execution.

### 4.5 Module Selection

The table should contain only:

- routines called by generated BASIC code;
- the transitive closure of dependencies of those routines;
- bootstrap, `INIT/END`, and traps;
- segment selection/restoration;
- table and resource management;
- routines explicitly classified as resident.

A module manifest should be created in assembly, for example:

```text
module:
  id
  entry
  dependencies
  resident / pageable
  ABI requirements
  size
  page restriction
```

The compiler already knows high-level calls through `DISP_*`. However, today it resolves directly to the real address through `bin_header_bin`. The new flow must preserve symbolic IDs until packaging, build the closure, and apply relocations for inter-module calls.

## 5. Evaluate

### 5.1 Proposed Plan

1. **Measurement and classification**
   - Generate a size map per module/routine.
   - Build a static ASM call graph and cross it with `DISP_*` emitted by C++.
   - Classify routines as mandatory resident, hot, pageable, optional per feature, or asset/data.

2. **Quick wins without new ABI**
   - Measure and eliminate unreachable code.
   - Reorganize Arkos/fillers and static assets.
   - Extract obviously optional modules per feature: disk, player, fonts, MTF, graphics, etc.
   - Ensure binary tests before and after.

3. **Prototype the dispatcher**
   - Implement a minimal dispatcher using the existing `MR_CALL` protocol.
   - Validate nested calls, return to the correct bank, parameters/flags, traps, and interrupts.
   - Test in Plain ROM and in each supported MegaROM mapper.

4. **Create the manifest and packager**
   - Declare modules, entries, and dependencies in assembly.
   - Make the build produce relocatable blocks and metadata instead of only a monolithic kernel.
   - Make C++ select the closure per program and position modules in MegaROM pages.

5. **Migrate services by families**
   - First rare and large services: file/disk, player, MTF, fonts, resources.
   - Then graphics commands and specialized functions.
   - Keep arithmetic, strings, and high-frequency services resident until measurement justifies moving them.

6. **Integrate the C++ generator**
   - Replace direct address resolution with service IDs.
   - Emit dispatcher ABI, including a safe form for conditional calls.
   - Adapt relocation, symbols, and exporters.

7. **Policy per mode**
   - Plain ROM: select only used modules, link directly when possible, and fail with a clear diagnostic if the total budget exceeds the format.
   - MegaROM: allow paged modules, with a compact table only for the used closure.

8. **Verification**
   - Layout and manifest tests.
   - Normalized binary tests of generated C++.
   - Bank return/nesting tests per routine.
   - Emulator integration for mappers and traps.
   - Do not use global distances between routines as a test; only declared local ABI contracts.

### 5.2 Recommendation

Approve the hybrid strategy:

- minimal resident core;
- modules selected per program;
- a single dispatcher in MegaROM;
- selection/direct linking in Plain ROM.

This delivers real expansion without sacrificing the performance of critical primitives.

The central jump table is viable for MegaROM and can remove the growth limit for non-resident routines. It does not eliminate the 16 KB limit of the resident core. The proposal should evolve the existing `MR_CALL` mechanism, not replace it.
