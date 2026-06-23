## MODIFIED Requirements

### Requirement: Package compiled code and resources into valid ROM images
As a ROM producer, the system SHALL package compiled code and resources into valid plain ROM or MegaROM pages so that the output cartridge image can run on target MSX hardware or emulators.

ROM build SHALL run only when compiler output is marked as compiled. Kernel/start pages SHALL be initialized before compiled code and resource payloads. Compiled code SHALL be split across 16K pages respecting start-page offsets. Resource map and data blocks SHALL be remapped with segment/address metadata and size checks. The builder SHALL report failure for invalid output states (empty code, oversized resources, unsupported plain-ROM resource size).

For MegaROM modes (ASCII8, KonamiSCC, and Konami4), the kernel SHALL be patched to use the appropriate segment-switch addresses. ASCII8 uses 0x6800/0x7000/0x7800. KonamiSCC and Konami4 both use 0x7000/0x8000/0xA000: the 0x6000-0x7FFF page switch is patched from 0x6800 to 0x7000, the 0x8000-0x9FFF page switch is patched from 0x7000 to 0x8000, and the 0xA000-0xBFFF page switch is patched from 0x7800 to 0xA000. The `fixKonamiMapper()` method SHALL apply the binary patch for both KonamiSCC and Konami4 modes using dispatch table entries (DISP_KONAMI_PATCH_*) to locate exact instruction addresses, instead of byte scanning. All 14 segment-switch points across the kernel SHALL be patched, including those in `MR_CHANGE_SGM`, `OPENMSX_EMULATOR_AUTODETECTION`, `megarom_ascii8_bug_fix`, and `verify.slots.test.megaram`.

#### Scenario: Build ROM from valid compiled program
- **WHEN** a successful compilation result with non-empty code is used for ROM build
- **THEN** output pages are generated
- **AND** output file is written
- **AND** ROM/code/resource share metrics are calculated

#### Scenario: Build Konami4 MegaROM with patched kernel
- **WHEN** a compilation result is built in Konami4 mode
- **THEN** the kernel binary is patched with Konami4 segment-switch addresses (0x7000 for the 0x6000-0x7FFF page, 0x8000 for the 0x8000-0x9FFF page, 0xA000 for the 0xA000-0xBFFF page) at all 14 patch points via dispatch table lookup
- **AND** all 14 segment-switch locations are modified in the kernel
- **AND** ROM is padded to a multiple of 128KB (8 pages)

#### Scenario: Reject plain ROM with resources above 16K page limit
- **WHEN** plain ROM mode is used and resource pages exceed one 16K page
- **THEN** build fails
- **AND** error indicates resource limit exceeded with MegaROM suggestion

#### Scenario: Fail when a resource block exceeds per-block limit
- **WHEN** a resource file block is larger than 16K
- **THEN** resource build fails
- **AND** error reports maximum resource block size exceeded
