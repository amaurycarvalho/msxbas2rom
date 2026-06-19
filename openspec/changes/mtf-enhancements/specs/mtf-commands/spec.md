## ADDED Requirements

### Requirement: MTF loads palette resources into VRAM
The system SHALL load a palette resource into VDP palette registers when CMD MTF is called with a palette resource number and no map operation.

#### Scenario: Load palette on MSX2 screen 2
- **WHEN** `CMD MTF 0` is executed with resource 0 being a palette file and SCREEN 2 is active on MSX2
- **THEN** the 16-color palette from the resource is written to VDP palette registers

#### Scenario: Load palette skipped on MSX1
- **WHEN** `CMD MTF 0` is executed with a palette resource on MSX1
- **THEN** the command returns immediately without modifying VDP registers

#### Scenario: Load palette ignored on unsupported screen mode
- **WHEN** `CMD MTF 0` is executed with SCREEN 0 active
- **THEN** the command returns immediately without loading

### Requirement: MTF loads tileset resources into VRAM
The system SHALL load a tileset resource into VRAM pattern and color tables when CMD MTF is called with a tileset resource number.

#### Scenario: Load tileset on screen 2
- **WHEN** `CMD MTF 1` is executed with resource 1 being a tileset file and SCREEN 2 is active
- **THEN** tile pattern data is copied to VRAM banks 0, 1, 2 (offsets 0x0000, 0x0800, 0x1000) and color data to banks 0, 1, 2 (offsets 0x2000, 0x2800, 0x3000)

#### Scenario: Tileset activates tiled mode
- **WHEN** a tileset resource is successfully loaded
- **THEN** SOMODE is set to 2 indicating tiled mode is active

### Requirement: MTF copies full map using relative screen coordinates (operation 0)
The system SHALL copy a full 32x24-tile screen from the map resource using screen-based col/row coordinates when operation 0 is specified or implied.

#### Scenario: Full map copy with defaults
- **WHEN** `CMD MTF 2` is executed with resource 2 being a map file
- **THEN** the map area starting at column 0, row 0 is copied to the screen name table at page 0

#### Scenario: Full map copy with explicit col and row
- **WHEN** `CMD MTF 2, 0, 3, 2` is executed
- **THEN** the map area starting at column 3 (x = 3 × 32 = 96), row 2 (y = 2 × 24 = 48) is copied to the screen name table

#### Scenario: Full map copy with page parameter on MSX2
- **WHEN** `CMD MTF 2, 0, 1, 0, 2` is executed on MSX2
- **THEN** the map area at column 1, row 0 is copied to VRAM at page 2 address (0x4000)

### Requirement: MTF copies full map using absolute tile coordinates (operation 1)
The system SHALL copy a full 32x24-tile screen from the map resource using absolute map pixel coordinates when operation 1 is specified.

#### Scenario: Full map copy with absolute coordinates
- **WHEN** `CMD MTF 2, 1, 128, 96` is executed
- **THEN** the map area starting at absolute position x=128, y=96 is copied to the screen name table

#### Scenario: Negative coordinate wrapping
- **WHEN** `CMD MTF 2, 1, -10, -5` is executed on a map of width 256 and height 192
- **THEN** x wraps to 246 (256 - 10) and y wraps to 187 (192 - 5) before the copy

#### Scenario: Coordinate modulo wrapping
- **WHEN** `CMD MTF 2, 1, 300, 250` is executed on a map of width 256 and height 192
- **THEN** x wraps to 44 (300 mod 256) and y wraps to 58 (250 mod 192) before the copy

#### Scenario: Absolute copy with page parameter on MSX2
- **WHEN** `CMD MTF 2, 1, 64, 48, 1` is executed on MSX2
- **THEN** the map area at x=64, y=48 is copied to VRAM at page 1 address (0x3800)

### Requirement: MTF copies partial map window (operation 2)
The system SHALL copy a rectangular window from the map resource to a specific screen position when operation 2 is specified.

#### Scenario: Window copy with all parameters
- **WHEN** `CMD MTF 2, 2, 20, 12, 10, 6, 5, 8` is executed
- **THEN** a 10×6 tile rectangle from map position (20, 12) is copied to screen position (5, 8)

#### Scenario: Window copy to top-left corner
- **WHEN** `CMD MTF 2, 2, 40, 20, 8, 4, 0, 0` is executed
- **THEN** an 8×4 tile rectangle from map position (40, 20) is copied to screen position (0, 0)

#### Scenario: Window copy with page parameter on MSX2
- **WHEN** `CMD MTF 2, 2, 0, 32, 16, 10, 8, 4, 1` is executed on MSX2
- **THEN** a 16×10 tile window is copied to page 1 VRAM address (0x3800) at screen position (8, 4)

#### Scenario: Window copy defaults page to 0 when omitted
- **WHEN** `CMD MTF 2, 2, 10, 10, 4, 4, 0, 0` is executed without a page parameter
- **THEN** the window is copied to page 0 (VRAM 0x1800)

### Requirement: Page parameter is ignored on MSX1
The system SHALL silently use page 0 (VRAM 0x1800) on MSX1 regardless of the page parameter value.

#### Scenario: Page parameter on MSX1
- **WHEN** `CMD MTF 2, 0, 0, 0, 3` is executed on MSX1
- **THEN** the map is copied to VRAM 0x1800 (page 0) ignoring the page=3 parameter

#### Scenario: Window copy page on MSX1
- **WHEN** `CMD MTF 2, 2, 0, 0, 8, 8, 0, 0, 2` is executed on MSX1
- **THEN** the window is copied to VRAM 0x1800 (page 0) ignoring page=2

### Requirement: Compiler validates MTF parameter count
The system SHALL accept 1 to 9 parameters for CMD MTF and report a syntax error for other counts.

#### Scenario: Single parameter accepted
- **WHEN** `CMD MTF 0` is compiled
- **THEN** compilation succeeds with default values for all other parameters

#### Scenario: Maximum 9 parameters accepted
- **WHEN** `CMD MTF 2, 2, 0, 0, 8, 8, 0, 0, 1` is compiled
- **THEN** compilation succeeds with all 9 parameters passed

#### Scenario: No parameters rejected
- **WHEN** `CMD MTF` is compiled with zero parameters
- **THEN** the compiler reports "CMD MTF parameters is missing"

#### Scenario: Too many parameters rejected
- **WHEN** `CMD MTF 2, 2, 0, 0, 8, 8, 0, 0, 1, 5` is compiled with 10 parameters
- **THEN** the compiler reports "CMD MTF syntax error"

### Requirement: Compiler emits RAM-based parameter storage
The system SHALL emit Z80 instructions that store MTF parameters to the DAC workarea (0xF7F6–0xF806) before calling the kernel.

#### Scenario: Operation 0 emits correct RAM stores
- **WHEN** `CMD MTF 2, 0, 1, 0` is compiled
- **THEN** the emitted code writes resource=2 to DAC+0, col=1 to DAC+2, row=0 to DAC+4, operation=0 to DAC+6, and defaults for window/page (0) to DAC+8 through DAC+16

#### Scenario: Operation 2 emits all 9 parameters to RAM
- **WHEN** `CMD MTF 2, 2, 20, 12, 10, 6, 5, 8, 1` is compiled
- **THEN** the emitted code writes resource=2, map_x=20, map_y=12, operation=2, width=10, height=6, screen_x=5, screen_y=8, page=1 to their respective DAC offsets
