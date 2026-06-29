## Context

The existing `ParserLineEvaluator::evaluateLine()` handles two preprocess-like directives at the parser level: `INCLUDE` (feeds lines recursively through the full lexer→parser pipeline) and `FILE`/`TEXT` (resource embedding). Both execute after lexing but before semantic analysis.

The expression evaluator already detects `identifier(...)` as function calls with parameters. The `coalesceSymbols()` method in `ParserContext` normalizes identifier symbols and applies type hints from DEFINT/DEFSTR/DEFSNG/DEFDBL.

DEF FN (keyword `FN` exists in the keyword set) and DEFINE (new keyword) are currently unimplemented.

## Goals / Non-Goals

**Goals:**
- Compile-time macro definition via `DEFINE name AS expr` and `DEF FN name(params)=expr`
- Compile-time macro expansion: all invocations replaced with expanded body lexemes before semantic analysis
- Shared parsing core for both syntaxes (parameter list extraction, body expression capture)
- Macro definitions stored in `ParserContext`, visible to `INCLUDE`-loaded files
- Recursive definition detection
- `@` character as valid identifier suffix for compile-time symbol visual convention
- Clear error messages for invalid syntax, duplicate names, argument mismatches, recursive cycles

**Non-Goals:**
- No runtime DEF FN support — `FN` invocation is compile-time expansion only, no Z80 code generation
- No module/namespace scoping — macros are flat in a single map
- No conditional compilation (`#ifdef` equivalent) — pure substitution only
- No text-level preprocessor — operates on lexeme streams, not raw text

## Decisions

### Decision 1: Parser-side macro expansion (same level as INCLUDE)

Macro expansion happens in `parser_line_evaluator.cpp::evaluateLine()`, after lexing and line tag registration, but before `evaluatePhrase()` / `evaluateStatement()`. This mirrors the INCLUDE model.

When `evaluateLine()` iterates lexemes to build phrases (the `while ((lexeme = lexerLine->getNextLexeme()))` loop after a numeric line tag), macro invocation identifiers are detected and expanded in-place — the lexeme stream is rewritten before phrase construction.

**Alternatives considered:**
- *Text-level preprocessor (before lexer)*: Simpler but cannot protect strings, REM, DATA, apostrophe comments from substitution
- *After expression evaluation (action tree level)*: More complex, requires the expression evaluator to invoke macro expansion, polluting the shunting-yard algorithm

**Verdict:** Parser-side is consistent with INCLUDE and keeps macro expansion invisible to the rest of the pipeline.

### Decision 2: Lexeme-stream substitution by token identity

Macro expansion substitutes lexemes by matching `name` field (uppercase, type-sensitive). For function-like macros, parameter name lexemes in the body are replaced by the corresponding argument lexemes (cloned). Object-like macros (no params) replace the invocation lexeme with the body lexemes directly.

- Protection: No substitution inside string literals, REM comments, apostrophe comments, or DATA statements (because the lexer already classifies these — expansion only touches identifier/keyword lexemes at the phrase level)
- `@` suffix: `MAXVALUES@` is the full identifier; the `@` is part of the lexeme value

### Decision 3: `@` in identifiers

`@` is added to `LexerLineStateContext::isIdentifier()` as a non-start character, same category as `$`, `%`, `!`, `#`. This allows `MAXVALUES@` to be lexed as a single identifier token. The `@` has no runtime meaning — it's purely a visual "this is a compile-time name" convention for the programmer.

### Decision 4: FN invocation coalescing

`FN SQUARE(2)` in source is lexed as three tokens: `FN` (keyword), `SQUARE` (identifier), `(` (separator). A new `coalesceFN()` step in `evaluateLine()` merges `FN` + `identifier` into a single combined lexeme: `type_keyword, subtype_function, value="FN_SQUARE"`.

The same `coalesceSymbols()` infrastructure handles deduplication. The expression evaluator then sees `FN_SQUARE(...)` as a standard function keyword with parameters.

### Decision 5: Definition scope with INCLUDE

Macro definitions are stored in `ParserContext`, which is shared across `INCLUDE` boundaries (the `IncludeLoader` reuses the same `lineEvaluator`). Definitions made before an `INCLUDE` are visible in the included file; definitions made inside an included file are visible to the parent file after the include returns.

This is simple, consistent with BASIC's flat namespace, and creates no scope management complexity.

### Decision 6: No 2-character limit on DEF FN names

The original MSX-BASIC only uses the first two characters of a DEF FN name. This implementation uses the full name. The `@` suffix for DEFINE is also useful here — `FN SQUARE@(X)` would be valid if the user wants visual clarity, but is not required.

### Decision 7: Recursive expansion and cycle detection

Expansion is lazy (macro body is stored as-is, parameters substituted at expansion time). When expanding, a recursion depth counter or visited set tracks the current call chain. If a macro references itself (directly or transitively), expansion fails with "Recursive DEFINE expansion" error.

## Risks / Trade-offs

- **[Risk] Macro names shadowing variables**: A `DEFINE MAX AS 10` would replace all occurrences of identifier `MAX` with `10`, even if `MAX` was intended as a variable. **Mitigation**: The `@` convention is the recommended style to avoid this; plain names without `@` still work but may shadow variables.
- **[Risk] FN coalescing changes expression evaluator input**: The `FN identifier` → `FN_IDENTIFIER` coalescing adds a special case. **Mitigation**: The change is localized to `evaluateLine()` and transparent to the expression evaluator.
- **[Risk] Macro visibility across INCLUDE creates coupling**: A macro defined in an include affects the parent file. **Mitigation**: This matches BASIC's flat scope model and is intuitive; document it clearly.
