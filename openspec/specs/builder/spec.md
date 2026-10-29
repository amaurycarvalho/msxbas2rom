# ROM Builder

## Purpose

Defines ROM image building: packaging compiled code and resources into valid plain ROM or MegaROM cartridge images.

## Requirements

### Requirement: Package compiled code and resources into valid ROM images
As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages for MSX hardware or emulators. ROM build SHALL run only for compiled output, splitting code across 16K pages, initializing kernel/start pages first, remapping resources with segment/address metadata and size checks, and failing on invalid output. MegaROM kernels SHALL be patched via dispatch-table entries for each mapper.

#### Scenario: ASCII8 build uses the ASCII8 switch addresses
- **WHEN** a compilation result is built in ASCII8 mode
- **THEN** the kernel SHALL use switch addresses 0x6800/0x7000/0x7800
- **AND** `MR_CHANGE_SGM` SHALL write two consecutive 8KB segments
- **AND** ROM is padded to a multiple of 128KB

#### Scenario: KonamiSCC build applies the Konami patches
- **WHEN** a compilation result is built in KonamiSCC mode
- **THEN** the kernel SHALL be patched with the 0x7000/0x8000/0xA000 switch addresses at all 14 patch points
- **AND** the Konami patches SHALL reuse the same dispatch-table entries as Konami4
- **AND** ROM is padded to a multiple of 128KB

#### Scenario: Dispatch-table patch operations use exact locations
- **WHEN** the builder patches the kernel for a MegaROM mode
- **THEN** it SHALL locate instructions through dispatch-table entries instead of byte scanning
- **AND** it SHALL support SeqReplace, ByteReplace, and NOP operations
- **AND** the `MR_CHANGE_SGM` patch SHALL reuse the `konami_patch_sgm_8000` dispatch entry and label
- **AND** `resourceSegment` SHALL use the 8KB-pair convention `pages.size() * 2`, since the kernel's `srl a` converts to 16KB at runtime

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

### Requirement: Konami mapper patch makes boot mapping unconditional

`fixKonamiMapper()` SHALL NOP the 14-byte `AB` signature guard at
`megarom_ascii8_bug_fix` for both Konami4 and KonamiSCC builds, so the boot bank
mapping always runs. The patch SHALL use a mapper-neutral dispatch alias named
`DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` that resolves to the shared guard address.
ASCII8 and ASCII16 builds SHALL keep the guard unchanged.

#### Scenario: Konami4 build NOPs the shared AB guard

- **WHEN** a compilation result is built in Konami4 mode
- **THEN** the 14 bytes starting at `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` are
  `0x00`
- **AND** the existing Konami4 segment-switch patches remain applied

#### Scenario: KonamiSCC build NOPs the shared AB guard

- **WHEN** a compilation result is built in KonamiSCC mode
- **THEN** the 14 bytes starting at `DISP_MEGAROM_PATCH_BUGFIX_AB_CHECK` are
  `0x00`
- **AND** the existing KonamiSCC segment-switch patches remain applied

#### Scenario: ASCII8 build keeps the AB guard

- **WHEN** a compilation result is built in ASCII8 mode
- **THEN** the 14-byte `AB` guard remains present in the kernel page

#### Scenario: ASCII16 build keeps the AB guard

- **WHEN** a compilation result is built in ASCII16 mode
- **THEN** the 14-byte `AB` guard remains present in the kernel page

#### Scenario: Invalid patch offset fails the build

- **WHEN** the guard dispatch address resolves outside the kernel page
- **THEN** the build fails with a Konami ROM format adjustment error
- **AND** no partial ROM is reported as valid

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
