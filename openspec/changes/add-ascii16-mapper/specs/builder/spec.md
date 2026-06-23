## MODIFIED Requirements

### Requirement: Package compiled code and resources into valid ROM images
As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware or emulators.

ROM build SHALL run only when compiler output is marked as compiled. Kernel/start pages SHALL be initialized before compiled code and resource payloads. Compiled code SHALL be split across 16K pages respecting start-page offsets. Resource map and data blocks SHALL be remapped with segment/address metadata and size checks. The builder SHALL report failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

For MegaROM modes, the kernel SHALL be patched to use the appropriate segment-switch behavior:
- **ASCII8**: Uses 0x7000/0x7800 switch addresses; `MR_CHANGE_SGM` writes two consecutive 8KB segments
- **KonamiSCC/Konami4**: Uses 0x8000/0xA000 switch addresses; `fixKonamiMapper()` patches via dispatch table entries (DISP_KONAMI_PATCH_*)
- **ASCII16**: Uses only 0x7000 switch address; `fixAscii16Mapper()` patches via dispatch table entries (DISP_ASCII16_PATCH_*) to NOP out the second segment-switch write (0x7800) and surrounding `inc a`/`dec a` in `MR_CHANGE_SGM`, NOP invalid ASCII8-only writes in the boot bugfix, and convert invalid port writes to 0x7000 where appropriate

The `fixAscii16Mapper()` method SHALL use dispatch table entries to locate exact instruction addresses in the kernel binary for patching. Patch operations SHALL include both byte-replace (changing port address operands) and NOP-sequence replacement (replacing instruction sequences with 0x00).

For ASCII16 mode, the `resourceSegment` SHALL be calculated as `pages.size()` (1:1 page-to-segment mapping) instead of `pages.size() * 2`.

#### Scenario: Build ROM from valid compiled program
- **WHEN** a successful compilation result with non-empty code is used for ROM build
- **THEN** output pages are generated
- **AND** output file is written
- **AND** ROM/code/resource share metrics are calculated

#### Scenario: Build ASCII16 MegaROM with patched kernel
- **WHEN** a compilation result is built in ASCII16 mode
- **THEN** the kernel binary is patched via dispatch table entries to NOP the second segment-switch in MR_CHANGE_SGM
- **AND** the ASCII8 boot bugfix is patched to use only 0x7000 writes
- **AND** `resourceSegment` equals `pages.size()` (not `pages.size() * 2`)
- **AND** ROM is padded to a multiple of 128KB

#### Scenario: Verify ASCII16 kernel patch point count
- **WHEN** an ASCII16 ROM is built
- **THEN** all ASCII16 patch points defined via DISP_ASCII16_PATCH_* SHALL be applied
- **AND** the patched kernel SHALL contain NOPs (0x00) at the expected locations in MR_CHANGE_SGM and the boot bugfix

#### Scenario: Reject plain ROM with resources above 16K page limit
- **WHEN** plain ROM mode is used and resource pages exceed one 16K page
- **THEN** build fails
- **AND** error indicates resource limit exceeded with MegaROM suggestion

#### Scenario: Fail when a resource block exceeds per-block limit
- **WHEN** a resource file block is larger than 16K
- **THEN** resource build fails
- **AND** error reports maximum resource block size exceeded
