## Context

The MSX BASIC compiler currently treats `subtype_double_decimal` identically to `subtype_single_decimal` — both occupy 3 bytes in RAM and use the same XBASIC single-precision math pack routines. The `subtype_double_decimal` enum value exists throughout the codebase (100+ references) but provides no precision benefit.

This design implements true double-precision floating point via the float-float (double-double) technique, where each `Double` value is the unevaluated sum of two single-precision floats: `x = x_hi + x_lo`, with `|x_lo| <= 0.5 ulp(x_hi)`. This provides approximately 48 bits of mantissa (2 × 23 bits) compared to single's 23 bits.

All arithmetic is emitted as inline Z80 code generation (Option B), preserving the existing pattern of not modifying the Z80 kernel assembly. This approach generates more ROM code per operation but keeps the build process simple and incremental.

## Goals / Non-Goals

**Goals:**
- Implement double-double arithmetic (add, sub, mul, div, neg, abs) via inline Z80 code using Dekker error-free transforms
- Implement double-double comparisons (=, <>, <, <=, >, >=) with correct tie-breaking on high+low parts
- Implement type conversions (Integer↔Double, Single↔Double, String↔Double)
- Implement all math functions (SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND) at double precision via first-order Taylor correction using existing single-precision XBASIC calls
- Store doubles as 6 bytes in RAM (high part at offset 0..2, low part at offset 3..5)
- Use B:HL (high) + C:DE (low) as the register representation
- Fix the `#`/`!` suffix swap bug in `literal_state.cpp`
- Preserve incremental compilation (no unnecessary clean targets)

**Non-Goals:**
- Modify the Z80 kernel ASM files
- Change the XBASIC math pack
- Optimize for ROM space (correctness over compactness)
- Support double-precision PRINT USING formatting beyond what single provides
- Run automated integration tests (user will run manually)

## Decisions

### Decision 1: Inline Code Generation Over Kernel Routines

**Chosen**: Option B — Compiler emits full float-float algorithm inline, calling existing XBASIC single-precision routines.

**Rationale**: No kernel ASM modifications needed; the Z80 kernel binary (`header.symbols.asm`) remains untouched. The existing build infrastructure (Makefile with `SRC_FILES` auto-discovery, `-MMD -MP` dependency tracking) works without changes. The compiler already emits complex multi-instruction sequences for each type-specific operation (see `compiler_expression_evaluator.cpp`), so this pattern is well-established.

**Alternative considered**: Option A (new kernel routines) would produce 3-byte `call` instructions instead of 30+ byte inline sequences. Rejected because it requires reassembling the Z80 kernel, updating `header.symbols.asm` address tables, and changing `compiler_hooks.h` address defines — complex and error-prone.

### Decision 2: Register Layout B:HL (high) + C:DE (low)

**Chosen**: High part in B:HL (same as current single), low part in C:DE.

**Rationale**: Mirrors the XBASIC convention where B:HL holds a single-precision value and C:DE holds the second operand. This allows direct calls to `def_XBASIC_ADD_FLOATS` (which expects B:HL and C:DE) without register shuffling for the high-part operations. The low part uses C:DE, which is the natural second-register-pair for XBASIC.

### Decision 3: Stack Layout — 8 bytes per double

**Chosen**: Push/pop 8 bytes per double value on the Z80 hardware stack.

```
Push order (bottom to top):
  push hl        ; HL = high mantissa
  push bc        ; B = high sign/exp  (+ extra C byte)
  push de        ; DE = low mantissa
  ld a, c
  push af        ; A = low sign/exp  (+ flags byte)

Pop order (reverse):
  pop af         ; A = low sign/exp
  pop de         ; DE = low mantissa  
  pop bc         ; B = high sign/exp
  pop hl         ; HL = high mantissa
  ld c, a        ; C = low sign/exp
```

**Rationale**: Z80 `push` operates on register pairs. Since each 3-byte single needs 4 bytes on stack (push of the 2-byte mantissa + push of a pair containing the 1-byte sign/exp), a double takes 8 bytes. The pop sequence restores all registers correctly with a single `ld c, a` at the end to set C.

### Decision 4: Float-Float Arithmetic Algorithms

All algorithms follow Dekker (1971), implemented as compiler-emitted Z80 sequences calling existing XBASIC single-precision routines at their fixed addresses (`def_XBASIC_ADD_FLOATS`, etc.).

**TwoSum(a, b) → (s, e)**
```
s = a + b                    ; call XBASIC_ADD_FLOATS  (B:HL + C:DE → B:HL)
t = s - a                    ; call XBASIC_SUBTRACT_FLOATS
e = (a - (s - t)) + (b - t)  ; two more sub/add calls
```

**FastTwoSum(a, b) → (s, e)** — precondition: |a| >= |b|
```
s = a + b                    ; call XBASIC_ADD_FLOATS
e = b - (s - a)              ; sub then sub
```

**Double addition**: high1:low1 + high2:low2 → high:low
```
(sh, s)  = TwoSum(high1, high2)
t        = low1 + low2       ; add
t        = t + s             ; add
(sh, sl) = FastTwoSum(sh, t)
```

**Double subtraction**: high1:low1 - high2:low2 → high:low
```
Negate high2 (subtract from zero → high2'=B:HL)
Negate low2  (subtract from zero → low2'=C:DE)
Double_add(high1:low1, high2':low2')
```

**Double multiplication**: high1:low1 * high2:low2 → high:low
```
(ph, p)  = TwoProd(high1, high2)   ; ph = high1*high2, p = error
t        = high1 * low2             ; cross terms
t        = t + (low1 * high2)
t        = t + p
(ph, pl) = FastTwoSum(ph, t)
```
The `TwoProd` operation uses Veltkamp splitting: split the 23-bit mantissa of each single into high(12 bits) and low(11 bits), multiply the pieces, and recombine. Implemented via XBASIC multiply and subtract calls.

**Double division**: high1:low1 / high2:low2 → high:low
```
qh       = high1 / high2         ; approximate quotient
r        = high1 - qh * high2    ; residual
t        = r + low1 - qh * low2  ; combine low parts
q        = t / high2             ; correction
(qh, ql) = FastTwoSum(qh, q)     ; renormalize
```

### Decision 5: Math Functions via First-Order Taylor Correction

Each math function f(x) is implemented as:
1. Call the XBASIC single-precision function on the high part: `y_hi = f(x_hi)`
2. Compute the correction term: `correction = x_lo * f'(x_hi)` using XBASIC multiply
3. Combine: `(y_hi, y_lo) = FastTwoSum(y_hi, correction)`

**SIN**: `sin(xh + xl) ≈ sin(xh) + xl * cos(xh)`
**COS**: `cos(xh + xl) ≈ cos(xh) - xl * sin(xh)`  
**TAN**: `tan(xh + xl) ≈ tan(xh) + xl / (cos(xh))^2`
**ATN**: `atan(xh + xl) ≈ atan(xh) + xl / (1 + xh^2)`
**EXP**: `exp(xh + xl) ≈ exp(xh) + xl * exp(xh)` (= exp(xh) * (1 + xl))
**LOG**: `log(xh + xl) ≈ log(xh) + xl / xh`
**SQR**: `sqrt(xh + xl) ≈ sqrt(xh) + xl / (2*sqrt(xh))`
**INT/FIX**: Apply integer truncation to the high part only; result is double-double with zero low part
**SGN**: Return sign of high part as a double-double
**ABS**: Negate both parts if high part is negative
**RND**: Call XBASIC_RND twice to produce two random singles, use as high and low parts

### Decision 6: Type Conversions

```
Integer → Double:  int2single(HL) → B:HL,  C:DE = 0
Single → Double:   copy B:HL → B:HL,  C:DE = 0
Double → Integer:  single2int(B:HL) → HL  (discard low part)
Double → Single:   return B:HL only  (discard C:DE)
String → Double:   str2single → B:HL,  C:DE = 0
Double → String:   print(B:HL)  (only print high part)
```

No XBASIC routine changes needed — all conversions use existing single-precision cast routines.

### Decision 7: Modified Files — Complete List

| Category | Files | Change |
|----------|-------|--------|
| **Lexer** | `literal_state.cpp` | Fix `#`/`!` suffix swap |
| **Lexer** | `identifier_state.cpp` | Verify correct behavior |
| **Lexer** | `unknown_state.cpp` | May need double literal handling |
| **Float Converter** | `compiler_float_converter.{h,cpp}` | `double2FloatLib` produces 2 singles |
| **Symbols** | `compiler_symbol_resolver.cpp` | var_size 3→6 for double |
| **Expression** | `compiler_expression_evaluator.{h,cpp}` | Add all double arithmetic, comparisons, casts |
| **Variable** | `compiler_variable_emitter.{h,cpp}` | 6-byte load/store for doubles |
| **FOR** | `compiler_for_statement_strategy.cpp` | 6-byte FOR counter/limit/step |
| **PRINT** | `compiler_print_statement_strategy.cpp` | Double PRINT handling |
| **INPUT** | `compiler_input_statement_strategy.cpp` | Double INPUT handling |
| **CDBL** | `compiler_cdbl_function_strategy.cpp` | Widen to full 6-byte double |
| **CSNG** | `compiler_csng_function_strategy.cpp` | Truncate to 3-byte single (keep high) |
| **CINT** | `compiler_cint_function_strategy.cpp` | Truncate double to integer |
| **Math functions** | `compiler_sin/cos/tan/atn/exp/log/sqr/int/fix/sgn/abs/rnd_function_strategy.cpp` | First-order Taylor correction for doubles |
| **Debug export** | `noice_export_strategy.cpp`, `cdb_export_strategy.cpp` | 6-byte double type reported |
| **Tests** | `tests/unit/src/test_double.cpp` | Unit tests for float converter, type system |
| **Tests** | `tests/integration/MATH/double_*.bas` | Integration test .bas files |

## Risks / Trade-offs

**[ROM size] Inline code generation increases ROM usage per double operation**
→ Each double add/sub generates ~120+ bytes of Z80 code (vs 3 bytes for a `call`). For a typical program with 50 double operations, this is ~6KB of ROM. Mitigation: The codebase already targets MegaROM (ASCII8/KonamiSCC) with up to 1MB capacity. If ROM size becomes critical, Option A (kernel routines) can be incrementally adopted later without changing the type system or variable layout.

**[Performance] Float-float arithmetic is slower than single-precision**
→ Each double add calls XBASIC_ADD_FLOATS ~4 times vs 1 time for single. For a Z80 at 3.58MHz, this is a few hundred extra cycles per operation. Acceptable for most BASIC programs. Mitigation: Use single-precision (`!` suffix) when maximum speed is needed; use double (`#` suffix) only when precision is required.

**[Complexity] TwoProd (double multiplication) is the most complex algorithm**
→ Requires Veltkamp splitting of singles into high/low 12/11-bit halves, which involves bit manipulation on the XBASIC float format (not IEEE 754). Mitigation: Implement the split via integer operations on the float's bit representation using the compiler's compile-time knowledge. Provide this as a helper method in `CompilerFloatConverter` and generate calls to generic single-precision split routines inline.

**[Stack depth] Each double push uses 8 bytes of Z80 hardware stack**
→ The Z80 stack starts at ~0xF380 (top of RAM). With ~10 nested double binary operations, that's 80 bytes of stack — well within the ~200+ byte safety margin. Mitigation: The expression evaluator already manages stack depth via `evalOperatorParms`. No change needed.

**[Precision] First-order Taylor correction for math functions provides ~48 bits, not full double-double**
→ The correction term adds the low part's contribution linearly. For well-behaved functions and moderate values of x_lo, this is accurate to ~48 bits. Mitigation: Functions with singularities near zero (LOG near 1, SQR near 0) may lose some precision; document this limitation. Full double-double implementations of transcendental functions would require substantially more code.

## Open Questions

- **Veltkamp split implementation**: The split of a 23-bit mantissa single into high(12) and low(11) halves for TwoProd requires bit-level access to the XBASIC float format. Can this be done purely via XBASIC arithmetic calls, or does it need inline bit manipulation Z80 code (masking, shifting)?
  - **Resolution during implementation**: Attempt arithmetic-only approach first (multiply by carefully chosen constants to split). If infeasible, add inline masking operations.

- **Double literal overflow detection**: When parsing a literal like `3.14159265358979#`, the float converter will produce a high+low pair. What happens if the user writes a literal with more precision than float-float can represent? The excess bits are silently dropped (same behavior as single literals today).
