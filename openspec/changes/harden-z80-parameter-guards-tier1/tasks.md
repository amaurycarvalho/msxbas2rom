# Tasks

## 1. Fixed sprite tables

- [ ] 1.1 Mask/validate sprite numbers in `SUB_GETSPRTBL_ADDR` and `SUB_GET_HITBOX_ADDR`; verify `PUT SPRITE 32` and `SET SPRITE HITBOX 32` leave `PLYBUF`/`SPRTBL` canaries unchanged.
- [ ] 1.2 Apply the same safe index handling to sprite pattern/color and collision table accesses; verify openMSX canary probes.

## 2. Direction normalization

- [ ] 2.1 Normalize runtime directions in `set_tile_flip`, `set_tile_rotate`, `set_sprite_flip`, and `set_sprite_rotate`; verify directions `3` and `-1` produce the documented safe result.
- [ ] 2.2 Add unit/regression coverage for valid `0..2` directions to ensure existing behavior is unchanged.

## 3. Resource failure and size checks

- [ ] 3.1 Check the resource lookup failure result in `CMD MTF`, `CMD WRTFNT`, `CMD WRTSPR`, `CMD RESTORE`, `CMD PLYLOAD`, and `CMD WRTVRAM`; verify missing-resource programs return without memory writes.
- [ ] 3.2 Check `BC >= 7` in `CMD WRTFNT` and skip short resources; verify a malformed short font does not produce an underflowed copy.
- [ ] 3.3 Verify `CMD RSCTORAM` remains the reference behavior and its existing tests still pass.

## 4. RUNASM and PLYSOUND guards

- [ ] 4.1 Make `CMD RUNASM` obtain the real resource size, reject missing/oversized/non-executable resources, and never jump into a 255-byte truncation; verify a 256-byte resource selected through a variable returns to the BASIC marker.
- [ ] 4.2 Guard `cmd_plysound` against effect `0`, channel `>2`, and inverted volume `>16`; verify safe return and no out-of-range channel/table writes.

## 5. Verification

- [ ] 5.1 Add emulator regression programs/probes for sprite-table canaries, missing/short resources, RUNASM size, and PLYSOUND invalid values.
- [ ] 5.2 Build the kernel and run `make test-kernel`, relevant unit tests, and the selected emulator/regression scenarios; record results.
- [ ] 5.3 Update kernel documentation/comments for the normalized index and failure behavior; verify `openspec validate --change harden-z80-parameter-guards-tier1`.

## Workflow follow-up

- Archive the change after review.
