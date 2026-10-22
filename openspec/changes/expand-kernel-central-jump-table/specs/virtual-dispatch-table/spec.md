# Virtual Dispatch Table

## REMOVED Requirements

### Requirement: header.bin SHALL be 0x8000 bytes with two logical halves

**Reason**: The runtime kernel image is now composed from resident and overflow modules, and the former virtual first half is replaced by a build-time manifest/oracle section.

**Migration**: Use the embedded module manifest described in `kernel-module-dispatch`; the packer composes the runtime kernel page.

### Requirement: Dispatch table SHALL occupy logical address 0x0000

**Reason**: The runtime central table lives in the fixed kernel page and is generated selectively after placement.

**Migration**: Resolve dispatch entries through the manifest and the runtime central table.

### Requirement: DEFS padding SHALL bridge the first half to logical address 0x4000

**Reason**: The build-time manifest is not required to be exactly 16 KB or to bridge into a fixed runtime page with `DEFS`.

**Migration**: Generate the manifest as embedded metadata; the packer places the runtime kernel page independently.

### Requirement: C++ compiler SHALL use first half for table lookups only

**Reason**: The compiler now uses the embedded manifest and deferred dispatch fixups, not direct lookups in a virtual first half.

**Migration**: Record `DISP_*` fixups and resolve them after packing.

### Requirement: 5 alignment nops SHALL be removed

**Reason**: The virtual table address alignment constraint no longer exists because the table is runtime-generated.

**Migration**: No action; the runtime table layout is controlled by the packer.

## ADDED Requirements

### Requirement: header.bin provides a build-time kernel manifest oracle

The kernel build SHALL emit a build-time manifest section describing dispatch exports, module ownership, residency policy, dependencies, and relocation data. The manifest SHALL NOT be copied into the runtime ROM as executable code.

#### Scenario: Manifest is separate from runtime kernel page
- **WHEN** the ROM is assembled
- **THEN** only the resident kernel page and overflow module images SHALL be copied to the ROM
- **AND** the manifest section SHALL NOT be emitted as executable kernel bytes

#### Scenario: Manifest lists dispatch ownership
- **WHEN** the manifest is inspected for a `DISP_*` index
- **THEN** it SHALL identify the exporting module
- **AND** it SHALL identify whether the module is fixed, flex, or relocatable

### Requirement: Embedded manifest supplies exports and relocations

The embedded module manifest SHALL provide every export entry point and every relocation record required by the packer. The compiler and builder SHALL NOT depend on absolute `header.symbols.asm` addresses for flex routines.

#### Scenario: Export address comes from the manifest
- **WHEN** the packer builds a central-table stub
- **THEN** it SHALL obtain the routine's module and offset from the manifest
- **AND** it SHALL compute the final address after placement

#### Scenario: Relocation record comes from the manifest
- **WHEN** a module is placed at a final address different from its assembly base
- **THEN** the packer SHALL apply the relocation records embedded for that module

### Requirement: Compiler uses the kernel boundary, not fixed addresses

The compiler SHALL obtain dispatch identity and module metadata through the read-only kernel boundary. It SHALL NOT embed absolute addresses of flex kernel routines in generated code.

#### Scenario: Flex routine address is not baked into user code
- **WHEN** the compiler emits a call to a flex kernel routine
- **THEN** the call site SHALL contain a dispatch fixup
- **AND** the final address SHALL be supplied by the packer

#### Scenario: Deterministic kernel images remain testable
- **WHEN** a test supplies a deterministic kernel manifest image
- **THEN** generated dispatch fixups and mapper patches SHALL use that image
- **AND** no global generated-kernel array SHALL be required
