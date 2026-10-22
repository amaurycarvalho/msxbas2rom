## ADDED Requirements

### Requirement: RESOURCE(n) compiler strategy delegates to CMD RSCTORAM

The C++ `RESOURCE(n)` function strategy SHALL emit code that stores the `CMD RSCTORAM` parameters and calls `DISP_cmd_rsctoram`. It SHALL use `resource = <evaluated n>`, `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` (`HEAPSIZ`). It SHALL NOT emit a call to `DISP_usr0`.

#### Scenario: RESOURCE(n) does not call usr0

- **WHEN** the compiler processes an expression containing `RESOURCE(<expr>)`
- **THEN** the generated code SHALL contain a `call` to the `cmd_rsctoram` dispatch target
- **AND** the generated code SHALL NOT contain a `call` to `usr0`

#### Scenario: RESOURCE(n) parameter setup

- **WHEN** the compiler processes `RESOURCE(<expr>)`
- **THEN** the generated code SHALL store `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` in the `PARM1` work area before the `cmd_rsctoram` call
- **AND** the argument `<expr>` SHALL be evaluated and cast to numeric before being stored as the resource number

#### Scenario: RESOURCE(n) result type

- **WHEN** the compiler processes `RESOURCE(<expr>)`
- **THEN** the function result type SHALL be numeric
