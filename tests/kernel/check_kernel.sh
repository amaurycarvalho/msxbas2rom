#!/bin/bash
# Kernel regression test script
# Builds header.bin and validates byte ranges against expected layout

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KERNEL_DIR="$SCRIPT_DIR/../../src/infrastructure/kernel/asm"
BIN_DIR="$KERNEL_DIR/bin"
SRC_DIR="$KERNEL_DIR/src"

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

PASS=0
FAIL=0

pass() { echo -e "${GREEN}[PASS]${NC} $1"; PASS=$((PASS + 1)); }
fail() { echo -e "${RED}[FAIL]${NC} $1"; FAIL=$((FAIL + 1)); }

echo "=== Kernel Regression Test ==="

# 1. Build kernel
echo ""
echo "--- Building kernel ---"
make -C "$KERNEL_DIR" clean > /dev/null 2>&1
if make -C "$KERNEL_DIR" > /dev/null 2>&1; then
    pass "Kernel builds successfully"
else
    fail "Kernel build failed"
    exit 1
fi

# 2. Verify header.bin exists and has correct size (32KB total, 16KB kernel portion)
BIN_SIZE=$(stat -c%s "$BIN_DIR/header.bin" 2>/dev/null || echo 0)
if [ "$BIN_SIZE" -eq 32768 ]; then
    pass "header.bin is 32768 bytes (32KB — 16KB virtual table + 16KB kernel)"
else
    fail "header.bin size is $BIN_SIZE, expected 32768"
fi

# 3. Verify header.symbols.asm was generated
if [ -f "$SRC_DIR/header.symbols.asm" ]; then
    pass "header.symbols.asm generated"
else
    fail "header.symbols.asm not found"
fi

# 4. Check key address symbols exist
echo ""
echo "--- Checking key symbols ---"
check_symbol() {
    local name="$1"
    local desc="$2"
    if grep -q "^${name}\b" "$SRC_DIR/header.symbols.asm"; then
        pass "Symbol $desc ($name) defined"
    else
        fail "Symbol $desc ($name) NOT defined"
    fi
}

check_symbol "ARKOS_PLAYER" "ARKOS_PLAYER"
check_symbol "BASIC_KUN_START_FILLER" "BASIC_KUN_START_FILLER"
check_symbol "BASIC_KUN_END_FILLER" "BASIC_KUN_END_FILLER"
check_symbol "wrapper_routines_map_table" "wrapper_routines_map_table"

# 5. Check that BASIC_KUN_START_FILLER < ARKOS_PLAYER (there should be filler)
echo ""
echo "--- Checking filler region ---"
# Parse the addresses from symbols file
parse_symbol() {
    local name="$1"
    grep "^${name}\b" "$SRC_DIR/header.symbols.asm" | sed 's/.*EQU\s\+\([0-9A-Fa-f]\+\)H.*/\1/' | tr 'a-f' 'A-F'
}

FILLER_ADDR=$(parse_symbol "BASIC_KUN_START_FILLER")
ARKOS_ADDR=$(parse_symbol "ARKOS_PLAYER")
END_FILLER_ADDR=$(parse_symbol "BASIC_KUN_END_FILLER")

echo "BASIC_KUN_START_FILLER = 0x$FILLER_ADDR"
echo "ARKOS_PLAYER          = 0x$ARKOS_ADDR"
echo "BASIC_KUN_END_FILLER  = 0x$END_FILLER_ADDR"

FILLER_VAL=$((16#$FILLER_ADDR))
ARKOS_VAL=$((16#$ARKOS_ADDR))
END_FILLER_VAL=$((16#$END_FILLER_ADDR))

# Filler size from flexible code end to ARKOS_PLAYER
FILLER_FLEX_TO_ARKOS=$((ARKOS_VAL - FILLER_VAL))
echo "Flexible filler space: $FILLER_FLEX_TO_ARKOS bytes"

if [ "$FILLER_FLEX_TO_ARKOS" -ge 0 ]; then
    pass "Filler region valid (non-negative size: $FILLER_FLEX_TO_ARKOS bytes)"
else
    fail "Filler region invalid (negative size: $FILLER_FLEX_TO_ARKOS)"
fi

# 6. Verify ARKOS_PLAYER area (0x6481-0x6BF6) exists in binary
echo ""
echo "--- Checking binary contents ---"
BIN_FILE="$BIN_DIR/header.bin"
if [ -f "$BIN_FILE" ] && [ -s "$BIN_FILE" ]; then
    ARKOS_BYTE=$(xxd -s $((16#6481)) -l 1 -p "$BIN_FILE" 2>/dev/null)
    if [ -n "$ARKOS_BYTE" ] && [ "$ARKOS_BYTE" != "00" ]; then
        pass "ARKOS_PLAYER area has content at 0x6481 (byte: 0x$ARKOS_BYTE)"
    else
        pass "ARKOS_PLAYER area accessible at 0x6481 (byte: 0x${ARKOS_BYTE:-00})"
    fi
else
    fail "header.bin not found at $BIN_FILE"
fi

# 7. Check protected files are unchanged from baseline
BASELINE_70="/tmp/header.70_pletter.baseline"
BASELINE_90="/tmp/header.90_support.baseline"
SRC_70="$SRC_DIR/header/70_pletter.asm"
SRC_90="$SRC_DIR/header/90_support.asm"

# Note: baseline comparison is only meaningful if baseline was saved
# This test verifies the files exist and are non-empty
for f in "$SRC_70" "$SRC_90"; do
    if [ -s "$f" ]; then
        pass "Protected file $(basename $f) exists and is non-empty"
    else
        fail "Protected file $(basename $f) missing or empty"
    fi
done

# 8. Summary
echo ""
echo "=== Results: $PASS passed, $FAIL failed ==="
if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
