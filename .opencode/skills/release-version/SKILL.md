---
name: release-version
description: Update the application release version across all version files and regenerate the changelog. Use when the user wants to set a new release version.
license: MIT
metadata:
  author: amaurycarvalho
  version: "1.0"
---

Update the application release version across the codebase and regenerate all changelog-related files.

**Input**: Version string (e.g., `1.0.1.0`). Must be provided by the user in the command or message. If omitted, show an error message and STOP.

**Steps**

1. **Extract the version**

   Parse the version string from the user's message or command. The version should match the pattern `X.Y.Z.W` (four numeric segments separated by dots). Examples: `1.0.1.0`, `2.0.0.0`.

   **If no version is provided** (the user did not include a version string in their message):
   - Display the following error message and STOP:
     ```
     ERROR: No version provided.
     Usage: /release-version <version>
     Example: /release-version 1.0.1.0
     ```
   - Do NOT proceed with any of the steps below.

2. **Update app_version in appinfo.h**

   Read `src/cli/appinfo.h` and find the line containing `#define app_version`.

   Replace the current value with the new version string:

   ```
   #define app_version "<new-version>"
   ```

3. **Update debian/changelog**

   Read `debian/changelog` and find the first line. It follows the format:

   ```
   msxbas2rom (<old-version>-0) bionic; urgency=low
   ```

   Replace `<old-version>` with `<new-version>`. Keep everything else unchanged.

4. **Update rpm spec**

   Read `rpmbuild/SPECS/msxbas2rom.spec` and find the `Version:` field (typically line 2).

   Replace the current version:

   ```
   Version: <new-version>
   ```

   Also update the date in the `%changelog` section to today's date (in the format `Day Mon DD YYYY`).

5. **Run openspec-changelog skill**

   Use the **Skill tool** to invoke the `openspec-changelog` skill. This will:
   - Regenerate CHANGELOG.md with the proper release entry
   - Update CHANGELOG-ARCHIVE.md
   - Rewrite debian/changelog with the correct entry
   - Rewrite the %changelog section in the RPM spec
   - Update info_history in appinfo.h

   The changelog skill reads `app_version` from `appinfo.h` (which was just updated in step 2) so it will use the new version.

6. **Verify consistency**

   Read back the modified files and confirm:
   - `src/cli/appinfo.h` has `#define app_version "<new-version>"`
   - `debian/changelog` has the correct version
   - `rpmbuild/SPECS/msxbas2rom.spec` has the correct `Version:` field

**Output On Success**

```
Release version updated to <version>

Files updated:
- src/cli/appinfo.h (app_version)
- debian/changelog
- rpmbuild/SPECS/msxbas2rom.spec
- CHANGELOG.md (via openspec-changelog)
- CHANGELOG-ARCHIVE.md (via openspec-changelog)
- src/cli/appinfo.h (info_history, via openspec-changelog)
```

**Output On Error**

```
ERROR: No version provided.
Usage: /release-version <version>
Example: /release-version 1.0.1.0
```

**Guardrails**
- Always validate that a version string was provided before making any changes
- Do NOT guess or auto-generate a version — the user must supply it explicitly
- The version format should follow `X.Y.Z.W` (e.g., `1.0.1.0`)
- After updating files in steps 2-4, always run openspec-changelog to ensure consistency
