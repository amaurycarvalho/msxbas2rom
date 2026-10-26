# Proposal

## Why

`TURBO()` hangs at runtime on MSX2+ and turbo R. `VDP()` and `MAKER()` use the
same USR2 path and share the defect. `MSX()` and `CMD TURBO` work, so the
failure is specific to the compiler's direct call into the USR2 dispatcher, not
the machines or the BASIC feature.

## What Changes

- Route `TURBO()`, `VDP()`, and `MAKER()` through the kernel dispatch mechanism
  instead of a direct absolute call to `USR2 + 1`.
- Pass the function selector in `HL` so the `USR2` dispatcher receives the
  intended function number.
- Add runtime regressions proving `TURBO()`, `VDP()`, and `MAKER()` return
  numeric values on MSX2+ and turbo R without hanging.
- Keep MSX1 behavior: `TURBO()` returns 0 and `CMD TURBO` remains a no-op.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `compiler`: USR2-backed functions must be emitted through kernel dispatch
  routing rather than direct resolved calls.

## Impact

- `src/application/compiler/functions/strategies/basic/turbo/compiler_turbo_function_strategy.cpp`
- `src/application/compiler/functions/strategies/basic/vdp/compiler_vdp_function_strategy.cpp`
- `src/application/compiler/functions/strategies/basic/maker/compiler_maker_function_strategy.cpp`
- `tests/unit/src/test_compiler_functions.cpp`
- `tests/integration/MISC/` or a new turbo regression scenario
- `openspec/specs/compiler/spec.md`
- Depends on: none.
- Enables: `add-turbo-r-regression-coverage` for `MISC/test1` and `MISC/test64`.
