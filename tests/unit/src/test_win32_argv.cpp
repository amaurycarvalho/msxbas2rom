/***
 * @file test_win32_argv.cpp
 * @brief Windows Unicode argv conversion unit tests (guarded)
 */
// NOLINTBEGIN

#include "win32_argv.h"
#include "doctest/doctest.h"

#ifdef _WIN32
TEST_SUITE("Win32Argv") {
  TEST_CASE("convertWindowsArgv returns original argv when __wargv is null") {
    std::vector<std::string> storage;
    std::vector<char*> argvPtrs;
    char arg0[] = "msxbas2rom";
    char* argv[] = {arg0};
    char** result = convertWindowsArgv(1, argv, storage, argvPtrs);
    CHECK(result == argv);
    CHECK(storage.empty());
    CHECK(argvPtrs.empty());
  }
}
#endif
// NOLINTEND
