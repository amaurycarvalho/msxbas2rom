# Agents

Consult `openspec/` folder for more information related this project, starting by `openspec/changes/import-existing-specs/specs/project-constitution/spec.md`. Skills provide specialized instructions and workflows for specific tasks.

## Tool Usage Rules

**Prefer cocoindex-code semantic search over grep/glob for code understanding.**

The project has a semantic index via `cocoindex-code` MCP (748 files, 3712 chunks: 2462 C++, 739 Markdown, 502 text/ASM).

Use `cocoindex-code_search` when:
- Finding implementations or understanding how features work
- Locating related code without knowing exact names or keywords
- Searching OpenSpec specs by meaning (requirements, scenarios, design decisions)
- Exploring unfamiliar parts of the codebase

Use `grep` only when:
- Matching exact strings (specific symbol names, error messages, string literals)
- Counting occurrences of a pattern

Use `glob` only when:
- Listing files by name pattern (e.g., `test_*.cpp`, `*.asm`)
- Discovering directory structure

Fall back to grep/glob only when semantic search returns insufficient or irrelevant results.

## Composition Root and Dependency Injection

- Production adapters are constructed only in `src/cli/runner/runner.cpp`, the CLI
  composition root. `main.cpp` only delegates to `runCli`.
- Application and domain components receive their external collaborators through
  constructors or parameter objects (`CompilationRequest`). Do not add global
  `getDefault*`/`setDefault*` providers, mutable process-wide state, or
  service-locator fallbacks.
- Tests compose their own collaborators. Use `CompilationHarness` from
  `tests/unit/src/test_support.h` for headless compilation and ROM inspection.
  File-backed adapters are covered only by explicit adapter tests marked with
  `// test-inventory: file-adapter`.
- Run `make test-architecture` to enforce the inward dependency rule, reject
  global composition providers, and reject temporary `.bas` files in headless
  unit tests.
