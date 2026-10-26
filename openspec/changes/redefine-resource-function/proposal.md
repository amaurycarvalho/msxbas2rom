## Dependencies

This change MUST be applied AFTER `add-rsctoram-offset-size`. It relies on the extended `CMD RSCTORAM` (`offset`/`size` parameters, invalid-resource detection, and the `HL = dest` / `0` return value). Do not implement this change before that one is complete. The dependency's kernel-space spike is documented in `../add-rsctoram-offset-size/kernel-space-proposal.md`; the selected plan keeps `cmd_rsctoram` **fixed/resident** in the former `31-cmd-input` slot, because the routine maps resource segments into page 2 on MegaROM and must not execute from an overflow segment.

## Why

The `RESOURCE(n)` function currently returns the raw resource pointer in ROM. On plain ROM that pointer lives in page 0 (`0x0000-0x3FFF`), and after `resource.close` restores the BIOS to page 0 the value is no longer directly readable or executable by compiled code (verified: `RESOURCE(0)` returns `&H1011` but `PEEK` reads BIOS bytes). MegaROM behaves the same way because the previous segment is restored. As a result `RESOURCE(n)` is effectively useless for user code such as `DEF USR = RESOURCE(n)` or `COPY RESOURCE(n) TO ...`.

With the extended `CMD RSCTORAM` (from `add-rsctoram-offset-size`), `RESOURCE(n)` can be redefined as a thin wrapper that copies the resource into free RAM and returns a directly usable RAM pointer.

Current codebase facts that constrain this change:

- `compiler_resource_function_strategy.cpp` currently emits `addKernelCall(DISP_usr0)`. Kernel `usr0` returns the ROM pointer and handles an invalid resource through `usr_def`, returning `0`.
- `FRE()` is compiled as `ld hl, (HEAPSIZ)` (`compiler_fre_function_strategy.cpp`, `def_HEAPSIZ = 0xC027`), so `size = FRE()` and `size = HEAPSIZ` are equivalent at runtime.
- `HEAPSTR` (`0xC025`) and `HEAPSIZ` (`0xC027`) are adjacent RAM words. `XBASIC_END` computes `FRETOP = HEAPSTR + HEAPSIZ - 256`, reserving the top 256 bytes of the heap for BASIC temporary strings.
- `RESOURCE(n)` becomes a single-slot loader: every call overwrites the same `HEAP()` address and does not advance `HEAPSTR`.
- The `GRAPH/test38a` regression baseline is marked `volatile: true` and has no probes, so it currently does not validate `COPY RESOURCE(0) TO (0,0)`.

## What Changes

- **BREAKING**: `RESOURCE(n)` no longer returns the ROM resource pointer. It now delegates to `CMD RSCTORAM` with `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` (the heap size `HEAPSIZ`), and returns the routine's result:
  - `0` when resource `n` does not exist;
  - `HEAP()` otherwise, containing a copy of `min(resource_size, FRE())` bytes.
- When the resource is larger than `FRE()`, the copy is **truncated** to `FRE()` bytes and a non-zero address is still returned (consistent with `size = min(resource_size, FRE())`).
- The resource bytes are copied verbatim (no decompression).
- `RESOURCESIZE(n)` is unchanged.
- `RESOURCE(n)` does not allocate memory and does not advance `HEAPSTR`; repeated calls overwrite the previous copy.
- No new kernel routine and no new dispatch entry are required; the existing `DISP_cmd_rsctoram` is reused.
- This change remains independent of the stale `refactor-resource-number-hl-protocol` draft.

## Capabilities

### New Capabilities

- `resource-functions`: Behavior of `RESOURCE(n)` and `RESOURCESIZE(n)`, including the new load-to-RAM semantics, the `CMD RSCTORAM` delegation, the single-slot loader behavior, and the failure (zero) result.

### Modified Capabilities

- `compiler`: The `RESOURCE(n)` compiler function strategy sets up the `CMD RSCTORAM` parameters and calls `DISP_cmd_rsctoram` instead of `DISP_usr0`.

## Impact

- **C++ compiler**: `compiler_resource_function_strategy.cpp` (parameter setup + `DISP_cmd_rsctoram`), and a shared `PARM1` offset definition with `CompilerCmdRscToRamHandler`.
- **Generated code size**: each `RESOURCE(n)` call site grows from a 3-byte `call usr0` to the `PARM1` setup sequence (roughly 20-30 bytes). This affects the plain-ROM user-code budget and should be documented.
- **Kernel**: none directly (reuses the extended `cmd_rsctoram`), but the dependency must provide invalid-resource detection and a preserved `HL = dest` result. The `add-rsctoram` kernel-space gate applies.
- **Heap/string risk**: `size = FRE()` can copy into the top 256-byte temporary-string reserve. The approved behavior uses `FRE()`; the limitation must be documented and regression tests should use resources that fit comfortably below `FRE() - 256`.
- **Compatibility**: `RESOURCE(n)` call sites change behavior. `tests/integration/GRAPH/test38a.bas` (`COPY RESOURCE(0) TO (0,0)`) now copies from RAM and must still work.
- **Docs**: `doc/misc/z80-binary-code.md` and the project wiki `Extended Functions` page must describe the new behavior.
- **Not impacted**: `RESOURCESIZE(n)`, `usr0`/`usr1`/`USR0`/`USR1`, resource map format/numbering.
- **Tests and regression**:
  - `GRAPH/test38a` is volatile and has no probes. Add `vram.probes` at `0x0000` with the first bytes of `NYANCAT.SC3` (`FE 00 00 FF 05 00 00 44 ...`); probes are evaluated even for volatile programs, so this validates the RAM-copy path without regenerating the baseline.
  - Add a deterministic regression program for `RESOURCE` valid, invalid (`0`), and (if a suitable large fixture is available) truncation; add a manifest entry and generate its baseline.
  - Run `MISC/rsc1` to confirm the shared `cmd_rsctoram` path did not regress.
  - Run `make test-emulator` because `RESOURCE` moves from fixed `usr0` (`32_usr`) to fixed `cmd_rsctoram` (`31-cmd-game-graphics`).
  - `make test-integration` remains the 4-mapper compilation matrix.
