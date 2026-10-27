# Spec Delta

## ADDED Requirements

### Requirement: Regression scenarios cover malformed kernel inputs

The emulator regression workflow SHALL include volatile scenarios that exercise malformed MTF maps, truncated `.SPR` resources, short fonts, invalid RUNASM resources, and out-of-range dynamic sprite/tile/direction values. These scenarios SHALL use RAM/VRAM probes or canaries and a bounded fallback time so infinite loops fail the run.

#### Scenario: Malformed MTF map is bounded
- **WHEN** the regression scenario executes a map with zero width/height
- **THEN** the program reaches its termination marker within the configured fallback time
- **AND** the configured memory probes pass

#### Scenario: Sprite table canary survives invalid sprite index
- **WHEN** the regression scenario executes `PUT SPRITE` or `SET SPRITE HITBOX` with an invalid sprite index
- **THEN** the canary bytes adjacent to the sprite tables are unchanged

#### Scenario: RUNASM oversized resource is not executed
- **WHEN** the regression scenario selects a resource larger than 255 bytes for `CMD RUNASM`
- **THEN** execution returns to the BASIC marker without entering the truncated routine
