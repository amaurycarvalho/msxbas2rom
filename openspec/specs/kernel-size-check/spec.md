# Kernel Size Check

## Purpose

Establishes a dual hard size ceiling for the Z80 kernel binary: `0x8000` bytes (32 KB) total for `header.bin`, with a `0x4000` byte (16 KB) sub-limit on the kernel ROM portion. The kernel occupies the second half of the binary (0x4000–0x7FFF) while the first half (0x0000–0x3FFF) holds the virtual dispatch table. A build-time verification prevents either portion from silently exceeding its budget.

## Requirements

### Requirement: Kernel ROM binary SHALL NOT exceed 0x8000 bytes total, with kernel portion SHALL NOT exceed 0x4000 bytes

The build system SHALL verify at build time that the Z80 kernel binary (`header.bin`) does not exceed `0x8000` bytes (32768 bytes). The actual kernel portion (second half, binary offset 0x4000 through 0x7FFF) SHALL NOT exceed `0x4000` bytes (16384 bytes). If either limit is exceeded, the build SHALL emit an alert identifying the actual size and the excess amount.

#### Scenario: Binary within 0x8000 limit builds silently

- **WHEN** the kernel is compiled and `header.bin` is 32768 bytes or fewer
- **THEN** no alert is emitted and the build proceeds normally

#### Scenario: Binary exceeds 0x8000 limit triggers alert

- **WHEN** the kernel is compiled and `header.bin` exceeds 32768 bytes
- **THEN** the build SHALL emit a visible alert message stating the actual size and how many bytes over the limit the binary is

#### Scenario: Kernel portion within 0x4000 limit

- **WHEN** the kernel is compiled and the second half of `header.bin` (bytes 16384-32767) is 16384 bytes or fewer
- **THEN** no alert regarding kernel portion size is emitted

