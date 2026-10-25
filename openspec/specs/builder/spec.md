# ROM Builder

## Purpose

Defines ROM image building: packaging compiled code and resources into valid plain ROM or MegaROM cartridge images.

## Requirements

### Requirement: Package compiled code and resources into valid ROM images
As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware or emulators.

ROM build SHALL run only when compiler output is marked as compiled. Kernel/start pages SHALL be initialized before compiled code and resource payloads. Compiled code SHALL be split across 16K pages respecting start-page offsets. Resource map and data blocks SHALL be remapped with segment/address metadata and size checks. The builder SHALL report failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

For MegaROM modes, the kernel SHALL be patched to use the appropriate segment-switch behavior:
- **ASCII8**: Uses 0x6800/0x7000/0x7800 switch addresses; `MR_CHANGE_SGM` writes two consecutive 8KB segments
- **KonamiSCC/Konami4**: Uses 0x7000/0x8000/0xA000 switch addresses; `fixKonamiMapper()` patches via dispatch table entries (DISP_KONAMI_PATCH_*)
- **ASCII16**: Uses only 0x7000 switch address for normal operation; `fixAscii16Mapper()` patches via dispatch table entries to rewrite `MR_CHANGE_SGM` with `push af; srl a; ld (0x7000),a; pop af; ret` (SeqReplace, 9 bytes), patch the boot bugfix to a single `ld (0x7000),a` write (1 ByteReplace + 3 NOPs), and patch the 4th/5th OPENMSX autodetection writes to 0x77FF (2× SeqReplace, 3 bytes each) for mapper identification. Total: 7 patch points.
- **ASCII16-X**: Uses the identical kernel patches as ASCII16 (same 7 patch points). The `fixAscii16Mapper()` method SHALL apply the same patches when `compileMode` is `ASCII16X`. Additionally, for `ASCII16X` mode only, the builder SHALL: (1) write the 8-byte signature `"ASCII16X"` at ROM offset 0x0010 in `pages[0]`; (2) NOP out the 14-byte `AB` signature check (`ld a,(0x8000); cp 0x41; jr nz; ld a,(0x8001); cp 0x42; jr nz`) via dispatch table entry `DISP_ASCII16X_PATCH_BUGFIX_AB_CHECK`.

The `fixKonamiMapper()` method SHALL apply the binary patch for both KonamiSCC and Konami4 modes using dispatch table entries (DISP_KONAMI_PATCH_*) to locate exact instruction addresses, instead of byte scanning. All 14 segment-switch points across the kernel SHALL be patched, including those in `MR_CHANGE_SGM`, `OPENMSX_EMULATOR_AUTODETECTION`, `megarom_ascii8_bug_fix`, and `verify.slots.test.megaram`.

The `fixAscii16Mapper()` method SHALL use dispatch table entries to locate exact instruction addresses in the kernel binary for patching. Patch operations SHALL include SeqReplace (writing an explicit byte sequence), ByteReplace (changing a single port address operand byte), and NOP (writing `0x00` for N bytes). The MR_CHANGE_SGM patch SHALL reuse the existing `konami_patch_sgm_8000` dispatch entry and label. For all MegaROM modes including ASCII16 and ASCII16X, `resourceSegment` SHALL use the 8KB-pair convention: `pages.size() * 2`. The kernel's `srl a` handles the 8KB→16KB conversion at runtime, so no builder-level adjustment is needed.

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

### Requirement: Builder SHALL pack kernel modules into resident and overflow regions

The ROM builder SHALL reserve fixed kernel regions, compute the used-module dependency closure, place flex modules in free kernel holes first, and place only overflow in the Plain ROM user page or MegaROM segments.

#### Scenario: Flex modules fit in kernel free holes
- **WHEN** all used flex modules fit in the free holes of the kernel page
- **THEN** the builder SHALL place them in the kernel page
- **AND** no overflow region SHALL be allocated for kernel modules

#### Scenario: Kernel overflow is placed outside the fixed page
- **WHEN** some used flex modules do not fit in the kernel page
- **THEN** the builder SHALL place only the non-fitting modules in the overflow region
- **AND** it SHALL keep the remaining modules resident

#### Scenario: Fixed player region is preserved
- **WHEN** the builder packs any module
- **THEN** it SHALL treat the Arkos player body and timer interrupt handler as a reserved fixed region
- **AND** it SHALL NOT overwrite or relocate them

### Requirement: Builder SHALL apply module relocations and cross-module fixups

The builder SHALL copy each relocatable module to its final address, apply every embedded relocation record using the difference between the final base and the assembly base, and resolve every cross-module import fixup after placement.

#### Scenario: Relocation adjusts internal references
- **WHEN** a module is placed at a final address different from its assembly base
- **THEN** every recorded 16-bit absolute reference SHALL be adjusted by the base delta

#### Scenario: Fixed references remain unchanged
- **WHEN** a module references a fixed kernel, BIOS, or RAM address
- **THEN** that reference SHALL NOT be adjusted by relocation

#### Scenario: Cross-module import resolves after placement
- **WHEN** a module has an import fixup for a call, conditional call, jump, or address-of reference
- **THEN** the builder SHALL patch it with the final resolved central-table stub or routine address
- **AND** the module image SHALL NOT retain its assembly-time placeholder

#### Scenario: Unresolved inbound reference fails the build
- **WHEN** a module would be relocated or forced to overflow while another module still references its assembly address
- **THEN** the build SHALL fail
- **AND** the diagnostic SHALL name the target module and the referencing module

### Requirement: Builder SHALL generate selective central-table stubs

The builder SHALL generate a resident `jp` stub or a paged dispatcher stub only for each used `DISP_*` entry and its dependency closure.

#### Scenario: Resident routine receives a direct stub
- **WHEN** a used dispatch entry resolves to a resident routine
- **THEN** the builder SHALL write a 3-byte `jp` stub in the central table

#### Scenario: Overflow routine receives a dispatcher stub
- **WHEN** a used dispatch entry resolves to an overflow routine
- **THEN** the builder SHALL write a dispatcher stub with the target segment and address

#### Scenario: Unused routine receives no stub
- **WHEN** a dispatch entry is not used and is not a dependency
- **THEN** the builder SHALL NOT write a stub for it

### Requirement: Builder SHALL validate module dependencies and placement budgets

Before assembling the ROM, the builder SHALL verify that every used module and dependency is available, every module fits its assigned region, and the final layout respects the resident, Plain ROM, and mapper budgets.

#### Scenario: Missing dependency fails the build
- **WHEN** a used module requires a module that is not in the embedded module library
- **THEN** the build SHALL fail
- **AND** the diagnostic SHALL identify the missing module and its dependent

#### Scenario: Placement budget violation fails the build
- **WHEN** a module does not fit its assigned hole, slot, or segment
- **THEN** the build SHALL fail with the module name, required size, and available size

#### Scenario: Fixed page-unsafe module cannot be overflowed
- **WHEN** the build asks to force overflow for a fixed resource-owning module
- **THEN** the build SHALL fail
- **AND** the diagnostic SHALL identify the module and state that fixed modules cannot be relocated

#### Scenario: Successful layout writes the ROM
- **WHEN** all dependencies and budgets are valid
- **THEN** the builder SHALL assemble the ROM artifact with resident modules, overflow modules, resources, and patched metadata

### Requirement: Builder SHALL keep BASIC compilation self-contained

The builder SHALL use only data embedded in `msxbas2rom` for module binaries, manifests, relocation records, and mapper patches. It SHALL NOT invoke an external assembler, linker, or module file during BASIC compilation.

#### Scenario: No external tool is invoked
- **WHEN** the builder assembles a ROM
- **THEN** it SHALL NOT execute pasmo, rasm, a linker, or any external process
- **AND** it SHALL NOT read module or manifest files from disk
