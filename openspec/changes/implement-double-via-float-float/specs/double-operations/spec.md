## ADDED Requirements

### Requirement: Double-double addition via Dekker TwoSum
The expression evaluator SHALL generate inline Z80 code for double-double addition (high1:low1 + high2:low2) using the Dekker algorithm:
1. `(sh, s) = TwoSum(high1, high2)` — using two XBASIC single-precision additions/subtractions
2. `t = low1 + low2` — single-precision add
3. `t = t + s` — accumulate error
4. `(sh, sl) = FastTwoSum(sh, t)` — renormalize

Each step calls `call def_XBASIC_ADD_FLOATS` or `call def_XBASIC_SUBTRACT_FLOATS` with register setup for single-precision operands in B:HL and C:DE.

#### Scenario: Add two double values
- **WHEN** expression `A# + B#` is compiled where both are double-precision
- **THEN** the emitted Z80 code performs a full float-float addition sequence
- **AND** the result in B:HL + C:DE represents the double-double sum

### Requirement: Double-double subtraction
The expression evaluator SHALL generate inline Z80 code for double-double subtraction (high1:low1 - high2:low2) by negating the second operand's high and low parts via subtraction from zero, then performing double-double addition.

#### Scenario: Subtract two double values
- **WHEN** expression `A# - B#` is compiled
- **THEN** B# is negated (high2 = -high2, low2 = -low2) and double addition is performed
- **AND** the result is a valid double-double value

### Requirement: Double-double multiplication via Dekker TwoProd
The expression evaluator SHALL generate inline Z80 code for double-double multiplication (high1:low1 * high2:low2) using:
1. `(ph, p) = TwoProd(high1, high2)` — using single-precision multiplication and error extraction
2. Cross terms: `t = high1 * low2 + high2 * low1 + p`
3. `(ph, pl) = FastTwoSum(ph, t)` — renormalize

#### Scenario: Multiply two double values
- **WHEN** expression `A# * B#` is compiled
- **THEN** a full float-float multiplication sequence is emitted
- **AND** the result preserves ~48 bits of mantissa

### Requirement: Double-double division
The expression evaluator SHALL generate inline Z80 code for double-double division (high1:low1 / high2:low2) using:
1. Approximate quotient: `qh = high1 / high2`
2. Residual computation and refinement steps to recover the low part

#### Scenario: Divide two double values
- **WHEN** expression `A# / B#` is compiled
- **THEN** a float-float division sequence is emitted
- **AND** division by zero behavior matches MSX BASIC convention

### Requirement: Double negation and absolute value
The expression evaluator SHALL generate inline Z80 code for double negation (negate both high and low parts) and absolute value (negate both parts if the high part is negative).

#### Scenario: Negate a double value
- **WHEN** unary `-` is applied to a double operand
- **THEN** both high and low singles are negated
- **AND** the result is a valid double-double

### Requirement: Double comparisons
The expression evaluator SHALL generate inline Z80 code for double-double comparisons (=, <>, <, <=, >, >=). A comparison SHALL first compare the high parts; if equal, compare the low parts (with appropriate sign handling). The result SHALL be an integer (0 for false, -1 for true) consistent with MSX BASIC convention.

#### Scenario: Compare equal doubles
- **WHEN** expression `A# = B#` is compiled and both doubles represent the same value
- **THEN** the comparison returns -1 (true)

#### Scenario: Compare unequal doubles
- **WHEN** expression `A# <> B#` is compiled and the doubles differ
- **THEN** the comparison returns -1 (true)

### Requirement: Type conversions involving double
The expression evaluator SHALL generate inline Z80 code for all type conversions involving doubles:
- **Integer → Double**: Convert integer to single (high), set low to zero
- **Single → Double**: Copy single to high part, set low to zero
- **Double → Integer**: Truncate the high part to integer via `def_XBASIC_CAST_FLOAT_TO_INTEGER`
- **Double → Single**: Return only the high part (truncation)
- **String → Double**: Parse string to single (high), set low to zero
- **Double → String**: Print high part only via `def_XBASIC_CAST_FLOAT_TO_STRING`

#### Scenario: Cast integer to double
- **WHEN** an integer value is cast to double (e.g., CDBL or implicit promotion)
- **THEN** B:HL contains the float representation of the integer
- **AND** C:DE contains zero

#### Scenario: Cast double to integer
- **WHEN** a double value is cast to integer (e.g., CINT)
- **THEN** the high part is truncated to integer via XBASIC
- **AND** the result is in HL

### Requirement: Math functions for double-precision
All math functions (SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND) SHALL accept double-precision operands and return double-precision results. The implementation SHALL use inline code generation that operates on both the high and low parts using float-float arithmetic sequences, calling existing XBASIC single-precision routines for each intermediate step.

#### Scenario: Calculate SIN of double
- **WHEN** `SIN(A#)` is compiled
- **THEN** the function compiles and emits inline code that computes sine at float-float precision
- **AND** the result is a valid double-double value

#### Scenario: Calculate SQR of double
- **WHEN** `SQR(A#)` is compiled
- **THEN** square root is computed using a float-float iterative method
- **AND** the result preserves higher precision than single

#### Scenario: Double RND produces double-double result
- **WHEN** `RND(A#)` is compiled
- **THEN** the random value is generated as a double-double
- **AND** the result has approximately 48 bits of precision

### Requirement: FOR/NEXT loops with double variables
When a FOR loop uses a double-precision counter variable, the compiler SHALL allocate 6-byte storage for each of the FOR counter, TO limit, and STEP value. The loop comparison and increment SHALL use double-double arithmetic.

#### Scenario: FOR loop with double counter
- **WHEN** `FOR A# = 1# TO 10# STEP 0.5#` is compiled
- **THEN** the counter variable A# occupies 6 bytes
- **AND** the TO and STEP values occupy 6 bytes each
- **AND** loop termination check uses double-double comparison
