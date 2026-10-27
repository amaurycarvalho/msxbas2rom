# Changelog Archive

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
with a MAJOR.MINOR.PATCH.BUILD scheme.

## [1.4.0.0] - 2026-10-26

### [2026-10-23-expand-kernel-central-jump-table](openspec/changes/archive/2026-10-23-expand-kernel-central-jump-table) Introduce a runtime central jump table and relocatable module packer for the kernel

#### Added
- Add a runtime central jump table in the fixed kernel page, with `jp` stubs for resident entries and `{segment, address}` dispatcher stubs for paged entries
- Add a module manifest (embedded in `msxbas2rom`) describing module ids, exports, imports, dependencies, placement policy, fixed regions, and patch points
- Add embedded relocation metadata for pre-assembled modules so BASIC compilation never invokes an external assembler, linker, or data file
- Add cross-module import fixups for calls and address-of pointers so fixed and relocatable modules never embed a movable module's address
- Add a build-time packer that reserves boot/dispatcher/traps/player regions, computes the used-module closure, fills kernel free holes first, and sends only overflow to the user page or MegaROM segments
- Add unit, kernel-regression, and emulator tests for dispatching, nested calls, interrupts, traps, relocations, and every supported mapper

#### Changed
- Enforce module isolation and page safety: modules that map resource data into `0x8000` while executing stay fixed, and forced overflow of a fixed module fails with a diagnostic
- Track the used `DISP_*` set during compilation and defer kernel call resolution to dispatch fixups, with the packer generating selective stubs and patching call sites
- Classify routines into fixed resident, flex high priority, flex low priority, and overflow
- Keep player/Arkos fixed-resident at `0x6481-0x6BD5` and keep the timer interrupt handler resident
- Split `90_support.asm` into graphics, math, strings, print, logic, and trap subgroups, pinning trap/interrupt-critical routines resident
- Keep `DISP_*` indices stable while replacing direct absolute kernel calls with stubs/fixups
- Update Plain ROM and MegaROM placement, budget validation, and diagnostics
- Internal breaking change: kernel routines may move out of the fixed page, flex-routine addresses in `header.symbols.asm` are no longer build constants, and `header.bin` is no longer the sole runtime kernel image

### [2026-10-26-accelerate-test-emulator](openspec/changes/archive/2026-10-26-accelerate-test-emulator) Accelerate openMSX regression captures with pre-boot breakpoints and conditional checks

#### Added
- Add a shared helper for `tests/dispatcher/` runtime checks that runs openMSX accelerated when no host input script is present, installs pre-boot breakpoints, and preserves the emulated-time delay as a fallback
- Add an explicit acceleration option to the `msx-memory-debug` skill
- Add optional breakpoint conditions (`LABEL=ADDRESS@CONDITION`) so MegaROM checks can require the correct mapper block before capturing

#### Changed
- Compile BASIC runtime-check programs with `-s --noi`, resolve `LIN_<line>` from `.noi` symbols, mask addresses with `& 0xFFFF`, and derive the mapper block from the symbol's segment
- Install the terminal-state breakpoint on the first line of the marker loop after the program has written its RAM marker
- Export and parse the pasmo `loop` symbol for the hand-written dispatcher prototype
- Preserve every existing check, marker address, machine selection, headless SDL behavior, and pass/fail result
- Keep `make test-regression`, product runtime behavior, and parallel emulator execution unchanged

### [2026-10-26-add-emulator-memory-regression-tests](openspec/changes/archive/2026-10-26-add-emulator-memory-regression-tests) Add an openMSX-based memory regression workflow for integration test programs

#### Added
- Add a `regression` Make target in `tests/integration/Makefile`, independent from the existing `run`/`test-integration` flow
- Add a regression runner and manifest under `tests/integration/regression/`
- Group programs into scenarios by feature folder and allow running a whole scenario or a single program
- Compile selected programs in ASCII8 mode by default, with `--noi` symbols and a dedicated output directory
- Execute ROMs in openMSX with deterministic capture: `END_PGM`/`END_STMT + 2` breakpoints, configurable emulated-time fallback, and automated keyboard/joystick input
- Validate RAM probes and stable RAM snapshots, VRAM text/name-table checks, and VRAM region hashes, using screenshots only as diagnostics
- Add baseline generation/refresh with stable-byte masking and volatile-program handling

#### Changed
- Extend the `msx-memory-debug` skill with VRAM capture, pre-boot breakpoints, input injection, and fallback capture
- Keep the regression workflow out of CI/CD

### [2026-10-26-add-regression-vram-probes](openspec/changes/archive/2026-10-26-add-regression-vram-probes) Add declarative VRAM probes to the regression manifest

#### Added
- Add `vram.probes` to the regression manifest schema and runner
- Add unit tests and document the manifest field

#### Changed
- Let each probe declare an absolute VRAM offset and expected `value` or `bytes`
- Evaluate VRAM probes before the volatile short-circuit so volatile programs can still assert exact VRAM state
- Report probe failures with the VRAM offset and expected/actual bytes

### [2026-10-26-add-turbo-r-regression-coverage](openspec/changes/archive/2026-10-26-add-turbo-r-regression-coverage) Add turbo R regression coverage and machine requirements

#### Added
- Add stable text expectations for `MSX()` and turbo output labels, marking timing-dependent values volatile
- Add a turbo R gate that verifies `MSX() = 3` and that `CMD TURBO` continues

#### Changed
- Mark the turbo R feature programs (`MISC/test1`, `MISC/test64`, `MISC/test65`) with `requires: turboR` so they run on `Panasonic_FS-A1GT`
- Keep performance-only users of `CMD TURBO`/`CALL TURBO` on MSX2+ unless they declare a turbo R requirement

### [2026-10-26-fix-put-sprite-or-color](openspec/changes/archive/2026-10-26-fix-put-sprite-or-color) Preserve MSX2 sprite color bits in PUT SPRITE

#### Added
- Add `tests/integration/GRAPH/sprite3.bas` to the regression workflow with a VRAM probe for sprite 1's 16 color-table bytes at `0x6C`

#### Changed
- Preserve bits 5 and 6 of the PUT SPRITE color parameter when writing the sprite color table on MSX2 and later
- Keep the MSX1 path limited to the 4-bit sprite color
- Generate a `GRAPH/sprite3` baseline after the fix

### [2026-10-26-fix-regression-boot-timing](openspec/changes/archive/2026-10-26-fix-regression-boot-timing) Enforce a per-machine-class minimum boot delay for regression captures

#### Added
- Add a per-machine-class minimum boot delay for fallback captures

#### Changed
- Default MSX2+, MSX2, and turbo R to at least 15 emulated seconds
- Keep program-specific `fallback_seconds` as the capture delay but use `max(fallback_seconds, machine_boot_seconds)` for the effective fallback
- Allow the machine boot minimum to be overridden in the manifest
- Document the timing rule and add runner tests for the effective value

### [2026-10-26-fix-regression-end-pgm-capture](openspec/changes/archive/2026-10-26-fix-regression-end-pgm-capture) Capture terminal state at the pre-cleanup END entry

#### Added
- Add regression checks that a graphics-mode program captured at END reports the graphics mode and uses the correct VRAM region layout

#### Changed
- Capture terminal state at `END_STMT + 2` for both explicit and implicit program termination
- Use `END_PGM` only as a fallback when `END_STMT` is unavailable
- Report a clear diagnostic when the captured VDP mode conflicts with the source-declared screen mode
- Keep the source fallback only for SCREEN 9-12 where VDP registers cannot distinguish the modes

### [2026-10-26-fix-regression-machine-selection](openspec/changes/archive/2026-10-26-fix-regression-machine-selection) Select regression machines by class and VDP screen mode

#### Added
- Introduce machine classes `msx1`, `msx2`, `msx2plus`, and `turboR` with configurable defaults; MSX2/MSX2+ use `Panasonic_FS-A1WX`
- Add an explicit `requires` manifest field while keeping `machine` as the highest-precedence override
- Add unit tests for parsing, machine selection, and VDP mode decoding

#### Changed
- Parse `SCREEN` literals with or without whitespace
- Infer MSX2+ for `SCREEN >= 4` or turbo-capable commands and require an explicit `requires: turboR` for turbo R programs
- Decode V9938/V9958 screen modes from VDP registers, using the source's last SCREEN for SCREEN 9-12
- Fail a regression run when the baseline machine differs from the selected machine
- Expose each program's selected machine and class in the audit/list output and run report

### [2026-10-26-fix-turbo-function-runtime](openspec/changes/archive/2026-10-26-fix-turbo-function-runtime) Route TURBO(), VDP(), and MAKER() through kernel dispatch

#### Added
- Add runtime regressions proving `TURBO()`, `VDP()`, and `MAKER()` return numeric values on MSX2+ and turbo R without hanging

#### Changed
- Route `TURBO()`, `VDP()`, and `MAKER()` through the kernel dispatch mechanism instead of a direct absolute call to `USR2 + 1`
- Pass the function selector in `HL` so the `USR2` dispatcher receives the intended function number
- Keep MSX1 behavior: `TURBO()` returns 0 and `CMD TURBO` remains a no-op

### [2026-10-26-incremental-local-lint](openspec/changes/archive/2026-10-26-incremental-local-lint) Run local lint incrementally while keeping CI full-tree

#### Added
- Add `make lint-all` for a full local lint run

#### Changed
- Keep `make lint` full-tree when `CI=true` so the CI workflow is unchanged
- Locally, lint only changed `src/**/*.cpp` files (working tree versus `HEAD`, including untracked files) and exit successfully with guidance when no C++ source changed
- Point `make quality-gate` at `lint-all` so the local quality gate remains full
- Keep `make lint-full` (full tree with `-fanalyzer`) unchanged
- Document the local/CI split in the contributing guide and the build command reference

#### Removed
- Remove the automatic fallback to full lint when a header or the `Makefile` changes; header and build-flag validation becomes an explicit `make lint-all` or `make quality-gate`

### [2026-10-26-refresh-regression-baselines](openspec/changes/archive/2026-10-26-refresh-regression-baselines) Refresh regression baselines for the new machine classes and capture fixes

#### Added
- Add the new `GRAPH/sprite3` baseline after the OR-color fix

#### Changed
- Regenerate baselines for programs whose machine class changed to MSX2+ or turbo R
- Regenerate baselines whose captured screen mode or VRAM regions changed because of the V9958 decoder and END-capture fixes
- Raise `MEGAROM/test51` fallback time so its 500-line program reaches the pre-cleanup END entry before capture, then regenerate its baseline
- Review every regenerated baseline for selected machine, screen mode, and region/layout changes and reject unexplained diffs
- Run the full local regression and the machine audit

## [1.3.1.0] - 2026-10-21

### [improve-mutation-score](openspec/changes/archive/2026-10-22-improve-mutation-score) Raise the mutation score with exact-output, sentinel-diverse unit tests

#### Added
- Add exact-output and sentinel-diverse coverage for MegaROM relocation/layout, resource readers, compiler semantic helpers, statement strategies, parser, symbols, lexer, and builder code
- Add normalized Z80 generated-code fingerprints and kernel dispatch-layout integrity checks as non-scoring regression coverage
- Add exhaustive relocation-switch coverage, including never-emitted conditional `call`/`jp` variants

#### Changed
- Require kernel-derived expected values to be computed at test runtime and compiler/Z80 constants to be asserted literally
- Raise the Mull timeout to 45000 ms for the measured post-Round-6 baseline
- Strengthen the mutation-testing spec with fresh-report, sentinel-diverse, C++-scope, and kernel-safe requirements

### [isolate-unit-tests-from-file-io](openspec/changes/archive/2026-10-22-isolate-unit-tests-from-file-io) Move CLI E2E scenarios to integration and isolate file-adapter tests in OS temporary directories

#### Added
- Add a cross-platform `TestTempDir` RAII helper for OS temporary directories
- Add an integration CLI characterization script preserving normal, quiet, debug, symbol, missing-input, and ASCII8-retry assertions

#### Changed
- Move CLI characterization out of the unit/mutation binary into `tests/integration`
- Remove the compile-based `FS`, `GRAPH`, and `MEGAROM` suites from `tests/unit/src/test_fs.cpp`
- Convert explicit file-adapter unit tests to OS temporary directories with automatic cleanup
- Strengthen `check_test_inventory.py` to reject filesystem I/O outside adapter tests and project-relative `tmp/` paths
- Reduce the plain-ROM overflow fixture from 900 to 200 lines

### [refactor-clean-architecture-testability](openspec/changes/archive/2026-10-21-refactor-clean-architecture-testability) Refactor compilation boundaries for in-memory testability and enforce Clean Architecture dependency rules

#### Added
- Establish an automated Clean Architecture boundary check with a version-controlled, pre-approved exception list
- Introduce application ports and infrastructure adapters for BASIC source input, INCLUDE resolution, ROM/resource/symbol output, and kernel-image access

#### Changed
- Make lexer/parser/compiler workflows accept in-memory source content with a logical source name
- Keep `BuildOptions` as a domain value object by removing its dependency on filesystem utilities
- Separate pure ROM image assembly from ROM-file persistence
- Extract the CLI compile/retry orchestration into an application use case with structured results and diagnostics
- Replace public, all-purpose parser/compiler contexts with narrower collaborators and encapsulated state
- Preserve CLI behavior, ROM bytes, diagnostics, resource formats, and cross-platform C++11 support

### [replace-service-locators-with-dependency-injection](openspec/changes/archive/2026-10-21-replace-service-locators-with-dependency-injection) Replace global service-locator providers with explicit constructor and composition-root dependency injection

#### Added
- Introduce a headless compilation test harness backed by in-memory source, kernel, resource, symbol, and ROM adapters

#### Changed
- Require ports through constructor or parameter-object injection instead of resolving defaults inside application classes
- Concentrate production adapter construction in the CLI composition root
- Make `CompilationRequest` carry a complete, required set of compilation services
- Pass `ResourceContentSource` through the resource reader factory instead of setting it after construction
- Migrate ROM, symbol, resource, lexer/parser/compiler, and characterization tests to in-memory assertions
- Preserve public CLI behavior, diagnostics, ROM bytes, resource bytes, and symbol formats

#### Removed
- Remove the five global provider pairs: source resolver, kernel image, resource content source, symbol output sink, and ROM writer
- Remove the global test adapter setup and static test source registry

## [1.3.0.0] - 2026-09-14

### [2026-09-11-add-msx-memory-debug-skill](openspec/changes/archive/2026-09-11-add-msx-memory-debug-skill) Add an agent-driven openMSX memory/screenshot capture skill for deterministic post-run inspection of MSX ROMs

#### Added
- Register the `msx-memory-debug` capability and its implementation under `.opencode/skills/msx-memory-debug-skill/`
- Tcl capture script reads `Main RAM` at `0xC000..0xFFFF` (16384 bytes) with `debug read_block`, saves a PNG screenshot, and exits openMSX
- Configuration via `OPENMSX_EXECUTABLE`, `OPENMSX_ARGS`, `OPENMSX_FLATPAK_APP`, `MSX_DEBUG_DELAY`, `MSX_DEBUG_MACHINE`, `MSX_DEBUG_SETTINGS`, `MSX_DEBUG_SCREENSHOT_MODE`

#### Changed
- Isolate openMSX from the user's interactive `settings.xml` by generating a minimal settings file, passing it with `-setting`, and deleting it on exit, so the skill no longer crashes on host-specific bindings
- Select the Sharp HB-8000 (Hotbit) machine explicitly with `-machine Sharp_HB-8000_1.2`, preventing a C-BIOS fallback
- Document reusable debugging strategies: freeze observable state with `POKE` plus a halt loop, map addresses with `-s --noi` and `header.symbols.asm`, use openMSX breakpoints, and rebuild the embedded kernel header after kernel edits
- Debug MSX2-oriented BASIC commands on the Panasonic FS-A1WX machine; the MSX1 default remains Sharp HB-8000 1.2

### [add-sprite-hitbox-margins](openspec/changes/archive/2026-09-14-add-sprite-hitbox-margins) Add per-sprite configurable collision hitbox margins via SET SPRITE HITBOX

#### Added
- Add the `SET SPRITE HITBOX` BASIC command family with global and per-sprite `ON`/`OFF`/`AUTO` forms plus explicit `<sprite>, <left>, <top>, <right>, <bottom>` margins
- Add the `HITBOX_TABLE` runtime table (32 × 5 bytes) storing pre-normalized relative bounds (`X0`, `X1`, `Y0`, `Y1`) and a per-sprite enabled flag
- Add the single kernel routine `set_sprite_hitbox` with a mode byte implementing all command forms
- Add the `HITBOX` and `AUTO` lexer keywords

#### Changed
- Make `COLLISION()` combine each sprite's position from `SPRTBL` with its relative bounds from `HITBOX_TABLE`, keeping the public `COLLISION()`/`COLLISION(n)`/`COLLISION(n1,n2)` API unchanged
- Reset `HITBOX_TABLE` to the default full-rectangle state on sprite clear (`SUB_CLRSPR`), covering the SCREEN sprite-size path

## [1.2.3.0] - 2026-09-05

### [2026-08-12-improve-code-coverage](openspec/changes/archive/2026-08-12-improve-code-coverage) Raise unit-test line coverage to ≥ 85% across the codebase

#### Added
- Add unit tests for the lowest-coverage compiler statements (SET, ON, COPY, PUT, SCREEN, CIRCLE, LET, PAINT, PSET, COLOR, KEY)
- Expand semantic-helper coverage (expression evaluator, variable emitter, code helper, float converter)
- Add tests for resource readers with little or no coverage (AKM reader, MTF map reader)
- Cover untested paths in `compiler.cpp` and `rom.cpp`
- Add tests for the graphics statement parser (graphics, put, on, set, screen)
- Add tests for low-coverage function strategies (MID$, INSTR$, STRING$, USING, USR)
- Add smoke tests for previously 0%-coverage files (`compiler_time_statement`, `compiler_open_grp_statement`, `vscode_helper`)
- Expand Z80 kernel (`z80.cpp`) coverage on unexercised paths

### [2026-09-05-raise-mutation-score-to-85](openspec/changes/archive/2026-09-05-raise-mutation-score-to-85) Raise the mutation score to ≥ 85% by killing surviving mutants with new unit tests

#### Added
- Add unit tests exercising every function-strategy result subtype (numeric, single, double)
- Add omitted-argument (null subtype) and boundary tests for statement strategies
- Add both-side equality and boundary tests across parser, lexer, domain, CLI, and symbols export modules
- Add exact-size boundary tests for resource readers and `compiler.cpp`/`rom.cpp`

#### Changed
- Raise the mutation-score threshold from 80% to 85% in the mutation run and check script
- Lower the Mull per-mutant timeout from 60000 ms to 10000 ms

### [2026-09-05-vscode-helper-initialize-into](openspec/changes/archive/2026-09-05-vscode-helper-initialize-into) Add VSCode scaffolding into a caller-chosen directory and repair the CLI spec

#### Added
- Add `VSCodeHelper::initializeInto(path)` to scaffold `.vscode` (`launch.json`, `tasks.json`, `debug.tcl`) under a target directory, reporting failure without writes when `.vscode` already exists or cannot be created

#### Changed
- Make `initialize()` delegate to `initializeInto(".")`, keeping the `--vscode` CLI behavior unchanged
- Refactor the CWD-dependent `VSCodeHelper` smoke test into hermetic tests under `tests/unit/tmp/` (content markers, cleanup, already-initialized scenario)
- Repair `openspec/specs/cli/spec.md` into valid main-spec form and document `--vscode` plus other implemented-but-undescribed CLI flags

## [1.2.2.0] - 2026-08-12

### [2026-08-12-fix-make-lint-warnings](openspec/changes/archive/2026-08-12-fix-make-lint-warnings) Fix `make lint` gate failures and indeterminate `Logger::LogEntry` fields

#### Fixed
- `src/infrastructure/logging/logger.h` / `logger.cpp`: give `LogEntry` explicit default and `(LogLevel, const string&)` constructors so all six fields are deterministically initialized, eliminating the `-Wmissing-field-initializers` errors and the indeterminate-`dummy` bug

#### Changed
- `src/application/compiler/helpers/semantic/compiler_code_helper.h` / `.cpp`: change `addBasicChar(char)` to `addBasicChar(unsigned char)`, matching its actual semantics (raw byte emission into `addLdiHL(unsigned char)`) and removing the `-Woverflow` errors on `0xC0`/`0xC5` MSX-BASIC tokens

#### Removed
- `src/application/compiler/helpers/semantic/compiler_code_optimizer.cpp`: drop the always-true `address >= def_wrapper_routines_map_table` lower-bound comparison in `getKernelCallAddr()` (the table base is `0x0000`, and the outer guard already ensures `address < 0x4000`), removing the `-Wtype-limits` error

## [1.2.1.0] - 2026-06-29

### [2026-06-29-fix-windows-startup-unicode-argv](openspec/changes/archive/2026-06-29-fix-windows-startup-unicode-argv) Fix Windows startup Unicode argv to ensure CLI runs reliably

#### Fixed
- Make Windows startup argument handling fail-safe before CLI parsing begins
- Avoid dangerous allocation from `WideCharToMultiByte()` failure paths, especially `len - 1` when `len == 0`
- Ensure informational invocations such as no arguments, `-h`, `--help`, `-v`, and `--version` print normally on Windows

#### Changed
- Preserve UTF-8 command-line support for Windows filenames when Unicode conversion succeeds

#### Added
- Add focused tests or build/run validation for Windows startup behavior where the project tooling allows it

## [1.2.0.0] - 2026-06-25

### [2026-06-25-mtf-enhancements](openspec/changes/archive/2026-06-25-mtf-enhancements) Add MTF window-copy and extended parameter handling

#### Added
- Add MTF operation 2: window-copy (partial map copy with configurable source rectangle and screen destination — preserves tiles outside the window via LDIRMV+LDIRVM, only 2 VDP transfers regardless of window size)
- Add screen page parameter scaffolding to all map operations (0, 1, 2) — compiler accepts and stores the parameter, kernel uses placeholder (always page 0 at 0x1800)
- Add unit tests for all parameter combinations and an integration test (`tests/integration/MTF/mtf5.bas`)

#### Changed
- Migrate parameter passing from Z80 registers to RAM workarea block (PARM1) for all operations, enabling up to 9 parameters
- Unify map copy code path: operations 0 (relative) and 1 (absolute) delegate to the same `window_copy` kernel routine by setting full-screen window parameters, eliminating ~70 bytes of duplicate copy-loop code
- Full-width optimization: when screen_x=0 and width=32, skip LDIRMV (1 VDP transfer) — ensures ops 0/1 maintain current performance
- Extend compiler handler to accept 1–9 parameters (was 1–4)
- Use incremental builds — no forced `make clean`

## [1.1.0.0] - 2026-06-23

### [2026-06-23-add-ascii16-mapper](openspec/changes/archive/2026-06-23-add-ascii16-mapper) Add ASCII16 MegaROM mapper support

#### Added
- Add ASCII16 compile mode to the CompileMode enum
- Add CLI flags -6 and --ascii16 for ASCII16 MegaROM selection
- Add dispatch-table-guided kernel binary patching via new fixAscii16Mapper() method: rewrites MR_CHANGE_SGM with push af; srl a; ld (0x7000),a; pop af; ret (SeqReplace, 9 bytes), patches boot bugfix to single ld (0x7000),1 write (1 ByteReplace + 3 NOPs), patches 4th/5th OPENMSX autodetection writes to 0x77FF (2× SeqReplace, 3 bytes each). Total: 7 patch points.
- Add dispatch table entries for new kernel patch-point labels (ascii16_patch_bugfix_inc1, ascii16_patch_bugfix_nopseq)
- Add unit tests for ASCII16 CLI parsing and ROM building
- Add integration test coverage using existing MEGAROM test programs compiled with --ascii16

#### Changed
- The compiler is unchanged — the kernel converts 8KB segment numbers to 16KB page numbers via srl a at runtime
- resourceSegment stays at pages.size() * 2 (8KB-pair convention unchanged across all MegaROM modes)
- Update output filename suffix to [ASCII16]
- Update status messages and help text in CLI

### [2026-06-23-add-ascii16x-mapper](openspec/changes/archive/2026-06-23-add-ascii16x-mapper) Add ASCII16-X MegaROM mapper support and signature

#### Added
- Add ASCII16X compile mode to the CompileMode enum
- Add CLI flags -7 and --ascii16x for ASCII16-X MegaROM selection
- Write the ASCII16-X autodetection signature ASCII16X at ROM offset 0x0010 in fixAscii16Mapper() for ASCII16X mode only
- Add unit tests for ASCII16X CLI parsing, ROM building, and signature byte verification

#### Changed
- Reserve 8 bytes in the kernel ROM header (via ds 8 between db 'MSXB2R' and INIT1:) shifting the entry point from 0x4010 to 0x4018; the KERNEL_END_FILLER absorbs the size delta keeping BIN_HEADER_BIN_LEN at 32768
- Update the hardcoded "LOADER" kernel symbol address from "4010" to "4018" in symbol_export_context.cpp
- Extend fixAscii16Mapper() guard to also apply kernel patching for ASCII16X mode (7 identical patch points: 1 SeqReplace + 3 NOP + 1 ByteReplace + 2 OMSX SeqReplace)
- Update output filename suffix to [ASCII16X]
- Update status messages and help text in CLI
- Regenerate header.h and header.symbols.asm via make header
- The compiler's segment math requires zero changes — kernel-side srl a conversion handles all MegaROM modes uniformly (only a code size overflow guard was added to ICpuOpcodeWriter::addCodeByte())

### [2026-06-23-add-konami4-mapper](openspec/changes/archive/2026-06-23-add-konami4-mapper) Add Konami4 MegaROM mapper support

#### Added
- Add Konami4 compile mode to the CompileMode enum
- Add CLI flags -4 and --konami for Konami4 MegaROM selection
- Add unit tests for Konami4 CLI parsing and ROM building
- Add integration test coverage using existing MEGAROM test programs compiled with --konami

#### Changed
- Reuse existing kernel segment-switch patching (Konami4 uses 0x8000/0xA000 upper-bank switch addresses, patched via dispatch table)
- Rename fixIfKonamiSCC() to fixKonamiMapper() with updated method comment and error message text; extend condition to include Konami4
- Update output filename suffix to [Konami]
- Update status messages in main.cpp to distinguish Konami4 from KonamiSCC

### [2026-06-23-expand-megarom-limit-to-2mb](openspec/changes/archive/2026-06-23-expand-megarom-limit-to-2mb) Expand the MegaROM compile limit from 1 MB to 2 MB

#### Changed
- #define COMPILE_MAX_PAGES em compiler.h passa de (16 * 4) para (16 * 8), dobrando COMPILE_CODE_SIZE de 1MB para 2MB

### Added

- Add `Konami4`, `ASCII16`, and `ASCII16X` compile modes with CLI flags `-4`/`--konami`, `-6`/`--ascii16`, and `-7`/`--ascii16x` for MegaROM mapper selection
- Add dispatch-table-guided kernel binary patching via `fixAscii16Mapper()` method with 7 patch points for ASCII16/ASCII16X
- Write ASCII16-X autodetection signature (`ASCII16X`) at ROM offset 0x0010
- Add dispatch table entries for new kernel patch-point labels
- Add unit tests for Konami4, ASCII16, and ASCII16X CLI parsing, ROM building, and signature byte verification
- Add integration test coverage for Konami4 and ASCII16 modes using existing MEGAROM test programs

### Changed

- Expand MegaROM compile code size limit from 1MB to 2MB by increasing `COMPILE_MAX_PAGES` from 64 to 128 pages
- Rename `fixIfKonamiSCC()` to `fixKonamiMapper()` and extend to support both KonamiSCC and Konami4; extend `fixAscii16Mapper()` guard to also apply for `ASCII16X` mode
- Reuse existing kernel segment-switch patching for Konami4 (0x8000/0xA000 switch addresses via dispatch table)
- Reserve 8 bytes in kernel ROM header shifting INIT1 from 0x4010 to 0x4018 for ASCII16-X
- Update hardcoded LOADER symbol address from 0x4010 to 0x4018 in `symbol_export_context.cpp`
- Update output filename suffixes (`[Konami]`, `[ASCII16]`, `[ASCII16X]`) and CLI status/help messages
- Regenerate header artifacts (header.h, header.symbols.asm) for shifted kernel layout

## [1.0.1.0] - 2026-06-23

### [2026-06-22-kernel-call-indirection](openspec/changes/archive/2026-06-22-kernel-call-indirection) Route XBASIC kernel calls through indirect dispatch entries

#### Added
- Adicionar ~55 novas entradas dw na tabela wrapper_routines_map_table em 20_runtime.asm, cobrindo todas as rotinas XBASIC atualmente chamadas com endereço direto (math, string, graphics, traps, casts, I/O)
- Adicionar ~55 novas constantes DISP_XBASIC_* em compiler_hooks.h, estendendo a cadeia de índices até DISP_ENTRIES = ~181

#### Changed
- Substituir ~117 chamadas cpu.addCall(def_XBASIC_*) por optimizer.addKernelCall(DISP_XBASIC_*) em todos os arquivos C++ que emitem código para essas rotinas (expression evaluator, math/string/graphics functions, statements)
- Substituir ~12 magic numbers (0x009C, 0x009F, 0x00C0, etc.) por constantes nomeadas def_CHSNS, def_CHGET, etc. em compiler_hooks.h
- Substituir todas as chamadas cpu.addCall(def_XBASIC_*) remanescentes (já há entradas na tabela para algumas rotinas XBASIC como XBASIC_CLS, XBASIC_PLAY, XBASIC_PRINT_*, etc.) por optimizer.addKernelCall(DISP_XBASIC_*)

### [2026-06-22-optimize-kernel-space](openspec/changes/archive/2026-06-22-optimize-kernel-space) Optimize and shrink the embedded kernel implementation

#### Added
- Add kernel regression tests that verify ROM binary integrity after optimizations

#### Changed
- Consolidate 4 consecutive ld (ENDPRG+n),a zero-stores into a loop/ldir (20_runtime.asm)
- Compact player idle-state initialization across 4 occurrences using sequential writes (33_player.asm)
- Replace 126-entry jp jump table with word-pointer dispatch table (20_runtime.asm)
- Update compiler_hooks.h C-side dispatch offsets to match new wrapper table layout

#### Fixed
- Fix GICINI double-call bug in cmd_mute — calls GICINI twice when only once is needed (31_cmd.asm)
- Fix MegaROM mode restore bug in verify.slots.test.ram — writes wrong value to mapper port (60_bios_helpers.asm)
- Fix missing carry clear before sbc hl,de in cmd_fopen heap calculation (34_file_handling.asm)
- Fix hardcoded 255-byte copy in resource.get_data that ignores real resource size (80_resources.asm)

#### Removed
- Remove the fully commented-out BDOS fallback block (90 lines, ~250 bytes) from 34_file_handling.asm
- Remove dead inc de in XBASIC_INIT function key loop (30_basic_helpers.asm)
- Remove dead ld a, 2 in cmd_finput state machine (34_file_handling.asm)
- Remove redundant push de/pop de guard in cmd_preflight_disk (34_file_handling.asm)
- Remove duplicate ld a, (STARTUP_CFG_FILEIO) load in run_user_basic_code_on_rom (20_runtime.asm)
- Remove redundant xor a in resource.address (80_resources.asm)

### [2026-06-22-split-header-bin-virtual-table](openspec/changes/archive/2026-06-22-split-header-bin-virtual-table) Split header.bin into a virtual dispatch table and kernel image

#### Added
- Build size check updates: LIMIT=16384 → LIMIT=32768 for the full binary.

#### Changed
- header.bin doubles from 0x4000 to 0x8000 bytes: First half (0x0000-0x3FFF) becomes a virtual compile-time-only section containing the dispatch table + zero-fill padding. Second half (0x4000-0x7FFF) is the actual kernel ROM image.
- Dispatch table moves to logical address 0x0000: wrapper_routines_map_table uses org 0000h and DEFS 0x4000 - $ to pad to the second half boundary. No second org needed — the padding naturally lands at logical address 0x4000.
- C++ address resolution adapts: getKernelCallAddr() handles the 0x0000-0x3FFF range for table lookups (reading bin_header_bin[address] directly) and the 0x4000-0x7FFF range for kernel addresses (also using bin_header_bin[address] directly — no - 0x4000 offset needed).
- ROM builder writes only the second half: addKernel() copies bin_header_bin + 0x4000 instead of the full binary.
- 5 nops removed from 20_runtime.asm (they existed only to align the table to 0x4102).

### [2026-06-22-wrapper-dispatch-table](openspec/changes/archive/2026-06-22-wrapper-dispatch-table) Replace wrapper addresses with DISP_ dispatch indices

#### Changed
- Q1 resolution: All dispatch index constants in compiler_hooks.h use the DISP_ prefix (e.g., DISP_cmd_clrkey, DISP_castParamFloatInt) — BREAKING renaming from def_* address constants
- Q2 resolution: getKernelCallAddr() is updated to resolve targets through the dw dispatch table instead of checking for 0xC3 (jp) opcode bytes. All C-side wrapper calls go through a single helper (addKernelCall()) using the dispatch index and calculating the real call address at compile time instead of runtime. Also, it eliminates the need for a separate addKernelDispatch() emission method and a wrapper_routines_map_start routine (20_runtime.asm).
- Kernel size constraint: The final kernel ROM SHALL NOT exceed 0x4000 bytes; a build-time check alerts if this limit is breached and an unit test must be written to test it.
- def_wrapper_routines_map_start must be removed.

### [2026-06-22-wrapper-table-restructure](openspec/changes/archive/2026-06-22-wrapper-table-restructure) Replace the jp jump table with compact word-pointer dispatch

#### Changed
- Replace the 126-entry jp jump table in 20_runtime.asm with a dw word-pointer table and a small dispatcher routine (~5 bytes)
- Update compiler_hooks.h C-side #define constants to emit dispatch indexes instead of the +3 byte-offset chain
- Update compiler_code_optimizer.cpp to resolve targets through the new dispatch mechanism
- Update all cpu.addCall(def_xxx) call sites (~100+ files) across the C++ compiler to use a new addKernelDispatch() helper that emits the dispatch sequence

#### Removed
- Remove the 5-byte padding introduced by the prior optimization to reclaim those bytes

### [2026-06-23-disable-unused-90-support-code](openspec/changes/archive/2026-06-23-disable-unused-90-support-code) Disable unreachable 90_support.asm labels while preserving offsets

#### Changed
- Desativar 58 labels em 90_support.asm que não possuem caminho de execução a partir de nenhum entry point externo (C++ via def_* macros ou outros .asm)
- Substituir cada bloco desativado por DEFS <tamanho>, 0x00 para manter endereços absolutos dos labels subsequentes
- Blocos a desativar (agrupados por proximidade):
- NÃO alterar (conforme solicitado): BASIC_KUN_START_FILLER, ARKOS_PLAYER, I7EA8 (CHR$), I7EC3 (HEX$), I7EC6 (OCT$), I7EC9 (BIN$), I7EEF (SPACE$), BASIC_KUN_END_FILLER

### [2026-06-23-extend-kernel-optimizations](openspec/changes/archive/2026-06-23-extend-kernel-optimizations) Extend kernel micro-optimizations across runtime and command routines

#### Changed
- Collapse intCompareGT into a delegation to intCompareLT via operand swap (ex de,hl + jp), eliminating 9 bytes of duplicated xor-bias logic in 21_logic_pack.asm
- Collapse intCompareGE into a delegation to intCompareLE via operand swap, eliminating another 9 bytes in 21_logic_pack.asm
- Replace four consecutive sla a instructions with add a, a in cmd_setfnt.default_colors.1 (31_cmd.asm lines 498-502), saving 4 bytes and 16 cycles
- Replace ld (MV_DPTR),hl / ld hl,(MV_DPTR) with push hl / pop hl in XBASIC_COPY_TO (30_basic_helpers.asm lines 441, 458), saving 4 bytes and 11 cycles
- Replace 4× sla e with ld a,e / 4× add a,a / ld e,a in FloatToBCD.add.2 (21_logic_pack.asm lines 246-249), saving 2 bytes
- Replace ld a,h / res 7,a / ld h,a with res 7,h in FloatToBCD (21_logic_pack.asm lines 175-177), saving 2 bytes
- Replace cpl + 2× ld (CURLIN),a with ld hl,0xFFFF / ld (CURLIN),hl in XBASIC_END (30_basic_helpers.asm lines 157-159), saving 1 byte
- Replace bit 7,h / jr z with jp p in XBASIC_TAB (30_basic_helpers.asm lines 635-636), saving 1 byte
- Replace 2× sla a with 2× add a,a in cmd_play (31_cmd.asm lines 14-15), saving 2 bytes
- Replace 4× sla a with 4× add a,a in cmd_page.delay (31_cmd.asm lines 1284-1287), saving 4 bytes
- Replace 4× sla a with 4× add a,a in cmd_mtf.palette.copy.to_vram.loop (31_cmd.asm lines 1716-1719), saving 4 bytes
- Replace 7× inc hl with ld de,7 / add hl,de in cmd_wrtfnt (31_cmd.asm lines 90-96), saving 3 bytes

#### Removed
- Remove commented-out Panasonic turbo check dead code in cmd_turbo (31_cmd.asm lines 534-536) and usr2_turbo (32_usr.asm lines 108-110)
- Remove commented-out MSX2 VDP fallback dead code in gfxVDP.set (60_bios_helpers.asm lines 913-917)
- Remove dead xor a from intSHR.1 shift loop (21_logic_pack.asm line 111), saving 1 byte and 4 cycles/iteration
- Remove dead commented-out source lines in XBASIC_END / XBASIC_INIT (30_basic_helpers.asm lines 153-154, 160, 201-202), zero bytes (cleanup)

### [2026-06-23-refactor-xbx-equ-indirection](openspec/changes/archive/2026-06-23-refactor-xbx-equ-indirection) Point XBASIC EQUs at native kernel labels

#### Added
- Adicionar label C7F44: em 90_support.asm no entry point mid-routine do XBASIC_COPY_STRING (offset +4 de C7F40)

#### Changed
- Alterar 80 EQUs XBASIC_*: equ 0xXXXX em 00_constants_and_workarea.asm para XBASIC_*: equ <label-nativo>, onde <label-nativo> é o label CXXXX/IXXXX/JXXXX já existente no mesmo endereço

#### Removed
- Remover o EQU redundante WriteParamBCD: equ 0x6C0B (conflito com label homônimo em 90_support.asm:190)

### Added

- Kernel regression test infrastructure (`make test-kernel`) with ROM binary integrity validation
- Kernel ROM size build-time check (max 0x4000 bytes) with Makefile alert and doctest unit test
- Full dispatch table coverage for all XBASIC math, string, graphics, trap, cast, and I/O routines (~55 new `dw` entries, ~55 new `DISP_XBASIC_*` constants)

### Changed

- Compact player idle-state initialization with sequential HL writes in 33_player.asm
- Consolidate 4 consecutive `ld (ENDPRG+n),a` zero-stores with inc-chain in 20_runtime.asm
- Remove redundant `push de`/`pop de` guard in `cmd_preflight_disk`
- Remove duplicate `ld a, (STARTUP_CFG_FILEIO)` load in `run_user_basic_code_on_rom`
- Remove redundant `xor a` in `resource.address`
- Restructure wrapper routine jump table from 126-entry `jp` to word-pointer dispatch (saves ~125 bytes)
- Unify wrapper dispatch through single `addKernelCall` path with index auto-detection, remove `addKernelDispatch` and dispatcher ASM routine
- Replace ~117 `cpu.addCall(def_XBASIC_*)` with `optimizer.addKernelCall(DISP_XBASIC_*)` across all compiler files for indirect dispatch
- Replace ~12 magic number BIOS addresses (`0x009C`, `0x009F`, etc.) with named constants (`def_CHSNS`, `def_CHGET`, etc.)
- Double `header.bin` from 0x4000 to 0x8000 bytes: first half (0x0000-0x3FFF) becomes virtual dispatch table section, second half (0x4000-0x7FFF) is the kernel ROM image
- Move dispatch table to logical address 0x0000 with `DEFS` padding to 0x4000 boundary, removing the need for a second `org`
- Adapt `getKernelCallAddr()` for two-range addressing (0x0000-0x3FFF table lookups with direct `bin_header_bin` indexing, 0x4000-0x7FFF kernel addresses)
- Update ROM builder to write only second half (`bin_header_bin + 0x4000`)
- Update build size check from 16384 to 32768 byte limit
- Disable 58 unreachable code labels in `90_support.asm` with `DEFS` fillers (preserving binary offsets)
- Redirect 80 XBASIC_* EQUs from hardcoded hex addresses to native Z80 labels (`C`/`I`/`J` convention) in `00_constants_and_workarea.asm`
- Add `C7F44` mid-routine entry point label for `XBASIC_COPY_STRING` in `90_support.asm`
- Collapse `intCompareGT`/`intCompareGE` into delegated comparisons via operand swap (18 bytes saved)
- Replace `sla a` sequences with `add a,a` in `cmd_setfnt`, `cmd_play`, `cmd_page`, `cmd_mtf` (14 bytes saved)
- Replace `ld (MV_DPTR),hl`/`ld hl,(MV_DPTR)` with `push hl`/`pop hl` (4 bytes saved)
- Micro-optimizations in `FloatToBCD`, `intSHR`, `XBASIC_END`, `XBASIC_TAB`, `cmd_wrtfnt` (10 bytes saved)
- Remove dead commented-out code blocks

### Fixed

- Fix MegaROM mode restore in `verify.slots.test.ram` (save flags, write 0 to port 0x8E)
- Fix missing carry clear before `sbc hl,de` in `cmd_fmaxfiles.set_heap_size`
- Fix hardcoded 255-byte copy in `resource.get_data` (cap at actual resource size)
- Fix duplicate `WriteParamBCD` EQU conflict with existing label causing compilation error
- Fix compiler error messages reporting tag index instead of physical file line number (use `tag->lexerLine->lineNumber`)
- Fix integration test Makefile not stopping on compilation error (`|| exit 1` in for loop)

### Removed

- Remove fully commented-out BDOS fallback block from 34_file_handling.asm
- Remove commented-out `VDP_wait` routine from 60_bios_helpers.asm
- Remove dead `inc de` in `XBASIC_INIT` function key loop
- Remove dead `ld a, 2` in `cmd_finput` state machine
- Remove 5 alignment `nop` instructions from `20_runtime.asm` (no longer needed after dispatch table move)

## [1.0.0.0] - 2026-06-19

### [2026-06-18-changelog-standardization](openspec/changes/archive/2026-06-18-changelog-standardization) Standardize CHANGELOG.md on Keep a Changelog and ISO dates

#### Changed
- Convert CHANGELOG.md to Keep a Changelog format with categorized sections (Added, Changed, Fixed, Removed, Security)
- Migrate date format from (YYYY/MM/DD) to ISO YYYY-MM-DD
- Convert placeholder #.#.#.# entries into ## [Unreleased] sections
- Update info_history (in appinfo.h) to contain only: current release details + summary of the last 2 releases + link to the current release on GitHub
- Update the architecture spec to document the new changelog format and sync conventions
- Update appinfo.h comments to reflect the new sync rule

### [2026-06-18-import-existing-specs](openspec/changes/archive/2026-06-18-import-existing-specs) Import existing user stories, ADRs, glossary, and governance into OpenSpec

#### Added
- Create archived changes for completed sprints (1-3 from plan.md)
- Create active changes for pending sprints (4-7 from plan.md)

#### Changed
- Migrate 8 user stories from specs/user-stories/ into capability specs under openspec/specs/<capability>/spec.md
- Migrate ADR-001 (architecture foundations) into openspec/specs/architecture/spec.md
- Migrate constitution.md into openspec/specs/governance/spec.md
- Migrate glossary.md into openspec/specs/glossary/spec.md
- Promote stable insights from insights.md into the architecture spec as ADR entries
- Update CHANGELOG.md to reference OpenSpec changes
- Update README.md to reference openspec/ as the spec source of truth

#### Removed
- Remove the old specs/ directory only after: (a) 3 verification rounds confirming nothing was left behind, and (b) explicit formal confirmation from the project maintainer

### [2026-06-18-refactor-free-ram-magic-constants](openspec/changes/archive/2026-06-18-refactor-free-ram-magic-constants) Replace free-RAM magic numbers with named compiler constants

#### Changed
- Replace inline magic constants in compiler.cpp:314-319 with named constants defined in compiler_hooks.h
- Document each constant's purpose and relationship to the kernel's assembly-level memory layout
- Preserve all existing numeric values exactly (no behavioral change to the percentage calculation)

### [2026-06-18-sprint-1-sdd-refactoring](openspec/changes/archive/2026-06-18-sprint-1-sdd-refactoring) Restructure the codebase into Clean Architecture layers and establish SDD

#### Added
- Introduce unit testing infrastructure with TDD
- Create the initial specs/ directory with user stories, ADRs, and supporting documents

#### Changed
- Restructure source code into Clean Architecture layers: src/cli, src/application, src/domain, src/infrastructure
- Apply SOLID principles across the codebase (especially Single Responsibility Principle)
- Establish SDD as the specification methodology

### [2026-06-18-sprint-2-on-stmts](openspec/changes/archive/2026-06-18-sprint-2-on-stmts) Add event-driven ON statements and fix boot initialization order

#### Added
- Add event-driven control flow support (ON INTERVAL, ON SPRITE, ON STRIG, ON KEY, ON STOP, ON ERROR)

#### Changed
- Refactor ON statement handling in compiler

#### Fixed
- Fix ON statement boot initialization order

### [2026-06-18-sprint-3-file-handling](openspec/changes/archive/2026-06-18-sprint-3-file-handling) Implement disk file-handling statements, functions, and runtime

#### Added
- Implement OPEN, CLOSE, PRINT#, INPUT#, LINE INPUT#, MAXFILES commands
- Implement EOF(), LOC(), LOF(), FPOS(), ERR(), DSKF() functions
- Add BDOS-based file I/O runtime in kernel assembly
- Implement disk vs non-disk memory profile switching
- Add INPUT#/LINE INPUT# field and line parsing modes

### [2026-06-19-cross-platform-fixes](openspec/changes/archive/2026-06-19-cross-platform-fixes) Fix Windows startup and cross-platform path/argument handling

#### Added
- Implement pathExists() for Windows (currently returns false — a known TODO)
- Add -- (end-of-options) support to CommandLineParser so filenames starting with - can be passed
- Add Windows Unicode filename support via wargv to UTF-8 conversion at startup

#### Changed
- Normalize Windows detection from #ifdef Win to standard #ifdef _WIN32 across all platform-guarded blocks

#### Fixed
- Fix uninitialized vscode bool in BuildOptions constructor (root cause of the "VSCode already initialized" error on Windows)

#### Removed
- Remove dead #include <malloc.h> from main.cpp (glibc-only, no symbols used)
- Remove -? help option (glob expansion risk in POSIX shells outweighs convenience)

### [2026-06-19-tile-color-pattern](openspec/changes/archive/2026-06-19-tile-color-pattern) Add SET/GET TILE COLOR syntax variants and kernel routines

#### Added
- Implement SET TILE COLOR with 3 syntax forms: simple (fc, bc), array (fc0..fc7 with optional bc0..bc7), and buffer (4-integer array)
- Implement GET TILE COLOR with buffer syntax and optional bank parameter
- Implement the Z80 kernel routines for buffer-based SET and GET (currently stub/missing)
- Add unit tests in tests/unit/ covering all syntax forms for both SET and GET TILE COLOR (compiler translation layer)
- Add integration test program in tests/integration/graph/ similar to test94.bas to exercise all syntaxes on real/virtual hardware

#### Changed
- Optionally accept bank parameter (0-2, default 3=all for SET, default 0 for GET)
- Buffer uses MSX native color table format: each 16-bit integer packs two rows (low byte = row N, high byte = row N+1), each byte = FC<<4 | BC
- Use LDIRVM/LDIRMV for block transfer (3 calls for 3 banks)

#### Fixed
- Fix the SET TILE COLOR buffer form calling convention (currently generates wrong Z80 code)

### [2026-06-19-tile-flip-rotate](openspec/changes/archive/2026-06-19-tile-flip-rotate) Add SET TILE FLIP/ROTATE with bank selection and kernel routines

#### Added
- Implement SET TILE FLIP <n>, <dir>[, <bank>]
- Implement SET TILE ROTATE <n>, <dir>[, <bank>]
- Add assembly-level tile flip/rotate routines in kernel (screen 2, text modes 0-1)
- Add unit tests (parser + compiler) and integration test (GRAPH/)

### [2026-06-19-tile-pattern-syntax](openspec/changes/archive/2026-06-19-tile-pattern-syntax) Add SET/GET TILE PATTERN inline, array, and bank forms

#### Added
- Add SET TILE PATTERN <n>, <4-int-array> [, <bank>] — DIM array of 4 integers, with optional bank select
- Add GET TILE PATTERN <n>, <4-int-array> [, <bank>] — read tile pattern into DIM array of 4 integers, with optional bank select
- Implement Z80 kernel bulk set_tile_pattern_buffer routine (LDIRVM block copy)
- Implement Z80 kernel get_tile_pattern routine (LDIRMV block read)

#### Changed
- Formalize SET TILE PATTERN <n>, (<l0>,...,<l7>) [, <bank>] — inline 8 bytes with optional bank select

#### Fixed
- Fix the calling convention mismatch in the compiler's buffer form path
- Fix broken jump table entries

### Added

- File handling support implementation (US-007).
- Spec migration to OpenSpec format.
- Changelog standardized to Keep a Changelog format.
- Windows Unicode argv support via `__wargv` to UTF-8 conversion on program startup.
- `--` (end-of-options) support in CLI parser for filenames starting with `-`.
- `SET TILE PATTERN` buffer form with 4-integer array and bank parameter.
- `GET TILE PATTERN` with bank parameter (former stub now implemented).
- Z80 kernel bulk LDIRVM/LDIRMV routines for `get_tile_pattern` / `set_tile_pattern_buffer`.
- `SET TILE COLOR` all syntax variants: inline FC/BC, per-row FC array (with/without BC array), and buffer form (`SET TILE COLOR <n>, <4-int-array> [, <bank>]`), with bank parameter support (0-2 for individual banks, 3 or omitted for all banks).
- `GET TILE COLOR` with bank parameter: `GET TILE COLOR <n>, <4-int-array> [, <bank>]` with LDIRMV read via `get_tile_color` (replaces stub).
- Z80 kernel routines for `set_tile_color_buf` (3-pass LDIRVM for bank=all) and `get_tile_color` with screen mode validation.
- Unit and integration tests for all SET/GET TILE COLOR syntax forms.
- `SET TILE FLIP <n>, <dir>[, <bank>]` with 3 directions (0=horizontal, 1=vertical, 2=both) and optional bank parameter (0-2 specific, 3=all).
- `SET TILE ROTATE <n>, <dir>[, <bank>]` with 3 directions (0=left, 1=right, 2=180) and optional bank parameter.
- Screen 2 validation (returns early on screen >= 5).
- Reuses existing sprite flip/rotate Z80 routines (`binaryReverseA`, `blockRotateL`, `blockRotateR`) for 8-byte tile blocks.
- Unit parser, compiler, and integration tests for all flip/rotate syntax forms.

### Changed

- Magic constants in RAM percentage calculation replaced with named constants.
- Platform detection normalized from `#ifdef Win` to standard `#ifdef _WIN32`.
- Removed dead includes (`<malloc.h>`, `<math.h>`) from `main.cpp`.

### Fixed

- Uninitialized `vscode` member causing "VSCode already initialized" error on Windows.
- `pathExists()` on Windows: replaced stub with `GetFileAttributesA`.
- `-?` help option removed due to glob expansion risk in POSIX shells.
- `set_tile_flip` / `set_tile_rotate`: direction in HL was clobbered by VRAM address operations (`get_tile_vram_addr` + `set_tile.copy`), causing all directions to behave as direction=2 (both). Fixed by saving HL on stack before VRAM ops.
- `SET TILE FLIP` / `SET TILE ROTATE` with 3 params: bank evaluation (`evalExpression`) overwrote HL (direction) before the call. Fixed by saving HL on stack before bank evaluation.
- `set_tile_rotate` left/right: destination buffer was `STRBUF+32` (sprite size) causing rotation output to be written past the tile data while source data was shifted to zero. Fixed by backing up tile to `STRBUF+8` and rotating from backup.

## [0.3.4.2] - 2026-04-14

### Fixed

- Sprite glitch bug fix (issue #19).

### Changed

- Z80 kernel SRP refactoring.

## [0.3.4.1] - 2026-04-09

### Fixed

- ELF export bug.

## [0.3.4.0] - 2026-04-08

### Fixed

- CDB empty code and ON statements bug fixes (issue #18).

## [0.3.3.9] - 2026-03-25

### Fixed

- CDB variable name bug.

## [0.3.3.8] - 2026-03-21

### Fixed

- VSCode parameter bug.

## [0.3.3.7] - 2026-03-20

### Added

- CDB symbol export feature (issue #17).
- Plain/megarom auto mode parameter (`-a`).
- VSCode MSX-BASIC project initializer parameter (`--vscode`).

### Fixed

- MAX name bug (issue #13).

### Changed

- Extensive code refactoring and memory allocation hardenings.

## [0.3.3.6] - 2026-01-28

### Fixed

- LINE statement parameters bug (issue #16).

## [0.3.3.5] - 2026-01-16

### Added

- SET/GET SPRITE COLOR/PATTERN new commands.

## [0.3.3.4] - 2025-09-26

### Changed

- MSX Tile Forge map loading optimization.

### Fixed

- Resource file check bug.

## [0.3.3.3] - 2025-09-20

### Added

- MSX Tile Forge support (CMD MTF).

## [0.3.3.2] - 2025-09-14

### Added

- NoICE OpenMSX Debugger support.

### Changed

- Resources refactoring and limit expanded (~3200).

### Fixed

- Minor bug fixes.

## [0.3.3.1] - 2025-09-04

### Added

- OMDS support.
- CLEAR statement.

### Changed

- Symbols/resources refactoring.

### Fixed

- Minor bug fixes.

## [0.3.3.0] - 2025-08-18

### Added

- New `-i` and `-o` parameters.

### Changed

- GitHub CI/CD and RPM/DEB packaging refactoring.
- Source code ported from Code::Blocks to VSCode.

## [0.3.2.9] - 2025-07-22

### Fixed

- MegaROM emulation detection bug.

## [0.3.2.8] - 2025-07-21

### Added

- ASCII8 MegaROM format support.

## [0.3.2.7] - 2025-04-25

### Fixed

- DRAW bug.

## [0.3.2.6] - 2025-01-20

### Fixed

- LINE bug.

### Added

- SCREEN ON/OFF statement.

## [0.3.2.5] - 2024-12-18

### Added

- KonamiSCC filename suffix added to MegaROM.

## [0.3.2.4] - 2024-09-05

### Added

- GET DATE `date_format` parameter.

## [0.3.2.3] - 2024-09-04

### Added

- Custom SET DATE/TIME, GET DATE/TIME statements.

## [0.3.2.2] - 2024-06-19

### Fixed

- MegaROM size estimate bug.

## [0.3.2.1] - 2024-05-23

### Fixed

- MegaROM > 256kb bug.

### Changed

- `-c` set as default mode.

## [0.3.2.0] - 2023-11-04

### Fixed

- CMD PAGE fix (Cadari bit).

## [0.3.1.9] - 2023-11-02

### Added

- CMD PAGE new command (Cadari bit).

## [0.3.1.8] - 2023-10-29

### Changed

- WIDTH statement optimization.

## [0.3.1.7] - 2023-10-28

### Changed

- VDP function optimization (Cadari bit).

## [0.3.1.6] - 2023-10-16

### Added

- Mouse support on MSX1 via PAD function.

## [0.3.1.5] - 2023-09-24

### Fixed

- Integer multiply in MegaROM bug.

## [0.3.1.4] - 2023-09-10

### Fixed

- Multiplying constant integers bug.

## [0.3.1.3] - 2023-09-02

### Added

- CMD RSCTORAM new command.

## [0.3.1.2] - 2023-08-27

### Added

- SET SPRITE ROTATE new command.

## [0.3.1.1] - 2023-08-26

### Added

- IPEEK()/IPOKE new commands.

## [0.3.1.0] - 2023-08-19

### Added

- SET SPRITE FLIP new command.

## [0.3.0.9] - 2023-07-03

### Fixed

- LINE parameters parse bug.

## [0.3.0.8] - 2023-06-24

### Fixed

- BASE function bug.

## [0.3.0.7] - 2023-06-23

### Fixed

- DATA last element parameter bug.

## [0.3.0.6] - 2023-06-22

### Fixed

- MegaROM resource allocation bug.

## [0.3.0.5] - 2023-06-07

### Fixed

- Integer division optimization bug.

## [0.3.0.4] - 2023-05-25

### Fixed

- PAINT statement on WebMSX USA TurboR bug.

## [0.3.0.3] - 2023-05-25

### Fixed

- COPY statement operator parameter bug.

## [0.3.0.2] - 2023-05-24

### Fixed

- COPY statement and SYMBOLs file (var address) bug.

## [0.3.0.1] - 2023-05-23

### Fixed

- COPY screen data in tiled mode bug.

## [0.3.0.0] - 2023-02-17

### Fixed

- TURBO function bug.

### Changed

- Integer math optimizations.

## [0.2.9.9] - 2023-01-20

### Added

- SET FONT new command.

## [0.2.9.8] - 2022-12-21

### Added

- MegaROM mode support for CMD, RESOURCES and Arkos Tracker player.

## [0.2.9.7] - 2022-07-20

### Fixed

- COLOR NEW/RESTORE bug on MSX1.

## [0.2.9.6] - 2022-07-15

### Fixed

- Tokenized THEN/GOTO bug.

### Added

- SET ADJUST/SCREEN/BEEP/TITLE/PROMPT support.

## [0.2.9.5] - 2022-05-23

### Added

- SCREEN COPY/PASTE, SCREEN SCROLL.
- SCREEN LOAD (.SCn as resources).
- SPRITE LOAD (CMD WRTSPR alternative).

## [0.2.9.4] - 2022-04-25

### Fixed

- Several bug fixes: try..catch exceptions, tokenized source code input detection, DATA/RESTORE misbehavior.

## [0.2.9.3] - 2022-04-05

### Added

- IRESTORE new command (for IREAD binary files).
- RESOURCESIZE function.

## [0.2.9.2] - 2022-01-30

### Added

- Support for TinySprite backup files with BLOAD and CMD WRTSPR.

### Fixed

- BLOAD for screen files bug.

## [0.2.9.1] - 2022-01-05

### Fixed

- LINE negative parameter bug.

## [0.2.9.0] - 2021-12-03

### Added

- SET TILE ON/OFF/COLOR/PATTERN new commands.

## [0.2.8.9] - 2021-11-03

### Fixed

- Expression evaluator of TIME/INKEY bug.
- Undeclared array syntax error.

## [0.2.8.8] - 2021-10-14

### Fixed

- Hexa constant bug.

### Added

- RESUME support (tokenized mode only).

## [0.2.8.7] - 2021-10-13

### Fixed

- TAB function token bug (tokenized mode).
- MOD and \ bug for single and double types.
- Numeric constant type identification bug.

## [0.2.8.6] - 2021-10-12

### Added

- "0" as format string on USING$() for printing left zeros.

## [0.2.8.5] - 2021-10-11

### Fixed

- SWAP bug fix 2.

### Changed

- Math operations parameters optimization 2.

## [0.2.8.4] - 2021-10-08

### Fixed

- SWAP bug.
- CMD PLYLOOP bug.

### Changed

- Math operations parameters optimization.

### Added

- PLYSTATUS().

## [0.2.8.3] - 2021-10-05

### Fixed

- PRINT USING bug.

### Changed

- PRINT USING optimization.

## [0.2.8.2] - 2021-09-30

### Added

- CIRCLE full parameters.

## [0.2.8.1] - 2021-09-28

### Fixed

- PSET parameters bug.

### Added

- SCREEN full parameters support.
- SET VIDEO support.
- COPY SCREEN support.

## [0.2.8.0] - 2021-09-24

### Fixed

- COLOR without parameters bug.
- COPY parameter bug.
- CMD WRTVRAM hardware/int bug.

### Changed

- Sprite collision optimization.

## [0.2.7.9] - 2021-09-21

### Fixed

- Running on SofaRun bug.

## [0.2.7.8] - 2021-09-10

### Added

- PRINT USING support for numeric values.
- USING$() new function.
- F1..F10 function keys return ASCII codes 246 to 255 respectively.

### Fixed

- END statement bug.

## [0.2.7.7] - 2021-08-31

### Fixed

- PAINT bug (screen 2 border color must be equal to filler color).

### Added

- TXT/CSV files resources support (CMD RESTORE).
- DATA statements now allocated in resources page.

## [0.2.7.6] - 2021-07-20

### Changed

- Float and String array index access optimization.

### Added

- COLLISION(<n1>,<n2>).

## [0.2.7.5] - 2021-07-16

### Changed

- Integer array index access optimization.
- Integer FOR/NEXT optimization.

## [0.2.7.4] - 2021-06-17

### Fixed

- AKM fix to playing at same speed in 50hz/60hz.

## [0.2.7.3] - 2021-06-16

### Changed

- Arkos Tracker 2 player support (PT3 deprecated).

## [0.2.7.2] - 2021-05-07

### Fixed

- Bug fix pack (duplicated line error in FOR..NEXT block).

## [0.2.7.1] - 2021-05-05

### Fixed

- Bug fix pack (setfnt default color).

### Added

- CMD UPDFNTCLR.

## [0.2.7.0] - 2021-05-01

### Added

- CMD CPU, CPU(), MAKER(), VDP() without parameters.

## [0.2.6.9] - 2021-04-29

### Fixed

- Bug fix pack (LOCATE).

### Changed

- Several code size optimizations.

### Added

- IDATA/IREAD implementation.

## [0.2.6.8] - 2021-04-27

### Added

- Change font in graphical mode (CMD WRTFNT and CMD SETFNT).

### Changed

- Initialization, data/heap pointers and USR() code optimization.

## [0.2.6.7] - 2021-04-25

### Added

- NTSC() function.

### Fixed

- Bug fix pack (LINE, PRINT ';' in tile mode).

## [0.2.6.6] - 2021-04-22

### Added

- SYMBOLs file create for use on OpenMSX debugger (`-s` option).

## [0.2.6.5] - 2021-04-16

### Changed

- OPEN 'GRP' optimization.
- LOCATE/PRINT in graphical mode.

## [0.2.6.4] - 2021-04-07

### Fixed

- MegaROM mode bug (running with SofaRun with MegaRAM).

## [0.2.6.3] - 2021-04-07

### Fixed

- PT3/MegaROM mode bug (running with ExecROM with MegaRAM).

## [0.2.6.2] - 2021-04-05

### Fixed

- PT3 bug (running in 64kb RAM with ExecROM without mapper, ex: Hotbit w/disk driver, Sony HB-F500, Victor HC-95, Toshiba HX-23F...).

## [0.2.6.1] - 2021-03-30

### Fixed

- PT3 player bug (32kb RAM machines, ex: Pioneer PX7).

## [0.2.6.0] - 2021-03-29

### Fixed

- Compatibility bug fix pack (hotbit).

## [0.2.5.9] - 2021-03-12

### Added

- Precomp/MegaROM: BLOAD.

## [0.2.5.8] - 2021-03-10

### Fixed

- Bug fix pack (OPEN, PAINT, temporary strings).

### Added

- Line number option.

## [0.2.5.7] - 2021-03-09

### Added

- MegaROM: RETURN \<line\>, ON..., R800 cpu (Turbo-R) math code support.

## [0.2.5.6] - 2021-03-03

### Fixed

- Precompilation: bug fix pack (LINE INPUT, CLEAR, RAM size).

### Added

- MegaROM: DATA, READ, RESTORE.

## [0.2.5.5] - 2021-03-02

### Added

- Experimental MegaROM option (GOTO, GOSUB, RETURN, FOR/NEXT).

### Fixed

- MSX 1 BIOS bug (CALSLT to page 0: Expert XP-800).

## [0.2.5.4] - 2021-02-24

### Fixed

- Precomp: bug fix pack (PT3).

## [0.2.5.3] - 2021-02-23

### Fixed

- Precomp: bug fix pack (TAB).

### Changed

- Start compiled program at screen 0.

## [0.2.5.2] - 2021-02-22

### Fixed

- Precomp: bug fix pack (USR).

### Changed

- Code size optimization.

## [0.2.5.1] - 2021-02-18

### Fixed

- Precomp: bug fix pack (BASE, PAINT, PSET).

### Changed

- Binary code size optimization.

## [0.2.5.0] - 2021-02-16

### Fixed

- Precomp: DATA/USR bug.

### Added

- PSG, partial file support (EOF, INPUT#, OPEN).

### Changed

- Binary code size optimization.

## [0.2.4.9] - 2021-02-16

### Fixed

- Precomp: bug fix pack (DEF, DIM, functions, DATA, LINE, OPEN).

## [0.2.4.8] - 2021-02-12

### Added

- Precomp: COLOR=NEW, COLOR=RESTORE, COLOR=(), COLOR SPRITE()=, COLOR SPRITE$()=, OPEN, CLOSE, PRINT#, DEF USR, USR, MAXFILE, CMD CLIP.

## [0.2.4.7] - 2021-01-25

### Added

- Precomp: PUT TILE, TILE, MSX.

## [0.2.4.6] - 2021-01-21

### Added

- Precomp: COPY .. TO HEAP / COPY HEAP TO ... .

## [0.2.4.5] - 2021-01-21

### Added

- Precomp: SET PAGE, SET SCROLL, COPY.

## [0.2.4.4] - 2021-01-19

### Fixed

- Precomp: PUT SPRITE fix, RESTORE fix.

## [0.2.4.3] - 2021-01-18

### Changed

- Precomp: array pointers optimization.

## [0.2.4.2] - 2021-01-18

### Changed

- Precomp: logical operations size optimization.

## [0.2.4.1] - 2021-01-17

### Fixed

- Precomp: CMD and internal USR bug.

## [0.2.4.0] - 2021-01-16

### Added

- Precomp: CMD and refactoring memory scheme.

## [0.2.3.9] - 2021-01-15

### Added

- Precomp: ON INTERVAL/SPRITE/STRIG/KEY/STOP.

## [0.2.3.8] - 2021-01-14

### Added

- Precomp: PUT SPRITE, SPRITE$, ON...GOTO/GOSUB and related.

## [0.2.3.7] - 2021-01-12

### Added

- Precomp: DATA/READ and special functions.

## [0.2.3.6] - 2021-01-12

### Added

- Precomp: graphical statements (CIRCLE, LINE, PAINT, DRAW).

## [0.2.3.5] - 2021-01-11

### Added

- Precomp: graphical statements (COLOR, WIDTH, SCREEN, PSET, PRESET).

## [0.2.3.4] - 2021-01-10

### Added

- Precomp: FOR/NEXT.

## [0.2.3.3] - 2021-01-09

### Added

- Precomp: IF/THEN/ELSE, IF/GOTO/ELSE, IF/GOSUB/ELSE.

## [0.2.3.2] - 2021-01-08

### Added

- Precomp: string functions.

## [0.2.3.1] - 2021-01-06

### Added

- Precomp: logical expressions and numeric functions.

## [0.2.3.0] - 2021-01-01

### Added

- Precompilation kernel.

## [0.2.2.0] - 2020-10-28

### Fixed

- PT3 joystick bug.
- Locate/Print commands in screen mode 2 bug.

### Changed

- Strip 'remark only' lines.

### Fixed

- `?` and `_` commands.

## [0.2.1.0] - 2020-10-13

### Added

- Sprite collision support (USR3 function).
- Replay a PT3 (CMD PT3REPLAY).

## [0.2.0.0] - 2020-08-19

### Added

- Built-In fonts support implemented (CMD SETFNT, CMD PT3LOOP).

## [0.1.9.0] - 2020-08-02

### Fixed

- Emulator / Hardware ROM loading bugs.

## [0.1.8.0] - 2020-06-14

### Fixed

- CALL TURBO ON/OFF bug.

## [0.1.7.0] - 2020-06-13

### Fixed

- CMD 2nd parameter, SPC(), CHR$(), STRING$(), eol (Windows) bugs.

## [0.1.6.0] - 2020-06-12

### Added

- CMD WRTFNT to load compressed .ALF files.

## [0.1.5.0] - 2020-06-12

### Fixed

- Windows version minor bugs.

## [0.1.4.0] - 2020-06-03

### Added

- PT3TOOLS support.

## [0.1.3.0] - 2020-05-28

### Added

- INCLUDE, WRTSCR, WRTCLR, WRTCHR, WRTSPRPAT, CMD DRAW implemented.

## [0.1.2.0] - 2020-05-27

### Added

- Resources initial implementation (TEXT, CMD PLAY).

## [0.1.1.0] - 2020-05-24

### Added

- Turbo mode implemented (xbasic).

## [0.1.0.0] - 2020-05-21

### Added

- Proof of concept.

See main [CHANGELOG](CHANGELOG.md) for newer releases.

[1.4.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.4.0.0
[1.3.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.3.1.0
[1.3.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.3.0.0
[1.2.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.3.0
[1.2.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.2.0
[1.2.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.1.0
[1.2.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.0.0
[1.1.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.1.0.0
[1.0.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.1.0
[1.0.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.0.0
[0.3.4.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.4.2
[0.3.4.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.4.1
[0.3.4.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.4.0
[0.3.3.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.9
[0.3.3.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.8
[0.3.3.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.7
[0.3.3.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.6
[0.3.3.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.5
[0.3.3.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.4
[0.3.3.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.3
[0.3.3.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.2
[0.3.3.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.1
[0.3.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.0
[0.3.2.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.9
[0.3.2.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.8
[0.3.2.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.7
[0.3.2.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.6
[0.3.2.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.5
[0.3.2.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.4
[0.3.2.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.3
[0.3.2.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.2
[0.3.2.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.1
[0.3.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.2.0
[0.3.1.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.9
[0.3.1.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.8
[0.3.1.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.7
[0.3.1.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.6
[0.3.1.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.5
[0.3.1.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.4
[0.3.1.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.3
[0.3.1.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.2
[0.3.1.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.1
[0.3.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.1.0
[0.3.0.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.9
[0.3.0.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.8
[0.3.0.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.7
[0.3.0.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.6
[0.3.0.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.5
[0.3.0.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.4
[0.3.0.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.3
[0.3.0.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.2
[0.3.0.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.1
[0.3.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.0.0
[0.2.9.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.9
[0.2.9.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.8
[0.2.9.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.7
[0.2.9.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.6
[0.2.9.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.5
[0.2.9.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.4
[0.2.9.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.3
[0.2.9.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.2
[0.2.9.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.1
[0.2.9.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.9.0
[0.2.8.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.9
[0.2.8.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.8
[0.2.8.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.7
[0.2.8.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.6
[0.2.8.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.5
[0.2.8.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.4
[0.2.8.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.3
[0.2.8.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.2
[0.2.8.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.1
[0.2.8.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.8.0
[0.2.7.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.9
[0.2.7.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.8
[0.2.7.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.7
[0.2.7.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.6
[0.2.7.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.5
[0.2.7.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.4
[0.2.7.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.3
[0.2.7.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.2
[0.2.7.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.1
[0.2.7.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.7.0
[0.2.6.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.9
[0.2.6.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.8
[0.2.6.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.7
[0.2.6.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.6
[0.2.6.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.5
[0.2.6.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.4
[0.2.6.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.3
[0.2.6.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.2
[0.2.6.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.1
[0.2.6.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.6.0
[0.2.5.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.9
[0.2.5.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.8
[0.2.5.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.7
[0.2.5.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.6
[0.2.5.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.5
[0.2.5.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.4
[0.2.5.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.3
[0.2.5.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.2
[0.2.5.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.1
[0.2.5.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.5.0
[0.2.4.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.9
[0.2.4.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.8
[0.2.4.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.7
[0.2.4.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.6
[0.2.4.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.5
[0.2.4.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.4
[0.2.4.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.3
[0.2.4.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.2
[0.2.4.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.1
[0.2.4.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.4.0
[0.2.3.9]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.9
[0.2.3.8]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.8
[0.2.3.7]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.7
[0.2.3.6]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.6
[0.2.3.5]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.5
[0.2.3.4]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.4
[0.2.3.3]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.3
[0.2.3.2]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.2
[0.2.3.1]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.1
[0.2.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.3.0
[0.2.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.2.0
[0.2.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.1.0
[0.2.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.2.0.0
[0.1.9.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.9.0
[0.1.8.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.8.0
[0.1.7.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.7.0
[0.1.6.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.6.0
[0.1.5.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.5.0
[0.1.4.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.4.0
[0.1.3.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.3.0
[0.1.2.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.2.0
[0.1.1.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.1.0
[0.1.0.0]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.1.0.0
