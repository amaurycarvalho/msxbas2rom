## ADDED Requirements

### Requirement: Dispatch table includes 16-bit segment switch routine entries

The word-pointer dispatch table `wrapper_routines_map_table` in `20_runtime.asm` SHALL contain `dw` entries for the new 16-bit segment switch routines: `MR_CHANGE_SGM_16`, `MR_CHANGE_SGM_16X`, `MR_JUMP_16`, `MR_CALL_16`, and `MR_GET_DATA_16`. Corresponding `DISP_*` constants SHALL be defined in `compiler_hooks.h` with contiguous indices. The `DISP_ENTRIES` constant SHALL be updated to reflect the new total.

#### Scenario: MR_CHANGE_SGM_16 has a dispatch entry

- **WHEN** the wrapper table is inspected
- **THEN** an entry `dw MR_CHANGE_SGM_16` exists
- **AND** `DISP_MR_CHANGE_SGM_16` is defined in `compiler_hooks.h` with index 223

#### Scenario: MR_CHANGE_SGM_16X has a dispatch entry

- **WHEN** the wrapper table is inspected
- **THEN** an entry `dw MR_CHANGE_SGM_16X` exists
- **AND** `DISP_MR_CHANGE_SGM_16X` is defined with index 224

#### Scenario: MR_JUMP_16 has a dispatch entry

- **WHEN** the wrapper table is inspected
- **THEN** an entry `dw MR_JUMP_16` exists
- **AND** `DISP_MR_JUMP_16` is defined with index 225

#### Scenario: MR_CALL_16 has a dispatch entry

- **WHEN** the wrapper table is inspected
- **THEN** an entry `dw MR_CALL_16` exists
- **AND** `DISP_MR_CALL_16` is defined with index 226

#### Scenario: MR_GET_DATA_16 has a dispatch entry

- **WHEN** the wrapper table is inspected
- **THEN** an entry `dw MR_GET_DATA_16` exists
- **AND** `DISP_MR_GET_DATA_16` is defined with index 227

#### Scenario: DISP_ENTRIES reflects new total

- **WHEN** `compiler_hooks.h` is read
- **THEN** `DISP_ENTRIES` equals 228 (was 223)
- **AND** all DISP values from 0 to 227 are reachable contiguously

### Requirement: 16-bit segment switch routines coexist with 8-bit routines

The kernel binary SHALL contain both 8-bit segment switch routines (`MR_CHANGE_SGM`, `MR_JUMP`, `MR_CALL`, `MR_GET_DATA`, `MR_GET_BYTE`) and 16-bit variants (`MR_CHANGE_SGM_16`, `MR_JUMP_16`, `MR_CALL_16`, `MR_GET_DATA_16`). The 8-bit routines SHALL remain unchanged and functional. The ROM builder patching SHALL determine which set is active at build time based on the compile mode.

#### Scenario: 8-bit routines remain functional after 16-bit additions

- **WHEN** a ROM is built in ASCII8 mode
- **THEN** the kernel's 8-bit `MR_CHANGE_SGM` routine is called for segment switches
- **AND** the 16-bit routines are present in the binary but not called

#### Scenario: 16-bit routines are called for ASCII16 mode

- **WHEN** a ROM is built in ASCII16 mode
- **THEN** segment switches use the 16-bit `MR_CHANGE_SGM_16` routine
- **AND** the kernel's 8-bit routines are not called for segment switching

### Requirement: Kernel workarea layout accommodates 16-bit segment variables

The kernel RAM workarea starting at 0xC010 SHALL have `RSCMAPSG` as a 2-byte variable (was 1 byte). All subsequent workarea variables (`RSCMAPT1`, `PLYSGTM`, `BASMEM`) SHALL be shifted by 1 byte to accommodate the expansion. The `def_BASMEM` constant in `compiler_hooks.h` SHALL be updated to reflect the new address.

#### Scenario: RSCMAPSG occupies 2 bytes

- **WHEN** the kernel workarea layout is inspected
- **THEN** `RSCMAPSG` is defined as a 2-byte variable (e.g., `equ 0xC033` with the next variable at `+2`)
- **AND** the kernel reads and writes both bytes when accessing the resource map segment

#### Scenario: BASMEM is shifted by 1 byte

- **WHEN** `compiler_hooks.h` is read
- **THEN** `def_BASMEM` is updated to the new address (shifted +1 from current)
- **AND** the C++ compiler uses the correct address for RAM allocation

### Requirement: resource.address reads 2-byte segment from resource map

The kernel routine `resource.address` in `80_resources.asm` SHALL read a 2-byte (WORD) segment value from the resource map entry, matching the new 6-byte-per-entry format. The routine SHALL pass the full 16-bit segment value to `MR_CHANGE_SGM_16` (for ASCII16 modes) or use only the low byte (for ASCII8/Konami modes).

#### Scenario: Resource map entry read extracts 2-byte segment

- **WHEN** `resource.address` reads a resource entry from the map
- **THEN** it reads 2 bytes for the segment number using 16-bit register pair operations
- **AND** the segment value is correctly passed to the active segment switch routine

#### Scenario: Resource map count and size extraction remain correct with 6-byte entries

- **WHEN** `resource.address` calculates the entry offset (count * 5 → count * 6)
- **THEN** the multiplication is updated to `x 6` from the current `x 5`
- **AND** all fields (offset, segment, size) are correctly extracted

## MODIFIED Requirements

### Requirement: compiler_hooks.h offers a DISP_ index for every dispatch entry

For every `dw` entry in the `wrapper_routines_map_table`, `compiler_hooks.h` SHALL contain a corresponding `#define DISP_*` constant with a sequential integer index starting from 0, matching the entry's position in the table. The `DISP_ENTRIES` constant SHALL equal the total number of dispatch entries, including the new 16-bit segment switch routines.

#### Scenario: DISP_ indexes are contiguous and unbroken

- **WHEN** `compiler_hooks.h` is read
- **THEN** every `#define` from `DISP_castParamFloatInt` (0) through `DISP_MR_GET_DATA_16` (227) is reachable by a contiguous sequence of integers
- **AND** `DISP_ENTRIES` equals 228
- **AND** all `DISP_*` values are within the range `[0, 227]`

#### Scenario: getKernelCallAddr resolves DISP_ entries correctly

- **WHEN** `getKernelCallAddr` is called with address `0x0000 + index * 2` where `index < DISP_ENTRIES`
- **THEN** the 2-byte word at the computed address in `bin_header_bin` encodes a valid kernel routine address (0x4000-0x7FFF)
- **AND** the resolved target address equals the actual routine address in the kernel binary
