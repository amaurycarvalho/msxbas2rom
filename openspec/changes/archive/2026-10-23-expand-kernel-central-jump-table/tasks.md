# Tasks

## 1. Baseline, Manifest, and Fixed Regions

- [x] 1.1 Produce a size map per kernel routine and module from the assembled kernel and symbols; verify the report lists every module with start, end, size, and owner.
- [x] 1.2 Define the embedded module manifest schema (id, exports, dependencies, policy, relocations, fixed address, patch points); verify a generated manifest validates against the schema for the current kernel.
- [x] 1.3 Inventory fixed regions and hardcoded address constraints (boot/header, player/Arkos, interrupt handler, trap-critical routines, mapper patch points); verify the inventory is recorded in the manifest and reviewed against `header.symbols.asm`.
- [x] 1.4 Prototype the dispatcher ABI outside the full kernel: resident `jp` stub, paged `call dispatcher; db seg; dw addr` stub, register preservation, segment restore, and nested call; verify with an emulator harness.
- [x] 1.5 Prototype dual-base relocation on one small module: assemble at two bases, diff, apply the generated relocation table at a third base; verify the relocated binary matches an assembly at that third base.
- [x] 1.6 Add unit tests for the manifest schema, size map, and relocation prototype; verify they run in the existing unit-test target.

## 2. Kernel Module Boundaries and Classification

- [x] 2.1 Define the routine-to-group mapping for `90_support.asm` and `31_cmd.asm`; verify every code label belongs to exactly one group (graphics, math, strings, print, logic, traps, game-graphics, tile/sprite, MTF, cold-graphics) and record the mapping.
- [x] 2.2 Split `90_support.asm` into `90-graphics`, `90-math`, `90-strings`, `90-print`, `90-logic`, and `90-traps` boundaries while preserving behavior; verify the kernel still assembles and the existing kernel regression test passes.
- [x] 2.3 Split `31_cmd.asm` into game-graphics, tile/sprite access, MTF, and cold graphics boundaries; verify the kernel still assembles and the integration suite is unchanged.
- [x] 2.4 Mark every group as fixed, flex-high, flex-low, or pinned in the manifest; verify `90_support` hot groups and tile/sprite are flex-high, resource-owning modules are fixed (page safety), and trap/interrupt-critical groups are pinned.
- [x] 2.5 Keep the Arkos player body and `player.int.control` at their current addresses and add the player region to the reserved map; verify `ARKOS_PLAYER` and `PLY_AKM_*` addresses are unchanged.
- [x] 2.6 Add manifest tests that reject a group with a direct cross-module reference; verify the test fails for an injected violation.

## 3. Relocatable Module Library and Packer Integration

- [x] 3.1 Generate embedded C++ headers for module binaries, manifests, relocation records, exports, dependencies, and mapper patch metadata at project build time; verify the generated headers compile into the CLI.
- [x] 3.2 Implement the read-only module/manifest boundary in C++ with an in-memory test implementation; verify unit tests can supply deterministic modules and relocations.
- [x] 3.3 Implement relocation application with fixed-reference exclusion; verify unit tests cover internal addresses, fixed kernel addresses, BIOS addresses, and RAM addresses.
- [x] 3.4 Implement dependency-closure computation from the used dispatch set and module dependencies; verify unit tests include transitive dependencies and reject missing modules.
- [x] 3.5 Implement kernel-first placement with fixed regions, holes, and flex priority; verify unit tests show high-priority groups placed before low-priority groups and overflow only when necessary.
- [x] 3.6 Implement the table-size/placement fixed-point iteration; verify a unit test stabilizes with a mix of resident and overflow groups.
- [x] 3.7 Implement placement diagnostics for resident-size, module-fit, Plain ROM, and mapper budgets; verify unit tests match the expected messages.
- [x] 3.8 Compose the resident kernel image from the used-module closure and omit unused modules; verify the composed resident image is at most 0x4000 bytes and unused modules produce no resident bytes.
- [x] 3.9 Add integration tests that build a ROM with a deterministic module library and verify resident/overflow/stub tables byte-for-byte.

## 4. Runtime Central Table and Dispatcher

- [x] 4.1 Implement central-table byte generation for resident stubs and paged stubs; verify a unit test checks exact bytes for both stub forms.
- [x] 4.2 Implement the dispatcher assembly routine with caller register preservation, `SGMADR` save/restore, and return to the original caller; verify with an emulator test for a single paged call.
- [x] 4.3 Implement selective table generation from the used dispatch set and dependency closure; verify unused entries produce no stub bytes.
- [x] 4.4 Add emulator tests for nested paged calls, conditional calls, and a timer interrupt during a paged call; verify the active segment and return path remain correct.
- [x] 4.5 Audit `MR_CALL`/`MR_GET_DATA`/`MR_GET_BYTE` segment save/restore and correct any use of `SGMADR-1` that does not restore the exact segment; verify with a MegaROM emulator test.
- [x] 4.6 Document the stub/dispatcher ABI in the kernel source and design notes; verify the documented byte sequences match the generated table.

## 5. Compiler Used-Set and Dispatch Fixups

- [x] 5.1 Add a used `DISP_*` set to the compiler context and record every `addKernelCall` and conditional variant; verify a unit test compiles a program and checks the expected used set.
- [x] 5.2 Change `addKernelCall` and conditional variants to emit a 3-byte placeholder plus a dispatch fixup; verify unit tests inspect the placeholder bytes and fixup index.
- [x] 5.3 Resolve dispatch fixups after the packer assigns stub addresses; verify an integration test compiles, packs, and checks that call targets point to the expected stubs.
- [x] 5.4 Route `MR_CALL`, `MR_JUMP`, `MR_GET_DATA`, and `MR_GET_BYTE` through dispatch fixups instead of direct index resolution; verify MegaROM integration tests still pass.
- [x] 5.5 Add tests that `DISP_*` values are unchanged across two different module layouts; verify the test compares the compiled `compiler_hooks.h` values.
- [x] 5.6 Update compiler-facing documentation for deferred dispatch fixups; verify the documented flow matches the implementation.

## 6. Plain ROM Integration

- [x] 6.1 Place Plain ROM overflow modules in `0x8000-0xBFFF` after start code and user code; verify an integration test checks the final addresses and ROM bytes.
- [x] 6.2 Validate `start code + user code + overflow <= 16384`; verify a failure test checks the size breakdown and MegaROM suggestion.
- [x] 6.3 Add the `--pin-game-graphics` option and its documentation; verify an option test forces residency or fails with a clear diagnostic.
- [x] 6.4 Add Plain ROM emulator tests for overflow modules, nested calls, conditional calls, and a game-loop graphics sequence; verify behavior and resident-page return.
- [x] 6.5 Document Plain ROM placement, budget rules, and diagnostics; verify the documented commands and error examples match the implementation.

## 7. MegaROM Integration

- [x] 7.1 Allocate overflow modules in segments after user code; verify an ASCII8 integration test checks segment numbers and stub metadata.
- [x] 7.2 Update `resourceSegment`, resource maps, and trap segment metadata after overflow allocation; verify resource and trap integration tests pass.
- [x] 7.3 Verify paged stubs use 8 KB-pair segment numbers and that ASCII16/ASCII16-X convert them through the mapper-patched switch; verify mapper tests inspect the patched bytes.
- [x] 7.4 Add emulator tests for ASCII8, Konami4, KonamiSCC, ASCII16, and ASCII16-X covering paged calls, nested calls, interrupts, and traps; verify all pass.
- [x] 7.5 Document MegaROM module allocation, segment ordering, and resource/trap interactions; verify the documented layout matches the generated artifact.

## 8. Group Migration and Integration Hardening

> Page-safety rule: modules that call `resource.open_and_get_address`/
> `resource.address` or execute while resource data is mapped at `0x8000`
> must remain in the fixed kernel page. This covers `32_usr`, `70_pletter`,
> `31-cmd-mtf`, `31-cmd-cold-graphics`, `31-cmd-game-graphics`, and
> `90-graphics`. `30_basic_helpers` (BLOAD, DATA/RESTORE/READ) and
> `80_resources` are already fixed.

- [x] 8.1 Migrate `34_file_handling` to flex-low/overflow and keep `70_pletter`, `31-cmd-mtf`, and `32_usr` fixed-resident; verify file I/O overflow runtime in Plain ROM and all MegaROM mappers, plus resident runtime and forced-overflow rejection for the fixed modules.
- [x] 8.2 Migrate `90-math`, `90-math-early`, `90-strings`, `21_logic_pack`, and `90-print` to relocatable flex/overflow; keep `31-cmd-cold-graphics`, `31-cmd-game-graphics`, and `90-graphics` fixed; keep `31-cmd-tile-sprite` flex-high/overflow; verify overflow runtime for the flex modules, resident runtime for the fixed resource modules, DATA/RESTORE/READ and `TAB`, and forced-overflow rejection for fixed modules.
- [x] 8.3 Add a graphics dispatch performance regression check and record the dispatcher measurements; verify the results are documented and compared with the resident baseline. The measured overflow group is `31-cmd-tile-sprite`; game graphics remains fixed for page safety.
- [x] 8.4 Run the full unit, kernel, integration, and architecture test suites; verify `make lint`, `make test`, `make test-kernel`, `make test-architecture` and `make test-integration` pass asynchronously.
- [x] 8.5 Update the affected specs, ADR references, and user documentation; verify `openspec validate expand-kernel-central-jump-table` passes.
- [x] 8.6 Verify no external tool or file is invoked during BASIC compilation by tracing the CLI execution; record the result in the change notes.

## 9. Module Import Boundary, Page Safety, and Enforcement

- [x] 9.1 Extend manifest generation with module imports: record target export, byte offset, and kind (`call`, `jump`, conditional, or address-of pointer); verify the generated header exposes them and unit tests cover each kind.
- [x] 9.2 Implement build-time import fixups in the composer and overflow path: apply each recorded import after placement; verify final bytes point to the resolved central-table stub and `DISP_*` indices 0-224 stay unchanged.
- [x] 9.3 Add generic address-of/pointer fixup support and verify `USRTAB` remains correct while `32_usr` stays fixed; verify forced overflow of `32_usr` is rejected. The earlier plan to overflow USR was rejected because USR0/USR1 execute from the module while `0x8000` is remapped to resource data.
- [x] 9.4 Enforce import validity in manifest generation: imports must target movable modules; cross-module `jr` is rejected; fixed/pinned references remain direct; verify negative tests fail on an injected violation.
- [x] 9.5 Add a packer guard that rejects forced overflow or relocation of a fixed module; verify the diagnostic names the module instead of silently producing a broken ROM.
- [x] 9.6 Keep the game-graphics group fixed-resident and verify forced overflow is rejected; generic atomic group placement remains implemented and unit-tested for future flex groups.
- [x] 9.7 Extend runtime coverage in `tests/dispatcher`: overflow runtime for file I/O, input, math, strings, logic, print, and tile/sprite in Plain ROM and all five MegaROM mappers; resident runtime and forced-overflow rejection for USR, MTF, pletter, cold graphics, game graphics, and `90-graphics`; resident DATA/RESTORE/READ in MegaROM.
- [x] 9.8 Audit and fix hardcoded kernel addresses: replace the stale `jp 0x7EF5` in `XBASIC_TAB` with `jp C7EF2`, and skip the start-code `C7369` patch when `90-graphics` is fixed; verify no other numeric kernel routine addresses remain.
- [x] 9.9 Correct the paged dispatcher ABI: restore caller A/BC/DE/HL/IX before entering the target, expose target result registers, preserve the target A across segment restore, and use a reentrant-safe scratch frame; verify with the openMSX prototype that the target sees arguments and returns results.

## Workflow follow-up

- Archive the change after section 9, 8.1, and 8.2 are complete and reviewed.
- Run the changelog skill as part of archiving.
- Publish release `1.4.0.0` using the project release workflow.
