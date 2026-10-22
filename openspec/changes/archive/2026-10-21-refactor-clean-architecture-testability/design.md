## Context

See proposal.md for motivation. The present dependency direction is inverted
in several high-value paths: `BuildOptions` in domain includes filesystem
helpers; `Lexer` and `IncludeLoader` open files; `Rom` both assembles pages and
writes a file; resource readers and symbol exporters own file I/O; and
compiler/ROM code reads `bin_header_bin` directly. `ParserContext` and
`CompilerContext` are public mutable service/state aggregates. These facts
make normal unit tests create temporary `.bas` files and make focused behavior
depend on infrastructure assets.

This change is split into two phases. **Phase A (testability-first)**
establishes the architecture boundary check and its approved exception list,
the in-memory source boundary (`Lexer`/`INCLUDE`/`BuildOptions`), the kernel
image boundary, and the resource/symbol input-output boundaries. A fresh Mull
run then establishes the baseline for `improve-mutation-score` Round 6.
**Phase B** completes diagnostics, ROM output, the application workflow/CLI,
and context decomposition after the mutation score is raised. The old
`mutation_report.json` is not a valid target map after Phase A; the survivor
inventory is regenerated before Round 6.

## Goals / Non-Goals

**Goals:**

- Enforce the dependency rule with an automated boundary check and a
  pre-approved, shrinking exception list.
- Deliver the testability-first Phase A before mutation work so tests run
  without temporary BASIC files and without a global kernel-image dependency.
- Restore inward dependency direction at source, kernel, output, and CLI
  boundaries.
- Make focused lexer/parser/compiler/ROM/resource/export tests in-memory and
  deterministic.
- Preserve CLI output, diagnostics, ROM bytes, resources, exported symbols,
  mapper behavior, and C++11 cross-platform support.
- Reduce broad mutable-context coupling incrementally without a big-bang
  strategy rewrite.

**Non-Goals:**

- Change the BASIC language, ROM format, symbol formats, resource formats, or
  Z80 kernel behavior.
- Remove file-backed CLI functionality or integration tests of its adapters.
- Run Mull during the refactor; mutation verification remains user-managed.
- Reformat or rename the entire tree merely to represent layers.

## Decisions

### Decision 1: Application-owned ports with infrastructure adapters

Introduce minimal interfaces at the application boundary for source lookup,
artifact writing, and kernel image access. File, generated-header, and CLI
implementations live in infrastructure/CLI; tests use in-memory adapters.
Ports describe needed operations rather than general filesystem APIs.

- Rationale: dependency inversion allows testing rules without a real OS or
  generated asset.
- Alternative: static filesystem utility seams. Rejected because global
  functions do not make collaborators substitutable or expose ownership.

### Decision 2: Source document is the common lexer input

The lexer consumes a source document (content plus logical name) and no longer
opens the primary source itself. A source resolver supplies documents for both
the initial program and `INCLUDE` statements. File-header validation belongs
to the file adapter; text validation/tokenization remains common.

- Rationale: this directly removes temporary `.bas` dependencies and gives
  includes the same abstraction.
- Alternative: retain `Lexer::load(filename)` plus add a test-only overload.
  Rejected because two input paths would drift and preserve the inversion.

### Decision 3: Separate assembly from persistence

ROM building creates an in-memory artifact first; an output port persists it.
Resource readers receive bytes/text through source ports, and symbol export
strategies write through a sink port. Adapter tests retain real file coverage.

- Rationale: most business assertions concern bytes/tables rather than files.
- Alternative: mock `fstream`/`FILE`. Rejected because it leaves ownership and
  error policy inside application code.

### Decision 4: Kernel access is a read-only injected image

The kernel boundary resolves dispatch entries and exposes immutable kernel
bytes. The existing generated header is wrapped by a production adapter. Tests
use compact deterministic images; integration tests retain the real image.

- Rationale: eliminates direct global dependence while preserving exact
  generated-code and mapper behavior.
- Alternative: copy `bin_header_bin` into every test fixture. Rejected because
  it is large, opaque, and couples tests to generated infrastructure.

### Decision 5: Extract a compilation use case before reducing CLI logic

An application workflow coordinates lexing, parsing, compiling, ROM assembly,
symbol export, retry policy, and structured diagnostics. `main.cpp` becomes a
composition/presentation adapter only.

- Rationale: orchestration and retry behavior become testable without process
  termination, printing, or output-file removal.
- Alternative: unit-test `main`. Rejected because process concerns obscure
  workflow behavior and make failure paths brittle.

### Decision 6: Contexts are decomposed by migration slice

Do not replace `CompilerContext`/`ParserContext` wholesale. For each migrated
strategy family, inject only its required state/collaborators and make that
slice private. Compatibility facades are temporary and deleted after callers
migrate.

- Rationale: reduces risk in a large strategy graph and preserves behavior.
- Alternative: one global dependency-injection container. Rejected because it
  would hide dependencies and retain service-locator coupling.

### Decision 7: Automated boundary check with approved exceptions

Add `scripts/check_clean_architecture.py` plus a version-controlled
`scripts/architecture-exceptions.json`. The checker enforces the inward
dependency rule and forbids direct filesystem, generated-kernel, and concrete
diagnostics access in `domain`/`application` outside an explicit entry. It
scans `#include` directives and the active (non-comment) source text, and it is
exercised by its own fixture-based tests. `make test-architecture` runs the
self-tests and the real scan; `make test`, `quality-gate`, and CI include it.

- Rationale: review-only enforcement drifts, especially during a large
  refactor. A machine-readable exception list makes the debt visible and
  prevents new violations.
- Alternative: rely on compiler include paths or code review. Rejected because
  neither detects direct `fopen`/`bin_header_bin` use, stale exceptions, or
  layer ownership intent.

### Decision 8: Exceptions are explicit, owned, and removed eagerly

The exception file contains exact repository-relative paths (no globs that
would silently cover new files), the forbidden dependency/pattern, the owning
task, and the removal criterion. The checker fails when an exception no longer
matches a violation, when its path disappears, or when the file is malformed.
The only permanent exception approved initially is the direct `pletter.h`
include from two application headers, because it is a pure compression
algorithm with no OS, kernel, or presentation dependency.

- Rationale: an exception list without expiry becomes a second architecture.
  Stale detection turns every completed migration into a required cleanup.
- Alternative: blanket directory exceptions. Rejected because they would let
  new files inherit old debt.

### Decision 9: Phase A precedes mutation; Phase B follows it

Split the migration into a testability-first Phase A and a completion Phase B:

- **Phase A:** boundary check and exceptions; source document boundary
  (tasks 2.1-2.5); kernel image boundary (tasks 3.1-3.2); resource and symbol
  in-memory boundaries (tasks 4.3-4.4); duration measurement.
- **Handoff:** the user runs `make mutation-run` once to create a fresh
  `mutation_report.json`; the survivor inventory is regenerated from it.
- **Mutation Round 6:** `improve-mutation-score` adds the killing tests against
  the stable Phase A APIs.
- **Phase B:** diagnostics (3.3-3.4), ROM persistence (4.1-4.2), application
  workflow/CLI (5.x), context decomposition (6.x), and final verification.
- **After Phase B:** a second user-managed Mull run reconciles the mutation
  score with the refactored source.

Mutation tests use two test-only seams: `compileSource(logicalName, source)`
and `kernelImage()`. Phase A backs them with the production source and kernel
boundaries; Phase B may change their internals without changing test call
sites.

- Rationale: it delivers the speed and determinism the user needs for a
  viable mutation run without waiting for all 27 refactor tasks, and it avoids
  targeting a survivor map that Phase A would invalidate.
- Alternative: run the full refactor before mutation. Rejected because it
  delays the 85% gate and changes the mutant denominator twice.
- Alternative: run mutation before any refactor. Rejected because temporary
  BASIC files and white-box kernel/context access would then be rewritten by
  the refactor.

## Approved Boundary Exceptions (initial)

These entries are the pre-approved baseline for the first boundary-check run.
The machine-readable form will live in `scripts/architecture-exceptions.json`.
Each temporary entry names the task that removes it; `check_clean_architecture.py`
fails if the entry becomes stale. Comments are ignored by the checker.

### Domain violations

| ID | Path | Forbidden dependency | Owner |
|---|---|---|---|
| A1 | `src/domain/options/build_options.cpp` | `fswrapper.h` | 2.5 |
| A2 | `src/domain/symbols/fix_node.cpp` | `cpu_opcode_writer.h` | 6.3 |
| A3 | `src/domain/symbols/symbols.h` | `symbol_manager.h` | 6.4 |

### Application -> infrastructure

| ID | Paths | Forbidden dependency | Owner |
|---|---|---|---|
| B1 | the 39 files below | `logger.h` | 3.3/3.4 |
| B2 | `resource_factory.cpp`, `resource_manager.cpp`, `compiler_bload_statement_strategy.cpp`, `compiler_file_statement_strategy.cpp` | `fswrapper.h` | 2.2/4.2/4.3 |
| B3 | `resources.h`, `resource_blob_packed_reader.h` | `pletter.h` | permanent |
| B4 | `src/application/builder/rom.cpp` | `header.h`, `start.h` | 3.2 |

B1 paths:

```text
src/application/builder/resources/interface/resource_reader.cpp
src/application/builder/resources/resource_manager.cpp
src/application/builder/resources/strategies/arkos/resource_akm_reader.cpp
src/application/builder/resources/strategies/arkos/resource_akx_reader.cpp
src/application/builder/resources/strategies/blob/resource_blob_chunk_packed_reader.cpp
src/application/builder/resources/strategies/blob/resource_blob_packed_reader.cpp
src/application/builder/resources/strategies/blob/resource_blob_reader.cpp
src/application/builder/resources/strategies/data/resource_data_reader.cpp
src/application/builder/resources/strategies/data/resource_txt_reader.cpp
src/application/builder/resources/strategies/mtf/resource_mtf_map_reader.cpp
src/application/builder/resources/strategies/mtf/resource_mtf_reader.cpp
src/application/builder/resources/strategies/sprite/resource_spr_reader.cpp
src/application/builder/rom.cpp
src/application/compiler/compiler.cpp
src/application/compiler/helpers/context/compiler_context.cpp
src/application/compiler/helpers/semantic/compiler_evaluator.cpp
src/application/compiler/helpers/semantic/compiler_expression_evaluator.cpp
src/application/compiler/helpers/semantic/compiler_float_converter.cpp
src/application/compiler/helpers/semantic/compiler_variable_emitter.cpp
src/application/compiler/statements/strategies/basic/dim/compiler_dim_statement_strategy.cpp
src/application/compiler/statements/strategies/graphics/line/compiler_line_statement_strategy.cpp
src/application/lexer/lexer.cpp
src/application/parser/helpers/context/parser_context.cpp
src/application/parser/helpers/semantic/assignment_evaluator.cpp
src/application/parser/helpers/semantic/expression_evaluator.cpp
src/application/parser/helpers/semantic/parser_line_evaluator.cpp
src/application/parser/parser.cpp
src/application/parser/statements/strategies/color/color_statement_strategy.cpp
src/application/parser/statements/strategies/data/data_statement_strategy.cpp
src/application/parser/statements/strategies/def/def_statement_strategy.cpp
src/application/parser/statements/strategies/dim/dim_statement_strategy.cpp
src/application/parser/statements/strategies/file/file_statement_strategy.cpp
src/application/parser/statements/strategies/for/for_statement_strategy.cpp
src/application/parser/statements/strategies/graphics/graphics_statement_strategy.cpp
src/application/parser/statements/strategies/if/if_statement_strategy.cpp
src/application/parser/statements/strategies/let/let_statement_strategy.cpp
src/application/parser/statements/strategies/put/put_statement_strategy.cpp
src/application/parser/statements/strategies/set/set_statement_strategy.cpp
src/application/parser/statements/strategies/time/time_statement_strategy.cpp
```

### Kernel globals

| ID | Paths | Forbidden pattern | Owner |
|---|---|---|---|
| C1 | `src/application/builder/rom.cpp`, `src/application/compiler/compiler.cpp`, `src/application/compiler/helpers/semantic/compiler_code_optimizer.cpp` | `bin_header_bin` | 3.2 |

### Direct persistence in application/domain

| ID | Paths | Forbidden pattern | Owner |
|---|---|---|---|
| D1 | the 10 files below | `fopen`, `ifstream`, `ofstream`, `remove` | 2.3/2.4/4.2/4.3/4.4 |

D1 paths:

```text
src/application/lexer/lexer.cpp
src/application/parser/helpers/preprocess/include_loader.cpp
src/application/builder/rom.cpp
src/application/builder/resources/strategies/blob/resource_blob_reader.cpp
src/application/builder/resources/strategies/data/resource_txt_reader.cpp
src/application/symbols/strategies/export/cdb/cdb_export_strategy.cpp
src/application/symbols/strategies/export/noice/noice_export_strategy.cpp
src/application/symbols/strategies/export/omds/omds_export_strategy.cpp
src/application/symbols/strategies/export/symbol/symbol_file_export_strategy.cpp
src/application/symbols/strategies/export/elf/elf_export_strategy.cpp
```

`src/application/builder/resources/resources.h` currently includes `<fstream>`
without a direct use. The preferred implementation removes the include; if a
transitive dependency is discovered, it becomes a temporary entry owned by
task 4.3.

## Risks / Trade-offs

- [Behavior drift while splitting I/O] -> Establish byte-for-byte and
  diagnostic characterization tests before each extraction; keep adapter and
  in-memory tests paired.
- [Port proliferation] -> Keep interfaces per use case and avoid generic
  filesystem abstractions.
- [Large context migration affects many strategies] -> Migrate one coherent
  family at a time, preserve a temporary facade, and compile the full unit
  tree after each slice.
- [Kernel fixture diverges from production] -> Run integration tests against
  the generated kernel and make fixtures declare only the dispatch entries
  required by their scenario.
- [Phase A invalidates the current mutation report] -> Treat
  `improve-mutation-score` Round 6 as blocked until a fresh `make mutation-run`
  regenerates the survivor inventory; do not reuse the 06/out line targets.
- [Speed gain is smaller than expected] -> Measure durations after the source
  boundary and after the resource/symbol boundaries; combine with the
  `Slow`/focused-test strategy described in `improve-mutation-score` before
  concluding the mutation baseline is viable.
- [Exception list becomes permanent debt] -> Stale-exception detection,
  per-entry owner/removal criterion, and task 7.5 make cleanup mandatory.
- [Context decomposition after mutation forces test edits] -> Keep the
  white-box relocation/layout harness in one file, use `Compiler::write` as
  the public surface, and route kernel access through the `kernelImage()` seam.

## Migration Plan

1. **Phase A:** add the boundary check, exception list, and self-tests; capture
   current duration and behavior baselines.
2. **Phase A:** add source/kernel/resource/symbol ports and in-memory adapters
   alongside existing paths; characterize equivalence and measure durations.
3. **Handoff:** the user runs `make mutation-run` to produce a fresh baseline;
   `improve-mutation-score` regenerates its survivor inventory and runs Round 6.
4. **Phase B:** complete diagnostics, ROM persistence, application workflow/CLI,
   and context decomposition in behavior-preserving slices.
5. **Cleanup:** remove temporary exceptions as their owning tasks complete;
   the boundary check must pass with only approved permanent exceptions.
6. **Final:** run normal unit/integration suites; the user runs the final Mull
   run and reconciles the mutation score with the refactored source.

Rollback is per slice: retain the old adapter path until its new equivalent is
covered and passing, then remove it in the same verified slice.
