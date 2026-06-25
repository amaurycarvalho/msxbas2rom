## Why

The current CMD MTF implementation supports only full-screen map copies with relative or absolute coordinates. For tile-based games with scrolling or partial screen updates (HUD overlays, dialog boxes, minimaps), a window-copy operation is essential. Additionally, screen page support enables double-buffering for smooth scrolling on MSX2.

## What Changes

- Add MTF operation 2: window-copy (partial map copy with configurable source rectangle and screen destination — preserves tiles outside the window via LDIRMV+LDIRVM, only 2 VDP transactions regardless of window size)
- Add screen page parameter scaffolding to all map operations (0, 1, 2) — compiler accepts and stores the parameter, kernel uses dummy/placeholder (always page 0 at 0x1800). Real page offset support is deferred to the `set-page-screen4` change, which will replace hardcoded VRAM addresses with BIOS variable reads (GRPNAM, GRPCGP, GRPCOL)
- Migrate parameter passing from Z80 registers to RAM workarea block (PARM1) for all operations, enabling up to 9 parameters
- Unify map copy code path: operations 0 (relative) and 1 (absolute) delegate to the same `window_copy` kernel routine by setting full-screen window parameters, eliminating ~70 bytes of duplicate copy-loop code
- Full-width optimization: when screen_x=0 and width=32, skip LDIRMV (1 VDP transfer) — ensures ops 0/1 maintain current performance
- Extend compiler handler to accept 1–9 parameters (was 1–4)
- Add unit tests for all parameter combinations and an integration test (`tests/integration/MTF/mtf5.bas`)
- Use incremental builds — no forced `make clean`

## Capabilities

### New Capabilities
- `mtf-commands`: Full MTF command syntax specification covering palette/tileset loading, map copies (relative, absolute, window), and screen page support

### Modified Capabilities
- *(none — existing MTF behavior is preserved as defaults)*

## Impact

- `src/application/compiler/statements/strategies/io/cmd/handlers/graphics/mtf/` — compiler handler: parameter count, RAM-block emit
- `src/application/compiler/helpers/hooks/compiler_hooks.h` — kernel address constants
- `src/infrastructure/kernel/asm/src/header/31_cmd.asm` — kernel runtime: window-copy, page parameter scaffolding (dummy), unified RAM reading
- `src/infrastructure/kernel/asm/src/header.symbols.asm` — new workarea symbols
- `tests/unit/src/test_compiler.cpp` — extended MTF parameter tests
- `tests/integration/MTF/mtf5.bas` — window-copy integration test
- Release 1.2.0.0
- **Depends on**: `set-page-screen4` for real page offset support (kernel reads `(GRPNAM)` instead of hardcoded addresses)
