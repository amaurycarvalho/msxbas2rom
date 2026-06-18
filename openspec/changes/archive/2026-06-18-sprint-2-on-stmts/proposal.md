## Why

Fix event-driven ON statement boot issues (issue #18) and refactor ON statement handling. Programs using ON STRIG, ON SPRITE, etc. were not loading correctly after boot, stuck in a limbo between BASIC and ROM initialization.

## What Changes

- Fix ON statement boot initialization order
- Refactor ON statement handling in compiler
- Add event-driven control flow support (ON INTERVAL, ON SPRITE, ON STRIG, ON KEY, ON STOP, ON ERROR)

## Capabilities

### New Capabilities
- `on-statements`: Multi-branch and event-driven control flow

### Modified Capabilities
- *(none)*

## Impact

- `src/application/compiler/` — ON statement compilation fixes
- `src/infrastructure/kernel/asm/` — runtime trap mechanism fixes
- `specs/user-stories/US-006` — new user story for ON statements
- Release 0.3.4.0
