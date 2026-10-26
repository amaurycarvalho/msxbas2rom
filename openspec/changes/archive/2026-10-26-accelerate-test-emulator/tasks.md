# Tasks

## 1. Shared runtime-check helper

- [x] 1.1 Add `tests/dispatcher/emulator.py` with symbol parsing for `.noi` and pasmo output, 16-bit address normalization, pre-boot breakpoint formatting, headless SDL setup, throttle selection, and capture invocation; verify with `python3 -m py_compile tests/dispatcher/emulator.py`.
- [x] 1.2 Resolve symbols against existing files: `tests/integration/IO/test1.noi` for `END_STMT`/`LIN_40` and a temporary pasmo listing for `loop`; verify the parsed addresses are masked to 16 bits.
- [x] 1.3 Implement the throttle rule so checks without host input use acceleration and checks with input keep real-time throttling; verify the generated environment in a short Python check.

## 2. Skill acceleration and conditional breakpoints

- [x] 2.1 Add `MSX_DEBUG_ACCELERATE` to `debug-msx-memory.sh`; verify no-input runs disable throttling, input runs keep it enabled, and an explicit `MSX_DEBUG_THROTTLE` wins.
- [x] 2.2 Add the optional `LABEL=ADDRESS@CONDITION` / `ADDRESS@CONDITION` breakpoint syntax; verify a conditional breakpoint captures only when its condition is true.
- [x] 2.3 Document acceleration and conditional breakpoints in `SKILL.md`, `README.md`, and the launcher usage text; verify the documented examples match the accepted syntax.
- [x] 2.4 Extend the shared helper so breakpoints carry an optional condition and so `.noi` segments map to 8 KiB or 16 KiB mapper blocks.
- [x] 2.5 Verify the helper's mapper-block mapping with a temporary BASIC ROM on Plain, ASCII8, ASCII16, ASCII16-X, Konami, and Konami SCC; every mode must capture marker `0x5A`.

## 3. Convert BASIC overflow checks

- [x] 3.1 Update `tests/dispatcher/check_plain_overflow.py` to compile with `-s --noi`, break at its terminal loop line, and capture through the helper; verify the script prints `[PASS]` and completes in under 3 seconds.
- [x] 3.2 Update `tests/dispatcher/check_megarom_overflow.py` to export symbols and use block-aware breakpoints for each mapper; verify all five mapper variants pass.
- [x] 3.3 Update `tests/dispatcher/check_megarom_resources.py` to compile with `-s --noi`, use a block-aware breakpoint at the appended terminal loop line, and capture through the helper; verify the script prints `[PASS]`.

## 4. Convert page-safe module checks

- [x] 4.1 Add an explicit terminal loop line to every case in `check_page_safe_modules.py` and compile each BASIC case with `-s --noi`; verify `.noi` files are created for the temporary builds.
- [x] 4.2 Route `run_emulator` through the shared helper with block-aware case breakpoints and preserve existing fallback delays; verify `tests/dispatcher/check_page_safe_modules.py` prints `[PASS]` for all safe, fixed, resident, and DATA cases.
- [x] 4.3 Document the marker-then-loop convention beside the case tables so future checks place the breakpoint after all observable writes.

## 5. Convert assembler dispatcher prototype

- [x] 5.1 Emit pasmo symbols for `tests/dispatcher/prototype.asm`, resolve the `loop` symbol, and pass it to the helper; verify `check_dispatcher.py` prints `[PASS]` and all expected RAM bytes still match.

## 6. Integration verification and documentation

- [x] 6.1 Run each dispatcher check individually and confirm every result matches the pre-change pass/fail outcome.
- [x] 6.2 Verify the fallback path by capturing a temporary ROM with an unreachable conditional breakpoint; confirm RAM is still produced after the configured delay.
- [x] 6.3 Run `make test-emulator` and compare wall-clock time with the recorded baseline of roughly eleven minutes; confirm the suite passes and finishes in the expected one-to-two-minute range.
- [x] 6.4 Update the emulator test documentation in `doc/misc/architecture.md` to describe accelerated execution, symbol-derived terminal breakpoints, mapper-block conditions, and fallback capture.

## Workflow follow-up

- Archive the change after review and verification pass.
