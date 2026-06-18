## Context

New tile color and pattern commands to match sprite counterparts. These are distinct from sprint 4 which improves existing syntax.

## Goals / Non-Goals

**Goals:**
- Implement SET/GET TILE COLOR
- Implement SET/GET TILE PATTERN

## Decisions

### Decision 1: Buffer-based API
Follow the same buffer array pattern as SET/GET SPRITE COLOR and SPRITE PATTERN.

## Risks / Trade-offs

- Requires integer buffer arrays for color/pattern data transfer.
