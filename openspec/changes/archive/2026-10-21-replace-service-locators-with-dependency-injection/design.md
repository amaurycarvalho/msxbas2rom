## Context

See `proposal.md` — Why. The ports introduced by the previous refactor already
have in-memory implementations (`InMemorySourceResolver`,
`InMemoryKernelImage`, `InMemoryResourceContentSource`,
`InMemorySymbolOutputSink`, `InMemoryRomWriter`). The remaining problem is the
composition mechanism: global provider functions and default constructors let
application classes resolve adapters themselves, and the unit binary installs a
global test resolver before any test runs.

## Goals / Non-Goals

**Goals:**

- Make the production dependency graph visible in one composition root.
- Give application components required, constructor-injected ports.
- Remove process-wide mutable composition state from `src/` and tests.
- Make the default unit-test path fully headless and deterministic.
- Preserve CLI output, exit codes, ROM/resource/symbol bytes, and diagnostics.

**Non-Goals:**

- Introduce a general-purpose DI container or service locator replacement.
- Change compiler algorithms, ROM formats, symbol formats, or resource formats.
- Remove file-backed functionality; file adapters remain production behavior.
- Require every pure utility function to become an object. Stateless helpers may
  remain free functions when they have no collaborators or lifecycle.

## Decisions

### Decision 1: Parameter object for the compilation graph

Replace per-call fallback lookups with a required `CompilationServices` value
inside `CompilationRequest`:

- source resolver
- kernel image
- resource content source
- symbol output sink
- ROM writer

`CompilationUseCase::execute` SHALL use only the supplied services. It SHALL NOT
call `getDefault*`, construct a file adapter, or mutate process-wide state.

Rationale: the use case has a stable, small set of ports; a parameter object
keeps the call site readable and avoids a long constructor parameter list.
Alternative: a full DI container. Rejected because it hides the graph and is
unnecessary for this object count.

### Decision 2: Constructor injection at each seam

- `Lexer` requires `SourceResolver` at construction; `load()` accepts options
  and source text/documents, not a resolver fallback.
- `Compiler` requires `KernelImage` at construction.
- `Rom` requires `RomWriter` at construction.
- `ResourceManager` and `ResourceReader` require `ResourceContentSource`.
- `SymbolManager` requires `SymbolOutputSink`.
- `ResourceFactory` becomes an instance owned by the resource manager and is
  constructed with the content source.

Rationale: required constructor arguments make missing wiring a compile-time
error instead of a runtime default. Alternative: setters plus null checks.
Rejected because setters allow partially constructed objects and preserve the
implicit resolution pattern.

### Decision 3: CLI is the composition root

`main.cpp` constructs the production adapters (`FileSourceResolver`,
`GeneratedKernelImage`, `FileResourceContentSource`, `FileSymbolOutputSink`,
`FileRomWriter`) and passes them through `CompilationRequest`. No application
header exposes a way to install or retrieve a global adapter.

Rationale: this is the standard composition-root pattern for a CLI application.
Alternative: an infrastructure-level service factory. Rejected because it would
move composition out of the executable and make the wiring less obvious.

### Decision 4: Diagnostics remain application-owned, not service-located

`Logger` remains an application-level diagnostic collector. Components may own a
collector, but they SHALL NOT resolve one globally. Injecting a collector into
every strategy is deferred because it does not affect file/UI testability and
would expand this refactor substantially.

Rationale: the static-provider problem is about outer adapters; the logger is an
in-memory value collector with no OS dependency. Alternative: inject a
diagnostic port everywhere now. Rejected as unrelated scope for this change.

### Decision 5: Headless test harness replaces global test adapters

Create a `CompilationHarness` in test support code owning:

- `InMemorySourceResolver`
- deterministic `InMemoryKernelImage`
- `InMemoryResourceContentSource`
- `InMemorySymbolOutputSink`
- `InMemoryRomWriter`

It exposes `compileSource(logicalName, source, options)` and `buildRom(...)`.
Tests receive the harness or construct it locally. No registry, no `test_main`
adapter installation, and no temporary `.bas` files for compilation tests.

Rationale: the harness makes the dependency graph explicit in tests and provides
the mutation-testing `compileSource` seam. Alternative: keep a test-wide file
resolver. Rejected because it retains global state and hides which tests depend
on file I/O.

### Decision 6: Adapter tests stay separate and explicit

File-backed adapters keep dedicated tests for real I/O, missing files, write
failures, and header diagnostics. Those tests may use temporary files but SHALL
be named/grouped as adapter tests. Application/compiler/ROM/symbol tests use only
in-memory adapters.

Rationale: deleting all file coverage would hide adapter regressions; mixing it
into every test would keep the suite slow and environment-dependent.

## Risks / Trade-offs

- **Large mechanical call-site change** (`Lexer`, `Compiler`, resource readers)
  → Introduce the harness first, then migrate one component family at a time;
  compile and run the unit suite after each slice.
- **Behavior drift in CLI output** → Keep characterization tests for normal,
  quiet, debug, error, and retry paths; compare before/after output.
- **Test coverage erosion during migration** → Port each existing assertion to
  the in-memory equivalent; do not delete a scenario without replacement
  coverage. Keep the same boundary and failure cases.
- **Factory lifetime bugs** → Make the reader factory owned by the resource
  manager and pass non-owning references only where the lifetime is clear.
- **Accidental hidden default reintroduced** → Add an architecture-check rule
  that rejects `getDefault`/`setDefault` provider symbols in application or CLI
  code, or a targeted source test for the same.

## Migration Plan

1. Remove the five global provider pairs and their static storage.
2. Make `CompilationRequest` carry required services; update `CompilationUseCase`
   and `main` composition.
3. Convert `Lexer`, `Compiler`, `Rom`, resource readers/factory, and
   `SymbolManager` to constructor injection.
4. Add the headless test harness and remove `test_main` global setup.
5. Migrate ROM, symbol, resource, lexer/parser/compiler, and characterization
   tests to the harness; keep a focused file-adapter suite.
6. Remove the test source registry and temporary `.bas` helpers.
7. Run `make test-architecture`, `make test`, `make lint`,
   `make test-integration`, and the CLI smoke checks.

Rollback is per slice: keep the previous constructor/overload available until
the new explicit wiring passes the suite, then remove it in the same slice.
