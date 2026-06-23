## MODIFIED Requirements

### Requirement: Package compiled code and resources into valid ROM images
As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware or emulators.

ROM build SHALL run only when compiler output is marked as compiled. Kernel/start pages SHALL be initialized before compiled code and resource payloads. Compiled code SHALL be split across 16K pages respecting start-page offsets. Resource map and data blocks SHALL be remapped with segment/address metadata and size checks. The builder SHALL report failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

For MegaROM modes, the kernel SHALL be patched to use the appropriate segment-switch behavior:
- **ASCII8**: Uses 0x7000/0x7800 switch addresses; `MR_CHANGE_SGM` writes two consecutive 8KB segments
- **KonamiSCC/Konami4**: Uses 0x8000/0xA000 switch addresses; `fixKonamiMapper()` patches via dispatch table entries (DISP_KONAMI_PATCH_*)
- **ASCII16**: Uses only 0x7000 switch address for normal operation; `fixAscii16Mapper()` patches via dispatch table entries to rewrite `MR_CHANGE_SGM` with `push af; srl a; ld (0x7000),a; pop af; ret` (SeqReplace, 9 bytes), patch the boot bugfix to a single `ld (0x7000),a` write (1 ByteReplace + 3 NOPs), and patch the 4th/5th OPENMSX autodetection writes to 0x77FF (2× SeqReplace, 3 bytes each) for mapper identification. Total: 7 patch points.

The `fixAscii16Mapper()` method SHALL use dispatch table entries to locate exact instruction addresses in the kernel binary for patching. Patch operations SHALL include SeqReplace (writing an explicit byte sequence), ByteReplace (changing a single port address operand byte), and NOP (writing `0x00` for N bytes). The MR_CHANGE_SGM patch SHALL reuse the existing `konami_patch_sgm_8000` dispatch entry and label.

For all MegaROM modes including ASCII16, `resourceSegment` SHALL use the 8KB-pair convention: `pages.size() * 2`. The kernel's `srl a` handles the 8KB→16KB conversion at runtime, so no builder-level adjustment is needed.

#### Scenario: Build ROM from valid compiled program
- **WHEN** a successful compilation result with non-empty code is used for ROM build
- **THEN** output pages are generated
- **AND** output file is written
- **AND** ROM/code/resource share metrics are calculated

#### Scenario: Build ASCII16 MegaROM with patched kernel
- **WHEN** a compilation result is built in ASCII16 mode
- **THEN** `MR_CHANGE_SGM` SHALL be rewritten with `push af; srl a; ld (0x7000),a; pop af; ret` via dispatch-table SeqReplace
- **AND** the ASCII8 boot bugfix SHALL be patched to a single `ld (0x7000),1` via ByteReplace + NOP operations
- **AND** the 4th and 5th OPENMSX autodetection writes SHALL be patched to 0x77FF for mapper identification
- **AND** `resourceSegment` SHALL equal `pages.size() * 2` (8KB-pair convention, same as other MegaROM modes)
- **AND** ROM is padded to a multiple of 128KB

#### Scenario: Verify ASCII16 kernel patch point count
- **WHEN** an ASCII16 ROM is built
- **THEN** all 7 ASCII16 patch points SHALL be applied (1 SeqReplace MR_CHANGE_SGM + 2 SeqReplace OMSX + 1 ByteReplace + 3 NOP)
- **AND** the patched kernel SHALL contain the `srl a` sequence in MR_CHANGE_SGM, 0x7000 writes from omsx_0-2, 0x77FF writes from omsx_3-4, and NOPs at the expected boot bugfix locations

#### Scenario: Reject plain ROM with resources above 16K page limit
- **WHEN** plain ROM mode is used and resource pages exceed one 16K page
- **THEN** build fails
- **AND** error indicates resource limit exceeded with MegaROM suggestion

#### Scenario: Fail when a resource block exceeds per-block limit
- **WHEN** a resource file block is larger than 16K
- **THEN** resource build fails
- **AND** error reports maximum resource block size exceeded
