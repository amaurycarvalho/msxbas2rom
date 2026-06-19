## Why

The current CMD MTF implementation supports only full-screen map copies with relative or absolute coordinates. For tile-based games with scrolling or partial screen updates (HUD overlays, dialog boxes, minimaps), a window-copy operation is essential. Additionally, screen page support enables double-buffering for smooth scrolling on MSX2.

## What Changes

- Add MTF operation 2: window-copy (partial map copy with configurable source rectangle and screen destination)
- Add screen page parameter to all map operations (0, 1, 2) — MSX2 only, ignored on MSX1
- Migrate parameter passing from Z80 registers to RAM workarea block (DAC) for all operations, enabling up to 9 parameters
- Extend compiler handler to accept 1–9 parameters (was 1–4)
- Add unit tests for all parameter combinations and an integration test (`tests/integration/mtf/mtf5.bas`)
- Use incremental builds — no forced `make clean`

## Capabilities

### New Capabilities
- `mtf-commands`: Full MTF command syntax specification covering palette/tileset loading, map copies (relative, absolute, window), and screen page support

### Modified Capabilities
- *(none — existing MTF behavior is preserved as defaults)*

## Impact

- `src/application/compiler/statements/strategies/io/cmd/handlers/graphics/mtf/` — compiler handler: parameter count, RAM-block emit
- `src/application/compiler/helpers/hooks/compiler_hooks.h` — kernel address constants
- `src/infrastructure/kernel/asm/src/header/31_cmd.asm` — kernel runtime: window-copy, page offset, unified RAM reading
- `src/infrastructure/kernel/asm/src/header.symbols.asm` — new workarea symbols
- `tests/unit/src/test_compiler.cpp` — extended MTF parameter tests
- `tests/integration/mtf/mtf5.bas` — window-copy integration test
- Release 1.1.0.0
