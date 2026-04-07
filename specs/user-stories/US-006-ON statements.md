# US-006 - Multi-branch and event-driven control flow with ON statements

## Story

As an MSX-BASIC developer, I need to use the ON statement with branching and event-driven variants so that I can implement indexed jumps and reactive behaviors based on runtime conditions, input devices, and system events.

## Acceptance Criteria

- The compiler supports indexed branching using:
ON <expr> GOTO
ON <expr> GOSUB
- The compiler supports event-driven constructs:
ON INTERVAL GOSUB
ON SPRITE GOSUB
ON STICK GOSUB
ON KEY GOSUB
ON STOP GOSUB
ON ERROR GOTO
- All referenced line numbers are resolved as symbols and validated during compilation.
- Event handlers are registered and triggered via runtime trap/polling mechanism.
- GOSUB handlers preserve return address and support nested calls.
- ON ERROR performs a non-returning jump (GOTO semantics).
- Event flags are reset after handling to avoid repeated triggering.
- Invalid syntax or missing line references generate compilation errors.
- Indexed ON ignores out-of-range values without raising runtime errors (MSX-compatible behavior).

---

## BDD Scenarios

### Scenario 1: Indexed ON GOTO

Given ON X GOTO 100,200,300
When X = 2
Then execution jumps to line 200

### Scenario 2: Indexed ON GOSUB

Given ON X GOSUB 400,500
When X = 1
Then subroutine at line 400 is executed
And returns after RETURN

### Scenario 3: ON INTERVAL trigger

Given ON INTERVAL GOSUB 1000
When the interval timer expires
Then subroutine at line 1000 is executed

### Scenario 4: ON SPRITE collision

Given ON SPRITE GOSUB 2000
When a sprite collision occurs
Then subroutine at line 2000 is executed

### Scenario 5: ON STICK input

Given ON STICK GOSUB 3000
When joystick input is detected
Then subroutine at line 3000 is executed

### Scenario 6: ON KEY press

Given ON KEY GOSUB 4000
When a key is pressed
Then subroutine at line 4000 is executed

### Scenario 7: ON STOP event

Given ON STOP GOSUB 5000
When STOP key is pressed
Then subroutine at line 5000 is executed

### Scenario 8: ON ERROR handling

Given ON ERROR GOTO 9000
When a runtime error occurs
Then execution jumps to line 9000

### Scenario 9: Invalid line reference

Given ON SPRITE GOSUB 9999
And line 9999 does not exist
When compiling
Then a compilation error is raised

---

## Technical Specification

### Syntax Variants

```
ON <expression> GOTO <line1>, <line2>, ..., <lineN>
ON <expression> GOSUB <line1>, <line2>, ..., <lineN>

ON INTERVAL GOSUB <line>
ON SPRITE GOSUB <line>
ON STICK GOSUB <line>
ON KEY GOSUB <line>
ON STOP GOSUB <line>
ON ERROR GOTO <line>
```

### Indexed Branching

- Expression evaluated via evalExpression()
- Result stored in HL
- Converted to 1-based index

Targets:
Registered via SymbolNode
Patched via FixNode

- Code generation:

Sequential compare/jump chain or jump table

### Expression Evaluation

- The expression after ON is evaluated using evalExpression().
- Result is expected in HL register (numeric subtype).
- Value is treated as integer index (1-based).

### Event-Driven Model

All event-based ON variants:

- Register handler address (symbol)
- Enable trap system via:

`addCheckTraps();`

- Rely on runtime polling inserted between statements.
- Ensures runtime polling or interrupt-like behavior.
- Implemented as injected checks between statements.

### Runtime Trap Mechanism

- Implemented via injected calls (`addCheckTraps()`)
- Executed between statements

```
Typical flow:
call CHECK_EVENT
jr z, continue
call HANDLER
```

### Error Handling

ON ERROR not implemented yet.

---

## References

- [ON..GOTO](https://www.msx.org/wiki/ON...GOTO)
- [ON..GOSUB](https://www.msx.org/wiki/ON...GOSUB)
- [ON INTERVAL](https://www.msx.org/wiki/ON_INTERVAL_GOSUB)
- [ON SPRITE](https://www.msx.org/wiki/ON_SPRITE_GOSUB)
- [ON ERROR](https://www.msx.org/wiki/ON_ERROR_GOTO)
- [ON STOP](msx.org/wiki/ON_STOP_GOSUB)
- [ON KEY](https://www.msx.org/wiki/ON_KEY_GOSUB)
