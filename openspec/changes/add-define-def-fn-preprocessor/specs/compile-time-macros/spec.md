## ADDED Requirements

### Requirement: Define compile-time object-like macros with DEFINE

The system SHALL support `DEFINE` directives that bind a name to a constant expression at compile time. Occurrences of the macro name in source code SHALL be replaced by the defined expression tokens before semantic analysis.

DEFINE directives SHALL appear at the start of a physical source line (may follow a numeric line tag). The name MAY use any characters valid in MSX-BASIC identifiers plus the `@` suffix. The expression SHALL be the remainder of the line after the `AS` keyword, lexed as a sequence of tokens.

Macro definitions SHALL apply from the point of declaration forward, including within files loaded via `INCLUDE`. Redefining an existing macro name SHALL be an error.

#### Scenario: Define and use a numeric constant
- **WHEN** the source contains:
  ```
  DEFINE MAXVALUES@ AS 10
  20 DIM A(MAXVALUES@)
  ```
- **THEN** the parser sees `20 DIM A(10)` before semantic analysis
- **AND** no runtime code is generated for the DEFINE line itself

#### Scenario: Duplicate macro definition
- **WHEN** the source defines the same macro name twice
- **THEN** the parser fails with a duplicate definition error

#### Scenario: Define using a numeric line tag
- **WHEN** the source contains `10 DEFINE PI@ AS 3.1415`
- **THEN** the macro is defined at line 10 and applies to subsequent lines

### Requirement: Define compile-time function-like macros with DEFINE

The system SHALL support parameterized DEFINE directives. Parameters SHALL be listed in parentheses after the macro name. When the macro is invoked with arguments, each parameter name in the body token stream SHALL be replaced by the corresponding argument token stream, using token identity matching.

Argument substitution SHALL be by full token identity (parameter name field matches exactly). Substitution SHALL NOT occur inside string literals, comments, or DATA statements.

Argument count mismatch (wrong number of arguments at invocation) SHALL be an error. Unknown parameter names in the body SHALL be left as-is.

#### Scenario: Define and invoke a function-like constant macro
- **WHEN** the source contains:
  ```
  DEFINE ADD@(X, Y) AS X + Y
  10 PRINT ADD@(3, 4)
  ```
- **THEN** the parser sees `10 PRINT 3 + 4` before semantic analysis

#### Scenario: Argument count mismatch
- **WHEN** a function-like macro is invoked with the wrong number of arguments
- **THEN** the parser fails with an error indicating the expected and actual counts

### Requirement: Define compile-time macros with DEF FN syntax

The system SHALL support the traditional MSX-BASIC `DEF FN` syntax. `DEF FN name(params) = expression` SHALL be treated identically to `DEFINE name(params) AS expression` — both are compile-time macro definitions.

The `FN` prefix SHALL be used at invocation: `FN name(args)`. The system SHALL coalesce `FN name` into a single lookup key before expansion.

The full function name SHALL be used for matching (no 2-character limit). No runtime FN support is required — the invocation expands at compile time.

#### Scenario: Define and invoke via DEF FN syntax
- **WHEN** the source contains:
  ```
  10 DEF FN SQUARE(X) = X ^ 2
  20 PRINT FN SQUARE(5)
  ```
- **THEN** the parser sees `20 PRINT 5 ^ 2` before semantic analysis
- **AND** no runtime FN dispatch is generated

#### Scenario: DEF FN without parameters (constant alias)
- **WHEN** the source contains `DEF FN PI() = 3.1415`
- **THEN** `FN PI()` expands to `3.1415` at each invocation

### Requirement: Recursive expansion with cycle detection

Macro bodies MAY reference other macros defined earlier. The system SHALL expand references transitively: if `A@` references `B@`, the expansion of `A@` SHALL include `B@`'s substitution.

Recursive cycles SHALL be detected and reported. If `A@` references `B@` and `B@` references `A@` (directly or through a chain), expansion SHALL fail with a clear error message.

#### Scenario: Transitive macro expansion
- **WHEN** the source contains:
  ```
  DEFINE PI@ AS 3.1415
  DEFINE TWO_PI@ AS 2 * PI@
  10 PRINT TWO_PI@
  ```
- **THEN** `TWO_PI@` expands to `2 * 3.1415`

#### Scenario: Recursive cycle detection
- **WHEN** the source defines `A@` referencing `B@` and `B@` referencing `A@`
- **THEN** expansion fails with a recursive definition error

### Requirement: Macro expansion protection in non-code contexts

Macro names SHALL NOT be expanded when they appear inside:
- String literals
- REM (remark) statements and apostrophe (`'`) comments
- DATA statement values

This protection SHALL apply to both `DEFINE` and `DEF FN` style macros.

#### Scenario: No expansion inside strings
- **WHEN** the source contains:
  ```
  DEFINE MAX@ AS 10
  10 PRINT "MAX@"
  ```
- **THEN** the string `"MAX@"` is printed literally, not expanded to `10`

#### Scenario: No expansion inside comments
- **WHEN** the source contains:
  ```
  DEFINE SECRET@ AS 42
  10 REM SECRET@ is hidden
  ```
- **THEN** the comment is preserved as-is
