/***
 * @file vscode_helper.h
 * @brief VSCode MSX-BASIC project Header
 */

#ifndef VSCODE_HELPER_HPP
#define VSCODE_HELPER_HPP

#include <string>

using namespace std;

class VSCodeHelper {
 private:
  string compilerAppFilename;
  string emulatorAppFilename;
  string emulatorAppArgs, emulatorAppArgsParsed;

  static string launchContent;
  static string tasksContent;
  static string debugContent;

  bool write(string fileName, string fileContent);

 public:
  string getCompilerAppFilename();
  string getEmulatorAppFilename();

  bool initialize();

  explicit VSCodeHelper(string compilerAppFilename);
  ~VSCodeHelper();
};

#endif  // VSCODE_HELPER_HPP
