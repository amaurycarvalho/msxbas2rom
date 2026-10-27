# Tasks

## 1. Shared validation and resource metadata

- [ ] 1.1 Add compiler helpers for declared-array checks, literal resource lookup, resource size/kind lookup, and signed literal ranges; verify helper unit tests.
- [ ] 1.2 Add a table-driven parameter case structure for command/function arity and type tests; verify it is used by at least the new sprite/MTF/resource cases.

## 2. Sprite buffers and indices

- [ ] 2.1 Require a declared array for `SET/GET SPRITE PATTERN/COLOR` using the existing declared-array helper; verify scalar and literal buffers fail and `DIM PB%(3,3)`/`DIM CB%(7)` pass.
- [ ] 2.2 Reject literal sprite indices outside `0..31` for PUT/COLOR/SET/GET sprite commands while keeping `COLLISION(-1)` valid; verify pass/fail cases in `tests/unit/src/test_compiler_graphics.cpp` or equivalent.
- [ ] 2.3 Document the sprite buffer/index contract in `sprites-handling` tests and verify the spec delta scenarios.

## 3. Tile literals

- [ ] 3.1 Reject negative literal tile banks for SET/GET TILE while preserving `0..2` and `>=3` meanings; verify unit cases for SET/GET pattern/color/flip/rotate.

## 4. MTF parameters

- [ ] 4.1 Validate MTF operation `0..2`, negative literal window sizes, and negative literal destination coordinates.
- [ ] 4.2 Preserve negative `map_x`/`map_y` literals for operations 1 and 2 and add a positive compilation test for `CMD MTF 2,1,-10,-5`.
- [ ] 4.3 Verify destination coordinates above screen bounds still compile and rely on runtime clipping.

## 5. Resources and RUNASM

- [ ] 5.1 Reject literal resource numbers outside the compiled resource map for resource-consuming commands; verify with a one-resource program using resource `0` and `1`.
- [ ] 5.2 Reject literal `CMD RUNASM` resources larger than 255 bytes or stored compressed; verify with 255-byte, 256-byte, and compressed fixtures.

## 6. Integration and documentation

- [ ] 6.1 Add CLI/unit negative cases for scalar sprite buffers, out-of-range literal indices, missing literal resources, oversized RUNASM, and invalid MTF literals.
- [ ] 6.2 Update affected specs/docs with the preserved negative-map-coordinate rule and the new literal validation; verify `openspec validate --change harden-cpp-parameter-validation-tier2`.
- [ ] 6.3 Run `make test-unit` and `make test-integration` and record the result.

## Workflow follow-up

- Archive the change after review.
