## Context

See `proposal.md - Why` and `proposal.md - Dependencies`. This change depends on `add-rsctoram-offset-size` and must be applied after it.

Relevant state:

- `RESOURCE(n)` compiles to `addKernelCall(DISP_usr0)`; kernel `usr0` returns the resource pointer and then calls `resource.close`, which restores the BIOS page (plain ROM) or the previous segment (MegaROM). The returned pointer is therefore not usable by user code.
- `HEAPSIZ` (`HEAPSTR+2`) is the free-heap size reported by `FRE()`. Variables live below `HEAPSTR`; the heap is `HEAPSTR..HEAPEND`, with the top 256 bytes reserved for BASIC temporary strings.
- After `add-rsctoram-offset-size`, `CMD RSCTORAM` accepts `offset`/`size` via the `PARM1` work area, clamps the copy to `min(size, resource_size - offset)`, and returns `HL = dest` or `0`.

## Goals / Non-Goals

**Goals:**

- Make `RESOURCE(n)` return a directly usable RAM pointer to a copy of the resource, or `0` when the resource does not exist.
- Reuse the extended `CMD RSCTORAM` instead of adding a dedicated kernel routine and dispatch entry.
- Keep `RESOURCESIZE(n)`, `usr0`/`usr1`, `USR0`/`USR1`, the resource map, and all existing `DISP_*` indices untouched.

**Non-Goals:**

- Decompressing pletter/compressed resources (raw copy only).
- Implementing a general heap allocator or advancing `HEAPSTR`.
- Changing the resource-number register protocol (left to `refactor-resource-number-hl-protocol`).
- Returning `0` when the resource is larger than `FRE()` (the approved behavior is truncation).

## Decisions

### Decision 1: Implement `RESOURCE(n)` as a thin wrapper over `CMD RSCTORAM`

The compiler strategy stores `resource = n`, `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` in the `PARM1` layout and calls `DISP_cmd_rsctoram`. The result in `HL` is the function result.

- **Rationale**: reuses the tested copy path, requires no new kernel routine and no new dispatch entry, and is the simplification requested.
- **Alternative rejected**: a dedicated `resource.load` kernel routine (superseded by the `add-rsctoram-offset-size` approach).

### Decision 2: Pass `size = FRE()` and let the kernel clamp

`cmd_rsctoram` already clamps to `min(size, resource_size - offset)`. Passing `size = FRE()` therefore yields `min(resource_size, FRE())` without any conditional code in the compiled expression.

- **Rationale**: avoids runtime comparisons/branches in the generated code and keeps the strategy simple.
- **Trade-off**: a resource larger than `FRE()` is silently truncated (approved behavior), rather than returning `0`.

### Decision 3: Propagate the routine's `HL` result directly

`RESOURCE(n)` returns whatever `cmd_rsctoram` returns: `dest` on a successful copy, `0` when the resource does not exist or the effective length is zero.

- **Rationale**: `HL` is already the function-result register, so no extra code is needed. `resource_size == 0` and a missing resource both yield `0`, which is acceptable.
- **Alternative rejected**: computing resource existence separately with `RESOURCESIZE(n)` (duplicates the check the kernel already performs).

### Decision 4: Share the `PARM1` layout with the CMD handler

The parameter offsets are defined in one header/local block reused by both `CompilerCmdRscToRamHandler` and the `RESOURCE` strategy.

- **Rationale**: prevents the two call sites from drifting.

### Decision 5: Hard dependency ordering

`redefine-resource-function` is not implementable before `add-rsctoram-offset-size`. The dependency is recorded in the proposal and in the first task.

## Risks / Trade-offs

- [Calling a block-copy routine from within an expression] → `cmd_rsctoram` uses `DI`/`EI` and `ldir`/pletter only; it does not touch the BASIC expression stack, and the result is returned in `HL`.
- [`HEAPSIZ` overlaps the 256-byte string reserve] → documented caveat; users relying on strings can pre-check with `RESOURCESIZE(n) <= FRE() - 256`.
- [Repeated `RESOURCE(n)` calls overwrite the same `HEAP()` address] → documented; the function is a single-slot loader, not an allocator.
- [Silent truncation for oversized resources] → documented in `resource-functions/spec.md` and the wiki; users can compare `RESOURCESIZE(n)` with `FRE()`.
- [`COPY RESOURCE(0) TO (0,0)` now copies from RAM] → re-run `tests/integration/GRAPH/test38a.bas`; `XBASIC_COPY_FROM`/`BLTVM` accept RAM addresses.
- [Dependency on an unmerged change] → task 1 blocks implementation until `add-rsctoram-offset-size` is applied.

## Migration Plan

1. Confirm `add-rsctoram-offset-size` is applied (extended `cmd_rsctoram` present).
2. Rewrite `ResourceCompilerFunctionStrategy` to set up the `PARM1` parameters and call `DISP_cmd_rsctoram`.
3. Rebuild the compiler.
4. Recompile and validate `test38a` plus runtime probes (resource that fits, truncated resource, nonexistent resource).
5. Update `doc/misc/z80-binary-code.md` and the wiki `Extended Functions` page.

Rollback: revert the strategy to `DISP_usr0`.

## Open Questions

None.
