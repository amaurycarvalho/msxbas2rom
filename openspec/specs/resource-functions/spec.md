# resource-functions Specification

## Purpose

Defines the behavior of the assembled-resource accessor functions `RESOURCE(n)` and `RESOURCESIZE(n)`.

## Requirements

### Requirement: RESOURCE(n) loads the resource into free RAM and returns its RAM address

The `RESOURCE(n)` function SHALL return the address of a RAM copy of resource `n`, not the ROM resource pointer. It SHALL place the copy at the first free RAM address reported by `HEAP()` and return that same address. `RESOURCE(n)` SHALL return `0` when the resource does not exist.

The amount copied SHALL be `min(resource_size, FRE())`. When the resource is larger than `FRE()`, the copy SHALL be truncated to `FRE()` bytes and the function SHALL still return the destination address. When the resource is smaller than or equal to `FRE()`, the whole resource SHALL be copied.

The resource bytes SHALL be copied verbatim (no decompression, no transformation).

#### Scenario: Existing resource that fits

- **WHEN** `RESOURCE(n)` is evaluated for a resource `n` that exists and whose size is less than or equal to `FRE()`
- **THEN** the whole resource SHALL be copied to the address returned by `HEAP()`
- **AND** the function SHALL return the address returned by `HEAP()`

#### Scenario: Resource larger than free memory

- **WHEN** `RESOURCE(n)` is evaluated for a resource `n` whose size is greater than `FRE()`
- **THEN** exactly `FRE()` bytes SHALL be copied to the address returned by `HEAP()`
- **AND** the function SHALL return the address returned by `HEAP()` (truncated copy, non-zero)

#### Scenario: Resource does not exist

- **WHEN** `RESOURCE(n)` is evaluated for a resource number `n` that is not present in the resource map
- **THEN** the function SHALL return `0`
- **AND** no byte of the heap area SHALL be modified

#### Scenario: Loaded copy is readable by compiled BASIC

- **WHEN** `RESOURCE(n)` returns a non-zero address `A` for a resource that exists
- **THEN** `PEEK(A)`, `PEEK(A+1)`, ... SHALL expose the resource bytes as stored in ROM, in the same order, up to the copied length

### Requirement: RESOURCE(n) is a single-slot loader

`RESOURCE(n)` SHALL NOT allocate memory and SHALL NOT advance `HEAPSTR`. Each evaluation SHALL write to the same `HEAP()` address and overwrite any previous copy. Callers SHALL consume the copy before the next `RESOURCE(n)` call or any operation that reuses the heap area.

`RESOURCE(n)` SHALL NOT reserve the top 256 bytes of the heap used by BASIC temporary strings; the approved copy length is `min(resource_size, FRE())`, which may overlap that reserve.

#### Scenario: Repeated calls overwrite the same address

- **WHEN** `RESOURCE(n)` is evaluated twice in succession
- **THEN** both calls SHALL return the same `HEAP()` address
- **AND** the second copy SHALL overwrite the first

#### Scenario: Heap start is not advanced

- **WHEN** `RESOURCE(n)` completes
- **THEN** `HEAPSTR` SHALL be unchanged
- **AND** `FRE()` SHALL be unchanged

### Requirement: RESOURCE(n) is defined by delegation to CMD RSCTORAM

`RESOURCE(n)` SHALL be equivalent to executing `CMD RSCTORAM n, HEAP(), 0, 0, FRE()` and returning the routine's `HL` result. It SHALL copy `min(resource_size, FRE())` bytes starting at resource byte `0`.

#### Scenario: Equivalent to CMD RSCTORAM

- **WHEN** `RESOURCE(n)` is evaluated
- **THEN** it SHALL produce the same RAM contents and the same returned address as `CMD RSCTORAM n, HEAP(), 0, 0, FRE()`
- **AND** the returned value SHALL be `0` when the command would return `0`

### Requirement: RESOURCESIZE(n) keeps its current behavior

The `RESOURCESIZE(n)` function SHALL remain unchanged. It SHALL return the size in bytes of resource `n`, and `0` when the resource does not exist.

#### Scenario: Valid resource size

- **WHEN** `RESOURCESIZE(n)` is evaluated for an existing resource `n`
- **THEN** it SHALL return the resource size in bytes

#### Scenario: Invalid resource size

- **WHEN** `RESOURCESIZE(n)` is evaluated for a resource number not present in the resource map
- **THEN** it SHALL return `0`
