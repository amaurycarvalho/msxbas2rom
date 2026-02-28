# Project Insights

This document captures emergent knowledge discovered during development. Write here your discoveries about this project.

---

## General rules

It is NOT a source of formal requirements or architectural rules.

If an insight becomes stable and recurring, it should be marked here and proposed to be promoted to:

- constitution.md (if normative)
- glossary.md (if semantic)
- an ADR (if architectural)

If an insight registered here is not relevant any more, remove it.

---

## Discoveries

### 1. Observed Code Patterns

- The codebase already follows the target layered structure from ADR-001: `src/cli`, `src/application`, `src/domain`, and `src/infrastructure`.
- `src/application/lexer/lexer_line` already uses State + Factory (`src/application/lexer/states/*` and `states/factory/lexer_line_state_factory.*`) with minimal behavior change in `lexer_line.cpp`.
- `src/application/symbols/symbol_manager` already uses Strategy + Factory (`src/application/symbols/strategies/*` and `strategies/factory/symbol_export_strategy_factory.*`) to dispatch export formats.

### 2. Implicit Conventions

- Refactors are being executed in two phases: first "one file per class" with no logic change, then pattern introduction (Strategy/State) with minimum code change.
- Build and test discovery are path-driven (recursive `find` in `Makefile` and `tests/unit/Makefile`), reducing manual source list maintenance after file moves.
- Unit tests compile almost the full production tree (`tests/unit/Makefile` includes all `src/**/*.cpp` except `src/cli/main.cpp`), which quickly reveals broken includes after refactors.

### 3. Repeated Architectural Decisions

- Behavioral polymorphism is being centralized in `application` layer via factories (`lexer` states and `symbols` export strategies), keeping orchestration classes slimmer.
- `infrastructure/kernel/asm` was isolated under infrastructure, reinforcing separation between business/application logic and low-level platform assets.

### 4. Edge Cases Frequently Missed by AI

- Parser remains monolithic (`src/application/parser/parser.h` + `parser.cpp`), with many command-specific methods; splitting must preserve parsing order/state side effects to avoid silent regressions.
- Current include guards still use legacy names in some files (example: `PARSE_H_INCLUDED` in `parser.h`), so automated renames can accidentally desynchronize guard names and file names.
- Filesystem concerns are only partially abstracted: `fswrapper` exists but there are still direct file operations in `application` and `cli` (`fopen`, `ifstream/ofstream`, `remove`), so broad refactors can easily break behavior if done without mapping each call site.

### 5. Performance Observations

- Recursive source discovery in makefiles scales well for frequent folder/file moves in this sprint, but it increases recompilation surface because many tests link against the full source set.

### 6. Testing Insights

- Unit tests are organized in `tests/unit/src/test_*.cpp` and executed individually (`test_lexer`, `test_parser`, `test_compiler`, `test_builder`, `test_fs`, `test_resources`, `test_options`), which helps isolate failures by module.
- `tests/deprecated` still contains large legacy assets and scenarios not part of the unit test pipeline; useful as manual regression corpus after major parser/compiler/builder refactors.

### 7. Refactoring Opportunities

- Next parser tasks can mirror the already successful `symbols` approach: extract command handlers into strategy classes and keep parser focused on flow control/context.
- The upcoming filesystem adapter task can be derisked by first cataloging all direct file-access call sites in `src/application/*` and `src/cli/*`, then migrating by feature slice.

### 8. Open Questions

- Should `fswrapper` evolve from free functions to an interface + injected implementation before parser/compiler/builder strategy refactors, or after them?
