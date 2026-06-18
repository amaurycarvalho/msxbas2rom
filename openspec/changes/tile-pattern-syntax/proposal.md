## Why

The existing SET/GET TILE PATTERN syntax is cumbersome and inconsistent with other sprite/tile commands. Improve the syntax to match the patterns established by SET/GET SPRITE PATTERN commands.

## What Changes

- Improve SET TILE PATTERN syntax for clarity and consistency
- Improve GET TILE PATTERN syntax for clarity and consistency
- Ensure tile pattern operations work across all screen modes

## Capabilities

### New Capabilities
- *(none — modifies existing tile functionality)*

### Modified Capabilities
- *(none — tile patterns not yet a formal capability spec)*

## Impact

- `src/application/compiler/statements/strategies/` — tile pattern statement strategies
- `src/application/compiler/functions/strategies/` — tile pattern function strategies
- Release 1.1.0.0
