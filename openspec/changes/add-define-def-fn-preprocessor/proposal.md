## Why

> **IMPORTANT**: This change is a **rough first pass** — it captures the exploration discovery and a proposed fix direction. It MUST be refined, validated with tests, and reviewed before any implementation (`/opsx-apply`). Do not apply in current state.

MSX-BASIC has no preprocessor. Constants must be repeated literally, and reusable expression logic requires runtime DEF FN or duplication. Adding compile-time macro expansion (DEFINE) alongside the familiar DEF FN syntax lets users write cleaner, more maintainable code without runtime overhead, and shares implementation logic between both syntaxes.

## What Changes

- New `DEFINE` directive: compile-time macro for constant substitution and parameterized expression expansion (`DEFINE MAXVALUES@ AS 10`)
- Full `DEF FN` / `FN` support using the same compile-time expansion engine (`DEF FN SQUARE(X) = X^2` → `PRINT FN SQUARE(2)` expands to `PRINT 2^2` at compile time)
- `@` character added as a valid identifier character (used as visual marker for compile-time symbols)
- `DEFINE` added to the keyword set
- Existing `DefStatementStrategy` enhanced to handle both `DEF FN` and `DEFINE` through a shared `parseDefinition()` core
- Macro definitions stored in `ParserContext` and visible to `INCLUDE`-loaded files
- Recursive definition detection with clear error messages
- No runtime FN machinery — both DEFINE and DEF FN are pure compile-time lexeme substitution, producing no code, hooks, or runtime support

## Capabilities

### New Capabilities

- `compile-time-macros`: Compile-time macro definition and expansion via DEFINE and DEF FN syntax, including parameterized (function-like) macros, argument substitution by token identity, recursive expansion (with cycle detection), and scope sharing with INCLUDE

### Modified Capabilities

- `lexer`: Add `@` as a valid identifier character (not at start position) to support the `NAME@` compile-time symbol convention
- `parser`: Add DEFINE directive handling, DEF FN / FN invocation handling in expression evaluation, and a new macro expansion step in the parser line evaluation pipeline

## Impact

- **Lexer** (`lexer_line_state.cpp`): One-line change to `isIdentifier()` to accept `@`
- **Parser** (`parser_line_evaluator.cpp`, `def_statement_strategy.cpp`, `expression_evaluator.cpp`, `parser_context.h/cpp`): New macro definition storage, shared parseDefinition logic, macro expansion step before evaluatePhrase, FN-name coalescing in expression path
- **Domain** (`lexeme.cpp`): Add `"DEFINE"` to keyword set
- **No compiler changes**: Macro expansion is purely a parser-side transform; the compiler never sees macro invocations
