## Context

ON statements (ON STRIG, ON SPRITE, etc.) had boot initialization issues. The runtime trap mechanism needed fixes to properly register event handlers before BASIC initialization completes.

## Goals / Non-Goals

**Goals:**
- Fix ON statement boot order
- Implement all ON statement variants

## Decisions

### Decision 1: Runtime trap polling
Event handlers implemented via `addCheckTraps()` calls injected between statements.

## Risks / Trade-offs

- Boot order fix required changes to kernel assembly initialization sequence.
