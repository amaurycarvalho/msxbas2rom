# Proposal

## Why

The 1.4.1.0 Launchpad builds for Ubuntu 26.04 (`resolute`, build 33879507) and 26.10 (`stonking`, build 33879508) abort in the unit suite: `ResourceBlobPackedReader round-trips a single byte` triggers a `std::vector` bounds assertion in `Pletter`. For a 1-byte blob, Pletter produces 6 bytes, but both the internal buffer and the caller's destination are sized `2 * n`, i.e. 2 bytes. Builds on older libstdc++, macOS, and MinGW pass only because their standard library does not check `operator[]`; the same write is a real heap overflow that can silently corrupt memory.

This is not a flaky or platform-specific test. The newer Ubuntu toolchains exposed a latent bug in resource compression, and the release cannot be published for those series until it is fixed. The fix also should remove the underlying unchecked-buffer contract so the same mistake cannot return through another caller.

## What Changes

- Replace `Pletter::pack`'s raw `unsigned char*` destination with a `std::vector<unsigned char>` output so the compressor returns an owned buffer with the exact compressed size. **BREAKING** for the two internal C++ call sites; no change to the BASIC language, kernel ABI, or ROM format.
- Make `Pletter::saves` grow dynamically (`clear`/`reserve`/`push_back`) instead of assuming `length * 2` bytes; this removes the fixed-buffer overflow.
- Add a single `Pletter::maxPackedSize(n)` bound (the literal worst case) for tests, reservations, and remaining capacity checks.
- Update `ResourceBlobPackedReader` and `ResourceBlobChunkPackedReader` to use the vector output. Preserve the chunked format (1-byte block size, 200-byte chunks) and the 8K resource limit.
- Keep compressed payloads byte-identical for inputs that already worked; blobs of 1..5 bytes now produce valid output instead of corrupting memory.
- Add regression tests for 1, 2, 3, 4, 5, and 6-byte blobs, the 8K boundary, empty input, and the `packedSize <= maxPackedSize(n)` contract.
- Harden the unit-test build and CI with `_GLIBCXX_ASSERTIONS` and ASan/UBSan (or Valgrind), so this class of error fails on every Linux toolchain rather than only on the newest Ubuntu.
- Document and validate the chunk reader's `200`/`255`/`1024` relationship against the new bound.

## Capabilities

### New Capabilities

- `resource-compression`: safe, size-bounded Pletter compression for every supported resource blob, with an owned output buffer and a defined output-size bound.

### Modified Capabilities

- _(none)_

## Impact

- `src/infrastructure/compression/pletter.h` and `pletter.cpp` — public API, internal `saves` buffer, size bound.
- `src/application/builder/resources/strategies/blob/resource_blob_packed_reader.cpp` and `resource_blob_chunk_packed_reader.cpp` — call sites and block sizing.
- `tests/unit/src/test_resources.cpp` and possibly `tests/unit/src/test_resources_extra.cpp` — small-blob and size-bound regression coverage.
- `tests/unit/Makefile`, `Makefile`, and CI workflow — checked-iterator and sanitizer execution.
- No ROM output change for already-working resources; no BASIC or kernel change.
- Unblocks the 26.04 (`resolute`) and 26.10 (`stonking`) PPA builds.
- Internal C++ API churn is limited to two call sites; the extra vector allocation per chunk is negligible for 200-byte chunks.
