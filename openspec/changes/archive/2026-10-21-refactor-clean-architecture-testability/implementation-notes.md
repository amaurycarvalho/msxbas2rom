# Implementation Notes

## Phase A baseline (task 1.1)

Captured at the start of Phase A:

- Commit/reference: `cc4751179d67671c1ec6a4ab1edc61cb1a948267`
- Unit suite: 546 test cases, 3755 assertions, all passing.
- Unit suite wall time: 37.42 s (`tests/unit/bin/test_unit`, `--duration=true`).
- Integration suite wall time: 25.81 s (`make test-integration`, 161 `.bas`
  files compiled in four ROM modes).

Slowest unit tests:

```text
10.650134 s: ResourceBlobPackedReader packs data at the 8K boundary
 2.030410 s: All statement strategies execute successfully
 1.592160 s: All function strategies execute successfully
 1.429269 s: Function strategies handle decimal/string argument subtypes
 1.207340 s: ResourceSprReader caps the sprite pattern table at 64 sprites
 1.025096 s: Resolves cross-segment GOSUB call in MegaROM
 0.979725 s: Resolves cross-segment conditional jumps in MegaROM
 0.902215 s: All CMD instruction handlers execute successfully
 0.708686 s: PRINT statement
 0.574138 s: Math functions compile with numeric/single/double arguments
```

A rerun of the unit suite at the same commit produced the same test and
assertion counts. The integration rerun also finished successfully.

## Phase A dependency inventory and characterization (task 1.2)

The automated checker replaced the manual inventory. Before Phase A the
approved debt was: domain -> `fswrapper.h`/`cpu_opcode_writer.h`/
`symbol_manager.h`; application -> concrete `logger.h` (39 files), `fswrapper.h`
(4 files), and the permanent `pletter.h` exception (2 files); direct
`fopen`/`ifstream`/`ofstream`/`remove` in 10 application files; generated
`bin_header_bin`/`header.h`/`start.h` in `rom.cpp`, `compiler.cpp`, and
`compiler_code_optimizer.cpp`.

After Phase A the remaining approved debt is the concrete logger include (39
files), the resource/path `fswrapper` includes (4 files), the permanent
`pletter.h` exception (2 files), and ROM persistence in `rom.cpp` (owned by
Phase B task 4.2). The domain now has no filesystem dependency, kernel access
is behind `KernelImage`, source/includes use `SourceResolver`, resource readers
use `ResourceContentSource`, and symbol exporters use `SymbolOutputSink`.

Characterization coverage added in this session:

- `tests/unit/src/test_source_boundary.cpp`: in-memory lexer/parser/compiler
  diagnostics, file-adapter diagnostics, in-memory `INCLUDE`, and path policy.
- `tests/unit/src/test_kernel_image.cpp`: deterministic kernel image, dispatch
  resolution, compiler relocation call resolution, and mapper patch offsets.
- `tests/unit/src/test_resource_boundary.cpp`: valid, boundary, empty, missing,
  truncation, and file-adapter equivalence for blob/text readers.
- `tests/unit/src/test_symbols.cpp`: in-memory symbol sink for all formats.
- `tests/unit/src/test_characterization.cpp`: logger/CLI diagnostic formatting,
  in-memory versus file-backed parser/compiler/ROM bytes, and the plain-ROM
  overflow input to the ASCII8 retry policy.

## Phase A handoff (task 1.6)

Post-Phase-A measurement at the same working tree:

- Architecture check: `make test-architecture` green with the remaining
  approved exceptions listed above.
- Unit suite: 565 test cases, 3875 assertions, all passing; wall time 47.14 s.
  The increase over the 37.42 s baseline comes from the new characterization
  tests (not removed or slowed existing tests).
- Mutation-scope proxy (normal suite excluding `Slow`): 564 test cases, 3872
  assertions, 1 skipped; wall time 26.02 s.
- Integration suite: 26.54 s when run against the rebuilt release binary
  (`make -C tests/integration run`), versus 25.81 s baseline. The complete
  `make test-integration` invocation also rebuilds the release binary and took
  398.27 s once in this session.

No test was removed. The fresh Mull baseline and Round 6 remain user-managed by
design; this is the Phase A handoff point.

## Phase B completion

- Diagnostics are application-owned (`src/application/diagnostics/logger.*`); no
  application source includes an infrastructure logger. The CLI remains the only
  place that prints collected diagnostics.
- `Rom` now assembles a `RomArtifact` (pages, sizes, shares, resource-map
  metadata, diagnostics) and persists it through `RomWriter`. The file adapter
  lives in `src/infrastructure/fs/file_rom_writer.*`; tests use
  `InMemoryRomWriter`.
- Compilation orchestration moved from `main.cpp` to
  `CompilationUseCase`. `main.cpp` now parses arguments, composes the production
  adapters, calls the use case, prints its output, prints the footer on success,
  and maps the result to the process exit code. The use case returns structured
  success/exit/retry/artifact/diagnostic data.
- Parser, compiler, and CPU workspace state are private. Callers use explicit
  accessor collaborators (`getLexer()`, `getOpts()`, `getCodeOptimizer()`,
  `getCodePointer()`, and so on), including parser/compiler strategies and
  semantic helpers. `FixNode` no longer depends on the application opcode
  writer; it receives the code pointer explicitly.
- Unit-test BASIC compilation uses the in-memory `TestSourceResolver` registered
  through `createTempBas`; only explicit source/file-adapter and CLI tests touch
  real files. The architecture checker now passes with only the permanent
  `pletter.h` exception.

Phase B verification:

- `make test-architecture`: green (2 approved violations, 1 permanent
  `pletter.h` exception).
- `make test`: 569 test cases, 3909 assertions, all passing.
- `make lint`: passing.
- `make -C tests/integration run`: all 161 integration programs compile in four
  ROM modes.
- CLI smoke checks preserve normal, quiet, missing-input, and symbol-output
  behavior.

Task 7.5 remains user-managed: run `make mutation-run` and `make mutation-check`.
