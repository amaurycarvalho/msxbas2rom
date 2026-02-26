# US-001 - Lexer

## Story
As a MSX BASIC developer, I need the lexer to tokenize source code lines into classified lexemes so that the parser can consume a normalized stream of tokens.

## Acceptance Criteria
- The lexer loads the input file and processes it line by line.
- Numeric line tags are preserved as numeric literals for downstream parsing.
- Reserved keywords and known functions are classified correctly.
- String, numeric, operator, separator and comment tokens are emitted with consistent subtype metadata.
- Lexical errors return a clear message and the failing source line number.

## BDD Scenarios
### Scenario 1: Tokenize a standard BASIC line
Given an input line `10 PRINT "HELLO"`
When lexical analysis is executed
Then the lexer outputs numeric literal `10`, keyword `PRINT`, and string literal `"HELLO"`
And token order matches source order

### Scenario 2: Detect invalid lexical content
Given an input containing an unsupported token sequence
When lexical analysis is executed
Then the lexer fails
And an error message is set
And the error line number points to the offending line

### Scenario 3: Recognize statement separators and inline comments
Given an input line with multiple statements and apostrophe comment syntax
When lexical analysis is executed
Then separators `:` are emitted as separator tokens
And apostrophe comments are emitted/handled as comment/operator marker according to lexer rules
