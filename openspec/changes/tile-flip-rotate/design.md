## Context

SET SPRITE FLIP/ROTATE already exist. Tile versions should follow the same pattern.

## Goals / Non-Goals

**Goals:**
- Implement SET TILE FLIP with dir: 0=horizontal, 1=vertical, 2=both
- Implement SET TILE ROTATE with dir: 0=left, 1=right, 2=180

## Decisions

### Decision 1: Mirror SPRITE FLIP/ROTATE exactly
Same parameter structure, same underlying VDP operations.

## Risks / Trade-offs

- Minimal — reuses proven sprite flip/rotate patterns.
