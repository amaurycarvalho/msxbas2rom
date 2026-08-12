## 1. Logger — deterministic LogEntry initialization

- [x] 1.1 Add a default constructor to `Logger::LogEntry` in `src/infrastructure/logging/logger.h` that initializes `severity` to `LogLevel::INFO`, `message`/`file` to empty strings, and `line`/`column`/`code` to `0`
- [x] 1.2 Add a `LogEntry(LogLevel, const string&)` constructor that sets `severity` and `message` from arguments and initializes the remaining fields as in 1.1
- [x] 1.3 Verify `logs.push_back({severity, message})` in `logger.cpp` still compiles against the new constructor

## 2. Compiler code helper — unsigned byte emission

- [x] 2.1 Change `addBasicChar(char c)` to `addBasicChar(unsigned char c)` in `compiler_code_helper.h`
- [x] 2.2 Update the `addBasicChar` definition signature in `compiler_code_helper.cpp` to match
- [x] 2.3 Confirm all `addBasicChar` callers (set statement strategy, `beginBasicSetStmt`) compile unchanged

## 3. Code optimizer — remove tautological bound check

- [x] 3.1 In `compiler_code_optimizer.cpp` `getKernelCallAddr()`, remove the always-true `address >= def_wrapper_routines_map_table &&` clause, retaining `address < def_wrapper_routines_map_table + DISP_ENTRIES * 2`

## 4. Verification

- [x] 4.1 Compile each changed `.cpp` file with the lint flags to confirm zero diagnostics (`g++ -fsyntax-only -Wall -Wextra -Werror -pedantic -Wno-unused-parameter -I…`)
- [x] 4.2 Run `make lint` (asynchronously, per governance) and confirm it passes
- [x] 4.3 Run `make test-unit` (asynchronously) to confirm no behavioral regressions
