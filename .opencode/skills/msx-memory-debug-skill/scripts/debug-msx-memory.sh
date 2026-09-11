#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat >&2 <<'EOF'
Usage:
  debug-msx-memory.sh <rom-file>

Environment:
  OPENMSX_EXECUTABLE     openMSX executable (default: openmsx)
  OPENMSX_ARGS           extra openMSX arguments, as shell words
  OPENMSX_FLATPAK_APP    Flatpak app id; uses: flatpak run <app-id>
  MSX_DEBUG_DELAY        emulated seconds before capture (default: 5)
  MSX_DEBUG_MACHINE      machine name (default: Sharp_HB-8000_1.2)
  MSX_DEBUG_SETTINGS     optional openMSX settings.xml file to use
  MSX_DEBUG_SCREENSHOT_MODE
                         scaled (default) or raw
EOF
  exit 2
}

[[ $# -eq 1 ]] || usage

ROM_INPUT=$1
if [[ ! -f "$ROM_INPUT" ]]; then
  echo "ERROR: ROM file not found: $ROM_INPUT" >&2
  exit 1
fi

ROM_PATH=$(realpath "$ROM_INPUT")
ROM_DIR=$(dirname -- "$ROM_PATH")
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
TCL_SCRIPT="$SCRIPT_DIR/msx-memory-capture.tcl"

if [[ ! -f "$TCL_SCRIPT" ]]; then
  echo "ERROR: Tcl support script not found: $TCL_SCRIPT" >&2
  exit 1
fi

# openMSX loads the user's settings.xml at startup. That file can contain a
# host- or version-specific key binding that openMSX does not recognise (for
# example an unknown "PrintScreen" keycode), which makes openMSX abort before
# the capture script ever runs. Use an isolated settings file so the capture is
# reproducible and independent from the user's interactive configuration.
SETTINGS_TEMP_FILE=""
cleanup() {
  if [[ -n "$SETTINGS_TEMP_FILE" ]]; then
    rm -f -- "$SETTINGS_TEMP_FILE"
  fi
}
trap cleanup EXIT

# The project targets the Brazilian Sharp HB-8000 (Hotbit). Select that machine
# explicitly: relying on openMSX's default would boot C-BIOS instead, and the
# user's own default_machine is not available when settings are isolated.
# MSX_DEBUG_MACHINE overrides this.
MACHINE="${MSX_DEBUG_MACHINE:-Sharp_HB-8000_1.2}"

if [[ -n "${MSX_DEBUG_SETTINGS:-}" ]]; then
  if [[ ! -f "$MSX_DEBUG_SETTINGS" ]]; then
    echo "ERROR: MSX_DEBUG_SETTINGS file not found: $MSX_DEBUG_SETTINGS" >&2
    exit 1
  fi
  SETTINGS_FILE=$(realpath "$MSX_DEBUG_SETTINGS")
else
  # Keep the file beside the ROM: unlike the host /tmp, this location is also
  # reachable by sandboxed (Flatpak) openMSX builds.
  SETTINGS_TEMP_FILE=$(mktemp "$ROM_DIR/.msx-memory-debug.XXXXXX.xml")
  SETTINGS_FILE="$SETTINGS_TEMP_FILE"

  cat > "$SETTINGS_FILE" <<'XML'
<!DOCTYPE settings SYSTEM 'settings.dtd'>
<settings>
  <settings/>
  <bindings/>
  <shortcuts/>
</settings>
XML
fi

DELAY="${MSX_DEBUG_DELAY:-5}"
SCREENSHOT_MODE="${MSX_DEBUG_SCREENSHOT_MODE:-scaled}"

if ! [[ "$DELAY" =~ ^[0-9]+([.][0-9]+)?$ ]]; then
  echo "ERROR: MSX_DEBUG_DELAY must be a non-negative number: $DELAY" >&2
  exit 2
fi

case "$SCREENSHOT_MODE" in
  scaled|raw) ;;
  *)
    echo "ERROR: MSX_DEBUG_SCREENSHOT_MODE must be 'scaled' or 'raw'." >&2
    exit 2
    ;;
esac

# Build the launcher as an argv array. This avoids eval for the normal
# executable path while still allowing a small, conventional shell-word list
# in OPENMSX_ARGS.
if [[ -n "${OPENMSX_FLATPAK_APP:-}" ]]; then
  command=(flatpak run "$OPENMSX_FLATPAK_APP")
else
  OPENMSX_EXECUTABLE="${OPENMSX_EXECUTABLE:-openmsx}"
  command=("$OPENMSX_EXECUTABLE")

  if [[ -n "${OPENMSX_ARGS:-}" ]]; then
    # OPENMSX_ARGS is intentionally a shell-word list, not arbitrary shell code.
    # shellcheck disable=SC2206
    extra_args=( ${OPENMSX_ARGS} )
    command+=("${extra_args[@]}")
  fi
fi

# Load the isolated settings file, then select the machine explicitly.
command+=(-setting "$SETTINGS_FILE" -machine "$MACHINE")

# Pass values through openMSX user settings. Tcl reads them after startup.
command+=(
  -cart "$ROM_PATH"
  -command "user_setting create string msxDebugRomPath {MSX debug ROM path} {$ROM_PATH}"
  -command "user_setting create string msxDebugDelay {MSX debug delay} {$DELAY}"
  -command "user_setting create string msxDebugScreenshotMode {MSX debug screenshot mode} {$SCREENSHOT_MODE}"
  -script "$TCL_SCRIPT"
)

printf 'Launching:'
printf ' %q' "${command[@]}"
printf '\n' >&2

"${command[@]}"
