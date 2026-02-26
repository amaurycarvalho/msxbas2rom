/***
 * @file test_parser.cpp
 * @brief MSXBAS2ROM parser unit testing
 */

// NOLINTBEGIN

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdio>
#include <fstream>
#include <string>

#include "doctest/doctest.h"
#include "lex.h"
#include "parse.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

TEST_SUITE("Parser") {
  TEST_CASE("Builds tags from valid numbered statements") {
    const std::string filename = createTempBas(
        "parser_valid.bas", "10 LET A=1\n20 PRINT A\n30 END\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);
    CHECK(parser.tags.size() >= 3);

    std::remove(filename.c_str());
  }

  TEST_CASE("Rejects INCLUDE with non-string parameter") {
    const std::string filename = createTempBas("parser_bad_include.bas",
                                               "10 INCLUDE 123\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);

    std::remove(filename.c_str());
  }

  TEST_CASE("Tracks FILE/TEXT directives as resources") {
    const std::string filename =
        createTempBas("parser_directive.bas", "FILE \"asset.bin\"\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);
    CHECK(parser.resourceCount == 1);

    std::remove(filename.c_str());
  }
}

// NOLINTEND
