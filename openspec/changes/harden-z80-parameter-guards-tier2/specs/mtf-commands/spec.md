# Spec Delta

## ADDED Requirements

### Requirement: MTF validates resource headers and destination bounds

`CMD MTF` SHALL treat the map header as untrusted input. A map with zero width or height, or a resource too short for the header, SHALL NOT produce an infinite loop or a division by zero. Destination `screen_x`/`screen_y` SHALL be clipped to the visible screen for negative and oversized values. Negative `map_x`/`map_y` SHALL continue to wrap modulo the map dimensions for operations 1 and 2.

#### Scenario: Malformed header returns safely
- **WHEN** a map resource declares zero width or height
- **THEN** `CMD MTF` returns without copying and without hanging

#### Scenario: Negative map coordinates wrap
- **WHEN** `CMD MTF 2,1,-1,-1` is executed on a 32x24 map
- **THEN** the effective source position is column 31, row 23

#### Scenario: Negative destination is clipped
- **WHEN** operation 2 receives `screen_x=-1`
- **THEN** the destination is clipped or skipped instead of wrapping to column 31
