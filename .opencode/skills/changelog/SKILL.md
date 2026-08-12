---
name: changelog
description: Update CHANGELOG.md and CHANGELOG-ARCHIVE.md, reconciling registered vs unregistered changes from both OpenSpec and SpecKit directory structures.
license: MIT
compatibility: Requires OpenSpec or SpecKit directory structure.
metadata:
  author: amaurycarvalho
  version: "2.0"
  generatedBy: "1.4.1"
---

Reconcile all changes between the project's changes/specs directories and the CHANGELOG files.

**Input**: None. The skill auto-detects the project structure and scans for all changes.

**Steps**

### 0. Detect project scenario

Check which directory structure exists at the project root:

- **OpenSpec**: If `openspec/changes/` exists.
  - Changes live in `openspec/changes/` (active) and `openspec/changes/archive/` (archived).
  - Each change is a subdirectory containing `proposal.md`, `design.md`, `tasks.md`, and optionally `specs/<subchange>/spec.md`.
  - Summary source: `proposal.md` (first sentence of "What Changes" or "Why" section).
  - Items source: `proposal.md` ("What Changes" bullet list).
  - Fallback for both: `tasks.md`.

- **SpecKit**: If `specs/` exists and OpenSpec does not.
  - Changes live in `specs/` (no separate archive directory; all specs are flat).
  - Each spec is a subdirectory containing `spec.md`, `quickstart.md`, `research.md`, `plan.md`, `tasks.md`, `data-model.md`, `checklists/`, `contracts/`.
  - Summary source: `plan.md` (text under `## Summary`) or `spec.md` (first heading / first paragraph).
  - Items source: `tasks.md` (task descriptions).
  - Fallback: `spec.md`.

If neither exists, abort with an error telling the user no recognized structure was found.

Set these context variables for use in later steps:
- `changes_dirs` — list of `(dir, label)` where label is `"active"` or `"archived"`.
- `summary_file` — primary file name for extracting the summary.
- `summary_fallback` — fallback file name for summary.
- `items_file` — primary file name for extracting item bullets.
- `items_fallback` — fallback file name for items.
- `path_prefix` — URL prefix for change links (e.g. `openspec/changes` or `specs`).

### 1. Collect all changes

1.1. **List subdirectories** in each `changes_dirs` entry. Each subdirectory name is a change name. Record its full relative path (e.g. `openspec/changes/my-feature` or `specs/002-kernel-pipeline-engine`) and whether it is active or archived.

1.2. Build a flat list of all change names with their metadata (name, path, is_archived).

### 2. Read existing CHANGELOG references

2.1. **Read `CHANGELOG.md`** and **`CHANGELOG-ARCHIVE.md`** in full.

2.2. Extract every change reference — any markdown heading `### [<change-name>](...)` or list item `- [<change-name>](...)` found under any release section (including Unreleased). Build a set of registered change names.

2.3. Also extract all release versions present. The current release is the first `## [<version>] -` heading that is not `[Unreleased]`. The latest release is the one with the highest semver (or the first non-Unreleased heading if order is guaranteed).

### 3. Identify unregistered changes

Compare the collected change names (step 1) with the registered set (step 2). Build a list of changes whose name does NOT appear in either CHANGELOG file.

If there are no unregistered changes, notify the user and exit.

### 4. Resolve release version

4.1. **Identify the current release version** from CHANGELOG.md (the first non-Unreleased `## [<version>] -` heading).

4.2. **Ask the user**:
   - **Keep current version**: Unregistered changes will be added under this same release.
   - **Update to new version**: Propose a semver bump (patch bump by default, e.g. `0.1.0` → `0.2.0`). Allow manual free-text input for the new version.

4.3. **If updating**:
   - The current release entry (from `## [<current-version>] -` to the next `## [` heading or EOF) will be moved to `CHANGELOG-ARCHIVE.md` before the new release is written.

### 5. Distribute unregistered changes

Present the list of unregistered changes to the user and ask three questions:

5.1. **Which changes go into the selected release** (step 4) in `CHANGELOG.md`? (multi-select)

5.2. **Which changes go into `CHANGELOG-ARCHIVE.md`**? (multi-select, default: none)
   - For each change selected here, ask: **Under which release version** should this change appear in the archive? (free text, default: the current version from step 4.1, or the archive version if the current release was already archived)

5.3. **Which changes go as Unreleased** in `CHANGELOG.md`? (default: all remaining changes not selected in 5.1 or 5.2)

### 6. Build content for each change

For each change assigned to any destination (release, archive, or unreleased):

6.1. **Extract summary**:
   - Try the `summary_file` first. Look for:
     - OpenSpec: `proposal.md` — find `## What Changes` or `## Why` section heading, take the first non-empty line after it.
     - SpecKit: `plan.md` — find `## Summary`, take text until the next heading.
   - If not found, try `summary_fallback`:
     - SpecKit: `spec.md` — use the first `# ` heading or first paragraph.
   - If nothing yields a summary, use the change directory name as the summary.

6.2. **Extract items**:
   - Try `items_file` first. For OpenSpec, read `proposal.md` and extract all bullet points under `## What Changes`. For SpecKit, read `tasks.md` and extract all task descriptions (lines starting with `- [ ]` or `- [X]`).
   - Fallback to `items_fallback` if primary file is missing.
   - Categorize each item using the same heuristics as the existing skill (see **Heuristics** below).
   - If no items found, the change entry will have no sub-items (just the summary heading).

6.3. **Build the change entry**:
   ```markdown
   ### [<change-name>](<relative-path>) <summary>

   #### Added
   - item

   #### Changed
   - item
   ```
   Only include categories that have at least one item.

### 7. Update CHANGELOG.md

7.1. **If version was updated** (step 4.2):
   - Locate the current release entry in CHANGELOG.md (from `## [<current-version>] -` to the next `## [` heading or EOF).
   - Read `CHANGELOG-ARCHIVE.md`.
   - Insert the current release entry into `CHANGELOG-ARCHIVE.md` right after its header (after the introductory paragraph and before the first existing `## [` entry).
   - Remove the entry from CHANGELOG.md.
   - Write the updated `CHANGELOG-ARCHIVE.md`.

7.2. **Build the release entry** for the resolved version (from step 4):
   ```markdown
   ## [<version>] - YYYY-MM-DD
   ```
   Append all change entries assigned to this release (step 5.1), ordered alphabetically by change name. Use today's date.

7.3. **Build the Unreleased section**:
   ```markdown
   ## [Unreleased]
   ```
   Append all change entries assigned to Unreleased (step 5.3), ordered alphabetically by change name.
   If no changes were assigned to Unreleased, leave the section empty (no items).

7.4. **Rebuild the full CHANGELOG.md**:
   - Keep the header (everything from the top of the file until the first `## [` heading).
   - Add the Unreleased section (from 7.3).
   - Add the release entry (from 7.2).
   - Add/update version compare links at the bottom:
     - `[Unreleased]: ...compare/v<version>...HEAD`
     - `[<version>]: ...releases/tag/v<version>`
   - Add the archive link: `See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.`
   - Derive the GitHub repo URL from existing links in CHANGELOG.md (preserve the existing format), or from `pyproject.toml` metadata if available, or ask the user.

   The final structure:
   ```markdown
   # Changelog

   ...

   ## [Unreleased]

   ### [<change>](path) summary

   ## [<version>] - YYYY-MM-DD

   ### [<change>](path) summary

   #### Added
   - item

   [Unreleased]: https://github.com/<owner>/<repo>/compare/v<version>...HEAD
   [<version>]: https://github.com/<owner>/<repo>/releases/tag/v<version>

   See [CHANGELOG Archive](CHANGELOG-ARCHIVE.md) for older releases.
   ```

### 8. Update CHANGELOG-ARCHIVE.md

8.1. **Group changes assigned to archive** (step 5.2) by their specified archive version:
   ```markdown
   ## [<archive-version>] - YYYY-MM-DD

   ### [<change-name>](<relative-path>) <summary>
   ```

8.2. **Rebuild the full CHANGELOG-ARCHIVE.md**:
   - Keep the header (everything from the top until the first `## [` heading).
   - Append the new archive entries (from 8.1), ordered by version (descending).
   - If the current release was moved here in step 7.1, that entry is already present.
   - Add/update version compare links at the bottom:
     - `[<version>]: ...releases/tag/v<version>`
   - Add the link to the main changelog: `See main [CHANGELOG](CHANGELOG.md) for newer releases.`

### 9. Verify consistency

Read back all modified files and verify:
- `CHANGELOG.md` has the correct release entry with all assigned changes.
- `CHANGELOG.md` has the Unreleased section (if any changes were assigned there).
- `CHANGELOG-ARCHIVE.md` has the archived entries properly formatted.
- No duplicate change references exist.
- Version compare links are correct and point to the right repository.

### 10. Suggest release-version skill

If the version was updated in step 4.2, suggest the user run the `release-version` skill manually to update the version number across all project files (pyproject.toml, __init__.py, etc.).

**Heuristics for categorization**

- Read the extracted item text. The first word often indicates the category:
  - "Implement", "Add", "Create", "Write", "Build", "Develop" → `#### Added`
  - "Change", "Migrate", "Update", "Rename", "Expand", "Reuse", "Convert", "Refactor" → `#### Changed`
  - "Fix", "Correct", "Patch", "Resolve" → `#### Fixed`
  - "Remove", "Delete", "Drop" → `#### Removed`
  - "Deprecate" → `#### Deprecated`
  - "Security" → `#### Security`
- If in doubt, assign to `#### Changed`.

**Graceful Degradation**

- If a change's primary content files do not exist, try the fallback.
- If neither exists, skip the change with a warning and list it as "skipped (no content files found)".
- If a change cannot be categorized, default all items to `#### Changed`.

**Output**

```markdown
## Changelog Update Complete

**Scenario:** <OpenSpec | SpecKit>
**Release version:** <version>
**Release date:** YYYY-MM-DD
**Changes registered in release:** N
**Changes archived:** N
**Changes marked as Unreleased:** N
**Changes skipped:** N

**Files updated:**
- CHANGELOG.md ✓
- CHANGELOG-ARCHIVE.md ✓

**Next steps:** If the version was updated, run the `release-version` skill to update version numbers across the project.
```
