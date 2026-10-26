## 1. Dependency gate

- [ ] 1.1 Verify `add-rsctoram-offset-size` is applied: extended `cmd_rsctoram` accepts `offset`/`size` via `PARM1`, checks the resource carry before copying, preserves `dest`, closes the resource on every path, and returns `HL = dest` / `0`. Do not start the remaining tasks until this holds
- [ ] 1.2 Verify the dependency's kernel-space gate passed: `header.bin` is 32768 bytes, `ARKOS_PLAYER` is `0x6481`, and `make test-kernel` passes

## 2. Compiler strategy

- [ ] 2.1 Rewrite `src/application/compiler/functions/strategies/basic/resource/compiler_resource_function_strategy.cpp` to evaluate and cast `n`, store it as `resource`, then store `dest = HEAP()`, `pletter = 0`, `offset = 0`, and `size = FRE()` in the shared `PARM1` layout, and call `addKernelCall(DISP_cmd_rsctoram)`. Verify the generated code for `A = RESOURCE(0)` contains a call to `cmd_rsctoram` and no `call` to `usr0`
- [ ] 2.2 Reuse the `PARM1` offset definitions from `CompilerCmdRscToRamHandler` (factor a shared constant/header if needed) so both call sites cannot drift. Verify both compile against the same offsets
- [ ] 2.3 Add or update a unit test under `tests/unit/` asserting that the `RESOURCE` function strategy:
  - stores the evaluated argument in the resource slot;
  - stores `dest` from `HEAPSTR`, `pletter = 0`, `offset = 0`, and `size` from `HEAPSIZ`;
  - targets `DISP_cmd_rsctoram` and not `DISP_usr0`;
  - preserves the numeric result type.
  Verify the test passes
- [ ] 2.4 Rebuild the compiler (`make release`) and verify the generated setup sequence and code-size growth

## 3. Integration and runtime validation

- [ ] 3.1 Add `vram.probes` to the `GRAPH/test38a` entry in `tests/integration/regression/manifest.json` at `0x0000` using the first bytes of `tests/integration/GRAPH/NYANCAT.SC3` (`FE 00 00 FF 05 00 00 44 ...`). Probes are evaluated even for volatile programs, so this validates `COPY RESOURCE(0) TO (0,0)` without regenerating the baseline. Run `make -C tests/integration regression PROGRAM=test38a`
- [ ] 3.2 Create a deterministic regression program (for example `tests/integration/MISC/rscfunc.bas`) that prints or probes:
  - `PEEK(RESOURCE(n))`/following bytes for a resource that fits;
  - `RESOURCE(<invalid>)` returning `0`;
  - truncation for a resource larger than `FRE()`, if a suitable large fixture is added; otherwise record this case as not covered by the regression suite.
  Add the program to `tests/integration/regression/manifest.json` with `vram.text.expected` and/or `ram_probes`, then generate its baseline once with `make -C tests/integration regression-baseline PROGRAM=<name>`
- [ ] 3.3 Run `make -C tests/integration regression PROGRAM=rsc1` and verify the existing `MISC/rsc1` baseline is unchanged (shared `cmd_rsctoram` path)
- [ ] 3.4 Run `make test-emulator` and verify the fixed-page/dispatcher checks still pass after `RESOURCE` moves from `usr0` to `cmd_rsctoram`
- [ ] 3.5 Run `make test-integration` and verify every integration `.bas` still compiles to a ROM for ASCII8, ASCII16, ASCII16-X, and Konami

## 4. Documentation

- [ ] 4.1 Update the project wiki `Extended Functions` page (external) to describe the new `RESOURCE(n)` behavior, the single-slot loader semantics, the truncation behavior, and the `FRE() - 256` temporary-string caveat
- [ ] 4.2 Update `doc/misc/z80-binary-code.md` (section 10.2 "RESOURCE()/RESOURCESIZE()" and the syntax summary table) to document that `RESOURCE(n)` now loads `min(RESOURCESIZE(n), FRE())` bytes to `HEAP()`, returns the `HEAP()` address, returns `0` only when the resource does not exist, does not advance `HEAPSTR`, and can overlap the top 256-byte string reserve
