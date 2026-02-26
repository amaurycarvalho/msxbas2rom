# US-005 - CLI

## Story
As a command-line user, I need a CLI that validates options and executes lexer, parser, compiler and ROM builder stages with clear diagnostics so that I can convert BASIC sources to ROM artifacts safely.

## Acceptance Criteria
- CLI validates arguments and returns non-zero exit code for invalid parameter combinations.
- CLI supports informational commands (`--help`, `--doc`, `--history`, `--ver`) and exits without compilation.
- CLI enforces input file existence and output overwrite behavior.
- CLI executes stages in order: lexical, syntactic, semantic, ROM build.
- CLI reports stage-specific errors and success metrics depending on quiet/debug/settings.

## BDD Scenarios
### Scenario 1: Show help and exit cleanly
Given the command `msxbas2rom -h`
When CLI starts
Then help text is printed
And process exits with status code 0

### Scenario 2: Reject missing input file
Given CLI invocation without a valid input filename
When compilation flow is requested
Then CLI prints an input-file error message
And process exits with status code 1

### Scenario 3: Run complete conversion pipeline
Given a valid BASIC input file and valid options
When CLI executes
Then lexer, parser, compiler and ROM builder run in sequence
And output ROM file is generated
And success summary is printed when not in quiet mode
