## Context

See `proposal.md - Why` and `proposal.md - Dependencies`. This change depends on `add-rsctoram-offset-size` and must be applied after it.

Relevant state:

- `RESOURCE(n)` compiles to `addKernelCall(DISP_usr0)` in `compiler_resource_function_strategy.cpp`. Kernel `usr0` returns the resource pointer and then calls `resource.close`, which restores the BIOS page (plain ROM) or the previous segment (MegaROM). The returned pointer is therefore not usable by user code. `usr0` returns `0` for an invalid resource through `usr_def`.
- `FRE()` compiles to `ld hl, (HEAPSIZ)` (`compiler_fre_function_strategy.cpp`, `def_HEAPSIZ = 0xC027`), so `size = FRE()` and `size = HEAPSIZ` are equivalent at runtime.
- `HEAPSIZ` (`HEAPSTR+2`) is the free-heap size reported by `FRE()`. Variables live below `HEAPSTR`; the heap is `HEAPSTR..HEAPEND`, with the top 256 bytes reserved for BASIC temporary strings: `XBASIC_END` sets `FRETOP = HEAPSTR + HEAPSIZ - 256`.
- `RESOURCE(n)` is a single-slot loader: it writes to `HEAP()` on every call and does not advance `HEAPSTR`.
- After `add-rsctoram-offset-size`, `CMD RSCTORAM` accepts `offset`/`size` via the `PARM1` work area, clamps the copy to `min(size, resource_size - offset)`, detects invalid resources before copying, and returns `HL = dest` or `0`.
- The `GRAPH/test38a` regression baseline is volatile with no probes, so it does not currently validate the `COPY RESOURCE(0) TO (0,0)` result.

## Goals / Non-Goals

**Goals:**

- Make `RESOURCE(n)` return a directly usable RAM pointer to a copy of the resource, or `0` when the resource does not exist.
- Reuse the extended `CMD RSCTORAM` instead of adding a dedicated kernel routine and dispatch entry.
- Keep `RESOURCESIZE(n)`, `usr0`/`usr1`, `USR0`/`USR1`, the resource map, and all existing `DISP_*` indices untouched.
- Make the runtime behavior observable in the regression suite.

**Non-Goals:**

- Decompressing pletter/compressed resources (raw copy only).
- Implementing a general heap allocator or advancing `HEAPSTR`.
- Changing the resource-number register protocol. `refactor-resource-number-hl-protocol` is a stale draft and is not a dependency.
- Returning `0` when the resource is larger than `FRE()` (the approved behavior is truncation).

## Decisions

### Decision 1: Implement `RESOURCE(n)` as a thin wrapper over `CMD RSCTORAM`

The compiler strategy evaluates and casts `n`, stores `resource = n`, `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` in the `PARM1` layout, and calls `DISP_cmd_rsctoram`. The result in `HL` is the function result.

- **Rationale**: reuses the tested copy path, requires no new kernel routine and no new dispatch entry, and is the simplification requested.
- **Ordering**: the evaluated and cast resource number must be stored before the strategy overwrites `HL` with `HEAP()`/`HEAPSIZ`.
- **Alternative rejected**: a dedicated `resource.load` kernel routine (superseded by the `add-rsctoram-offset-size` approach).

### Decision 2: Pass `size = FRE()` and let the kernel clamp

`cmd_rsctoram` already clamps to `min(size, resource_size - offset)`. Passing `size = FRE()` therefore yields `min(resource_size, FRE())` without any conditional code in the compiled expression.

- **Rationale**: avoids runtime comparisons/branches in the generated code and keeps the strategy simple.
- **Trade-off**: a resource larger than `FRE()` is silently truncated (approved behavior), rather than returning `0`.

### Decision 3: Propagate the routine's `HL` result directly

`RESOURCE(n)` returns whatever `cmd_rsctoram` returns: `dest` on a successful copy, `0` when the resource does not exist or the effective length is zero.

- **Rationale**: `HL` is already the function-result register, so no extra code is needed. `resource_size == 0` and a missing resource both yield `0`, which is acceptable.
- **Dependency**: `add-rsctoram` must implement the carry check and preserve `dest`; otherwise `RESOURCE(invalid)` returns garbage and `RESOURCE(valid)` returns the wrong address.
- **Alternative rejected**: computing resource existence separately with `RESOURCESIZE(n)` (duplicates the check the kernel already performs).

### Decision 4: Share the `PARM1` layout with the CMD handler

The parameter offsets are defined once and reused by both `CompilerCmdRscToRamHandler` and the `RESOURCE` strategy.

- **Rationale**: prevents the two call sites from drifting.
- **Implementation**: expose the offsets from the same header/local constants used by the CMD handler; do not duplicate numeric offsets.

### Decision 5: Keep the approved `FRE()` copy length and document the string reserve

`RESOURCE(n)` copies up to `FRE()` bytes, even though the top 256 bytes are reserved for temporary strings.

- **Rationale**: preserves the approved `min(resource_size, FRE())` contract and avoids extra runtime arithmetic in the generated expression.
- **Trade-off**: a resource whose size is between `FRE() - 256` and `FRE()` can overwrite the temporary string area. This must be documented in the wiki and `doc/misc/z80-binary-code.md`.
- **Testing**: use resources that fit comfortably below `FRE() - 256`; do not add a regression fixture that intentionally corrupts the string workspace.

### Decision 6: Single-slot loader

`RESOURCE(n)` does not advance `HEAPSTR` and does not allocate a new buffer. Each call returns the same `HEAP()` address and overwrites the previous copy.

- **Rationale**: no heap allocator is in scope; the function is a loader, not an allocator.
- **Consequence**: callers must consume the copy before the next `RESOURCE(n)` call or any heap/string operation that reuses that area.

### Decision 7: Hard dependency ordering

`redefine-resource-function` is not implementable before `add-rsctoram-offset-size`, including its invalid-resource and return-value work. The dependency is recorded in the proposal and in the first task.

## Risks / Trade-offs

- [Calling a block-copy routine from within an expression] → `cmd_rsctoram` uses `DI`/`EI` and `ldir`/pletter only; it does not touch the BASIC expression stack, and the result is returned in `HL`.
- [`HEAPSIZ` overlaps the 256-byte string reserve] → documented caveat; users relying on strings can pre-check with `RESOURCESIZE(n) <= FRE() - 256`.
- [Repeated `RESOURCE(n)` calls overwrite the same `HEAP()` address] → documented; the function is a single-slot loader, not an allocator.
- [Silent truncation for oversized resources] → documented in `resource-functions/spec.md` and the wiki; users can compare `RESOURCESIZE(n)` with `FRE()`.
- [Generated code growth] → each `RESOURCE` call site grows by the `PARM1` setup. This may matter for plain-ROM user-code limits; quantify it during implementation and document it.
- [`COPY RESOURCE(0) TO (0,0)` now copies from RAM] → add a VRAM probe for the SCREEN 3 copy in `GRAPH/test38a` so the regression suite validates it; `XBASIC_COPY_FROM`/`BLTVM` accept RAM addresses.
- [Invalid resource handling depends on `add-rsctoram`] → task 1 validates the carry check before the strategy is implemented.
- [Dependency on an unmerged change] → task 1 blocks implementation until `add-rsctoram-offset-size` is applied, including its kernel-space gate.

## Migration Plan

1. Confirm `add-rsctoram-offset-size` is applied: extended `cmd_rsctoram` present, carry check returns `0`, `dest` preserved, and the kernel budget is satisfied.
2. Rewrite `ResourceCompilerFunctionStrategy` to set up the `PARM1` parameters and call `DISP_cmd_rsctoram`.
3. Add the unit test for the emitted setup and dispatch target.
4. Rebuild the compiler and recompile `test38a`.
5. Add the `GRAPH/test38a` VRAM probes and the new deterministic `RESOURCE` regression program; run `MISC/rsc1`, `make test-emulator`, and `make test-integration`.
6. Update `doc/misc/z80-binary-code.md` and the wiki `Extended Functions` page.

Rollback: revert the strategy to `DISP_usr0`. The regression probes can remain as harmless validations of the old or new behavior only if they are adjusted; otherwise revert them with the strategy.

## Open Questions

None. The `FRE()` vs `FRE() - 256` trade-off is documented as a known limitation rather than changing the approved behavior.
