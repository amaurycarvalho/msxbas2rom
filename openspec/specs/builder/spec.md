## ADDED Requirements

### Requirement: Package compiled code and resources into valid ROM images
As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware or emulators.

ROM build SHALL run only when compiler output is marked as compiled. Kernel/start pages SHALL be initialized before compiled code and resource payloads. Compiled code SHALL be split across 16K pages respecting start-page offsets. Resource map and data blocks SHALL be remapped with segment/address metadata and size checks. The builder SHALL report failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

#### Scenario: Build ROM from valid compiled program
- **WHEN** a successful compilation result with non-empty code is used for ROM build
- **THEN** output pages are generated
- **AND** output file is written
- **AND** ROM/code/resource share metrics are calculated

#### Scenario: Reject plain ROM with resources above 16K page limit
- **WHEN** plain ROM mode is used and resource pages exceed one 16K page
- **THEN** build fails
- **AND** error indicates resource limit exceeded with MegaROM suggestion

#### Scenario: Fail when a resource block exceeds per-block limit
- **WHEN** a resource file block is larger than 16K
- **THEN** resource build fails
- **AND** error reports maximum resource block size exceeded
