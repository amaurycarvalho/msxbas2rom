/***
 * @file test_options.cpp
 * @brief MSXBAS2ROM CLI options unit testing
 */

// NOLINTBEGIN

#include "build_options_setup.h"
#include "doctest/doctest.h"

TEST_SUITE("Options") {
  TEST_CASE("Parses auto mode with default plain compile mode") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "--auto";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.autoROM == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::Plain);
    CHECK(opts.megaROM == false);
  }

  TEST_CASE("Parses -4 flag for Konami4 compile mode") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "-4";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::Konami4);
    CHECK(opts.megaROM == true);
    CHECK(opts.outputFilename.find("[Konami].rom") != std::string::npos);
  }

  TEST_CASE("Parses --konami flag for Konami4 compile mode") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "--konami";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::Konami4);
    CHECK(opts.megaROM == true);
    CHECK(opts.outputFilename.find("[Konami].rom") != std::string::npos);
  }

  TEST_CASE("Parses -6 flag for ASCII16 compile mode") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "-6";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::ASCII16);
    CHECK(opts.megaROM == true);
    CHECK(opts.outputFilename.find("[ASCII16].rom") != std::string::npos);
  }

  TEST_CASE("Parses --ascii16 flag for ASCII16 compile mode") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "--ascii16";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::ASCII16);
    CHECK(opts.megaROM == true);
    CHECK(opts.outputFilename.find("[ASCII16].rom") != std::string::npos);
  }

  TEST_CASE("Parses -7 flag for ASCII16X compile mode") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "-7";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::ASCII16X);
    CHECK(opts.megaROM == true);
    CHECK(opts.outputFilename.find("[ASCII16X].rom") != std::string::npos);
  }

  TEST_CASE("Parses --ascii16x flag for ASCII16X compile mode") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "--ascii16x";
    char arg2[] = "program.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.compileMode == BuildOptions::CompileMode::ASCII16X);
    CHECK(opts.megaROM == true);
    CHECK(opts.outputFilename.find("[ASCII16X].rom") != std::string::npos);
  }

  TEST_CASE("Keeps explicit megarom selection when auto mode is present") {
    BuildOptionsSetup opts;
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
