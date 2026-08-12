## 1. Lexer Suffix Fix

- [ ] 1.1 Fix `#`/`!` suffix mapping in `literal_state.cpp` — swap lines so `#` → `subtype_double_decimal` and `!` → `subtype_single_decimal`
- [ ] 1.2 Verify `identifier_state.cpp` suffix mapping is correct (`#` → double, `!` → single)
- [ ] 1.3 Verify `unknown_state.cpp` double subtype assignment is correct

## 2. Double Literal Handling (Float Converter)

- [ ] 2.1 Modify `CompilerFloatConverter::double2FloatLib()` to produce two singles (high + low) in the output `int* words` array
- [ ] 2.2 Implement float-float decomposition: high = XBASIC_single(double), low = XBASIC_single(double - float(double))
- [ ] 2.3 Update all callers of `double2FloatLib` for the new 6-word output format
- [ ] 2.4 Update `str2FloatLib` to handle double-precision string parsing (produce high+low pair)

## 3. Kernel Assembly: Double Utility Routines

- [ ] 3.1 Add `doubleLoadHL` routine (DISP 229) — load 6 bytes from (HL) into B:HL + C:DE (~20 bytes)
- [ ] 3.2 Add `doubleStoreHL` routine (DISP 230) — store B:HL + C:DE to 6 bytes at (HL) (~18 bytes)
- [ ] 3.3 Add `doubleFastTwoSum` routine (DISP 224) — Dekker FastTwoSum(a,b) → (s,e) using `exx` for saves, push/pop for intermediates (~50 bytes)
- [ ] 3.4 Add `doubleTwoSum` routine (DISP 225) — full TwoSum calling `doubleFastTwoSum` + 3 XBASIC refinement calls (~55 bytes delta)
- [ ] 3.5 Add `doubleTwoProd` routine (DISP 226) — Veltkamp split on MBF format + XBASIC multiply calls for partial products (~160 bytes)
- [ ] 3.6 Add `doubleDiv` routine (DISP 227) — approximate quotient + residual refinement + renormalization via `doubleFastTwoSum` (~110 bytes)
- [ ] 3.7 Add `doubleCompare` routine (DISP 228) — compare high parts, early exit; low-part tie-break when equal (~45 bytes)
- [ ] 3.8 Add `doubleNeg` routine (DISP 231) — call existing `floatNeg` (DISP 40) twice: high part then low part (~15 bytes)
- [ ] 3.9 Add 8 new `dw` entries to `wrapper_routines_map_table` in `20_runtime.asm` for DISP 224-231
- [ ] 3.10 Verify total routine sizes fit within the 647-byte `BASIC_KUN_START_FILLER` area
- [ ] 3.11 Regenerate `header.h` via pasmo (`make header.h`) and verify no regressions in existing compilation
- [ ] 3.12 Run `make test-integration` to verify all existing .bas files still compile with the modified kernel

## 4. Kernel Dispatch: compiler_hooks.h

- [ ] 4.1 Add `#define DISP_doubleTwoSum 224` through `#define DISP_doubleNeg 231` constants
- [ ] 4.2 Update `DISP_ENTRIES` from 224 to 232
- [ ] 4.3 Verify `getKernelCallAddr()` resolves addresses for indices 0-231

## 5. Symbol Resolution (6-Byte Allocation)

- [ ] 5.1 Change `var_size` for `subtype_double_decimal` from 3 to 6 in `compiler_symbol_resolver.cpp`
- [ ] 5.2 Change `x_factor` for double arrays from 3 to 6 (coordinate with `fix-dim-position-order` change)
- [ ] 5.3 Verify RAM size tracking (`cpu.context->ram_size`, `ram_pointer`) correctly advances by 6
- [ ] 5.4 Verify memory limit check (`def_RAM_SIZE`) accounts for larger double variables

## 6. Variable Loading and Storage

- [ ] 6.1 Implement 6-byte double load via `optimizer.addKernelCall(DISP_doubleLoadHL)` after computing variable address in HL
- [ ] 6.2 Implement 6-byte double store via `optimizer.addKernelCall(DISP_doubleStoreHL)` in `compiler_variable_emitter.cpp`
- [ ] 6.3 Implement 6-byte double store for array elements in `compiler_variable_emitter.cpp`
- [ ] 6.4 Update stack push/pop in `evalOperatorParms` for double operands: 8 bytes (push hl, push bc, push de, ld a,c, push af) and reverse pop
- [ ] 6.5 Update stack push/pop in operator handlers (evalOperator) for recovering double operands from the 8-byte stack layout

## 7. Double-Double Arithmetic (Expression Evaluator)

- [ ] 7.1 Implement double addition (`+`) — emit `optimizer.addKernelCall(DISP_doubleTwoSum)` with operand setup
- [ ] 7.2 Implement double subtraction (`-`) — emit `optimizer.addKernelCall(DISP_doubleNeg)` then `DISP_doubleTwoSum`
- [ ] 7.3 Implement double multiplication (`*`) — emit `optimizer.addKernelCall(DISP_doubleTwoProd)`
- [ ] 7.4 Implement double division (`/`) — emit `optimizer.addKernelCall(DISP_doubleDiv)`
- [ ] 7.5 Implement double negation (unary `-`) — inline: swap high→B:HL, call `floatNeg` (DISP 40), swap low→B:HL, call `floatNeg`, swap back
- [ ] 7.6 Implement double absolute value (ABS as unary) — inline: check sign bit (bit 7 of H), conditionally negate both parts
- [ ] 7.7 Update `evalOperatorParms` to push 8 bytes per double operand (currently 4 bytes per single)
- [ ] 7.8 Update `evalOperatorCast` for mixed-type expressions involving doubles

## 8. Double-Double Comparisons

- [ ] 8.1 Implement double equality (`=`) — emit `ld a,0` then `optimizer.addKernelCall(DISP_doubleCompare)`
- [ ] 8.2 Implement double inequality (`<>`) — emit `ld a,1` then `optimizer.addKernelCall(DISP_doubleCompare)`
- [ ] 8.3 Implement double less than (`<`) — emit `ld a,2` then `optimizer.addKernelCall(DISP_doubleCompare)`
- [ ] 8.4 Implement double less/equal (`<=`) — emit `ld a,3` then `optimizer.addKernelCall(DISP_doubleCompare)`
- [ ] 8.5 Implement double greater than (`>`) — emit `ld a,4` then `optimizer.addKernelCall(DISP_doubleCompare)`
- [ ] 8.6 Implement double greater/equal (`>=`) — emit `ld a,5` then `optimizer.addKernelCall(DISP_doubleCompare)`

## 9. Type Conversions Involving Double

- [ ] 9.1 Implement Integer → Double cast: convert HL to B:HL via `DISP_XBASIC_CAST_INTEGER_TO_FLOAT`, set C:DE = 0 (inline)
- [ ] 9.2 Implement Single → Double cast: copy B:HL, set C:DE = 0 (ld c,0; ld de,0 — inline, 4 bytes)
- [ ] 9.3 Implement Double → Integer cast: call `DISP_XBASIC_CAST_FLOAT_TO_INTEGER` on B:HL, discard C:DE (inline)
- [ ] 9.4 Implement Double → Single cast: return B:HL only (truncation, discard C:DE — no code emitted)
- [ ] 9.5 Implement String → Double cast: call `DISP_XBASIC_CAST_STRING_TO_FLOAT`, set C:DE = 0 (inline)
- [ ] 9.6 Implement Double → String cast: call `DISP_XBASIC_CAST_FLOAT_TO_STRING` on B:HL only (inline)
- [ ] 9.7 Update `addCast()` method in `compiler_expression_evaluator.cpp` to handle all new double cast paths

## 10. Math Function Strategies (Taylor Correction + Kernel Renormalization)

- [ ] 10.1 Implement `SIN` for double: call `DISP_XBASIC_SIN` on xh, compute xl*cos(xh) via `DISP_XBASIC_MULTIPLY_FLOATS`, renormalize via `DISP_doubleFastTwoSum`
- [ ] 10.2 Implement `COS` for double: call `DISP_XBASIC_COS` on xh, compute -xl*sin(xh), renormalize
- [ ] 10.3 Implement `TAN` for double: call `DISP_XBASIC_TAN`, compute xl/cos²(xh), renormalize
- [ ] 10.4 Implement `ATN` for double: call `DISP_XBASIC_ATN`, compute xl/(1+xh²), renormalize
- [ ] 10.5 Implement `EXP` for double: call `DISP_XBASIC_EXP`, multiply by (1+xl), renormalize
- [ ] 10.6 Implement `LOG` for double: call `DISP_XBASIC_LOG`, add xl/xh, renormalize
- [ ] 10.7 Implement `SQR` for double: call `DISP_XBASIC_SQR`, add xl/(2*sqrt(xh)), renormalize
- [ ] 10.8 Implement `INT` for double: call `DISP_XBASIC_INT` on high part, zero low part
- [ ] 10.9 Implement `FIX` for double: call `DISP_XBASIC_FIX` on high part, zero low part
- [ ] 10.10 Implement `SGN` for double: call `DISP_XBASIC_SGN_FLOAT` on high part, zero low part
- [ ] 10.11 Implement `ABS` for double: check sign bit on high part, conditionally negate both
- [ ] 10.12 Implement `RND` for double: call `DISP_XBASIC_RND` twice — first result as high part, second as low part

## 11. Statement Strategies (FOR, PRINT, INPUT)

- [ ] 11.1 Update `compiler_for_statement_strategy.cpp` — allocate 6 bytes for FOR counter, TO limit, and STEP when variable is double
- [ ] 11.2 Implement double FOR loop comparison: emit `DISP_doubleCompare` call between counter and limit
- [ ] 11.3 Implement double FOR loop increment: emit `DISP_doubleTwoSum` call (counter + STEP)
- [ ] 11.4 Update `compiler_print_statement_strategy.cpp` — handle double values in PRINT (print high part only, discard low)
- [ ] 11.5 Update `compiler_print_statement_strategy.cpp` — PRINT USING for doubles: emit high part only (non-goal: full double formatting)
- [ ] 11.6 Update `compiler_input_statement_strategy.cpp` — handle double values in INPUT

## 12. Basic Type Conversion Strategies

- [ ] 12.1 Update `compiler_cdbl_function_strategy.cpp` — cast to full 6-byte double (not 3-byte): preserve high, set low=0
- [ ] 12.2 Update `compiler_csng_function_strategy.cpp` — truncate double to single (extract B:HL, discard C:DE)
- [ ] 12.3 Update `compiler_cint_function_strategy.cpp` — truncate double to integer via high part

## 13. Debug Symbol Export

- [ ] 13.1 Update `noice_export_strategy.cpp` — report double type as 6 bytes instead of 3
- [ ] 13.2 Update `cdb_export_strategy.cpp` — report double type as 6 bytes

## 14. Unit Tests

- [ ] 14.1 Create `tests/unit/src/test_double.cpp` with Doctest test cases
- [ ] 14.2 Test `CompilerFloatConverter::double2FloatLib` for zero, small values, large values, negative values
- [ ] 14.3 Test float-float decomposition consistency: high + low ≈ original double
- [ ] 14.4 Test `Lexeme` subtype assignments for `#` and `!` suffixes
- [ ] 14.5 Test variable size allocation for double subtype (6 bytes)
- [ ] 14.6 Test that `DISP_ENTRIES` is 232 and new DISP_double* constants resolve correctly
- [ ] 14.7 Verify unit tests compile and pass with `make test-unit`

## 15. Integration Tests (BAS Sources)

- [ ] 15.1 Create `tests/integration/MATH/double_add.bas` — test double addition with various values
- [ ] 15.2 Create `tests/integration/MATH/double_sub.bas` — test double subtraction
- [ ] 15.3 Create `tests/integration/MATH/double_mul.bas` — test double multiplication (validates TwoProd / Veltkamp split)
- [ ] 15.4 Create `tests/integration/MATH/double_div.bas` — test double division
- [ ] 15.5 Create `tests/integration/MATH/double_compare.bas` — test all six double comparison operators
- [ ] 15.6 Create `tests/integration/MATH/double_math_funcs.bas` — test SIN, COS, SQR, LOG, EXP with doubles
- [ ] 15.7 Create `tests/integration/MATH/double_conversions.bas` — test CDBL, CSNG, CINT with doubles
- [ ] 15.8 Create `tests/integration/MATH/double_for_next.bas` — test FOR/NEXT loops with double counter
- [ ] 15.9 Create `tests/integration/MATH/double_suffix.bas` — test `#` suffix for literals and variables
- [ ] 15.10 Ensure all .bas files compile with `make test-integration` (produces .rom files for manual verification)

## 16. Build & Verification

- [ ] 16.1 Rebuild kernel binary: `make` in `src/infrastructure/kernel/asm/` (pasmo → header.bin → header.h)
- [ ] 16.2 Verify kernel binary is exactly 32768 bytes (no page expansion)
- [ ] 16.3 Verify `BASIC_KUN_START_FILLER` padding shrunk correctly (code replaced filler bytes)
- [ ] 16.4 Verify incremental C++ compilation works after kernel regeneration
- [ ] 16.5 Verify `make test-unit` passes all unit tests
- [ ] 16.6 Verify `make` produces the release binary successfully
- [ ] 16.7 Verify all existing integration tests still compile (no regressions)
