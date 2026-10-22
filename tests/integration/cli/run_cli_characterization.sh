#!/usr/bin/env bash
# End-to-end CLI characterization. Runs outside the unit/mutation binary.

set -u

APP="${APP_PATH:-../../bin/Release/msxbas2rom}"
if [ ! -x "$APP" ]; then
  echo "CLI characterization: app not found or not executable: $APP" >&2
  exit 1
fi

WORK="$(mktemp -d "${TMPDIR:-/tmp}/msxbas2rom_cli_XXXXXX")" || {
  echo "CLI characterization: cannot create temporary directory" >&2
  exit 1
}
cleanup() { rm -rf "$WORK"; }
trap cleanup EXIT

fail() {
  echo "CLI characterization FAILED: $*" >&2
  exit 1
}

contains() {
  printf '%s' "$1" | grep -Fq "$2" || fail "expected output to contain: $2"
}

run_capture() {
  set +e
  OUTPUT="$("$APP" "$@" 2>&1)"
  STATUS=$?
  set -e
}

write_program() {
  cat > "$WORK/$1" <<'BAS'
10 PRINT "HI"
20 END
BAS
}

set -e

write_program normal.bas
run_capture "$WORK/normal.bas"
[ "$STATUS" -eq 0 ] || fail "normal exit code $STATUS"
contains "$OUTPUT" "MSXBAS2ROM"
contains "$OUTPUT" "Converting "
contains "$OUTPUT" "Compilation finished with success"
find "$WORK" -maxdepth 1 -name '*.rom' -print -quit | grep -q . || \
  fail "normal compilation produced no ROM"

rm -f "$WORK"/*.rom
run_capture -q "$WORK/normal.bas"
[ "$STATUS" -eq 0 ] || fail "quiet exit code $STATUS"
[ -z "$OUTPUT" ] || fail "quiet mode produced output"
find "$WORK" -maxdepth 1 -name '*.rom' -print -quit | grep -q . || \
  fail "quiet compilation produced no ROM"

rm -f "$WORK"/*.rom
run_capture -d "$WORK/normal.bas"
[ "$STATUS" -eq 0 ] || fail "debug exit code $STATUS"
contains "$OUTPUT" "Displaying lexical analysis"
find "$WORK" -maxdepth 1 -name '*.rom' -print -quit | grep -q . || \
  fail "debug compilation produced no ROM"

rm -f "$WORK"/*.rom "$WORK"/*.symbol
run_capture --symbol "$WORK/normal.bas"
[ "$STATUS" -eq 0 ] || fail "symbol exit code $STATUS"
contains "$OUTPUT" "Symbols file created"
find "$WORK" -maxdepth 1 -name '*.symbol' -print -quit | grep -q . || \
  fail "symbol compilation produced no symbol file"

run_capture "$WORK/missing-input.bas"
[ "$STATUS" -eq 1 ] || fail "missing input exit code $STATUS"
contains "$OUTPUT" "ERROR: Input file not found!"

{
  line=10
  i=0
  while [ "$i" -lt 200 ]; do
    printf '%d A=%d\n' "$line" "$i"; line=$((line + 1))
    printf '%d B=A+%d\n' "$line" "$i"; line=$((line + 1))
    printf '%d C=A*B\n' "$line"; line=$((line + 1))
    printf '%d PRINT C\n' "$line"; line=$((line + 1))
    i=$((i + 1))
  done
  printf '%d END\n' "$line"
} > "$WORK/retry.bas"

rm -f "$WORK"/*.rom
run_capture -a "$WORK/retry.bas"
[ "$STATUS" -eq 0 ] || fail "retry exit code $STATUS"
contains "$OUTPUT" "Plain ROM overflow detected"
contains "$OUTPUT" "ASCII8 MegaROM"
contains "$OUTPUT" "Compilation finished with success"
find "$WORK" -maxdepth 1 -name '*.rom' -print -quit | grep -q . || \
  fail "retry compilation produced no ROM"

echo "CLI characterization passed"
