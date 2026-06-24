## ADDED Requirements

### Requirement: Compiler code buffer supports up to 2MB of generated Z80 code

The compiler SHALL allocate a code buffer (`COMPILE_CODE_SIZE`) of at least 2MB (128 pages of 16KB) so that compiled Z80 code can fill a 2048KB MegaROM without hitting a buffer overflow before the resource manager's 8-bit segment limit (256 segments × 8KB).

The compiler SHALL report an error when compiled code exceeds the buffer limit, referencing the actual `COMPILE_CODE_SIZE` and `COMPILE_MAX_PAGES` values dynamically.

#### Scenario: Compile program that produces 1.5MB of Z80 code

- **WHEN** a BASIC program compiles to 1.5MB of Z80 code in MegaROM mode
- **THEN** compilation succeeds
- **AND** all code is written to the destination buffer
- **AND** no buffer overflow error is emitted

#### Scenario: Buffer limit error uses dynamic values

- **WHEN** compiled code exceeds the buffer limit
- **THEN** error message references the current `COMPILE_CODE_SIZE` value in bytes
- **AND** error message references the current `COMPILE_MAX_PAGES` value in pages

### Requirement: All MegaROM formats reach 2048KB limit with 8-bit segments

The system SHALL allow any supported MegaROM mapper format (ASCII8, ASCII16, ASCII16X, Konami4, KonamiSCC) to produce ROMs up to 2048KB total, limited only by the 8-bit segment storage (0-255 segments × 8KB).

The resource manager SHALL enforce a maximum of 256 segments (`resourceBlockSegment > 255`) corresponding to 2048KB of addressable ROM space.

#### Scenario: ASCII8 ROM reaches 2MB with code and resources

- **WHEN** a BASIC program with ASCII8 mapper generates Z80 code and resources filling 2048KB
- **THEN** ROM is built successfully
- **AND** resource block segment does not exceed 255
- **AND** no size limit error is emitted

#### Scenario: ASCII16 ROM reaches 2MB

- **WHEN** a BASIC program with ASCII16 mapper fills 2048KB
- **THEN** ROM is built successfully
- **AND** segment numbers (after SRL A conversion to 16KB pages) fit within the mapper's 8-bit page register

#### Scenario: ROM exceeds 2048KB limit

- **WHEN** compiled code and resources exceed 2048KB (256 segments)
- **THEN** resource manager emits error "MegaROM size limit exceeded (2048K)"
- **AND** build fails
