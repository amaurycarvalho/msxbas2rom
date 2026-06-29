## 1. Lexer and Domain Changes

- [ ] 1.1 Add `@` to `LexerLineStateContext::isIdentifier()` as a valid non-start character (alongside `$`, `%`, `!`, `#`)
- [ ] 1.2 Add `"DEFINE"` to the keyword set in `lexeme.cpp`

## 2. Macro Definition Storage

- [ ] 2.1 Define `MacroDef` struct in its own header (or in a new `macro_def.h`): fields for `paramNames`, `bodyLexemes`, and `defStyle` (DEFINE vs DEF_FN)
- [ ] 2.2 Add `map<string, MacroDef> defineMap` to `ParserContext`
- [ ] 2.3 Add `resetDefineMap()` call in `ParserContext::reset()`

## 3. Shared Definition Parser

- [ ] 3.1 Implement `parseDefinition()` as a free function or method on `ParserContext` that reads a macro name, optional `(params)`, and body expression, returning a populated `MacroDef`
- [ ] 3.2 Handle syntax variations: DEFINE uses `AS` as separator, DEF FN uses `=` as separator
- [ ] 3.3 Validate: duplicate name detection, non-empty name, well-formed parameter list
- [ ] 3.4 Store the body as a captured `LexerLineContext` (lexeme stream, not raw text)

## 4. DEFINE Directive in Parser Line Evaluation

- [ ] 4.1 In `ParserLineEvaluator::evaluateLine()`, add detection for `DEFINE` keyword (after the numeric line tag check, alongside the existing `INCLUDE` / `FILE` / `TEXT` block)
- [ ] 4.2 Call `parseDefinition()` and store the result in `defineMap`
- [ ] 4.3 Skip code generation for the DEFINE line (no actions/tags created)

## 5. DEF FN Directive in DefStatementStrategy

- [ ] 5.1 In `DefStatementStrategy::execute()`, add detection for `FN` as the first identifier after `DEF` (before the type-name checks)
- [ ] 5.2 Call the same `parseDefinition()` with `=` as the body separator and `DEF_FN` style
- [ ] 5.3 Skip code generation for the DEF FN line

## 6. FN Invocation Coalescing and Macro Expansion

- [ ] 6.1 In `ParserLineEvaluator::evaluateLine()`, add a `coalesceFN()` helper that walks the phrase lexemes and merges `FN` (keyword) + `identifier` into a single keyword lexeme `"FN_" + name` with `subtype_function`
- [ ] 6.2 Implement `expandMacro()` that looks up a lexeme value in `defineMap`, substitutes parameter tokens with argument tokens by identity matching, and returns the expanded lexeme stream
- [ ] 6.3 Hook the expansion into the phrase-building loop: when a lexeme matches a defineMap key (or a coalesced FN_ key), replace the invocation lexemes with expanded body lexemes
- [ ] 6.4 Handle object-like macros (no params): simple replacement of invocation with body lexemes
- [ ] 6.5 Handle function-like macros (with params): match argument lexemes (between parentheses) against parameter names and substitute in body

## 7. Recursive Expansion and Cycle Detection

- [ ] 7.1 Add recursion tracking during expansion (depth counter or visited set per expansion chain)
- [ ] 7.2 Fail with `"Recursive DEFINE expansion"` if a macro references itself directly or transitively
- [ ] 7.3 Ensure transitive expansion works for non-cyclic chains (macro body referencing another macro)

## 8. Error Messages and Edge Cases

- [ ] 8.1 `"Invalid DEFINE syntax"` — malformed directive line
- [ ] 8.2 `"Duplicate DEFINE name"` — redefining an existing macro
- [ ] 8.3 `"Unknown macro argument"` — if parameter name doesn't exist in the definition
- [ ] 8.4 `"Macro argument count mismatch"` — wrong number of invocation arguments
- [ ] 8.5 `"Recursive DEFINE expansion"` — cycle detected
- [ ] 8.6 Ensure macros are NOT expanded inside string literals, REM lines, apostrophe comments, or DATA values (lexer already prevents this since expansion only runs on lexeme streams)

## 9. Tests

- [ ] 9.1 Test DEFINE constant: source → expanded lexeme stream
- [ ] 9.2 Test DEFINE parameterized: source → expanded with argument substitution
- [ ] 9.3 Test DEF FN (definition and invocation)
- [ ] 9.4 Test transitive expansion (A → B → C)
- [ ] 9.5 Test recursive cycle detection
- [ ] 9.6 Test duplicate definition error
- [ ] 9.7 Test argument count mismatch
- [ ] 9.8 Test macro expansion NOT happening inside strings/comments
- [ ] 9.9 Test `@` identifier lexing
- [ ] 9.10 Test DEFINE inside INCLUDE'd file (visibility)
