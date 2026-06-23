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
- **THEN** the commented-out `;in a, (64)` / `;cp 247` / `;ret nz` block no longer exists in `cmd_turbo` (`31_cmd.asm` lines 534-536)

#### Scenario: usr2_turbo panasonic check removed
- **WHEN** the optimization is complete
- **THEN** the commented-out `;in a, (64)` / `;cp 247` / `;jp nz, usr_def` block no longer exists in `usr2_turbo` (`32_usr.asm` lines 108-110)

#### Scenario: gfxVDP.set MSX2 fallback removed
- **WHEN** the optimization is complete
- **THEN** the commented-out MSX2 VDP fallback block no longer exists in `gfxVDP.set` (`60_bios_helpers.asm` lines 913-917)

#### Scenario: Dead comment lines in XBASIC helpers removed
- **WHEN** the optimization is complete
- **THEN** commented-out `;ld (MAXFIL), a`, `;ld (NLONLY), a`, `;ld (DSKDIS), a`, and `; ld a, 0x0C` / `; rst 0x18` lines no longer exist in `30_basic_helpers.asm`

### Requirement: FloatToBCD.add.2 uses add a,a instead of sla e
The `FloatToBCD.add.2` routine in `21_logic_pack.asm` SHALL route the shift operand through register A using `add a, a` instead of performing `sla e` directly, producing identical E register results.

#### Scenario: BCD nibble shift
- **WHEN** `FloatToBCD.add.2` is called with a remainder in E (0x00-0x09)
- **THEN** E contains the value shifted left by 4 bits (low nibble → high nibble)

#### Scenario: Register A not relied upon by caller
- **WHEN** `FloatToBCD.add.2` returns after the shift
- **THEN** the caller (`FloatToBCD.add`) correctly consumes E (via `or e`) without depending on A value from this routine

### Requirement: FloatToBCD uses res 7,h directly
The `FloatToBCD` routine in `21_logic_pack.asm` SHALL reset bit 7 of H directly using `res 7,h` instead of the `ld a,h` / `res 7,a` / `ld h,a` sequence.

#### Scenario: Sign bit reset
- **WHEN** `FloatToBCD` executes the sign-reset step with any float value in B:HL
- **THEN** bit 7 of H is cleared, identical to the pre-optimization behavior

### Requirement: intSHR.1 removes dead xor a
The `intSHR.1` loop in `21_logic_pack.asm` SHALL omit the `xor a` instruction, producing identical carry chain behavior through `SRA H` / `RR L` for all shift counts and input values.

#### Scenario: Single bit shift
- **WHEN** `intSHR` is called with HL=0x8000, E=1
- **THEN** HL=0xC000 (arithmetic right shift preserves sign)

#### Scenario: Multi-bit shift with carry chain
- **WHEN** `intSHR` is called with HL=0x4001, E=4
- **THEN** HL=0x0400 (bits shifted correctly through carry chain)

### Requirement: XBASIC_END uses ld hl,0xFFFF / ld (CURLIN),hl
The `XBASIC_END` routine in `30_basic_helpers.asm` SHALL write 0xFFFF to CURLIN using a 16-bit store instead of two 8-bit stores via the accumulator.

#### Scenario: CURLIN set to 0xFFFF
- **WHEN** `XBASIC_END` initializes the CURLIN variable
- **THEN** both `(CURLIN)` and `(CURLIN+1)` contain 0xFF

### Requirement: XBASIC_TAB uses jp p instead of bit 7,h / jr z
The `XBASIC_TAB` routine in `30_basic_helpers.asm` SHALL test the sign of the SBC result using `jp p` instead of `bit 7,h` / `jr z`.

#### Scenario: Positive result (no clamping)
- **WHEN** `sbc hl,de` produces a non-negative result (HL >= DE)
- **THEN** the jump to `XBASIC_TAB.1` is taken, L retains the computed value

#### Scenario: Negative result (clamp to zero)
- **WHEN** `sbc hl,de` produces a negative result (HL < DE)
- **THEN** L is set to 0 (clamped)

### Requirement: cmd_play uses add a,a instead of sla a
The `cmd_play` routine in `31_cmd.asm` SHALL use two `add a,a` instructions to multiply the channel-enable bit by 4, instead of two `sla a` instructions.

#### Scenario: Channel C enabled (ARG=1)
- **WHEN** `cmd_play` executes with `(ARG)=1`
- **THEN** register C contains 4 (MUSICF bit for channel C enabled)

#### Scenario: Channel C disabled (ARG=0)
- **WHEN** `cmd_play` executes with `(ARG)=0`
- **THEN** register C contains 0 (MUSICF bit for channel C disabled)

### Requirement: cmd_page.delay uses add a,a instead of sla a
The `cmd_page.delay` routine in `31_cmd.asm` SHALL use four `add a,a` instructions to shift the delay#1 value into the high nibble, instead of four `sla a` instructions.

#### Scenario: Delay value composition
- **WHEN** `cmd_page.delay` executes with D=0x05 (delay#1=5), E=0x03 (delay#0=3)
- **THEN** register B contains 0x53 before the WRTVDP call

### Requirement: cmd_mtf.palette.copy.to_vram.loop uses add a,a instead of sla a
The `cmd_mtf.palette.copy.to_vram.loop` routine in `31_cmd.asm` SHALL use four `add a,a` instructions to shift the red component into the high nibble, instead of four `sla a` instructions.

#### Scenario: Palette color composition
- **WHEN** the palette loop processes a color entry with red=7, green=0, blue=7
- **THEN** register A contains 0x77 before the `S.SETPLT` call

### Requirement: cmd_wrtfnt uses ld de,7 / add hl,de instead of 7× inc hl
The `cmd_wrtfnt` routine in `31_cmd.asm` SHALL advance HL by 7 bytes using `ld de,7` / `add hl,de` instead of seven consecutive `inc hl` instructions, producing an identical HL value.

#### Scenario: Skip resource header
- **WHEN** `cmd_wrtfnt` skips the 7-byte resource header
- **THEN** HL points to the font data payload, matching pre-optimization behavior

### Requirement: Behavioral compatibility preserved after micro-optimizations
All optimized assembly routines SHALL produce identical results for identical input compared to the pre-optimization code.

#### Scenario: Comparison functions return identical results
- **WHEN** `intCompareGT` and `intCompareGE` are called with any signed 16-bit input pair
- **THEN** the returned HL value (0xFFFF or 0x0000) matches the pre-optimization result

#### Scenario: All integration tests pass
- **WHEN** the full integration test suite runs against the optimized kernel
- **THEN** all existing `.bas` test files compile without errors
