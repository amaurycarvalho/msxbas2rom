# Proposal

## Why

Tier 1 and Tier 2 add guards per command. Tier 3 makes the policy durable: a shared kernel guard layer prevents new handlers from repeating the same failures, and malformed-resource/performance regression coverage keeps the guards honest in a size-constrained 16 KB kernel.

## What Changes

- Introduce shared kernel guard helpers/macros for parameter count, fixed-table index normalization, resource open/size validation, and safe no-op/failure returns.
- Migrate remaining command handlers to the shared policy, removing inconsistent per-handler behavior.
- Define the allowed failure policy for dynamic invalid values: documented no-op, clamp, or error result; never table overrun, truncated execution, or infinite loop.
- Add a malformed-resource regression suite covering MTF maps, `.SPR` files, fonts, RUNASM, and resource numbers.
- Add a performance and size budget check for the guard layer so the fixed kernel page and hot paths remain within established limits.
- Document runtime parameter contracts and safe-failure behavior in the kernel specs and user-facing documentation.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `kernel-parameter-guards`: define the shared guard layer, uniform failure policy, and resource-size/malformed-input contract for all kernel command handlers.
- `emulator-regression-tests`: require malformed-resource scenarios and probe-based validation for the guard layer.

## Impact

- Kernel assembly modules across all command families.
- Kernel size and placement checks; possible need to reduce filler or move modules.
- Regression fixtures and manifest scenarios for malformed resources.
- No compiler changes except documentation/tests that exercise the runtime guards.
