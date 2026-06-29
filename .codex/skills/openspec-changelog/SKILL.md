---
name: openspec-changelog
description: Update CHANGELOG.md, CHANGELOG-ARCHIVE.md, debian/changelog, RPM spec, and info_history from a specific change. Asks for change name if not provided.
license: MIT
compatibility: Requires openspec CLI.
metadata:
  author: openspec
  version: "1.1"
  generatedBy: "1.4.1"
---

Update all changelog-related files from a specific change.

**Input**: Optional change name (directory name under `openspec/changes/` or `openspec/changes/archive/`). If not provided, the skill will ask the user.

**Steps**

### 1. Resolve change name and release version

1.1. **If a change name was provided as input**, use it directly. Otherwise:
   - Find the most recently archived change by running `ls -t openspec/changes/archive/ | head -1`.
   - List all unarchived changes: `ls openspec/changes/` (filtering out `archive/`).
   - Ask the user to pick a change name, presenting the last archived change and all unarchived changes as suggestions.

1.2. **Read the change's proposal.md** at `openspec/changes/<change-name>/proposal.md` (or `openspec/changes/archive/<change-name>/proposal.md`). Search for the target release in the Impact section by looking for a line matching `- **Target**: Release <version>` or `- **Release**: <version>`. Extract the version string (e.g., `1.5.0.0`).

1.3. **If no release is specified** in the proposal, default to the **current release** from `src/cli/appinfo.h` (extract the value of `app_version`, e.g., `"1.1.0.0"`).

1.4. **Ask the user to confirm the release version** and allow them to edit it. Present the detected release as the default suggestion.

1.5. **Compare the confirmed release** with the release recorded in the change's proposal.md. If they differ, **update the proposal.md** by replacing the old `- **Target**: Release <old>` line with `- **Target**: Release <confirmed>`. If no Target line exists, add `- **Target**: Release <confirmed>` to the Impact section.

### 2. Determine change classification and CHANGELOG strategy

2.1. **Read `src/cli/appinfo.h`** to get the current release version from `app_version`. This is the **latest published release**.

2.2. **Check if the confirmed release matches the current release** in `appinfo.h`:
   - **Same version**: The change targets the current release.
   - **Newer version**: The change targets a future release.

2.3. **Read `CHANGELOG.md`** and check if an entry for the confirmed release already exists. Look for a heading matching `## [<version>] -`.

### 3. Update CHANGELOG.md — release entry exists

If an entry for the confirmed release **already exists** in `CHANGELOG.md`:

3.1. **Build the change's content** as a sub-section with Keep a Changelog categories. Use the following structure:

    ```markdown
    ### [<change-name>](<change-name-relative-path>) <ultra-condensed summary>

    #### Added
    - item 1

    #### Changed
    - item 1

    #### Fixed
    - item 1
    ```

    Where:
    - `<change-name>` is the change directory name.
    - `<change-name-relative-path>` is the relative path to the change in the project (`openspec/changes/<change-name>` or `openspec/changes/archive/<change-name>`).
    - `<ultra-condensed summary>` is a 1-sentence summary extracted from the proposal's "Why" or "What Changes" section title.
    - Items come from the "What Changes" bullet list in `proposal.md`, categorized as:
      - `#### Added` — items starting with "Implement", "Add", "Create", "Write", or describing new features
      - `#### Changed` — items starting with "Change", "Migrate", "Update", "Rename", "Expand", "Reuse", "Convert", or describing behavior changes
      - `#### Fixed` — items starting with "Fix", "Correct", or describing bug fixes
      - `#### Removed` — items starting with "Remove", "Delete"
      - `#### Deprecated` — items starting with "Deprecate"
      - `#### Security` — items starting with "Security" or describing security improvements

3.2. **Insert or update the change's sub-section** within the existing release entry in `CHANGELOG.md`. If a sub-section for this change name already exists, replace it entirely. If not, append it after the release heading and any existing change sub-sections.

3.3. **Review the release entry's other information** (other change sub-sections that already existed). Verify that every change listed there still has an archived or active proposal at its path. If a change sub-section references a change that no longer exists, remove it. Update any outdated summary text to match the current proposal.

### 4. Update CHANGELOG.md — release entry does NOT exist

If an entry for the confirmed release **does NOT exist** in `CHANGELOG.md`:

4.1. **Find the current release entry** in `CHANGELOG.md` (the entry matching `app_version` from `appinfo.h`). Move its entire content (from `## [<current-version>] -` to the next `## [` heading or end of file) into `CHANGELOG-ARCHIVE.md`:
   - Read `CHANGELOG-ARCHIVE.md`.
   - Insert the moved content right after the header (after the intro paragraph and before the first `## [` entry), preserving archive ordering.
   - Write the updated `CHANGELOG-ARCHIVE.md`.

4.2. **Build the new release entry** for the confirmed release into `CHANGELOG.md` using the change sub-section structure:

    ```markdown
    ## [<version>] - YYYY-MM-DD

    ### [<change-name>](<change-name-relative-path>) <ultra-condensed summary>

    #### Added
    - item 1

    #### Changed
    - item 1
    ```

    Use today's date as `YYYY-MM-DD`.

4.3. **Rebuild `CHANGELOG.md`**:
   - Keep the header (everything from the top until the first `## [` heading).
   - Add the new Unreleased section (see step 5).
   - Add the new release entry (from 4.2).
   - Add/update version compare links at the bottom:
     - `[Unreleased]: ...compare/v<new-version>...HEAD`
     - `[<version>]: ...releases/tag/v<version>`
   - Add the link to the archive: `See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.`

    The structure should be:

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

### 5. Update the Unreleased section

5.1. **Scan all unarchived changes** in `openspec/changes/` (excluding `archive/`):
   - If the change being processed in this skill run is **not archived**, skip it (do not include it in Unreleased, since it was already placed in the release section).
   - Include all other unarchived changes.

5.2. **For each unarchived change**, read its `proposal.md` and extract an ultra-condensed 1-sentence summary. Build an Unreleased entry:

    ```markdown
    ## [Unreleased]

    ### Added

    - [<change-name>](<change-name-relative-path>) <ultra-condensed summary>
    ```

5.3. **Replace the existing Unreleased section** in `CHANGELOG.md` with the newly built content from 5.2. If there are no unarchived changes, set the Unreleased section to:

    ```markdown
    ## [Unreleased]
    ```

    (with no items).

### 6. Build the ultra-simplified summary

Create a 1-2 line summary of the change that captures its essence. For example:

```
Release 1.5.0.0: 16-bit segment support for ASCII16 up to 4MB and ASCII16X up to 8MB
```

Use the most prominent feature of the change as the summary.

### 7. Update debian/changelog

Write the debian changelog entry. Use the current date and author info from the git log or from existing entries. The format:

```
msxbas2rom (<version>-0) bionic; urgency=low

  * <ultra-simplified summary>

 -- Amaury Carvalho <amauryspires@gmail.com>  <date>
```

Where `<date>` follows Debian format: `Day, DD Mon YYYY HH:MM:SS ±TZ` (e.g., `Thu, 19 Jun 2026 18:00:00 -0300`).

**Replace the entire debian/changelog content** — remove any previous release entries.

### 8. Update rpmbuild/SPECS/msxbas2rom.spec

Update the `%changelog` section. The format:

```
%changelog
* <Day Mon DD YYYY> Amaury Carvalho <amauryspires@gmail.com>
- <ultra-simplified summary>
```

**Replace the entire %changelog section** — remove any previous entries.

Also update the `Version:` field at the top of the spec file to match the confirmed release version.

### 9. Update info_history in appinfo.h

Replace the `info_history` string content so it contains:

```
Version history

## [<version>] - YYYY-MM-DD

<ultra-simplified summary>

See full changelog:
https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v<version>
```

**Replace the entire `info_history` string** — remove any previous release entries.

### 10. Verify consistency

Read back all modified files and verify:
- `CHANGELOG.md` has the release entry for the change's version
- `CHANGELOG.md` has the Unreleased section (if there are active unarchived changes)
- `CHANGELOG-ARCHIVE.md` has the previous releases appended
- `debian/changelog` has only the latest release entry
- `rpmbuild/SPECS/msxbas2rom.spec` has only the latest release entry in %changelog and the correct Version:
- `src/cli/appinfo.h` info_history has only the change's release summary

**Heuristics for categorization**

- Read the "What Changes" bullets from `proposal.md`. The bullet text often starts with verbs like "Implement", "Add", "Fix", "Remove", "Change", "Deprecate".
- If a bullet starts with "Implement" or "Add" or "Create" or "Write" → `#### Added`
- If a bullet starts with "Change" or "Migrate" or "Update" or "Rename" or "Expand" or "Reuse" or "Convert" → `#### Changed`
- If a bullet starts with "Fix" or "Correct" → `#### Fixed`
- If a bullet starts with "Remove" or "Delete" → `#### Removed`
- If a bullet starts with "Deprecate" → `#### Deprecated`
- If in doubt, read the `tasks.md` for that change — task descriptions provide more context.

**Graceful Degradation**

- If `proposal.md` does not exist for a change, read `tasks.md` and derive items from task descriptions.
- If neither `proposal.md` nor `tasks.md` exist, skip the change with a warning.
- If a change has no explicit version in `proposal.md`, scan `tasks.md` for "Release" mentions. If none found, treat it as targeting the next version (Unreleased).

**Output**

```markdown
## Changelog Update Complete

**Change:** <change-name>
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
**Commentary:** release-push skill can now be used manually to publish it on GitHub.
```
