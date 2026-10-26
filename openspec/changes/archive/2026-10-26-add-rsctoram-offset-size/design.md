## Context

See `proposal.md - Why`. Current state relevant to the approach:

- `CompilerCmdRscToRamHandler` evaluates the resource into `HL`, the destination into `DE`, and the pletter flag into `A`, then calls `DISP_cmd_rsctoram`.
- Kernel `cmd_rsctoram` lives in `31-cmd-game-graphics.asm`. It writes `HL` to `(DAC)` and calls `resource.open_and_get_address` (which reads the resource number from `(DAC)`), then either `ldir` (uncompressed) or `resource.ram.unpack` (pletter) to the destination.
- `resource.open_and_get_address` returns with carry set for an invalid resource number. `cmd_rsctoram` currently ignores carry, so it can copy garbage instead of writing nothing. `usr0` (`32_usr.asm`) already handles this case and returns `0` through `usr_def`.
- The uncompressed `ldir` leaves `HL` at the end of the source, and `resource.close` may change registers. A success return value must therefore be saved explicitly.
- `CMD MTF` already establishes the pattern of storing all command parameters in the `PARM1` work area (`0xF6E8`) and reading them inside the kernel. `compiler_hooks.h` already defines `def_PARM1`.
- The resource map entry provides only the data pointer, segment, and size; no compression flag exists.
- The kernel is full: `BASIC_KUN_START_FILLER` is at `0x647B` and the fixed `ARKOS_PLAYER` starts at `0x6481`, leaving **6 bytes**. Adding 10 bytes to `cmd_rsctoram` makes the `DEFS` filler wrap and produces a 65536-byte `header.bin` with overlapping code. `scripts/generate_kernel_manifest.py`, `tests/kernel/test_kernel_manifest.py`, and `tests/unit/src/test_kernel_manifest.cpp` assert the `ARKOS_PLAYER` address.
- **Residency constraint**: `cmd_rsctoram` calls `resource.open_and_get_address`, which on MegaROM maps the resource segment into page 2 (`0x8000-0xBFFF`). If the routine itself is placed in a page-2 overflow module, the resource mapping unmaps its code. The routine must therefore stay **resident in the fixed kernel page**; `flex-high` is only safe if a packing guarantee forces resident placement, which the current packer does not provide.
- **Kernel-space spike**: see `kernel-space-proposal.md` for the complete measurements, the prototype evidence, and the selected fixed-slot relocation plan (tasks 0.1 and 0.2).

## Goals / Non-Goals

**Goals:**

- Add optional `offset` and `size` to `CMD RSCTORAM`, with defaults `0` and "full remaining resource".
- Clamp the copy to `min(size, resource_size - offset)`.
- Treat a `size` of `0` (omitted or explicit) as the full remaining resource.
- Return `HL = dest` on success and `HL = 0` on failure, so the routine can be reused by `RESOURCE(n)`.
- Keep 2- and 3-argument calls backward compatible.
- Keep the change independent of the stale `refactor-resource-number-hl-protocol` draft.

**Non-Goals:**

- Decompressing a sub-range of a pletter stream (offset/size are raw-copy only).
- Changing the parser (CMD already forwards all comma-separated arguments to the handler).
- Changing `resource.open_and_get_address` or the resource-number protocol.
- Adding a new dispatch entry (the existing `DISP_cmd_rsctoram` is reused).
- Moving `ARKOS_PLAYER` or recompiling the Arkos player.

## Decisions

### Decision 1: Pass parameters through the `PARM1` work area

The handler stores `resource`, `dest`, `pletter`, `offset`, and `size` as words in `PARM1`, and the kernel reads them there. Resource, destination, and pletter are migrated from `HL`/`DE`/`A` into `PARM1` too, so the interface is uniform (the `CMD MTF` pattern).

- **Rationale**: five parameters do not fit in registers; the work area is the established convention and removes the `HL`/`DE`/`A` coupling.
- **Alternative rejected**: pass `offset`/`size` on the stack (used by `set_sprite_hitbox`), which mixes calling conventions and is harder to keep ABI-stable.
- **Alternative rejected**: keep `HL`/`DE`/`A` and add only `offset`/`size` to `PARM1`, which leaves a hybrid convention.
- The `PARM1` offsets MUST be shared by `CompilerCmdRscToRamHandler` and the `RESOURCE` strategy (for example via compiler hooks constants) so the two call sites cannot drift.

### Decision 2: `size = 0` means "full remaining resource"

Because the compiler does not know resource sizes at compile time, the default `size` is encoded as `0`, and the kernel interprets `size = 0` as `resource_size - offset`. An explicit `size = 0` therefore also copies the full remaining resource; callers that want to copy nothing simply do not call the command.

- **Rationale**: keeps the compiler free of resource-metadata lookups and matches "default = resource size".
- **Trade-off**: cannot express "copy exactly zero bytes"; accepted as meaningless.
- **Specification**: the requirement and scenarios must state this exception to the `min(size, ...)` rule explicitly.

### Decision 3: Ignore `offset`/`size` when `pletter = 1`

The pletter path continues to decompress the whole resource; `offset` and `size` are read but not used.

- **Rationale**: streaming decompression cannot be bounded by a simple byte count without decompressing the skipped prefix, and there is no use case in scope.

### Decision 4: Return `HL = dest` or `0`, preserving `dest` before the copy

`cmd_rsctoram` saves `dest` before `ldir`/pletter and restores it into `HL` immediately before `ret`. It sets `HL = 0` when the resource number is invalid or the effective length is zero.

- **Rationale**: `ldir` leaves `HL` at the source end, and `resource.close` cannot be relied on to preserve `HL`. Saving `dest` is the only way to make the function-like contract reliable.
- **Compatibility**: `CMD RSCTORAM` as a statement ignores `HL`, so existing usage is unaffected.

### Decision 5: Effective length clamp

`cmd_rsctoram` computes `remaining = resource_size - offset` (0 if `offset >= resource_size`) and `effective = (size == 0) ? remaining : min(size, remaining)`, then copies `effective` bytes. If `effective == 0`, it writes nothing and returns `0`.

- **Rationale**: lets `RESOURCE(n)` pass `size = FRE()` and get `min(resource_size, FRE())` for free.

### Decision 6: Detect invalid resources before copying

After `resource.open_and_get_address`, `cmd_rsctoram` checks carry before reading `resource_size` or executing `ldir`/`resource.ram.unpack`. On carry, it skips the copy, calls `resource.close`, and returns `HL = 0`.

- **Rationale**: `resource.address` returns carry for a resource number beyond the map and leaves `HL`/`BC` in an undefined state. Without the check, the current routine copies garbage. This is required by the "no byte written" and `RESOURCE(invalid) == 0` contracts.
- **Precedent**: `usr0` performs the same check and routes to `usr_def`.

### Decision 7: Kernel-space budget is a prerequisite

The implementation must include one of:

- reclaim at least the required bytes from pre-`ARKOS_PLAYER` code (the current 6-byte filler is not enough);
- place the routine in a fixed/resident kernel slot without changing `ARKOS_PLAYER` or the player symbols;
- split the change so that a kernel-space reduction lands first.

**Chosen plan (spike)**: layout-preserving **fixed/resident** relocation of `cmd_rsctoram` into the former `31-cmd-input` slot at `0x5492-0x5537`.

- Remove the 26-byte body from `31-cmd-game-graphics.asm` and replace it with `DEFS 26` so `player.int.control` and `ARKOS_PLAYER` do not move.
- Create `31-cmd-rsctoram.asm` with the 86-byte routine plus a 79-byte filler (165 bytes total) and swap it for the `31-cmd-input` include in `31_cmd.asm`.
- Move `31-cmd-input` to the extra relocatable modules; it does not read resources and may be placed in a hole or overflow.
- Add `cmd_pad equ 0` in `20_runtime.asm` for the dispatch-name table.

This keeps the routine resident in page 1, which is mandatory because `resource.open_and_get_address` maps the resource segment into page 2 on MegaROM.

- **Rationale**: the monolithic template must still assemble with `ARKOS_PLAYER` at `0x6481` and `player.int.control` at `0x5659`. A negative `DEFS` wraps to 64 KB and corrupts the image. The 165-byte slot is the smallest existing kernel hole that fits the 86-byte routine, and replacing it with the same total size preserves every fixed address.
- **Rejected (after spike)**: placing the routine in an overflow module (`flex-high` + overflow). It builds, but on MegaROM the resource segment replaces page 2 and unmaps the routine.
- **Rejected**: moving `ARKOS_PLAYER`; the generator, kernel tests, and player binary address would all need changes, which is outside this change.
- **Deferred alternative**: moving the built-in fonts plus `cmd_setfnt` (~1,399 bytes) to a relocatable module; larger and better handled as a separate change if more space is needed.

## PARM1 layout

| Offset | Field | Size |
| ------ | ----- | ---- |
| `PARM1 + 0` | resource number | word |
| `PARM1 + 2` | destination address | word |
| `PARM1 + 4` | pletter flag (`0`/`1`) | word |
| `PARM1 + 6` | offset | word |
| `PARM1 + 8` | size (`0` = full remaining) | word |

The handler and the `RESOURCE` strategy MUST use the same shared offsets, mirroring the `compiler_cmd_mtf_handler.cpp` pattern.

## Risks / Trade-offs

- [Kernel page has only 6 bytes free] → use the fixed 165-byte `31-cmd-input` slot and preserve the total size with a 79-byte filler. Verify `ARKOS_PLAYER`, `player.int.control`, and `BASIC_KUN_START_FILLER` after every kernel change.
- [Resource mapping vs code location] → `resource.open_and_get_address` maps the resource into page 2 on MegaROM. `cmd_rsctoram` MUST stay resident in page 1; placing it in a page-2 overflow module is unsafe.
- [`31-cmd-input` moves to the extra relocatable modules] → verify its dispatch name (`cmd_pad`), force-overflow test, and Plain ROM/MegaROM placement.
- [Resource number protocol still flows through `(DAC)` inside `cmd_rsctoram`] → the kernel sets `(DAC)` from the `PARM1` resource number before calling `resource.open_and_get_address`, matching the current protocol.
- [`PARM1` clobbered by called routines] → `PARM1` is the documented command-parameter buffer (used by `cmd_mtf`); the routines called by `cmd_rsctoram` do not use it. Validate in the runtime tests.
- [Invalid resource detection] → not present in the current routine; the carry check is a new behavior and must be covered by runtime tests (missing resource leaves RAM untouched and returns `0`).
- [Return value preservation] → save `dest` before `ldir`; do not rely on `resource.close` preserving `HL`.
- [MegaROM resource with `offset` crossing a segment boundary] → the routine copies from a single mapped window; offsets are intended for resources that fit within the mapped segment. Documented limitation; not a regression (the whole-resource copy has the same constraint).
- [Changing `cmd_rsctoram`'s register interface] → only `CompilerCmdRscToRamHandler` calls it; changing both sides atomically keeps the ABI consistent.
- [Stale resource-protocol draft] → `refactor-resource-number-hl-protocol` is not an active dependency and must not be treated as one. If it is ever revived, rebase its `cmd_rsctoram` task against the new `PARM1` layout.

## Migration Plan

1. **Kernel budget gate**: the spike selected the fixed-slot plan in `kernel-space-proposal.md`. Remove the 26-byte body from `31-cmd-game-graphics.asm`, replace it with a same-size filler, put the 86-byte routine plus a 79-byte filler in `31-cmd-rsctoram.asm` at the former `31-cmd-input` slot (`0x5492-0x5537`), and move `31-cmd-input` to the extra relocatable modules. Verify the fixed symbols (`player.int.control`, `ARKOS_PLAYER`, `BASIC_KUN_START_FILLER`) are unchanged.
2. Implement kernel `cmd_rsctoram` in `31-cmd-rsctoram.asm` to read `PARM1`, check carry, handle `offset`/`size`, preserve `dest`, close on every path, and return `HL`. The module MUST remain fixed/resident; do not move it to page-2 overflow.
3. Update `CompilerCmdRscToRamHandler` to accept 2-5 arguments and write the `PARM1` layout.
4. Rebuild the kernel and compiler; verify `header.bin` is 32768 bytes, `ARKOS_PLAYER` is `0x6481`, and the player symbols are unchanged.
5. Add regression coverage: `MISC/rsc1` unchanged, `make test-emulator`, new offset/size program with a manifest entry and baseline, plus `make test-kernel`.
6. Update the wiki `Extended Commands` page and `doc/misc/z80-binary-code.md`.

Rollback: revert the handler and the kernel routine together. A kernel-space reclamation change should land separately so it can be reverted independently if needed.

## Open Questions

None. The kernel-space spike selected the layout-preserving relocation plan documented in `kernel-space-proposal.md`.
