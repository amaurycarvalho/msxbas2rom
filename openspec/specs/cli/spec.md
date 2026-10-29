# CLI

## Purpose

Defines the contract for the `msxbas2rom` command-line interface: argument validation, compile mode selection, input/output path handling, verbosity and informational flags, debugger symbol export, and VSCode project scaffolding initialization.

## Requirements

### Requirement: Provide CLI that validates options and executes pipeline stages
As a command-line user, the system SHALL provide a CLI that validates options and runs the lexer, parser, compiler, and ROM builder stages with clear diagnostics. It SHALL reject invalid parameter combinations with a non-zero exit code, enforce input existence and output overwrite behavior, support the compile-mode and informational flags, and report stage-specific errors and success metrics. On Windows it SHALL safely initialize command-line arguments before parsing.

#### Scenario: Compile mode flags select the mapper
- **WHEN** a compile mode flag is supplied
- **THEN** `-c`/`--compile` and `-0`/`--plain` SHALL select Plain ROM
- **AND** `-x`/`--megarom` SHALL select ASCII8 as the default MegaROM mode
- **AND** `-8`/`--ascii8`, `-k`/`--scc`, `-4`/`--konami`, `-6`/`--ascii16`, and `-7`/`--ascii16x` SHALL select their mappers
- **AND** `-a`/`--auto` SHALL fall back to ASCII8 when Plain ROM overflows

#### Scenario: Informational flags exit without compiling
- **WHEN** `--help`, `--doc`, `--history`, or `--ver` is supplied
- **THEN** the requested information is printed
- **AND** no compilation SHALL run

#### Scenario: Show help and exit cleanly
- **WHEN** the command `msxbas2rom -h` is executed
- **THEN** help text is printed
- **AND** the help text lists `-4`/`--konami` and `-6`/`--ascii16` among compile options
- **AND** process exits with status code 0

#### Scenario: Windows no-argument startup shows help
- **WHEN** the Windows executable is run without arguments
- **THEN** the application prints the header and help text
- **AND** process exits with status code 0
- **AND** the process does not crash, abort, or return silently before printing output

#### Scenario: Windows help startup survives Unicode conversion failure
- **WHEN** Windows Unicode argv conversion is unavailable or returns a conversion failure before CLI parsing
- **AND** the Windows executable is run with `-h` or `--help`
- **THEN** the application falls back to original `argv`
- **AND** help text is printed
- **AND** process exits with status code 0
- **AND** no allocation is attempted from a negative or zero conversion length

#### Scenario: Windows version startup survives Unicode conversion failure
- **WHEN** Windows Unicode argv conversion is unavailable or returns a conversion failure before CLI parsing
- **AND** the Windows executable is run with `-v` or `--version`
- **THEN** the application falls back to original `argv`
- **AND** version text is printed
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

#### Scenario: Select Konami4 compile mode
- **WHEN** the command `msxbas2rom -4 program.bas` is executed
- **THEN** `compileMode` is set to `Konami4`
- **AND** output filename contains `[Konami]`
- **AND** `megaROM` flag is set to true

#### Scenario: Select Konami4 via long flag
- **WHEN** the command `msxbas2rom --konami program.bas` is executed
- **THEN** `compileMode` is set to `Konami4`
- **AND** output filename contains `[Konami]`
- **AND** `megaROM` flag is set to true

#### Scenario: Select ASCII16 compile mode
- **WHEN** the command `msxbas2rom -6 program.bas` is executed
- **THEN** `compileMode` is set to `ASCII16`
- **AND** output filename contains `[ASCII16]`
- **AND** `megaROM` flag is set to true

#### Scenario: Select ASCII16 via long flag
- **WHEN** the command `msxbas2rom --ascii16 program.bas` is executed
- **THEN** `compileMode` is set to `ASCII16`
- **AND** output filename contains `[ASCII16]`
- **AND** `megaROM` flag is set to true

### Requirement: CLI can scaffold a VSCode MSX-BASIC project

The `--vscode` flag SHALL initialize the VSCode MSX-BASIC project scaffolding (a `.vscode` folder containing `launch.json`, `tasks.json`, and `debug.tcl`) in the current working directory. When `.vscode` already exists, the CLI SHALL report the project as already initialized and SHALL NOT overwrite or alter pre-existing files. The create-or-report contract is defined by the `vscode-scaffolding` capability.

#### Scenario: Initialize a new VSCode project
- **WHEN** the command `msxbas2rom --vscode` is executed in a directory that does not yet contain a `.vscode` folder
- **THEN** a `.vscode` folder with `launch.json`, `tasks.json`, and `debug.tcl` is created in that directory
- **AND** the CLI reports that the project was initialized successfully
- **AND** process exits with status code 0

#### Scenario: Report already-initialized VSCode project
- **WHEN** the command `msxbas2rom --vscode` is executed in a directory that already contains a `.vscode` folder
- **THEN** the CLI reports that the project is already initialized
- **AND** no pre-existing file inside `.vscode` is modified
- **AND** process exits with status code 1

### Requirement: CLI exports debugger symbol files

The CLI SHALL export debugger symbol files for the compiled ROM. Flags `-s`, `--noi`, and `--noice` SHALL select NoICE `.noi`, `--omds` OpenMSX `.omds`, `--cdb` `.cdb`, `--symbol` `.symbol`, and `--elf` ELF `.elf`. On success the CLI SHALL write the symbol file under the output path and state in the success summary that it was created.

#### Scenario: Generate NoICE symbols with the short flag
- **WHEN** the command `msxbas2rom -s program.bas` compiles successfully
- **THEN** a `.noi` symbol file is created next to the output ROM
- **AND** the success summary states that a symbols file was created

#### Scenario: Select a symbol format via its long flag
- **WHEN** the command `msxbas2rom --cdb program.bas` compiles successfully
- **THEN** a symbol file with the selected format extension (`.cdb`) is created next to the output ROM

### Requirement: CLI writes MSX-BASIC line numbers into the binary

The `-l`/`--lin` flag SHALL write the MSX-BASIC line numbers into the generated binary. When enabled, the code emitted for each BASIC source line SHALL store the current line number into the `CURLIN` system variable at runtime, so the executing BASIC line is always available to runtime error reporting and debuggers.

#### Scenario: Write line numbers into the binary
- **WHEN** the command `msxbas2rom -l program.bas` compiles successfully
- **THEN** the generated code updates `CURLIN` with the current MSX-BASIC line number as it executes

### Requirement: CLI supports configurable input and output paths

The CLI SHALL support the `-i`/`--inputPath` and `-o`/`--outputPath` options to control where source file resources resolve and where generated artifacts are written. Both default to the directory of the input file. The output ROM and any symbol files SHALL be written under the output path, and file resources referenced by the BASIC source (e.g. via `BLOAD`/`LOAD`) SHALL resolve relative to the input path.

#### Scenario: Write output artifacts under the output path
- **WHEN** the command `msxbas2rom -o build program.bas` is executed and compiles successfully
- **THEN** the output ROM is written to `build/program.rom`

#### Scenario: Default output is next to the input file
- **WHEN** the command `msxbas2rom path/program.bas` is executed and compiles successfully
- **THEN** the output ROM is written to `path/program.rom`

#### Scenario: Relative resources resolve against the input path
- **WHEN** the compiled source loads a file by name (e.g. `BLOAD "SPRITES.BIN"`) and the CLI is invoked with `-i assets`
- **THEN** the referenced file is resolved as `assets/SPRITES.BIN`

### Requirement: CLI provides verbosity and informational flags

The CLI SHALL support the informational flags `-h`/`--help`, `-D`/`--doc`, `-H`/`--history`, and `-v`/`--version`, each of which SHALL print the corresponding text and exit without compiling. The CLI SHALL support the `-q`/`--quiet` flag to suppress non-error output (header and success summary) while still reporting errors, and the `-d`/`--debug` flag to include debug-level details in the output.

#### Scenario: Print documentation and exit
- **WHEN** the command `msxbas2rom -D` or `msxbas2rom --doc` is executed
- **THEN** the reference guide text is printed
- **AND** process exits with status code 0

#### Scenario: Print version and exit
- **WHEN** the command `msxbas2rom -v` or `msxbas2rom --version` is executed
- **THEN** the application version text is printed
- **AND** process exits with status code 0

#### Scenario: Quiet mode suppresses the success summary
- **WHEN** the command `msxbas2rom -q program.bas` is executed and compiles successfully
- **THEN** no header or success summary is printed
- **AND** process exits with status code 0

#### Scenario: Quiet mode still reports errors
- **WHEN** the command `msxbas2rom -q program.bas` is executed with a missing input file
- **THEN** the input-file error message is printed
- **AND** process exits with status code 1

#### Scenario: Debug mode includes debug details
- **WHEN** the command `msxbas2rom -d program.bas` is executed and compiles successfully
- **THEN** the output includes debug-level details in addition to the normal output
- **AND** process exits with status code 0
