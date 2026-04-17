#!/bin/bash

FILE="src/cli/appinfo.h"

if [ ! -f "$FILE" ]; then
  echo "Error: $FILE not found."
  exit 1
fi

# Extract current version
if grep -q '^#define[[:space:]]\+app_version' "$FILE"; then
  VERSION=$(grep '^#define[[:space:]]\+app_version' "$FILE" | sed 's/.*"\(.*\)".*/\1/')
else
  echo "Warning: app_version not found."
  exit 1
fi

# Extract latest history entry (first after "Version history")
ENTRY=$(awk '
/Version history/ {
  found=1
  next
}
found && NF {
  print
  exit
}
' "$FILE")

if [ -z "$ENTRY" ]; then
  echo "Warning: Could not extract version history entry."
  exit 1
fi

# Parse entry
# Format: VERSION – (DATE) description

PARSED_VERSION=$(echo "$ENTRY" | sed 's/ –.*//')
DATE=$(echo "$ENTRY" | sed 's/.*(\(.*\)).*/\1/')
DESC=$(echo "$ENTRY" | sed 's/.*) //')

# Output
echo "Current Version : $VERSION"
echo "                  MAJOR.MINOR.PATCH.BUILD"
echo "Release Date    : $DATE"
echo "Description     : $DESC"
