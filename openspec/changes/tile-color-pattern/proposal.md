## Why

Add SET/GET TILE COLOR and SET/GET TILE PATTERN as new commands, completing the tile manipulation API to match the existing SPRITE COLOR/PATTERN commands.

## What Changes

- Implement SET TILE COLOR command
- Implement GET TILE COLOR command
- Implement SET TILE PATTERN command (new — distinct from syntax improvement in sprint 4)
- Implement GET TILE PATTERN command (new — distinct from syntax improvement in sprint 4)

## Capabilities

### New Capabilities
- *(none — extends tile functionality)*

### Modified Capabilities
- *(none)*

## Impact

- `src/application/compiler/` — new color/pattern strategies
- `src/application/parser/` — new command parsing
- Release 1.3.0.0
