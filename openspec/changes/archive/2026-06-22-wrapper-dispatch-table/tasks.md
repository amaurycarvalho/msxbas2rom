## 1. ASM-Side: Remove Dispatcher, Relocate Table

- [x] 1.1 Remove the dispatcher routine at `wrapper_routines_map_start` (lines 167-174 of `20_runtime.asm`): the 6-byte `ld a,(hl); inc hl; ld h,(hl); ld l,a; jp (hl)` block is no longer needed since `addKernelCall` resolves targets at compile time
- [x] 1.2 Relocate `wrapper_routines_map_table` label to 0x4102 (where the dispatcher was), aligning with the existing NOP padding after `jp 0x8010`
- [x] 1.3 Rebuild kernel and verify: `wrapper_routines_map_table` at 0x4102, `header.bin` still 16384 bytes, filler space ≥ 120 bytes
- [x] 1.4 Verify `header.symbols.asm` — all wrapper routine symbols still resolve correctly (updated kernel test to check `wrapper_routines_map_table` instead of `_start`)

## 2. C-Side: compiler_hooks.h — Update Constants

- [x] 2.1 Update `def_wrapper_routines_map_table` from `(def_wrapper_routines_map_start + 6)` to `0x4102` (reflects new table location at the dispatcher's old address)
- [x] 2.2 Remove `def_wrapper_routines_map_start` (0x4102 constant) — the dispatcher routine is gone, no code should reference its address
- [x] 2.3 Verify all `DISP_*` constants are 0-based sequential indices (0–125) matching the wrapper table order — already correct from `wrapper-table-restructure`

## 3. C-Side: compiler_code_optimizer — Unify via addKernelCall (Design D2)

- [x] 3.1 Update `addKernelCall()` to accept a dispatch index directly (by computing `def_wrapper_routines_map_table + index * 2` internally) instead of requiring callers to pass the calculated address
- [x] 3.2 Update `getKernelCallAddr()` to handle both old `0xC3` JP entries (for BIOS/ROM addresses outside the wrapper table) and new `dw` entries (for wrapper dispatch). For wrapper table entries, read the 2-byte word pointer from `bin_header_bin`; for legacy entries, keep the `0xC3` check
- [x] 3.3 Remove `addKernelDispatch(unsigned char index)` method from `CompilerCodeOptimizer` (header and implementation) — replaced by unified `addKernelCall`
- [x] 3.4 Remove `addKernelDispatch` declaration from `compiler_code_optimizer.h`

## 4. C-Side: Call Site Migration — Replace addKernelDispatch with addKernelCall

- [x] 4.1 Replace all `context->codeOptimizer->addKernelDispatch(DISP_xxx)` with `context->codeOptimizer->addKernelCall(DISP_xxx)` across all call sites (~100+ files)
- [x] 4.2 Remove direct `def_wrapper_routines_map_table + DISP_xxx * 2` computation from `optimizer.addKernelCall()` callers — `addKernelCall` now handles the offset internally
- [x] 4.3 Verify no remaining `addKernelDispatch` calls exist in the codebase

## 5. C-Side: Conditional Dispatch — addKernelCallNZ

- [x] 5.1 Add `addKernelCallNZ(uint8_t index)` method to `CompilerCodeOptimizer` — resolves the target via `getKernelCallAddr(def_wrapper_routines_map_table + index * 2)` and emits `cpu.addCallNZ(resolved_target)`
- [x] 5.2 Update `compiler_variable_emitter.cpp` — replace `cpu.addLdHL(def_wrapper_routines_map_table + DISP_cmd_fmaxfiles * 2); cpu.addCallNZ(def_wrapper_routines_map_start)` with `context->codeOptimizer->addKernelCallNZ(DISP_cmd_fmaxfiles)`
- [x] 5.3 Update `compiler_maxfiles_statement_strategy.cpp` — same replacement using `addKernelCallNZ(DISP_cmd_fmaxfiles)`
- [x] 5.4 Verify no remaining references to `def_wrapper_routines_map_start` exist in the codebase

## 6. C-Side: compiler.cpp — MR_* Direct Resolution

- [x] 6.1 Verify `mr_call_target`, `mr_jump_target`, `mr_get_data_target` are computed correctly from `bin_header_bin` using `def_wrapper_routines_map_table + DISP_MR_xxx * 2` (already implemented in `wrapper-table-restructure` — no `addMRKernelCall` helper needed because the byte-patching code writes raw address values, not opcodes)
- [x] 6.2 Test MegaROM compilation (`bin/Debug/msxbas2rom -a tests/integration/MEGAROM/test51.bas`) — must compile without errors

## 7. Build System: Kernel Size Check

- [x] 7.1 Add a post-build step in `src/infrastructure/kernel/asm/Makefile` that checks `header.bin` size ≤ 16384 bytes
- [x] 7.2 On breach, emit alert: `WARNING: Kernel size <X> bytes exceeds 0x4000 limit by <Y> bytes`
- [x] 7.3 Add an explicit doctest unit test in `tests/unit/src/test_kernel.cpp` that verifies `header.bin` size ≤ 0x4000 with a descriptive assertion message
- [x] 7.4 Verify the kernel size check runs as part of `make test-kernel` (Makefile check) and `make test-unit` (doctest)

## 8. Cleanup and Verification

- [x] 8.1 Verify no `addKernelDispatch` declarations or calls remain
- [x] 8.2 Verify no `def_wrapper_routines_map_start` references remain
- [x] 8.3 Verify `context->codeOptimizer->addKernelDispatch` pattern is fully replaced
- [x] 8.4 Build kernel and verify: `header.bin` ≤ 16384 bytes, `wrapper_routines_map_table` at 0x4102
- [x] 8.5 Full build (`make`) — must compile without errors
- [x] 8.6 Run `make test-unit` — all unit tests pass (168/168 including new kernel size doctest)
- [x] 8.7 Run `make test-integration` — all `.bas` files compile without errors (only pre-existing mtf5.bas syntax error)
- [x] 8.8 Run `make test-kernel` — kernel binary integrity verified (11/11 passed)
