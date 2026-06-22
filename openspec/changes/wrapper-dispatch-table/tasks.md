## 1. ASM-Side: Remove Dispatcher, Relocate Table

- [ ] 1.1 Remove the dispatcher routine at `wrapper_routines_map_start` (lines 167-174 of `20_runtime.asm`): the 6-byte `ld a,(hl); inc hl; ld h,(hl); ld l,a; jp (hl)` block is no longer needed since `addKernelCall` resolves targets at compile time
- [ ] 1.2 Relocate `wrapper_routines_map_table` label to 0x4102 (where the dispatcher was), aligning with the existing NOP padding after `jp 0x8010`
- [ ] 1.3 Rebuild kernel and verify: `wrapper_routines_map_table` at 0x4102, `header.bin` still 16384 bytes, filler space ≥ 120 bytes
- [ ] 1.4 Verify `header.symbols.asm` — all wrapper routine symbols still resolve correctly

## 2. C-Side: compiler_hooks.h — Update Constants

- [ ] 2.1 Update `def_wrapper_routines_map_table` from `(def_wrapper_routines_map_start + 6)` to `0x4102` (reflects new table location at the dispatcher's old address)
- [ ] 2.2 Remove `def_wrapper_routines_map_start` (0x4102 constant) — the dispatcher routine is gone, no code should reference its address
- [ ] 2.3 Verify all `DISP_*` constants are 0-based sequential indices (0–125) matching the wrapper table order — already correct from `wrapper-table-restructure`

## 3. C-Side: compiler_code_optimizer — Unify via addKernelCall (Design D2)

- [ ] 3.1 Update `addKernelCall()` to accept a dispatch index directly (by computing `def_wrapper_routines_map_table + index * 2` internally) instead of requiring callers to pass the calculated address
- [ ] 3.2 Update `getKernelCallAddr()` to handle both old `0xC3` JP entries (for BIOS/ROM addresses outside the wrapper table) and new `dw` entries (for wrapper dispatch). For wrapper table entries, read the 2-byte word pointer from `bin_header_bin`; for legacy entries, keep the `0xC3` check
- [ ] 3.3 Remove `addKernelDispatch(unsigned char index)` method from `CompilerCodeOptimizer` (header and implementation) — replaced by unified `addKernelCall`
- [ ] 3.4 Remove `addKernelDispatch` declaration from `compiler_code_optimizer.h`

## 4. C-Side: Call Site Migration — Replace addKernelDispatch with addKernelCall

- [ ] 4.1 Replace all `context->codeOptimizer->addKernelDispatch(DISP_xxx)` with `optimizer.addKernelCall(DISP_xxx)` across all call sites (~100+ files)
- [ ] 4.2 Remove direct `def_wrapper_routines_map_table + DISP_xxx * 2` computation from `optimizer.addKernelCall()` callers — `addKernelCall` now handles the offset internally
- [ ] 4.3 Verify no remaining `addKernelDispatch` calls exist in the codebase

## 5. C-Side: Conditional Dispatch — addKernelCallNZ

- [ ] 5.1 Add `addKernelCallNZ(uint8_t index)` method to `CompilerCodeOptimizer` — resolves the target via `getKernelCallAddr(def_wrapper_routines_map_table + index * 2)` and emits `cpu.addCallNZ(resolved_target)`
- [ ] 5.2 Update `compiler_variable_emitter.cpp` — replace `cpu.addLdHL(def_wrapper_routines_map_table + DISP_cmd_fmaxfiles * 2); cpu.addCallNZ(def_wrapper_routines_map_start)` with `optimizer.addKernelCallNZ(DISP_cmd_fmaxfiles)`
- [ ] 5.3 Update `compiler_maxfiles_statement_strategy.cpp` — same replacement using `addKernelCallNZ(DISP_cmd_fmaxfiles)`
- [ ] 5.4 Verify no remaining references to `def_wrapper_routines_map_start` exist in the codebase

## 6. C-Side: compiler.cpp — MR_* Direct Resolution

- [ ] 6.1 Verify `mr_call_target`, `mr_jump_target`, `mr_get_data_target` are computed correctly from `bin_header_bin` using `def_wrapper_routines_map_table + DISP_MR_xxx * 2` (already implemented in `wrapper-table-restructure` — no `addMRKernelCall` helper needed because the byte-patching code writes raw address values, not opcodes)
- [ ] 6.2 Test MegaROM compilation (`bin/Debug/msxbas2rom -a tests/integration/MEGAROM/test51.bas`) — must compile without errors

## 7. Build System: Kernel Size Check

- [ ] 7.1 Add a post-build step in `src/infrastructure/kernel/asm/Makefile` that checks `header.bin` size ≤ 16384 bytes
- [ ] 7.2 On breach, emit alert: `WARNING: Kernel size <X> bytes exceeds 0x4000 limit by <Y> bytes`
- [ ] 7.3 Add an explicit doctest unit test in `tests/unit/` that verifies `bin_header_bin` size ≤ 0x4000 with a descriptive assertion message
- [ ] 7.4 Verify the kernel size check runs as part of `make test-kernel`

## 8. Cleanup and Verification

- [ ] 8.1 Verify no `addKernelDispatch` declarations or calls remain
- [ ] 8.2 Verify no `def_wrapper_routines_map_start` references remain
- [ ] 8.3 Verify `context->codeOptimizer->addKernelDispatch` pattern is fully replaced
- [ ] 8.4 Build kernel and verify: `header.bin` ≤ 16384 bytes, `wrapper_routines_map_table` at 0x4102
- [ ] 8.5 Full build (`make`) — must compile without errors
- [ ] 8.6 Run `make test-unit` — all unit tests pass (including new kernel size doctest)
- [ ] 8.7 Run `make test-integration` — all `.bas` files compile without errors
- [ ] 8.8 Run `make test-kernel` — kernel binary integrity verified
