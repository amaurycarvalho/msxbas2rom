# Kernel Resource Protocol

## Purpose

Define the canonical HL register protocol for passing resource numbers between callers (C++ compiler, kernel CMD/USR handlers) and the `resource.*` kernel routines. Ensure resource numbers are never written to or read from the DAC work area (`0xF7F6`), and document preservation requirements around `resource.open` which destroys H on the non-MegaROM path.

## ADDED Requirements

### Requirement: resource.open accepts resource number via HL

The `resource.open` routine in `60_bios_helpers.asm` SHALL accept the resource number in the HL register pair. The routine SHALL NOT read or consume the resource number (it is reserved for future use). The routine SHALL NOT read the DAC work area (`0xF7F6`) for the resource number.

The non-MegaROM path of `resource.open` (`select_rsc_on_page_0`) SHALL set `ld h, 000h` as a side effect of page-zero slot selection. Callers that need the resource number after `resource.open` SHALL preserve HL via stack (`push hl` / `pop hl`). The MegaROM path via `MR_CHANGE_SGM` SHALL preserve HL.

#### Scenario: Non-MegaROM caller preserves HL across resource.open

- **WHEN** a non-MegaROM system calls `resource.open` with `hl = resource number`
- **AND** the caller needs the resource number after `resource.open` returns
- **THEN** the caller SHALL wrap the call with `push hl` / `pop hl` or equivalent stack preservation
- **AND** HL after the call SHALL hold the original resource number (restored from stack)

#### Scenario: MegaROM caller with HL preservation

- **WHEN** a MegaROM system calls `resource.open` with `hl = resource number`
- **THEN** MR_CHANGE_SGM SHALL NOT modify HL
- **AND** the resource number in HL SHALL survive the call without stack preservation

#### Scenario: resource.open does not read DAC

- **WHEN** `resource.open` is called with any value in HL
- **THEN** the routine SHALL NOT execute `ld a, (DAC)`, `ld hl, (DAC)` or any instruction reading the DAC work area
- **AND** the comment header SHALL state `; in: hl = resource number (reserved)`

### Requirement: resource.address accepts resource number via HL

The `resource.address` routine in `80_resources.asm` SHALL accept the resource number in the HL register pair. The routine SHALL internally preserve HL into BC before calling `resource.count` (which overwrites HL with the resource map start address). After preservation, the routine SHALL use BC for resource number validation and the `add hl, bc` multiplication step, keeping the existing output contract unchanged (`hl = resource address, bc = resource size, a = segment`).

#### Scenario: resource.address translates HL to BC internally

- **WHEN** `resource.address` is called with `hl = resource number N`
- **THEN** the routine SHALL execute `push hl` / `call resource.count` / `pop bc` to copy the resource number into BC
- **AND** the resource number validation (`cp b` / `cp c`) SHALL use BC as the source of the resource number
- **AND** the offset calculation (`add hl, bc` for ×5) SHALL reference BC as the resource number

#### Scenario: resource.address returns unchanged output contract

- **WHEN** `resource.address` is called with a valid resource number (0 ≤ N < resource count)
- **THEN** `hl` SHALL contain the resource data address
- **AND** `bc` SHALL contain the resource size in bytes
- **AND** `a` SHALL contain the segment number (0 if no MegaROM)

### Requirement: resource.get_data accepts resource number via HL

The `resource.get_data` routine in `80_resources.asm` SHALL accept the resource number in the HL register pair. The routine SHALL preserve HL across the internal `resource.open` call via `push hl` / `pop hl`, then pass HL to `resource.address`.

#### Scenario: resource.get_data preserves HL across resource.open

- **WHEN** `resource.get_data` is called with `hl = resource number`
- **THEN** the routine SHALL execute `push hl` / `call resource.open` / `pop hl` before calling `resource.address`
- **AND** `resource.address` SHALL receive the original resource number in HL

### Requirement: resource.open_and_get_address accepts resource number via HL

The `resource.open_and_get_address` routine in `60_bios_helpers.asm` SHALL accept the resource number in the HL register pair. The routine SHALL preserve HL across `resource.open` via `push hl` / `pop hl`, then jump to `resource.address` with HL intact. The routine SHALL NOT read `(DAC)` for the resource number.

#### Scenario: resource.open_and_get_address uses HL not DAC

- **WHEN** `resource.open_and_get_address` is called with `hl = resource number`
- **THEN** the routine SHALL execute `push hl` / `call resource.open` / `pop hl` / `jp resource.address`
- **AND** the instruction `ld bc, (DAC)` SHALL NOT appear in the routine

### Requirement: All kernel CMD handlers pass resource number via HL

Every kernel CMD handler in `31_cmd.asm` that consumes a resource number SHALL receive it via the HL register pair from the C++ compiler generated code. Handlers SHALL NOT read `(DAC)` to obtain the resource number. Handlers that call `resource.get_data`, `resource.address`, or `resource.open_and_get_address` SHALL pass the resource number directly in HL without intermediate memory writes.

#### Scenario: cmd_play passes HL directly to resource.get_data

- **WHEN** the kernel `cmd_play` handler is entered with `hl = resource number`
- **THEN** the handler SHALL call `resource.get_data` with HL intact
- **AND** the instruction `ld bc, (DAC)` SHALL NOT appear in `cmd_play`

#### Scenario: cmd_rsctoram passes HL directly without DAC write

- **WHEN** the kernel `cmd_rsctoram` handler is entered with `hl = resource number`, `de = dest address`, `a = pletter flag`
- **THEN** the handler SHALL call `resource.open_and_get_address` with HL intact
- **AND** the instruction `ld (DAC), hl` SHALL NOT appear in `cmd_rsctoram`

### Requirement: C++ compiler emits resource number via HL without DAC write

The C++ compiler SHALL NOT emit `cpu.addLdiiHL(def_DAC)` for any resource-consuming kernel call. Since the Z80 expression evaluator already places the result in HL, the compiler SHALL emit only the `addKernelCall(DISP_*)` instruction after expression evaluation, passing the resource number directly in HL.

#### Scenario: CMD PLAY compiler handler does not write to DAC

- **WHEN** the compiler processes a `CMD PLAY <expr>` statement
- **THEN** the generated Z80 code SHALL contain `ld hl, <resource number>` (from expression evaluation)
- **AND** the generated code SHALL NOT contain `ld (0xF7F6), hl`
- **AND** the generated code SHALL contain `call <cmd_play>` (via addKernelCall)

### Requirement: Non-resource CMD parameters use PARM1 instead of DAC

CMD handlers that use DAC for parameters that are NOT resource numbers (turbo flag, player loop flag, subsong number, sound effect number) SHALL read those parameters from `PARM1` (`0xF6E8`) instead of `DAC` (`0xF7F6`). The C++ compiler SHALL emit `cpu.addLdiiHL(def_PARAM1)` instead of `cpu.addLdiiHL(def_DAC)` for these handlers.

#### Scenario: cmd_turbo reads flag from PARM1

- **WHEN** the kernel `cmd_turbo` handler is entered
- **THEN** the handler SHALL execute `ld a, (PARM1)` (address `0xF6E8`) to read the turbo flag
- **AND** the instruction `ld a, (DAC)` (address `0xF7F6`) SHALL NOT appear in `cmd_turbo`

#### Scenario: cmd_plysong reads subsong from PARM1

- **WHEN** the kernel `cmd_plysong` handler is entered
- **THEN** the handler SHALL execute `ld a, (PARM1)` to read the subsong number
- **AND** the instruction `ld a, (DAC)` SHALL NOT appear in `cmd_plysong`

### Requirement: BLOAD block pointer scratch uses PARM1 instead of DAC

The `cmd_screen_load` and `XBASIC_BLOAD` routines SHALL use `PARM1` and `PARM1+2` as scratch storage for the block address pointer and segment during the BLOAD loop, instead of `DAC` and `DAC+2`.

#### Scenario: cmd_screen_load stores block pointer in PARM1

- **WHEN** `cmd_screen_load` calls `resource.open_and_get_address` and reads the first block header
- **THEN** the routine SHALL store the block address in `(PARM1)` (`0xF6E8`) and the segment in `(PARM1+2)` (`0xF6EA`)
- **AND** `ARG` SHALL hold the block count (unchanged)
- **AND** the instructions `ld (DAC), hl` and `ld (DAC+2), a` SHALL NOT appear in `cmd_screen_load`

#### Scenario: XBASIC_BLOAD.get_next_block reads block pointer from PARM1

- **WHEN** `XBASIC_BLOAD.get_next_block` executes its loop body
- **THEN** the routine SHALL read the block address via `ld hl, (PARM1)` and the segment via `ld a, (PARM1+2)`
- **AND** the routine SHALL update the block address via `ld (PARM1), de` and the segment via `ld (PARM1+2), a`
- **AND** no reference to `(DAC)` or `(DAC+2)` SHALL appear as block pointer storage
