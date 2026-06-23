## MODIFIED Requirements

### Requirement: Convert parsed BASIC actions into Z80 machine code

As a ROM builder user, the system SHALL convert parsed BASIC actions into Z80 machine code and symbol/resource metadata so that the ROM image can be generated reliably.

Compilation SHALL start only when parser output contains tags/actions. The compiler SHALL emit start/end program stubs and generated code for each tag. Duplicate or invalid symbols/flow states (e.g., FOR without NEXT) SHALL be reported as compile errors. DATA/IDATA parser flags SHALL produce corresponding compiler-managed resources. The compiler SHALL track generated code/RAM usage and expose metrics used by the CLI. For ASCII16 and ASCII16-X modes, the compiler SHALL emit 16-bit segment values in segment bridges and cross-segment fixups, while maintaining 8-bit emission for ASCII8, Konami4, and KonamiSCC modes.

#### Scenario: Compile valid parser output

- **WHEN** semantic analysis is executed on a parser result with valid tags and actions
- **THEN** compilation succeeds
- **AND** code size is greater than zero
- **AND** RAM usage metrics are available

#### Scenario: Fail on duplicated line symbol mapping

- **WHEN** parser tags map to an already declared line symbol
- **THEN** compilation fails
- **AND** error message indicates line number already declared

#### Scenario: Register DATA and IDATA resources

- **WHEN** parser output has `has_data` and `has_idata` enabled and semantic analysis is executed
- **THEN** the compiler adds data and idata resources to the resource manager

#### Scenario: ASCII16 mode emits 9-byte bridges with 16-bit segment

- **WHEN** the compiler writes a segment-crossing bridge in ASCII16 or ASCII16-X mode
- **THEN** the bridge is 9 bytes containing `LD HL, <segm16>; LD DE, 0x8000; JP MR_JUMP`
- **AND** the `step` variable for code blocks is 9

#### Scenario: ASCII8 mode emits 8-byte bridges with 8-bit segment

- **WHEN** the compiler writes a segment-crossing bridge in ASCII8 or Konami mode
- **THEN** the bridge is 8 bytes containing `LD A, <segm8>; LD HL, 0x8000; JP MR_JUMP`
- **AND** the `step` variable for code blocks is 8

#### Scenario: Cross-segment fixup writes 2-byte segment for 16-bit modes

- **WHEN** a cross-segment fixup is resolved in ASCII16 or ASCII16-X mode
- **THEN** the segment number is written as 2 bytes at the fixup preamble location
- **AND** the preamble uses `LD HL, <segm16>` for the segment value

#### Scenario: Segment total calculation remains unchanged for all modes

- **WHEN** the compiler calculates `segm_total` after writing all code blocks
- **THEN** the formula `((segm_last + 1) / 16 + 1) * 16` is used regardless of mapper
- **AND** the rounding to multiple of 16 segments (128KB granularity) is preserved

## ADDED Requirements

### Requirement: Compiler uses uses16BitSegments helper for conditional segment width

The compiler SHALL use `opts->uses16BitSegments()` to determine whether to emit 8-bit or 16-bit segment values in bridges, fixup preambles, and cross-segment patching. The compiler SHALL NOT check the `compileMode` enum directly for segment width decisions. The segment numbering logic (ASCII8-style, two per 16KB page, starting at 2, incrementing by 2) SHALL remain identical for all mappers.

#### Scenario: Bridge step size is conditional on mapper

- **WHEN** the compiler determines step size for segment-crossing bridges
- **THEN** `step = opts->uses16BitSegments() ? 9 : 8` for code blocks
- **AND** `step = 3` for data blocks (unchanged)

#### Scenario: Fixup preamble emits conditional segment width

- **WHEN** the compiler creates a fixup preamble for cross-segment calls
- **THEN** for 16-bit modes, the preamble includes `LD HL, 0` (3 bytes) for the segment placeholder
- **AND** for 8-bit modes, the preamble includes `LD A, 0` (2 bytes) for the segment placeholder

#### Scenario: Segment numbering algorithm is mapper-agnostic

- **WHEN** the compiler assigns segment numbers to code blocks
- **THEN** segment numbers start at 2 and increment by 2 per 8KB page regardless of mapper
- **AND** `codeItem->segm = segm_last` for addresses < 0xA000, `segm_last + 1` otherwise
