# Tasks

## 1. Baseline, Manifest, and Fixed Regions

- [ ] 1.1 Produce a size map per kernel routine and module from the assembled kernel and symbols; verify the report lists every module with start, end, size, and owner.
- [ ] 1.2 Define the embedded module manifest schema (id, exports, dependencies, policy, relocations, fixed address, patch points); verify a generated manifest validates against the schema for the current kernel.
- [ ] 1.3 Inventory fixed regions and hardcoded address constraints (boot/header, player/Arkos, interrupt handler, trap-critical routines, mapper patch points); verify the inventory is recorded in the manifest and reviewed against `header.symbols.asm`.
- [ ] 1.4 Prototype the dispatcher ABI outside the full kernel: resident `jp` stub, paged `call dispatcher; db seg; dw addr` stub, register preservation, segment restore, and nested call; verify with an emulator harness.
- [ ] 1.5 Prototype dual-base relocation on one small module: assemble at two bases, diff, apply the generated relocation table at a third base; verify the relocated binary matches an assembly at that third base.
- [ ] 1.6 Add unit tests for the manifest schema, size map, and relocation prototype; verify they run in the existing unit-test target.

## 2. Kernel Source Split and Classification

- [ ] 2.1 Split `90_support.asm` into `90-graphics`, `90-math`, `90-strings`, `90-print`, `90-logic`, and `90-traps` boundaries while preserving behavior; verify the kernel still assembles and the existing kernel regression test passes.
- [ ] 2.2 Split `31_cmd.asm` into game-graphics, tile/sprite access, MTF, and cold graphics boundaries; verify the kernel still assembles and the integration suite is unchanged.
- [ ] 2.3 Mark every group as fixed, flex-high, flex-low, or pinned in the manifest; verify game graphics and `90_support` hot groups are flex-high and trap/interrupt-critical groups are pinned.
- [ ] 2.4 Keep the Arkos player body and `player.int.control` at their current addresses and add the player region to the reserved map; verify `ARKOS_PLAYER` and `PLY_AKM_*` addresses are unchanged.
- [ ] 2.5 Add manifest tests that reject a group with a direct cross-module reference; verify the test fails for an injected violation.

## 3. Runtime Central Table and Dispatcher

- [ ] 3.1 Implement central-table byte generation for resident stubs and paged stubs; verify a unit test checks exact bytes for both stub forms.
- [ ] 3.2 Implement the dispatcher assembly routine with caller register preservation, `SGMADR` save/restore, and return to the original caller; verify with an emulator test for a single paged call.
- [ ] 3.3 Implement selective table generation from the used dispatch set and dependency closure; verify unused entries produce no stub bytes.
- [ ] 3.4 Add emulator tests for nested paged calls, conditional calls, and a timer interrupt during a paged call; verify the active segment and return path remain correct.
- [ ] 3.5 Audit `MR_CALL`/`MR_GET_DATA`/`MR_GET_BYTE` segment save/restore and correct any use of `SGMADR-1` that does not restore the exact segment; verify with a MegaROM emulator test.
- [ ] 3.6 Document the stub/dispatcher ABI in the kernel source and design notes; verify the documented byte sequences match the generated table.

## 4. Compiler Used-Set and Dispatch Fixups

- [ ] 4.1 Add a used `DISP_*` set to the compiler context and record every `addKernelCall` and conditional variant; verify a unit test compiles a program and checks the expected used set.
- [ ] 4.2 Change `addKernelCall` and conditional variants to emit a 3-byte placeholder plus a dispatch fixup; verify unit tests inspect the placeholder bytes and fixup index.
- [ ] 4.3 Resolve dispatch fixups after the packer assigns stub addresses; verify an integration test compiles, packs, and checks that call targets point to the expected stubs.
- [ ] 4.4 Route `MR_CALL`, `MR_JUMP`, `MR_GET_DATA`, and `MR_GET_BYTE` through dispatch fixups instead of direct index resolution; verify MegaROM integration tests still pass.
- [ ] 4.5 Add tests that `DISP_*` values are unchanged across two different module layouts; verify the test compares the compiled `compiler_hooks.h` values.
- [ ] 4.6 Update compiler-facing documentation for deferred dispatch fixups; verify the documented flow matches the implementation.

## 5. Module Library, Relocations, and Packer

- [ ] 5.1 Generate embedded C++ headers for module binaries, manifests, relocation records, and mapper patch metadata at project build time; verify the generated headers compile into the CLI.
- [ ] 5.2 Implement the read-only module/manifest boundary in C++ with an in-memory test implementation; verify unit tests can supply deterministic modules and relocations.
- [ ] 5.3 Implement relocation application with fixed-reference exclusion; verify unit tests cover internal addresses, fixed kernel addresses, BIOS addresses, and RAM addresses.
- [ ] 5.4 Implement dependency-closure computation from the used dispatch set and module dependencies; verify unit tests include transitive dependencies and reject missing modules.
- [ ] 5.5 Implement kernel-first placement with fixed regions, holes, and flex priority; verify unit tests show high-priority groups placed before low-priority groups and overflow only when necessary.
- [ ] 5.6 Implement the table-size/placement fixed-point iteration; verify a unit test stabilizes with a mix of resident and overflow groups.
- [ ] 5.7 Implement placement diagnostics for resident-size, module-fit, Plain ROM, and mapper budgets; verify unit tests match the expected messages.
- [ ] 5.8 Add integration tests that build a ROM with a deterministic module library and verify resident/overflow/stub tables byte-for-byte.

## 6. Plain ROM Integration

- [ ] 6.1 Place Plain ROM overflow modules in `0x8000-0xBFFF` after start code and user code; verify an integration test checks the final addresses and ROM bytes.
- [ ] 6.2 Validate `start code + user code + overflow <= 16384`; verify a failure test checks the size breakdown and MegaROM suggestion.
- [ ] 6.3 Add the `--pin-game-graphics` option and its documentation; verify an option test forces residency or fails with a clear diagnostic.
- [ ] 6.4 Add Plain ROM emulator tests for paged overflow, nested calls, conditional calls, and a game-loop graphics sequence; verify behavior and segment restoration.
- [ ] 6.5 Document Plain ROM placement, budget rules, and diagnostics; verify the documented commands and error examples match the implementation.

## 7. MegaROM Integration

- [ ] 7.1 Allocate overflow modules in segments after user code; verify an ASCII8 integration test checks segment numbers and stub metadata.
- [ ] 7.2 Update `resourceSegment`, resource maps, and trap segment metadata after overflow allocation; verify resource and trap integration tests pass.
- [ ] 7.3 Verify paged stubs use 8 KB-pair segment numbers and that ASCII16/ASCII16-X convert them through the mapper-patched switch; verify mapper tests inspect the patched bytes.
- [ ] 7.4 Add emulator tests for ASCII8, Konami4, KonamiSCC, ASCII16, and ASCII16-X covering paged calls, nested calls, interrupts, and traps; verify all pass.
- [ ] 7.5 Document MegaROM module allocation, segment ordering, and resource/trap interactions; verify the documented layout matches the generated artifact.

## 8. Group Migration and Integration Hardening

- [ ] 8.1 Migrate file I/O, MTF, fonts/compression, date/time, and USR groups to flex/overflow; verify their integration tests pass with the groups resident and with them forced to overflow.
- [ ] 8.2 Migrate extended math/strings and cold graphics to flex-low while keeping game graphics flex-high; verify a placement test shows game graphics resident first.
- [ ] 8.3 Add a game-graphics performance regression check and record the dispatcher/segment-cache measurements; verify the results are documented and compared with the resident baseline.
- [ ] 8.4 Run the full unit, kernel, integration, and architecture test suites; verify `make test`, `make test-kernel`, and `make test-architecture` pass asynchronously.
- [ ] 8.5 Update the affected specs, ADR references, and user documentation; verify `openspec validate expand-kernel-central-jump-table` passes.
- [ ] 8.6 Verify no external tool or file is invoked during BASIC compilation by tracing the CLI execution; record the result in the change notes.

## Workflow follow-up

- Archive the change after review and test approval.
- Run the changelog skill as part of archiving.
- Publish release `1.4.0.0` using the project release workflow.
