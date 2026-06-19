## ADDED Requirements

### Requirement: Kernel assembly routines preserve behavioral compatibility after optimization
All Z80 assembly routines in `infrastructure/kernel/asm/src/header/` SHALL produce identical output for identical input after optimization. The ROM binary SHALL be byte-for-byte identical except for filler padding bytes.

#### Scenario: ROM binary identity after optimization
- **WHEN** the kernel is compiled before and after optimizations
- **THEN** `header.bin` differs only in `BASIC_KUN_START_FILLER` padding bytes (range `$` to `0x6481` and `0x7FFE` to `0x8000`), and `ARKOS_PLAYER` area (0x6481–0x6BF6) is byte-identical

#### Scenario: All integration tests pass unchanged
- **WHEN** the integration test suite runs against the optimized kernel
- **THEN** all existing `.bas` test files compile without errors

### Requirement: Free space in BASIC_KUN_START_FILLER increases
The `BASIC_KUN_START_FILLER` region SHALL contain at least 200 additional free bytes compared to the pre-optimization baseline.

#### Scenario: Filler space growth
- **WHEN** the kernel is compiled after optimizations
- **THEN** the `DEFS` directive at `BASIC_KUN_START_FILLER` fills at least 200 more bytes with `0x00` than before optimization

### Requirement: Completely commented-out dead code is removed
Lines that are fully commented out and have zero active references anywhere in the codebase SHALL be removed from source files.

#### Scenario: BDOS fallback block removal
- **WHEN** the optimization is complete
- **THEN** the commented-out `"FILE HANDLING BDOS IMPLEMENTATIONS"` section (lines 487–576 of `34_file_handling.asm`) no longer exists in the source

#### Scenario: VDP_wait dead code removal
- **WHEN** the optimization is complete
- **THEN** the commented-out `VDP_wait` routine (lines 528–541 of `60_bios_helpers.asm`) no longer exists in the source

### Requirement: cmd_mute calls GICINI exactly once
The `cmd_mute` routine in `31_cmd.asm` SHALL call the `GICINI` initialization routine exactly once, not twice.

#### Scenario: Single GICINI call
- **WHEN** `cmd_mute` is invoked
- **THEN** `GICINI` is called exactly once via `call`, followed by `ei` and `ret`

### Requirement: verify.slots.test.ram restores MegaROM to mode 0
The `verify.slots.test.ram` routine in `60_bios_helpers.asm` SHALL write `0x00` to port `0x8E` before returning, ensuring the MegaROM mapper is in mode 0 (block change ON).

#### Scenario: MegaROM mode restoration
- **WHEN** `verify.slots.test.ram` completes its RAM test
- **THEN** the value written to port `0x8E` is `0x00` (mode 0), regardless of the test value used

### Requirement: resource.get_data respects actual resource size
The `resource.get_data` routine in `80_resources.asm` SHALL copy no more than the actual resource size (up to a maximum of 255 bytes), rather than always copying 255 bytes.

#### Scenario: Small resource copy
- **WHEN** `resource.get_data` is called for a resource of size 50 bytes
- **THEN** exactly 50 bytes are copied to `BUF`, not 255

#### Scenario: Large resource capped at 255
- **WHEN** `resource.get_data` is called for a resource of size 1000 bytes
- **THEN** exactly 255 bytes are copied to `BUF`

### Requirement: Regression tests verify kernel binary integrity
The test suite SHALL include a regression test that compares the compiled `header.bin` against a known-good baseline, verifying that only filler regions differ.

#### Scenario: Kernel binary hash check
- **WHEN** the kernel regression test runs
- **THEN** `header.bin` is built and compared byte-range by byte-range against the expected layout, confirming code regions are unchanged

### Requirement: Protected files are not modified
Files `70_pletter.asm` and `90_support.asm` SHALL remain byte-identical to their pre-optimization state.

#### Scenario: Protected files unchanged
- **WHEN** the optimization is complete
- **THEN** `70_pletter.asm` and `90_support.asm` have zero diff compared to the pre-optimization baseline

### Requirement: Incremental builds continue working
The Makefile SHALL support incremental compilation — modifying a single `.asm` source file SHALL trigger recompilation of only `header.bin`, not a full clean rebuild.

#### Scenario: Incremental compilation after source edit
- **WHEN** a single `.asm` file in `src/header/` is modified
- **THEN** `make` recompiles `header.bin` without requiring `make clean` first
