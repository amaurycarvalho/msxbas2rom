## ADDED Requirements

### Requirement: LogEntry fields are deterministically initialized

The `Logger::LogEntry` struct SHALL initialize all of its data members to well-defined values at construction time. Constructing an entry from a severity and message SHALL set `severity` and `message` to the supplied values and SHALL set `file` to an empty string and `line`, `column`, and `code` to `0`. Default-constructing an entry SHALL yield an equivalent well-defined state.

#### Scenario: Entry constructed from severity and message

- **WHEN** a `LogEntry` is constructed from a `LogLevel` severity and a message string
- **THEN** `severity` and `message` SHALL hold the supplied values
- **AND** `file` SHALL be an empty string
- **AND** `line`, `column`, and `code` SHALL be `0`

#### Scenario: Dummy entry for empty messages is well-defined

- **WHEN** `Logger::add()` (or `info`/`warning`/`debug`/`error`) is called with a message that is empty after trimming
- **THEN** the returned entry SHALL NOT contain indeterminate `severity`, `line`, `column`, or `code` values
