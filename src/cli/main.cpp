/***
 * @file main.cpp
 * @brief MSXBAS2ROM compiler CLI main entry
 * @author Amaury Carvalho
 * @copyright (GNU GPL3) 2019~
 */

#include <vector>

#ifdef _WIN32
#include "win32_argv.h"
#endif

#include "runner.h"

int main(int argc, char* argv[]) {
#ifdef _WIN32
  std::vector<std::string> utf8Storage;
  std::vector<char*> utf8Argv;
  argv = convertWindowsArgv(argc, argv, utf8Storage, utf8Argv);
#endif

  return runCli(argc, argv);
}
