## Why

> **IMPORTANT**: This change is a **rough first pass** — it captures the exploration discovery and a proposed fix direction. It MUST be refined, validated with tests, and reviewed before any implementation (`/opsx-apply`). Do not apply in current state.

MSX BASIC's `subtype_double_decimal` currently occupies 3 bytes and uses the same single-precision math routines as `subtype_single_decimal`, making it a semantic phantom — double-precision variables exist in the language grammar but offer no precision benefit. This change implements true double-precision floating point via the float-float (double-double) technique, where each double is the sum of two single-precision floats (6 bytes total), providing approximately 48 bits of mantissa versus the single's 23 bits. Double-precision is essential for numerical accuracy in scientific computation, financial calculations, and iterative algorithms susceptible to single-precision round-off.

## What Changes

- **New true double type**: `Double` = two singles (6 bytes in RAM: high part + low part), using float-float arithmetic via Dekker error-free transforms
- **Full arithmetic**: add, subtract, multiply, divide, negate, abs for double-double
- **Comparisons**: =, <>, <, <=, >, >= implemented for double operands
- **Type casts**: Integer→Double, Single→Double, Double→Integer, Double→Single, String↔Double
- **Math functions **: SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND for doubles — implemented as first-order Taylor correction using existing XBASIC single-precision calls and the new kernel double utilities for renormalization
- **Variable storage**: Double variables occupy 6 bytes (was 3); loading/storing uses B:HL (high) + C:DE (low) register pairs via lightweight kernel routines (`doubleLoad`, `doubleStore`)
- **Stack discipline**: Double values push/pop 8 bytes on Z80 hardware stack (4 register-pair pushes, 6 bytes of useful data + 2 wasted due to pair alignment)
- **FOR/NEXT loops**: Use double-precision counter, target, and step when variable is double
- **Debug symbols**: CDB and NoICE export updated for 6-byte double type
- Fix the `#`/`!` suffix mapping bug in `literal_state.cpp` where numeric literal suffixes are swapped versus MSX BASIC convention
- **BREAKING**: `double2FloatLib()` return type changes (now produces two singles instead of one)

### Architecture Decision: Mixed Strategy (Kernel Utilities + Inline Generation)

Complex float-float algorithms (TwoSum, FastTwoSum, TwoProd, double division, double comparison, double load/store) are implemented as **size-optimized Z80 kernel utility routines** (~458 bytes total across 8 routines in `90_support.asm`), with 8 new dispatch entries in `wrapper_routines_map_table`. The compiler emits `call doubleTwoSum` (3 bytes) instead of emitting the full TwoSum inline (~100+ bytes). Simple sequences (push/pop, negation via existing `floatNeg`, zero-initialization) remain inline.

This mixed strategy reduces ROM overhead from ~6KB per typical program to ~1KB, keeps runtime speed for singles unchanged, and makes the critical TwoProd (Veltkamp split) implementable — it requires bit-level manipulations on the XBASIC MBF float format that are impossible to express purely via XBASIC arithmetic calls inline.

> **Warning**: The TwoProd / Veltkamp split for double multiplication needs further validation. The XBASIC MBF float format differs from IEEE 754 (sign at bit 23 instead of 31, no hidden bit in mantissa encoding, different exponent bias). The Veltkamp split requires isolating the upper 12 bits of a 23-bit mantissa — feasible in Z80 assembly via masking/shifting, but the exact mask values and normalization steps must be verified against the XBASIC float representation. If the split cannot be made to work with XBASIC-format inputs, the double multiplication algorithm would need revision.

## Capabilities

### New Capabilities

- `double-type`: Double-precision floating point type definition, storage (6 bytes), variable allocation, loading/storing from/to RAM using B:HL + C:DE register pairs, type suffix `#`, DEFDBL statement handling, and type identification throughout the compiler pipeline
- `double-operations`: Full float-float arithmetic (add, sub, mul, div, neg, abs), comparisons (=, <>, <, <=, >, >=), type conversions (cast to/from Integer, Single, String), and all math functions (SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND) implemented via calls to new kernel double utility routines, using Dekker error-free transforms calling existing XBASIC single-precision routines
- `kernel-double-utilities`: Z80 assembly utility routines for float-float arithmetic (doubleTwoSum, doubleFastTwoSum, doubleTwoProd, doubleDiv, doubleCompare, doubleLoadHL, doubleStoreHL) added to `90_support.asm`, with dispatch entries in `wrapper_routines_map_table`

### Modified Capabilities

- `lexer`: Fix `#` and `!` suffix mapping in `literal_state.cpp` to match MSX BASIC convention (`#` = double, `!` = single) and `identifier_state.cpp` behavior
- `compiler`: Expression evaluator, type casting, variable emitter, symbol resolver, FOR statement strategy, and PRINT statement strategy updated to handle 6-byte double type distinctly from 3-byte single; complex arithmetic emitted as `call DISP_double*` instead of inline code blocks
- `kernel-runtime`: `wrapper_routines_map_table` expanded from 224 to 232 entries (8 new dispatch slots); `90_support.asm` gains new double utility routines placed in the existing `BASIC_KUN_START_FILLER` zero-fill area (647 bytes available, routines estimated at ~458 bytes)

## Impact

- **Domain**: `src/domain/lexer/lexeme.h` (no enum change needed, subtype already exists)
- **Lexer**: `src/application/lexer/states/implementations/literal/literal_state.cpp` (suffix fix), `src/application/lexer/states/implementations/identifier/identifier_state.cpp`, `src/application/lexer/states/implementations/unknown/unknown_state.cpp`
- **Kernel assembly**: `src/infrastructure/kernel/asm/src/header/90_support.asm` (new double utility routines in filler area), `src/infrastructure/kernel/asm/src/header/20_runtime.asm` (8 new dw entries in dispatch table)
- **Kernel dispatch**: `src/application/compiler/helpers/hooks/compiler_hooks.h` (8 new DISP_* constants, DISP_ENTRIES 224→232)
- **Kernel binary**: `src/infrastructure/kernel/header.h` regenerated (filler bytes replaced by actual code, same 32768-byte size)
- **Compiler core**: `src/application/compiler/helpers/semantic/compiler_float_converter.{h,cpp}` (double→2 singles), `compiler_expression_evaluator.{h,cpp}` (emit `optimizer.addKernelCall(DISP_double*)` instead of inline sequences), `compiler_variable_emitter.{h,cpp}` (6-byte load/store via kernel calls), `compiler_symbol_resolver.{h,cpp}` (var_size 3→6)
- **Function strategies**: All math strategies under `src/application/compiler/functions/strategies/math/` and `src/application/compiler/functions/strategies/basic/cdbl/`, `csng/`, `cint/`
- **Z80 writer**: `src/infrastructure/kernel/z80.{h,cpp}` (no changes needed; existing push/pop/call ops are sufficient)
- **Tests**: `tests/unit/src/test_double.cpp` (unit), `tests/integration/MATH/` (new .bas integration tests)
- **Build**: Kernel must be reassembled (pasmo → header.h → recompile C++); incremental C++ compilation still works after kernel changes
- Release 1.3.0.0
