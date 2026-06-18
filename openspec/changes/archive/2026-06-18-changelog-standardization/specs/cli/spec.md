## MODIFIED Requirements

### Requirement: Provide CLI that validates options and executes pipeline stages
As a command-line user, the system SHALL provide a CLI that validates options and executes lexer, parser, compiler, and ROM builder stages with clear diagnostics so that users can convert BASIC sources to ROM artifacts safely.

The CLI SHALL validate arguments and return non-zero exit code for invalid parameter combinations. The CLI SHALL support informational commands (`--help`, `--doc`, `--history`, `--ver`) and exit without compilation. The CLI SHALL enforce input file existence and output overwrite behavior. The CLI SHALL execute stages in order: lexical, syntactic, semantic, ROM build. The CLI SHALL report stage-specific errors and success metrics depending on quiet/debug/settings.

The `--history` flag SHALL display the `info_history` string, which contains only the current release entry, a summary of the last 2 releases, and a link to the current release on GitHub — not the full changelog.

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

#### Scenario: History shows current release only
- **WHEN** the command `msxbas2rom --history` is executed
- **THEN** the output SHALL contain the current release entry with categorized changes
- **AND** a summary of the last 2 releases
- **AND** a link to the current release on GitHub
- **AND** NOT contain the full changelog of all releases
