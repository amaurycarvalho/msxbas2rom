# Kernel Call Routing

## MODIFIED Requirements

### Requirement: Wrapper table covers all kernel routines called from C++

The runtime central jump table SHALL contain a dispatch entry for every Z80 kernel routine called from C++ and for every dependency required by those routines. No such routine in the `src/infrastructure/kernel/asm/src/header/` subtree SHALL be called via a hardcoded absolute address; calls SHALL resolve through resident or paged dispatch stubs.

#### Scenario: All XBASIC math routines have dispatch entries

- **WHEN** the central table is inspected
- **THEN** dispatch entries exist for `XBASIC_MULTIPLY_INTEGERS`, `XBASIC_DIVIDE_INTEGERS`, `XBASIC_ADD_FLOATS`, `XBASIC_SUBTRACT_FLOATS`, `XBASIC_MULTIPLY_FLOATS`, `XBASIC_DIVIDE_FLOATS`
- **AND** entries exist for `XBASIC_SIN`, `XBASIC_COS`, `XBASIC_TAN`, `XBASIC_ATN`, `XBASIC_EXP`, `XBASIC_LOG`, `XBASIC_SQR`, `XBASIC_RND`, `XBASIC_INT`, `XBASIC_FIX`, `XBASIC_SGN_INT`, `XBASIC_SGN_FLOAT`, `XBASIC_ABS_INT`
- **AND** each used entry is a stub referencing the resident routine or the paged dispatcher

#### Scenario: All XBASIC string routines have dispatch entries

- **WHEN** the central table is inspected
- **THEN** entries exist for `XBASIC_LEFT`, `XBASIC_RIGHT`, `XBASIC_MID`, `XBASIC_MID_ASSIGN`, `XBASIC_INSTR`, `XBASIC_VAL`, `XBASIC_STRING`, `XBASIC_OCT_HEX_BIN`, `XBASIC_INKEY`
- **AND** each used entry is a stub referencing the resident routine or the paged dispatcher

#### Scenario: All XBASIC compare/cast routines have dispatch entries

- **WHEN** the central table is inspected
- **THEN** entries exist for all float comparison routines (`XBASIC_COMPARE_FLOATS_EQ/NE/GT/GE/LT/LE`)
- **AND** entries exist for all string comparison routines (`XBASIC_COMPARE_STRING_WITH_NULBUF_EQ/NE/GT/GE/LT/LE`)
- **AND** entries exist for all cast routines (`XBASIC_CAST_INTEGER_TO_FLOAT`, `XBASIC_CAST_INTEGER_TO_STRING`, `XBASIC_CAST_FLOAT_TO_INTEGER`, `XBASIC_CAST_FLOAT_TO_STRING`, `XBASIC_CAST_STRING_TO_FLOAT`)
- **AND** entries exist for `XBASIC_CONCAT_STRINGS`, `XBASIC_COPY_STRING`, `XBASIC_COPY_STRING_TO_NULBUF`, `XBASIC_POWER_FLOAT_TO_INTEGER`, `XBASIC_POWER_FLOAT_TO_FLOAT`

#### Scenario: All XBASIC graphics routines have dispatch entries

- **WHEN** the central table is inspected
- **THEN** entries exist for `XBASIC_POINT`, `XBASIC_LINE`, `XBASIC_BOX`, `XBASIC_BOXF`, `XBASIC_PSET`, `XBASIC_PAINT`, `XBASIC_CIRCLE`, `XBASIC_CIRCLE2`
- **AND** entries exist for `XBASIC_COLOR_RGB`, `XBASIC_COLOR_SPRITE`, `XBASIC_COLOR_SPRSTR`, `XBASIC_SCREEN_SPRITE`, `XBASIC_SPRITE_ASSIGN`
- **AND** entries exist for `XBASIC_SET_PAGE`, `XBASIC_SET_SCROLL`, `XBASIC_VDP`, `XBASIC_VPEEK`, `XBASIC_VPOKE`

#### Scenario: All XBASIC trap and I/O routines have dispatch entries

- **WHEN** the central table is inspected
- **THEN** entries exist for `XBASIC_TRAP_ON`, `XBASIC_TRAP_OFF`, `XBASIC_TRAP_STOP`, `XBASIC_TRAP_CHECK`
- **AND** entries exist for `XBASIC_SWAP_INTEGER`, `XBASIC_SWAP_STRING`, `XBASIC_SWAP_FLOAT`
- **AND** entries exist for `XBASIC_PRINT_TAB`, `XBASIC_PRINT_INT`, `XBASIC_PRINT_FLOAT`, `XBASIC_PRINT_CRLF`, `WriteParamBCD`

#### Scenario: All BIOS entry points have named constants

- **WHEN** `compiler_hooks.h` is inspected
- **THEN** named `#define` constants exist for `CHSNS` (0x009C), `CHGET` (0x009F), `BEEP` (0x00C0), `FNKSB` (0x00C9), `ERAFNK` (0x00CC), `GTSTCK` (0x00D5), `GTTRIG` (0x00D8), `GTPDL` (0x00DE), `CHGCLR` (0x0062), and `RDPSG` (0x0096)

#### Scenario: Dispatch fixup resolves to a resident or paged stub

- **WHEN** the compiler records a used `DISP_*` index and the packer finalizes placement
- **THEN** the fixup SHALL resolve to a 3-byte resident stub when the routine is resident
- **AND** it SHALL resolve to a paged dispatcher stub when the routine is overflow
- **AND** the resolved stub address SHALL be within the fixed kernel page

#### Scenario: Pre-assembled module reference uses an import fixup

- **WHEN** a pre-assembled kernel module refers to a routine in another module
- **THEN** the reference SHALL be recorded as an import fixup and resolved after placement
- **AND** the module bytes SHALL NOT retain the target routine's assembly address
