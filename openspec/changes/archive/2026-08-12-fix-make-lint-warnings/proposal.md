## Why

`make lint` (`g++ -fsyntax-only -Wall -Wextra -Werror -pedantic -fanalyzer`) currently fails with 7 errors across 3 source files, blocking the lint gate and masking any future static-analysis regressions. Two of the three are pure type-signature warnings; the third (missing `LogEntry` field initializers) also hides a latent bug: the `Logger::dummy` member is default-initialized with indeterminate `int` fields (`line`, `column`, `code`) and an indeterminate `severity`, which can be returned to callers when `add()` receives an empty message.

## What Changes

- `src/infrastructure/logging/logger.h` / `logger.cpp`: give `LogEntry` explicit default and `(LogLevel, const string&)` constructors so all six fields are deterministically initialized, eliminating the `-Wmissing-field-initializers` errors and the indeterminate-`dummy` bug.
- `src/application/compiler/helpers/semantic/compiler_code_helper.h` / `.cpp`: change `addBasicChar(char)` to `addBasicChar(unsigned char)`, matching its actual semantics (raw byte emission into `addLdiHL(unsigned char)`) and removing the `-Woverflow` errors on `0xC0`/`0xC5` MSX-BASIC tokens.
- `src/application/compiler/helpers/semantic/compiler_code_optimizer.cpp`: drop the always-true `address >= def_wrapper_routines_map_table` lower-bound comparison in `getKernelCallAddr()` (the table base is `0x0000`, and the outer guard already ensures `address < 0x4000`), removing the `-Wtype-limits` error.

## Capabilities

### New Capabilities

- `logging`: The `Logger`/`LogEntry` contract — `LogEntry` fields SHALL be deterministically initialized (no indeterminate values), including the `dummy` entry returned for empty messages.

### Modified Capabilities

None. The `addBasicChar` type change and the `getKernelCallAddr` lower-bound removal are pure implementation details; dispatch-table bounds semantics in `wrapper-dispatch-table` and compiler/kernel-call behaviors are preserved byte-for-byte.

## Impact

- `src/infrastructure/logging/logger.h`, `src/infrastructure/logging/logger.cpp`
- `src/application/compiler/helpers/semantic/compiler_code_helper.h`, `compiler_code_helper.cpp`
- `src/application/compiler/statements/strategies/graphics/set/compiler_set_statement_strategy.cpp` (callers of `addBasicChar` remain source-compatible)
- `src/application/compiler/helpers/semantic/compiler_code_optimizer.cpp`

No runtime behavior or generated-ROM output changes. Target release: **1.2.2.0**.
