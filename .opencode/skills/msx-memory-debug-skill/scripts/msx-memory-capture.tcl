# msx-memory-capture.tcl
#
# Capture the Z80-visible RAM range C000-FFFF and a screenshot, then exit.
#
# Parameters are supplied by the launcher as openMSX user_settings:
#   msxDebugRomPath
#   msxDebugDelay
#   msxDebugScreenshotMode

puts "==== MSX Memory Debug Session ===="

proc capture_memory {} {
  global msxDebugRomPath msxDebugScreenshotMode

  set rom_path $msxDebugRomPath
  set output_dir [file dirname $rom_path]
  set base_name [file rootname [file tail $rom_path]]

  set ram_start 0xC000
  set ram_size  0x4000
  set ram_file [file join $output_dir "${base_name}.ram-c000-ffff.bin"]
  set screen_file [file join $output_dir "${base_name}.screen.png"]

  puts "ROM: $rom_path"
  puts "RAM range: 0xC000-0xFFFF"
  puts "RAM output: $ram_file"
  puts "Screenshot output: $screen_file"

  # 'Main RAM' is an openMSX debuggable exposing the CPU-addressed main RAM.
  # read_block returns the complete block as a Tcl byte string.
  if {[catch {debug read_block "Main RAM" $ram_start $ram_size} data]} {
    puts stderr "ERROR: unable to read Main RAM: $data"
    return -code error $data
  }

  if {[string length $data] != $ram_size} {
    set msg "ERROR: expected $ram_size bytes, got [string length $data]"
    puts stderr $msg
    return -code error $msg
  }

  set fh [open $ram_file wb]
  fconfigure $fh -translation binary -encoding binary
  puts -nonewline $fh $data
  close $fh

  if {$msxDebugScreenshotMode eq "raw"} {
    screenshot -raw $screen_file
  } else {
    screenshot $screen_file
  }

  puts "Capture completed."
  puts "RAM bytes: [string length $data]"
  puts "Screenshot: $screen_file"

  # Remove temporary settings before terminating the emulator.
  catch {user_setting destroy msxDebugRomPath}
  catch {user_setting destroy msxDebugDelay}
  catch {user_setting destroy msxDebugScreenshotMode}

  exit
}

proc start_capture {} {
  global msxDebugDelay

  if {![info exists msxDebugDelay]} {
    puts stderr "ERROR: msxDebugDelay was not provided."
    exit 2
  }

  if {![string is double -strict $msxDebugDelay] || $msxDebugDelay < 0} {
    puts stderr "ERROR: invalid msxDebugDelay: $msxDebugDelay"
    exit 2
  }

  puts "Waiting $msxDebugDelay emulated seconds before capture..."
  after time $msxDebugDelay capture_memory
}

# The ROM is already inserted by -cart. Wait for startup before scheduling
# the emulated-time delay, so the script does not race the initial machine setup.
after time 0.1 start_capture
