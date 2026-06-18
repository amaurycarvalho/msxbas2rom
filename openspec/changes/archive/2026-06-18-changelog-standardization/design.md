## Context

The project currently has:
- `CHANGELOG.md` with a flat list format: `- version – (YYYY/MM/DD) description;`
- `info_history` in `src/cli/appinfo.h` duplicating the entire changelog as a C raw string literal (~260 lines)
- `debian/changelog` (Debian format) and `rpmbuild/SPECS/msxbas2rom.spec` (RPM `%changelog` format) — these are packaging-specific and will remain unchanged
- Architecture spec at `openspec/specs/architecture/spec.md` requiring version replication across files but not specifying changelog format

Key constraints:
- `info_history` is a compile-time string constant — no runtime generation
- `debian/changelog` and RPM spec changelog use their own formats and are out of scope
- The versioning scheme is MAJOR.MINOR.PATCH.BUILD (4-part) and must remain consistent across all files

## Goals / Non-Goals

**Goals:**
- Convert CHANGELOG.md to Keep a Changelog format with categorized sections
- Migrate all dates to ISO 8601 (YYYY-MM-DD)
- Replace `#.#.#.#` placeholder versions with `## [Unreleased]` sections
- Shorten `info_history` to show current release + last 2 releases summary + GitHub release link
- Update architecture and CLI specs to document the new conventions
- Add sync instructions in appinfo.h comments

**Non-Goals:**
- Changing debian/changelog or RPM spec format
- Auto-generating info_history from CHANGELOG.md (manual sync is acceptable)
- Backfilling categories for historical entries (existing entries will be classified based on their descriptions)
- Changing the `--history` CLI flag name or behavior semantics (still prints info_history)

## Decisions

### Decision 1: Keep a Changelog as the format standard
**Why:** It is the de facto standard for open-source changelogs, well-known, and human-readable.

Categories used: `Added`, `Changed`, `Deprecated`, `Removed`, `Fixed`, `Security`
Each release gets a `## [version] - YYYY-MM-DD` header.
The `## [Unreleased]` header holds changes not yet released.

### Decision 2: info_history shows current release + last 2 + release link
**Why:** Reduces binary size and maintenance burden. Users who want full history use `CHANGELOG.md` or `git log`.

Format:
```
Version history

## [current] - YYYY-MM-DD
[Current release entries...]

## [last-1] - YYYY-MM-DD
[Summary of last-1...]

## [last-2] - YYYY-MM-DD
[Summary of last-2...]

See full changelog:
https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v[current]
```

### Decision 3: Manual sync, documented in comments
**Why:** No build-time code generation needed. The `appinfo.h` comment already warns about syncing — it will be updated to reference the changelog sync rule.

### Decision 4: Historical entries categorized by description
**Why:** Rather than leaving the full history uncategorized, each historical entry will be assigned to the most appropriate category (Added, Fixed, Changed) based on its description. Entries that clearly describe bug fixes → `Fixed`, new features → `Added`, refactors/optimizations → `Changed`.

## Risks / Trade-offs

- **Risk:** Historical entries may be miscategorized → low impact, can be corrected later
- **Risk:** info_history may fall out of sync with CHANGELOG.md → mitigation: clear documentation in appinfo.h comments and a task in the release checklist
- **Trade-off:** Manual sync vs auto-generation — manual sync is simpler for a C project without a build script layer; if it proves error-prone, a future change could add a script to extract the current release section from CHANGELOG.md
