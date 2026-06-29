## MODIFIED Requirements

### Requirement: Transform lexed tokens into action and symbol structures

The parser SHALL iterate through lexer lines and build tags/actions for each valid statement. Numeric line labels SHALL be registered and duplicated line numbers SHALL be rejected. Statements split by `:` SHALL be parsed as independent phrases (except protected contexts like IF flow). INCLUDE directives SHALL load extra source only when the parameter is a string literal. Syntax errors SHALL return a clear error message and source line number.

The parser SHALL additionally support DEFINE and DEF FN directives as compile-time macro definitions. Macro definitions SHALL be stored in the parser context and SHALL be expanded before semantic analysis when the macro name (or FN + name pair) is encountered in subsequent source lines. Macro expansion SHALL operate on lexeme streams and SHALL be invisible to the compiler.

#### Scenario: Build actions from a valid numbered program
- **WHEN** lexed input with numbered lines and valid commands undergoes syntactic analysis
- **THEN** the parser returns success
- **AND** tag/action structures are populated for each statement

#### Scenario: Reject invalid INCLUDE usage
- **WHEN** an `INCLUDE` directive with a non-string parameter is parsed
- **THEN** the parser returns failure
- **AND** the error message is `Invalid parameter in INCLUDE keyword`

#### Scenario: Reject duplicated line number declaration
- **WHEN** two parsed lines use the same numeric tag
- **THEN** duplicate tag detection is triggered
- **AND** processing fails with a line declaration conflict message

#### Scenario: Parse and expand a DEFINE directive
- **WHEN** the parser encounters `DEFINE MAX@ AS 100` on a line
- **THEN** the macro is stored in the define map
- **AND** no action/tag structure is generated for that line

#### Scenario: Expand FN invocation in expression
- **WHEN** the parser encounters `FN SQUARE(5)` in an expression
- **THEN** the FN + identifier pair is coalesced and the macro expansion engine replaces it with the body lexemes
- **AND** the expression evaluator sees only the expanded tokens
