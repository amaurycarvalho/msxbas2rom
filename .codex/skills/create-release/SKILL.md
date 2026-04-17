---
name: create-release
description: Automatically creates a release tag and branch using the version defined in src/cli/appinfo.h
license: GPL-3.0
metadata:
  author: amaurycarvalho
  version: "1.0"
compatibility: Requires git and bash
---

## Objective

Automate the release process:

- Extract the version from `src/cli/appinfo.h`;
- Create an annotated Git tag;
- Create a release branch;
- Push everything.

## Instructions

1. Read the file `src/cli/appinfo.h`;
2. Extract the value of `app_version`;
3. Run the script `scripts/release.sh`.

## Execution

Execute:

```bash
bash scripts/release.sh
```

## Notes

- Abort if the version is not found;
- Abort if the tag already exists;
- Always return to the master branch.
