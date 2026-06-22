## ADDED Requirements

### Requirement: SET PAGE works on Screen 4 with display page and active page

The system SHALL support `SET PAGE <DisplayPage>,<ActivePage>` when the current screen mode is Screen 4 (SCRMOD=5) on MSX2. Page 0 SHALL use the default VDP VRAM layout (patterns at 0x0000, colors at 0x2000, name table at 0x1800). Pages 1 through 7 SHALL be stored in free VRAM starting at 0x3900, each occupying 0x3400 bytes (3 banks of patterns, 3 banks of colors, 1 name table). The display page SHALL control which page is visible on screen via VDP register R#2. The active page SHALL control where write operations target via BIOS variables GRPNAM, GRPCOL, GRPCGP, NAMBAS, and CGPBAS.

#### Scenario: Set display page on Screen 4
- **WHEN** SCREEN 4 is active and `SET PAGE 2` is executed
- **THEN** VDP register R#2 is programmed to display page 2's name table
- **AND** the tile set (patterns + colors) for page 2 is loaded from VRAM storage into the active VRAM area (0x0000 for patterns, 0x2000 for colors)

#### Scenario: Set display page and active page on Screen 4
- **WHEN** SCREEN 4 is active and `SET PAGE 1, 3` is executed
- **THEN** VDP register R#2 is programmed to display page 1's name table (display page)
- **AND** BIOS variables GRPNAM, GRPCOL, GRPCGP are updated to target page 3's data (active page)
- **AND** subsequent write operations (CLS, PRINT, PUT TILE, etc.) affect page 3

#### Scenario: Page 0 uses default VRAM layout
- **WHEN** SCREEN 4 is initialized and `SET PAGE 0` is executed
- **THEN** the VDP registers and BIOS variables match the default MSX2 Screen 4 initialization
- **AND** patterns are at VRAM 0x0000, colors at VRAM 0x2000, name table at VRAM 0x1800

#### Scenario: SET PAGE with single parameter on Screen 4
- **WHEN** `SET PAGE 3` is executed on Screen 4
- **THEN** both display page and active page are set to 3
- **AND** the tile set for page 3 is loaded into the active VRAM area

#### Scenario: SET PAGE preserves tile data between page switches
- **WHEN** tiles are written to page 2 via SET TILE or CMD MTF
- **AND** the page is switched to page 3 and then back to page 2
- **THEN** the tiles previously written to page 2 are still present

#### Scenario: Invalid page number on Screen 4
- **WHEN** `SET PAGE 8` is executed on Screen 4
- **THEN** an error is reported because 8 exceeds the maximum page count (7)

### Requirement: Compiler dispatches SET PAGE to correct kernel routine

The compiler SHALL emit runtime code that checks the current screen mode (SCRMOD at 0xFCAF) when compiling a SET PAGE statement. If SCRMOD equals 5 (Screen 4), the compiler SHALL call the kernel routine `screen4_set_page`. If SCRMOD is not 5, the compiler SHALL call the existing kernel routine `XBASIC_SET_PAGE` (C70CC) for screen 5+ compatibility.

#### Scenario: Compiler emits Screen 4 page dispatch
- **WHEN** a BAS file contains `SET PAGE 2, 1` after `SCREEN 4`
- **THEN** the compiler emits Z80 code that checks SCRMOD at runtime
- **AND** if SCRMOD equals 5, calls `screen4_set_page` with display page 2 and active page 1
- **AND** if SCRMOD does not equal 5, calls `XBASIC_SET_PAGE` (C70CC) with display page 2 and stores active page 1 at ACPAGE

#### Scenario: Compiler maintains existing SET PAGE behavior for screen 5+
- **WHEN** a BAS file contains `SET PAGE 0, 1` after `SCREEN 5`
- **THEN** the compiled Z80 code calls `XBASIC_SET_PAGE` (C70CC) and stores active page at ACPAGE
- **AND** the behavior is identical to before this change

### Requirement: Graphics commands respect active page on Screen 4

Commands and functions that read or write to VRAM SHALL target the active page when SCREEN 4 is in use. This includes: CLS, PRINT, COPY, CMD MTF, SCREEN LOAD, BLOAD of screen data, SET TILE, PUT TILE, GET TILE, TILE(), and BASE().

#### Scenario: CLS clears active page name table
- **WHEN** SCREEN 4 is active and `SET PAGE 0, 2` is executed
- **AND** `CLS` is executed
- **THEN** the name table of page 2 is cleared
- **AND** page 0's name table (the displayed page) is NOT affected

#### Scenario: PRINT writes to active page
- **WHEN** SCREEN 4 is active, SET TILE is ON, `SET PAGE 0, 1` is executed
- **AND** `PRINT "HELLO"` is executed
- **THEN** the text is written to the name table of page 1
- **AND** the text does NOT appear on the displayed page 0

#### Scenario: PUT TILE writes to active page
- **WHEN** SCREEN 4 is active, SET TILE is ON, `SET PAGE 0, 3` is executed
- **AND** `PUT TILE 42, (10, 10)` is executed
- **THEN** tile 42 is written to position (10, 10) in the name table of page 3

#### Scenario: TILE() reads from active page
- **WHEN** SCREEN 4 is active, SET TILE is ON, `SET PAGE 0, 2` is executed
- **AND** `A = TILE(5, 5)` is executed
- **THEN** the tile value at position (5, 5) of page 2 is returned

#### Scenario: BASE() returns active page addresses
- **WHEN** SCREEN 4 is active and `SET PAGE 0, 3` is executed
- **AND** `B = BASE(0)` is called
- **THEN** B returns the name table address of page 3
