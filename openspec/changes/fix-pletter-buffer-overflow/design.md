# Design

## Context

See `proposal.md` for the failure and motivation. The current compressor has two unsafe assumptions:

- `Pletter::saves::init` allocates `length * 2` bytes and writes control/data bytes through `operator[]`.
- Callers allocate `std::vector` destinations with `size * 2` and pass a raw pointer to `Pletter::pack`.

A 1-byte input produces 6 compressed bytes, so both buffers are too small. `_GLIBCXX_ASSERTIONS` catches the internal vector write; AddressSanitizer catches the final `memcpy` into the caller's vector. The compressed byte format and the kernel decoder are not changing.

The unit suite currently builds with the project Makefile, not with distro hardening flags. The failing PPA environments are the only ones that expose the bug because their libstdc++ checks `operator[]`.

## Goals / Non-Goals

**Goals:**

- Remove the raw destination buffer and the fixed `2 * n` internal buffer from the compression path.
- Keep the Pletter output format, chunk format, and ROM payloads unchanged for inputs that already worked.
- Define one output-size bound used by tests and chunk validation.
- Add regression coverage for 1..6-byte blobs and a hardened test execution path.

**Non-Goals:**

- Changing the compression algorithm or kernel decompression routine.
- Changing the 8K per-resource limit, the 200-byte chunk size, or the 1-byte block-size field.
- Reworking the resource map or ROM layout.
- Adding a C++ decompressor.

## Decisions

### Decision 1: Return the compressed payload through `std::vector<unsigned char>`

`Pletter::pack` will take an output reference:

```cpp
int pack(const unsigned char* pData, int dataSize,
         std::vector<unsigned char>& out);
```

It returns the number of compressed bytes, or `0` for invalid input (null pointer or empty input). The vector owns the exact output; callers no longer provide capacity and cannot under-size a destination.

Alternatives considered:

- **Raw pointer plus capacity parameter (2A):** smaller API change, but callers still choose the capacity and a wrong choice becomes a runtime error rather than a compile-time contract. Rejected in favor of the owned output requested for this change.
- **Keep `unsigned char*` and add an internal-only safe buffer:** leaves the unsafe public contract intact and still requires the caller to calculate a bound. Rejected.

The return type stays `int` because `ResourceReader::packedSize` is an `int` and the maximum blob is 8192 bytes. A `std::size_t` return would force more call-site churn for no practical benefit.

### Decision 2: Make the internal `saves` buffer grow dynamically

`saves::init` will stop pre-sizing to `length * 2`:

- `init`: set counters and `buf.clear()`; optionally `buf.reserve(maxPackedSize(length))` for reuse.
- `claimevent`: `ep = buf.size(); buf.push_back(0);` to reserve a control byte.
- `adddata`: `buf.push_back(d);`.
- `addevent`: write the accumulated control byte at `ep`.
- `done`: pad the last control byte, then return the buffer (moved or copied into `out`).

`dp` can be replaced by `buf.size()`; if it is kept, it must be updated after each `push_back`. The existing `ep`, `p`, and `e` semantics stay the same. The buffer capacity is reused across chunked calls because `clear()` preserves capacity.

Alternatives considered:

- **Keep the fixed buffer and size it with the new bound:** fixes this case but leaves the compressor dependent on a hand-maintained bound and keeps latent overflow risk if the format changes. Rejected as the final design; the bound is still exposed for validation.
- **Use `std::array` or a larger stack buffer:** still requires a hard-coded maximum and does not remove the caller-side risk.

### Decision 3: Define the bound as the literal worst case

The format emits:

- 3 header bits,
- one control bit per literal byte,
- 34 terminator bits,
- and one payload byte per literal byte (the first byte is stored without a literal flag).

For `n` bytes, the literal path therefore needs `n + 36` control bits and `n` data bytes:

```text
maxPackedSize(n) = n + ceil((n + 36) / 8)
                 = n + (n + 43) / 8      // integer division
```

Measured output agrees: `n=1 -> 6`, `n=5 -> 11`, `n=6 -> 12`, `n=200 -> 230`.

Matches only reduce control bits and payload bytes relative to the literal path because the optimizer only selects a match when its cost is strictly lower than the literal cost. The bound is used for `reserve`, chunk validation, and tests. It is not needed for safety once the output vector grows dynamically, but it is the contract that tests assert.

`Pletter::pack` sets `savelength = false`; if that ever changes, the format adds two payload bytes and the bound must be increased by two. The design records this explicitly and a test will fail if the bound is exceeded.

### Decision 4: Update both resource readers to consume the vector output

`ResourceBlobPackedReader::pack`:

- keep the existing 8K check before compression;
- create a local `std::vector<unsigned char> packed`;
- call the new `Pletter::pack` with the uncompressed `data[0]`;
- on success, swap `data[0]` with `packed` and set `packedSize = bytesPacked`.

This removes the `data.emplace_back(size * 2)` / `data.erase(data.begin())` sequence entirely.

`ResourceBlobChunkPackedReader::load`:

- keep the 200-byte chunk size and the 1-byte block-size prefix;
- replace the 1024-byte stack buffer with a local vector per chunk;
- reject `packed.size() > 255` with the existing diagnostic;
- append `packed.size() + 1` bytes to `data`, store the size byte, and copy the payload.

The per-chunk vector is small (at most about 230 bytes) and the number of chunks for a 16K resource is bounded, so the extra allocation is negligible. The block-count and total-size logic stays unchanged.

### Decision 5: Validate the chunk constants with the bound

The chunk reader uses 200-byte chunks and a 1-byte size field. The largest input whose worst-case packed size still fits 255 bytes is 222 bytes:

```text
222 + (222 + 43) / 8 = 255
223 + (223 + 43) / 8 = 256
```

Keep 200, which has margin, and add a compile-time check that the configured chunk size remains within the 1-byte field:

```cpp
static_assert(Pletter::maxPackedSize(kChunkSize) <= 255,
              "chunk compression must fit the 1-byte block size");
```

Make `maxPackedSize` `constexpr` with a single return expression so it can be used in `static_assert` under C++11. The 1024-byte stack buffer disappears; the vector output handles the actual size.

### Decision 6: Harden the test execution path

- Add `-D_GLIBCXX_ASSERTIONS` to the regular unit-test build so vector bounds errors fail on any Linux toolchain.
- Add a sanitizer target that builds and runs the unit suite with `-fsanitize=address,undefined -fno-omit-frame-pointer`. Because the normal unit link uses `-static -static-libstdc++ -static-libgcc`, the sanitizer target must use a non-static link and ideally a separate binary/object directory so it does not clobber the normal build. Disable `--coverage` for this target.
- If the sanitizer build is too invasive, Valgrind on the normal binary is an acceptable fallback; it catches the raw `memcpy` write even when `_GLIBCXX_ASSERTIONS` is off.
- CI runs the hardened path in addition to the existing coverage and integration jobs.

Alternatives considered:

- **Only add `_GLIBCXX_ASSERTIONS`:** catches the internal vector write but not the raw `memcpy` into the caller's vector. Not sufficient.
- **Only add ASan:** catches both, but a checked-iterator build is cheap and gives faster local feedback. Both are useful.

### Decision 7: Keep tests in the existing in-memory resource fixtures

The resource tests already use `InMemoryResourceContentSource` and `createTempFile` helpers that do not touch the filesystem. New cases stay in `tests/unit/src/test_resources.cpp` and use the same helpers, preserving the unit/integration boundary. No new project-relative temporary paths or `.bas` files are introduced.

## Risks / Trade-offs

- **Compressed bytes could change accidentally** → Keep the emission order unchanged; add golden-byte or integration regression checks for representative inputs; the vector output only changes ownership, not the sequence of `add*` calls.
- **ASan and the static link are incompatible** → Use a dedicated non-static sanitizer target with separate output paths; fall back to Valgrind if needed.
- **Per-chunk allocation** → Chunks are at most 200 bytes; allocation cost is negligible. If profiling shows an issue, reserve/reuse a vector in the chunk reader.
- **The literal bound is tied to the current format** → Keep the derivation in the design and add a test that asserts actual output never exceeds it. Add two bytes to the bound if `savelength` is enabled.
- **The 1-byte block size is coupled to the chunk size** → Keep 200 and add a `static_assert` against the bound; do not silently truncate `packed.size()`.
- **Internal API churn** → There are only two call sites; compile errors identify them. No BASIC, kernel, or ROM ABI change.
- **Allocation failure** → `std::bad_alloc` remains an exceptional condition, as before. Input validation (null, empty) returns 0 and follows the existing resource error path.

## Migration Plan

1. Add the vector API, `maxPackedSize`, and dynamic `saves` buffer.
2. Update `ResourceBlobPackedReader` and `ResourceBlobChunkPackedReader`.
3. Add regression tests for 1..6-byte blobs, the 8K boundary, empty input, and the size bound.
4. Add `_GLIBCXX_ASSERTIONS` to the unit build and a sanitizer/Valgrind CI path.
5. Run `make test`, the sanitizer target, `make test-coverage`, and `make test-architecture`.
6. Verify a PPA-style build on Ubuntu 26.04/26.10 or the closest available container.

Rollback is a normal revert: resource files, ROM contents for valid inputs, and the kernel decoder are unchanged.
