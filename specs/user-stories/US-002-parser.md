# US-002 - Parser

## Story
As a MSX BASIC developer, I need the parser to transform lexed tokens into action and symbol structures so that valid programs can be compiled into ROM code.

## Acceptance Criteria
- Parser iterates through lexer lines and builds tags/actions for each valid statement.
- Numeric line labels are registered and duplicated line numbers are rejected.
- Statements split by `:` are parsed as independent phrases (except protected contexts like IF flow).
- INCLUDE directives load extra source only when parameter is a string literal.
- Syntax errors return a clear error message and source line number.

## BDD Scenarios
### Scenario 1: Build actions from a valid numbered program
Given lexed input with numbered lines and valid commands
When syntactic analysis is executed
Then the parser returns success
And tag/action structures are populated for each statement

### Scenario 2: Reject invalid INCLUDE usage
Given an `INCLUDE` directive with a non-string parameter
When syntactic analysis is executed
Then the parser returns failure
And the error message is `Invalid parameter in INCLUDE keyword`

### Scenario 3: Reject duplicated line number declaration
Given two parsed lines using the same numeric tag
When parser output is consumed by compilation flow
Then duplicate tag detection is triggered
And processing fails with a line declaration conflict message
