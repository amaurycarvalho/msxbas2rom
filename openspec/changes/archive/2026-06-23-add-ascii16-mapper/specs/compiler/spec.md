## ADDED Requirements

### Requirement: Compiler emits 8KB-pair segment numbers compatible with all MegaROM mappers
The compiler SHALL emit 8KB-pair segment numbers (2, 4, 6, ...) for all MegaROM modes including ASCII16. The ASCII16 kernel SHALL handle the 8KB→16KB conversion at runtime via `srl a` in the patched `MR_CHANGE_SGM` routine. No compiler changes are required for ASCII16 support.

#### Scenario: ASCII16 uses same segment math as ASCII8
- **WHEN** a program is compiled in ASCII16 mode
- **THEN** `segm_last` SHALL start at 2 (same as ASCII8)
- **AND** `segm_last` SHALL increment by 2 per 16KB page (same as ASCII8)
- **AND** the segment-skip preamble SHALL be the standard 8-byte sequence (same as ASCII8)
- **AND** fixup `segm_from`/`segm_to` SHALL use `* 2 + 2` multipliers (same as ASCII8)
