## Dependencies

This change MUST be applied AFTER `add-rsctoram-offset-size`. It relies on the extended `CMD RSCTORAM` (`offset`/`size` parameters and the `HL = dest` / `0` return value). Do not implement this change before that one is complete.

## Why

The `RESOURCE(n)` function currently returns the raw resource pointer in ROM. On plain ROM that pointer lives in page 0 (`0x0000-0x3FFF`), and after `resource.close` restores the BIOS to page 0 the value is no longer directly readable or executable by compiled code (verified: `RESOURCE(0)` returns `&H1011` but `PEEK` reads BIOS bytes). MegaROM behaves the same way because the previous segment is restored. As a result `RESOURCE(n)` is effectively useless for user code such as `DEF USR = RESOURCE(n)` or `COPY RESOURCE(n) TO ...`.

With the extended `CMD RSCTORAM` (from `add-rsctoram-offset-size`), `RESOURCE(n)` can be redefined as a thin wrapper that copies the resource into free RAM and returns a directly usable RAM pointer.

## What Changes

- **BREAKING**: `RESOURCE(n)` no longer returns the ROM resource pointer. It now delegates to `CMD RSCTORAM` with `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` (the heap size `HEAPSIZ`), and returns the routine's result:
  - `0` when resource `n` does not exist;
  - `HEAP()` otherwise, containing a copy of `min(resource_size, FRE())` bytes.
- When the resource is larger than `FRE()`, the copy is **truncated** to `FRE()` bytes and a non-zero address is still returned (consistent with `size = min(resource_size, FRE())`).
- The resource bytes are copied verbatim (no decompression).
- `RESOURCESIZE(n)` is unchanged.
- No new kernel routine and no new dispatch entry are required; the existing `DISP_cmd_rsctoram` is reused.
- This change remains independent of `refactor-resource-number-hl-protocol`.

## Capabilities

### New Capabilities

- `resource-functions`: Behavior of `RESOURCE(n)` and `RESOURCESIZE(n)`, including the new load-to-RAM semantics, the `CMD RSCTORAM` delegation, and the failure (zero) result.

### Modified Capabilities

- `compiler`: The `RESOURCE(n)` compiler function strategy sets up the `CMD RSCTORAM` parameters and calls `DISP_cmd_rsctoram` instead of `DISP_usr0`.

## Impact

- **C++ compiler**: `compiler_resource_function_strategy.cpp` (parameter setup + `DISP_cmd_rsctoram`), and possibly a small shared helper with `CompilerCmdRscToRamHandler`.
- **Kernel**: none (reuses the extended `cmd_rsctoram`).
- **Compatibility**: `RESOURCE(n)` call sites change behavior. `tests/integration/GRAPH/test38a.bas` (`COPY RESOURCE(0) TO (0,0)`) now copies from RAM and must still work.
- **Docs**: `doc/misc/z80-binary-code.md` and the project wiki `Extended Functions` page must describe the new behavior.
- **Not impacted**: `RESOURCESIZE(n)`, `usr0`/`usr1`/`USR0`/`USR1`, resource map format/numbering.
