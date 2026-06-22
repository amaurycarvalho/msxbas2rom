## ADDED Requirements

### Requirement: CMD MTF uses dynamic VRAM addresses from BIOS variables

The CMD MTF kernel routine SHALL read VRAM base addresses from BIOS work area variables (GRPCGP at 0xF3CB for pattern generator, GRPCOL at 0xF3C9 for color table, GRPNAM at 0xF3C7 for name table) instead of using hardcoded VRAM addresses. Bank offsets (0x0000, 0x0800, 0x1000) SHALL be added to the BIOS variable values at runtime. This SHALL work on all screen modes where CMD MTF is supported (SCREEN 2 and SCREEN 4).

#### Scenario: MTF writes tileset to GRPCGP-based addresses
- **WHEN** `CMD MTF 0` is executed on SCREEN 4 with default VRAM layout
- **THEN** pattern data is written to VRAM at GRPCGP (0x0000), GRPCGP+0x0800, GRPCGP+0x1000
- **AND** color data is written to VRAM at GRPCOL (0x2000), GRPCOL+0x0800, GRPCOL+0x1000
- **AND** the map data is written to GRPNAM (0x1800)

#### Scenario: MTF works with custom VRAM layout
- **WHEN** GRPCGP is set to 0x4000, GRPCOL is set to 0x6000, and GRPNAM is set to 0x5800
- **AND** `CMD MTF 1` is executed
- **THEN** tileset data is written to VRAM at the custom addresses with correct bank offsets
- **AND** the map data is written to the custom GRPNAM address

#### Scenario: MTF palette uses BIOS variables unchanged
- **WHEN** `CMD MTF 0` is executed with a resource containing palette data
- **THEN** the palette handling continues to use S.SETPLT (subrom palette routine) as before
- **AND** no palette-related changes are needed because palettes are VDP-register based, not VRAM-address based

#### Scenario: MTF with operation and coordinate parameters still works
- **WHEN** `CMD MTF 0, 1, 100, 50` is executed (absolute map coords)
- **THEN** the map window at (100, 50) is loaded from resource 0
- **AND** the map data is written to the address stored in GRPNAM
