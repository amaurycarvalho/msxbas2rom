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
  MSX_DEBUG_DELAY        emulated seconds before fallback capture (default: 5)
  MSX_DEBUG_MACHINE      machine name (default: Sharp_HB-8000_1.2)
  MSX_DEBUG_SETTINGS     optional openMSX settings.xml file to use
  MSX_DEBUG_SCREENSHOT_MODE
                         scaled (default) or raw
  MSX_DEBUG_SCREENSHOT   1 (default) or 0 to skip the screenshot artifact
  MSX_DEBUG_VRAM         1/full or START:LENGTH to capture physical VRAM
  MSX_DEBUG_VRAM_START   optional VRAM start address (default: 0)
  MSX_DEBUG_VRAM_LENGTH  optional VRAM byte count (default: physical size - start)
  MSX_DEBUG_BREAKPOINTS  comma-separated ADDRESS or LABEL=ADDRESS breakpoints,
                         optionally followed by @CONDITION, installed before the
                         ROM starts (for example
                         END_PGM=0x80a0,END_STMT+2=0x8065@[pc_in_slot X X 2])
  MSX_DEBUG_ACCELERATE  1/on to disable throttling when no input script is set
  MSX_DEBUG_INPUT        path to a timed input script
  MSX_DEBUG_THROTTLE     on or off; openMSX throttle setting
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
TCL_PROCS="$SCRIPT_DIR/msx-memory-capture-procs.tcl"
XTST_HELPER="$SCRIPT_DIR/xtest_key.py"

if [[ ! -f "$TCL_SCRIPT" ]]; then
  echo "ERROR: Tcl support script not found: $TCL_SCRIPT" >&2
  exit 1
fi
if [[ ! -f "$TCL_PROCS" ]]; then
  echo "ERROR: Tcl support procedures not found: $TCL_PROCS" >&2
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
SCREENSHOT="${MSX_DEBUG_SCREENSHOT:-1}"

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

case "$SCREENSHOT" in
  0|1|true|false|on|off) ;;
  *)
    echo "ERROR: MSX_DEBUG_SCREENSHOT must be 1/0/true/false/on/off: $SCREENSHOT" >&2
    exit 2
    ;;
esac

# Normalise a non-negative integer accepted as decimal (42) or hexadecimal
# (0x2a). Echo the decimal value.
parse_uint() {
  local value=$1
  if [[ "$value" =~ ^[0-9]+$ ]]; then
    printf '%d' "$value"
  elif [[ "$value" =~ ^0[xX][0-9a-fA-F]+$ ]]; then
    printf '%d' "$((value))"
  else
    return 1
  fi
}

VRAM_MODE="${MSX_DEBUG_VRAM:-}"
VRAM_START="${MSX_DEBUG_VRAM_START:-}"
VRAM_LENGTH="${MSX_DEBUG_VRAM_LENGTH:-}"

if [[ -n "$VRAM_MODE" ]]; then
  case "$VRAM_MODE" in
    0|false|off|"") ;;
    1|true|on|full) ;;
    *:*)
      VRAM_START=${VRAM_MODE%%:*}
      VRAM_LENGTH=${VRAM_MODE#*:}
      VRAM_MODE=1
      ;;
    *)
      echo "ERROR: MSX_DEBUG_VRAM must be 1/full or START:LENGTH: $VRAM_MODE" >&2
      exit 2
      ;;
  esac
fi

if [[ -n "$VRAM_START" && -n "$VRAM_MODE" && "$VRAM_MODE" != "0" && "$VRAM_MODE" != "false" && "$VRAM_MODE" != "off" ]]; then
  if ! VRAM_START=$(parse_uint "$VRAM_START"); then
    echo "ERROR: invalid MSX_DEBUG_VRAM_START: $VRAM_START" >&2
    exit 2
  fi
fi
if [[ -n "$VRAM_LENGTH" && -n "$VRAM_MODE" && "$VRAM_MODE" != "0" && "$VRAM_MODE" != "false" && "$VRAM_MODE" != "off" ]]; then
  if ! VRAM_LENGTH=$(parse_uint "$VRAM_LENGTH"); then
    echo "ERROR: invalid MSX_DEBUG_VRAM_LENGTH: $VRAM_LENGTH" >&2
    exit 2
  fi
fi

INPUT_SCRIPT="${MSX_DEBUG_INPUT:-}"
if [[ -n "$INPUT_SCRIPT" && ! -f "$INPUT_SCRIPT" ]]; then
  echo "ERROR: MSX_DEBUG_INPUT file not found: $INPUT_SCRIPT" >&2
  exit 1
fi

THROTTLE="${MSX_DEBUG_THROTTLE:-}"
if [[ -n "$THROTTLE" ]]; then
  case "$THROTTLE" in
    on|off) ;;
    *)
      echo "ERROR: MSX_DEBUG_THROTTLE must be 'on' or 'off': $THROTTLE" >&2
      exit 2
      ;;
  esac
fi

ACCELERATE="${MSX_DEBUG_ACCELERATE:-}"
if [[ -n "$ACCELERATE" ]]; then
  case "$ACCELERATE" in
    1|true|yes|on) ACCELERATE=1 ;;
    0|false|no|off|"") ACCELERATE=0 ;;
    *)
      echo "ERROR: MSX_DEBUG_ACCELERATE must be 1/0/true/false/on/off: $ACCELERATE" >&2
      exit 2
      ;;
  esac
else
  ACCELERATE=0
fi

if [[ "$ACCELERATE" == "1" && -z "$THROTTLE" && -z "$INPUT_SCRIPT" ]]; then
  THROTTLE=off
fi

BREAKPOINTS="${MSX_DEBUG_BREAKPOINTS:-}"

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
  -command "user_setting create string msxDebugScreenshot {MSX debug screenshot enabled} {$SCREENSHOT}"
  -command "user_setting create string msxDebugMachine {MSX debug machine} {$MACHINE}"
)

if [[ -n "$VRAM_MODE" ]]; then
  command+=(
    -command "user_setting create string msxDebugVram {MSX debug VRAM capture} {$VRAM_MODE}"
    -command "user_setting create string msxDebugVramStart {MSX debug VRAM start} {$VRAM_START}"
    -command "user_setting create string msxDebugVramLength {MSX debug VRAM length} {$VRAM_LENGTH}"
  )
fi

if [[ -n "$INPUT_SCRIPT" ]]; then
  command+=(
    -command "user_setting create string msxDebugInput {MSX debug input script} {$INPUT_SCRIPT}"
    -command "user_setting create string msxDebugXtstHelper {MSX debug XTEST helper} {$XTST_HELPER}"
  )
  if grep -Eq '^[[:space:]]*joy(button)?[[:space:]]' "$INPUT_SCRIPT"; then
    command+=(-command "plug joyporta keyjoystick1")
  fi
fi

if [[ -n "$THROTTLE" ]]; then
  command+=(-command "set throttle $THROTTLE")
fi

if [[ -n "$BREAKPOINTS" ]]; then
  # Procedures must exist before the breakpoint callbacks can fire.
  command+=(-command "source {$TCL_PROCS}")

  index=1
  IFS=',' read -r -a breakpoint_items <<< "$BREAKPOINTS"
  for item in "${breakpoint_items[@]}"; do
    item=$(echo "$item" | xargs)
    [[ -z "$item" ]] && continue

    label="bp${index}"
    condition=""
    if [[ "$item" == *"@"* ]]; then
      condition="${item#*@}"
      item="${item%%@*}"
      condition=$(echo "$condition" | xargs)
    fi

    address="$item"
    if [[ "$item" == *=* ]]; then
      label="${item%%=*}"
      address="${item#*=}"
    fi

    if ! address_decimal=$(parse_uint "$address"); then
      echo "ERROR: invalid breakpoint address: $item" >&2
      exit 2
    fi

    if [[ -n "$condition" ]]; then
      command+=(
        -command "debug set_bp ${address_decimal} {$condition} {msx_debug_bp_fired {$label} ${address_decimal}}"
      )
    else
      command+=(
        -command "debug set_bp ${address_decimal} 1 {msx_debug_bp_fired {$label} ${address_decimal}}"
      )
    fi
    index=$((index + 1))
  done
fi

command+=(-script "$TCL_SCRIPT")

printf 'Launching:'
printf ' %q' "${command[@]}"
printf '\n' >&2

"${command[@]}"
