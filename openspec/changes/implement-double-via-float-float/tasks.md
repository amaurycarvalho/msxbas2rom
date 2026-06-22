## 1. Lexer Suffix Fix

- [ ] 1.1 Fix `#`/`!` suffix mapping in `literal_state.cpp` — swap lines so `#` → `subtype_double_decimal` and `!` → `subtype_single_decimal`
- [ ] 1.2 Verify `identifier_state.cpp` suffix mapping is correct (`#` → double, `!` → single)
- [ ] 1.3 Verify `unknown_state.cpp` double subtype assignment is correct

## 2. Double Literal Handling (Float Converter)

- [ ] 2.1 Modify `CompilerFloatConverter::double2FloatLib()` to produce two singles (high + low) in the output `int* words` array
- [ ] 2.2 Implement float-float decomposition: high = XBASIC_single(double), low = XBASIC_single(double - float(double))
- [ ] 2.3 Update all callers of `double2FloatLib` for the new 6-word output format
- [ ] 2.4 Update `str2FloatLib` to handle double-precision string parsing (produce high+low pair)

## 3. Symbol Resolution (6-Byte Allocation)

- [ ] 3.1 Change `var_size` for `subtype_double_decimal` from 3 to 6 in `compiler_symbol_resolver.cpp`
- [ ] 3.2 Verify RAM size tracking (`cpu.context->ram_size`, `ram_pointer`) correctly advances by 6
- [ ] 3.3 Verify memory limit check (`def_RAM_SIZE`) accounts for larger double variables

## 4. Variable Loading and Storage

- [ ] 4.1 Implement 6-byte double load in `compiler_expression_evaluator.cpp`: `ld a,(var)`, `ld b,a`, `ld hl,(var+1)`, `ld a,(var+3)`, `ld c,a`, `ld de,(var+4)`
- [ ] 4.2 Implement 6-byte double store for scalar variables in `compiler_variable_emitter.cpp`
- [ ] 4.3 Implement 6-byte double store for array elements in `compiler_variable_emitter.cpp`
- [ ] 4.4 Update stack push/pop for double operands: 8 bytes (push hl, push bc, push de, ld a,c, push af) and reverse pop

## 5. Double-Double Arithmetic (Expression Evaluator)

- [ ] 5.1 Implement helper method for TwoSum(Fast) — compiler emits inline Z80 sequence calling `def_XBASIC_ADD_FLOATS` and `def_XBASIC_SUBTRACT_FLOATS`
- [ ] 5.2 Implement double addition (`+`) via Dekker TwoSum + FastTwoSum sequence
- [ ] 5.3 Implement double subtraction (`-`) via negation + addition
- [ ] 5.4 Implement double multiplication (`*`) via TwoProd (Veltkamp split) + cross terms + renormalization
- [ ] 5.5 Implement double division (`/`) via approximate quotient + residual refinement
- [ ] 5.6 Implement double negation (unary `-`) — negate both high and low parts
- [ ] 5.7 Implement double absolute value (ABS as unary) — negate if high part negative

## 6. Double-Double Comparisons

- [ ] 6.1 Implement double equality (`=`) — compare high parts, if equal compare low parts
- [ ] 6.2 Implement double inequality (`<>`) — inverse of equality
- [ ] 6.3 Implement double less than (`<`) — compare high parts, tie-break with low parts
- [ ] 6.4 Implement double less/equal (`<=`) — compare high parts, tie-break with low parts
- [ ] 6.5 Implement double greater than (`>`) — compare high parts, tie-break with low parts
- [ ] 6.6 Implement double greater/equal (`>=`) — compare high parts, tie-break with low parts

## 7. Type Conversions Involving Double

- [ ] 7.1 Implement Integer → Double cast: convert HL to B:HL via `def_XBASIC_CAST_INTEGER_TO_FLOAT`, set C:DE = 0
- [ ] 7.2 Implement Single → Double cast: copy B:HL, set C:DE = 0 (zero low part)
- [ ] 7.3 Implement Double → Integer cast: call `def_XBASIC_CAST_FLOAT_TO_INTEGER` on B:HL, discard C:DE
- [ ] 7.4 Implement Double → Single cast: return B:HL only (truncation, discard C:DE)
- [ ] 7.5 Implement String → Double cast: call `def_XBASIC_CAST_STRING_TO_FLOAT`, set C:DE = 0
- [ ] 7.6 Implement Double → String cast: call `def_XBASIC_CAST_FLOAT_TO_STRING` on B:HL only
- [ ] 7.7 Update `addCast()` method in `compiler_expression_evaluator.cpp` to handle all new double cast paths

## 8. Math Function Strategies (Taylor Correction)

- [ ] 8.1 Implement `SIN` for double: `sin(xh+xl) ≈ sin(xh) + xl*cos(xh)` via inline Z80
- [ ] 8.2 Implement `COS` for double: `cos(xh+xl) ≈ cos(xh) - xl*sin(xh)` via inline Z80
- [ ] 8.3 Implement `TAN` for double: `tan(xh+xl) ≈ tan(xh) + xl/(cos(xh))^2`
- [ ] 8.4 Implement `ATN` for double: `atan(xh+xl) ≈ atan(xh) + xl/(1+xh^2)`
- [ ] 8.5 Implement `EXP` for double: `exp(xh+xl) ≈ exp(xh) * (1+xl)`
- [ ] 8.6 Implement `LOG` for double: `log(xh+xl) ≈ log(xh) + xl/xh`
- [ ] 8.7 Implement `SQR` for double: `sqrt(xh+xl) ≈ sqrt(xh) + xl/(2*sqrt(xh))`
- [ ] 8.8 Implement `INT` for double: int(high part), zero low part
- [ ] 8.9 Implement `FIX` for double: fix(high part), zero low part
- [ ] 8.10 Implement `SGN` for double: sign of high part as double-double
- [ ] 8.11 Implement `ABS` for double: negate both parts if high negative
- [ ] 8.12 Implement `RND` for double: call `def_XBASIC_RND` twice for two singles

## 9. Statement Strategies (FOR, PRINT, INPUT)

- [ ] 9.1 Update `compiler_for_statement_strategy.cpp` — allocate 6 bytes for FOR counter, TO limit, and STEP when variable is double
- [ ] 9.2 Implement double FOR loop comparison (check if counter has reached TO limit using double comparison)
- [ ] 9.3 Implement double FOR loop increment (counter = counter + STEP using double addition)
- [ ] 9.4 Update `compiler_print_statement_strategy.cpp` — handle double values in PRINT
- [ ] 9.5 Update `compiler_input_statement_strategy.cpp` — handle double values in INPUT

## 10. Basic Type Conversion Strategies

- [ ] 10.1 Update `compiler_cdbl_function_strategy.cpp` — cast to full 6-byte double (not 3-byte)
- [ ] 10.2 Update `compiler_csng_function_strategy.cpp` — truncate double to single (extract B:HL, discard C:DE)
- [ ] 10.3 Update `compiler_cint_function_strategy.cpp` — truncate double to integer via high part

## 11. Debug Symbol Export

- [ ] 11.1 Update `noice_export_strategy.cpp` — report double type as 6 bytes instead of 3
- [ ] 11.2 Update `cdb_export_strategy.cpp` — report double type as 6 bytes

## 12. Unit Tests

- [ ] 12.1 Create `tests/unit/src/test_double.cpp` with Doctest test cases
- [ ] 12.2 Test `CompilerFloatConverter::double2FloatLib` for zero, small values, large values, negative values
- [ ] 12.3 Test float-float decomposition consistency: high + low ≈ original double
- [ ] 12.4 Test `Lexeme` subtype assignments for `#` and `!` suffixes
- [ ] 12.5 Test variable size allocation for double subtype (6 bytes)
- [ ] 12.6 Verify unit tests compile and pass with `make test-unit`

## 13. Integration Tests (BAS Sources)

- [ ] 13.1 Create `tests/integration/MATH/double_add.bas` — test double addition with various values
- [ ] 13.2 Create `tests/integration/MATH/double_sub.bas` — test double subtraction
- [ ] 13.3 Create `tests/integration/MATH/double_mul.bas` — test double multiplication
- [ ] 13.4 Create `tests/integration/MATH/double_div.bas` — test double division
- [ ] 13.5 Create `tests/integration/MATH/double_compare.bas` — test double comparisons
- [ ] 13.6 Create `tests/integration/MATH/double_math_funcs.bas` — test SIN, COS, SQR, LOG, EXP with doubles
- [ ] 13.7 Create `tests/integration/MATH/double_conversions.bas` — test CDBL, CSNG, CINT with doubles
- [ ] 13.8 Create `tests/integration/MATH/double_for_next.bas` — test FOR/NEXT loops with double counter
- [ ] 13.9 Create `tests/integration/MATH/double_suffix.bas` — test `#` suffix for literals and variables
- [ ] 13.10 Ensure all .bas files compile with `make test-integration` (produces .rom files for manual verification)

## 14. Build & Verification

- [ ] 14.1 Verify incremental compilation works (no unnecessary clean of artifacts)
- [ ] 14.2 Verify `make test-unit` passes all unit tests
- [ ] 14.3 Verify `make` produces the release binary successfully
- [ ] 14.4 Verify all existing integration tests still compile (no regressions)
