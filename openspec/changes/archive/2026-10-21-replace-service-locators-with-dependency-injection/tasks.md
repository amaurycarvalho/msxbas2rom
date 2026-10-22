## 1. Baseline and guardrails

- [x] 1.1 Capture the current CLI and test baseline: normal, quiet, debug, missing-input, symbol, and ASCII8-retry outputs/exit codes; unit test count and duration. Record the reference commit and verify a rerun matches.
- [x] 1.2 Add an architecture-check guard that rejects `getDefault*`/`setDefault*` composition providers in `src/` and test-wide adapter setup in `tests/unit/src/test_main.cpp`; verify a deliberately injected symbol fails the check and is reverted.
- [x] 1.3 Add a test-inventory guard that fails when a compilation test creates or deletes a temporary `.bas` file outside an explicit adapter test; verify the current explicit adapter exceptions are documented and the guard passes.

## 2. Remove global service providers

- [x] 2.1 Remove `getDefaultSourceResolver`/`setDefaultSourceResolver`; make `Lexer` require a `SourceResolver` at construction and update production/test call sites. Verify lexer, parser, compiler, and ROM tests pass.
- [x] 2.2 Remove `getDefaultKernelImage`/`setDefaultKernelImage`; make `Compiler` require a `KernelImage` and obtain it explicitly in the compilation workflow. Verify generated-code and ROM characterization tests pass.
- [x] 2.3 Remove `getDefaultResourceContentSource`/`setDefaultResourceContentSource`; make `ResourceManager`, `ResourceReader`, and `ResourceFactory` receive an explicit content source. Verify blob/text/sprite/MTF reader tests pass.
- [x] 2.4 Remove `getDefaultSymbolOutputSink`/`setDefaultSymbolOutputSink`; make `SymbolManager` require a symbol output sink. Verify all symbol format tests pass.
- [x] 2.5 Remove `getDefaultRomWriter`/`setDefaultRomWriter`; make `Rom` require a `RomWriter`. Verify ROM artifact and write-failure tests pass.
- [x] 2.6 Make every `CompilationRequest` service required and remove fallback/default resolution from `CompilationUseCase`; verify success, failure, and retry paths with explicitly supplied services.

## 3. Composition root and CLI boundary

- [x] 3.1 Move all production adapter construction into `main.cpp` as the composition root and remove global adapter installation. Verify the CLI still compiles and the architecture check passes.
- [x] 3.2 Add CLI characterization coverage for normal, quiet, debug, missing-input, symbol, and retry output/exit codes through the composition root. Verify the outputs match the task 1.1 baseline.
- [x] 3.3 Remove now-unused convenience constructors and setters that allow partially wired application objects. Verify the complete source tree builds under C++11.

## 4. Headless test harness and migration

- [x] 4.1 Add `CompilationHarness` with in-memory source, kernel, resource, symbol, and ROM adapters plus `compileSource(logicalName, source)` and `buildRom(...)` helpers. Verify a harness self-test compiles and inspects a ROM without touching the filesystem.
- [x] 4.2 Remove the test-wide adapter setup from `test_main.cpp` and delete the static test source registry. Verify the unit binary starts without global composition state.
- [x] 4.3 Migrate `test_rom.cpp` to `InMemoryRomWriter` and `RomArtifact` assertions; retain one explicit `FileRomWriter` failure test. Verify the same page, size, share, mapper-patch, and failure behaviors are covered.
- [x] 4.4 Migrate `test_symbols.cpp` to `InMemorySymbolOutputSink`; retain one explicit file-sink success/failure test. Verify all CDB, NoICE, OMDS, ELF, and symbol content assertions remain.
- [x] 4.5 Migrate `test_builder.cpp` and resource-reader tests to `InMemoryResourceContentSource`; retain explicit file-adapter tests for missing/empty/truncated files. Verify resource map and size-limit behavior remains covered.
- [x] 4.6 Migrate lexer, parser, compiler, and expression tests to the harness or local in-memory resolvers; remove per-file `createTempBas` file creation. Verify `make -C tests/unit run` after each migrated family.
- [x] 4.7 Reassess `test_fs.cpp` file-based compilation scenarios; move them to integration coverage or convert them to headless harness tests while preserving the same BASIC programs and assertions.
- [x] 4.8 Keep or add a dedicated file-adapter suite covering `FileSourceResolver`, `FileRomWriter`, `FileResourceContentSource`, and `FileSymbolOutputSink`; clearly separate it from application behavior tests.

## 5. Final verification

- [x] 5.1 Run `make test-architecture`, `make test`, `make lint`, and `make -C tests/integration run`; compare unit count, coverage, and CLI behavior with the task 1.1 baseline.
- [x] 5.2 Verify the test inventory contains no temporary `.bas` creation outside explicit adapter tests and no global adapter setup; confirm the new guards fail on a deliberate violation.
- [x] 5.3 Verify `compileSource(logicalName, source)` is available for mutation tests and that `improve-mutation-score` task 24.3 can use it without filesystem setup.
- [x] 5.4 Document the composition-root and constructor-injection convention in the project guidance so future changes do not reintroduce global providers.

## Workflow follow-up

- Review the change with the user and archive it only after the verification is accepted.
