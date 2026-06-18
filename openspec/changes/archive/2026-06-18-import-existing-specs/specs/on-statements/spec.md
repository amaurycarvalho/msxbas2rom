## ADDED Requirements

### Requirement: Support multi-branch and event-driven control flow with ON statements
As an MSX-BASIC developer, the system SHALL support ON statement variants for indexed branching and event-driven control flow so that developers can implement indexed jumps and reactive behaviors based on runtime conditions, input devices, and system events.

The compiler SHALL support indexed branching using `ON <expr> GOTO` and `ON <expr> GOSUB`. The compiler SHALL support event-driven constructs: ON INTERVAL GOSUB, ON SPRITE GOSUB, ON STRIG GOSUB, ON KEY GOSUB, ON STOP GOSUB, ON ERROR GOTO. All referenced line numbers SHALL be resolved as symbols and validated during compilation. Event handlers SHALL be registered and triggered via runtime trap/polling mechanism. GOSUB handlers SHALL preserve return address and support nested calls. ON ERROR SHALL perform a non-returning jump (GOTO semantics). Event flags SHALL be reset after handling to avoid repeated triggering. Invalid syntax or missing line references SHALL generate compilation errors. Indexed ON SHALL ignore out-of-range values without raising runtime errors (MSX-compatible behavior).

#### Scenario: Indexed ON GOTO
- **WHEN** `ON X GOTO 100,200,300` and X = 2
- **THEN** execution jumps to line 200

#### Scenario: Indexed ON GOSUB
- **WHEN** `ON X GOSUB 400,500` and X = 1
- **THEN** subroutine at line 400 is executed
- **AND** returns after RETURN

#### Scenario: ON INTERVAL trigger
- **WHEN** ON INTERVAL GOSUB 1000 is active and interval timer expires
- **THEN** subroutine at line 1000 is executed

#### Scenario: ON SPRITE collision
- **WHEN** ON SPRITE GOSUB 2000 is active and a sprite collision occurs
- **THEN** subroutine at line 2000 is executed

#### Scenario: ON STRIG input
- **WHEN** ON STRIG GOSUB 1000,1001,1002,1003,1004 is active and STRIG(0)..STRIG(4) ON, and spacebar is pressed
- **THEN** subroutine at line 1000 is executed
- **AND** each joystick button maps to its corresponding handler

#### Scenario: ON KEY press
- **WHEN** ON KEY GOSUB 4000 is active and a key is pressed
- **THEN** subroutine at line 4000 is executed

#### Scenario: ON STOP event
- **WHEN** ON STOP GOSUB 5000 and STOP ON are active and STOP key is pressed
- **THEN** subroutine at line 5000 is executed

#### Scenario: ON ERROR handling
- **WHEN** ON ERROR GOTO 9000 is active and a runtime error occurs
- **THEN** execution jumps to line 9000

#### Scenario: Invalid line reference in ON statement
- **WHEN** ON SPRITE GOSUB 9999 is compiled and line 9999 does not exist
- **THEN** a compilation error is raised

### Technical Specification — ON Statement Syntax

```
ON <expression> GOTO <line1>, <line2>, ..., <lineN>
ON <expression> GOSUB <line1>, <line2>, ..., <lineN>

ON INTERVAL GOSUB <line>
ON SPRITE GOSUB <line>
ON STRIG GOSUB <line>
ON KEY GOSUB <line>
ON STOP GOSUB <line>
ON ERROR GOTO <line>
```

**Indexed Branching:**
- Expression evaluated via `evalExpression()`, result in HL register
- Converted to 1-based index; targets registered via SymbolNode, patched via FixNode
- Code generation uses sequential compare/jump chain or jump table

**Event-Driven Model:**
- All event variants register handler address (symbol) and enable trap system via `addCheckTraps()`
- Rely on runtime polling injected between statements

**Runtime Trap Mechanism:**
- Implemented via injected calls (`addCheckTraps()`) executed between statements
- Typical flow: `call CHECK_EVENT` / `jr z, continue` / `call HANDLER`

**Implementation Notes:**
- ON ERROR not implemented yet
- STOP statement works like END

### References
- [ON..GOTO](https://www.msx.org/wiki/ON...GOTO)
- [ON..GOSUB](https://www.msx.org/wiki/ON...GOSUB)
- [ON INTERVAL](https://www.msx.org/wiki/ON_INTERVAL_GOSUB)
- [ON SPRITE](https://www.msx.org/wiki/ON_SPRITE_GOSUB)
- [ON ERROR](https://www.msx.org/wiki/ON_ERROR_GOTO)
- [ON STOP](https://www.msx.org/wiki/ON_STOP_GOSUB)
- [ON KEY](https://www.msx.org/wiki/ON_KEY_GOSUB)
- [ON STRIG](https://www.msx.org/wiki/STRIG())
