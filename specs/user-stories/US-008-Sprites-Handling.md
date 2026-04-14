# US-008 - Sprites Handling

## Story

As an MSX-BASIC developer, I want to use SPRITE's specialized statements and functions in my programs so that I can manage sprite assets on the screen.

## Acceptance Criteria

- [ ] Support `SPRITE ON`, `SPRITE OFF`, `SPRITE STOP`, and `SPRITE LOAD <n>`.
- [ ] Support `PUT SPRITE` with optional color/pattern parameters and `COORD`/`STEP` coordinates.
- [ ] Support `SPRITE$(<n>) = <string>` assignment for sprite pattern definition.
- [ ] Support `COLOR SPRITE(<n>) = <color>` and `COLOR SPRITE$(<n>) = <string>` statements.
- [ ] Support `SET SPRITE FLIP`, `SET SPRITE ROTATE`, `SET SPRITE PATTERN`, `SET SPRITE COLOR`.
- [ ] Support `GET SPRITE PATTERN` and `GET SPRITE COLOR`.
- [ ] Support `COLLISION()` function with 0, 1, or 2 parameters.
- [ ] Support Tiny Sprite `.SPR` resource parsing for `SPRITE LOAD`.

---

## BDD Scenarios

### Scenario 1: Enable sprite collision trap

Given a program with `ON SPRITE GOSUB 2000`
When it executes `SPRITE ON`
Then the sprite trap is enabled for collision handling

### Scenario 2: Disable sprite collision trap

Given a program that previously executed `SPRITE ON`
When it executes `SPRITE OFF`
Then the sprite trap is disabled

### Scenario 3: Stop sprite collision trap

Given a program that previously executed `SPRITE ON`
When it executes `SPRITE STOP`
Then the sprite trap is stopped without clearing its handler

### Scenario 4: Load Tiny Sprite resource

Given a program with `FILE "sprite1.spr"`
When it executes `SPRITE LOAD 0`
Then the sprite resource 0 is loaded into the sprite tables

### Scenario 5: Define sprite pattern via SPRITE$

Given a program that builds a pattern string
When it executes `SPRITE$(0) = A$`
Then sprite pattern 0 is updated with the string data

### Scenario 6: Set sprite color via COLOR SPRITE

Given a valid sprite index and a color value
When it executes `COLOR SPRITE(1) = 33`
Then the color attribute of sprite 1 is updated

### Scenario 7: Set sprite color definition via COLOR SPRITE$

Given a valid sprite index and a color string
When it executes `COLOR SPRITE$(0) = CHR$(12)+CHR$(10)`
Then the sprite color definition is updated from the string data

### Scenario 8: Put sprite with coordinates only

Given sprite pattern data already assigned to sprite 0
When it executes `PUT SPRITE 0, (100, 100)`
Then the sprite appears at the given coordinates

### Scenario 9: Put sprite with color and pattern

Given sprite pattern data already assigned
When it executes `PUT SPRITE 0, (100, 100), 15, 2`
Then the sprite is rendered using color 15 and pattern 2

### Scenario 10: Put sprite using STEP coordinates

Given a previous sprite position was set
When it executes `PUT SPRITE 0, STEP(5, -2)`
Then the sprite moves relative to the last position

### Scenario 11: Read and write sprite pattern buffer

Given a sprite pattern buffer `PB%` is allocated
When it executes `GET SPRITE PATTERN 0, PB%`
And it later executes `SET SPRITE PATTERN 1, PB%`
Then sprite pattern 1 matches sprite pattern 0

### Scenario 12: Read and write sprite color buffer

Given a sprite color buffer `CB%` is allocated
When it executes `GET SPRITE COLOR 0, CB%`
And it later executes `SET SPRITE COLOR 1, CB%`
Then sprite color for sprite 1 matches sprite 0

### Scenario 13: Flip sprite pattern

Given a valid sprite index and a direction value
When it executes `SET SPRITE FLIP 0, 1`
Then the sprite pattern is flipped vertically

### Scenario 14: Rotate sprite pattern

Given a valid sprite index and a direction value
When it executes `SET SPRITE ROTATE 0, 2`
Then the sprite pattern is rotated 180 degrees

### Scenario 15: Detect any collision

Given multiple sprites are placed on screen
When it evaluates `COLLISION()`
Then it returns the collided sprite number or -1 when none collide

### Scenario 16: Detect collision for a specific sprite

Given multiple sprites are placed on screen
When it evaluates `COLLISION(2)`
Then it returns the collided sprite number for sprite 2 or -1

### Scenario 17: Detect collision between two sprites

Given multiple sprites are placed on screen
When it evaluates `COLLISION(4, 5)`
Then it returns 5 if sprite 4 collided with sprite 5, otherwise -1

---

## Technical Specification

### Tiny Sprite Support

Syntax:

```
SPRITE LOAD <resource number>
```

Example:

```
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

Notes:

- .SPR files are in plain text format and can be opened by any text editor or edited by [Tiny Sprite](https://msx.jannone.org/tinysprite/tinysprite.html);
- Use with sprite parameter size 2 or 3 on SCREEN statement (16x16 sprite size);
- Limited to 64 unique 16x16 shapes (patterns) for each file uploaded to the Sprite Generator Table;
- The first 32 shapes uploaded are automatically assigned in the Sprite Attribute Table.

### Sprite Collision Detection Functions

Syntax:

```
- COLLISION() return if any sprite collided with another sprite, else
      return -1;
- COLLISION(<n>) return if a sprite <n> collided with another sprite,
      else return -1;
- COLLISION(<n1>,<n2>) return n2 if sprite n1 collided with n2, else
      return -1;

    <-1=no collision|collided sprite number> = COLLISION( <-1=any sprite | sprite1> [, <sprite2> ] )
```

Example:

```
       Beep if any sprite collided with each other:
          SN# = COLLISION(-1)
          IF SN# >= 0 THEN BEEP
       Beep if any sprite collided with sprite 2:
          SN# = COLLISION(2)
          IF SN# >= 0 THEN BEEP
       Beep if sprite 4 collided with sprite 5:
          SN# = COLLISION(5)
          IF SN# = 4 THEN BEEP
       Beep if sprite 5 collided with sprite 4 (direct test):
          SN# = COLLISION( 5, 4 )
          IF SN# >= 0 THEN BEEP
       Beep if sprite 0 collided with sprite 1 (direct test):
          SN# = COLLISION( 0, 1 )
          IF SN# >= 0 THEN BEEP
```

Notes:

- Sprites with same X and Y position are considered the same object, thus there's no collision in this case.

### Sprite Extended Commands

Syntax:

```
Set/Get sprite color

    SET SPRITE COLOR <n>, <8 integers color buffer array>
    GET SPRITE COLOR <n>, <8 integers color buffer array>
      Example:
        10 DIM CB%(7)
        20 GET SPRITE COLOR 0, CB%
        30 SET SPRITE COLOR 1, CB%

  Set/Get sprite pattern

    SET SPRITE PATTERN <n>, <16 integers pattern buffer array>
    GET SPRITE PATTERN <n>, <16 integers pattern buffer array>
      Example:
        10 DIM PB%(3,3)
        20 GET SPRITE PATTERN 0, PB%
        30 SET SPRITE PATTERN 1, PB%

  Flip a sprite pattern

    SET SPRITE FLIP <n>, <dir: 0=horizontal, 1=vertical, 2=both>

  Rotate a sprite pattern

    SET SPRITE ROTATE <n>, <dir: 0=left, 1=right, 2=180 degrees>
```

## References

- [SPRITE$()](<https://www.msx.org/wiki/SPRITE$()>);
- [COLOR SPRITE()](<https://www.msx.org/wiki/COLOR_SPRITE()>);
- [COLOR SPRITE$()](<https://www.msx.org/wiki/COLOR_SPRITE$()>);
- [PUT SPRITE](https://www.msx.org/wiki/PUT_SPRITE);
- [Tiny Sprite Support](https://github.com/amaurycarvalho/msxbas2rom/wiki/TS-Support);
- [Sprite Extended Commands](https://github.com/amaurycarvalho/msxbas2rom/wiki/Extended-Commands#sprite-extended-commands);
- [Sprite Collision Detection Functions](https://github.com/amaurycarvalho/msxbas2rom/wiki/Extended-Functions#sprite-collision-detection-functions).
