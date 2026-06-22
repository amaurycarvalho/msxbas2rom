## ADDED Requirements

### Requirement: Wrapper table covers all kernel routines called from C++
The word-pointer dispatch table `wrapper_routines_map_table` in `20_runtime.asm` SHALL contain a `dw` entry for every Z80 kernel routine that the C++ compiler emits calls to. No kernel routine in the `src/infrastructure/kernel/asm/src/header/` subtree that is reachable from compiled user code SHALL be called via a hardcoded absolute address.

#### Scenario: All XBASIC math routines have dispatch entries
- **WHEN** the wrapper table is inspected
- **THEN** dispatch entries exist for `XBASIC_MULTIPLY_INTEGERS`, `XBASIC_DIVIDE_INTEGERS`, `XBASIC_ADD_FLOATS`, `XBASIC_SUBTRACT_FLOATS`, `XBASIC_MULTIPLY_FLOATS`, `XBASIC_DIVIDE_FLOATS`
- **AND** entries exist for `XBASIC_SIN`, `XBASIC_COS`, `XBASIC_TAN`, `XBASIC_ATN`, `XBASIC_EXP`, `XBASIC_LOG`, `XBASIC_SQR`, `XBASIC_RND`, `XBASIC_INT`, `XBASIC_FIX`, `XBASIC_SGN_INT`, `XBASIC_SGN_FLOAT`, `XBASIC_ABS_INT`
- **AND** each entry is a `dw` followed by a Z80 label referencing the actual routine

#### Scenario: All XBASIC string routines have dispatch entries
- **WHEN** the wrapper table is inspected
- **THEN** entries exist for `XBASIC_LEFT`, `XBASIC_RIGHT`, `XBASIC_MID`, `XBASIC_MID_ASSIGN`, `XBASIC_INSTR`, `XBASIC_VAL`, `XBASIC_STRING`, `XBASIC_OCT_HEX_BIN`, `XBASIC_INKEY`
- **AND** each entry is a `dw` followed by a Z80 label referencing the actual routine

#### Scenario: All XBASIC compare/cast routines have dispatch entries
- **WHEN** the wrapper table is inspected
- **THEN** entries exist for all float comparison routines (`XBASIC_COMPARE_FLOATS_EQ/NE/GT/GE/LT/LE`)
- **AND** entries exist for all string comparison routines (`XBASIC_COMPARE_STRING_WITH_NULBUF_EQ/NE/GT/GE/LT/LE`)
- **AND** entries exist for all cast routines (`XBASIC_CAST_INTEGER_TO_FLOAT`, `XBASIC_CAST_INTEGER_TO_STRING`, `XBASIC_CAST_FLOAT_TO_INTEGER`, `XBASIC_CAST_FLOAT_TO_STRING`, `XBASIC_CAST_STRING_TO_FLOAT`)
- **AND** entries exist for `XBASIC_CONCAT_STRINGS`, `XBASIC_COPY_STRING`, `XBASIC_COPY_STRING_TO_NULBUF`, `XBASIC_POWER_FLOAT_TO_INTEGER`, `XBASIC_POWER_FLOAT_TO_FLOAT`

#### Scenario: All XBASIC graphics routines have dispatch entries
- **WHEN** the wrapper table is inspected
- **THEN** entries exist for `XBASIC_POINT`, `XBASIC_LINE`, `XBASIC_BOX`, `XBASIC_BOXF`, `XBASIC_PSET`, `XBASIC_PAINT`, `XBASIC_CIRCLE`, `XBASIC_CIRCLE2`
- **AND** entries exist for `XBASIC_COLOR_RGB`, `XBASIC_COLOR_SPRITE`, `XBASIC_COLOR_SPRSTR`, `XBASIC_SCREEN_SPRITE`, `XBASIC_SPRITE_ASSIGN`
- **AND** entries exist for `XBASIC_SET_PAGE`, `XBASIC_SET_SCROLL`, `XBASIC_VDP`, `XBASIC_VPEEK`, `XBASIC_VPOKE`

#### Scenario: All XBASIC trap and I/O routines have dispatch entries
- **WHEN** the wrapper table is inspected
- **THEN** entries exist for `XBASIC_TRAP_ON`, `XBASIC_TRAP_OFF`, `XBASIC_TRAP_STOP`, `XBASIC_TRAP_CHECK`
- **AND** entries exist for `XBASIC_SWAP_INTEGER`, `XBASIC_SWAP_STRING`, `XBASIC_SWAP_FLOAT`
- **AND** entries exist for `XBASIC_PRINT_TAB`, `XBASIC_PRINT_INT`, `XBASIC_PRINT_FLOAT`, `XBASIC_PRINT_CRLF`, `WriteParamBCD`

#### Scenario: All BIOS entry points have named constants
- **WHEN** `compiler_hooks.h` is inspected
- **THEN** named `#define` constants exist for `CHSNS` (0x009C), `CHGET` (0x009F), `BEEP` (0x00C0), `FNKSB` (0x00C9), `ERAFNK` (0x00CC), `GTSTCK` (0x00D5), `GTTRIG` (0x00D8), `GTPDL` (0x00DE), `CHGCLR` (0x0062), and `RDPSG` (0x0096)

### Requirement: wrapper_routines_map_table uses symbolic labels exclusively
The wrapper table in `20_runtime.asm` SHALL use symbolic Z80 labels for all entries, never hardcoded numeric addresses. The table position within the kernel (relative ordering of entries) SHALL remain stable across kernel recompilations to maintain backward compatibility of the C++ `DISP_*` dispatch index constants.

#### Scenario: Table entries are symbolic labels
- **WHEN** the wrapper table source (`20_runtime.asm`) is read
- **THEN** all `dw` entries reference Z80 labels (e.g., `dw XBASIC_MULTIPLY_INTEGERS`)
- **AND** no entry uses a bare hex address (e.g., `dw 0x7631`)

### Requirement: compiler_hooks.h offers a DISP_ index for every dispatch entry
For every `dw` entry in the `wrapper_routines_map_table`, `compiler_hooks.h` SHALL contain a corresponding `#define DISP_*` constant with a sequential integer index starting from 0, matching the entry's position in the table. The `DISP_ENTRIES` constant SHALL equal the total number of dispatch entries.

#### Scenario: DISP_ indexes are contiguous and unbroken
- **WHEN** `compiler_hooks.h` is read
- **THEN** every `#define` from `DISP_castParamFloatInt` (0) through the last entry is reachable by a contiguous sequence of integers
- **AND** `DISP_ENTRIES` equals the total number of `DISP_*` constants defined in the dispatch section
- **AND** all `DISP_*` values are within the range `[0, DISP_ENTRIES - 1]`

#### Scenario: getKernelCallAddr resolves DISP_ entries correctly
- **WHEN** `getKernelCallAddr` is called with address `0x0000 + index * 2` where `index < DISP_ENTRIES`
- **THEN** the 2-byte word at the computed address in `bin_header_bin` encodes a valid kernel routine address (0x4000-0x7FFF)
- **AND** the resolved target address equals the actual routine address in the kernel binary
