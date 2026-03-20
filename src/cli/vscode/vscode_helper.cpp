#include "vscode_helper.h"

#include "fswrapper.h"

VSCodeHelper::VSCodeHelper(string compilerAppFilename) {
  this->compilerAppFilename = compilerAppFilename;

#ifdef Win
  this->emulatorAppFilename = "openmsx.exe";
#else
  //! @remark check if flatpak openMSX is installed (linux/macOS)
  if (pathExists("/var/lib/flatpak/app/org.openmsx.openMSX")) {
    this->emulatorAppFilename = "flatpak";
    this->emulatorAppArgs = "run org.openmsx.openMSX";
    this->emulatorAppArgsParsed = "\"run\",\"org.openmsx.openMSX\",";
  } else {
    this->emulatorAppFilename = "openmsx";
  }
#endif

  replaceAll(launchContent, "${emulatorAppFilename}", emulatorAppFilename);
  replaceAll(launchContent, "${emulatorAppArgs}", emulatorAppArgs);

  replaceAll(tasksContent, "${compilerAppFilename}", compilerAppFilename);
  replaceAll(tasksContent, "${emulatorAppFilename}", emulatorAppFilename);
  replaceAll(tasksContent, "${emulatorAppArgsParsed}", emulatorAppArgsParsed);
}

VSCodeHelper::~VSCodeHelper() = default;

bool VSCodeHelper::write(string fileName, string fileContent) {
  FILE* file;
  if ((file = fopen(fileName.c_str(), "w"))) {
    fwrite(fileContent.c_str(), 1, fileContent.size(), file);
    fclose(file);
    return true;
  }
  return false;
}

void VSCodeHelper::replaceAll(string& str, const string& from,
                              const string& to) {
  if (from.empty()) return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

string VSCodeHelper::getCompilerAppFilename() {
  return compilerAppFilename;
}

string VSCodeHelper::getEmulatorAppFilename() {
  return emulatorAppFilename;
}

bool VSCodeHelper::initialize() {
  string pathName = ".vscode";
  string launchName = pathJoin(pathName, "launch.json");
  string tasksName = pathJoin(pathName, "tasks.json");
  string debugName = pathJoin(pathName, "debug.tcl");

  if (createPath(pathName)) {
    write(launchName, launchContent);
    write(tasksName, tasksContent);
    write(debugName, debugContent);
    return true;
  }

  return false;
}

string VSCodeHelper::launchContent =
    R"({
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Run",
      "type": "node-terminal",
      "request": "launch",
      "command": "exit",
      "cwd": "${workspaceFolder}",
      "preLaunchTask": "Debug on openMSX"
    }
  ]
}
    )";

string VSCodeHelper::tasksContent =
    R"({
  "version": "2.0.0",
  "tasks": [
    {
      "label": "build",
      "type": "shell",

      "command": "${compilerAppFilename}",

      "args": ["${file}", "-a", "-q", "--noi"],

      "group": {
        "kind": "build",
        "isDefault": true
      },
      "problemMatcher": {
        "owner": "msxbas2rom",
        "fileLocation": ["relative", "${workspaceFolder}"],
        "pattern": {
          "regexp": "^(.+\\.bas):(\\d+): ERROR: (.+)$",
          "file": 1,
          "line": 2,
          "message": 3
        }
      }
    },

    {
      "label": "Run on openMSX",
      "dependsOn": ["build"],
      "type": "shell",
      "command": "${emulatorAppFilename}",
      "args": [
        ${emulatorAppArgsParsed}
        "-command",
        "'user_setting create string fileBasenameNoExtension \"File base name no extension\" \"${workspaceFolder}/${fileBasenameNoExtension}\"'",
        "-command",
        "'user_setting create boolean debugMode \"Debug mode\" false'",
        "-script",
        ".vscode/debug.tcl"
      ],
      "group": {
        "kind": "test",
        "isDefault": true
      }
    },

    {
      "label": "Debug on openMSX",
      "dependsOn": ["build"],
      "type": "shell",
      "command": "${emulatorAppFilename}",
      "args": [
        ${emulatorAppArgsParsed}
        "-command",
        "'user_setting create string fileBasenameNoExtension \"File base name no extension\" \"${workspaceFolder}/${fileBasenameNoExtension}\"'",
        "-command",
        "'user_setting create boolean debugMode \"Debug mode\" true'",
        "-script",
        ".vscode/debug.tcl"
      ],
      "group": {
        "kind": "test"
      }
    }
  ]
}
    )";

string VSCodeHelper::debugContent =
    R"(#debug.tcl

puts "==== MSXBAS2ROM Run Session ===="

#-----------------------------------------------
# symbol load procedure
#-----------------------------------------------

proc load_symbols {} {
  global noi_file

  puts "Loading debug symbols: $noi_file" 
  debug symbols load $noi_file NoICE

  # search for program start address symbol
  if {[catch {debug symbols lookup} result]} {
    puts "No symbols found."
    return
  }

  # iterate and filter by name containing "LIN_"
  foreach entry $result {
    if {[dict exists $entry name] && [dict exists $entry value]} {
        set name [dict get $entry name]

        if {[string first "LIN_" $name] != -1} {
            set addr [dict get $entry value]
            puts "Setting breakpoint for $name at: $addr"
            debug breakpoint create -address $addr
        }
    }
  }

  puts "Debugger ready."
}

#-----------------------------------------------
# main procedure
#-----------------------------------------------

proc main {} {
  global fileBasenameNoExtension debugMode noi_file

  #-----------------------------------------------
  # show parameters
  #-----------------------------------------------

  if {[llength $fileBasenameNoExtension] == 0} {
    puts "No .bas file found." 
    return  
  }
  puts "BAS base name: $fileBasenameNoExtension"

  puts "Debug mode: $debugMode"

  #-----------------------------------------------
  # get .rom files in current directory
  #-----------------------------------------------

  set rom_files [glob -nocomplain $fileBasenameNoExtension*.rom]
  if {[llength $rom_files] == 0} {
    puts "No .rom file found." 
    return 
  }
  set rom_file [lindex $rom_files 0]

  #-----------------------------------------------
  # load ROM into emulator
  #-----------------------------------------------

  puts "Loading ROM: $rom_file"
  cart $rom_file

  #-----------------------------------------------
  # run in debug mode
  #-----------------------------------------------

  if {$debugMode} {
    # get .noi files in current directory
    set noi_files [glob -nocomplain $fileBasenameNoExtension*.noi]
    if {[llength $noi_files] == 0} {
      puts "No .noi file found." 
      return 
    }
    set noi_file [lindex $noi_files 0]

    # run after emulator startup
    after 1000 load_symbols 
  }

  #-----------------------------------------------
  # delete parameters
  #-----------------------------------------------

  user_setting destroy fileBasenameNoExtension
  user_setting destroy debugMode
}

after 1000 main
    )";
