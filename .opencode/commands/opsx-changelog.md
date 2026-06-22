---
description: Update CHANGELOG.md, CHANGELOG-ARCHIVE.md, debian/changelog, RPM spec, and info_history from archived and active changes
---

Update all changelog-related files from archived and active changes.

**Steps**

1. Use the Skill tool to invoke `openspec-changelog`.

2. The skill will:
   - Read the current version from `src/cli/appinfo.h`
   - Scan all archived and active changes in `openspec/changes/`
   - Build the release entry for CHANGELOG.md
   - Build the Unreleased section for any active unarchived changes
   - Move previous releases from CHANGELOG.md to CHANGELOG-ARCHIVE.md
   - Update debian/changelog, rpm spec, and info_history with ultra-simplified summary
   - Verify consistency across all files
