## MODIFIED Requirements

### Requirement: Package compiled code and resources into valid ROM images
As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware or emulators.

ROM build SHALL run only when compiler output is marked as compiled. Kernel/start pages SHALL be initialized before compiled code and resource payloads. Compiled code SHALL be split across 16K pages respecting start-page offsets. Resource map and data blocks SHALL be remapped with segment/address metadata and size checks. The builder SHALL report failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

For MegaROM modes, the kernel SHALL be patched to use the appropriate segment-switch behavior:
- **ASCII8**: Uses 0x6800/0x7000/0x7800 switch addresses; `MR_CHANGE_SGM` writes two consecutive 8KB segments
- **KonamiSCC/Konami4**: Uses 0x7000/0x8000/0xA000 switch addresses; `fixKonamiMapper()` patches via dispatch table entries (DISP_KONAMI_PATCH_*)
- **ASCII16**: Uses only 0x7000 switch address for normal operation; `fixAscii16Mapper()` patches via dispatch table entries to rewrite `MR_CHANGE_SGM` with `push af; srl a; ld (0x7000),a; pop af; ret` (SeqReplace, 9 bytes), patch the boot bugfix to a single `ld (0x7000),a` write (1 ByteReplace + 3 NOPs), and patch the 4th/5th OPENMSX autodetection writes to 0x77FF (2× SeqReplace, 3 bytes each). Total: 7 patch points.
- **ASCII16-X**: Uses the identical kernel patches as ASCII16. The `fixAscii16Mapper()` method SHALL apply the same 7 dispatch-table patches when `compileMode` is `ASCII16X`. Additionally, for `ASCII16X` mode only, the builder SHALL: (1) write the 8-byte signature `"ASCII16X"` (`41 53 43 49 49 31 36 58`) at ROM offset 0x0010 in `pages[0]`; (2) NOP out the 14-byte `AB` signature check at `megarom_ascii8_bug_fix` (`ld a,(0x8000); cp 0x41; jr nz; ld a,(0x8001); cp 0x42; jr nz`) via dispatch table entry `DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK` (index 223), ensuring the bugfix always runs. The 8-byte reservation in the kernel header is unconditional (via ASM `ds 8`); only the fill value differs by mode (0x00 for non-ASCII16X, signature bytes for ASCII16X).

The `fixAscii16Mapper()` method SHALL use dispatch table entries to locate exact instruction addresses in the kernel binary for patching. Patch operations SHALL include SeqReplace (writing an explicit byte sequence), ByteReplace (changing a single port address operand byte), and NOP (writing `0x00` for N bytes). The signature write is a fixed-offset operation at ROM offset 0x0010 and does not use the dispatch table.

For all MegaROM modes, `resourceSegment` SHALL use the 8KB-pair convention: `pages.size() * 2`. The kernel's `srl a` handles the 8KB→16KB conversion at runtime, so no builder-level adjustment is needed for any mapper.

#### Scenario: Build ROM from valid compiled program
- **WHEN** a successful compilation result with non-empty code is used for ROM build
- **THEN** output pages are generated
- **AND** output file is written
- **AND** ROM/code/resource share metrics are calculated

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

#### Scenario: Reject plain ROM with resources above 16K page limit
- **WHEN** plain ROM mode is used and resource pages exceed one 16K page
- **THEN** build fails
- **AND** error indicates resource limit exceeded with MegaROM suggestion

#### Scenario: Fail when compiled code exceeds maximum size
- **WHEN** the compiled code buffer reaches the `COMPILE_CODE_SIZE` limit (1 MB / 64 pages) during compilation in any MegaROM mode
- **THEN** the compiler SHALL stop emitting code and report an error indicating the limit was exceeded
- **AND** the ROM builder SHALL receive an empty code size and fail with a clear error message

#### Scenario: Fail when a resource block exceeds per-block limit
- **WHEN** a resource file block is larger than 16K
- **THEN** resource build fails
- **AND** error reports maximum resource block size exceeded
