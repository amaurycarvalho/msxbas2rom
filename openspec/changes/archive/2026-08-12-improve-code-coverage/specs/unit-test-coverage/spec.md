## ADDED Requirements

### Requirement: Global line coverage must reach 85%

The project SHALL achieve line coverage of at least 85% across all source files compiled in the unit test suite, as verified by `gcovr` executed via `make coverage` in `tests/unit/`.

#### Scenario: Coverage report passes the threshold
- **WHEN** `make coverage` is executed in `tests/unit/`
- **THEN** gcovr SHALL report line coverage ≥ 85.0%
- **THEN** the command SHALL exit with code 0 (pass --fail-under-line 85)

### Requirement: Compiler SET statement has adequate test coverage

The file `compiler_set_statement_strategy.cpp` SHALL have line coverage ≥ 70% (currently 44.9%, 316/704 lines). Tests MUST cover at least: BGM, ADJUST, BEEP, DISPLAY, PAGE, TITLE, VIDEO, CLIP, PASSWORD, INTERVAL, SCREEN, and SEARCH sub-commands.

#### Scenario: SET BGM compiles correctly
- **WHEN** a .bas program containing `SET BGM,1` is compiled
- **THEN** the compiler SHALL generate valid Z80 opcodes
- **THEN** no error SHALL be reported

#### Scenario: SET VIDEO mode compiles correctly
- **WHEN** a .bas program containing `SET VIDEO,0` is compiled
- **THEN** the compiler SHALL select the correct video mode handler
- **THEN** valid opcodes SHALL be emitted

#### Scenario: SET SCREEN compiles without errors
- **WHEN** a .bas program containing `SET SCREEN,1` is compiled
- **THEN** the compiler SHALL process the screen mode change
- **THEN** valid opcodes SHALL be emitted

### Requirement: Compiler ON statement path coverage expanded

The file `compiler_on_statement_strategy.cpp` SHALL have line coverage ≥ 50% (currently 20.8%, 64/307 lines). Tests MUST cover ON ERROR, ON STOP, ON SPRITE, ON INTERVAL, and ON KEY variants.

#### Scenario: ON ERROR GOTO compiles correctly
- **WHEN** a .bas program containing `ON ERROR GOTO 1000` is compiled
- **THEN** the compiler SHALL generate error handler setup opcodes
- **THEN** the generated code SHALL route to the specified line on error

### Requirement: Compiler graphics statements have expanded coverage

The files `compiler_copy_statement_strategy.cpp`, `compiler_put_statement_strategy.cpp`, `compiler_screen_statement_strategy.cpp`, `compiler_circle_statement_strategy.cpp`, and `compiler_paint_statement_strategy.cpp` SHALL each have line coverage ≥ 65%.

#### Scenario: COPY statement with source-destination compiles
- **WHEN** a .bas program containing `COPY(0,0)-(31,23) TO (0,0)` is compiled
- **THEN** the compiler SHALL generate VRAM-to-VRAM copy opcodes
- **THEN** coordinates SHALL be correctly encoded

#### Scenario: PUT SPRITE statement compiles
- **WHEN** a .bas program containing `PUT SPRITE 0,(10,10),15,0` is compiled
- **THEN** the compiler SHALL generate sprite attribute write opcodes
- **THEN** the sprite pattern and color SHALL be correctly set

### Requirement: Expression evaluator edge cases covered

The file `compiler_expression_evaluator.cpp` SHALL have line coverage ≥ 80% (currently 66%, 464/703 lines). Tests MUST cover: type coercion between integer/float/string, operator precedence chains, function calls within expressions, and unary operators.

#### Scenario: Mixed-type expression coerces correctly
- **WHEN** an expression mixing integer and float operands (e.g., `A% + 3.14`) is compiled
- **THEN** the expression evaluator SHALL correctly promote types
- **THEN** valid opcodes for float addition SHALL be generated

### Requirement: Compiler variable emitter path coverage

The file `compiler_variable_emitter.cpp` SHALL have line coverage ≥ 65% (currently 43.8%, 88/201 lines). Tests MUST cover: scalar assignment, array element assignment, DEFUSR function assignment, and type-checking paths.

#### Scenario: Array element assignment works
- **WHEN** a .bas program containing `A(5) = 42` is compiled
- **THEN** the variable emitter SHALL calculate the correct array offset
- **THEN** the value SHALL be stored at the correct memory address

### Requirement: Resource readers have basic validation coverage

The file `resource_akm_reader.cpp` SHALL have line coverage ≥ 30% (currently 2.5%, 4/159 lines). The file `resource_mtf_map_reader.cpp` SHALL have line coverage ≥ 50% (currently 22.6%, 21/93 lines).

#### Scenario: AKM reader accepts valid minimal file
- **WHEN** a valid minimal AKM binary file is provided
- **THEN** the reader SHALL correctly parse the header and metadata
- **THEN** no error SHALL be returned

#### Scenario: MTF map reader parses tile data
- **WHEN** a valid MTF map file is provided
- **THEN** the reader SHALL extract tile indices correctly
- **THEN** the tile count SHALL match the header specification

### Requirement: All existing tests continue to pass

The system SHALL maintain backward compatibility with all existing test cases. No existing test SHALL be modified or removed.

#### Scenario: Existing test suite unaffected
- **WHEN** `make run` is executed in `tests/unit/` after adding new tests
- **THEN** all previously passing tests SHALL continue to pass
- **THEN** total test count SHALL increase (not decrease)
