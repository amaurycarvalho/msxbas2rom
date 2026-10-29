# resource-commands Specification

## Purpose

Defines the behavior of the `CMD RSCTORAM` memory command, including its optional `offset` and `size` parameters for copying a bounded byte range of a resource into RAM.

## Requirements

### Requirement: CMD RSCTORAM accepts an optional byte offset and copy size

`CMD RSCTORAM <resource>, <dest>[, <pletter>[, <offset>[, <size>]]]` SHALL copy bytes from the resource to RAM. When `offset` is omitted it SHALL default to `0`. When `size` is omitted it SHALL default to the number of resource bytes remaining after `offset`. A `size` of `0`, whether omitted or explicitly supplied, SHALL select the full remaining resource (`resource_size - offset`). When `size` is non-zero, the number of bytes actually copied SHALL be `min(size, resource_size - offset)`.

#### Scenario: Only resource and destination given

- **WHEN** `CMD RSCTORAM <resource>, <dest>` is executed
- **THEN** the entire resource SHALL be copied to `<dest>`, starting at the resource's first byte

#### Scenario: Offset and size given

- **WHEN** `CMD RSCTORAM <resource>, <dest>, 0, <offset>, <size>` is executed with non-zero `size` and `offset + size <= resource_size`
- **THEN** exactly `size` bytes SHALL be copied to `<dest>`
- **AND** the first copied byte SHALL be resource byte `offset`

#### Scenario: Size larger than the remaining resource

- **WHEN** `CMD RSCTORAM <resource>, <dest>, 0, <offset>, <size>` is executed with non-zero `size` and `offset + size > resource_size`
- **THEN** only `resource_size - offset` bytes SHALL be copied
- **AND** the copy SHALL NOT read past the end of the resource

#### Scenario: Size of zero selects the full remaining resource

- **WHEN** `CMD RSCTORAM <resource>, <dest>, 0, <offset>, 0` is executed
- **THEN** all bytes from `offset` to the end of the resource SHALL be copied
- **AND** this SHALL be equivalent to omitting `size`

#### Scenario: Offset at or beyond the end of the resource

- **WHEN** `CMD RSCTORAM <resource>, <dest>, 0, <offset>, <size>` is executed and `offset >= resource_size`
- **THEN** no byte SHALL be written to RAM
- **AND** `HL` SHALL be `0`

### Requirement: pletter decompression ignores offset and size

When `pletter = 1` the `offset` and `size` parameters SHALL be ignored, and the whole compressed resource SHALL be decompressed to `<dest>` as before.

#### Scenario: pletter with offset and size supplied

- **WHEN** `CMD RSCTORAM <resource>, <dest>, 1, <offset>, <size>` is executed
- **THEN** the whole compressed resource SHALL be decompressed to `<dest>`
- **AND** `offset` and `size` SHALL NOT affect the result

### Requirement: cmd_rsctoram returns the destination address or zero

The kernel routine backing `CMD RSCTORAM` SHALL return `HL = <dest>` on success, and `HL = 0` when the resource does not exist or when nothing is copied (for example, `offset` at or beyond the resource end). The destination value SHALL be preserved across the copy and the resource close. This return value SHALL be available to callers that invoke the routine as a function-like operation.

#### Scenario: Successful copy returns destination

- **WHEN** `CMD RSCTORAM` copies at least one byte
- **THEN** `HL` SHALL equal the destination address

#### Scenario: Missing resource returns zero

- **WHEN** `CMD RSCTORAM` is executed with a resource number not present in the resource map
- **THEN** `HL` SHALL equal `0`
- **AND** no byte SHALL be written to RAM
- **AND** the resource SHALL be closed before returning

#### Scenario: Nothing copied returns zero

- **WHEN** `CMD RSCTORAM` computes an effective copy length of zero
- **THEN** `HL` SHALL equal `0`
- **AND** no byte SHALL be written to RAM
