# msx-memory-capture.tcl
#
# Capture the Z80-visible RAM range C000-FFFF, optionally VRAM and a
# screenshot, then exit. Capture happens at the first configured breakpoint or,
# when no breakpoint fires, after the configured emulated-time fallback.
#
# Parameters are supplied by the launcher as openMSX user_settings:
#   msxDebugRomPath
#   msxDebugDelay
#   msxDebugScreenshotMode
#   msxDebugScreenshot
#   msxDebugVram, msxDebugVramStart, msxDebugVramLength
#   msxDebugInput, msxDebugXtstHelper, msxDebugMachine

set script_dir [file dirname [info script]]
source [file join $script_dir msx-memory-capture-procs.tcl]

puts "==== MSX Memory Debug Session ===="

if {[info exists msxDebugScreenshotMode]} {
  if {$msxDebugScreenshotMode ni {scaled raw}} {
    puts stderr "ERROR: msxDebugScreenshotMode must be 'scaled' or 'raw'."
    exit 2
  }
}

# The ROM is already inserted by -cart. Wait for startup before scheduling the
# emulated-time fallback, so the script does not race the initial machine setup.
after time 0.1 [list msx_debug_start]
