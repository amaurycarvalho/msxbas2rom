# Tasks

## 1. Compressor API and internal buffer

- [ ] 1.1 Add `Pletter::maxPackedSize(n)` as a `constexpr` helper returning `n + (n + 43) / 8`; verify in a new `tests/unit/src/test_pletter.cpp` that the values for `n=1`, `5`, `6`, `200`, and `8192` are `6`, `11`, `12`, `230`, and `9221` respectively.
- [ ] 1.2 Change `Pletter::pack` to return the compressed payload through `std::vector<unsigned char>&`, returning `0` for null or empty input; verify `tests/unit/src/test_pletter.cpp` covers 1..6-byte inputs, empty input, null input, exact output size, and `size <= maxPackedSize(n)`.
- [ ] 1.3 Make `Pletter::saves` grow dynamically (`clear`, optional `reserve`, `push_back`) and remove the `length * 2` assumption and the `dp` indexing dependency; verify the 1..6-byte tests and the 8K test pass with `-D_GLIBCXX_ASSERTIONS`.
- [ ] 1.4 Capture compressed bytes for representative inputs from the pre-change revision (for example 96-byte constant, incrementing, and 8K constant data), add them as expected byte sequences in `tests/unit/src/test_pletter.cpp`, and verify the new implementation matches byte-for-byte.

## 2. Resource readers

- [ ] 2.1 Update `ResourceBlobPackedReader::pack` to use the vector output and replace `data[0]` with the compressed vector, preserving the 8K limit and `packedSize`; verify the 1..6-byte and 8K resource tests in `tests/unit/src/test_resources.cpp`.
- [ ] 2.2 Update `ResourceBlobChunkPackedReader::load` to use the vector output, keep 200-byte chunks and the one-byte size prefix, reject `packed.size() > 255`, and add a named chunk-size constant plus `static_assert(Pletter::maxPackedSize(kChunkSize) <= 255)`; verify existing chunk tests and a new incompressible 200-byte chunk test.
- [ ] 2.3 Add resource-level regression cases for 1, 2, 3, 4, 5, and 6-byte blobs, empty input, and the size-bound contract; verify `make test-unit` passes.
- [ ] 2.4 Add a Fixed entry under `[Unreleased]` in `CHANGELOG.md` linking this change and describing the small-blob overflow fix; verify the link resolves and `openspec validate fix-pletter-buffer-overflow --type change` passes.

## 3. Test hardening

- [ ] 3.1 Add `-D_GLIBCXX_ASSERTIONS` to the regular unit-test build in `tests/unit/Makefile`; verify `make test-unit` passes and the 1-byte test no longer aborts.
- [ ] 3.2 Add a sanitizer mode/target (`SANITIZE=1`) to `tests/unit/Makefile` using `-fsanitize=address,undefined -fno-omit-frame-pointer`, a non-static link, separate object/binary directories, and no coverage flags; verify `make test-sanitize` passes without clobbering `bin/test_unit`.
- [ ] 3.3 Add a CI job for the hardened test path in `.github/workflows/ci.yml`; verify the job is green on a branch/PR and, as a negative control, fails against a temporary copy of the pre-fix `2 * n` buffer.
- [ ] 3.4 Document the hardened commands in `CONTRIBUTING.md`; verify the documented `make test-unit` and `make test-sanitize` commands run as written.

## 4. Integration and release checks

- [ ] 4.1 Run `make test`, `make test-coverage`, `make test-architecture`, and `make test-integration`; verify all pass and coverage remains at least 85%.
- [ ] 4.2 Run the emulator/regression path that exercises `tests/integration/MISC/rsc1.bas` (`CMD RSCTORAM` on a Pletter-compressed resource) and verify the decompressed bytes match the source; if no automated assertion covers it, add the smallest emulator assertion for that program.
- [ ] 4.3 Run `make test-regression` and verify no existing ROM baseline changes for resources that already compressed successfully.
- [ ] 4.4 Build the Debian package with `make debian` and, when available, run the build in an Ubuntu 26.04 container; verify `dh_auto_test` passes under `_GLIBCXX_ASSERTIONS`.

## Workflow follow-up

- After merge/release, confirm the Launchpad builds for Ubuntu 26.04 (`resolute`) and 26.10 (`stonking`) succeed.
- Archive the change after review.
- During release, run the changelog workflow and verify the Unreleased entry is moved under the release version.
