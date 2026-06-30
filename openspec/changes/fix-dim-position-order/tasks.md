## 1. Parser: compute array dimension parameters in DimStatementStrategy

- [ ] 1.1 Add `x_factor` computation in `parseStatement`: derive from `lexeme->subtype` (string=256, numeric=2, single/double=3)
- [ ] 1.2 Add `x_size` computation from the 1st DIM parameter literal (`stoi(value) + 1`), with zero-size rejection
- [ ] 1.3 Add `y_size` computation from the 2nd DIM parameter (if present), defaulting to 1 for 1D arrays
- [ ] 1.4 Compute `y_factor = x_factor * x_size` and `array_size = y_factor * y_size`
- [ ] 1.5 Keep existing max-2-dimensions check and non-constant-size rejection (already present, just relocate)
- [ ] 1.6 Verify all error messages match the compiler's existing messages for the same conditions

## 2. Compiler: reduce DimStatementStrategy to validation

- [ ] 2.1 Remove `x_factor`, `x_size`, `y_size`, `y_factor`, `array_size` computation from `cmd_dim` — these are now set by the parser
- [ ] 2.2 Keep validation that `array_size` is non-zero (reports "Array declaration is missing" if not)
- [ ] 2.3 Keep identifier type check, dimension count check, and parameter existence checks
- [ ] 2.4 Move `isArray = true` setting — it was redundant (parser already sets it) but keep for defense

## 3. Tests: parser unit tests

- [ ] 3.1 Add test: `DIM A%(5)` sets `x_factor=2`, `x_size=6`, `array_size=12`
- [ ] 3.2 Add test: `DIM B!(3,4)` sets `x_factor=3`, `x_size=4`, `y_size=5`, `y_factor=12`, `array_size=60`
- [ ] 3.3 Add test: `DIM C$(2)` sets `x_factor=256`, `x_size=3`, `array_size=768`
- [ ] 3.4 Add test: `DIM D%(2,3,4)` rejects with error for >2 dimensions
- [ ] 3.5 Add test: `DIM E%(N)` rejects with error for non-constant size

## 4. Tests: compiler and integration tests

- [ ] 4.1 Add compiler test: program with array reference before DIM line compiles without errors
- [ ] 4.2 Add integration test (BAS source): `10 A%(1)=5:PRINT A%(1) : 20 DIM A%(2)` — verify the ROM runs correctly
- [ ] 4.3 Add integration test: the original bug scenario (`10 GOSUB 100:20 PRINT A%(1):30 END:100 DIM A%(2):110 A%(1)=5:120 RETURN`)

## 5. Verify

- [ ] 5.1 Run `make test-unit` — all existing tests pass
- [ ] 5.2 Run `make test-integration` — all existing integration tests pass
- [ ] 5.3 Run `make release` — clean build succeeds
