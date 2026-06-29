/***
 * @file win32_argv.h
 * @brief Fail-safe Windows Unicode argv conversion
 * @author Amaury Carvalho
 * @copyright (GNU GPL3) 2019~
 */
#ifndef WIN32_ARGV_H
#define WIN32_ARGV_H

#ifdef _WIN32
#include <windows.h>

#include <string>
#include <vector>

/// Convert __wargv to UTF-8, falling back to original argv on failure.
/// @param argc  Argument count from main()
/// @param argv  Original narrow argv from main()
/// @param storage  Vector to keep converted strings alive through CLI parsing
/// @param argvPtrs  Parallel vector of pointers into storage for the parser
/// @return  Pointer to argvPtrs.data() on success, original argv on failure
static inline char** convertWindowsArgv(
    int argc, char* argv[], std::vector<std::string>& storage,
    std::vector<char*>& argvPtrs) {
  if (!__wargv) return argv;
  storage.reserve(argc);
  argvPtrs.reserve(argc);
  for (int i = 0; i < argc; i++) {
    wchar_t* warg = __wargv[i];
    if (!warg) return argv;
    int len = WideCharToMultiByte(CP_UTF8, 0, warg, -1, NULL, 0, NULL, NULL);
    if (len <= 0) return argv;
    std::string converted(static_cast<size_t>(len) - 1, '\0');
    if (WideCharToMultiByte(CP_UTF8, 0, warg, -1, &converted[0], len, NULL,
                            NULL) == 0)
      return argv;
    storage.push_back(std::move(converted));
    argvPtrs.push_back(&storage.back()[0]);
  }
  return argvPtrs.data();
}

#endif // _WIN32
#endif // WIN32_ARGV_H
