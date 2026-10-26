# MSXBAS2ROM Architecture and Developer Guide

This document is an onboarding-oriented overview of how MSXBAS2ROM turns an
MSX-BASIC source file into a ROM image. It also explains where to add or change
keywords, statements, functions, and Z80 kernel routines.

The normative requirements live in `openspec/specs/`. When this document and a
spec disagree, the spec wins.

## 1. What the application does

MSXBAS2ROM is a C++11 command-line compiler. Its main stages are:

1. Read command-line options and the input `.bas` file.
2. Tokenize the BASIC source (lexer).
3. Build an action/tag tree and validate syntax (parser).
4. Generate Z80 machine code, symbols, fixups, and resources (compiler).
5. Pack the Z80 kernel modules, compiled code, and resources into ROM pages
   (builder).
6. Persist the assembled ROM through an output adapter.

The C++ binary is self-contained for BASIC compilation: the Z80 kernel,
module manifests, relocation data, and mapper patches are embedded at project
build time. No external assembler or linker runs when a user compiles a BASIC
program.

## 2. Layered architecture

Dependencies point inward:

```
+-----------------------------------------------------------+
| CLI (src/cli)                                             |
| argument parsing, presentation, production composition    |
+---------------------------+-------------------------------+
                            |
+---------------------------v-------------------------------+
| Application (src/application)                             |
| compilation workflow, lexer, parser, compiler, builder    |
| domain objects, ports (source, kernel, ROM, symbols)      |
+---------------------------+-------------------------------+
                            |
+---------------------------v-------------------------------+
| Domain (src/domain)                                       |
| lexemes, actions/tags, symbols, kernel models, options    |
+---------------------------+-------------------------------+
                            ^
+---------------------------+-------------------------------+
| Infrastructure (src/infrastructure)                       |
| file adapters, generated kernel image, Z80 opcode writer, |
| compression                                               |
+-----------------------------------------------------------+
```

Important rules enforced by `make test-architecture`:

- `domain` must not depend on `application`, `cli`, or `infrastructure`.
- `application` must not depend on `cli` or concrete filesystem/generated
  kernel adapters.
- Application behavior receives collaborators through constructors or
  `CompilationRequest`; do not add global providers or service locators.
- Headless unit tests use in-memory adapters and must not create temporary
  `.bas` files.

## 3. Source tree map

| Path | Responsibility |
| --- | --- |
| `src/cli/main.cpp` | Process entry point; delegates to `runCli`. |
| `src/cli/runner/runner.cpp` | Production composition root: builds file adapters, parses options, calls `CompilationUseCase`. |
| `src/cli/options/` | CLI option setup and validation. |
| `src/cli/parser/` | Command-line parser. |
| `src/application/compilation/` | End-to-end compilation workflow and request/result objects. |
| `src/application/lexer/` | Line-by-line lexer, state machine, and lexer helpers. |
| `src/application/parser/` | Syntax parsing, action tree construction, statement parsing strategies. |
| `src/application/compiler/` | Semantic analysis, Z80 emission, expression/function evaluation, symbols, fixups. |
| `src/application/builder/` | ROM layout, kernel module packing, resource loading, ROM artifact assembly. |
| `src/application/kernel/` | Kernel image port and in-memory test implementation. |
| `src/application/source/` | Source resolver port and source document model. |
| `src/application/symbols/` | Symbol export strategies. |
| `src/domain/lexer/` | Lexeme types and keyword/function sets. |
| `src/domain/parser/` | `ActionNode`, `TagNode`. |
| `src/domain/symbols/` | Symbol, fixup, code-node, and FOR/NEXT models. |
| `src/domain/kernel/` | Kernel module model, central table, dispatch fixup model, size map. |
| `src/domain/options/` | `BuildOptions` used across application and CLI. |
| `src/infrastructure/fs/` | File-backed source, resource, symbol, and ROM adapters. |
| `src/infrastructure/kernel/` | Generated kernel image, module library, Z80 opcode writer. |
| `src/infrastructure/kernel/asm/` | Z80 kernel sources and the assembly-to-header build. |
| `src/infrastructure/compression/` | Pletter compression support. |

## 4. End-to-end compilation flow

```
main()
  -> runCli()                              [src/cli/runner/runner.cpp]
       -> BuildOptionsSetup::parse()
       -> construct file adapters
       -> CompilationRequest
       -> CompilationUseCase::execute()    [src/application/compilation]
            -> Lexer::load() / evaluate()
            -> Parser::evaluate()
            -> Compiler::build()
            -> Rom::build()
                 -> KernelPacker::pack()
                 -> KernelComposer::compose()
                 -> patch mapper / start code
                 -> add compiled code + overflow modules + resources
                 -> RomArtifact
            -> RomWriter::write()
       -> print diagnostics and exit status
```

### 4.1 CLI and composition

`runCli` is the only place where production adapters are constructed:

- `FileSourceResolver`
- `GeneratedKernelImage`
- `GeneratedKernelModuleLibrary`
- `FileResourceContentSource`
- `FileSymbolOutputSink`
- `FileRomWriter`

They are passed to `CompilationUseCase` through `CompilationRequest`. This keeps
the application layer independent of the filesystem and of the generated kernel
arrays.

### 4.2 Lexer

`Lexer` reads source lines through `SourceResolver`. `LexerLineEvaluator` runs a
state machine over each line; states under
`src/application/lexer/states/implementations/` classify identifiers,
keywords, literals, operators, separators, and comments.

Keyword and function recognition ultimately comes from the sets in
`src/domain/lexer/lexeme.cpp`:

- `keywords`: names recognized as BASIC keywords/statements.
- `booleanOps`: operator keywords such as `AND`, `OR`, `XOR`.
- `functions`: names recognized as functions in expressions.

The output is a list of `Lexeme` objects with type/subtype/value metadata.

### 4.3 Parser

`Parser` walks the lexer lines and builds:

- one `TagNode` per numbered line (or per directive/resource marker);
- an `ActionNode` tree for statements, functions, and their parameters.

`ParserLineEvaluator` splits statements on `:` and dispatches each phrase.
Special syntax is handled by parser statement strategies in
`src/application/parser/statements/strategies/`. Simple statements can reuse the
generic strategy. The parser also records flags used later by the compiler,
such as file support, traps, `DATA`/`IDATA`, fonts, MTF, Arkos, and resources.

### 4.4 Compiler

`Compiler::build` performs semantic analysis:

1. Registers the `END` routine and the start-of-program stub.
2. For every tag, compiles its actions and records the emitted code range.
3. Registers the end-of-program stub and support routines.
4. Adds `DATA`/`IDATA` resources when requested by the parser.
5. Resolves symbols and fixups.

Statement dispatch is keyword-based through
`CompilerStatementStrategyFactory`; expression functions use
`CompilerFunctionStrategyFactory`. Both are Strategy + Factory implementations.

Code emission goes through `ICpuOpcodeWriter` (`Z80OpcodeWriter` in production).
Compiler strategies may:

- emit raw instructions with `cpu.add*()` methods;
- call stable BIOS addresses with `cpu.addCall(def_*)` constants from
  `compiler_hooks.h`;
- call a kernel routine through
  `optimizer.addKernelCall(DISP_*)` / `addKernelCallNZ(DISP_*)`.

`addKernelCall` is important: it records the used dispatch index, emits a
3-byte call placeholder, and stores a `DispatchFixup`. The builder later
patches that placeholder to the final central-table stub. Extracted strategies
must never embed a hardcoded kernel-internal address.

### 4.5 ROM builder

`Rom::build` assembles the final image:

| Step | What it does |
| --- | --- |
| `addKernel` | Packs used kernel modules, composes the fixed `0x4000-0x7FFF` page, reserves the central table, patches dispatch fixups, adds the start page, and applies mapper patches. |
| `addCompiledCode` | Calls `Compiler::write` to relocate compiled code and resolve line/symbol/segment fixups, then copies code into ROM pages after the start code. |
| `addOverflowModules` | Relocates kernel modules that did not fit the kernel page, applies cross-module imports, and places them in the Plain ROM user page or MegaROM segments. |
| `addResources` | Builds the resource map, calculates the resource segment/address, and writes those values into the start page. |
| `assembleArtifact` | Produces the in-memory `RomArtifact` (pages, sizes, shares, diagnostics). |
| `writer->write` | Persists the ROM through `RomWriter`. |

For Plain ROM the kernel resides in `0x4000-0x7FFF` and start code plus user code
in `0x8000-0xBFFF`. For MegaROM, user code and overflow modules are distributed
across 8 KB-pair segments, and resource data follows them.

If a Plain ROM overflows and auto mode is enabled, `CompilationUseCase` retries
as ASCII8 MegaROM.

### 4.6 Resources

`ResourceManager` and `ResourceFactory` load resources from the
`ResourceContentSource` port. Supported inputs include text, CSV, SCREEN, SPR,
Arkos, MTF variants, and raw blobs. Resources are packed (for example with
Pletter), packed into pages, and described by a resource map. The builder writes
the map address and segment into fixed offsets in the start page.

## 5. Z80 kernel architecture

The Z80 kernel is infrastructure, but its layout is central to the compiler.

### 5.1 Assembly build flow

```
src/infrastructure/kernel/asm/src/header.asm
  -> pasmo -> bin/header.bin
  -> xxd   -> src/infrastructure/kernel/header.h

src/infrastructure/kernel/asm/src/start.asm
  -> pasmo -> bin/start.bin
  -> xxd   -> src/infrastructure/kernel/start.h

bin/header.bin + header.symbols.asm
  -> scripts/generate_kernel_manifest.py
  -> kernel_manifest_generated.h
     kernel_dispatch_generated.h

bin/header.bin + manifest metadata
  -> scripts/generate_relocatable_modules.py
  -> kernel_relocatable_generated.h
```

Regenerate with:

```bash
make -C src/infrastructure/kernel/asm all
```

Required tools are `pasmo`, `xxd`, and Python 3. Generated headers under
`src/infrastructure/kernel/` are build artifacts; do not edit them by hand.

`header.asm` is an orchestrator. It includes domain modules from
`src/infrastructure/kernel/asm/src/header/`, for example:

- `20_runtime.asm`: virtual wrapper dispatch table and runtime entry points.
- `22_dispatcher.asm`: paged dispatcher for MegaROM overflow modules.
- `30_basic_helpers.asm`: BASIC helpers.
- `31-cmd*.asm`: CMD handlers grouped by domain.
- `32_usr.asm`: USR support.
- `33_player.asm`: Arkos player.
- `34_file_handling.asm`: disk/file I/O.
- `60_bios_helpers.asm`: BIOS wrappers.
- `61_megarom.asm`: MegaROM support and mapper patch points.
- `70_pletter.asm`: decompression.
- `80_resources.asm`: resource access.
- `90-*.asm`: XBASIC support routines (graphics, print, math, strings, logic).

### 5.2 Kernel image layout and dispatch

`header.bin` is an 0x8000-byte image. Its first 0x4000 bytes contain a
compile-time virtual dispatch table (`wrapper_routines_map_table`, assembled at
`org 0`); this half is an oracle for address resolution and is not written as
kernel code. The actual kernel code is linked at `0x4000-0x7FFF`, and
`start.bin` is the BASIC loader/start page.

At build time, `KernelPacker`:

1. Computes the used module set from the compiler's used `DISP_*` indices plus
   dependencies and required modules.
2. Places fixed modules and fixed regions first.
3. Packs flexible modules into free holes of the kernel page.
4. Places overflow modules in the Plain ROM user page or MegaROM segments.
5. Emits only the central-table stubs that are needed.

Each used dispatch entry gets one of two stub forms:

- resident: `jp routine` (3 bytes);
- paged MegaROM overflow: `call dispatcher; db segment; dw address` (6 bytes).

The paged dispatcher preserves the caller ABI and the current segment, switches
to the target segment, calls the routine, restores the segment, and returns.

Module placement policy comes from
`src/infrastructure/kernel/module-groups.json`:

- `fixed`: must stay resident at its assembled address;
- `pinned`: fixed and critical (for example traps);
- `flex-high` / `flex-low`: packed into kernel holes before overflow.

Module binaries, exports, imports, relocations, dependencies, and patch points
are embedded through the generated manifests. That is why BASIC compilation
never needs `pasmo` or external files.

### 5.3 Start code

`start.asm` initializes the MSX environment, selects the ROM, initializes BASIC,
and then enters a small BASIC stub that executes compiled code through
`DEFUSR9`/`USR9`. The builder patches its file-I/O flag, resource map address,
and resource map segment. It also patches the single call to the game-graphics
kernel entry (`C7369`) when that module is movable.

### 5.4 Mapper patching

For MegaROM modes the builder patches kernel instructions located through
dispatch entries:

- **ASCII8**: segment-switch addresses `0x6800/0x7000/0x7800`.
- **Konami4 / KonamiSCC**: `0x7000/0x8000/0xA000`.
- **ASCII16 / ASCII16-X**: `MR_CHANGE_SGM` is rewritten to convert 8 KB-pair
  segment numbers to 16 KB pages, plus the required boot and emulator-detection
  patches. ASCII16-X also writes its signature and disables the ASCII8 `AB`
  detection check.

## 6. Adding a new MSX-BASIC statement or command

There are two related paths:

- **Statement** (for example `PSET`, `SCREEN`, `SET`): a top-level BASIC
  keyword mapped by the compiler statement factory.
- **CMD subcommand** (for example `CMD CLRSCR`): parsed under `CMD` and mapped
  by the CMD handler factory.

The steps below cover a normal statement. The CMD variant is noted at the end.

### Step 1: Register the keyword

Edit `src/domain/lexer/lexeme.cpp` and add the uppercase name to the
`keywords` set. If the word must also be recognized inside expressions as a
function, add it to `functions` as well. Add boolean operators only to
`booleanOps`.

Without this step the lexer treats the word as an identifier or unknown token.

### Step 2: Add parser support

Every statement keyword needs a parser strategy. In
`src/application/parser/statements/factory/parser_statement_strategy_factory.cpp`:

- reuse `genericStrategy` when the arguments are ordinary expressions;
- reuse an existing strategy when the syntax matches;
- otherwise create a new strategy under
  `src/application/parser/statements/strategies/<name>/`.

A parser strategy implements `IParserStatementStrategy::execute` and builds
`ActionNode`s for the compiler. Add its header to the parser factory header and
an instance member in `ParserStatementStrategyFactory`.

If the parser must remember a feature (file I/O, traps, DATA, fonts, etc.),
set the corresponding `ParserContext` flag.

### Step 3: Add a compiler statement strategy

Create a class under
`src/application/compiler/statements/strategies/<domain>/<name>/`:

- header named `compiler_<name>_statement_strategy.h`;
- implementation `compiler_<name>_statement_strategy.cpp`;
- inherit `ICompilerStatementStrategy`;
- implement `bool execute(shared_ptr<CompilerContext> context)`;

Then:

1. Include the header in
   `src/application/compiler/statements/factory/compiler_statement_strategy_factory.h`.
2. Add an instance member to the factory.
3. Register the keyword in the factory constructor in
   `compiler_statement_strategy_factory.cpp`.

The strategy reads `context->getCurrentAction()` and emits code. Use the
expression evaluator for parameters, for example
`context->getExpressionEvaluator()->evalExpression(action)`.

Good reference implementations:

- simple statement: `compiler_beep_statement_strategy.cpp`;
- graphics with parameters and kernel calls:
  `compiler_pset_statement_strategy.cpp`;
- statement with subcommands: `compiler_set_statement_strategy.cpp`.
- `CMD` forwarding: `compiler_cmd_statement_strategy.cpp`.

### Step 4: Emit code

Use `cpu.add*()` for inline Z80 and `optimizer.addKernelCall(DISP_*)` for kernel
routines. Record any parser feature flags required for support code.

If the statement needs a new kernel routine, continue with Section 8. If it is
implemented entirely with inline instructions or existing dispatch entries,
skip the kernel work.

### Step 5: Add tests and specifications

- Add a headless unit test under `tests/unit/src/` using `CompilationHarness`.
- Add or extend an integration `.bas` under `tests/integration/` when runtime
  behavior matters.
- Follow the project's OpenSpec workflow: add or update the relevant capability
  spec in `openspec/specs/` or an active change in `openspec/changes/`.

Run `make test-unit`, `make test-integration`, and `make test-architecture`.

### CMD subcommand variant

If the new command is invoked as `CMD <NAME>`:

1. Add the name to `keywords` in `lexeme.cpp` so it tokenizes correctly.
2. Add a handler under
   `src/application/compiler/statements/strategies/io/cmd/handlers/<domain>/<name>/`
   implementing `ICompilerCmdHandler`.
3. Register the handler in `compiler_cmd_handler_factory.cpp` and its header.
4. The handler usually emits `optimizer.addKernelCall(DISP_cmd_<name>)`.

Existing handlers such as `compiler_cmd_clrscr_handler.cpp` are minimal
examples.

## 7. Adding a new MSX-BASIC function

Functions are evaluated inside expressions and must return a lexeme subtype.

### Step 1: Register as a function

In `src/domain/lexer/lexeme.cpp`:

- add the name to `keywords`;
- add the name to `functions`.

The second set is what marks the token as `subtype_function` so the parser treats
it as part of an expression.

### Step 2: Implement the function strategy

Create a class under
`src/application/compiler/functions/strategies/<domain>/<name>/`:

- inherit `ICompilerFunctionStrategy`;
- implement:

```cpp
int execute(shared_ptr<CompilerContext> context,
            shared_ptr<ActionNode> action,
            int* result,
            unsigned int parmCount);
```

`result[0..parmCount-1]` contains the subtypes of the evaluated arguments.
Validate `parmCount` and the argument subtypes. Emit instructions or call
existing helpers. Return the result subtype (`Lexeme::subtype_numeric`,
`subtype_string`, `subtype_single_decimal`, etc.), or
`Lexeme::subtype_unknown` on an unsupported form.

Then include the header in
`compiler_function_strategy_factory.h`, add the instance member, and register
the keyword in `compiler_function_strategy_factory.cpp`.

Good references:

- `compiler_abs_function_strategy.cpp` (integer/float handling);
- `compiler_len_function_strategy.cpp` (string argument and inline result);
- `compiler_sin_function_strategy.cpp` (kernel dispatch call).

### Step 3: Add tests

Unit tests should compile a small BASIC program that uses the function and
assert compilation success and, where practical, generated code size/opcodes.
Integration tests should run the resulting ROM when runtime behavior matters.

## 8. Adding a kernel routine and dispatch entry

Whenever a compiler strategy should call a new Z80 routine through the dispatch
mechanism, all of these pieces must stay aligned.

### Step 1: Implement the routine in assembly

Add the label and implementation to the appropriate module under
`src/infrastructure/kernel/asm/src/header/`. Prefer an existing domain module
over creating a new one. If you create a new module:

- include it from `header.asm`;
- add its classification to
  `src/infrastructure/kernel/module-groups.json`;
- make sure cross-module calls go through exported labels and are fixup-safe
  (the manifest generator rejects direct references to movable modules and
  cross-module `jr`).

### Step 2: Add it to the dispatch table

In `src/infrastructure/kernel/asm/src/header/20_runtime.asm`, add:

```asm
  dw your_routine
```

inside `wrapper_routines_map_table`, immediately before the closing
`DEFS 0x4000 - $` line. The order in this table defines the dispatch index.
Append new entries at the end so existing indices do not move.

### Step 3: Add the matching C++ dispatch constant

In `src/application/compiler/helpers/hooks/compiler_hooks.h`:

```cpp
#define DISP_your_routine <next index>
#define DISP_ENTRIES <previous count + 1>
```

The index must match the position in `wrapper_routines_map_table`. The constant
name itself does not have to match the assembly label, but keeping them similar
avoids confusion.

### Step 4: Regenerate the kernel artifacts

```bash
make -C src/infrastructure/kernel/asm all
```

This rebuilds `header.h`, `start.h`, `kernel_manifest_generated.h`,
`kernel_dispatch_generated.h`, and `kernel_relocatable_generated.h`.

### Step 5: Call it from the compiler

```cpp
context->getCodeOptimizer()->addKernelCall(DISP_your_routine);
```

Never call a kernel-internal routine with a hardcoded address or plain
`cpu.addCall(0x....)`. Stable BIOS entry points are the exception; use named
`def_*` constants from `compiler_hooks.h`.

### Step 6: Validate

Run:

```bash
make test-kernel
make test-emulator   # optional, requires openMSX
make test-unit
make test-integration
make test-regression # optional, requires openMSX
```

`make test-kernel` checks binary integrity, generated kernel manifest data,
and relocation data. `make test-emulator` builds the release binary and runs
the openMSX-dependent checks: dispatcher behavior, Plain ROM overflow,
MegaROM overflow/resources, and page-safe module rules. The emulator checks
skip when openMSX is not available.

## 9. Locating and changing existing statements/functions

Use this path map:

| What you want to change | Where to look |
| --- | --- |
| Is a word a keyword or function? | `src/domain/lexer/lexeme.cpp` (`keywords`, `functions`, `booleanOps`) |
| How a statement is parsed | `src/application/parser/statements/factory/parser_statement_strategy_factory.cpp` and `.../strategies/` |
| How a statement emits Z80 | `src/application/compiler/statements/factory/compiler_statement_strategy_factory.cpp` and `.../strategies/` |
| How a function emits Z80 | `src/application/compiler/functions/factory/compiler_function_strategy_factory.cpp` and `.../strategies/` |
| Kernel dispatch index | `src/application/compiler/helpers/hooks/compiler_hooks.h` (`DISP_*`) |
| Dispatch table order | `src/infrastructure/kernel/asm/src/header/20_runtime.asm` |
| Z80 routine body | `src/infrastructure/kernel/asm/src/header/*.asm` |
| Module residency/priority | `src/infrastructure/kernel/module-groups.json` |
| Mapper patch points | `src/infrastructure/kernel/asm/src/header/61_megarom.asm` and `src/application/builder/rom.cpp` |
| `CMD` subcommands | `src/application/compiler/statements/strategies/io/cmd/` |
| ROM page layout | `src/application/builder/rom.cpp`, `kernel_packer.cpp` |
| CLI flags and output paths | `src/cli/options/`, `src/cli/parser/`, `src/domain/options/build_options.h` |

Useful search examples:

```bash
grep -R "PSET" src/domain src/application | sort
grep -R "DISP_XBASIC_PSET" src | sort
grep -R "compiler_pset_statement_strategy" src | sort
grep -n "PSET" src/infrastructure/kernel/asm/src/header/*.asm
```

When changing existing behavior, change the source strategy and the matching
spec/tests together. Do not patch generated headers under
`src/infrastructure/kernel/`.

## 10. Build, test, and quality commands

```bash
make debug                  # debug binary -> bin/Debug/msxbas2rom
make release                # optimized binary -> bin/Release/msxbas2rom
make test-unit              # headless unit tests (in-memory adapters)
make test-architecture      # dependency/boundary checks and test inventory
make test-kernel            # Z80 kernel binary/manifest/relocation checks
make test-emulator          # openMSX runtime checks (needs release and openMSX)
make test-integration       # compiles integration BASIC programs (needs release)
make test-regression        # runtime RAM/VRAM regression tests (see 10.1; needs openMSX)
make quality-gate           # lint + architecture + coverage + kernel + integration
```

To regenerate the kernel after ASM changes:

```bash
make -C src/infrastructure/kernel/asm all
```

Then rebuild the C++ application, because the generated `header.h`,
`start.h`, and manifest headers are compiled into it.

### 10.1 Runtime regression tests (`make test-regression`)

`make test-regression` runs the local, memory-first regression workflow for
compiled MSX-BASIC programs. Unlike `make test-integration`, which only compiles
the integration corpus, this target executes selected ROMs in openMSX and
validates their runtime state instead of relying on screenshots.

The root target is equivalent to:

```bash
make -C tests/integration regression
```

It is intentionally local-only. It is not part of `make quality-gate`, and no CI
workflow invokes it.

Requirements:

- a built release compiler (`make release`);
- openMSX available as `openmsx`;
- a graphical X session;
- for joystick input scripts, `python-xlib` (the skill uses XTEST to drive
  openMSX's `keyjoystick1`).

#### Commands

| Command | Purpose |
| --- | --- |
| `make test-regression` | Run every discovered scenario and program. |
| `make -C tests/integration regression SCENARIO=INPUT` | Run only one feature folder. |
| `make -C tests/integration regression SCENARIO=INPUT PROGRAM=test8` | Run one program. |
| `make -C tests/integration regression PROGRAM=test8` | Run a program by name across scenarios. |
| `make -C tests/integration regression-list` | List scenarios/programs without compiling or launching openMSX. |
| `make -C tests/integration regression-baseline` | Generate or refresh baselines for all selected programs. |
| `make -C tests/integration regression-baseline SCENARIO=INPUT PROGRAM=test8` | Refresh only the selected baseline. |

#### Workflow

1. Build the compiler and generate baselines from a known-good build with
   `make -C tests/integration regression-baseline`.
2. Inspect the generated baselines under
   `tests/integration/regression/baselines/` before treating them as a gate.
3. Run `make test-regression` after code changes. A failure names the program,
   memory kind, and the differing address/range or VRAM region.
4. When a change intentionally alters runtime behavior, refresh only the
   affected baseline (`SCENARIO=`/`PROGRAM=`) and review the diff.
5. If a program is inherently timing-dependent, mark it `"volatile": true` in
   the manifest and keep explicit RAM probes for the behavior that must remain
   stable.

#### How it works

The workflow lives in `tests/integration/regression/`:

| Path | Responsibility |
| --- | --- |
| `runner.py` | Scenario discovery, compilation, capture orchestration, validation, baselines, and reports. |
| `manifest.json` | Per-scenario/per-program overrides: machine, fallback time, input scripts, RAM probes, VRAM checks, volatile ranges, and volatile programs. |
| `manifest.schema.json` | JSON Schema for the manifest. |
| `baselines/` | Versioned reference data (stable RAM masks/values and stable VRAM region hashes). |
| `artifacts/` | Gitignored per-run captures, logs, and reports. |
| `test_runner.py` | Unit tests for the validation helpers. |

Scenarios are the existing feature folders (`PRINT`, `GRAPH`, `INPUT`, ...).
Programs are `.bas` files discovered recursively inside each scenario. The runner
compiles each selected program in ASCII8 mode with `-s --noi` into an isolated
output directory under `artifacts/`; the normal integration compile matrix is not
used.

Machine selection defaults to the Sharp HB-8000 (MSX1). Programs whose source
uses `SCREEN 5` or higher run on the configured MSX2 machine
(`Panasonic_FS-A1WX`). A manifest override can pin a different machine.

Execution reuses the `msx-memory-debug` skill:

- breakpoints at `END_PGM` and `END_STMT + 2` capture terminating programs at
  their real terminal state;
- non-terminating programs use a frame-aligned fallback after the configured
  emulated-time delay (default 10 seconds in the manifest);
- manifest input scripts inject keyboard and joystick events;
- the skill writes RAM (`0xC000-0xFFFF`), physical VRAM, VDP registers, and an
  optional screenshot beside the ROM.

Validation is memory-first and evaluated in this order:

1. explicit RAM probes (`ram_probes`);
2. stable RAM snapshot comparison, masking configured volatile ranges (the
   default masks the BIOS work area/stack at `0xF000-0xFFFF`);
3. VRAM text extraction for text modes or VRAM region hashes for graphical
   modes;
4. screenshots only as a visual fallback when no memory check exists.

Baselines are generated by capturing each selected program multiple times
(default three) and storing only bytes and region hashes that are identical
across those captures. VRAM regions that vary across runs are recorded as
unstable and skipped. Programs that use `RND`, `TIME`, `TIME$`, `DATE$`, or
`INTERVAL` are marked volatile automatically; other timing-dependent programs are
marked explicitly in `manifest.json`. Volatile programs skip full snapshots but
still evaluate their configured probes and expected text.

Each run writes:

- `artifacts/runs/<run-id>/report.txt` and `report.json` — summary and per-program
  pass/fail causes;
- `artifacts/runs/<run-id>/<scenario>/<program>/` — per-program build, capture,
  emulator log, and optional screenshot;
- `artifacts/runs/latest` — symlink to the most recent run.

To capture diagnostic screenshots for programs that normally use memory checks,
run the runner directly with `--screenshots`:

```bash
python3 tests/integration/regression/runner.py run --scenario GRAPH --program globe --screenshots
```

Joystick input requires focus on the openMSX window. The skill releases any
scripted keys before exiting and clears residual keys before each program, so one
interactive test cannot contaminate later programs.

## 11. Practical conventions and pitfalls

- **Keep dispatch indices stable.** Append new `DISP_*` entries; do not
  renumber existing ones. The order in `20_runtime.asm`, the constants in
  `compiler_hooks.h`, and the generated dispatch header must agree.
- **Use `addKernelCall`, not absolute addresses.** It enables module packing,
  central-table stub generation, overflow placement, and dispatch fixups.
- **BIOS calls are different.** External BIOS entry points use named `def_*`
  constants and `cpu.addCall(def_*)`.
- **Do not edit generated files.** Regenerate `header.h`, `start.h`, and the
  `kernel_*_generated.h` files from the ASM build.
- **Respect module placement policies.** Resource-owning, player, trap, and
  interrupt-critical modules are fixed-resident. Forcing them to overflow must
  fail.
- **Keep generated code C++11-compatible.** The project builds as C++11 on
  Linux, Windows, and macOS.
- **Follow the repository conventions.** No `friend`, no hardcoded magic
  addresses in compiler calls, no global service providers, and no temporary
  `.bas` files in headless unit tests.
- **Update specs and tests.** This is a spec-driven project; consult
  `openspec/specs/architecture/spec.md` before architectural changes.
