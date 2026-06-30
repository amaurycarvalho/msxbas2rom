## ADDED Requirements

### Requirement: DIM statement strategy computes array dimension parameters during parsing

The parser's `DimStatementStrategy` SHALL compute the array dimension parameters (`x_factor`, `x_size`, `y_size`, `y_factor`, `array_size`) on the shared lexeme during syntactic analysis, in addition to setting `isArray = true`. The `x_factor` SHALL be derived from the variable's subtype (2 for integer, 3 for float, 256 for string). The `x_size`/`y_size` SHALL be derived from the DIM size literal (`size + 1`). The parser SHALL reject arrays with more than 2 dimensions and non-constant DIM size parameters.

#### Scenario: Parser sets array parameters for integer array
- **WHEN** a `DIM A%(5)` statement is parsed
- **THEN** the A% lexeme has `isArray = true`, `x_factor = 2`, `x_size = 6`, `array_size = 12`

#### Scenario: Parser sets array parameters for single-precision array
- **WHEN** a `DIM B!(3,4)` statement is parsed
- **THEN** the B! lexeme has `isArray = true`, `x_factor = 3`, `x_size = 4`, `y_size = 5`, `y_factor = 12`, `array_size = 60`

#### Scenario: Parser sets array parameters for string array
- **WHEN** a `DIM C$(2)` statement is parsed
- **THEN** the C$ lexeme has `isArray = true`, `x_factor = 256`, `x_size = 3`, `array_size = 768`

#### Scenario: Parser rejects array with more than 2 dimensions
- **WHEN** a `DIM D%(2,3,4)` statement is parsed
- **THEN** the parser returns an error: "Arrays with more than 2 dimensions isn't supported"

#### Scenario: Parser rejects non-constant DIM size
- **WHEN** a `DIM E%(N)` statement is parsed (where N is a variable)
- **THEN** the parser returns an error: "Array 1st dimension index must be a integer constant"
