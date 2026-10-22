# Wrapper Dispatch Table

## REMOVED Requirements

### Requirement: Wrapper routines are dispatched via word-pointer table

**Reason**: The table is no longer a virtual compile-time word-pointer table at `0x0000`; dispatch is now a runtime central jump table in the fixed kernel page with selective resident and paged stubs.

**Migration**: Use the runtime central-table and dispatcher requirements in `kernel-module-dispatch`. Existing `DISP_*` indices remain the stable source of dispatch identity.

### Requirement: compiler_hooks.h SHALL define dispatch table at logical address 0x0000

**Reason**: The runtime table lives in the fixed kernel page, and the compiler no longer resolves calls by reading a virtual table at logical `0x0000`.

**Migration**: Resolve `DISP_*` indices through the embedded module manifest and dispatch fixups in `kernel-module-dispatch`.

### Requirement: getKernelCallAddr SHALL handle two distinct address ranges

**Reason**: Kernel calls are resolved through deferred dispatch fixups, not by interpreting a virtual table range or a kernel-page JP.

**Migration**: Record the used `DISP_*` index at emission time and resolve it after the packer assigns stub addresses.

### Requirement: MegaROM addresses SHALL resolve via direct index

**Reason**: MR_CALL, MR_JUMP, and MR_GET_DATA are now ordinary dispatch entries resolved through the same fixup and stub mechanism as every other kernel call.

**Migration**: Emit the corresponding `DISP_MR_*` fixup; the packer resolves it to a resident stub.

## ADDED Requirements

### Requirement: Runtime central table contains selective dispatch stubs

The runtime central table SHALL contain a stub only for each used `DISP_*` entry and its required dependencies. Resident entries SHALL be `jp` stubs; overflow entries SHALL be dispatcher stubs carrying segment and address metadata.

#### Scenario: Resident stub is emitted
- **WHEN** a used dispatch entry resolves to a routine in the fixed kernel page
- **THEN** the table SHALL contain a 3-byte `jp` stub to that routine

#### Scenario: Paged stub is emitted
- **WHEN** a used dispatch entry resolves to an overflow routine
- **THEN** the table SHALL contain a stub that passes the target segment and address to the dispatcher

#### Scenario: Unused entry has no stub
- **WHEN** a `DISP_*` entry is not used by generated code or its dependency closure
- **THEN** the table SHALL NOT contain a stub for that entry

### Requirement: Compiler resolves kernel calls through build-time dispatch fixups

The compiler SHALL keep emitting kernel calls through `addKernelCall(DISP_*)` and conditional variants, but it SHALL resolve them through dispatch fixups after placement instead of reading a direct routine address during code generation.

#### Scenario: Call site stays a 3-byte placeholder
- **WHEN** the compiler emits a kernel call
- **THEN** the call instruction SHALL remain a 3-byte call placeholder
- **AND** the target SHALL be recorded as a dispatch fixup

#### Scenario: Conditional call uses the same fixup
- **WHEN** the compiler emits `addKernelCallNZ(DISP_*)` or another conditional variant
- **THEN** the conditional call target SHALL be resolved by the same dispatch fixup mechanism

#### Scenario: Fixup is applied after packing
- **WHEN** the packer has assigned all stub addresses
- **THEN** every dispatch fixup SHALL be patched to its resolved stub address

### Requirement: DISP indices remain stable across kernel relayouts

The numeric value of every `DISP_*` constant SHALL remain stable when kernel modules are moved, packed, or sent to overflow.

#### Scenario: Relayout does not renumber DISP constants
- **WHEN** the kernel module layout changes between two builds
- **THEN** the `DISP_*` values SHALL be identical
- **AND** only the resolved stub addresses SHALL change
