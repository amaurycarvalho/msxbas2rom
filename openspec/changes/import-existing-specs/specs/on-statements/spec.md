## ADDED Requirements

### Requirement: Support multi-branch and event-driven control flow with ON statements
The system SHALL support ON statement variants for indexed branching and event-driven control flow so that developers can implement indexed jumps and reactive behaviors based on runtime conditions, input devices, and system events.

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
