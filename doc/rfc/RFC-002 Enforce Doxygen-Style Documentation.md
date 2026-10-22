# RFC-002: Enforce Doxygen-Style Documentation in C++

| Field | Value |
|---|---|
| Status | Draft |
| Date | 2026-10-08 |
| Scope | C++ source code, build system, CI quality gate |

## Problem

The Python codebase uses `flake8-annotations` and `flake8-docstrings` to enforce type annotations and docstrings automatically. The C++ codebase has no equivalent mandatory documentation check.

Current state:

- `.clang-tidy` enables static analysis and style checks, but no documentation checks.
- `WarningsAsErrors` is empty, so documentation-related diagnostics would not fail the build.
- `Makefile` runs `g++ -fsyntax-only -Wall -Wextra -Werror -pedantic` as `lint`, but this does not validate Doxygen comments or require their presence.
- `.clang-format` only handles formatting.
- There is no `Doxyfile`, no `docs-check` target, and no CI step that fails on missing or invalid documentation.
- Public classes, functions, parameters, and return values can be merged without documentation.

This creates inconsistent API documentation, makes onboarding harder, and allows documentation debt to grow silently.

## Root Cause

The root cause is the absence of a documentation-specific quality gate. The existing tooling checks syntax, style, and static-analysis issues, but none of it treats missing or malformed Doxygen comments as an error.

Specifically:

- `.clang-tidy` does not include `clang-diagnostic-documentation`.
- `Makefile` does not invoke Doxygen or any documentation coverage tool.
- `quality-gate` does not include a documentation step.
- No project-wide Doxygen configuration exists to define what “documented” means.

## Options

### Option 1: Clang `-Wdocumentation` / `clang-diagnostic-documentation`

Use Clang’s built-in documentation warnings through `clang-tidy`.

**Pros:** Fast, already available with Clang, catches malformed tags and unknown commands.  
**Cons:** Does not enforce the presence of documentation. It only validates comments that already exist.

### Option 2: Doxygen with `WARN_AS_ERROR`

Configure Doxygen to warn on undocumented members and treat warnings as errors.

**Pros:** Directly detects missing documentation. Can enforce parameter and return documentation. Can be scoped to public APIs.  
**Cons:** Requires a `Doxyfile`. Can be noisy during initial adoption. Slower than a syntax-only lint.

### Option 3: Cpplint

Use `cpplint` to enforce Google-style comment rules.

**Pros:** Lightweight, easy to run.  
**Cons:** Not Doxygen-aware. Does not guarantee complete API documentation.

### Option 4: Coverxygen coverage threshold

Use Doxygen XML output plus Coverxygen to measure documentation coverage and fail below a threshold.

**Pros:** Gradual adoption possible. Gives measurable coverage numbers.  
**Cons:** Extra tooling and configuration. Threshold must be maintained.

### Option 5: Custom Clang plugin or script

Write a custom checker for project-specific rules.

**Pros:** Maximum control.  
**Cons:** High maintenance cost. Not justified unless requirements are very unusual.

## Better Solution

Adopt a combined approach:

1. **Doxygen is the source of truth** for documentation presence and completeness.
2. **Clang-Tidy validates Doxygen syntax** and catches malformed tags early.
3. **Makefile exposes local targets** so developers can run the same checks as CI.
4. **`quality-gate` fails** if documentation is missing or invalid.
5. **Optionally add Coverxygen** only if Doxygen’s `WARN_IF_UNDOCUMENTED` proves too strict during migration.

### Required Changes to Attached Files

#### 1. `.clang-tidy`

Add documentation diagnostics and make them fail as errors.

```yaml
Checks: >
  clang-analyzer-*,     # static analysis engine
  bugprone-*,           # static analysis engine
  google-*,             # google clang checks
  modernize-*,          # safer, modern constructs
  performance-*,        # can catch inefficient or risky code
  portability-*,
  readability-*,
  clang-diagnostic-documentation,
  clang-diagnostic-documentation-unknown-command,
  -modernize-use-trailing-return-type,
  -readability-magic-numbers

WarningsAsErrors: "clang-diagnostic-documentation*"
```

This makes malformed Doxygen comments fail the lint. It does not replace Doxygen’s presence checks.

#### 2. `.clang-format`

No mandatory change is required. Optionally, enforce comment reflow:

```yaml
BasedOnStyle: Google
AllowShortFunctionsOnASingleLine: Empty
ReflowComments: true
```

Formatting alone cannot guarantee documentation.

#### 3. `Makefile`

Add Doxygen variables and targets. Then update `quality-gate` to include documentation checks.

Suggested additions:

```make
# ----------------------------
# Documentation
# ----------------------------

DOXYGEN = doxygen
DOXYFILE = Doxyfile
DOXYGEN_OUT = docs/doxygen
DOXYGEN_WARN = $(DOXYGEN_OUT)/doxygen-warnings.log

docs:
	@echo "📚 Generating documentation..."
	@$(DOXYGEN) $(DOXYFILE)
	@echo "✅ Documentation generated in $(DOXYGEN_OUT)"

docs-check:
	@echo "🔍 Checking Doxygen documentation..."
	@$(DOXYGEN) $(DOXYFILE)
	@echo "✅ Doxygen documentation check passed!"

docs-clean:
	@echo "🧹 Cleaning documentation artifacts..."
	@rm -rf $(DOXYGEN_OUT)
	@echo "✅ Documentation cleaned"
```

Add a Clang-Tidy target. This assumes a `compile_commands.json` exists, for example generated with `bear -- make debug` or `compiledb make debug`.

```make
lint-tidy:
	@echo "🔍 Running clang-tidy..."
	@find $(SRC) -name '*.cpp' -print0 | xargs -0 clang-tidy -p .
	@echo "✅ clang-tidy passed!"
```

Update `quality-gate`:

```make
quality-gate: lint lint-tidy docs-check test-coverage coverage test-kernel test-integration
	@echo "✅ Quality gate completed"
```

This makes missing or invalid documentation block the quality gate.

#### 4. New File: `Doxyfile`

Create a `Doxyfile` with mandatory documentation warnings:

```text
PROJECT_NAME           = msxbas2rom
OUTPUT_DIRECTORY       = docs/doxygen
INPUT                  = src
RECURSIVE              = YES
FILE_PATTERNS          = *.h *.hpp *.cpp
EXTRACT_ALL            = NO
EXTRACT_PRIVATE        = NO
WARN_IF_UNDOCUMENTED   = YES
WARN_IF_DOC_ERROR      = YES
WARN_NO_PARAMDOC       = YES
WARN_AS_ERROR          = YES
GENERATE_HTML          = YES
GENERATE_LATEX         = NO
HAVE_DOT               = NO
QUIET                  = NO
```

With `WARN_AS_ERROR = YES`, Doxygen exits non-zero when warnings are emitted. That is the key enforcement mechanism.

#### 5. Optional New File: `scripts/check-doxygen-coverage.sh`

If `WARN_IF_UNDOCUMENTED` is too strict initially, use Coverxygen for a coverage threshold instead.

Example policy:

- Phase 1: warn only, target 60% public API coverage.
- Phase 2: fail below 80%.
- Phase 3: fail below 95%.

#### 6. Optional CI Workflow

Add a CI step equivalent to:

```yaml
- name: Documentation check
  run: make docs-check
- name: Clang-Tidy check
  run: make lint-tidy
```

#### 7. Documentation Standard

Add a short section to `CONTRIBUTING.md`:

- Every public header, class, struct, enum, and function must use Doxygen comments.
- Use `@brief`, `@param`, `@return`, and `@throws` where applicable.
- Example:

```cpp
/**
 * @brief Converts an MSX BASIC source file into a ROM image.
 *
 * @param sourcePath Path to the BASIC source file.
 * @param outputPath Path where the ROM image will be written.
 * @return true on success, false otherwise.
 */
bool convertToRom(const std::string& sourcePath, const std::string& outputPath);
```

## Evaluate

**Effectiveness:** Doxygen with `WARN_AS_ERROR` is the strongest direct equivalent to `flake8-docstrings`. It can fail the build when public APIs lack documentation. Clang-Tidy adds early syntax validation for Doxygen tags.

**Adoption cost:** The main cost is initial documentation debt. Scoping with `EXTRACT_PRIVATE = NO` limits enforcement to public APIs. If that is still too noisy, start with Coverxygen thresholds and tighten over time.

**Maintenance:** Low. `Doxyfile` and Makefile targets are stable. Clang-Tidy configuration is already present and only needs two additions.

**Risk:** Doxygen may warn on third-party or generated code. Mitigate with `EXCLUDE` and `EXCLUDE_PATTERNS` in `Doxyfile`.

**Recommendation:** Adopt the combined Doxygen + Clang-Tidy + Makefile quality-gate approach. It gives the C++ codebase the same practical guarantee as `flake8-annotations` and `flake8-docstrings`: documentation is no longer optional, and violations fail the build.
