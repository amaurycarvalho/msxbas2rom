# Sprites Handling

## Purpose

Defines the contract for sprite handling in compiled MSX-BASIC programs: sprite activation and collision traps, sprite pattern and color definition, `PUT SPRITE` placement, Tiny Sprite resources, and the `COLLISION()` function.

## Requirements

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

### Requirement: SET SPRITE HITBOX configures per-sprite collision hitboxes

The system SHALL support the `SET SPRITE HITBOX` command to configure collision hitboxes independently per sprite. `SET SPRITE HITBOX ON` SHALL initialize all 32 sprites with zero margins (full rectangle). `SET SPRITE HITBOX OFF` SHALL disable collision for all 32 sprites. `SET SPRITE HITBOX <sprite>` and `SET SPRITE HITBOX <sprite> ON` SHALL reset the specified sprite to zero margins (enabled). `SET SPRITE HITBOX <sprite> OFF` SHALL disable collision for the specified sprite. `SET SPRITE HITBOX <sprite>, <left>, <top>, <right>, <bottom>` SHALL set explicit margins for the specified sprite, with omitted trailing margins defaulting to zero.

#### Scenario: ON initializes full hitboxes
- **WHEN** `SET SPRITE HITBOX ON` is executed
- **THEN** all 32 sprites use a full-rectangle collision hitbox (zero margins)

#### Scenario: OFF disables all collisions
- **WHEN** `SET SPRITE HITBOX OFF` is executed
- **THEN** no sprite collides with any other sprite

#### Scenario: Reset a single sprite
- **WHEN** `SET SPRITE HITBOX 3` is executed after sprite 3 had custom margins
- **THEN** sprite 3's hitbox returns to a full rectangle

#### Scenario: Enable a single sprite
- **WHEN** `SET SPRITE HITBOX 3 ON` is executed after sprite 3 was disabled
- **THEN** sprite 3 is enabled with zero margins (full rectangle)

#### Scenario: Disable a single sprite
- **WHEN** `SET SPRITE HITBOX 3 OFF` is executed
- **THEN** sprite 3 no longer participates in collision detection, while other sprites remain unaffected

#### Scenario: Explicit margins
- **WHEN** `SET SPRITE HITBOX 3, 2, 3, 2, 4` is executed
- **THEN** sprite 3's collision rectangle is inset by LEFT=2, TOP=3, RIGHT=2, BOTTOM=4 pixels

#### Scenario: Omitted trailing margins default to zero
- **WHEN** `SET SPRITE HITBOX 3, 2` is executed
- **THEN** sprite 3 has LEFT=2 and TOP=RIGHT=BOTTOM=0

### Requirement: Hitbox margins shrink the collision rectangle

Margins SHALL be expressed in pixels and SHALL reduce the sprite collision rectangle. The system SHALL enforce `0 <= margin <= SPRSIZ` and `LEFT+RIGHT <= SPRSIZ`, `TOP+BOTTOM <= SPRSIZ` for the current effective sprite size. Invalid values SHALL be clamped to the valid range so the collision rectangle never inverts. A zero-width or zero-height collision area SHALL be valid and SHALL represent a non-colliding sprite.

#### Scenario: Shrinking sprite A removes a collision
- **WHEN** two 16x16 sprites overlap by one pixel and both are given 2-pixel margins on all sides
- **THEN** the collision is no longer reported

#### Scenario: Zero-width hitbox is valid
- **WHEN** `SET SPRITE HITBOX 1, 8, 0, 8, 0` is executed on a 16x16 sprite
- **THEN** the sprite has a zero-width collision area and does not collide

#### Scenario: Out-of-range margins are clamped
- **WHEN** `SET SPRITE HITBOX 1, 20, 5, 0, 0` is executed on a 16x16 sprite
- **THEN** the margins are clamped so the resulting collision rectangle does not invert

#### Scenario: Horizontal margins shrink the horizontal extent
- **WHEN** two 16x16 sprites moving horizontally toward each other have `LEFT`/`RIGHT` margins
- **THEN** the collision is reported only when their shrunken horizontal rectangles overlap, using the sprite's real X axis

#### Scenario: Vertical margins do not affect a horizontal approach
- **WHEN** two 16x16 sprites at the same Y move horizontally toward each other and have only `TOP`/`BOTTOM` margins
- **THEN** the horizontal collision distance is unchanged from the zero-margin case

### Requirement: Collision uses the sprite's real X and Y axes

The collision test SHALL combine each sprite's real X position with its relative X bounds and its real Y position with its relative Y bounds. A candidate sprite's vertical bounds SHALL be read from its `y0` field, never from `x1`.

#### Scenario: Full-rectangle collision distance matches the sprite size
- **WHEN** two 16x16 sprites with default hitboxes approach horizontally
- **THEN** the collision is reported when their 16-pixel-wide rectangles overlap, not at an offset distance

### Requirement: Hitboxes persist until explicitly changed

Hitbox configuration SHALL be persistent. Moving a sprite with `PUT SPRITE` SHALL NOT alter its hitbox. Hitbox configuration SHALL remain in effect until another `SET SPRITE HITBOX` command or a sprite clear/size change.

#### Scenario: Hitbox survives sprite movement
- **WHEN** a sprite with custom margins is moved with `PUT SPRITE`
- **THEN** its collision hitbox remains the same relative to the sprite origin

#### Scenario: Hitbox persists across collision checks
- **WHEN** `COLLISION()` is evaluated repeatedly after `SET SPRITE HITBOX`
- **THEN** the configured hitbox is used consistently

### Requirement: SET SPRITE HITBOX AUTO derives hitboxes from sprite patterns

`SET SPRITE HITBOX AUTO` SHALL compute, once, the bounding rectangle of the visible pixels of each sprite's VRAM pattern, where a pixel is visible iff its pattern bit is set. `SET SPRITE HITBOX <sprite> AUTO` SHALL do the same for the specified sprite only. Fully transparent sprites SHALL be disabled (never collide). `AUTO` SHALL NOT maintain a dynamic relationship with VRAM.

#### Scenario: AUTO computes a bounding box
- **WHEN** `SET SPRITE HITBOX AUTO` is executed on a sprite whose visible pixels form a centered diamond
- **THEN** the sprite's hitbox is the tightest rectangle containing those pixels

#### Scenario: AUTO disables transparent sprites
- **WHEN** `SET SPRITE HITBOX AUTO` is executed on a fully transparent sprite
- **THEN** that sprite never participates in a collision

#### Scenario: AUTO for a single sprite
- **WHEN** `SET SPRITE HITBOX 3 AUTO` is executed
- **THEN** only sprite 3's hitbox is derived from its pattern; other sprites keep their configured hitboxes

#### Scenario: AUTO is one-shot
- **WHEN** `SET SPRITE HITBOX AUTO` is executed and the sprite pattern is later changed
- **THEN** the hitbox is not recalculated until `AUTO` is executed again

### Requirement: COLLISION() uses effective hitboxes with unchanged API

`COLLISION()`, `COLLISION(n)`, and `COLLISION(n1,n2)` SHALL keep their existing interface and result format. The collision test SHALL use the effective hitbox (sprite position combined with the configured relative bounds). Collision detection SHALL NOT read sprite pattern data from VRAM. The existing behavior for sprites at the same X/Y position SHALL be preserved and SHALL be evaluated independently of hitbox margins.

#### Scenario: Disabled sprite never collides
- **WHEN** a sprite with a disabled hitbox overlaps another active sprite
- **THEN** `COLLISION()` does not report them as colliding

#### Scenario: Shrinking sprite B removes a collision
- **WHEN** sprite B is given large margins so its effective rectangle no longer overlaps sprite A
- **THEN** `COLLISION()` does not report a collision

#### Scenario: Existing COLLISION forms work unchanged
- **WHEN** `COLLISION()`, `COLLISION(n)`, and `COLLISION(n1,n2)` are evaluated
- **THEN** each returns the collided sprite number or -1 as before

### Requirement: Default hitbox behavior preserves existing programs

When no `SET SPRITE HITBOX` command is executed, every sprite SHALL use a full-rectangle hitbox equivalent to the previous behavior. Sprite clear/size changes SHALL reset hitboxes to this default state.

#### Scenario: No hitbox command gives full rectangle
- **WHEN** a program never executes `SET SPRITE HITBOX`
- **THEN** collisions behave exactly as before this change (full sprite rectangle)

#### Scenario: Sprite size change resets hitboxes
- **WHEN** a sprite has custom margins and the sprite size is changed via `SCREEN`
- **THEN** the sprite's hitbox is reset to the default full rectangle

## Technical Specification — Sprites

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

## References
- [SPRITE$()](<https://www.msx.org/wiki/SPRITE$()>)
- [COLOR SPRITE()](<https://www.msx.org/wiki/COLOR_SPRITE()>)
- [COLOR SPRITE$()](<https://www.msx.org/wiki/COLOR_SPRITE$()>)
- [PUT SPRITE](https://www.msx.org/wiki/PUT_SPRITE)
- [Tiny Sprite Support](https://github.com/amaurycarvalho/msxbas2rom/wiki/TS-Support)
- [Sprite Extended Commands](https://github.com/amaurycarvalho/msxbas2rom/wiki/Extended-Commands#sprite-extended-commands)
- [Sprite Collision Detection Functions](https://github.com/amaurycarvalho/msxbas2rom/wiki/Extended-Functions#sprite-collision-detection-functions)
