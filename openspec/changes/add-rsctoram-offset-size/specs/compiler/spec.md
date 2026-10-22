## ADDED Requirements

### Requirement: CMD RSCTORAM handler accepts optional offset and size

The `CompilerCmdRscToRamHandler` SHALL accept between 2 and 5 arguments for `CMD RSCTORAM`: `<resource>, <dest>[, <pletter>[, <offset>[, <size>]]]`. It SHALL store all arguments in the `PARM1` work area for the kernel routine and SHALL resolve omitted `offset` (default `0`) and omitted `size` (default full resource).

#### Scenario: Two-argument form uses defaults

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>`
- **THEN** the emitted code SHALL store `pletter = 0`, `offset = 0`, and a `size` value that selects the full resource
- **AND** the emitted code SHALL call `cmd_rsctoram`

#### Scenario: Five-argument form passes offset and size

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>, <pletter>, <offset>, <size>`
- **THEN** the emitted code SHALL store the provided `pletter`, `offset`, and `size` values in the `PARM1` work area

#### Scenario: Too many arguments is a syntax error

- **WHEN** the compiler processes `CMD RSCTORAM` with more than five arguments
- **THEN** compilation SHALL fail with a `CMD RSCTORAM syntax error`

#### Scenario: Existing three-argument form is unchanged

- **WHEN** the compiler processes `CMD RSCTORAM <resource>, <dest>, <pletter>`
- **THEN** the emitted code SHALL use `offset = 0` and the full resource size
- **AND** the generated call SHALL be compatible with the previous behavior
