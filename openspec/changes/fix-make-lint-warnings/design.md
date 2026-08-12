## Context

`make lint` runs `g++ -fsyntax-only -Wall -Wextra -Werror -pedantic -fanalyzer -Wno-unused-parameter` over the whole source tree in a single invocation. It currently fails with three distinct diagnostics:

1. `-Wmissing-field-initializers` at `logger.cpp:23` — `logs.push_back({severity, message})` fills only 2 of `LogEntry`'s 6 fields.
2. `-Woverflow` at `compiler_set_statement_strategy.cpp:237,278` — `addBasicChar(0xC5)`/`addBasicChar(0xC0)` narrow `int` 197/192 into a signed `char`.
3. `-Wtype-limits` at `compiler_code_optimizer.cpp:247` — `address >= def_wrapper_routines_map_table` is always true because the macro is `0x0000` and `address` is `unsigned int`.

The codebase builds with `-std=c++11` (Makefile `CFLAGS`), which matters for the `LogEntry` fix (see Decisions).

## Goals / Non-Goals

**Goals:**
- Make `make lint` pass cleanly with zero diagnostics.
- Eliminate the latent indeterminate-field bug in `Logger::dummy`.
- Preserve byte-identical generated ROM output (no behavioral change).

**Non-Goals:**
- Parallelizing or otherwise restructuring the `lint` Makefile target (it is slow due to `-fanalyzer`; this is tracked separately, not required for this fix).
- Adding a logging capability spec.
- Touching unrelated `-fanalyzer`-only or future diagnostics.

## Decisions

**Decision 1 — `LogEntry`: explicit constructors instead of default member initializers or a 6-field braced init.**
- Chosen: add a default constructor and a `LogEntry(LogLevel, const string&)` constructor that initialize all six fields.
- Rejected A: default member initializers (`int line = 0;` etc.). Under `-std=c++11` a class with NSDMIs is not an aggregate, so `logs.push_back({severity, message})` would stop compiling. This is a C++14 feature.
- Rejected B: minimal `push_back({severity, message, "", 0, 0, 0})`. Fixes the warning but leaves `Logger::dummy` (member `LogEntry dummy;`) default-initialized with indeterminate `severity`/`line`/`column`/`code`, which `add()` returns for empty messages.
- The default constructor also fixes `dummy` for free.

**Decision 2 — `addBasicChar`: change parameter `char` → `unsigned char`.**
- The method's contract is "emit a raw byte", and it already forwards to `cpu.addLdiHL(unsigned char n)`. `unsigned char` is the correct type.
- Rejected: casting at the two call sites (`static_cast<char>(0xC5)`). Narrower fix, but leaves the API type wrong and any future `0x80`–`0xFF` caller would reintroduce the warning.
- All 20 existing callers pass `char` literals (`','`, `'('`, `'"'`), a `char` variable (`s[i]`), or int hex literals (`0x0F`, `0x1C`, `0xC0`, `0xC5`); all convert cleanly to `unsigned char` under `-Wall -Wextra` (no `-Wsign-conversion` enabled).

**Decision 3 — `getKernelCallAddr`: remove the redundant lower-bound check.**
- `def_wrapper_routines_map_table` is `0x0000`; the outer `if (address < 0x4000)` already bounds the upper end, so `address >= 0` is a tautology. Removing it preserves the `wrapper-dispatch-table` spec's table-range semantics (reads `bin_header_bin[address]` for `0x0000 ≤ address < DISP_ENTRIES*2`).
- Rejected: casting/comparing through a signed intermediate to silence the warning. Obfuscates intent for no benefit.

## Risks / Trade-offs

- [Risk] The constructor added to `LogEntry` changes it from an aggregate; some unseen code may rely on aggregate initialization elsewhere. → Mitigation: grep confirms the only braced-init of `LogEntry` is `logger.cpp:23`; all other uses are member reads or `push_back(entry)`.
- [Risk] `addBasicChar(unsigned char)` could silently change output if a caller passed a negative `char` expecting sign extension. → Mitigation: byte emission is unsigned by design (`addLdiHL(unsigned char)`); no caller relies on negative values.
- [Risk] Removing the lower-bound guard could change behavior if `def_wrapper_routines_map_table` were ever changed from `0x0000`. → Mitigation: the macro is `#define def_wrapper_routines_map_table 0x0000` and the `wrapper-dispatch-table` spec pins it to `0x0000`; the upper-bound guard (`+ DISP_ENTRIES * 2`) is retained.
