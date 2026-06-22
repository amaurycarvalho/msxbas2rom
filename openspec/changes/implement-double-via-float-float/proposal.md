## Why

MSX BASIC's `subtype_double_decimal` currently occupies 3 bytes and uses the same single-precision math routines as `subtype_single_decimal`, making it a semantic phantom — double-precision variables exist in the language grammar but offer no precision benefit. This change implements true double-precision floating point via the float-float (double-double) technique, where each double is the sum of two single-precision floats (6 bytes total), providing approximately 48 bits of mantissa versus the single's 23 bits. Double-precision is essential for numerical accuracy in scientific computation, financial calculations, and iterative algorithms susceptible to single-precision round-off.

## What Changes

- **New true double type**: `Double` = two singles (6 bytes in RAM: high part + low part), using float-float arithmetic via inline code generation (Dekker-style error-free transforms)
- **Full arithmetic**: add, subtract, multiply, divide, negate, abs for double-double via compiler-emitted Z80 sequences calling existing XBASIC single-precision routines
- **Comparisons**: =, <>, <, <=, >, >= implemented for double operands via inline code generation
- **Type casts**: Integer→Double, Single→Double, Double→Integer, Double→Single, String↔Double
- **Math functions reimplemented** for doubles: SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND — all via inline code generation pattern (cast double to single for existing XBASIC calls is not acceptable)
- **Variable storage**: Double variables occupy 6 bytes (was 3); loading/storing uses B:HL (high) + C:DE (low) register pairs
- **Stack discipline**: Double values push/pop 6 bytes on Z80 hardware stack
- **FOR/NEXT loops**: Use double-precision counter, target, and step when variable is double
- **PRINT USING**: Handles double-precision formatting
- **Debug symbols**: CDB and NoICE export updated for 6-byte double type
- Fix the `#`/`!` suffix mapping bug in `literal_state.cpp` where numeric literal suffixes are swapped versus MSX BASIC convention
- **BREAKING**: `double2FloatLib()` return type changes (now produces two singles instead of one)

## Capabilities

### New Capabilities

- `double-type`: Double-precision floating point type definition, storage (6 bytes), variable allocation, loading/storing from/to RAM using B:HL + C:DE register pairs, type suffix `#`, DEFDBL statement handling, and type identification throughout the compiler pipeline
- `double-operations`: Full float-float arithmetic (add, sub, mul, div, neg, abs), comparisons (=, <>, <, <=, >, >=), type conversions (cast to/from Integer, Single, String), and all math functions (SIN, COS, TAN, ATN, EXP, LOG, SQR, INT, FIX, SGN, ABS, RND) implemented via inline code generation using Dekker error-free transforms calling existing XBASIC single-precision routines

### Modified Capabilities

- `lexer`: Fix `#` and `!` suffix mapping in `literal_state.cpp` to match MSX BASIC convention (`#` = double, `!` = single) and `identifier_state.cpp` behavior
- `compiler`: Expression evaluator, type casting, variable emitter, symbol resolver, FOR statement strategy, and PRINT statement strategy updated to handle 6-byte double type distinctly from 3-byte single

## Impact

- **Domain**: `src/domain/lexer/lexeme.h` (no enum change needed, subtype already exists)
- **Lexer**: `src/application/lexer/states/implementations/literal/literal_state.cpp` (suffix fix), `src/application/lexer/states/implementations/identifier/identifier_state.cpp`, `src/application/lexer/states/implementations/unknown/unknown_state.cpp`
- **Compiler core**: `src/application/compiler/helpers/semantic/compiler_float_converter.{h,cpp}` (double→2 singles), `compiler_expression_evaluator.{h,cpp}` (arithmetic, comparisons, casting), `compiler_variable_emitter.{h,cpp}` (6-byte load/store), `compiler_symbol_resolver.{h,cpp}` (var_size 3→6)
- **Function strategies**: All math strategies under `src/application/compiler/functions/strategies/math/` and `src/application/compiler/functions/strategies/basic/cdbl/`, `csng/`, `cint/`
- **Z80 writer**: `src/infrastructure/kernel/z80.{h,cpp}` (new opcode helpers if needed for register shuffling)
- **Tests**: `tests/unit/src/test_double.cpp` (unit), `tests/integration/MATH/` (new .bas integration tests)
- **Build**: No kernel ASM modifications (inline code generation approach); incremental compilation preserved
- Release 1.3.0.0
