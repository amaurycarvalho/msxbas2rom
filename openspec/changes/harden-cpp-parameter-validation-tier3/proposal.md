# Proposal

## Why

Tier 1 and Tier 2 fix concrete parameter-loss bugs and add targeted validation. Tier 3 addresses the systemic cause: command/function parameters are evaluated with broad implicit conversions, fixed-table indices use ad-hoc checks, and unsupported statements do not have a single validation policy. Without this, new handlers can repeat the same failures.

## What Changes

- Introduce a strict type policy for command parameters: numeric-only command parameters reject string expressions instead of silently routing them through `VAL`; string-only parameters reject numeric expressions unless an explicit conversion is documented.
- Apply the policy to the compiler's statement and function strategies with representative migrations, leaving expression operators and `LET` conversion semantics unchanged.
- Define and apply a complete fixed-table index policy for sprite, tile, KEY, STRIG, and resource indices, including literal, variable, and sentinel forms such as `COLLISION(-1)`.
- Replace the ad-hoc "no-op" behavior for unsupported statements with a documented policy: either implement the parameter contract or reject it with a clear syntax error.
- Document the parameter contracts and compatibility notes in the affected specs and user-facing documentation.
- Add exhaustive, table-driven unit tests for every migrated command/function and integration/CLI negative tests for representative failures.

## Capabilities

### New Capabilities

- _(none)_

### Modified Capabilities

- `compiler`: define strict command/function parameter typing and a unified fixed-table index policy.

## Impact

- Broad but mechanical changes across C++ statement/function strategies and their tests.
- Potential compatibility break for programs relying on implicit string-to-number command parameters.
- No kernel changes required for compile-time rejections; runtime variable validation remains in the Z80 guard changes.
- User-facing documentation and spec scenarios must be updated with the compatibility notes.
