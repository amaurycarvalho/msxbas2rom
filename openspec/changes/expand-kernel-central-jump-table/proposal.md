# Proposal

## Why

The Z80 kernel is effectively full: `header.bin` uses 16382 of the 16384 available kernel bytes, leaving 2 bytes. New runtime features cannot be added without a structural change. The current `wrapper_routines_map_table` is only a compile-time oracle; generated code calls absolute kernel addresses, so every routine must live in the fixed 16 KB kernel page.

This change introduces a runtime central jump table, build-time module packing, and embedded relocation metadata so non-essential kernel code can live in the kernel free space when it fits, and overflow to the user page (Plain ROM) or to MegaROM segments only when necessary. The player/Arkos code stays fixed-resident because it runs from the interrupt while the song resource occupies the `0x8000` window.

**Target release:** `1.4.0.0`.

## What Changes

- Add a runtime **central jump table** in the fixed kernel page. Resident entries resolve through `jp` stubs; paged entries use a dispatcher stub carrying `{segment, address}`.
- Add a **module manifest** (embedded in `msxbas2rom`) describing module ids, exports, dependencies, placement policy, fixed regions, and patch points.
- Add **embedded relocation metadata** for pre-assembled modules, so the BASIC compilation never invokes an external assembler, linker, or data file.
- Add a **build-time packer** that reserves boot/dispatcher/traps/player regions, computes the used-module closure, fills kernel free holes first, and sends only overflow to the user page or MegaROM segments.
- Track the **used `DISP_*` set** during compilation and defer kernel call resolution to dispatch fixups; the packer generates selective stubs and patches call sites.
- Classify routines into **fixed resident**, **flex high priority** (game graphics, `90_support` math/strings/print/logic), **flex low priority**, and **overflow**.
- Keep **player/Arkos fixed-resident** at its current address (`0x6481-0x6BD5`) and keep the timer interrupt handler resident.
- Split `90_support.asm` into graphics, math, strings, print, logic, and trap subgroups; trap/interrupt-critical routines are pinned resident.
- Keep `DISP_*` indices stable while replacing direct absolute kernel calls with stubs/fixups.
- Update Plain ROM and MegaROM placement, budget validation, and diagnostics.
- Add unit, kernel-regression, and emulator tests for dispatching, nested calls, interrupts, traps, relocations, and every supported mapper.

**BREAKING (internal):** kernel routines may move out of the fixed page; `header.symbols.asm` addresses for flex routines are no longer build constants, and any direct absolute reference to a flex routine must become a manifest/DISP reference. `header.bin` is no longer the sole runtime kernel image; the packer composes the final kernel/module layout per program.

## Capabilities

### New Capabilities

- `kernel-module-dispatch`: runtime central jump table, dispatcher ABI, module manifest, module placement policies, embedded relocations, and selective stub generation for the Z80 kernel.

### Modified Capabilities

- `kernel-call-routing`: kernel calls are no longer resolved directly from a virtual table; the compiler emits dispatch fixups and the builder patches resident stubs or paged dispatcher stubs.
- `wrapper-dispatch-table`: the dispatch table becomes a runtime structure in the kernel page, generated selectively for used routines, replacing the virtual-only word-pointer table.
- `virtual-dispatch-table`: the first half of `header.bin` becomes a build-time manifest/oracle for module exports and relocation data rather than the only source of callable addresses.
- `kernel-size-check`: the hard limit becomes the resident kernel page budget plus per-module/segment budgets; total kernel module images may exceed 16 KB in MegaROM, and Plain ROM validates user code plus overflow against 16 KB.
- `builder`: packages resident modules, flex modules, and overflow modules; performs placement, relocation, stub generation, segment allocation, and budget validation.
- `compiler`: records used `DISP_*` entries, emits deferred dispatch fixups, and preserves the existing `addKernelCall`/conditional-call API.
- `architecture`: records the kernel modularization ADR, the fixed-player rule, the module isolation rule, and the build-time allocation principle.
- `testable-compilation-boundaries`: kernel bytes, module manifests, relocation data, and dispatch addresses are supplied through the existing read-only kernel boundary and remain deterministic in tests.

## Impact

- **Kernel assembly:** `src/infrastructure/kernel/asm/src/header/*.asm`, especially `20_runtime.asm`, `31_cmd.asm`, `33_player.asm`, `90_support.asm`, and the Makefile that generates embedded headers.
- **Compiler:** `CompilerCodeOptimizer`, `Compiler`, `CompilerContext`, `compiler_hooks.h`, and all `addKernelCall`/conditional call sites.
- **Builder:** `Rom`, `RomArtifact`, kernel image boundary, start-code/resource/trap segment patching, and mapper fixups.
- **Generated assets:** new embedded module library, manifest, and relocation tables (generated at project build time, not BASIC compile time).
- **Specs:** new `kernel-module-dispatch`; deltas for the capabilities listed above.
- **Runtime constraints:** Plain ROM remains limited to 48 KB and shares `0x8000-0xBFFF` with overflow modules; MegaROM gains real capacity through paged modules.
- **Dependencies:** no new external tool or file at BASIC compile time; the project build may continue using pasmo/rasm to produce embedded modules.
- **Release:** `1.4.0.0`.
