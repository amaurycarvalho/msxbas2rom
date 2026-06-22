## 1. Baseline and Preparation

- [x] 1.1 Build current kernel and capture `header.bin` and `header.symbols.asm` as pre-restructure baseline
- [x] 1.2 Capture list of all 279 `cpu.addCall(def_xxx)` call sites for migration tracking (116 files)

## 2. ASM-Side: Wrapper Table Restructure

- [x] 2.1 Count the 126 `jp` entries in `20_runtime.asm` to verify exact count
- [x] 2.2 Replace 126-entry `jp` table with `dw` word-pointer table in `20_runtime.asm` preserving exact order
- [x] 2.3 Add dispatcher routine at `wrapper_routines_map_start` — loads table offset from HL, fetches word pointer, jumps via `jp (hl)`
- [x] 2.4 Remove 5-byte NOP padding between `jp 0x8010` and the wrapper table (dispatcher fills this space)
- [x] 2.5 Build kernel and verify: `wrapper_routines_map_start` still at 0x4102, `header.bin` is 16384 bytes, dispatcher + table < 378 bytes
- [x] 2.6 Verify `header.symbols.asm` — all wrapper routine symbols still resolve correctly

## 3. C-Side: Headers and Helpers

- [x] 3.1 Update `compiler_hooks.h`: replace `+3` address chain with `DISP_` prefix 0-based index constants (e.g., `#define DISP_cmd_clrkey 1`)
- [x] 3.2 Keep `def_wrapper_routines_map_start` at 0x4102; define `def_wrapper_routines_map_table` for dw table base address
- [x] 3.3 Add `addKernelDispatch(uint8_t index)` method to `CompilerCodeOptimizer` — emits `ld hl, table_base + index*2; call 0x4102`
- [x] 3.4 Update `compiler_code_optimizer.getKernelCallAddr()` to resolve through `dw` table entries instead of checking for `0xC3` (jp) opcode

## 4. C-Side: Call Site Migration — Integer/Float Ops

- [x] 4.1 Update `compiler_expression_evaluator.cpp` — replace `optimizer.addKernelCall(def_intCompare*)` and `cpu.addCall(def_floatNEG)` etc. with dispatch
- [x] 4.2 Update `compiler_for_statement_strategy.cpp` — replace `optimizer.addKernelCall(def_intCompareGT)` with dispatch
- [x] 4.3 Update integer shift operations (`def_intSHR`, `def_intSHL`) in expression evaluator

## 5. C-Side: Call Site Migration — Command Handlers

- [x] 5.1 Update all `src/application/compiler/statements/strategies/io/cmd/handlers/graphics/*/` files
- [x] 5.2 Update `src/application/compiler/statements/strategies/io/cmd/handlers/io/clrkey/`
- [x] 5.3 Update `src/application/compiler/statements/strategies/io/cmd/handlers/control/` files
- [x] 5.4 Update remaining command handler files

## 6. C-Side: Call Site Migration — Player and Sound

- [x] 6.1 Update player-related files
- [x] 6.2 Update `def_player_initialize` and `def_player_unhook` call sites

## 7. C-Side: Call Site Migration — File I/O

- [x] 7.1 Update file handling files
- [x] 7.2 Update `def_cmd_preflight_disk` call sites

## 8. C-Side: Call Site Migration — XBASIC and Functions

- [x] 8.1 Update XBASIC routine call sites
- [x] 8.2 Update input/read functions
- [x] 8.3 Update `def_castParamFloatInt` call sites

## 9. C-Side: Call Site Migration — Misc and Remaining

- [x] 9.1 Update sprite/tile operations
- [x] 9.2 Update USR functions
- [x] 9.3 Update MegaROM calls
- [x] 9.4 Update date/time
- [x] 9.5 Update remaining
- [x] 9.6 Update screen/font

## 10. C-Side: Cleanup and Verification

- [x] 10.1 Verify no remaining `cpu.addCall(def_` calls exist in the codebase (grep for pattern)
- [x] 10.2 Verify no remaining direct `def_` address usage that assumes absolute addresses
- [x] 10.3 Remove old `def_*` constants from `compiler_hooks.h` that are no longer referenced

## 11. Build and Test

- [x] 11.1 Build kernel and verify: filler space increased by at least 115 bytes (120 bytes), `wrapper_routines_map_start` at 0x4102
- [x] 11.2 Full release build (`make`) — compiles without errors
- [x] 11.3 Run `make test-unit` — 167/167 unit tests pass
- [x] 11.4 Run `make test-integration` — all `.bas` files compile without errors
- [x] 11.5 Run `make test-kernel` — 11/11 kernel tests pass
- [x] 11.6 Compare before/after ROM binaries — filler space increased by 120 bytes
- [x] 11.7 Verify incremental build works
