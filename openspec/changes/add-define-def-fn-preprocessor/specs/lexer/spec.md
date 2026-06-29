## MODIFIED Requirements

### Requirement: Tokenize MSX-BASIC source code into classified lexemes

The lexer SHALL load the input file and process it line by line. Numeric line tags SHALL be preserved as numeric literals for downstream parsing. Reserved keywords and known functions SHALL be classified correctly. String, numeric, operator, separator, and comment tokens SHALL be emitted with consistent subtype metadata. Lexical errors SHALL return a clear message and the failing source line number.

The lexer SHALL accept `@` as a valid identifier character (non-start position only, same rule as `$`, `%`, `!`, `#`), enabling identifiers such as `MAXVALUES@` to be lexed as a single identifier token.

#### Scenario: Tokenize a standard BASIC line
- **WHEN** lexical analysis is executed on input `10 PRINT "HELLO"`
- **THEN** the lexer outputs numeric literal `10`, keyword `PRINT`, and string literal `"HELLO"`
- **AND** token order matches source order

#### Scenario: Detect invalid lexical content
- **WHEN** lexical analysis is executed on input containing an unsupported token sequence
- **THEN** the lexer fails
- **AND** an error message is set with the offending line number

#### Scenario: Recognize statement separators and inline comments
- **WHEN** lexical analysis is executed on input with multiple statements and apostrophe comment syntax
- **THEN** separators `:` are emitted as separator tokens
- **AND** apostrophe comments are handled according to lexer rules

#### Scenario: Lex identifier with @ suffix
- **WHEN** lexical analysis is executed on input `MAXVALUES@`
- **THEN** the lexer emits a single identifier token `MAXVALUES@`
