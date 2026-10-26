## ADDED Requirements

### Requirement: RESOURCE(n) compiler strategy delegates to CMD RSCTORAM

The C++ `RESOURCE(n)` function strategy SHALL emit code that stores the `CMD RSCTORAM` parameters and calls `DISP_cmd_rsctoram`. It SHALL evaluate and cast `n`, store it as the resource number, then use `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` (`HEAPSIZ`). It SHALL NOT emit a call to `DISP_usr0`. The `PARM1` offsets SHALL be shared with `CompilerCmdRscToRamHandler`.

#### Scenario: RESOURCE(n) does not call usr0

- **WHEN** the compiler processes an expression containing `RESOURCE(<expr>)`
- **THEN** the generated code SHALL contain a `call` to the `cmd_rsctoram` dispatch target
- **AND** the generated code SHALL NOT contain a `call` to `usr0`

#### Scenario: RESOURCE(n) parameter setup

- **WHEN** the compiler processes `RESOURCE(<expr>)`
- **THEN** the generated code SHALL store `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` in the `PARM1` work area before the `cmd_rsctoram` call
- **AND** the argument `<expr>` SHALL be evaluated and cast to numeric before being stored as the resource number
- **AND** storing the resource number SHALL happen before `HL` is overwritten by the `HEAP()`/`FRE()` setup

#### Scenario: RESOURCE(n) uses the shared PARM1 layout

- **WHEN** the compiler emits `RESOURCE(<expr>)` and `CMD RSCTORAM`
- **THEN** both SHALL use the same resource, destination, pletter, offset, and size offsets

#### Scenario: RESOURCE(n) result type

- **WHEN** the compiler processes `RESOURCE(<expr>)`
- **THEN** the function result type SHALL be numeric
