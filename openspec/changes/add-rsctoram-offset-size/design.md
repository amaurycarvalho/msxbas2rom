## Context

See `proposal.md - Why`. Current state relevant to the approach:

- `CompilerCmdRscToRamHandler` evaluates the resource into `HL`, the destination into `DE`, and the pletter flag into `A`, then calls `DISP_cmd_rsctoram`.
- Kernel `cmd_rsctoram` opens the resource through `resource.open_and_get_address` (which currently reads the resource number from `(DAC)`), then either `ldir` (uncompressed) or `resource.ram.unpack` (pletter) to the destination.
- `CMD MTF` already establishes the pattern of storing all command parameters in the `PARM1` work area (`0xF6E8`) and reading them inside the kernel.
- The resource map entry provides only the data pointer, segment, and size; no compression flag exists.

## Goals / Non-Goals

**Goals:**

- Add optional `offset` and `size` to `CMD RSCTORAM`, with defaults `0` and "full remaining resource".
- Clamp the copy to `min(size, resource_size - offset)`.
- Return `HL = dest` on success and `HL = 0` on failure, so the routine can be reused by `RESOURCE(n)`.
- Keep 2- and 3-argument calls backward compatible.
- Keep the change independent of the resource-number register protocol.

**Non-Goals:**

- Decompressing a sub-range of a pletter stream (offset/size are raw-copy only).
- Changing the parser (CMD already forwards all comma-separated arguments to the handler).
- Changing `resource.open_and_get_address` or the resource-number protocol.
- Adding a new dispatch entry (the existing `DISP_cmd_rsctoram` is reused).

## Decisions

### Decision 1: Pass parameters through the `PARM1` work area

The handler stores `resource`, `dest`, `pletter`, `offset`, and `size` as words in `PARM1`, and the kernel reads them there. Resource, destination, and pletter are migrated from `HL`/`DE`/`A` into `PARM1` too, so the interface is uniform (the `CMD MTF` pattern).

- **Rationale**: five parameters do not fit in registers; the work area is the established convention and removes the `HL`/`DE`/`A` coupling. It also makes the change independent of `refactor-resource-number-hl-protocol`, which rewrites `cmd_rsctoram`'s use of `DAC`.
- **Alternative rejected**: pass `offset`/`size` on the stack (used by `set_sprite_hitbox`), which mixes calling conventions and is harder to keep ABI-stable.
- **Alternative rejected**: keep `HL`/`DE`/`A` and add only `offset`/`size` to `PARM1`, which leaves a hybrid convention and still conflicts with the pending refactor.

### Decision 2: `size = 0` means "full remaining resource"

Because the compiler does not know resource sizes at compile time, the default `size` is encoded as `0`, and the kernel interprets `size = 0` as `resource_size - offset`. An explicit `size = 0` therefore copies the full remaining resource; callers that want to copy nothing simply do not call the command.

- **Rationale**: keeps the compiler free of resource-metadata lookups and matches "default = resource size".
- **Trade-off**: cannot express "copy exactly zero bytes"; accepted as meaningless.

### Decision 3: Ignore `offset`/`size` when `pletter = 1`

The pletter path continues to decompress the whole resource; `offset` and `size` are read but not used.

- **Rationale**: streaming decompression cannot be bounded by a simple byte count without decompressing the skipped prefix, and there is no use case in scope.

### Decision 4: Return `HL = dest` or `0`

`cmd_rsctoram` sets `HL = dest` after a successful copy and `HL = 0` when the resource number is invalid or the effective length is zero.

- **Rationale**: lets `RESOURCE(n)` invoke the command as a function-like operation and propagate the result, avoiding a dedicated kernel routine.
- **Compatibility**: `CMD RSCTORAM` as a statement ignores `HL`, so existing usage is unaffected.

### Decision 5: Effective length clamp

`cmd_rsctoram` computes `remaining = resource_size - offset` (0 if `offset >= resource_size`) and `effective = (size == 0) ? remaining : min(size, remaining)`, then copies `effective` bytes.

- **Rationale**: lets `RESOURCE(n)` pass `size = FRE()` and get `min(resource_size, FRE())` for free.

## PARM1 layout

| Offset | Field | Size |
| ------ | ----- | ---- |
| `PARM1 + 0` | resource number | word |
| `PARM1 + 2` | destination address | word |
| `PARM1 + 4` | pletter flag (`0`/`1`) | word |
| `PARM1 + 6` | offset | word |
| `PARM1 + 8` | size (`0` = full remaining) | word |

The handler defines these offsets locally, mirroring `compiler_cmd_mtf_handler.cpp`.

## Risks / Trade-offs

- [Resource number protocol still flows through `(DAC)` inside `cmd_rsctoram`] → the kernel sets `(DAC)` from the `PARM1` resource number before calling `resource.open_and_get_address`, matching the current protocol. When `refactor-resource-number-hl-protocol` lands, this single write is removed.
- [`PARM1` clobbered by called routines] → `PARM1` is the documented command-parameter buffer (used by `cmd_mtf`); the routines called by `cmd_rsctoram` do not use it.
- [MegaROM resource with `offset` crossing a segment boundary] → the routine copies from a single mapped window; offsets are intended for resources that fit within the mapped segment. Documented limitation; not a regression (the whole-resource copy has the same constraint).
- [Changing `cmd_rsctoram`'s register interface] → only `CompilerCmdRscToRamHandler` calls it; changing both sides atomically keeps the ABI consistent.
- [Conflict with `refactor-resource-number-hl-protocol` task 2.4] → that refactor already lists `cmd_rsctoram`; whichever lands second must rebase the `(DAC)` write. Documented.

## Migration Plan

1. Rewrite kernel `cmd_rsctoram` to read `PARM1`, handle `offset`/`size`, and return `HL`.
2. Update `CompilerCmdRscToRamHandler` to accept 2-5 arguments and write the `PARM1` layout.
3. Rebuild the kernel and compiler.
4. Add integration tests (offset, clamp, invalid resource, pletter=1) and validate in openMSX.
5. Update the wiki `Extended Commands` page.

Rollback: revert both the handler and the kernel routine together.

## Open Questions

None.
