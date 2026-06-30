## Context

The compiler uses a single-pass code generation approach: it iterates through parsed tags (BASIC source lines) in sequential order and generates Z80 code for each. Array dimension parameters (`x_factor`, `x_size`, `y_size`, `y_factor`, `array_size`) are currently computed by `CompilerDimStatementStrategy::cmd_dim()` during compilation — *after* the DIM tag is reached in the source order.

If an array reference (e.g., `PRINT A%(1)`) appears on a line *before* the `DIM A%(2)` line, the compiler encounters it with `x_factor` still at its default value of 0. The variable emitter computes `offset = index * x_factor = 0`, producing code that always accesses element 0 regardless of the index.

The `coalesceSymbols` mechanism in `parser_context.cpp:80` ensures all occurrences of the same variable share a single lexeme object. The parser's `DimStatementStrategy` already sets `isArray = true` during parsing — but does not set the dimension parameters, leaving them to the compiler phase.

## Goals / Non-Goals

**Goals:**
- Array references work correctly regardless of whether they appear before or after the DIM declaration in source order
- `x_factor`, `x_size`, `y_size`, `y_factor`, `array_size` are set on the shared lexeme during parsing (syntactic analysis), before compilation begins
- Compiler `DimStatementStrategy` only validates pre-computed values instead of recomputing them

**Non-Goals:**
- No changes to the two-phase pipeline (parser → compiler) itself
- No changes to the Z80 kernel, ROM builder, or CLI
- No changes to `compiler_variable_emitter.cpp` or `compiler_expression_evaluator.cpp` — they already read `x_factor` etc. from the lexeme and work correctly when values are set
- No changes to runtime RAM allocation (`saveSymbols`)

## Decisions

### Decision 1: Compute array parameters in parser's DimStatementStrategy

**Chosen**: Move the computation of `x_factor`, `x_size`, `y_size`, `y_factor`, and `array_size` from `CompilerDimStatementStrategy::cmd_dim()` into `ParserDimStatementStrategy::parseStatement()`.

**Rationale**: The parser already has all the information needed:
- Variable subtype (suffix `%`/`!`/`#`/`$` or DEFINT/DEFSNG/DEFDBL/DEFSTR) → determines `x_factor` (2/3/3/256)
- DIM size literal → determines `x_size` / `y_size`
- The lexeme object is already shared via `coalesceSymbols`, so setting these during parsing makes them visible to the compiler regardless of source order

**Alternatives considered:**
- *Two-pass compiler* (Option 2): First pass collects DIM info, second generates code. More robust but requires restructuring the compilation loop — higher risk and scope.
- *Deferred runtime offset* (Option 3): Generate runtime index calculation that reads stride from RAM. Adds overhead to every array access and requires a RAM location for each array's stride.

### Decision 2: Keep compiler DimStatementStrategy for validation only

**Chosen**: The compiler's `cmd_dim` will validate that computed values are non-zero and consistent, but will not recompute them.

**Rationale**: The parser phase already validates syntax. Keeping a lightweight validation in the compiler provides defense-in-depth without adding meaningful complexity.

## Risks / Trade-offs

- **[Subtype mismatch]**: The parser must determine array element type (for `x_factor`) identically to how the compiler previously determined it. If the parser and compiler disagree on subtype for any edge case (e.g., undeclared variable without suffix), `x_factor` could be wrong. **Mitigation**: The parser's `coalesceSymbols` already applies the same DEFINT/DEFSNG/DEFDBL/DEFSTR logic as the compiler would. The subtype is set before `DimStatementStrategy` runs. Verify with unit tests.
- **[Dimension count]**: The compiler previously rejected arrays with >2 dimensions. The parser must enforce the same limit. **Mitigation**: Add the same check in `parseStatement`.
- **[Non-constant DIM size]**: The compiler rejects non-constant DIM sizes with "Array 1st dimension index must be a integer constant". The parser must replicate this check. **Mitigation**: The parser has access to the lexeme stream before expression evaluation — verify the size parameter is a numeric literal and reject if not.
