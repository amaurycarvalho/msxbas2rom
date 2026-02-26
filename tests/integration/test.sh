#!/usr/bin/env bash
# ------------------------------
# MSXBAS2ROM integration testing
# ------------------------------

# Stop script on error
set -e

# Variables 
APP_PATH="../../bin/Release/msxbas2rom"
BAS_FILES=$(find . -type f -name "*.bas")

# Clean old rom files
echo "🧹 Cleaning old ROM files..."
find . -type f -name "*.rom" -delete

# Find all .bas files recursively and compiling it
echo "📦 Building ROM files..."
for file in $BAS_FILES; do
    echo "Compiling ${file}"
    ${APP_PATH} -q ${file} || ${APP_PATH} -q -x ${file}
done

# Clean new rom files
echo "🧹 Cleaning new ROM files..."
find . -type f -name "*.rom" -delete

echo "✅ Integration test script finished"
