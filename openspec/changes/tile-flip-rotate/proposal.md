## Why

Add SET TILE FLIP and SET TILE ROTATE commands, mirroring the existing SET SPRITE FLIP/ROTATE functionality, to provide complete tile manipulation capabilities.

## What Changes

- Implement SET TILE FLIP command (horizontal, vertical, both)
- Implement SET TILE ROTATE command (left, right, 180 degrees)

## Capabilities

### New Capabilities
- *(none — extends tile functionality)*

### Modified Capabilities
- *(none)*

## Impact

- `src/application/compiler/statements/strategies/` — new flip/rotate strategies
- `src/application/parser/` — new command parsing rules
- `src/infrastructure/kernel/asm/` — new runtime routines if needed
- Release 1.2.0.0
