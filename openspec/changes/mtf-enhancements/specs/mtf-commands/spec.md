## ADDED Requirements

### Requirement: MTF loads palette resources into VRAM
The system SHALL load a palette resource into VDP palette registers when CMD MTF is called with a palette resource number and no map operation.

#### Scenario: Load palette on MSX2 screen 2 or 4
- **WHEN** `CMD MTF 0` is executed with resource 0 being a palette file and SCREEN 2 or 4 is active on MSX2
- **THEN** the 16-color palette from the resource is written to VDP palette registers

#### Scenario: Load palette skipped on MSX1
- **WHEN** `CMD MTF 0` is executed with a palette resource on MSX1
- **THEN** the command returns immediately without modifying VDP registers

#### Scenario: Load palette ignored on unsupported screen mode
- **WHEN** `CMD MTF 0` is executed with SCREEN 0 or 1 active
- **THEN** the command returns immediately without loading

### Requirement: MTF loads tileset resources into VRAM
The system SHALL load a tileset resource into VRAM pattern and color tables when CMD MTF is called with a tileset resource number.

#### Scenario: Load tileset on screen 2 or 4
- **WHEN** `CMD MTF 1` is executed with resource 1 being a tileset file and SCREEN 2 or 4 is active
- **THEN** tile pattern data is copied to VRAM banks 0, 1, 2 (offsets 0x0000, 0x0800, 0x1000) and color data to banks 0, 1, 2 (offsets 0x2000, 0x2800, 0x3000)

#### Scenario: Tileset activates tiled mode
- **WHEN** a tileset resource is successfully loaded
- **THEN** SOMODE is set to 2 indicating tiled mode is active

### Requirement: MTF copies full map using relative screen coordinates (operation 0)
The system SHALL copy a full 32x24-tile screen from the map resource using screen-based col/row coordinates when operation 0 is specified or implied. Operation 0 SHALL delegate to the window_copy kernel routine by setting full-screen window parameters (width=32, height=24, screen_x=0, screen_y=0).

#### Scenario: Full map copy with defaults
- **WHEN** `CMD MTF 2` is executed with resource 2 being a map file
- **THEN** the map area starting at column 0, row 0 is copied to the screen name table at page 0

#### Scenario: Full map copy with explicit col and row
- **WHEN** `CMD MTF 2, 0, 3, 2` is executed
- **THEN** the map area starting at column 3 (x = 3 × 32 = 96), row 2 (y = 2 × 24 = 48) is copied to the screen name table

#### Scenario: Operation 0 delegates to window_copy
- **WHEN** `CMD MTF 2, 0, 5, 3` is executed
- **THEN** the kernel computes map_x=160 (5×32), map_y=72 (3×24). `MTF_WIN_W_PARM=32`, `MTF_WIN_H_PARM=24`, `MTF_SCR_X_PARM=0`, `MTF_SCR_Y_PARM=0` are already set by the compiler as defaults
- **AND** execution continues at `cmd_mtf.window_copy` (single code path for all map copies)
- **AND** the full-width optimization (screen_x=0, width=32) skips LDIRMV, resulting in 1 LDIRVM

#### Scenario: Full map copy with page parameter (page scaffolding)
- **WHEN** `CMD MTF 2, 0, 1, 0, 2` is executed on MSX2
- **THEN** the page parameter (2) is stored in `MTF_PAGE_PARM` at PARM1+16
- **AND** the map area at column 1, row 0 is copied to the screen name table at 0x1800 (dummy — kernel ignores page value; real page support deferred to `set-page-screen4`)

### Requirement: MTF copies full map using absolute tile coordinates (operation 1)
The system SHALL copy a full 32x24-tile screen from the map resource using absolute map pixel coordinates when operation 1 is specified. Operation 1 SHALL delegate to the window_copy kernel routine by setting full-screen window parameters (width=32, height=24, screen_x=0, screen_y=0).

#### Scenario: Full map copy with absolute coordinates
- **WHEN** `CMD MTF 2, 1, 128, 96` is executed
- **THEN** the map area starting at absolute position x=128, y=96 is copied to the screen name table

#### Scenario: Negative coordinate wrapping
- **WHEN** `CMD MTF 2, 1, -10, -5` is executed on a map of width 256 and height 192
- **THEN** x wraps to 246 (256 - 10) and y wraps to 187 (192 - 5) before the copy

#### Scenario: Coordinate modulo wrapping
- **WHEN** `CMD MTF 2, 1, 300, 250` is executed on a map of width 256 and height 192
- **THEN** x wraps to 44 (300 mod 256) and y wraps to 58 (250 mod 192) before the copy

#### Scenario: Operation 1 delegates to window_copy
- **WHEN** `CMD MTF 2, 1, 64, 48` is executed on MSX2
- **THEN** after coordinate wrapping, the kernel sets MTF_COLX_PARM=64, MTF_ROWY_PARM=48. `MTF_WIN_W_PARM=32`, `MTF_WIN_H_PARM=24`, `MTF_SCR_X_PARM=0`, `MTF_SCR_Y_PARM=0` are already set by the compiler
- **AND** execution continues at `cmd_mtf.window_copy`
- **AND** the full-width optimization (screen_x=0, width=32) skips LDIRMV, resulting in 1 LDIRVM

#### Scenario: Absolute copy with page parameter (page scaffolding)
- **WHEN** `CMD MTF 2, 1, 64, 48, 1` is executed on MSX2
- **THEN** the page parameter (1) is stored in `MTF_PAGE_PARM`
- **AND** the map area at x=64, y=48 is copied to the screen name table at 0x1800 (dummy — kernel ignores page value)

### Requirement: MTF copies partial map window (operation 2)
The system SHALL copy a rectangular window from the map resource to a specific screen position when operation 2 is specified, preserving all tiles outside the window area unchanged in VRAM.

#### Scenario: Window copy with all parameters
- **WHEN** `CMD MTF 2, 2, 20, 12, 10, 6, 5, 8` is executed
- **THEN** a 10×6 tile rectangle from map position (20, 12) is copied to screen position (5, 8)
- **AND** tiles outside the 10×6 window area remain unchanged in VRAM

#### Scenario: Window copy preserves tiles above and below the window
- **WHEN** `CMD MTF 2, 2, 0, 0, 16, 10, 8, 4` is executed and the screen already has tiles displayed
- **THEN** rows 0..3 (above the window) and rows 14..23 (below the window) are not modified in VRAM

#### Scenario: Window copy preserves tiles left and right of the window
- **WHEN** `CMD MTF 2, 2, 10, 10, 4, 4, 10, 10` is executed and the screen already has tiles displayed
- **THEN** columns 0..9 and 14..31 on each affected row remain unchanged in VRAM

#### Scenario: Window copy performs exactly 2 VDP transfers
- **WHEN** `CMD MTF 2, 2, 0, 0, 10, 6, 5, 8` is executed
- **THEN** the kernel performs 1 LDIRMV read and 1 LDIRVM write (2 VDP transactions total)
- **AND** the number of VDP transactions does not vary with window height

#### Scenario: Full-width window skips LDIRMV (1 VDP transfer)
- **WHEN** `CMD MTF 2, 2, 0, 0, 32, 6, 0, 8` is executed (screen_x=0, width=32)
- **THEN** the full-width optimization activates: LDIRMV is skipped because all bytes read would be overwritten
- **AND** only 1 LDIRVM transfer is performed

#### Scenario: Window copy to top-left corner
- **WHEN** `CMD MTF 2, 2, 40, 20, 8, 4, 0, 0` is executed
- **THEN** an 8×4 tile rectangle from map position (40, 20) is copied to screen position (0, 0)

#### Scenario: Window copy clips width and height to screen bounds
- **WHEN** `CMD MTF 2, 2, 0, 0, 40, 30, 20, 20` is executed (width=40 exceeds screen width, height=30 exceeds screen height)
- **THEN** width is clipped to 12 (32 - 20) and height is clipped to 4 (24 - 20)
- **AND** only the clipped 12×4 window is copied to VRAM

#### Scenario: Window copy clipped to zero size performs no VDP operations
- **WHEN** `CMD MTF 2, 2, 0, 0, 10, 6, 32, 0` is executed (screen_x=32 is off-screen, clipping yields width=0)
- **THEN** no LDIRMV or LDIRVM operations are performed
- **AND** VRAM is not modified

#### Scenario: Window copy with page parameter (page scaffolding)
- **WHEN** `CMD MTF 2, 2, 0, 32, 16, 10, 8, 4, 1` is executed on MSX2
- **THEN** the page parameter (1) is stored in `MTF_PAGE_PARM`
- **AND** a 16×10 tile window is copied to 0x1800 (dummy) at screen position (8, 4)

#### Scenario: Window copy defaults page to 0 when omitted
- **WHEN** `CMD MTF 2, 2, 10, 10, 4, 4, 0, 0` is executed without a page parameter
- **THEN** the window is copied to page 0 (VRAM 0x1800)

### Requirement: Window-copy preserves VRAM tiles outside the window
The system SHALL read the VRAM region surrounding the window before overwriting it with map data, ensuring tiles outside the window remain unchanged.

#### Scenario: Surrounding tiles preserved after window copy
- **WHEN** `CMD MTF 2, 2, 10, 10, 4, 4, 5, 5` is executed on a screen that already displays tiles
- **THEN** the tiles at screen positions (4, 5) and (9, 5) on the same rows as the window are unchanged
- **AND** the tiles at screen positions (5, 4) and (5, 9) on the same columns as the window are unchanged
- **AND** rows completely outside the window range (0..4 and 10..23) are unchanged

#### Scenario: Window at screen edge preserves adjacent tiles
- **WHEN** `CMD MTF 2, 2, 0, 0, 8, 8, 0, 0` is executed
- **THEN** tiles at column 8 on rows 0..7 (to the right of the window) are unchanged
- **AND** tiles on rows 8..23 (below the window) are unchanged

#### Scenario: Full-screen window is equivalent to full map copy
- **WHEN** `CMD MTF 2, 2, 0, 0, 32, 24, 0, 0` is executed
- **THEN** the entire 32×24 screen is overwritten with map data (no tiles preserved, result is identical to operation 0/1)
- **AND** 1 LDIRVM transfer is performed (full-width optimization: screen_x=0, width=32 skips LDIRMV)

### Requirement: Page parameter scaffolding — kernel ignores page value
The system SHALL accept and store the page parameter in the PARM1 workarea (`MTF_PAGE_PARM` at PARM1+16), but SHALL NOT compute page-based VRAM addresses. The kernel SHALL always target `0x1800` for the name table regardless of the page value. Real page offset support is deferred to the `set-page-screen4` change.

#### Scenario: Page parameter stored but ignored on MSX2
- **WHEN** `CMD MTF 2, 0, 0, 0, 3` is executed on MSX2
- **THEN** the page parameter (3) is stored in `MTF_PAGE_PARM` at PARM1+16
- **AND** the map is copied to VRAM 0x1800 (kernel ignores page value)

#### Scenario: Window copy page parameter stored but ignored
- **WHEN** `CMD MTF 2, 2, 0, 0, 8, 8, 0, 0, 2` is executed on MSX2
- **THEN** the page parameter (2) is stored in `MTF_PAGE_PARM`
- **AND** the window is copied to VRAM 0x1800 (kernel ignores page value)

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
The system SHALL emit Z80 instructions that store MTF parameters to the PARM1 workarea (0xF6E8–0xF6F8) before calling the kernel.

#### Scenario: Operation 0 emits correct RAM stores
- **WHEN** `CMD MTF 2, 0, 1, 0` is compiled
- **THEN** the emitted code writes resource=2 to PARM1+0, col=1 to PARM1+2, row=0 to PARM1+4, operation=0 to PARM1+6, and defaults for window/page (0) to PARM1+8 through PARM1+16

#### Scenario: Operation 2 emits all 9 parameters to RAM
- **WHEN** `CMD MTF 2, 2, 20, 12, 10, 6, 5, 8, 1` is compiled
- **THEN** the emitted code writes resource=2, map_x=20, map_y=12, operation=2, width=10, height=6, screen_x=5, screen_y=8, page=1 to their respective PARM1 offsets
