#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
APPINFO="$PROJECT_ROOT/src/cli/appinfo.h"

if [ ! -f "$APPINFO" ]; then
    echo "ERROR: Could not find $APPINFO" >&2
    exit 1
fi

VERSION=$(sed -n 's/^#define app_version "\(.*\)"/\1/p' "$APPINFO")

if [ -z "$VERSION" ]; then
    echo "ERROR: Could not extract version from $APPINFO" >&2
    exit 1
fi

echo "=== Releasing v$VERSION ==="

git tag -a "v$VERSION" -m "Release $VERSION"
echo "  ✓ Tag v$VERSION created"

git push origin --tags
echo "  ✓ Tags pushed"

git checkout -b "release/v$VERSION"
echo "  ✓ Branch release/v$VERSION created"

git push origin "release/v$VERSION"
echo "  ✓ Branch release/v$VERSION pushed"

git switch master
echo "  ✓ Switched back to master"

echo ""
echo "=== Release v$VERSION pushed to GitHub successfully ==="
