## ADDED Requirements

### Requirement: Kernel ROM size SHALL NOT exceed 0x4000 bytes
The build system SHALL verify at build time that the Z80 kernel ROM binary (`header.bin`) does not exceed `0x4000` bytes (16384 bytes). If the kernel exceeds this limit, the build SHALL emit an alert identifying the actual size and the excess amount.

#### Scenario: Kernel within limit builds silently
- **WHEN** the kernel is compiled and `header.bin` is 16384 bytes or fewer
- **THEN** no alert is emitted and the build proceeds normally

#### Scenario: Kernel exceeds limit triggers alert
- **WHEN** the kernel is compiled and `header.bin` exceeds 16384 bytes
- **THEN** the build SHALL emit a visible alert message stating the actual size and how many bytes over the limit the kernel is
