#include "vscode_helper.h"

#include <regex>

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

  launchContent =
      regex_replace(launchContent, regex(R"(^\$\{emulator App Filename\}$)"),
                    emulatorAppFilename);
  launchContent = regex_replace(
      launchContent, regex(R"(^\$\{emulator App Args\}$)"), emulatorAppArgs);

  tasksContent =
      regex_replace(tasksContent, regex(R"(^\$\{compiler App Filename\}$)"),
                    compilerAppFilename);
  tasksContent =
      regex_replace(tasksContent, regex(R"(^\$\{emulator App Filename\}$)"),
                    emulatorAppFilename);
  tasksContent =
      regex_replace(tasksContent, regex(R"(^\$\{emulator App Args Parsed\}$)"),
                    emulatorAppArgsParsed);
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
    return true;
  }

  return false;
}

string VSCodeHelper::launchContent =
    R"(
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Run",
      "type": "node-terminal",
      "request": "launch",
      "command": "${emulatorAppFilename} ${emulatorAppArgs} -cart ${fileBasenameNoExtension}*.rom -script .vscode/debug.tcl",
      "cwd": "${workspaceFolder}"
    }
  ]
}
    )";

string VSCodeHelper::tasksContent =
    R"(
{
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
        "-cart",
        "${workspaceFolder}/${fileBasenameNoExtension}*.rom"
      ],
      "group": {
        "kind": "test",
        "isDefault": true
      }
    }
  ]
}
    )";

string VSCodeHelper::debugContent =
    R"(
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
    )";
