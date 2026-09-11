## MODIFIED Requirements

### Requirement: Package compiled code and resources into valid ROM images

As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware or emulators.

ROM build SHALL run only when compiler output is marked as compiled. Kernel/start pages SHALL be initialized before compiled code and resource payloads. Compiled code SHALL be split across 16K pages respecting start-page offsets. Resource map and data blocks SHALL be remapped with segment/address metadata and size checks. For ASCII16 and ASCII16-X modes, segment values in the resource map and ROM header SHALL be stored as 2-byte (WORD) values. MegaROM size limit SHALL be 2048K for ASCII8/Konami mappers, 4096K for ASCII16, and 8192K for ASCII16-X. The builder SHALL report failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

#### Scenario: Build ROM from valid compiled program

- **WHEN** a successful compilation result with non-empty code is used for ROM build
- **THEN** output pages are generated
- **AND** output file is written
- **AND** ROM/code/resource share metrics are calculated

#### Scenario: Reject plain ROM with resources above 16K page limit

- **WHEN** plain ROM mode is used and resource pages exceed one 16K page
- **THEN** build fails
- **AND** error indicates resource limit exceeded with MegaROM suggestion

#### Scenario: Fail when a resource block exceeds per-block limit

- **WHEN** a resource file block is larger than 16K
- **THEN** resource build fails
- **AND** error reports maximum resource block size exceeded

#### Scenario: Resource map stores 2-byte segments for ASCII16 mode

- **WHEN** a ROM is built in ASCII16 or ASCII16-X mode
- **THEN** each resource map entry is 6 bytes (WORD offset + WORD segment + WORD size)
- **AND** the segment field stores the full 16-bit value

#### Scenario: ROM header stores 2-byte resource map segment

- **WHEN** a ROM is built in any MegaROM mode
- **THEN** the ROM header at offset 0x800D stores the resource map segment as a WORD (2 bytes)
- **AND** BASTEXT is at offset 0x800F

#### Scenario: MegaROM limit differs by mapper

- **WHEN** a ROM exceeds the per-mapper size limit during resource map building
- **THEN** build fails with error indicating the mode-specific limit (2048K for ASCII8/Konami, 4096K for ASCII16, 8192K for ASCII16-X)

#### Scenario: Build Konami4 MegaROM with patched kernel
- **WHEN** a compilation result is built in Konami4 mode
- **THEN** the kernel binary is patched with Konami4 segment-switch addresses (0x7000 for the 0x6000-0x7FFF page, 0x8000 for the 0x8000-0x9FFF page, 0xA000 for the 0xA000-0xBFFF page) at all 14 patch points via dispatch table lookup
- **AND** all 14 segment-switch locations are modified in the kernel
- **AND** ROM is padded to a multiple of 128KB (8 pages)

#### Scenario: Build ASCII16 MegaROM with patched kernel
- **WHEN** a compilation result is built in ASCII16 mode
- **THEN** `MR_CHANGE_SGM` SHALL be rewritten with `push af; srl a; ld (0x7000),a; pop af; ret` via dispatch-table SeqReplace
- **AND** the ASCII8 boot bugfix SHALL be patched to a single `ld (0x7000),1` via ByteReplace + NOP operations
- **AND** the 4th and 5th OPENMSX autodetection writes SHALL be patched to 0x77FF for mapper identification
- **AND** `resourceSegment` SHALL equal `pages.size() * 2` (8KB-pair convention, same as other MegaROM modes)
- **AND** ROM is padded to a multiple of 128KB

#### Scenario: Verify ASCII16 kernel patch point count
- **WHEN** an ASCII16 ROM is built
- **THEN** all 7 ASCII16 patch points SHALL be applied (1 SeqReplace MR_CHANGE_SGM + 2 SeqReplace OMSX + 1 ByteReplace + 3 NOPs)
- **AND** the patched kernel SHALL contain the `srl a` sequence in MR_CHANGE_SGM, 0x7000 writes from omsx_0-2, 0x77FF writes from omsx_3-4, and NOPs at the expected boot bugfix locations

#### Scenario: Build ASCII16-X MegaROM with patched kernel
- **WHEN** a compilation result is built in ASCII16-X mode
- **THEN** `MR_CHANGE_SGM` SHALL be rewritten with `push af; srl a; ld (0x7000),a; pop af; ret` via dispatch-table SeqReplace
- **AND** the ASCII8 boot bugfix SHALL be patched to a single `ld (0x7000),1` via ByteReplace + NOP operations
- **AND** the 14-byte bugfix `AB` check SHALL be NOPped via dispatch table entry `DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK`
- **AND** all 7 ASCII16 patch points SHALL be applied
- **AND** `resourceSegment` SHALL equal `pages.size() * 2` (8KB-pair convention, same as other MegaROM modes)
- **AND** ROM is padded to a multiple of 128KB

#### Scenario: ASCII16-X signature written at ROM offset 0x0010
- **WHEN** an ASCII16-X ROM is built
- **THEN** `pages[0].data() + 0x0010` through `pages[0].data() + 0x0017` SHALL contain `A S C I I 1 6 X` (hex 41 53 43 49 49 31 36 58)
- **AND** the signature SHALL NOT be present for non-ASCII16X modes (bytes remain 0x00 from ASM `ds 8`)

## ADDED Requirements

### Requirement: Kernel binary is patched for 16-bit segment routines in ASCII16/ASCII16-X modes

For ASCII16 and ASCII16-X compile modes, the ROM builder SHALL patch the kernel binary to insert 16-bit segment switch routines (`MR_CHANGE_SGM_16` or `MR_CHANGE_SGM_16X`) and redirect segment-switch dispatch calls. The patching SHALL use the existing dispatch-table-based mechanism with new DISP entries and patch definitions. The kernel binary SHALL remain unmodified for ASCII8, Konami4, and KonamiSCC modes. The patching SHALL handle NOP insertion where additional code space is needed, absorbed by the kernel end filler.

#### Scenario: ASCII16 patch inserts MR_CHANGE_SGM_16 routine

- **WHEN** a ROM is built in ASCII16 mode
- **THEN** the kernel is patched with SeqReplace to insert the `MR_CHANGE_SGM_16` byte sequence
- **AND** dispatch calls to segment switch routines use the 16-bit variants

#### Scenario: ASCII16-X patch inserts MR_CHANGE_SGM_16X routine

- **WHEN** a ROM is built in ASCII16-X mode
- **THEN** the kernel is patched to insert the `MR_CHANGE_SGM_16X` byte sequence with address encoding logic
- **AND** the ASCII16-X signature "ASCII16X" is written at ROM offset 0x0010

#### Scenario: ASCII8 kernel is NOT patched for 16-bit routines

- **WHEN** a ROM is built in ASCII8 mode
- **THEN** the kernel binary is not modified with 16-bit segment routines
- **AND** segment switches use the original 8-bit `MR_CHANGE_SGM`
