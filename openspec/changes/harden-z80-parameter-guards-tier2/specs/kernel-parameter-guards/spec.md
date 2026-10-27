# Spec Delta

## ADDED Requirements

### Requirement: MTF malformed headers and window bounds are safe

The MTF kernel SHALL verify that the map resource is large enough for its width/height header and SHALL NOT divide by zero or loop forever when width or height is zero. It SHALL preserve negative `map_x`/`map_y` modulo wrapping for operations 1 and 2. It SHALL clip destination `screen_x`/`screen_y` correctly for negative and oversized values, and SHALL use a defined 16-bit interpretation for window width/height.

#### Scenario: Zero-height map does not hang
- **WHEN** a map header declares height `0` and the command receives a negative `map_y`
- **THEN** the command returns without entering an unbounded adjustment loop

#### Scenario: Negative map coordinates still wrap
- **WHEN** `CMD MTF 2,1,-1,-1` is executed on a 32x24 map
- **THEN** the source coordinates become `31` and `23`

#### Scenario: Negative destination does not draw at the opposite edge
- **WHEN** operation 2 receives `screen_x=-1`
- **THEN** the window is clipped or skipped
- **AND** it is not drawn at column `31`

#### Scenario: Invalid operation is a no-op
- **WHEN** operation receives a dynamic value outside `0..2`
- **THEN** the command returns without copying map data

### Requirement: Pattern and color table indices are bounded

Sprite and tile pattern/color access SHALL normalize or reject indices that would address outside the intended table. This covers `SET/GET SPRITE PATTERN/COLOR`, `SET TILE PATTERN/COLOR`, and flip/rotate pattern reads.

#### Scenario: Invalid sprite pattern index does not address adjacent VRAM
- **WHEN** a sprite pattern operation receives an index outside the sprite pattern table range
- **THEN** the operation is normalized or skipped

#### Scenario: Invalid tile pattern index is bounded
- **WHEN** a tile pattern/color operation receives a tile index that would leave the intended table
- **THEN** the write is normalized or skipped

### Requirement: Tiny Sprite resources are size-checked

`cmd_wrtspr` SHALL verify that the resource contains the declared number of pattern and color bytes before copying to VRAM.

#### Scenario: Truncated SPR resource does not overrun
- **WHEN** the declared sprite count requires more bytes than the resource contains
- **THEN** the copy is limited or skipped
- **AND** no bytes beyond the resource are read

### Requirement: Font, mode, and channel dynamic values are bounded

`CMD SETFNT` SHALL treat only style `0..3` and bank `0..2` or all as valid. `CMD TURBO` SHALL use only values `0` and `1`. `CMD PAGE` SHALL use only mode `0`/`1` and delays `0..15`. `CMD CLIP` SHALL use only `0`/`1`. `CMD PLAY` SHALL use only channel `0`/`1`.

#### Scenario: Invalid dynamic font style is safe
- **WHEN** `CMD SETFNT` receives style `4`
- **THEN** the command does not select an out-of-range font resource

#### Scenario: Invalid dynamic turbo value is safe
- **WHEN** `CMD TURBO` receives a value other than `0` or `1`
- **THEN** the kernel does not pass an undefined value to the CPU-speed BIOS call
