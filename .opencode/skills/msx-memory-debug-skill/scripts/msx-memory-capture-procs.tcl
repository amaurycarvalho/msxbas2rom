# msx-memory-capture-procs.tcl
#
# Capture procedures shared by the early breakpoint setup (-command) and the
# main capture script (-script). Sourcing this file only defines procedures; it
# never schedules anything or exits openMSX.
#
# Parameters are supplied by the launcher as openMSX user_settings:
#   msxDebugRomPath, msxDebugDelay, msxDebugScreenshotMode
#   msxDebugScreenshot, msxDebugVram, msxDebugVramStart, msxDebugVramLength
#   msxDebugInput, msxDebugXtstHelper, msxDebugMachine, msxDebugBreakpoints

proc msx_debug_user_setting {name default} {
  if {[info exists ::$name]} {
    return [set ::$name]
  }
  return $default
}

proc msx_debug_bool {value default} {
  if {$value eq ""} {
    return $default
  }
  switch -- [string tolower $value] {
    1 - true - yes - on { return 1 }
    0 - false - no - off { return 0 }
    default { return $default }
  }
}

proc msx_debug_capture {trigger} {
  global msxDebugRomPath msxDebugScreenshotMode msxDebugCaptureDone
  global msxDebugVram msxDebugVramStart msxDebugVramLength
  global msxDebugMachine

  if {[info exists msxDebugCaptureDone] && $msxDebugCaptureDone} {
    return
  }
  set msxDebugCaptureDone 1

  # Release any joystick keys held by the input script before reading memory, so
  # a capture that exits early cannot leak a pressed host key into later runs.
  msx_debug_release_all

  set rom_path $msxDebugRomPath
  set output_dir [file dirname $rom_path]
  set base_name [file rootname [file tail $rom_path]]

  set ram_start 0xC000
  set ram_size  0x4000
  set ram_file [file join $output_dir "${base_name}.ram-c000-ffff.bin"]
  set screen_file [file join $output_dir "${base_name}.screen.png"]
  set screenshot_enabled [msx_debug_bool [msx_debug_user_setting msxDebugScreenshot 1] 1]

  puts "ROM: $rom_path"
  puts "Trigger: $trigger"
  puts "RAM range: 0xC000-0xFFFF"
  puts "RAM output: $ram_file"

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

  set vram_mode [msx_debug_user_setting msxDebugVram ""]
  set vram_size 0
  if {$vram_mode ne "" && [msx_debug_bool $vram_mode 0]} {
    set physical_size [debug size "physical VRAM"]
    set vram_start 0
    set vram_length $physical_size

    set configured_start [msx_debug_user_setting msxDebugVramStart ""]
    set configured_length [msx_debug_user_setting msxDebugVramLength ""]
    if {$configured_start ne ""} {
      if {![string is integer -strict $configured_start] || $configured_start < 0} {
        puts stderr "ERROR: invalid VRAM start address: $configured_start"
        return -code error "invalid VRAM start address"
      }
      set vram_start $configured_start
    }
    if {$configured_length ne ""} {
      if {![string is integer -strict $configured_length] || $configured_length < 0} {
        puts stderr "ERROR: invalid VRAM length: $configured_length"
        return -code error "invalid VRAM length"
      }
      set vram_length $configured_length
    }
    if {$vram_start + $vram_length > $physical_size} {
      set msg "ERROR: VRAM range $vram_start..[expr {$vram_start + $vram_length - 1}] exceeds physical VRAM size $physical_size"
      puts stderr $msg
      return -code error $msg
    }

    if {[catch {debug read_block "physical VRAM" $vram_start $vram_length} vram]} {
      puts stderr "ERROR: unable to read physical VRAM: $vram"
      return -code error $vram
    }
    if {[string length $vram] != $vram_length} {
      set msg "ERROR: expected $vram_length VRAM bytes, got [string length $vram]"
      puts stderr $msg
      return -code error $msg
    }

    set vram_file [file join $output_dir "${base_name}.vram.bin"]
    set vf [open $vram_file wb]
    fconfigure $vf -translation binary -encoding binary
    puts -nonewline $vf $vram
    close $vf

    # Capture the VDP register block so callers can derive screen/table
    # addresses without hardcoding machine-specific constants.
    set regs_file [file join $output_dir "${base_name}.vdp-regs.bin"]
    if {![catch {debug read_block "VDP regs" 0 64} regs]} {
      set rf [open $regs_file wb]
      fconfigure $rf -translation binary -encoding binary
      puts -nonewline $rf $regs
      close $rf
    }

    set vram_size $vram_length
    puts "VRAM range: $vram_start..[expr {$vram_start + $vram_length - 1}]"
    puts "VRAM output: $vram_file"
  }

  if {$screenshot_enabled} {
    if {$msxDebugScreenshotMode eq "raw"} {
      screenshot -raw $screen_file
    } else {
      screenshot $screen_file
    }
    puts "Screenshot: $screen_file"
  }

  set meta_file [file join $output_dir "${base_name}.capture.txt"]
  set wants_meta 0
  foreach setting {msxDebugVram msxDebugBreakpoints msxDebugInput} {
    if {[info exists ::$setting] && [set ::$setting] ne ""} {
      set wants_meta 1
    }
  }
  if {$wants_meta} {
    set mf [open $meta_file w]
    puts $mf "trigger=$trigger"
    puts $mf "emulated_time=[machine_info time]"
    puts $mf "machine=[msx_debug_user_setting msxDebugMachine ""]"
    puts $mf "ram_size=$ram_size"
    puts $mf "vram_size=$vram_size"
    puts $mf "screenshot=[expr {$screenshot_enabled ? 1 : 0}]"
    close $mf
  }

  puts "Capture completed."
  puts "RAM bytes: [string length $data]"
  if {$vram_size} {
    puts "VRAM bytes: $vram_size"
  }

  # Remove temporary settings before terminating the emulator.
  foreach setting {
    msxDebugRomPath msxDebugDelay msxDebugScreenshotMode msxDebugScreenshot
    msxDebugVram msxDebugVramStart msxDebugVramLength msxDebugInput
    msxDebugXtstHelper msxDebugMachine msxDebugBreakpoints
  } {
    catch {user_setting destroy $setting}
  }

  exit
}

proc msx_debug_bp_fired {label addr} {
  msx_debug_capture "breakpoint:$label@$addr"
}

# Keyboard matrix positions for keys that do not map to a printable character.
# Each value is {row bit-mask}.
proc msx_debug_key_matrix {name} {
  switch -- [string toupper $name] {
    SPACE      { return {8 1} }
    RETURN     { return {7 128} }
    ENTER      { return {7 128} }
    BACKSPACE  { return {7 32} }
    TAB        { return {7 8} }
    ESC - ESCAPE { return {7 2} }
    UP         { return {8 32} }
    DOWN       { return {8 64} }
    LEFT       { return {8 16} }
    RIGHT      { return {8 128} }
    SHIFT      { return {6 1} }
    CTRL       { return {6 2} }
    GRAPH      { return {6 4} }
    CAPS       { return {6 8} }
    default    { return {} }
  }
}

proc msx_debug_xtst {action keysym {hold 0.1}} {
  global msxDebugXtstHelper msxDebugHeldKeys
  if {![info exists msxDebugXtstHelper] || $msxDebugXtstHelper eq ""} {
    puts stderr "WARNING: no XTEST helper configured; joystick input skipped"
    return
  }
  if {[catch {exec python3 $msxDebugXtstHelper $action $keysym $hold &} err]} {
    puts stderr "WARNING: joystick input helper failed: $err"
    return
  }
  if {![info exists msxDebugHeldKeys]} {
    set msxDebugHeldKeys [dict create]
  }
  if {$action eq "press"} {
    dict set msxDebugHeldKeys $keysym 1
  } elseif {$action eq "release"} {
    catch {dict unset msxDebugHeldKeys $keysym}
  }
}

proc msx_debug_release_all {} {
  global msxDebugHeldKeys
  if {![info exists msxDebugHeldKeys]} {
    return
  }
  foreach keysym [dict keys $msxDebugHeldKeys] {
    msx_debug_xtst release $keysym 0
  }
  set msxDebugHeldKeys [dict create]
}

proc msx_debug_press_key {name duration} {
  set position [msx_debug_key_matrix $name]
  if {[llength $position] == 2} {
    lassign $position row mask
    keymatrixdown $row $mask
    if {$duration > 0} {
      after time $duration [list msx_debug_release_key $row $mask]
    } else {
      after time 0.05 [list msx_debug_release_key $row $mask]
    }
    return
  }

  # Single printable characters are typed through openMSX's keyboard input.
  if {[string length $name] == 1} {
    type $name
    return
  }
  puts stderr "WARNING: unsupported key in input script: $name"
}

proc msx_debug_release_key {row mask} {
  catch {keymatrixup $row $mask}
}

proc msx_debug_schedule_input {} {
  global msxDebugInput

  if {![info exists msxDebugInput] || $msxDebugInput eq ""} {
    return
  }
  if {![file exists $msxDebugInput]} {
    puts stderr "WARNING: input script not found: $msxDebugInput"
    return
  }

  set fh [open $msxDebugInput r]
  set line_number 0
  while {[gets $fh line] >= 0} {
    incr line_number
    set line [string trim $line]
    if {$line eq "" || [string index $line 0] eq "#"} {
      continue
    }
    set words [regexp -all -inline {\S+} $line]
    set command [lindex $words 0]
    if {[llength $words] < 2} {
      puts stderr "WARNING: malformed input line $line_number: $line"
      continue
    }
    set when [lindex $words 1]
    if {![string is double -strict $when] || $when < 0} {
      puts stderr "WARNING: invalid input time on line $line_number: $when"
      continue
    }

    switch -- $command {
      type {
        if {![regexp {^type\s+\S+\s+(.*)$} $line -> text]} {
          puts stderr "WARNING: malformed type line $line_number: $line"
          continue
        }
        after time $when [list type $text]
      }
      key {
        set name [lindex $words 2]
        set duration 0.1
        if {[llength $words] >= 4} {
          set duration [lindex $words 3]
        }
        after time $when [list msx_debug_press_key $name $duration]
      }
      joy {
        set direction [string tolower [lindex $words 2]]
        set state [string tolower [lindex $words 3]]
        set keysym [dict get {
          left Left right Right up Up down Down
        } $direction]
        set action [expr {$state eq "down" ? "press" : "release"}]
        after time $when [list msx_debug_xtst $action $keysym 0]
      }
      joybutton {
        set button [string tolower [lindex $words 2]]
        set state [string tolower [lindex $words 3]]
        set keysym [expr {$button eq "a" ? "Control_L" : "m"}]
        set action [expr {$state eq "down" ? "press" : "release"}]
        after time $when [list msx_debug_xtst $action $keysym 0]
      }
      default {
        puts stderr "WARNING: unknown input command on line $line_number: $command"
      }
    }
  }
  close $fh
}

proc msx_debug_start {} {
  global msxDebugDelay

  if {[info exists msxDebugDelay] && $msxDebugDelay ne ""} {
    if {![string is double -strict $msxDebugDelay] || $msxDebugDelay < 0} {
      puts stderr "ERROR: invalid msxDebugDelay: $msxDebugDelay"
      exit 2
    }
  } else {
    set msxDebugDelay 5
  }

  msx_debug_schedule_input

  puts "Waiting up to $msxDebugDelay emulated seconds for a breakpoint or fallback capture..."
  set regression_capture 0
  foreach setting {msxDebugVram msxDebugInput msxDebugBreakpoints} {
    if {[info exists ::$setting] && [set ::$setting] ne ""} {
      set regression_capture 1
    }
  }
  if {$regression_capture} {
    after time $msxDebugDelay [list msx_debug_fallback]
  } else {
    after time $msxDebugDelay [list msx_debug_capture fallback]
  }
}

proc msx_debug_fallback {} {
  # Capture at a VDP frame boundary so a fixed emulated-time fallback sees a
  # deterministic program state instead of a partially elapsed scanline/frame.
  after frame [list msx_debug_capture fallback]
}
