## ADDED Requirements

### Requirement: Transform lexed tokens into action and symbol structures
As an MSX-BASIC developer, the parser SHALL transform lexed tokens into action and symbol structures so that valid programs can be compiled into ROM code.

The parser SHALL iterate through lexer lines and build tags/actions for each valid statement. Numeric line labels SHALL be registered and duplicated line numbers SHALL be rejected. Statements split by `:` SHALL be parsed as independent phrases (except protected contexts like IF flow). INCLUDE directives SHALL load extra source only when the parameter is a string literal. Syntax errors SHALL return a clear error message and source line number.

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
