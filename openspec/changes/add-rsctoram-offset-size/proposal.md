## Why

`CMD RSCTORAM` currently copies an entire resource to RAM, always from its first byte, and provides no way to limit how much is copied. Two needs are unmet:

- Partial/segmented loads of a resource (start at an arbitrary byte, copy a bounded range).
- A safe way to copy a resource into the free RAM area without overflowing it.

Extending `CMD RSCTORAM` with optional `offset` and `size` parameters solves both and lets `RESOURCE(n)` reuse the existing command instead of requiring a dedicated kernel routine (see the dependent change `redefine-resource-function`).

Current codebase facts that constrain this change:

- The kernel routine lives in `src/infrastructure/kernel/asm/src/header/31-cmd-game-graphics.asm` (`cmd_rsctoram`), not in `31_cmd.asm`, which now only includes the split CMD modules.
- `cmd_rsctoram` currently receives `resource` in `HL`, `dest` in `DE`, and `pletter` in `A`, writes the resource number to `(DAC)`, and calls `resource.open_and_get_address`.
- The uncompressed path uses `ldir`, which leaves `HL` pointing past the copied source; `resource.close` cannot be relied on to preserve a return value.
- The routine does not check the carry produced by `resource.address`/`resource.open_and_get_address` for a missing resource. `usr0` already shows the required pattern (`32_usr.asm`).
- The kernel page has only 6 bytes of free space before the fixed `ARKOS_PLAYER` block (`BASIC_KUN_START_FILLER` at `0x647B`, `ARKOS_PLAYER` at `0x6481`). Adding code without reclaiming space makes the `DEFS` filler wrap and corrupts the kernel layout; `scripts/generate_kernel_manifest.py` and the kernel tests also hard-code `ARKOS_PLAYER` at `0x6481`.
- `refactor-resource-number-hl-protocol` is a stale "do not apply" draft that still refers to `31_cmd.asm`. It is not an active dependency; the `(DAC)` write remains required until that draft is rewritten.

## What Changes

- Extend the syntax to `CMD RSCTORAM <resource>, <dest>[, <pletter>[, <offset>[, <size>]]]`:
  - `offset` (default `0`): first byte of the resource to copy.
  - `size` (default `resource_size - offset`): number of bytes to copy.
  - A `size` of `0` (omitted or explicitly supplied) selects the full remaining resource.
  - The effective copy length SHALL be `min(size, resource_size - offset)` when `size != 0`.
- When `pletter = 1`, the `offset` and `size` parameters SHALL be ignored and the whole compressed resource SHALL be decompressed as before.
- The kernel `cmd_rsctoram` SHALL:
  - detect an invalid resource from the carry flag before copying;
  - close the resource on every exit path;
  - return `HL = dest` on success and `HL = 0` when the resource does not exist or nothing is copied.
- The command parameters SHALL be passed via the `PARM1` work area (same pattern as `CMD MTF`). The kernel still sets `(DAC)` from `PARM1` before calling `resource.open_and_get_address`.
- Backward compatible: `CMD RSCTORAM <resource>, <dest>[, <pletter>]` keeps its current runtime behavior.
- **Kernel-space gate**: the implementation MUST include a plan to fit the new routine in the kernel page (reclaim bytes, adjust placement, or move the routine to a relocatable module without changing the fixed ARKOS addresses). The change is not implementable while only 6 bytes are available.

## Capabilities

### New Capabilities

- `resource-commands`: Behavior of `CMD RSCTORAM`, including the optional `offset`/`size` parameters, clamping, `pletter` interaction, invalid-resource handling, and the return-value contract.

### Modified Capabilities

- `compiler`: The `CMD RSCTORAM` handler accepts up to five arguments and stores all parameters in the `PARM1` work area.

## Impact

- **Kernel ASM**: `cmd_rsctoram` moves from `31-cmd-game-graphics.asm` to the fixed `31-cmd-rsctoram.asm` (former `31-cmd-input` slot). It reads parameters from `PARM1`, applies `offset`/`size`, checks the resource carry, preserves `dest` for the return value, and returns `HL`.
- **Kernel budget**: the spike selected a layout-preserving **fixed/resident** relocation of `cmd_rsctoram` into the former `31-cmd-input` slot at `0x5492-0x5537` (`kernel-space-proposal.md`). The old 26-byte body becomes a same-size filler; the 86-byte routine plus a 79-byte filler occupies the 165-byte slot; `31-cmd-input` moves to the extra relocatable modules. `ARKOS_PLAYER` remains at `0x6481` and `player.int.control` remains at `0x5659`.
- **Residency**: `cmd_rsctoram` must not be placed in page-2 overflow. `resource.open_and_get_address` maps the resource segment into page 2 on MegaROM, which would unmap the routine. `flex-high` is only safe with a resident-placement guarantee; the fixed slot provides that guarantee.
- **C++ compiler**: `CompilerCmdRscToRamHandler` accepts `offset`/`size` and writes the `PARM1` layout. The generated code for the 2- and 3-argument forms becomes larger than the current register-based sequence; this must be accounted for in user-code size.
- **Compatibility**: existing 2- and 3-argument `CMD RSCTORAM` calls behave identically at runtime.
- **Enables**: the dependent change `redefine-resource-function` (must be applied after this one).
- **Tests and regression**:
  - `MISC/rsc1` has a non-volatile baseline (`tests/integration/regression/baselines/MISC/rsc1.json`) covering the 2-argument uncompressed path and the 3-argument pletter path, including full RAM and VRAM text. Run it and keep the baseline unchanged unless the behavior intentionally changes.
  - `tests/dispatcher/check_page_safe_modules.py` (`make test-emulator`) runs 3-argument `CMD RSCTORAM` with pletter in openMSX and must keep passing. Its `31-cmd-game-graphics` fixed case must switch to a routine that remains in that module, and `31-cmd-rsctoram` needs explicit resident runtime coverage on Plain ROM and MegaROM.
  - The `31-cmd-input` SAFE_CASE must still work after the module moves to the extra relocatable modules, including `--force-overflow-module 31-cmd-input`.
  - Add a deterministic regression program for offset/size (exact range, clamp, offset at end, missing resource, `size=0`), with a manifest entry and RAM/VRAM checks; generate its baseline once.
  - `make test-kernel` must pass, including the ARKOS/fixed-region assertions.
  - `31-cmd-input` is now placed dynamically, so Plain ROM overflow and MegaROM segment budgets change; verify `check_plain_overflow.py` and `check_megarom_overflow.py`.
  - `make test-integration` remains the 4-mapper compilation matrix; it does not validate runtime semantics.
