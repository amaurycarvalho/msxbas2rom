---
name: update-version
description: Updates the application version following Semantic Versioning license: GPL-3.0
metadata:
  author: amaurycarvalho
  version: "1.0"
compatibility: Requires bash
---

## Objective
This skill updates the application version following Semantic Versioning.

## Instructions

1. Read current version

Read the current version from:
src/cli/appinfo.h

Expected format:
MAJOR.MINOR.PATCH.BUILD

Semantic Versioning rules:
- MAJOR: incompatible changes
- MINOR: backward-compatible features
- PATCH: backward-compatible fixes
- BUILD: build number

Reference: https://semver.org/

2. Ask for new version

Prompt:
"Enter the new version (MAJOR.MINOR.PATCH.BUILD). Leave blank to cancel:"

If blank → STOP

3. Ask for description

Prompt:
"Enter the version description:"

If blank → STOP

4. Ask for release date

Prompt:
"Enter release date (YYYY/MM/DD) or leave blank for today:"

If blank → use current date

5. Format description

Format:
MAJOR.MINOR.PATCH.BUILD – (YYYY/MM/DD) description

6. Apply changes

File 1: src/cli/appinfo.h
- Update #define app_version
- Insert new entry at top of info_history

File 2: debian/changelog
Format:
msxbas2rom (VERSION-0) bionic; urgency=low

File 3: rpmbuild/SPECS/msxbas2rom.spec
Update:
Version: VERSION

File 4: CHANGELOG.md
Insert at top:
- VERSION – (DATE) description;

7. Summary
Output summary of updated files and new version.

## Execution

Execute:

```bash
bash scripts/update_version.sh
```
