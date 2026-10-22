## Why

The Clean Architecture refactor introduced application ports, but the first
implementation kept composition implicit through global `setDefault*` /
`getDefault*` providers and test-wide adapter setup. This is a service-locator
pattern: constructors resolve collaborators on their own, initialization order
becomes implicit, tests share mutable global state, and the real dependency
graph is hidden from callers.

Now that the ports and in-memory adapters exist, composition can be made
explicit without changing compiler behavior, ROM bytes, diagnostics, or the CLI
contract. Doing it now also removes the main source of file-based/UI-oriented
unit tests, making the suite faster and more deterministic.

## What Changes

- Remove the five global provider pairs: source resolver, kernel image,
  resource content source, symbol output sink, and ROM writer.
- Require ports through constructor/parameter-object injection instead of
  resolving defaults inside application classes.
- Concentrate production adapter construction in `main.cpp` as the composition
  root; application code only sees abstractions.
- Make `CompilationRequest` carry a complete, required set of compilation
  services instead of falling back to global defaults.
- Pass `ResourceContentSource` through the resource reader factory instead of
  setting it after construction.
- Remove the global test adapter setup and static test source registry.
- Introduce a headless compilation test harness backed by in-memory source,
  kernel, resource, symbol, and ROM adapters.
- Migrate ROM, symbol, resource, lexer/parser/compiler tests to in-memory
  assertions; retain a small explicit adapter suite for real file I/O.
- Preserve the public CLI behavior, diagnostics, ROM bytes, resource bytes, and
  symbol formats.

## Capabilities

### New Capabilities

None.

### Modified Capabilities

- `architecture`: Dependency injection and composition become explicit:
  application components receive owned abstractions; global mutable providers
  and implicit service location are no longer permitted.
- `unit-test-coverage`: Unit tests must exercise application behavior through
  in-memory adapters without global composition; file/UI tests are limited to
  explicit adapter coverage. Existing test intent and coverage must be
  preserved while tests are migrated.

## Impact

- `src/application/source`, `src/application/kernel`,
  `src/application/builder`, `src/application/compilation`,
  `src/application/parser`, `src/application/compiler`,
  `src/application/symbols`, and `src/application/lexer` constructors and
  composition seams.
- `src/cli/main.cpp` becomes the production composition root.
- `tests/unit/src/test_main.cpp`, existing test helpers, and the ROM, symbol,
  resource, lexer, parser, compiler, and characterization suites.
- No external CLI/API, ROM format, symbol format, or resource format change.
