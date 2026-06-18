## ADDED Requirements

### Requirement: Support sprite management commands and functions
The system SHALL support SPRITE ON/OFF/STOP/LOAD, PUT SPRITE, SPRITE$ assignment, COLOR SPRITE, SET/GET SPRITE PATTERN/COLOR/FLIP/ROTATE, and COLLISION() function so that developers can manage sprite assets on screen.

#### Scenario: Enable and disable sprite collision trap
- **WHEN** ON SPRITE GOSUB 2000 is active and SPRITE ON is executed
- **THEN** the sprite trap is enabled for collision handling
- **WHEN** SPRITE OFF is executed
- **THEN** the sprite trap is disabled

#### Scenario: Stop sprite collision trap
- **WHEN** SPRITE ON was previously executed and SPRITE STOP is executed
- **THEN** the sprite trap is stopped without clearing its handler

#### Scenario: Load Tiny Sprite resource
- **WHEN** FILE "sprite1.spr" and SPRITE LOAD 0 are executed
- **THEN** sprite resource 0 is loaded into the sprite tables

#### Scenario: Define sprite pattern via SPRITE$
- **WHEN** SPRITE$(0) = A$ is executed with a valid pattern string
- **THEN** sprite pattern 0 is updated with the string data

#### Scenario: Set sprite color via COLOR SPRITE
- **WHEN** COLOR SPRITE(1) = 33 is executed
- **THEN** the color attribute of sprite 1 is updated

#### Scenario: Put sprite with coordinates
- **WHEN** PUT SPRITE 0, (100, 100) is executed with assigned pattern data
- **THEN** the sprite appears at the given coordinates

#### Scenario: Put sprite using STEP coordinates
- **WHEN** PUT SPRITE 0, STEP(5, -2) is executed after a previous position was set
- **THEN** the sprite moves relative to the last position

#### Scenario: Read and write sprite pattern buffer
- **WHEN** GET SPRITE PATTERN 0, PB% is executed followed by SET SPRITE PATTERN 1, PB%
- **THEN** sprite pattern 1 matches sprite pattern 0

#### Scenario: Read and write sprite color buffer
- **WHEN** GET SPRITE COLOR 0, CB% is executed followed by SET SPRITE COLOR 1, CB%
- **THEN** sprite color for sprite 1 matches sprite 0

#### Scenario: Flip sprite pattern
- **WHEN** SET SPRITE FLIP 0, 1 is executed
- **THEN** the sprite pattern is flipped vertically

#### Scenario: Rotate sprite pattern
- **WHEN** SET SPRITE ROTATE 0, 2 is executed
- **THEN** the sprite pattern is rotated 180 degrees

#### Scenario: Detect sprite collisions
- **WHEN** COLLISION() is evaluated with multiple sprites on screen
- **THEN** it returns the collided sprite number or -1 when none collide
- **WHEN** COLLISION(2) is evaluated
- **THEN** it returns the collided sprite number for sprite 2 or -1
- **WHEN** COLLISION(4, 5) is evaluated
- **THEN** it returns 5 if sprite 4 collided with sprite 5, otherwise -1
