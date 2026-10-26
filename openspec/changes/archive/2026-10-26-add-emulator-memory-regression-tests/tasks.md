# Tasks

## 1. Extend the MSX memory debug skill

- [x] 1.1 Add opt-in VRAM capture to the skill launcher and Tcl capture script, writing a raw VRAM artifact beside the ROM; verify the artifact size matches the emulated VDP for both MSX1 and MSX2 machines.
- [x] 1.2 Add pre-boot breakpoint configuration to the skill launcher using openMSX `-command` arguments; verify a short-lived program is captured at `END_PGM` instead of after returning to BASIC.
- [x] 1.3 Add timed keyboard and joystick input injection to the skill capture script; verify a program waiting at `INPUT` and a program polling `STICK`/`STRIG` both advance.
- [x] 1.4 Add fallback capture semantics so a configured emulated-time capture runs when no breakpoint fires; verify a looping program produces RAM, VRAM, and screenshot artifacts.
- [x] 1.5 Preserve the legacy invocation contract; verify a ROM-only invocation still produces the same RAM and screenshot artifacts with the default machine and delay.
- [x] 1.6 Update the skill README/SKILL documentation with the new options and verify each documented example runs as written.

## 2. Scaffold the regression workflow

- [x] 2.1 Create `tests/integration/regression/` with runner, manifest, capture integration, baseline, and artifact directories; verify the expected paths are present.
- [x] 2.2 Add the JSON manifest schema for scenarios, per-program overrides, machine choice, input scripts, RAM probes, VRAM checks, and volatile ranges; verify a sample manifest parses with the standard library.
- [x] 2.3 Add `regression`, `regression-baseline`, and `regression-list` targets to `tests/integration/Makefile`, independent from `run`; verify `make -C tests/integration regression-list` lists scenarios and programs without compiling or launching openMSX.
- [x] 2.4 Add scenario and program selectors (`SCENARIO=`, `PROGRAM=`) with validation for unknown selectors; verify selecting one scenario runs only that folder and selecting one program runs only that file.
- [x] 2.5 Document the regression workflow in `tests/integration/README.md` with local-only usage examples; verify each documented command works.

## 3. Compile and execute selected programs

- [x] 3.1 Implement ASCII8 compilation with `--noi` symbols and an isolated output directory; verify selected programs produce a ROM and `.noi` file without touching existing integration artifacts.
- [x] 3.2 Implement execution through the extended memory debug skill with per-program machine selection (MSX1 default, MSX2 for `SCREEN 5+`); verify one MSX1 and one MSX2 program run with the expected machine.
- [x] 3.3 Implement terminal-state capture using `END_PGM` and `END_STMT + 2`; verify a terminating program captures before returning to BASIC.
- [x] 3.4 Implement fallback capture for non-terminating programs; verify a looping program and an `INPUT` program both produce captures.
- [x] 3.5 Implement manifest-driven keyboard and joystick input scripts; verify at least one `INPUT` program and one `ON`/`STICK` program advance past their waiting state.

## 4. Validate RAM and VRAM

- [x] 4.1 Implement RAM probe checks against configured addresses and expected values; verify a matching probe passes and a mismatching probe fails with the address and values reported.
- [x] 4.2 Implement stable RAM snapshot comparison with configurable volatile ranges; verify a changed stable byte fails and a changed volatile byte does not.
- [x] 4.3 Implement text-mode VRAM extraction from the name table; verify a `SCREEN 0/1` program's printed text is compared against configured expected text.
- [x] 4.4 Implement graphical-mode VRAM region hashing for `SCREEN 2` and `SCREEN 5+`; verify a region difference fails and reports the region name and range.
- [x] 4.5 Implement screenshot capture as a diagnostic/fallback check only; verify a program with memory checks passes without a screenshot and a program without memory checks records that its decision was visual.

## 5. Baselines, volatility, and reporting

- [x] 5.1 Implement baseline generation that runs each selected program multiple times and stores only stable bytes or region hashes; verify two consecutive baseline runs produce the same stable baseline.
- [x] 5.2 Implement baseline refresh limited to the selected scenario or program; verify unselected baselines are unchanged.
- [x] 5.3 Implement volatile-program handling for `RND`/`TIME`/clock-based programs; verify differing volatile bytes do not fail a volatile program while its stable probes still run.
- [x] 5.4 Implement per-program artifact output (RAM, VRAM, optional screenshot) and a summary report with pass/fail and failure cause; verify a passing run and a deliberately failing run both produce the expected report entries.

## 6. Integration verification

- [x] 6.1 Run the full regression suite once to generate reviewed baselines, then run it again and verify all selected ASCII8 programs pass or are explicitly marked volatile.
- [x] 6.2 Verify `make -C tests/integration run` and the root `make test-integration` behavior are unchanged and do not invoke the regression workflow.
- [x] 6.3 Verify no CI workflow file invokes the regression target and document that the workflow is local-only.

## Workflow follow-up

- Archive the change after the project's review requirements are satisfied.
- Verify the archived result.
