# resource-compression Specification

## Purpose

Defines safe, size-bounded compression of binary resource blobs so that every supported resource is packaged without buffer overflow, silent heap corruption, or format ambiguity.

## Requirements

### Requirement: Compress all supported non-empty resource blobs

The resource compressor SHALL accept every non-empty binary blob from 1 byte through the builder's per-resource limit and SHALL produce a complete compressed payload for each.

#### Scenario: Single-byte blob is compressed

- **WHEN** a 1-byte binary resource is loaded for compressed storage
- **THEN** compression succeeds
- **AND** the resource is marked as packed
- **AND** `packedSize` is greater than zero

#### Scenario: Small multi-byte blobs are compressed

- **WHEN** a binary resource of 2, 3, 4, 5, or 6 bytes is loaded for compressed storage
- **THEN** compression succeeds for each size
- **AND** no bounds assertion, crash, or heap corruption occurs

#### Scenario: Empty blob is rejected

- **WHEN** a 0-byte resource is loaded for compressed storage
- **THEN** compression fails
- **AND** the error identifies the resource as empty

#### Scenario: Resource at the per-resource limit is compressed

- **WHEN** a binary resource of exactly 8192 bytes is loaded for compressed storage
- **THEN** compression succeeds
- **AND** the resource is marked as packed

### Requirement: Compressed output is bounded by input size

For an `n`-byte input, the resource compressor SHALL NOT emit more than `n + ceil((n + 36) / 8)` bytes. The bound covers the format header, literal control bits, terminator, and payload bytes.

#### Scenario: Incompressible data respects the bound

- **WHEN** incompressible data of any supported size from 1 through 8192 bytes is compressed
- **THEN** the packed size is less than or equal to `n + ceil((n + 36) / 8)`

#### Scenario: Small inputs respect the bound

- **WHEN** 1-byte, 5-byte, and 6-byte blobs are compressed
- **THEN** their packed sizes do not exceed the bound
- **AND** compression does not write outside the produced output

### Requirement: Compression returns an exact-size owned output

The resource compressor SHALL return the compressed payload in an owned output whose size equals the number of emitted bytes. Callers SHALL NOT supply or size a raw destination buffer.

#### Scenario: Output size is exact

- **WHEN** compression succeeds
- **THEN** the returned payload length equals `packedSize`
- **AND** no trailing uninitialized or padding bytes are exposed

#### Scenario: Caller does not pre-size the destination

- **WHEN** a caller requests compression
- **THEN** it does not provide a raw destination capacity
- **AND** the compressor alone determines the output allocation

#### Scenario: Compression failure is reported

- **WHEN** compression cannot produce output, for example because the input is empty
- **THEN** the operation reports failure to the caller
- **AND** it does not abort as part of normal input validation

### Requirement: Existing compressed format and payloads are preserved

The change SHALL NOT alter the compressed byte format or the decoder contract. Payloads for inputs that already compressed successfully SHALL remain byte-identical.

#### Scenario: Existing payload bytes are unchanged

- **WHEN** a resource that previously compressed successfully is compressed after the change
- **THEN** its compressed byte sequence matches the previous implementation

#### Scenario: Kernel decoder remains compatible

- **WHEN** a ROM built with the new compressor is run
- **THEN** the existing kernel decompression routine accepts the payload
- **AND** the decompressed bytes match the original resource

### Requirement: Chunked resource compression validates block limits

The chunked resource loader SHALL compress chunks of at most 200 bytes and SHALL store each compressed block with a one-byte size prefix. It SHALL reject a configuration or payload whose compressed block would exceed the one-byte size limit instead of truncating it.

#### Scenario: Incompressible 200-byte chunk fits

- **WHEN** a 200-byte incompressible chunk is compressed
- **THEN** its packed size is at most 255 bytes
- **AND** the stored block contains the size byte followed by exactly that many packed bytes

#### Scenario: Block size overflow is rejected

- **WHEN** a chunk configuration could produce a packed block larger than 255 bytes
- **THEN** the loader fails with a clear diagnostic
- **AND** it does not truncate the block size or write past the block

#### Scenario: Block count and total size are preserved

- **WHEN** a chunked resource is loaded
- **THEN** the block count and packed size describe every emitted block
- **AND** the resulting resource remains loadable by the existing chunk decoder
