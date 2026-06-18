## ADDED Requirements

### Requirement: Provide CLI that validates options and executes pipeline stages
The system SHALL provide a CLI that validates options and executes lexer, parser, compiler, and ROM builder stages with clear diagnostics so that users can convert BASIC sources to ROM artifacts safely.

#### Scenario: Show help and exit cleanly
- **WHEN** the command `msxbas2rom -h` is executed
- **THEN** help text is printed
- **AND** process exits with status code 0

#### Scenario: Reject missing input file
- **WHEN** CLI is invoked without a valid input filename
- **THEN** CLI prints an input-file error message
- **AND** process exits with status code 1

#### Scenario: Run complete conversion pipeline
- **WHEN** a valid BASIC input file and valid options are provided
- **THEN** lexer, parser, compiler, and ROM builder run in sequence
- **AND** output ROM file is generated
- **AND** success summary is printed when not in quiet mode
