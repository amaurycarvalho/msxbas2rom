# US-004 - Builder (ROM + Resources)

## Story
As a ROM producer, I need the builder to package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware/emulators.

## Acceptance Criteria
- ROM build runs only when compiler output is marked as compiled.
- Kernel/start pages are initialized before compiled code and resource payloads.
- Compiled code is split across 16K pages respecting start-page offsets.
- Resource map and data blocks are remapped with segment/address metadata and size checks.
- Builder reports failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

## BDD Scenarios
### Scenario 1: Build ROM from valid compiled program
Given a successful compilation result with non-empty code
When ROM build is executed
Then output pages are generated
And output file is written
And ROM/code/resource share metrics are calculated

### Scenario 2: Reject plain ROM with resources above 16K page limit
Given plain ROM mode and resource pages exceeding one 16K page
When ROM write is executed
Then build fails
And error indicates resource limit exceeded with MegaROM suggestion

### Scenario 3: Fail when a resource block exceeds per-block limit
Given a resource file block larger than 16K
When resource map build is executed
Then resource build fails
And error reports maximum resource block size exceeded
