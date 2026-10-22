# ROM Builder

## ADDED Requirements

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

### Requirement: Builder SHALL apply module relocations

The builder SHALL copy each relocatable module to its final address and apply every embedded relocation record using the difference between the final base and the assembly base.

#### Scenario: Relocation adjusts internal references
- **WHEN** a module is placed at a final address different from its assembly base
- **THEN** every recorded 16-bit absolute reference SHALL be adjusted by the base delta

#### Scenario: Fixed references remain unchanged
- **WHEN** a module references a fixed kernel, BIOS, or RAM address
- **THEN** that reference SHALL NOT be adjusted by relocation

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

#### Scenario: Successful layout writes the ROM
- **WHEN** all dependencies and budgets are valid
- **THEN** the builder SHALL assemble the ROM artifact with resident modules, overflow modules, resources, and patched metadata

### Requirement: Builder SHALL keep BASIC compilation self-contained

The builder SHALL use only data embedded in `msxbas2rom` for module binaries, manifests, relocation records, and mapper patches. It SHALL NOT invoke an external assembler, linker, or module file during BASIC compilation.

#### Scenario: No external tool is invoked
- **WHEN** the builder assembles a ROM
- **THEN** it SHALL NOT execute pasmo, rasm, a linker, or any external process
- **AND** it SHALL NOT read module or manifest files from disk
