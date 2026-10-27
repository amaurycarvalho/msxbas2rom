# Tasks

## 1. MTF safety

- [ ] 1.1 Validate the MTF map header size and reject zero width/height before coordinate math; verify a zero-header fixture returns within the regression fallback time.
- [ ] 1.2 Preserve negative `map_x`/`map_y` wrapping for operations 1 and 2; verify `CMD MTF 2,1,-1,-1` on a 32x24 map reaches the expected wrapped position.
- [ ] 1.3 Fix destination `screen_x`/`screen_y` clipping for negative/oversized values and handle 16-bit window width/height; verify probe cases for negative destination and oversized window.
- [ ] 1.4 Treat dynamic operation values outside `0..2` as a no-op; verify VRAM is unchanged.

## 2. Pattern/color and SPR bounds

- [ ] 2.1 Bound sprite and tile pattern/color indices before BIOS table address calls; verify invalid indices do not modify adjacent VRAM.
- [ ] 2.2 Check Tiny Sprite resource size against the declared sprite count in `cmd_wrtspr`; verify a truncated `.spr` fixture does not overrun the resource.

## 3. Font and mode guards

- [ ] 3.1 Bound `CMD SETFNT` style to `0..3` and bank to `0..2`/all; verify invalid dynamic values do not write outside the pattern table.
- [ ] 3.2 Bound dynamic `CMD TURBO`, `CMD PAGE`, `CMD CLIP`, and `CMD PLAY` values to their documented ranges; verify safe execution on openMSX.

## 4. Verification and documentation

- [ ] 4.1 Add MTF/sprite/tile regression programs and manifest probes for the new guards.
- [ ] 4.2 Build the kernel and run `make test-kernel`, relevant unit tests, and the MTF/GRAPH regression scenarios; record results.
- [ ] 4.3 Document the MTF negative-map versus destination-clipping rule and the dynamic parameter policies; verify `openspec validate --change harden-z80-parameter-guards-tier2`.

## Workflow follow-up

- Archive the change after review.
