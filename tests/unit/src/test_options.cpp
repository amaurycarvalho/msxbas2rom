/***
 * @file test_options.cpp
 * @brief MSXBAS2ROM CLI options unit testing
 */

// NOLINTBEGIN

#include <cstring>
#include <stdexcept>

#include "build_options_setup.h"
#include "cliparser.h"
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

  TEST_CASE("Accepts dash-prefixed filename after double dash") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "--";
    char arg2[] = "-weird.bas";
    char* argv[] = {arg0, arg1, arg2};

    REQUIRE(opts.parse(3, argv) == true);
    CHECK(opts.error == false);
    CHECK(opts.inputFilename.find("-weird.bas") != std::string::npos);
  }

  TEST_CASE("Rejects unknown option flags") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "-q";
    char arg2[] = "-z";
    char arg3[] = "f.bas";
    char* argv[] = {arg0, arg1, arg2, arg3};

    CHECK(opts.parse(4, argv) == false);
    CHECK(opts.error == true);
    CHECK(opts.errorMessage.find("Unexpected argument") != std::string::npos);
  }

  TEST_CASE("Rejects a second filename argument") {
    BuildOptionsSetup opts;
    char arg0[] = "msxbas2rom";
    char arg1[] = "f.bas";
    char arg2[] = "g.bas";
    char* argv[] = {arg0, arg1, arg2};

    CHECK(opts.parse(3, argv) == false);
    CHECK(opts.error == true);
    CHECK(opts.errorMessage.find("Unexpected argument") != std::string::npos);
  }
}

TEST_SUITE("CommandLineParser") {
  TEST_CASE("Parses option with value and filename") {
    CommandLineParser parser;
    std::string captured;
    parser.addOption("-x", "--xxx", "desc", true, false,
                     [&](const std::string& v) { captured = v; });

    char arg0[] = "prog";
    char arg1[] = "-x";
    char arg2[] = "42";
    char arg3[] = "file.bas";
    char* argv[] = {arg0, arg1, arg2, arg3};
    parser.parse(4, argv);

    CHECK(captured == "42");
    CHECK(parser.getFilename() == "file.bas");
  }

  TEST_CASE("Missing option value throws") {
    CommandLineParser parser;
    parser.addOption("-x", "--xxx", "desc", true, false,
                     [](const std::string&) {});

    char arg0[] = "prog";
    char arg1[] = "-x";
    char* argv[] = {arg0, arg1};
    CHECK_THROWS_AS(parser.parse(2, argv), std::runtime_error);
  }
}

// NOLINTEND
