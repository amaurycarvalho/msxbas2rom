## ADDED Requirements

### Requirement: intCompareGT delegates to intCompareLT with operand swap
The `intCompareGT` routine in `21_logic_pack.asm` SHALL delegate to `intCompareLT` by swapping operands via `ex de,hl`, producing identical results to the pre-optimization implementation for all signed 16-bit integer inputs.

#### Scenario: GT positive comparison
- **WHEN** `intCompareGT` is called with HL=100, DE=50
- **THEN** HL=0xFFFF (true, since 100 > 50)

#### Scenario: GT negative comparison
- **WHEN** `intCompareGT` is called with HL=-50, DE=-100
- **THEN** HL=0xFFFF (true, since -50 > -100)

#### Scenario: GT equal values
- **WHEN** `intCompareGT` is called with HL=42, DE=42
- **THEN** HL=0x0000 (false, since 42 is not greater than 42)

#### Scenario: GT mixed sign
- **WHEN** `intCompareGT` is called with HL=1, DE=-1
- **THEN** HL=0xFFFF (true, since 1 > -1)

#### Scenario: GT edge values
- **WHEN** `intCompareGT` is called with HL=0x7FFF (max signed), DE=0x8000 (min signed)
- **THEN** HL=0xFFFF (true, since 32767 > -32768)

### Requirement: intCompareGE delegates to intCompareLE with operand swap
The `intCompareGE` routine in `21_logic_pack.asm` SHALL delegate to `intCompareLE` by swapping operands via `ex de,hl`, producing identical results to the pre-optimization implementation for all signed 16-bit integer inputs.

#### Scenario: GE positive comparison
- **WHEN** `intCompareGE` is called with HL=100, DE=50
- **THEN** HL=0xFFFF (true, since 100 >= 50)

#### Scenario: GE equal values
- **WHEN** `intCompareGE` is called with HL=42, DE=42
- **THEN** HL=0xFFFF (true, since 42 >= 42)

#### Scenario: GE false case
- **WHEN** `intCompareGE` is called with HL=10, DE=20
- **THEN** HL=0x0000 (false, since 10 is not >= 20)

### Requirement: cmd_setfnt.default_colors.1 uses add a,a instead of sla a
The `cmd_setfnt.default_colors.1` routine in `31_cmd.asm` SHALL use four consecutive `add a, a` instructions to multiply the FORCLR value by 16, instead of four `sla a` instructions, producing identical register A and carry flag results.

#### Scenario: Multiply FORCLR by 16 with add a,a
- **WHEN** `cmd_setfnt.default_colors.1` executes with `(FORCLR)=0x03`
- **THEN** register A contains 0x30 after the four `add a, a` instructions

#### Scenario: Carry flag propagation
- **WHEN** `cmd_setfnt.default_colors.1` executes with `(FORCLR)=0x10`
- **THEN** the final `or (hl)` combines the OR'd colors correctly for `FILVRM`

### Requirement: XBASIC_COPY_TO uses stack-based save/restore for HL
The `XBASIC_COPY_TO` routine in `30_basic_helpers.asm` SHALL use `push hl` and `pop hl` to preserve the destination address across coordinate calculations, instead of storing to and loading from `MV_DPTR`.

#### Scenario: HL preserved after coordinate adjustment
- **WHEN** `XBASIC_COPY_TO` is called with a destination address in HL
- **THEN** after coordinate calculations (which clobber HL and DE), the original destination address is correctly restored from the stack

#### Scenario: Stack integrity maintained
- **WHEN** `XBASIC_COPY_TO` pushes HL and later pops it
- **THEN** the stack pointer is restored to its original value before the routine returns

### Requirement: Dead commented code removed from kernel source
Fully commented-out code blocks with zero active references SHALL be removed from kernel assembly source files.

#### Scenario: cmd_turbo panasonic check removed
- **WHEN** the optimization is complete
- **THEN** the commented-out `;in a, (64)` / `;cp 247` / `;ret nz` block no longer exists in `cmd_turbo` (`31_cmd.asm`)

#### Scenario: usr2_turbo panasonic check removed
- **WHEN** the optimization is complete
- **THEN** the commented-out `;in a, (64)` / `;cp 247` / `;jp nz, usr_def` block no longer exists in `usr2_turbo` (`32_usr.asm`)

#### Scenario: gfxVDP.set MSX2 fallback removed
- **WHEN** the optimization is complete
- **THEN** the commented-out MSX2 VDP fallback block no longer exists in `gfxVDP.set` (`60_bios_helpers.asm`)

### Requirement: Behavioral compatibility preserved after micro-optimizations
All optimized assembly routines SHALL produce identical results for identical input compared to the pre-optimization code.

#### Scenario: Comparison functions return identical results
- **WHEN** `intCompareGT` and `intCompareGE` are called with any signed 16-bit input pair
- **THEN** the returned HL value (0xFFFF or 0x0000) matches the pre-optimization result

#### Scenario: All integration tests pass
- **WHEN** the full integration test suite runs against the optimized kernel
- **THEN** all existing `.bas` test files compile without errors
