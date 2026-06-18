## ADDED Requirements

### Requirement: Support sprite management commands and functions
As an MSX-BASIC developer, the system SHALL support SPRITE ON, SPRITE OFF, SPRITE STOP, SPRITE LOAD, PUT SPRITE, SPRITE$ assignment, COLOR SPRITE, COLOR SPRITE$, SET/GET SPRITE PATTERN, SET/GET SPRITE COLOR, SET SPRITE FLIP, SET SPRITE ROTATE, and COLLISION() function so that developers can manage sprite assets on screen.

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

#### Scenario: Set sprite color definition via COLOR SPRITE$
- **WHEN** COLOR SPRITE$(0) = CHR$(12)+CHR$(10) is executed
- **THEN** the sprite color definition is updated from the string data

#### Scenario: Put sprite with coordinates
- **WHEN** PUT SPRITE 0, (100, 100) is executed with assigned pattern data
- **THEN** the sprite appears at the given coordinates

#### Scenario: Put sprite with color and pattern
- **WHEN** PUT SPRITE 0, (100, 100), 15, 2 is executed
- **THEN** the sprite is rendered using color 15 and pattern 2

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

### Technical Specification — Sprites

**Tiny Sprite Support:**
- Syntax: `SPRITE LOAD <resource number>` with `.SPR` files in plain text format
- Limited to 64 unique 16x16 shapes per file; first 32 shapes auto-assigned to Sprite Attribute Table
- Use with sprite parameter size 2 or 3 on SCREEN statement

Example program:
```basic
FILE "sprite1.spr"   ' resource 0: msx1 sprite set
FILE "sprite2.spr"   ' resource 1: msx2 sprite set

10 SCREEN 2, 2, 0    ' screen mode 2 (msx1)
20 SPRITE LOAD 0     ' load resource 0 (msx1 sprite set)
30 GOSUB 100         ' show sprite on screen

50 SCREEN 7, 2, 0    ' screen mode 7 (msx2)
60 SPRITE LOAD 1     ' load resource 1 (msx2 sprite set)
70 GOSUB 100         ' show sprite on screen

90 SCREEN 0
91 END

100 COLOR 15,4,0
101 CLS
102 PUT SPRITE 0, (100,100)
103 PUT SPRITE 1, (100,100)
104 PUT SPRITE 2, (100,100)
105 A$ = INPUT$(1)
106 RETURN
```

**Sprite Collision Detection Functions:**
```
COLLISION()     → returns collided sprite number or -1
COLLISION(<n>)  → returns sprite that <n> collided with, or -1
COLLISION(<n1>,<n2>) → returns n2 if n1 collided with n2, else -1
```
- Sprites with same X and Y position are considered same object (no collision)

Usage examples:
```basic
' Beep if any sprite collided with each other:
SN% = COLLISION(-1)
IF SN% >= 0 THEN BEEP

' Beep if any sprite collided with sprite 2:
SN% = COLLISION(2)
IF SN% >= 0 THEN BEEP

' Beep if sprite 4 collided with sprite 5:
SN% = COLLISION(5)
IF SN% = 4 THEN BEEP

' Direct test: beep if sprite 5 collided with sprite 4:
SN% = COLLISION(5, 4)
IF SN% >= 0 THEN BEEP

' Direct test: beep if sprite 0 collided with sprite 1:
SN% = COLLISION(0, 1)
IF SN% >= 0 THEN BEEP
```

**Sprite Extended Commands:**
- `SET SPRITE COLOR <n>, <array>` / `GET SPRITE COLOR <n>, <array>` (8-integer buffer)
- `SET SPRITE PATTERN <n>, <array>` / `GET SPRITE PATTERN <n>, <array>` (16-integer buffer)
- `SET SPRITE FLIP <n>, <dir>` where dir: 0=horizontal, 1=vertical, 2=both
- `SET SPRITE ROTATE <n>, <dir>` where dir: 0=left, 1=right, 2=180 degrees

### References
- [SPRITE$()](<https://www.msx.org/wiki/SPRITE$()>)
- [COLOR SPRITE()](<https://www.msx.org/wiki/COLOR_SPRITE()>)
- [COLOR SPRITE$()](<https://www.msx.org/wiki/COLOR_SPRITE$()>)
- [PUT SPRITE](https://www.msx.org/wiki/PUT_SPRITE)
- [Tiny Sprite Support](https://github.com/amaurycarvalho/msxbas2rom/wiki/TS-Support)
- [Sprite Extended Commands](https://github.com/amaurycarvalho/msxbas2rom/wiki/Extended-Commands#sprite-extended-commands)
- [Sprite Collision Detection Functions](https://github.com/amaurycarvalho/msxbas2rom/wiki/Extended-Functions#sprite-collision-detection-functions)
