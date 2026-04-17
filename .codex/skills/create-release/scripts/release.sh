#!/usr/bin/env bash

set -e

FILE="src/cli/appinfo.h"

if [ ! -f "$FILE" ]; then
  echo "File not found: $FILE"
  exit 1
fi

# Version extraction
VERSION=$(grep '#define app_version' "$FILE" | sed -E 's/.*"([^"]+)".*/\1/')

if [ -z "$VERSION" ]; then
  echo "Cannot extract the version from $FILE"
  exit 1
fi

TAG="v$VERSION"
MESSAGE="Release $VERSION"
BRANCH="release/$VERSION"

echo "📦 Version: $VERSION"
echo "🏷️  Creating tag: $TAG"
echo "🌿 Creating branch: $BRANCH"

# Git execution
git tag -a "$TAG" -m "$MESSAGE"
git push origin --tags

git checkout -b "$BRANCH"
git push origin "$BRANCH"

git switch master

echo "✅ Version created successfully."
