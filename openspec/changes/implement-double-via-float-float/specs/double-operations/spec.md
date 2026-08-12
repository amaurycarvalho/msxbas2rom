## ADDED Requirements

### Requirement: Double-double addition via Dekker TwoSum kernel routine
The expression evaluator SHALL emit a call to `DISP_doubleTwoSum` (225) or `DISP_doubleFastTwoSum` (224) for double-double addition. The kernel routine implements the Dekker algorithm:
1. `(sh, s) = TwoSum(high1, high2)` — using XBASIC single-precision additions/subtractions
2. `t = low1 + low2` — single-precision add
3. `t = t + s` — accumulate error
4. `(sh, sl) = FastTwoSum(sh, t)` — renormalize

The compiler SHALL set up registers with operands in B:HL (high1) + C:DE (low1) and push the second operand high2:low2 on the stack before the call, following the kernel routine's documented calling convention.

#### Scenario: Add two double values
- **WHEN** expression `A# + B#` is compiled where both are double-precision
- **THEN** the compiler emits `call` to the kernel double-add utility
- **AND** the result in B:HL + C:DE represents the double-double sum

### Requirement: Double-double subtraction via kernel routine
The expression evaluator SHALL emit a call to `DISP_doubleNeg` (231) to negate the second operand, then `DISP_doubleTwoSum` (225) for the addition. The double negation SHALL call the existing single-precision `floatNeg` (DISP 40) twice: once for the high part and once for the low part.

#### Scenario: Subtract two double values
- **WHEN** expression `A# - B#` is compiled
- **THEN** B# is negated (high2 = -high2, low2 = -low2) and double addition is performed
- **AND** the result is a valid double-double value

### Requirement: Double-double multiplication via Dekker TwoProd kernel routine
The expression evaluator SHALL emit a call to `DISP_doubleTwoProd` (226) for double-double multiplication. The kernel routine implements Veltkamp splitting of the XBASIC MBF-format single-precision mantissas into high(12 bits) and low(11 bits) halves, computes the partial products via XBASIC multiply/add/sub calls, and renormalizes via FastTwoSum.

#### Scenario: Multiply two double values
- **WHEN** expression `A# * B#` is compiled
- **THEN** a call to the kernel double-multiply utility is emitted
- **AND** the result preserves ~48 bits of mantissa

#### Scenario: TwoProd handles MBF format
- **WHEN** the Veltkamp split is performed on an XBASIC MBF float
- **THEN** the split correctly isolates the upper 12 mantissa bits
- **AND** `a_hi + a_lo == a` to within single-precision rounding
- **AND** `|a_lo| <= 0.5 ulp(a_hi)`

### Requirement: Double-double division via kernel routine
The expression evaluator SHALL emit a call to `DISP_doubleDiv` (227) for double-double division. The kernel routine implements:
1. Approximate quotient: `qh = high1 / high2` via XBASIC divide
2. Residual computation and refinement steps to recover the low part
3. Renormalization via `doubleFastTwoSum`

#### Scenario: Divide two double values
- **WHEN** expression `A# / B#` is compiled
- **THEN** a call to the kernel double-divide utility is emitted
- **AND** division by zero behavior matches MSX BASIC convention (XBASIC divide handles this)

### Requirement: Double negation and absolute value
The expression evaluator SHALL generate inline code for double negation: call the existing `floatNeg` (DISP 40) twice — once for the high part (negate B:HL → B:HL) and once for the low part (swap C:DE to B:HL, call floatNeg, swap back). Double absolute value SHALL conditionally negate both parts if the high part sign bit (bit 7 of H) is set.

#### Scenario: Negate a double value
- **WHEN** unary `-` is applied to a double operand
- **THEN** both high and low singles are negated
- **AND** the result is a valid double-double

### Requirement: Double comparisons via kernel routine
The expression evaluator SHALL emit a call to `DISP_doubleCompare` (228) for double-double comparisons (=, <>, <, <=, >, >=). The kernel routine SHALL first compare the high parts via `XBASIC_COMPARE_FLOATS`; if equal, compare the low parts (with appropriate sign handling). The result SHALL be in HL (0xFFFF for true, 0x0000 for false) consistent with MSX BASIC convention. The six comparison operators SHALL use the same kernel routine with a modifier byte (or separate dispatch entries) to select the comparison mode.

#### Scenario: Compare equal doubles
- **WHEN** expression `A# = B#` is compiled and both doubles represent the same value
- **THEN** the call to the kernel double-compare routine returns 0xFFFF (true)

#### Scenario: Compare unequal doubles
- **WHEN** expression `A# <> B#` is compiled and the doubles differ
- **THEN** the comparison returns 0xFFFF (true)

#### Scenario: Compare with sign handling
- **WHEN** one double is negative and the other is zero
- **THEN** the comparison correctly distinguishes -0.0 from +0.0 according to MSX BASIC rules
- **AND** low-part tie-breaking only occurs when high parts are exactly equal

### Requirement: Type conversions involving double
The expression evaluator SHALL generate inline Z80 code for all type conversions involving doubles. These are compact sequences (8-14 bytes each) that do not warrant kernel utility routines:
- **Integer → Double**: Convert integer to single (high) via `def_XBASIC_CAST_INTEGER_TO_FLOAT`, set C:DE = 0
- **Single → Double**: Copy B:HL, set C:DE = 0 (ld c,0; ld de,0)
- **Double → Integer**: Truncate the high part to integer via `def_XBASIC_CAST_FLOAT_TO_INTEGER`
- **Double → Single**: Return only B:HL (discard C:DE)
- **String → Double**: Parse string to single (high) via `def_XBASIC_CAST_STRING_TO_FLOAT`, set C:DE = 0
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
All math functions (SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND) SHALL accept double-precision operands and return double-precision results. The implementation SHALL use inline code generation for the function-specific logic (calling the existing XBASIC single-precision function and computing the Taylor correction term), then SHALL call `DISP_doubleFastTwoSum` (224) for renormalization of the result into a valid double-double.

#### Scenario: Calculate SIN of double
- **WHEN** `SIN(A#)` is compiled
- **THEN** `XBASIC_SIN` (DISP 132) is called on the high part
- **AND** the correction term `xl * cos(xh)` is computed via XBASIC multiply
- **AND** `doubleFastTwoSum` (DISP 224) renormalizes the result
- **AND** the result is a valid double-double value

#### Scenario: Calculate SQR of double
- **WHEN** `SQR(A#)` is compiled
- **THEN** square root is computed using the Taylor correction method
- **AND** `doubleFastTwoSum` renormalizes
- **AND** the result preserves higher precision than single

#### Scenario: Double RND produces double-double result
- **WHEN** `RND(A#)` is compiled
- **THEN** the random value is generated as a double-double (two calls to XBASIC_RND)
- **AND** the result has approximately 48 bits of precision

#### Scenario: INT/FIX/SGN work on doubles
- **WHEN** `INT(A#)` or `FIX(A#)` is compiled
- **THEN** the integer truncation is applied to the high part
- **AND** the low part is zero
- **AND** the result is a double-double with zero low part

### Requirement: FOR/NEXT loops with double variables
When a FOR loop uses a double-precision counter variable, the compiler SHALL allocate 6-byte storage for each of the FOR counter, TO limit, and STEP value in the loop's workspace area. The loop comparison SHALL call `DISP_doubleCompare` (228) and the loop increment SHALL call `DISP_doubleTwoSum` (225) for addition.

#### Scenario: FOR loop with double counter
- **WHEN** `FOR A# = 1# TO 10# STEP 0.5#` is compiled
- **THEN** the counter variable A# occupies 6 bytes in the FOR workspace
- **AND** the TO and STEP values occupy 6 bytes each
- **AND** loop termination check uses the kernel double-compare utility
- **AND** loop increment uses the kernel double-add utility
