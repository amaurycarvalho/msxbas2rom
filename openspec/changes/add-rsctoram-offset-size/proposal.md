## Why

`CMD RSCTORAM` currently copies an entire resource to RAM, always from its first byte, and provides no way to limit how much is copied. Two needs are unmet:

- Partial/segmented loads of a resource (start at an arbitrary byte, copy a bounded range).
- A safe way to copy a resource into the free RAM area without overflowing it.

Extending `CMD RSCTORAM` with optional `offset` and `size` parameters solves both and lets `RESOURCE(n)` reuse the existing command instead of requiring a dedicated kernel routine (see the dependent change `redefine-resource-function`).

## What Changes

- Extend the syntax to `CMD RSCTORAM <resource>, <dest>[, <pletter>[, <offset>[, <size>]]]`:
  - `offset` (default `0`): first byte of the resource to copy.
  - `size` (default `resource_size - offset`): number of bytes to copy.
  - The effective copy length SHALL be `min(size, resource_size - offset)`.
  - Omitted `size` is treated as the full remaining resource.
- When `pletter = 1`, the `offset` and `size` parameters SHALL be ignored and the whole compressed resource SHALL be decompressed as before.
- The kernel `cmd_rsctoram` SHALL return `HL = dest` on success and `HL = 0` when the resource does not exist or nothing is copied.
- The command parameters SHALL be passed via the `PARM1` work area (same pattern as `CMD MTF`). This keeps the change independent of the resource-number register protocol (`refactor-resource-number-hl-protocol`).
- Backward compatible: `CMD RSCTORAM <resource>, <dest>[, <pletter>]` keeps its current behavior.

## Capabilities

### New Capabilities

- `resource-commands`: Behavior of `CMD RSCTORAM`, including the optional `offset`/`size` parameters, clamping, `pletter` interaction, and the return-value contract.

### Modified Capabilities

- `compiler`: The `CMD RSCTORAM` handler accepts up to five arguments and stores all parameters in the `PARM1` work area.

## Impact

- **Kernel ASM**: `cmd_rsctoram` (`31_cmd.asm`) reads parameters from `PARM1`, applies `offset`/`size`, and returns `HL`.
- **C++ compiler**: `CompilerCmdRscToRamHandler` accepts `offset`/`size` and writes the `PARM1` layout.
- **Compatibility**: existing 2- and 3-argument `CMD RSCTORAM` calls behave identically.
- **Enables**: the dependent change `redefine-resource-function` (must be applied after this one).
- **Tests**: extend integration coverage for offset/size, clamping, invalid resource, and pletter=1.
