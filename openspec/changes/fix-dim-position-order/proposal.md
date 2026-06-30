## Why

> **IMPORTANT**: This change is a **rough first pass** — it captures the exploration discovery and a proposed fix direction. It MUST be refined, validated with tests, and reviewed before any implementation (`/opsx-apply`). Do not apply in current state.

When a `DIM` statement appears on a line in the middle of the source code, array variables declared there cannot be correctly used on lines that appear before the DIM line. The compiler's single-pass code generation encounters array references before the DIM statement has set the array's stride (`x_factor`), size (`x_size`), and total memory (`array_size`) on the shared lexeme. With `x_factor` still at its default value of 0, all array index computations produce an offset of 0, so every pre-DIM array access reads/writes element 0 regardless of the index used.

RAM allocation (`saveSymbols`) is not affected because it runs after all compilation completes, so `array_size` is available by then. Only code emission is broken.

## What Changes

- **Parser `DimStatementStrategy`**: Compute `x_factor`, `x_size`, `y_size`, `y_factor`, `array_size` on the lexeme during parsing (syntactic analysis), so these values are available to the compiler regardless of source-line ordering.
- **Compiler `DimStatementStrategy`**: Remove duplicate computation of those same array parameters. The compiler phase only validates what the parser already computed.
- **Compiler variable emitter / expression evaluator**: No structural changes needed — they already read `x_factor` etc. from the lexeme. With the values set during parsing, they will work correctly regardless of DIM position.

## Capabilities

### New Capabilities

(none)

### Modified Capabilities

- `parser`: DIM statement strategy SHALL compute array dimension parameters (x_factor, x_size, y_size, y_factor, array_size) during syntactic analysis, in addition to setting `isArray = true`.
- `compiler`: DIM statement strategy SHALL validate array parameters computed by the parser rather than recomputing them. Array references in expressions SHALL work correctly even when they appear on source lines before the DIM declaration.

## Impact

- `src/application/parser/statements/strategies/dim/dim_statement_strategy.cpp` — gains array parameter computation logic
- `src/application/compiler/statements/strategies/basic/dim/compiler_dim_statement_strategy.cpp` — loses duplicate computation, becomes validation-only
- No changes to `compiler_variable_emitter.cpp`, `compiler_expression_evaluator.cpp`, or `compiler_symbol_resolver.cpp`
- No changes to the Z80 kernel, ROM builder, or CLI
