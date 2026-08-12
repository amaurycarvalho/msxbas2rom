## Context

The MSX BASIC compiler currently treats `subtype_double_decimal` identically to `subtype_single_decimal` — both occupy 3 bytes in RAM and use the same XBASIC single-precision math pack routines. The `subtype_double_decimal` enum value exists throughout the codebase (100+ references) but provides no precision benefit.

This design implements true double-precision floating point via the float-float (double-double) technique, where each `Double` value is the unevaluated sum of two single-precision floats: `x = x_hi + x_lo`, with `|x_lo| <= 0.5 ulp(x_hi)`. This provides approximately 48 bits of mantissa (2 × 23 bits) compared to single's 23bits.

Complex float-float algorithms (TwoSum, FastTwoSum, TwoProd, division, comparison) are implemented as Z80 kernel utility routines in `90_support.asm`, placed in the existing `BASIC_KUN_START_FILLER` zero-fill area (647 bytes at 0x61FA-0x6480). Simple operations (push/pop, negation) are emitted inline by the compiler. This mixed strategy keeps ROM overhead low (~1KB per program vs ~6KB for pure inline), makes TwoProd (Veltkamp split) implementable, and requires no kernel page expansion.

**Key constraints understood**:
- Kernel binary (`header.bin`) = 32KB; only the 16KB at 0x4000-0x7FFF is written to ROM
- Free space available: 649 bytes (647 in BASIC_KUN_START_FILLER + 2 in KERNEL_END_FILLER)
- Dispatch table: 224 entries (0-223), expandable to 232
- XBASIC float format is MBF (Microsoft Binary Format), NOT IEEE 754: sign at bit 23, no hidden mantissa bit, exponent bias differs
- Kernel already has `floatNeg` (DISP 40) for single-precision negation — usable for double negation

## Goals / Non-Goals

**Goals:**
- Implement double-double arithmetic (add, sub, mul, div, neg, abs) via calls to new kernel utility routines
- Implement double-double comparisons (=, <>, <, <=, >, >=) with correct tie-breaking
- Implement type conversions (Integer↔Double, Single↔Double, String↔Double)
- Implement all math functions (SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND) at double precision via first-order Taylor correction + kernel renormalization utilities
- Store doubles as 6 bytes in RAM (high part at offset 0..2, low part at offset 3..5)
- Use B:HL (high) + C:DE (low) as the register representation
- Fix the `#`/`!` suffix swap bug in `literal_state.cpp`
- Preserve incremental C++ compilation after kernel reassembly

**Non-Goals:**
- Rewrite the XBASIC math pack for native 32-bit mantissa support
- Change the kernel binary size (stays 32768 bytes)
- Support double-precision PRINT USING formatting beyond what single provides (the high part is printed; low part discarded)
- Run automated integration tests (user will run manually)
- Modify Arkos Player or move it from its current address (stays at 0x6481-0x6BF6)

## Decisions

### Decision 1: Mixed Strategy — Kernel Utilities + Inline Short Sequences

**Chosen**: Complex float-float algorithms live as Z80 kernel utility routines in `90_support.asm`. Simple sequences (push/pop, negation, zero-initialization) are emitted inline by the compiler.

**Rationale**: The pure inline approach (original Option B) has three critical problems:
1. **TwoProd requires bit-level manipulation on MBF format** — masking, shifting, splitting 23-bit mantissa into 12+11 bit halves. These operations cannot be expressed via XBASIC arithmetic calls (add/sub/mul/div). A kernel routine solves this.
2. **ROM overhead is prohibitive** — ~120 bytes per double addition inline vs 3 bytes via `call`. A typical program with 50 double operations saves ~4.6KB of ROM.
3. **Kernel space exists** — 647 bytes of zero-fill in `BASIC_KUN_START_FILLER` can hold all 8 utility routines (~458 bytes estimated).

Simple sequences stay inline because they are already minimal (5 bytes for push/pop, 3 bytes for `call floatNeg` × 2 = 6 bytes for double negation vs the kernel `call doubleNeg` cost of 3 bytes + 25 kernel bytes).

**Alternatives considered:**
- *Pure inline (original)*: TwoProd likely impossible; ROM bloat of ~6KB per program
- *Native double rewrite of XBASIC math pack*: Requires ~2250 bytes of kernel code (doesn't fit in 649 bytes), full rewrite of 13 math routines + coefficient tables, maintains two codebases in parallel. Overkill for the float-float precision target.

### Decision 2: Register Layout B:HL (high) + C:DE (low)

**Chosen**: High part in B:HL (same as current single), low part in C:DE.

**Rationale**: Mirrors the XBASIC convention where B:HL holds the first operand and C:DE holds the second. This allows direct calls to `XBASIC_ADD_FLOATS` (which expects B:HL and C:DE) without register shuffling for high-part operations.

### Decision 3: Stack Layout — 8 bytes per double

**Chosen**: Push/pop 8 bytes per double value on the Z80 hardware stack (4 register-pair operations: push hl, push bc, push de, push af). Though the double holds 6 bytes of useful data, Z80 push operates on register pairs, requiring the extra 2 bytes.

```
Push (8 bytes total):
  push hl        ; HL = high mantissa (2 bytes)
  push bc        ; B  = high sign/exp, C = garbage (2 bytes)
  push de        ; DE = low mantissa (2 bytes)
  ld a, c        ; A  = low sign/exp
  push af        ; A  = low sign/exp, F = garbage (2 bytes)

Pop (reverse, 8 bytes):
  pop af         ; A  = low sign/exp
  pop de         ; DE = low mantissa
  pop bc         ; B  = high sign/exp
  pop hl         ; HL = high mantissa
  ld c, a        ; C  = low sign/exp
```

### Decision 4: Kernel Utility Routines — Sizing and Dispatch

Eight new routines placed at the `BASIC_KUN_START_FILLER` location (0x61FA), replacing the current 647-byte zero fill. Size estimates are conservative upper bounds; actual sizes may be smaller after optimization.

| Routine | DISP Index | Purpose | Est. Size | Key Optimizations |
|---------|-----------|---------|-----------|-------------------|
| `doubleFastTwoSum` | 224 | `(sh, e) = FastTwoSum(a, b)` | ~50 B | `exx` for intermediate saves, reuses caller's stack frame |
| `doubleTwoSum` | 225 | `(sh, e) = TwoSum(a, b)` | ~55 B delta | Calls `doubleFastTwoSum` as first step (code sharing), +3 XBASIC calls for error refinement |
| `doubleTwoProd` | 226 | `(ph, e) = TwoProd(a, b)` via Veltkamp | ~160 B | Mask + shift on MBF format, split-and-multiply via XBASIC, inline bit ops |
| `doubleDiv` | 227 | `q = a / b` double-double | ~110 B | Approx quotient + residual refinement + renormalization via `doubleFastTwoSum` |
| `doubleCompare` | 228 | Compare two doubles, return 0/-1 in HL | ~45 B | Early exit on high part mismatch; low-part tie-break only when high equal |
| `doubleLoadHL` | 229 | Load 6 bytes from (HL) into B:HL+C:DE | ~20 B | `ld b,(hl)` + increment pointer through 6 bytes |
| `doubleStoreHL` | 230 | Store B:HL+C:DE to 6 bytes at (HL) | ~18 B | Reverse of load; uses `ex de,hl` to juggle pointer vs mantissa |
| `doubleNeg` | 231 | Negate a double (both high and low) | ~15 B | Calls existing `floatNeg` (DISP 40) twice |

**Total estimated**: ~458 bytes (fits in 647-byte filler with ~189 bytes margin)

**Dispatch table impact**: `wrapper_routines_map_table` grows from 224 to 232 entries (16 bytes added to the 448-byte table). The `DEFS 0x4000 - $` in `20_runtime.asm` automatically absorbs this — the zero padding after the dispatch table simply shrinks by 16 bytes.

### Decision 5: Float-Float Arithmetic Algorithms (Kernel Routines)

**doubleFastTwoSum(a, b) → B:HL=s, C:DE=e** — precondition: `|a| >= |b|`
```
s = a + b                    ; call XBASIC_ADD_FLOATS
e = b - (s - a)              ; sub then sub
```
Uses `exx` to preserve intermediate `s` while loading `a` from shadow registers. Saves/restores `a` and `b` on the hardware stack (push/pop) rather than WRKARE RAM to minimize byte count.

**doubleTwoSum(a, b) → B:HL=s, C:DE=e**
```
(sh, e_F2S) = doubleFastTwoSum(a, b)   ; fast path
t = (a - (s - t)) + (b - t)             ; compute full error term via XBASIC calls
```
Calls `doubleFastTwoSum` internally, then performs 3 additional XBASIC calls for the full TwoSum error term. This code-sharing saves ~30 bytes vs two independent routines.

**doubleTwoProd(a, b) → B:HL=ph, C:DE=pl** via Veltkamp split
```
Split a into a_hi(12 bits) + a_lo(11 bits)
Split b into b_hi(12 bits) + b_lo(11 bits)
ph = a * b                    ; approximate product
pl = ((a_hi*b_hi - ph) + a_hi*b_lo + a_lo*b_hi) + a_lo*b_lo
```
The Veltkamp split operates on the MBF float format:
1. Extract mantissa bits from HL (bits 0-22, sign at bit 23 via H)
2. Mask upper 12 bits of mantissa → a_hi
3. Compute a_lo = a - a_hi via XBASIC subtract
4. Same for b
5. Compute 4 sub-products via XBASIC multiply, combine via XBASIC add/sub

**doubleDiv(a_hi:a_lo / b_hi:b_lo) → B:HL=qh, C:DE=ql**
```
qh = a_hi / b_hi               ; approximate quotient via XBASIC div
r = a_hi - qh * b_hi            ; residual
t = r + a_lo - qh * b_lo        ; combine low parts
ql = t / b_hi                   ; correction via XBASIC div
(qh, ql) = doubleFastTwoSum(qh, ql)  ; renormalize
```

**doubleCompare(B:HL+C:DE, STACK:B:HL+C:DE) → HL=0xFFFF or 0x0000**
```
Compare high parts via XBASIC_COMPARE_FLOATS
If not equal → return result
If equal → compare low parts via XBASIC_COMPARE_FLOATS → return
```
The second operand (on the right of the operator) is passed on the stack in the same 8-byte layout. The routine pops it, compares, and returns.

### Decision 6: Math Functions via First-Order Taylor Correction

Each math function f(x) for double `xh + xl`:
1. Call the XBASIC single-precision function on the high part: `yh = f(xh)`
2. Compute the correction term: `correction = xl * f'(xh)` via XBASIC multiply
3. Renormalize: `(yh, yl) = doubleFastTwoSum(yh, correction)` via kernel call

| Function | Formula | f'(x) |
|----------|---------|-------|
| SIN | `sin(xh) + xl * cos(xh)` | cos(x) |
| COS | `cos(xh) - xl * sin(xh)` | -sin(x) |
| TAN | `tan(xh) + xl / cos(xh)^2` | 1/cos^2(x) |
| ATN | `atan(xh) + xl / (1 + xh^2)` | 1/(1+x^2) |
| EXP | `exp(xh) * (1 + xl)` | exp(x) |
| LOG | `log(xh) + xl / xh` | 1/x |
| SQR | `sqrt(xh) + xl / (2*sqrt(xh))` | 1/(2*sqrt(x)) |
| INT | int(xh), low=0 | — |
| FIX | fix(xh), low=0 | — |
| SGN | sign(xh) as double | — |
| ABS | negate both if xh negative | — |
| RND | two XBASIC_RND calls | — |

Each math function strategy emits inline Z80: calls the XBASIC function, computes the correction via XBASIC multiply/divide, then calls `doubleFastTwoSum` (DISP 224) for renormalization.

### Decision 7: Type Conversions

```
Integer → Double:  int2single(HL) → B:HL,  C:DE = 0
Single → Double:   copy B:HL → B:HL,  C:DE = 0
Double → Integer:  single2int(B:HL) → HL  (discard low part)
Double → Single:   return B:HL only  (discard C:DE)
String → Double:   str2single → B:HL,  C:DE = 0
Double → String:   print(B:HL)  (only print high part)
```

No XBASIC routine changes needed — all conversions use existing single-precision cast routines. Cast code emitted inline (compact, 8-14 bytes each).

### Decision 8: Modified Files — Complete List

| Category | Files | Change |
|----------|-------|--------|
| **Kernel ASM** | `90_support.asm` | New double utility routines in filler area (0x61FA-0x6480) |
| **Kernel ASM** | `20_runtime.asm` | 8 new `dw` entries in dispatch table (DISP 224-231) |
| **Kernel header** | `compiler_hooks.h` | 8 new `DISP_double*` constants, `DISP_ENTRIES` 224→232 |
| **Kernel binary** | `header.h` | Regenerated via pasmo → xxd after assembly |
| **Lexer** | `literal_state.cpp` | Fix `#`/`!` suffix swap |
| **Lexer** | `identifier_state.cpp` | Verify correct behavior |
| **Lexer** | `unknown_state.cpp` | May need double literal handling |
| **Float Converter** | `compiler_float_converter.{h,cpp}` | `double2FloatLib` produces 2 singles |
| **Symbols** | `compiler_symbol_resolver.cpp` | `var_size` 3→6 for double; `x_factor` 3→6 for double arrays |
| **Expression** | `compiler_expression_evaluator.{h,cpp}` | Emit `call DISP_double*` for arithmetic/comparisons; 8-byte double push/pop in `evalOperatorParms` |
| **Variable** | `compiler_variable_emitter.{h,cpp}` | 6-byte load/store for doubles (inline or via `call doubleLoad/Store`) |
| **FOR** | `compiler_for_statement_strategy.cpp` | 6-byte FOR counter/limit/step; loop check via `doubleCompare` |
| **PRINT** | `compiler_print_statement_strategy.cpp` | Double PRINT handling (print high part, discard low for USING) |
| **INPUT** | `compiler_input_statement_strategy.cpp` | Double INPUT handling |
| **CDBL** | `compiler_cdbl_function_strategy.cpp` | Widen to full 6-byte double |
| **CSNG** | `compiler_csng_function_strategy.cpp` | Truncate to 3-byte single (keep high) |
| **CINT** | `compiler_cint_function_strategy.cpp` | Truncate double to integer |
| **Math functions** | `compiler_[sin/cos/tan/atn/exp/log/sqr/int/fix/sgn/abs/rnd]_function_strategy.cpp` | First-order Taylor correction + `call doubleFastTwoSum` |
| **Debug export** | `noice_export_strategy.cpp`, `cdb_export_strategy.cpp` | 6-byte double type reported |
| **Tests** | `tests/unit/src/test_double.cpp` | Unit tests for float converter, type system |
| **Tests** | `tests/integration/MATH/double_*.bas` | Integration test .bas files |

## Risks / Trade-offs

**[ROM size] Kernel utility approach keeps ROM overhead manageable**
Each complex double operation → `call DISP_double*` = 3 bytes (vs ~120 bytes inline). For 50 double operations: ~150 bytes of calls vs ~6KB of inline code. Kernel routines add ~458 bytes (one-time cost). Net savings: ~5.4KB per typical program. The kernel binary remains 32768 bytes — filler bytes are simply replaced by actual code.

**[Performance] Float-float arithmetic is slower than single-precision**
Each double add calls XBASIC_ADD_FLOATS ~5 times vs 1 time for single. Acceptable for most BASIC programs. Single-precision (`!` suffix) when speed is needed; double (`#` suffix) when precision is required.

**[Complexity] TwoProd (double multiplication) is the highest-risk component**
The Veltkamp split requires bit-level manipulation on XBASIC MBF format floats. The exact mask values and sign/exp handling must be validated. If the MBF format proves incompatible with the standard Veltkamp approach, an alternative multiplication algorithm (e.g., Newton iteration for reciprocal + multiply) would be needed.

**[Stack depth] Each double push uses 8 bytes of Z80 hardware stack**
The Z80 stack starts at ~0xF380 (top of RAM). With ~10 nested double binary operations, that's 80 bytes of stack — well within the ~200+ byte safety margin. The expression evaluator already manages stack depth via `evalOperatorParms`.

**[Precision] First-order Taylor correction provides ~48 bits, not full double-double**
The correction term adds the low part's contribution linearly. For well-behaved functions and moderate x_lo, accurate to ~48 bits. Functions with singularities near zero (LOG near 1, SQR near 0) may lose some precision.

**[Kernel binary size] The 16KB ROM page is near capacity**
Only 649 bytes free (647 + 2), with routines estimated at ~458 bytes. A ~189-byte margin exists. If actual routine sizes exceed estimates, the Arkos Player can be moved to the end of the page (as planned in a TODO comment in 90_support.asm:66-72), freeing an additional 1876 bytes contiguous.

**[Build process] Kernel must be reassembled**
`make` in `src/infrastructure/kernel/asm/` runs pasmo → produces `bin/header.bin` → xxd generates `header.h` → C++ recompiles. This is a one-time step when kernel ASM changes. Incremental C++ compilation works normally after the kernel binary is regenerated.

**[Array dimension x_factor for doubles] Coordinated with fix-dim-position-order**
The `fix-dim-position-order` change sets `x_factor=3` for both single and double in the parser. After this change, double arrays need `x_factor=6`. If both changes are applied, the parser's dim strategy must distinguish singles (x_factor=3) from doubles (x_factor=6). This is noted as a cross-change dependency.

## Open Questions

- **Veltkamp split on MBF format**: The split of a 23-bit mantissa single into high(12) and low(11) halves for TwoProd requires bit-level access to the XBASIC MBF float format (sign at bit 23, no hidden bit, biased exponent). The exact mask values and normalization steps must be validated against the MBF representation.
  - **Recommended spike**: Write a standalone .asm that performs the Veltkamp split on representative MBF floats and verifies via pasmo assembly. Check that `a_hi + a_lo == a` and `|a_lo| <= 0.5 ulp(a_hi)`.

- **Double literal overflow detection**: When parsing a literal like `3.14159265358979#`, the float converter will produce a high+low pair. What happens if the user writes a literal with more precision than float-float can represent? The excess bits are silently dropped (same behavior as single literals today).

- **Comparison with single/int operands**: When one operand is double and the other is single or int, the compiler promotes the smaller type to double (low=0). This works for arithmetic but for comparisons, a single with low=0 may not equal a double that holds the "same" value with a non-zero low part. Is this acceptable, or should comparison between double and non-double types first cast the double to single?

- **DIM arrays with x_factor=6**: The `fix-dim-position-order` change currently sets `x_factor=3` for both single and double. After this change, double arrays need `x_factor=6`. The two changes must coordinate — either order of application must be handled.
