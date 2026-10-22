## 1. Phase A — prerequisites and characterization baseline

Phase A is the testability-first prerequisite for `improve-mutation-score`
Round 6. It starts immediately; it no longer depends on mutation task 24.2a.

- [x] 1.1 Capture the current normal-suite duration and behavioral baseline (`./bin/test_unit --duration=true`, unit suite, integration suite, ROM/resource/symbol characterization outputs) and record the commit/reference at the start of Phase A. Verify the recorded baseline matches a rerun of the same commands.
- [x] 1.2 Inventory direct filesystem, generated-kernel, concrete-logger, and public-context dependencies in the affected modules; add characterization tests for current CLI diagnostics, lexer/parser/compiler output, ROM bytes, resource bytes, symbol text, and retry behavior. Verify `make -C tests/unit run`.
- [x] 1.3 Add `scripts/check_clean_architecture.py` and `scripts/tests/test_check_clean_architecture.py`; the checker SHALL enforce the inward dependency rule, detect active direct `fopen`/`ifstream`/`ofstream`/`remove`, `bin_header_bin`/`header.h`/`start.h`, and concrete `fswrapper`/`logger` access in `domain`/`application`, ignore comments, and report file plus forbidden dependency. Verify its fixture tests demonstrate that a new violation fails and an approved exception passes.
- [x] 1.4 Create `scripts/architecture-exceptions.json` from the approved baseline in `design.md`, with exact paths, forbidden dependency/pattern, owning task, and removal criterion. Verify the checker fails on a stale entry, a missing path, and a malformed file; verify the real scan passes with only listed exceptions.
- [x] 1.5 Wire `make test-architecture` into `make test`, `quality-gate`, and the CI quality-gate workflow. Verify a deliberately injected unapproved violation fails the command and is reverted.
- [x] 1.6 Measure Phase A durations after the source, kernel, and resource/symbol boundaries are in place; record the mutation baseline (normal suite excluding `Slow`) and confirm the handoff criteria for `improve-mutation-score` Round 6. Verify the measurement is reproducible and no test was removed.

## 2. Phase A — source and path boundaries

- [x] 2.1 Define application-level source-document and source-resolver contracts containing source text and logical name, with an in-memory implementation for tests. Verify a lexer test accepts BASIC text without a `.bas` file.
- [x] 2.2 Implement the file-backed source adapter in infrastructure, including existing missing-file, empty-file, and tokenized-file diagnostics. Verify adapter-focused tests preserve the current messages and file-header behavior.
- [x] 2.3 Migrate `Lexer` to consume source documents and preserve line numbers, logical source names, tokenization, and logger diagnostics. Verify equivalent in-memory and file-backed programs yield equal lexer results.
- [x] 2.4 Migrate `IncludeLoader` and parser include handling to the source resolver. Verify nested, missing, and invalid `INCLUDE` scenarios work with both in-memory and file adapters without a temporary `.bas` primary source.
- [x] 2.5 Remove `fswrapper` use from `BuildOptions`; relocate path/derived-output-name policy to an application/outer boundary while preserving all compile-mode filenames. Verify domain tests require no filesystem dependency and CLI naming tests remain unchanged.

## 3. Kernel image boundary (Phase A) and diagnostics (Phase B)

3.1-3.2 are Phase A; 3.3-3.4 are Phase B.

- [x] 3.1 Define a read-only application kernel-image contract for bytes, length, and dispatch-entry resolution, plus a deterministic in-memory test image. Verify call relocation and mapper-patch tests use the supplied image.
- [x] 3.2 Wrap `bin_header_bin` and generated dispatch constants in the infrastructure kernel adapter; migrate compiler, optimizer, and ROM mapping code off direct global-array reads. Verify existing real-kernel integration/ROM tests retain byte-for-byte output.
- [x] 3.3 Define a narrow diagnostic collection/presentation boundary that preserves current severity, source name, line number, and CLI formatting. Verify application tests can inspect diagnostics without constructing a concrete presentation adapter.
- [x] 3.4 Migrate lexer, parser, compiler, ROM, resource manager, and reader construction away from internally creating concrete loggers where the dependency crosses an application boundary. Verify existing diagnostics remain equivalent through CLI and in-memory tests.

## 4. Resource/symbol boundaries (Phase A) and ROM persistence (Phase B)

4.3-4.4 are Phase A; 4.1-4.2 are Phase B.

- [x] 4.1 Extract an in-memory ROM artifact from `Rom` containing pages, sizes, shares, resource-map data, and build diagnostics before persistence. Verify current ROM tests assert the same bytes without output-file creation.
- [x] 4.2 Define and implement an outer ROM writer adapter; migrate ROM persistence and write-failure behavior to it. Verify file adapter tests still detect unwritable output and in-memory writer tests observe the assembled image.
- [x] 4.3 Define resource-content source ports for text and binary readers; migrate direct `ifstream`/`fopen` readers in incremental reader families. Verify each migrated reader has in-memory valid, boundary, truncation, and file-adapter compatibility tests.
- [x] 4.4 Define a symbol-output sink and migrate CDB, NoICE, OMDS, ELF, and symbol-file exporters. Verify in-memory sink output equals existing fixture output and file adapter failure behavior remains covered.

## 5. Phase B — application compilation workflow and CLI boundary

- [x] 5.1 Define a compilation request/result use case that accepts source/options and ports, and returns structured artifacts plus diagnostics. Verify successful and failed compile paths without CLI output capture or filesystem mutation.
- [x] 5.2 Move lexical, syntactic, semantic, ROM-build, symbol-export, and automatic ASCII8 retry orchestration from `main.cpp` into the use case without changing observable CLI behavior. Verify retry, quiet/debug behavior, and exit outcomes through characterization tests.
- [x] 5.3 Reduce `main.cpp` to argument parsing, production-adapter composition, result presentation, and process exit mapping. Verify CLI integration tests preserve user-visible text and return codes on supported platforms.

## 6. Phase B — incremental parser/compiler context decomposition

- [x] 6.1 Encapsulate parser execution state and migrate one parser strategy family to explicit narrow collaborators; retain only temporary compatibility access needed by unmigrated callers. Verify parser and strategy unit tests pass before deleting the compatibility path.
- [x] 6.2 Repeat the parser-context migration for remaining strategy families and semantic helpers, removing public mutable fields once no caller uses them. Verify `make -C tests/unit run` after each family.
- [x] 6.3 Encapsulate compiler execution state and migrate one compiler strategy/helper family at a time to explicit dependencies for opcode emission, symbols, fixups, resources, and diagnostics. Verify exact generated-code/ROM characterization tests after each family.
- [x] 6.4 Remove obsolete context facades and public all-purpose fields after all callers migrate. Verify production compilation and the complete unit suite build under C++11.

## 7. Phase A/B boundary verification and handoff

- [x] 7.1 Confirm no unit test that compiles BASIC creates, loads, or deletes a temporary `.bas` file; retain file use only in explicit infrastructure/CLI adapter tests. Verify with the architecture-boundary check and targeted test inventory.
- [x] 7.2 Confirm no domain source includes infrastructure and no application source directly uses concrete filesystem persistence or generated-kernel globals outside designated adapters. Verify the dependency-boundary check reports no violations.
- [x] 7.3 Run the complete unit and integration suites from their required working directories; compare recorded ROM/resource/symbol/diagnostic characterization outputs with the baseline.
- [x] 7.4 Remove every temporary exception whose owning Phase A/Phase B task is complete; verify `make test-architecture` passes with only the approved permanent `pletter.h` exception and the report identifies no unowned debt.
- [x] 7.5 (user-managed) After Phase B and reconciliation with `improve-mutation-score` Round 6, run `make mutation-run` and `make mutation-check`; regenerate the survivor inventory and confirm the required Mutation Score is retained or improved. Do not run Mull as part of ordinary implementation tasks.
