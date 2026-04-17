#!/bin/bash

CURRENT=$(grep '#define app_version' src/cli/appinfo.h | sed 's/.*"\(.*\)".*/\1/')

echo "Current version: $CURRENT"
echo "Expected format: MAJOR.MINOR.PATCH.BUILD"
echo "Example: 1.2.3.4"
echo ""

read -p "Enter new version (MAJOR.MINOR.PATCH.BUILD): " VERSION
if [ -z "$VERSION" ]; then
  echo "Cancelled."
  exit 0
fi

if ! [[ "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
  echo "Invalid version format."
  exit 1
fi

read -p "Enter description: " DESC
if [ -z "$DESC" ]; then
  echo "Cancelled."
  exit 0
fi

read -p "Enter release date (YYYY/MM/DD): " DATE
if [ -z "$DATE" ]; then
  DATE=$(date +%Y/%m/%d)
fi

FULL="$VERSION – ($DATE) $DESC"

echo "Updating files..."

# appinfo.h
if grep -q '^#define[[:space:]]\+app_version[[:space:]]\+' src/cli/appinfo.h; then
  sed -i '0,/^#define[[:space:]]\+app_version[[:space:]]\+".*"/s//\#define app_version "'"$VERSION"'"/' src/cli/appinfo.h
else
  echo "Warning: app_version define not found in appinfo.h. Skipping update."
fi

if grep -q "Version history" src/cli/appinfo.h; then
  if grep -q "$VERSION – (" src/cli/appinfo.h; then
    echo "Warning: Version already exists in appinfo.h. Skipping."
  else
    tmp=$(mktemp)

    awk -v entry="$FULL;" '
    BEGIN { inserted=0 }
    {
      print

      if ($0 ~ /Version history/ && inserted==0) {
        print ""
        print entry
        inserted=1
      }
    }
    ' src/cli/appinfo.h > "$tmp"

    mv "$tmp" src/cli/appinfo.h
  fi
else
  echo "Warning: Version history section not found in appinfo.h. Skipping update."
fi

# debian changelog
if grep -q "^msxbas2rom (" debian/changelog; then
  sed -i '1{
  s/^msxbas2rom (.*) bionic; urgency=low/msxbas2rom ('"$VERSION"'-0) bionic; urgency=low/
  }' debian/changelog
else
  echo "Warning: debian/changelog format not recognized. Skipping update."
fi

# rpm spec file
if grep -q "^Version:" rpmbuild/SPECS/msxbas2rom.spec; then
  sed -i '0,/^Version:[[:space:]]*.*/s//Version: '"$VERSION"'/' rpmbuild/SPECS/msxbas2rom.spec
else
  echo "Warning: Version field not found in spec file. Skipping update."
fi

# CHANGELOG.md
if grep -q "^# Version history" CHANGELOG.md; then
  if grep -q "^- $VERSION" CHANGELOG.md; then
    echo "Warning: Version already exists in CHANGELOG.md. Skipping."
  else
    tmp=$(mktemp)

    {
      echo "# Version history"
      echo ""
      echo "- $FULL;"
      echo ""
      tail -n +2 CHANGELOG.md
    } > "$tmp"

    mv "$tmp" CHANGELOG.md
  fi
else
  echo "Warning: CHANGELOG.md format not recognized. Skipping update."
fi

# Summary
echo "Done!"
echo ""
echo "Summary:"
echo "- Version updated to: $VERSION"
echo "- Description: $DESC"
echo "- Date: $DATE"
echo "- Files updated:"
echo "  * src/cli/appinfo.h"
echo "  * debian/changelog"
echo "  * rpmbuild/SPECS/msxbas2rom.spec"
echo "  * CHANGELOG.md"
