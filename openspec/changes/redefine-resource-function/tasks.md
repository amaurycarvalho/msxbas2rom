## 1. Dependency gate

- [ ] 1.1 Verify `add-rsctoram-offset-size` is applied (extended `cmd_rsctoram` accepts `offset`/`size` via `PARM1` and returns `HL = dest` / `0`). Do not start the remaining tasks until this holds

## 2. Compiler strategy

- [ ] 2.1 Rewrite `src/application/compiler/functions/strategies/basic/resource/compiler_resource_function_strategy.cpp` to store `resource = n`, `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` in the shared `PARM1` layout and call `addKernelCall(DISP_cmd_rsctoram)`. Verify the generated code for `A = RESOURCE(0)` contains a call to `cmd_rsctoram` and no `call` to `usr0`
- [ ] 2.2 Reuse the `PARM1` offset definitions from `CompilerCmdRscToRamHandler` (factor a shared constant/header if needed) so both call sites cannot drift. Verify both compile against the same offsets
- [ ] 2.3 Add or update a unit test under `tests/unit/` asserting that the `RESOURCE` function strategy targets `DISP_cmd_rsctoram`, stores the expected `PARM1` values, and preserves the numeric result type. Verify the test passes

## 3. Integration and runtime validation

- [ ] 3.1 Recompile `tests/integration/GRAPH/test38a.bas` and run the ROM in openMSX; verify `COPY RESOURCE(0) TO (0,0)` still copies the screen correctly from the RAM copy
- [ ] 3.2 Create a runtime probe compiled with `-c` that (a) confirms `PEEK(RESOURCE(n))` matches the resource bytes for a resource that fits, (b) confirms `RESOURCE(<invalid>)` returns `0`, and (c) confirms a resource larger than `FRE()` is truncated to `FRE()` bytes and still returns the `HEAP()` address. Verify each case from the openMSX screenshot/RAM dump
- [ ] 3.3 Run `make test-integration` and verify every integration `.bas` still compiles to a ROM without errors

## 4. Documentation

- [ ] 4.1 Update the project wiki `Extended Functions` page (external) to describe the new `RESOURCE(n)` behavior, the truncation semantics, and the raw-copy/`FRE()` caveats
- [ ] 4.2 Update `doc/misc/z80-binary-code.md` (section 10.2 "RESOURCE()/RESOURCESIZE()" and the syntax summary table) to document that `RESOURCE(n)` now loads `min(RESOURCESIZE(n), FRE())` bytes to `HEAP()` and returns `0` only when the resource does not exist
