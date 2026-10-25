# Virtual Dispatch Table

## Purpose

Defines the virtual compile-time-only dispatch table section occupying the first 0x4000 bytes of `header.bin`. The C++ compiler reads this section to resolve kernel routine addresses at build time via `bin_header_bin[]`, but it is never written to the output ROM. This separates the dispatch oracle from the kernel binary, freeing kernel ROM space.

## Requirements

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
