#debug.tcl

puts "==== MSXBAS2ROM Debug Session ===="
#find first.noi file in current directory
    set noi_files[glob - nocomplain *.noi]

    if {[llength $noi_files] == 0} {puts "No .noi file found." return }

#get first file
set noi_file[lindex $noi_files 0]

#remove extension to get ROM name
    set rom[file rootname $noi_file]

    puts "ROM base name: $rom"

    proc load_symbols{} {
  global noi_file

      puts "Loading debug symbols: $noi_file" debug symbols load $noi_file NoICE
#resolve symbol
      if {
    [catch {debug symbols lookup - name START_PGM} result]
  }
  {puts "Symbol START_PGM not found." return }

#extract address
      set entry[lindex $result 0] set addr[dict get $entry value]

      puts "Setting breakpoint at start of the program: $addr" debug breakpoint
          create -
      address $addr

      puts "Debugger ready."
}

#run after emulator startup
after 1000 load_symbols
    