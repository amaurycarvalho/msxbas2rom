## ADDED Requirements

### Requirement: Double variable occupies 6 bytes in RAM
The compiler SHALL allocate 6 bytes per double-precision variable (`subtype_double_decimal`), storing the high-order single (3 bytes: B:HL) followed by the low-order single (3 bytes: C:DE). The var_size for `subtype_double_decimal` in `compiler_symbol_resolver.cpp` SHALL be 6 (changed from 3). Array element size (x_factor) SHALL be 6 for double arrays.

#### Scenario: Double variable allocation
- **WHEN** a variable with subtype `subtype_double_decimal` is allocated
- **THEN** the symbol resolver assigns 6 bytes of RAM
- **AND** the address fixups reference the high part at offset 0 and low part at offset 3

#### Scenario: Double array element size
- **WHEN** a DIM statement declares a double array (e.g., `DIM A#(10)`)
- **THEN** `x_factor` is 6 (not 3)
- **AND** `array_size = x_factor * x_size * y_size` reflects 6-byte elements

### Requirement: Double loading uses kernel utility or inline code
The expression evaluator SHALL load a double value from RAM into registers as B:HL (high part) + C:DE (low part). Loading may be emitted inline (ld a,(var), ld b,a, ld hl,(var+1), ld a,(var+3), ld c,a, ld de,(var+4)) or via `call DISP_doubleLoadHL` (DISP 229) when HL is set to the variable address. Array element access SHALL use the same load sequence after computing the element address in HL.

#### Scenario: Load double variable into registers
- **WHEN** a double variable reference is evaluated in an expression
- **THEN** B:HL contain the high-order single
- **AND** C:DE contain the low-order single
- **AND** 6 bytes are read from the variable's RAM location

### Requirement: Double storage writes B:HL + C:DE to RAM
The variable emitter SHALL store a double value from registers (B:HL high, C:DE low) into 6 consecutive bytes of RAM. Storage may be emitted inline (ld a,b, ld (var),a, ld (var+1),hl, ld a,c, ld (var+3),a, ld (var+4),de) or via `call DISP_doubleStoreHL` (DISP 230) when HL is set to the destination address.

#### Scenario: Store double to scalar variable
- **WHEN** a double value in B:HL + C:DE is assigned to a scalar double variable
- **THEN** 6 bytes are written to the variable's RAM address
- **AND** high part occupies offset 0..2, low part occupies offset 3..5

#### Scenario: Store double to array element
- **WHEN** a double value is assigned to a double array element
- **THEN** the array address is computed and 6 bytes are stored sequentially
- **AND** register push/pop order preserves all 6 bytes

### Requirement: Double stack discipline uses 8 bytes
The expression evaluator SHALL push and pop 8 bytes on the Z80 hardware stack for double values during binary operations. Since a double holds 6 bytes of useful data but Z80 `push` operates on register pairs, the stack footprint is 8 bytes: push hl (high mantissa), push bc (B = high sign/exp), push de (low mantissa), and push af (A = low sign/exp) after `ld a, c`. The pop order SHALL reverse the push order: pop af, pop de, pop bc, pop hl, ld c, a.

#### Scenario: Push double before binary operation
- **WHEN** evaluating a binary operation with two double operands
- **THEN** the first operand result (B:HL + C:DE) is pushed onto the stack as 8 bytes
- **AND** the second operand is then loaded into registers for the operation

#### Scenario: Pop double after stack load
- **WHEN** a double value was previously pushed and must be restored
- **THEN** 8 bytes are popped from the stack into B:HL + C:DE
- **AND** register order is consistent with the push order

### Requirement: Double type suffix # is consistently recognized
The lexer SHALL recognize `#` as the double-precision suffix and `!` as the single-precision suffix in both identifier context (`identifier_state.cpp`) and literal context (`literal_state.cpp`). The current inverted mapping in `literal_state.cpp` SHALL be fixed.

#### Scenario: Numeric literal with # suffix
- **WHEN** the lexer encounters `1.5#`
- **THEN** the lexeme subtype is `subtype_double_decimal`

#### Scenario: Numeric literal with ! suffix
- **WHEN** the lexer encounters `1.5!`
- **THEN** the lexeme subtype is `subtype_single_decimal`

#### Scenario: Variable identifier with # suffix
- **WHEN** the lexer encounters `A#` as an identifier
- **THEN** the lexeme subtype is `subtype_double_decimal`

### Requirement: Float converter produces two singles for double literals
The `CompilerFloatConverter::double2FloatLib()` SHALL accept a C++ `double` value and produce two XBASIC-format single-precision values (high part + low part) stored in the output `int* words` array using the float-float decomposition: the high part is the direct conversion of the double to XBASIC single format, and the low part is the conversion of the residual `double - float(double)` to XBASIC single format.

#### Scenario: Convert double zero
- **WHEN** double value 0.0 is converted
- **THEN** both high and low singles are zero (6 words = all zeros)

#### Scenario: Convert double with residual
- **WHEN** double value 1.23456789 is converted
- **THEN** the high part approximately equals the XBASIC single of 1.23456789
- **AND** the low part is non-zero (captures the residual magnitude)
- **AND** the double-double sum `high + low` is closer to the original C++ double than `high` alone
