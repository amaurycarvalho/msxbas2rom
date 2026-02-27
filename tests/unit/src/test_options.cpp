/***
 * @file test_options.cpp
 * @brief MSXBAS2ROM CLI options unit testing
 */

// NOLINTBEGIN

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "options.h"

TEST_SUITE("Options") {
  TEST_CASE("Parses auto mode with default plain compile mode") {
    BuildOptions opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "--auto";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.autoROM == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::Plain);
    CHECK(opts.megaROM == false);
  }

  TEST_CASE("Keeps explicit megarom selection when auto mode is present") {
    BuildOptions opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "-a";
    char arg2[] = "-x";
    char arg3[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2, arg3};

    REQUIRE(opts.parse(4, argv) == true);
    CHECK(opts.autoROM == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::ASCII8);
    CHECK(opts.megaROM == true);
    CHECK(opts.outputFilename.find("[ASCII8].rom") != std::string::npos);
  }
}

// NOLINTEND
