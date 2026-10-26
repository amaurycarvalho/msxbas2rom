# Design

## Context

See `proposal.md` for motivation. The current `lint` target is a single compiler invocation over `SRC_FILES` (all 300 source files). The CI workflow calls `make lint` and uses the default checkout; no lint range or changed-file information reaches the build. Locally, the same target always lints the whole tree. The `Makefile` already relies on POSIX shell tools and defines `SRC_FILES` at line 92, so the change can stay in the existing build file.

## Goals / Non-Goals

**Goals:**

- Keep CI lint behavior byte-for-byte equivalent from the workflow's point of view.
- Make local `make lint` proportional to the changed `.cpp` set.
- Provide an explicit full local lint command and keep the local quality gate full.
- Avoid new scripts, dependencies, or CI workflow edits.

**Non-Goals:**

- Changed-file detection in CI or PR range handling.
- Reverse-dependency analysis for headers.
- Parallel lint execution, caching, or `-fanalyzer` changes.
- Changing the lint flags or the set of files considered source.

## Decisions

**Decision 1 - Detect CI with `CI=true` in the `Makefile`.**
- Chosen: the `lint` recipe branches on the `CI` make variable, which GitHub Actions sets to `true`.
- Rejected: changing `ci.yml` to call `make lint-all` (touches CI and adds a second contract), and adding a separate `lint-changed` target (local command would not be `make lint`).

**Decision 2 - Select local files with `git diff HEAD` plus untracked files.**
- Chosen: `git diff --name-only --diff-filter=ACMR HEAD -- 'src/*.cpp'` covers staged and unstaged tracked changes; `git ls-files --others --exclude-standard -- 'src/*.cpp'` adds new files. `ACMR` excludes deletions.
- Rejected: parsing `git status --porcelain` (more state handling and quoting), and merge-base ranges (slower and outside the local working-tree scope).

**Decision 3 - Remove the header/Makefile force-full rule.**
- Chosen: local lint always uses the changed `.cpp` set. Header-only or Makefile-only changes produce no compilation; the skip message points to `make lint-all`. This keeps the local loop consistently fast even though header fanout is high.
- Rejected: forcing full lint on headers/Makefile (the rule this change removes), and `.d`-based reverse dependencies (more precise but not minimal; `.d` files are absent in CI and can be stale).

**Decision 4 - Fall back to full lint without Git.**
- Chosen: if `git rev-parse --git-dir` fails, lint all sources. Source archives and non-Git workspaces keep the previous behavior.
- Rejected: skipping lint without Git (silently weaker behavior).

**Decision 5 - Add `lint-all` and keep `quality-gate` full.**
- Chosen: `lint-all` runs `SRC_FILES` without `-fanalyzer`; `quality-gate` depends on `lint-all`. `lint-full` remains the full analyzer target.
- Rejected: reusing `lint-full` for the full local check (it is much slower and has a different contract), and leaving `quality-gate` on the incremental target (would silently weaken the local gate).

**Decision 6 - Keep the logic in the `Makefile` recipe.**
- Chosen: a short POSIX shell block inside the existing target. No new files, no new runtime dependency, and the existing `SRC_FILES` variable is reused.
- Rejected: a Python helper (more testable, but disproportionate for this scope) and a dedicated shell script (new file and path handling for no clear gain).

**Decision 7 - Make failure and skip states explicit.**
- Chosen: the compiler command and pass message are chained with `&&`, so a failing file prevents the pass message and propagates a non-zero status. The skip branch prints only guidance.
- Rejected: a trailing unconditional pass message, which can mask a compiler failure or report success after a skip.

## Risks / Trade-offs

- [Header-only change is not linted locally] -> The skip message recommends `make lint-all`; CI still lints the full tree, and `quality-gate` is full.
- [Build-flag changes are only tested on changed files locally] -> Document `make lint-all` after `Makefile` edits; CI catches the full-tree result.
- [Committed branch changes are not visible with a `HEAD` base] -> CI covers them on push; developers can run `make lint-all` or `make quality-gate` before pushing.
- [Non-GitHub CI does not set `CI=true`] -> Document that CI must set `CI=true` to retain full-tree behavior; otherwise the workflow can call `make lint-all`.
- [Git paths with spaces would split in the shell] -> No source path contains spaces; the existing build already assumes space-free relative paths.
- [No automated test for the selection shell] -> The spec scenarios are validated manually during implementation; keeping the logic in one recipe limits the surface area.
