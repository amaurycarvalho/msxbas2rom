# Design

## Context

See `proposal.md` - Why. The function strategies currently do:

```cpp
cpu.addLdA(5);
cpu.addCall(context->getCodeOptimizer()->getKernelCallAddr(DISP_usr2) + 1);
```

That emits a direct absolute call to `USR2 + 1`, skipping `ld a,l` and relying
on `A` for the selector. `CMD TURBO` and other kernel calls use
`addKernelCall(DISP_...)`, which resolves through the dispatch stub that maps and
restores the kernel bank. A local probe compiled `TURBO()` to `call 0x0B049`;
execution stopped there, while `MSX()` (inline) and `CMD TURBO` (dispatch) worked.
`VDP()` and `MAKER()` use the same direct-call pattern.

## Goals / Non-Goals

**Goals:**

- Make `TURBO()`, `VDP()`, and `MAKER()` execute through the supported kernel
  call path.
- Preserve the numeric return value and MSX1 behavior.
- Add runtime coverage that would have caught the hang.

**Non-Goals:**

- Redesigning the USR2 dispatch table.
- Adding turbo R hardware features beyond the existing commands/functions.
- Changing BASIC syntax.

## Decisions

### D1: Use `addKernelCall(DISP_usr2)` with the selector in HL

Replace the direct call with:

```cpp
cpu.addLdHL(5);  // selector: TURBO=5, VDP=4, MAKER=6
context->getCodeOptimizer()->addKernelCall(DISP_usr2);
```

The dispatch stub maps the kernel bank and calls `usr2`, whose first instruction
reads `L`. This removes the `+1` hack and the direct absolute address.

### D2: Apply the same fix to TURBO, VDP, and MAKER

All three strategies share the defect. Fixing only `TURBO()` would leave
`VDP()`/`MAKER()` broken and `MISC/test64` unreliable.

### D3: Alternative considered: dedicated dispatch entries

Adding `usr2_turbo`, `usr2_vdp`, and `usr2_maker` entries would be clearer but
requires kernel table/wrapper changes and increases the kernel surface. The
existing `DISP_usr2` entry plus an HL selector is smaller and preserves the
current dispatcher contract.

### D4: Runtime regression coverage

Add a small program that prints `TURBO()`, `VDP()`, and `MAKER()` values and
continues to an `END`/input wait. Run it on MSX1, MSX2+, and turbo R. The MSX1
run verifies `TURBO()` returns 0; the MSX2+/turbo R runs verify execution
continues.

## Risks / Trade-offs

- **[Risk] The dispatch stub clobbers HL or DE** -> The existing kernel-call
  contract preserves the registers the caller needs; add a runtime test that
  reads the returned value.
- **[Risk] `usr2` expects a different register convention** -> It reads `L`;
  the new sequence sets HL explicitly, matching the routine.
- **[Risk] MSX1 behavior changes** -> Keep the selector and let `usr2_turbo`
  return 0 on MSX1; verify with a runtime test.

## Migration Plan

- Compiler-only change; no kernel table changes required.
- Rebuild and run the unit/function-size tests and the new runtime test.
- Roll back by restoring the direct call, but keep the runtime test as a
  regression guard.
