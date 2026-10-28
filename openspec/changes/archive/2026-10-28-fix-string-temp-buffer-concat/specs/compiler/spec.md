# Spec Delta

## ADDED Requirements

### Requirement: String-function results use distinct temporary storage

When a string expression evaluates more than one string-function result, each result SHALL remain valid until the enclosing expression finishes. Concatenating two `STRING$` results SHALL preserve both operands in evaluation order, even when both calls use the same character and lengths differ.

#### Scenario: Two STRING$ results are concatenated

- **WHEN** `PRINT STRING$(5,65)+STRING$(6,66)` is compiled and executed
- **THEN** the output contains five `A` characters followed by six `B` characters
- **AND** it does not contain the second result duplicated in place of the first

#### Scenario: STRING$ results from variables are concatenated

- **WHEN** `X%=5`, `Y%=1`, and `PRINT STRING$(X%,65)+STRING$(6-Y%,66)` is executed
- **THEN** the output contains five `A` characters followed by five `B` characters

#### Scenario: STRING$ results survive nested concatenation

- **WHEN** a concatenation combines string-function results with literals or other string functions
- **THEN** every operand contributes its own characters in evaluation order
- **AND** no earlier result is overwritten by a later function call

### Requirement: Startup selects a deterministic 40-column text mode

The generated ROM startup SHALL explicitly select `SCREEN 0` and `WIDTH 40`, regardless of the machine's boot default. A `PRINT` statement without a trailing `;` or `,` SHALL leave the cursor at column 0 of the following row.

#### Scenario: Consecutive PRINT lines start at the left margin

- **WHEN** a program executes `PRINT "HELLO"` followed by `PRINT "WORLD"`
- **THEN** `HELLO` and `WORLD` appear on consecutive rows
- **AND** both strings begin at the screen's left margin
- **AND** the visible text does not drift or wrap because of a width mismatch

#### Scenario: Startup screen layout matches the text width

- **WHEN** a program captures the terminal text state without issuing `SCREEN` or `WIDTH`
- **THEN** the captured name table and text width correspond to the standard 40-column `SCREEN 0` layout
- **AND** the ROM does not select a 32-column VDP layout while writing 40-column text

#### Scenario: The same 40-column layout is used on every machine class

- **WHEN** the same program runs on MSX1, MSX2, MSX2+, and turbo R machines
- **THEN** it uses the same 40-column `SCREEN 0` text layout
- **AND** the captured name table starts at `0x0000`
- **AND** the machine's non-Japanese/Japanese boot default does not change the output columns

### Requirement: STRING$ enforces the shared function arity bound

`STRING$` SHALL reject five or more arguments with a syntax error before evaluating or storing argument subtypes, consistent with the shared function arity policy. Its valid two-argument form SHALL continue to compile.

#### Scenario: Valid STRING$ compiles

- **WHEN** `A$=STRING$(3,65)` is compiled
- **THEN** compilation succeeds

#### Scenario: STRING$ with five arguments is rejected

- **WHEN** `A$=STRING$(3,65,1,2,3)` is compiled
- **THEN** compilation fails with a syntax error
- **AND** the compiler does not evaluate or store the excess arguments
