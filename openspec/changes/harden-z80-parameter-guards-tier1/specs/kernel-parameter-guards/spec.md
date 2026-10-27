# Spec Delta

## Purpose

Defines runtime validation for kernel command parameters and resource metadata so that invalid dynamic values fail safely instead of indexing fixed tables, executing truncated code, or corrupting RAM.

## ADDED Requirements

### Requirement: Kernel sprite table accesses use the 32-sprite range

Every kernel routine that indexes a 32-entry sprite table SHALL normalize or reject values outside `0..31` before computing the table address. This includes `PUT SPRITE`, `SET SPRITE HITBOX`, sprite pattern/color access, and collision helpers. A value of 32 SHALL NOT address the player buffer or another adjacent work area.

#### Scenario: PUT SPRITE 32 does not corrupt the player buffer
- **WHEN** `PUT SPRITE 32,(100,100)` is executed
- **THEN** the write is normalized or ignored
- **AND** the bytes at `PLYBUF` are unchanged

#### Scenario: SET SPRITE HITBOX 32 does not corrupt the sprite table
- **WHEN** `SET SPRITE HITBOX 32,1,1,1,1` is executed
- **THEN** the write is normalized or ignored
- **AND** the bytes at `SPRTBL` are unchanged

### Requirement: Tile and sprite directions are normalized at runtime

Kernel flip and rotate routines SHALL normalize runtime direction values to the documented `0..2` range. Values outside that range SHALL produce a documented safe result rather than an arbitrary transformation.

#### Scenario: Direction 3 is safe
- **WHEN** `SET TILE FLIP` or `SET SPRITE ROTATE` receives direction `3` from a variable
- **THEN** the routine does not perform an undefined mix of transformations

#### Scenario: Negative direction is safe
- **WHEN** a flip/rotate routine receives a negative direction from a variable
- **THEN** the result is defined and does not index outside the buffer

### Requirement: Resource consumers honor resource lookup failure

Kernel commands that open a resource SHALL check the failure result of the resource lookup before using `HL` or `BC`. This applies to `CMD MTF`, `CMD WRTFNT`, `CMD WRTSPR`, `CMD RESTORE`, `CMD PLYLOAD`, and `CMD WRTVRAM`. Invalid resource numbers SHALL NOT cause an undefined pointer or size to be used.

#### Scenario: Missing resource does not write
- **WHEN** `CMD MTF 9` or `CMD RESTORE 9` refers to a missing resource
- **THEN** the command returns without copying or drawing
- **AND** no VDP or RAM write uses an undefined address

#### Scenario: Short font resource is rejected
- **WHEN** `CMD WRTFNT` receives a decompressed resource shorter than the seven-byte header
- **THEN** no underflowed `LDIRVM` length is used

### Requirement: CMD RUNASM does not execute truncated or invalid resources

`CMD RUNASM` SHALL refuse to execute a resource that is missing, larger than 255 bytes, or not directly executable. It SHALL NOT execute a 255-byte truncation of a larger resource.

#### Scenario: Oversized dynamic resource is not executed
- **WHEN** `CMD RUNASM` is called with a variable selecting a 256-byte resource
- **THEN** the kernel returns without jumping into the truncated copy

#### Scenario: Missing resource is not executed
- **WHEN** `CMD RUNASM` is called with a variable selecting a missing resource
- **THEN** the kernel returns without jumping

### Requirement: PLYSOUND validates effect, channel, and inverted volume

`cmd_plysound` SHALL ignore or reject effect number `0`, channels outside `0..2`, and inverted volumes above `16`. It SHALL NOT index the sound-effect table or channel state with an invalid value.

#### Scenario: Effect zero is not played
- **WHEN** `CMD PLYSOUND 0` is executed
- **THEN** the kernel returns without an out-of-range sound-effect lookup

#### Scenario: Channel above two is safe
- **WHEN** `CMD PLYSOUND 1,3` is executed
- **THEN** no channel state outside the three Arkos channels is overwritten
