# Kernel Module Dispatch

## Purpose

Defines runtime dispatch and build-time packing for Z80 kernel modules, including the central jump table, dispatcher ABI, residency policies, embedded relocations, and overflow placement in Plain ROM and MegaROM.

## ADDED Requirements

### Requirement: Runtime central jump table resides in the fixed kernel page

The kernel SHALL expose a central jump table in the fixed `0x4000-0x7FFF` page. Each used dispatch entry SHALL be a stub that transfers control to a resident routine or to the paged dispatcher with segment and address metadata.

#### Scenario: Resident entry uses a direct stub
- **WHEN** a dispatched routine is classified as resident
- **THEN** its central-table entry SHALL be a 3-byte `jp` to the routine
- **AND** no segment switch SHALL occur when the entry is called

#### Scenario: Paged entry uses the dispatcher
- **WHEN** a dispatched routine is classified as overflow
- **THEN** its central-table entry SHALL invoke the dispatcher with the target segment and address
- **AND** the dispatcher SHALL select that segment before calling the routine

#### Scenario: Unused entries are omitted
- **WHEN** a `DISP_*` entry is neither called by generated code nor required by a dependency
- **THEN** the packer SHALL NOT emit a stub for it

### Requirement: Dispatcher preserves the caller ABI and restores the segment

The paged dispatcher SHALL preserve caller argument and result registers, save the current segment, select the target segment, call the routine, restore the previous segment, and return control to the original caller.

#### Scenario: Nested cross-segment call returns to the caller segment
- **WHEN** a paged routine calls a routine in a different segment
- **THEN** the dispatcher SHALL save the caller segment
- **AND** after the callee returns, the dispatcher SHALL restore the caller segment
- **AND** control SHALL return to the instruction after the original call

#### Scenario: Conditional call is safe
- **WHEN** generated code emits a conditional call to a central-table entry
- **THEN** the stub SHALL execute only when the condition is true
- **AND** no inline dispatcher data SHALL be executed when the condition is false

#### Scenario: Interrupt path does not corrupt the active segment
- **WHEN** an interrupt handler switches segments during a paged dispatch
- **THEN** the handler SHALL save and restore the active segment
- **AND** the interrupted routine SHALL resume with its segment mapped

### Requirement: Build-time module packing fills the kernel page first

The builder SHALL reserve fixed regions, compute the used-module dependency closure, place flex modules in free kernel holes first, and place only the remaining overflow outside the fixed kernel page.

#### Scenario: Kernel free space is used before overflow
- **WHEN** used flex modules fit in the free holes of the kernel page
- **THEN** the builder SHALL place them in the kernel page
- **AND** no flex module SHALL be sent to the user page or a MegaROM segment

#### Scenario: Plain ROM overflow stays within the user page
- **WHEN** Plain ROM is selected and some used flex modules do not fit in the kernel page
- **THEN** the builder SHALL place the overflow in `0x8000-0xBFFF` after user code
- **AND** it SHALL fail if start code, user code, and overflow exceed that page

#### Scenario: MegaROM overflow uses segments after user code
- **WHEN** MegaROM is selected and some used flex modules do not fit in the kernel page
- **THEN** the builder SHALL place the overflow in segments allocated after user code
- **AND** it SHALL keep resource and trap segment metadata consistent

### Requirement: Module data and relocations are embedded in msxbas2rom

Module binaries, dependency metadata, exports, placement policies, fixed regions, and relocation tables SHALL be embedded in `msxbas2rom`. BASIC compilation SHALL NOT require an external assembler, linker, or module data file.

#### Scenario: BASIC compilation is self-contained
- **WHEN** a BASIC program is compiled
- **THEN** no external assembler, linker, or module file SHALL be invoked or read
- **AND** placement and relocation SHALL be performed by `msxbas2rom`

#### Scenario: Relocation adjusts module-internal absolute references
- **WHEN** a relocatable module is placed at a final address different from its assembly base
- **THEN** the builder SHALL adjust every recorded relocation by the base delta
- **AND** references to fixed kernel, BIOS, and RAM addresses SHALL remain unchanged

### Requirement: Relocatable modules are isolated from each other

A relocatable module SHALL reference only its own routines and fixed kernel entry points. Cross-module calls SHALL go through the central table or a module export table; a module SHALL NOT contain a direct absolute reference to another module.

#### Scenario: Intra-module call is relocated
- **WHEN** a module calls one of its own routines
- **THEN** the call target SHALL be adjusted by the module's relocation table
- **AND** the module SHALL NOT need to know its final address at assembly time

#### Scenario: Cross-module call uses the central table
- **WHEN** a module needs a routine from another module
- **THEN** it SHALL call the fixed central-table entry
- **AND** it SHALL NOT embed the other module's address

### Requirement: Fixed, flex, and overflow placement policies are explicit

The system SHALL classify routines as fixed resident, flex high priority, flex low priority, or overflow. Fixed routines SHALL remain at designated addresses; flex routines SHALL be packed into kernel free space before overflow placement.

#### Scenario: Player and interrupt-critical routines are fixed
- **WHEN** the packer places the Arkos player body, its timer interrupt handler, or trap/interrupt-critical routines
- **THEN** those routines SHALL remain in the fixed kernel page at designated addresses
- **AND** they SHALL NOT be paged or relocated

#### Scenario: Game graphics are flex high priority
- **WHEN** the packer places PUT SPRITE, PUT TILE, tile/sprite access, or collision routines
- **THEN** it SHALL treat them as a single high-priority flex group
- **AND** it SHALL place them in the kernel page before moving lower-priority groups to overflow

#### Scenario: Low-priority groups move first
- **WHEN** the kernel page cannot hold every used flex group
- **THEN** the packer SHALL move low-priority groups to overflow before moving high-priority groups

### Requirement: Player and Arkos remain fixed-resident

The Arkos player body and its timer interrupt handler SHALL remain in the fixed kernel page at their designated addresses. They SHALL NOT be paged, relocated, or recompiled during BASIC compilation.

#### Scenario: Interrupt executes the player without a page switch
- **WHEN** the timer interrupt plays or mutes a song
- **THEN** the player body SHALL execute from the fixed kernel page
- **AND** no page switch SHALL be introduced to reach the player body

#### Scenario: Song data remains in the mapped window
- **WHEN** the player reads music data during playback
- **THEN** the song resource SHALL remain mapped in the `0x8000-0xBFFF` window
- **AND** the player code SHALL remain mapped at `0x4000-0x7FFF`

### Requirement: Dispatch fixups preserve stable DISP indices

The compiler SHALL record used `DISP_*` indices and emit deferred dispatch fixups. `DISP_*` indices SHALL remain stable across kernel relayouts and SHALL NOT be renumbered by module placement.

#### Scenario: Used dispatch entries are recorded
- **WHEN** generated code calls `addKernelCall(DISP_x)` or a conditional variant
- **THEN** the compiler SHALL record `DISP_x` in the used dispatch set
- **AND** the call site SHALL remain a 3-byte call placeholder until fixup

#### Scenario: Fixup resolves to a resident or paged stub
- **WHEN** the packer finalizes module placement
- **THEN** it SHALL resolve each used `DISP_*` fixup to the corresponding resident or paged stub address

### Requirement: Plain ROM budget diagnostics are actionable

When Plain ROM cannot fit start code, user code, and overflow modules in `0x8000-0xBFFF`, the builder SHALL fail with a diagnostic listing the code size, each used group, and which groups were resident or overflow.

#### Scenario: Overflow does not fit
- **WHEN** Plain ROM placement exceeds the `0x8000-0xBFFF` page
- **THEN** the build SHALL fail
- **AND** the diagnostic SHALL list the code size, used group sizes, and overflow groups
- **AND** it SHALL suggest MegaROM

### Requirement: MegaROM overflow uses 8 KB-pair segment numbers

The builder SHALL allocate MegaROM overflow modules using 8 KB-pair segment numbers and SHALL rely on the mapper-patched segment-switch routine for 16 KB mappers.

#### Scenario: ASCII8 and Konami overflow allocation
- **WHEN** overflow modules are placed in an ASCII8 or Konami MegaROM
- **THEN** each module's stub SHALL carry the 8 KB-pair segment number
- **AND** the mapper-patched switch routine SHALL select the pair

#### Scenario: ASCII16 overflow allocation
- **WHEN** overflow modules are placed in an ASCII16 or ASCII16-X MegaROM
- **THEN** the stub SHALL use the same 8 KB-pair segment numbering
- **AND** the mapper-patched switch routine SHALL convert it to a 16 KB page
