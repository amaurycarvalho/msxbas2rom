---
name: openspec-changelog
description: Update CHANGELOG.md, CHANGELOG-ARCHIVE.md, debian/changelog, RPM spec, and info_history from archived changes. Run automatically after every archive.
license: MIT
compatibility: Requires openspec CLI.
metadata:
  author: openspec
  version: "1.0"
  generatedBy: "1.4.1"
---

Update all changelog-related files from archived and active changes.

**Input**: None. Always reads current version from `appinfo.h` and scans all changes automatically.

**Steps**

1. **Get current release version**

   Read `src/cli/appinfo.h` and extract the value of `app_version` (e.g., `"1.0.0.0"`). This is the latest release version.

2. **Scan all changes — both archived and active**

   List all subdirectories of `openspec/changes/archive/` and `openspec/changes/` (excluding `archive/`, `.`, `..`).

   For each change, read `proposal.md` and find the target release version by searching for a line matching `- Release <version>` in the Impact section. If the proposal does not specify a release, skip it.

3. **Classify changes**

   - **Archived changes matching latest version**: Their content goes into the release section of CHANGELOG.md.
   - **Active changes matching latest version**: Their content also goes into the same release section (these are completed or in-progress changes that target the current release).
   - **Active changes targeting a NEWER version**: Their content goes into `## [Unreleased]`.
   - **Archived changes for older versions**: Ignored (already recorded).

4. **Build the release entry for CHANGELOG.md**

   Collect the "What Changes" bullet list from `proposal.md` of all changes classified for the latest release. Use these rules:

   - If a bullet mentions a new feature, add it under `### Added`.
   - If a bullet mentions a change in existing behavior, add it under `### Changed`.
   - If a bullet mentions a bug fix, add it under `### Fixed`.
   - If a bullet mentions deprecation, add it under `### Deprecated`.
   - If a bullet mentions removals, add it under `### Removed`.
   - If a bullet mentions security, add it under `### Security`.

   Deduplicate items that appear across multiple changes. Format the date as `YYYY-MM-DD` (if not known, use today's date).

   Use this template for the release entry:

   ```markdown
   ## [version] - YYYY-MM-DD

   ### Added
   - item 1
   - item 2

   ### Changed
   - item 1

   ### Fixed
   - item 1
   ```

5. **Build the Unreleased section**

   For each active change classified for Unreleased (targets a newer version), add a single ultra-simplified summary line under `## [Unreleased]` using this pattern:

   ```markdown
   ## [Unreleased]

   ### Added
   - [change-name] Ultra-simplified summary of the change
   ```

   The summary should be 1 sentence maximum, extracted from the proposal's "Why" or "What Changes" section title.

6. **Update CHANGELOG.md**

   - Read the current `CHANGELOG.md`.
   - Move all existing release entries (everything between `## [version] - ...` sections) into `CHANGELOG-ARCHIVE.md`, appending them before the existing archive content (after the header).
   - Prepend the new release entry (from step 4) and the new Unreleased section (from step 5) to `CHANGELOG.md`, preserving the file's header and links structure.
   - Update the version compare links at the bottom of `CHANGELOG.md`:
     - `[Unreleased]: ...compare/v<new-version>...HEAD`
     - `[<version>]: ...releases/tag/v<version>`

   The updated `CHANGELOG.md` should have this structure:

   ```markdown
   # Changelog
   ...
   ## [Unreleased]
   ...
   ## [<version>] - YYYY-MM-DD
   ...
   [Unreleased]: https://github.com/amaurycarvalho/msxbas2rom/compare/v<version>...HEAD
   [<version>]: https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v<version>
   
   See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
   ```

7. **Build the ultra-simplified summary**

   Create a 1-2 line summary of the entire latest release that captures its essence. For example:

   ```
   Release 1.0.0.0: File handling support implementation (US-007)
   ```

   Use the most prominent feature of the release as the summary.

8. **Update debian/changelog**

   Write the debian changelog entry. Use the current date and author info from the git log or from existing entries. The format:

   ```
   msxbas2rom (<version>-0) bionic; urgency=low

     * <ultra-simplified summary>

    -- Amaury Carvalho <amauryspires@gmail.com>  <date>
   ```

   Where `<date>` follows Debian format: `Day, DD Mon YYYY HH:MM:SS ±TZ` (e.g., `Thu, 19 Jun 2026 18:00:00 -0300`).

   **Replace the entire debian/changelog content** — remove any previous release entries.

9. **Update rpmbuild/SPECS/msxbas2rom.spec**

   Update the `%changelog` section. The format:

   ```
   %changelog
   * <Day Mon DD YYYY> Amaury Carvalho <amauryspires@gmail.com>
   - <ultra-simplified summary>
   ```

   **Replace the entire %changelog section** — remove any previous entries.

   Also update the `Version:` field at the top of the spec file to match `app_version`.

10. **Update info_history in appinfo.h**

    Replace the `info_history` string content so it contains:

    ```
    Version history

    ## [<version>] - YYYY-MM-DD

    <ultra-simplified summary>

    See full changelog:
    https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v<version>
    ```

    **Replace the entire `info_history` string** — remove any previous release entries.

11. **Verify consistency**

    Read back all modified files and verify:
    - `CHANGELOG.md` has the release entry for the latest version
    - `CHANGELOG.md` has the Unreleased section (if there are active unarchived changes)
    - `CHANGELOG-ARCHIVE.md` has the previous releases appended
    - `debian/changelog` has only the latest release entry
    - `rpmbuild/SPECS/msxbas2rom.spec` has only the latest release entry in %changelog
    - `src/cli/appinfo.h` info_history has only the latest release summary

**Heuristics for categorization**

- Read the "What Changes" bullets from `proposal.md`. The bullet text often starts with verbs like "Implement", "Add", "Fix", "Remove", "Change", "Deprecate".
- If a bullet starts with "Implement" or "Add" → `### Added`
- If a bullet starts with "Change" or "Migrate" or "Update" → `### Changed`
- If a bullet starts with "Fix" → `### Fixed`
- If a bullet starts with "Remove" → `### Removed`
- If a bullet starts with "Deprecate" → `### Deprecated`
- If in doubt, read the `tasks.md` for that change — task descriptions provide more context.

**Graceful Degradation**

- If `proposal.md` does not exist for a change, read `tasks.md` and derive items from task descriptions.
- If neither `proposal.md` nor `tasks.md` exist, skip the change with a warning.
- If a change has no explicit version in `proposal.md`, scan `tasks.md` for "Release" mentions. If none found, treat it as targeting the next version (Unreleased).

**Output**

```markdown
## Changelog Update Complete

**Version:** <version>
**Release date:** YYYY-MM-DD
**Changes archived:** N changes
**Unreleased changes:** M changes
**Files updated:**
- CHANGELOG.md ✓
- CHANGELOG-ARCHIVE.md ✓
- debian/changelog ✓
- rpmbuild/SPECS/msxbas2rom.spec ✓
- src/cli/appinfo.h (info_history) ✓
```
