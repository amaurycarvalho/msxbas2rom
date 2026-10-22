## 1. Kernel — cmd_rsctoram

- [ ] 1.1 Rewrite `cmd_rsctoram` in `src/infrastructure/kernel/asm/src/header/31_cmd.asm` to read `resource`, `dest`, `pletter`, `offset`, and `size` from the `PARM1` layout, compute `effective = min(size, resource_size - offset)` (with `size = 0` meaning full remaining), copy, and return `HL = dest` or `HL = 0`. Verify with `make -C src/infrastructure/kernel/asm` that the kernel assembles without size warnings
- [ ] 1.2 Verify the pletter path (`pletter = 1`) still decompresses the whole resource and ignores `offset`/`size`. Verify by disassembling the routine or a targeted runtime probe

## 2. Compiler — CMD RSCTORAM handler

- [ ] 2.1 Update `src/application/compiler/statements/strategies/io/cmd/handlers/io/rsctoram/compiler_cmd_rsctoram_handler.cpp` to accept 2-5 arguments, define the `PARM1` offsets, and store `resource`, `dest`, `pletter`, `offset`, and `size` (defaults `pletter=0`, `offset=0`, `size=0`). Verify the generated code for the 2-, 3-, and 5-argument forms stores the expected `PARM1` values
- [ ] 2.2 Reject more than five arguments with a `CMD RSCTORAM syntax error`. Verify compilation fails for a 6-argument call
- [ ] 2.3 Rebuild the compiler (`make release`) and verify the build succeeds

## 3. Tests

- [ ] 3.1 Add integration `.bas` coverage for `CMD RSCTORAM` with offset/size (exact range, size larger than remaining, offset at end) and verify the copied RAM bytes in openMSX
- [ ] 3.2 Add integration coverage that a missing resource leaves RAM untouched and produces the failure return. Verify from the openMSX RAM dump
- [ ] 3.3 Run `make test-integration` and verify all `.bas` files still compile without errors and existing 2-/3-argument `CMD RSCTORAM` tests are unaffected

## 4. Documentation

- [ ] 4.1 Update the project wiki `Extended Commands` page (external) to document `CMD RSCTORAM <resource>, <dest>[, <pletter>[, <offset>[, <size>]]]` and the defaults
- [ ] 4.2 Update `doc/misc/z80-binary-code.md` (section 10.1 `CMD RSCTORAM` and the syntax summary table) to document the new optional `offset` and `size` parameters, their defaults (`0` and full remaining resource), the `size` clamp, and the pletter interaction
