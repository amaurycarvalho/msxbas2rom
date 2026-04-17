---
name: get-version
description: Displays the current application version, release date, and description
license: GPL-3.0
metadata:
  author: amaurycarvalho
  version: "1.0"
compatibility: Requires bash
---

## Objective

This skill reads and displays the current application version along with its release date and description.

## Instructions

1. Read current version

Read the current version from:
src/cli/appinfo.h

Expected format:
#define app_version "MAJOR.MINOR.PATCH.BUILD"

2. Read version history

Locate the `info_history` block and extract the **first entry** (most recent version).

Expected format:

MAJOR.MINOR.PATCH.BUILD – (YYYY/MM/DD) description

3. Output

Display:

- Current version
- Release date
- Description

4. Validation

If any required information cannot be found, display a warning and exit.

## Execution

Execute:

```bash
bash scripts/get_version.sh
```
