# Architecture

## ADDED Requirements

### Requirement: ADR: Kernel modularization uses build-time packing and runtime dispatch

The kernel SHALL be organized as fixed-resident core code plus relocatable modules. Module placement and relocation SHALL be decided at build time; runtime SHALL only select the segment recorded in the central table. Plain ROM SHALL place overflow in the user page, and MegaROM SHALL place overflow in segments.

#### Scenario: Allocation is build-time
- **WHEN** a program is compiled
- **THEN** the used-module closure and module placement SHALL be computed by the builder
- **AND** the runtime SHALL NOT allocate or relocate kernel modules

#### Scenario: Plain ROM overflow shares the user page
- **WHEN** Plain ROM cannot keep all used flex modules in the kernel page
- **THEN** overflow SHALL be placed in `0x8000-0xBFFF`
- **AND** the build SHALL fail if the combined page budget is exceeded

#### Scenario: MegaROM overflow uses segments
- **WHEN** MegaROM cannot keep all used flex modules in the kernel page
- **THEN** overflow SHALL be placed in segments after user code
- **AND** mapper-patched segment switching SHALL be used at runtime

### Requirement: ADR: Player and Arkos remain fixed-resident

The Arkos player body and timer interrupt handler SHALL remain in the fixed kernel page at their designated addresses. They SHALL NOT be paged, relocated, or recompiled during BASIC compilation because playback runs from the interrupt while the song resource occupies the `0x8000` window.

#### Scenario: Interrupt context stays page-safe
- **WHEN** the timer interrupt executes the player
- **THEN** the player code SHALL already be mapped in the fixed page
- **AND** no page switch SHALL be required to execute it

#### Scenario: Song data window is preserved
- **WHEN** the player reads song data
- **THEN** the song resource SHALL remain mapped in `0x8000-0xBFFF`
- **AND** the player code SHALL remain in `0x4000-0x7FFF`

### Requirement: ADR: Relocatable modules are isolated and self-describing

A relocatable module SHALL know only its own routines and fixed kernel entry points. It SHALL carry embedded relocation metadata for its internal absolute references, and cross-module calls SHALL use the central table or a module export table.

#### Scenario: Module does not embed another module's address
- **WHEN** a module needs a routine from another module
- **THEN** it SHALL call a fixed central-table entry
- **AND** it SHALL NOT contain that routine's absolute address

#### Scenario: Module relocation is self-describing
- **WHEN** the builder places a module
- **THEN** it SHALL use the module's embedded relocation records
- **AND** it SHALL NOT require an external linker or module file

### Requirement: ADR: BASIC compilation remains self-contained

The `msxbas2rom` binary SHALL contain the module library, manifests, relocation records, and mapper patch metadata needed to compile a BASIC program. BASIC compilation SHALL NOT depend on an external assembler, linker, or data file.

#### Scenario: Self-contained BASIC build
- **WHEN** a user compiles a BASIC program
- **THEN** all kernel module data and metadata SHALL come from the `msxbas2rom` binary
- **AND** no external tool or file SHALL be required
