# Parser

## Purpose

Defines the parser contract: transforming lexed tokens into action and symbol structures for compilation.

## Requirements

### Requirement: Transform lexed tokens into action and symbol structures
As an MSX-BASIC developer, the parser SHALL transform lexed tokens into action and symbol structures so valid programs can be compiled into ROM code. It SHALL build tags/actions per statement, register numeric line labels while rejecting duplicates, parse `:`-separated statements as independent phrases except protected contexts, load INCLUDE only for string literals, and report syntax errors with the source line number.

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
