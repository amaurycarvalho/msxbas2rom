# Spike: kernel-space proposal for `cmd_rsctoram`

**Status**: proposal only — no repository code changed.

**Scope**: tasks 0.1 and 0.2 of `add-rsctoram-offset-size` (measure free space and define the space plan).

**Method**: measurements from the current tree plus throw-away prototypes in temporary copies of the repository (`git archive HEAD` extracted under `/tmp`). The temporary copies were used to validate the build pipeline; the repository itself was not modified.

## 1. Current measurements

| Item | Value |
| --- | --- |
| Kernel page | `0x4000-0x7FFF` (16,384 bytes) |
| `BASIC_KUN_START_FILLER` | `0x647B` |
| `ARKOS_PLAYER` | `0x6481` |
| Free space before `ARKOS_PLAYER` | **6 bytes** |
| `KERNEL_END_FILLER` | `0x7FFE` |
| Free space at the end | 2 bytes |
| `player.int.control` | `0x5659` (hard-coded in the manifest generator/tests) |
| Current `cmd_rsctoram` | `0x48F8-0x4912` = **26 bytes** |
| Candidate extended `cmd_rsctoram` | **86 bytes** final (88 bytes in the first prototype) |

The extended routine is roughly **62 bytes larger** than the current one. Even using the 6-byte filler before `ARKOS_PLAYER`, a resident implementation needs to reclaim about **56 bytes**.

Hard constraints found in the build:

- `scripts/generate_kernel_manifest.py` rejects a moved `ARKOS_PLAYER` (`0x6481`) and a moved `player.int.control` (`0x5659`).
- `tests/kernel/test_kernel_manifest.py` and `tests/unit/src/test_kernel_manifest.cpp` assert `ARKOS_PLAYER == 0x6481`.
- Adding 10 bytes to `cmd_rsctoram` in the current layout makes the filler `DEFS` wrap, producing a 65,536-byte `header.bin` with overlapping code.
- Merely deleting the old routine is also unsafe: it shifts `player.int.control` to `0x563F` and the manifest generation fails. The removed bytes must be replaced by a same-size filler (or the dependent addresses/tests must be redesigned).

## 2. Why overflow (`flex-high`) is unsafe for this routine

`cmd_rsctoram` calls `resource.open_and_get_address`, which on MegaROM calls `MR_CHANGE_SGM` and maps the **resource segment into page 2** (`0x8000-0xBFFF`, `Seg_P8000_SW`/`Seg_PA000_SW`).

If `cmd_rsctoram` is placed in an overflow module, its code also lives in page 2. The moment `resource.open_and_get_address` switches page 2 to the resource segment, the routine's own code is unmapped and execution fails. The same applies to any routine that reads resources through page 2.

Therefore:

- `flex-high` is **not** inherently safe. It is safe only if the packer guarantees that the module is placed **resident in a kernel hole** (page 1). The current packer does not guarantee that; it tries holes and otherwise sends the module to overflow.
- On Plain ROM, overflow code lives in page 2 while resources are selected on page 0, so overflow would happen to work. The MegaROM case is the blocker.
- The routine must be **resident** in the fixed kernel page. Marking it `fixed` is the simplest guarantee; a new "must be resident" packing policy would also work, but is a larger architectural change.

This invalidates the first prototype's overflow placement as a production plan, even though it built correctly.

## 3. Kernel holes available for a resident placement

The dynamic kernel removes flex modules from the fixed template and places them later. The final kernel therefore has holes where the flex modules used to be. Measured from the current manifest and `relocatable-modules.json`:

| Hole | Size | Template occupant | Suitability for `cmd_rsctoram` |
| --- | ---: | --- | --- |
| `0x4A86-0x505F` | 1497 | `31-cmd-tile-sprite` | Too wasteful (86 + 1411 filler) |
| `0x5492-0x5537` | **165** | `31-cmd-input` | **Best fit** (86 + 79 filler) |
| `0x5764-0x59AD` | 585 | `34_file_handling` | Possible, but 497 bytes of filler |
| `0x6C01-0x6C23` | 34 | `90-math-early` | Too small |
| `0x7504-0x7FFE` | 2810 | `90-print`/`90-math`/`90-strings`/`21_logic_pack` | Possible only with major template surgery |

The 165-byte slot at `0x5492-0x5537` is the natural fit:

- `31-cmd-input` is `flex-low`, does **not** read resources, and is already relocatable.
- Replacing its template slot with `cmd_rsctoram` (86 bytes) plus a 79-byte `DEFS` filler keeps every subsequent fixed address unchanged, including `player.int.control` at `0x5659` and `ARKOS_PLAYER` at `0x6481`.
- `31-cmd-input` moves to the `extra_modules` list and is placed by the packer in another hole or in overflow; it is safe there because it does not call `resource.*`.

## 4. Options considered

### Option A — reclaim ~56-62 bytes inside the fixed page

Candidate micro-optimizations:

- deduplicate the `usr0`/`usr1` open/address/close sequence (~8-12 bytes);
- share the argument-loading sequence of `cmd_ramtoram`/`cmd_ramtovram`/`cmd_vramtoram` (~6-9 bytes);
- rebase the stale `refactor-resource-number-hl-protocol` draft (~0-4 bytes net in the kernel).

These are not enough on their own and would mix unrelated refactorings into this change. **Not recommended as the primary plan.**

### Option B (rejected) — move `cmd_rsctoram` to an overflow module

Validated at build level but **unsafe on MegaROM** for the page-2 resource-mapping reason in section 2. Rejected.

### Option C (recommended) — fixed resident module in the `31-cmd-input` slot

1. Remove the 26-byte `cmd_rsctoram` body from `31-cmd-game-graphics.asm`.
2. Replace it with `DEFS 26, 0x00` so `player.int.control`, `ARKOS_PLAYER`, `BASIC_KUN_START_FILLER`, and all following fixed addresses remain unchanged.
3. Add `src/infrastructure/kernel/asm/src/header/31-cmd-rsctoram.asm` with the extended routine plus `DEFS 79, 0x00` (total 165 bytes).
4. Replace the `31-cmd-input` include in `31_cmd.asm` with `31-cmd-rsctoram.asm`.
5. Add `cmd_pad equ 0` in `20_runtime.asm` so the compile-time dispatch-name table still spells `cmd_pad` after `31-cmd-input` leaves the template.
6. Move `31-cmd-input` from `modules` to `extra_modules` in `relocatable-modules.json` (`source: 31-cmd-input.asm`, `group: input`, `classification: flex-low`, `policy: flex-low`).
7. Add `31-cmd-rsctoram` to `module-groups.json` as `fixed`.
8. The new module is fixed/resident at `0x5492`; `DISP_cmd_rsctoram` becomes a 3-byte resident stub. `31-cmd-input` remains relocatable and is placed in a hole/overflow when used.

Prototype results (temporary copy only):

- Kernel assembly succeeds; `header.bin` remains **32,768 bytes**.
- `BASIC_KUN_START_FILLER` remains `0x647B`; `ARKOS_PLAYER` remains `0x6481`; `player.int.control` remains `0x5659`.
- Manifest: `31-cmd-rsctoram` is fixed at `0x5492`, size 165 bytes (86 code + 79 filler).
- Relocatable modules: `31-cmd-input` is generated as an extra module (9 relocatable modules total).
- `tests/kernel/check_kernel.sh` passes (11/11; filler still 6 bytes, ARKOS content intact).
- `tests/kernel/test_kernel_manifest.py` passes (7 tests).
- `tests/kernel/check_relocation.py` passes.
- `make release` succeeds.
- Compiling `tests/integration/MISC/rsc1.bas` (ASCII8) places `31-cmd-rsctoram` **resident** at `0x21650` (`0x5492`); the central table is 96 bytes (resident stub, not the 6-byte paged stub).
- A minimal `A=PAD(0)` program places the `31-cmd-input` extra module **resident** at `0x22687` (`0x5897`), demonstrating that the moved module is still usable.
- The same `A=PAD(0)` program with `--force-overflow-module 31-cmd-input` compiles and places `31-cmd-input` in MegaROM segment 4, preserving the existing page-safe test path.

This is the recommended plan: it guarantees residency, preserves all fixed addresses, and costs only 79 bytes of fixed filler.

### Option D — move the built-in fonts plus `cmd_setfnt` to a relocatable module

The built-in font data is 1,124 bytes (`font.resource.data.1/2/3` in `80_resources.asm`) and `cmd_setfnt` is about 275 bytes, for roughly **1,399 bytes** before `ARKOS_PLAYER`.

- Moving both code and data together avoids data-address imports across segments.
- It is a much larger change: the font subsystem becomes paged/overflow, with page-safety and `CMD SETFNT` runtime coverage required.
- **Recommended as a follow-up** only if more kernel space is needed for future features.

### Option E — move `ARKOS_PLAYER` to the end of the page

Rejected for this change. It does not reduce total code size, and it requires recompiling `arkosplayer.bin`, updating hard-coded player symbols, the manifest generator, and the kernel tests.

### Option F — move the clamp into compiler-generated code

Complementary, not required. It could shrink the fixed routine but increases generated user code and duplicates the clamp in every `CMD RSCTORAM`/`RESOURCE` call site.

## 5. Recommendation

Adopt **Option C** for `add-rsctoram-offset-size`:

- `cmd_rsctoram` is fixed/resident, so `resource.open_and_get_address` can safely map page 2 on MegaROM;
- the fixed kernel layout remains byte-for-byte stable;
- no new dispatch index is required;
- the only permanent cost is 79 bytes of filler in the former `31-cmd-input` slot;
- `31-cmd-input` remains available as a relocatable module.

If the project later needs more headroom, schedule **Option D** as a separate change (`relocate-builtin-fonts`).

## 6. Implementation outline (for the apply phase)

1. Create `src/infrastructure/kernel/asm/src/header/31-cmd-rsctoram.asm` with the extended routine and a 79-byte filler. Use unique local labels (for example `cmd_rsctoram.copy`) because module labels are exported.
2. Remove the old body from `31-cmd-game-graphics.asm` and insert `DEFS 26, 0x00`.
3. Swap the `31-cmd-input` include in `31_cmd.asm` for `31-cmd-rsctoram.asm`.
4. Add the `cmd_pad equ 0` dispatch-name placeholder in `20_runtime.asm`.
5. Move `31-cmd-input` to `extra_modules` in `relocatable-modules.json`; add `31-cmd-rsctoram` to `module-groups.json` as fixed.
6. Update `CompilerCmdRscToRamHandler` to store/read the `PARM1` layout.
7. Run `make test-kernel`, `make test-emulator`, the new offset/size regression, `MISC/rsc1`, and the four-mapper `make test-integration`.

## 7. Test impact to account for

- `tests/dispatcher/check_page_safe_modules.py` currently uses `GAME_PROGRAM` (`CMD RSCTORAM`) to verify that `31-cmd-game-graphics` is fixed. After the move, `cmd_rsctoram` is no longer in that module; switch that case to a program that exercises a remaining `31-cmd-game-graphics` routine (for example `CMD PLAY`/`CMD WRTSCR`), and add an explicit **resident** runtime check for `31-cmd-rsctoram`.
- The `31-cmd-input` SAFE_CASE still works because the module remains a relocatable/overflow module; verify that `--force-overflow-module 31-cmd-input` still places it in overflow.
- `MISC/rsc1` must keep its baseline unchanged; its 2/3-argument `CMD RSCTORAM` calls must still work with the resident module.
- Add Plain ROM and MegaROM runtime coverage for `cmd_rsctoram`, especially a MegaROM program that actually maps a resource (the failure mode is MegaROM-only).
- Keep the new offset/size regression program and its baseline.

## 8. Risks

- The fixed routine reads resources while resident in page 1; this is the safe configuration. Any future attempt to mark it `flex-high`/overflow must be rejected unless a "must be resident" packing guarantee is added.
- `31-cmd-input` moves from the template to an extra module; verify its module group, dispatch name (`cmd_pad`), force-overflow test, and plain/MegaROM placement.
- The 79-byte filler is permanent fixed space; future kernel-space work should account for it.
- The extra module's local labels are exported; unique names are required to avoid duplicate-export failures.
- The `cmd_pad equ 0` placeholder is intentionally a non-kernel address; do not let it become a real export.
- Plain ROM and MegaROM overflow budgets change because `31-cmd-input` is now placed dynamically; run `check_plain_overflow.py` and `check_megarom_overflow.py`.

## 9. Spike conclusion

Task 0.1 is complete: the current kernel has only 6 usable bytes before `ARKOS_PLAYER`, and the extended routine is ~62 bytes larger.

Task 0.2 proposal is revised: **`cmd_rsctoram` must remain resident**. The recommended plan is **Option C**, a fixed module at `0x5492` in the former `31-cmd-input` slot (86 bytes code + 79 bytes filler), with `31-cmd-input` moved to the relocatable/extra modules. The earlier overflow option was rejected because it unmaps the routine on MegaROM when the resource segment is selected.

No code was changed in the repository for this spike.

**Implementation note (post-spike)**: the final clamp implementation is 86 bytes (16-bit comparisons), leaving 79 bytes of filler. The real repository build was applied and verified with the same layout: `header.bin` 32768 bytes, `ARKOS_PLAYER` `0x6481`, `player.int.control` `0x5659`, `BASIC_KUN_START_FILLER` `0x647B`; `make test-kernel`, `make test-unit`, `make test-emulator`, `make test-integration`, the new `MISC/rsc_offset` regression, and the unchanged `MISC/rsc1` baseline all pass.
