/***
 * @file test_lexer.cpp
 * @brief MSXBAS2ROM lexer unit testing
 */

// NOLINTBEGIN

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdio>
#include <fstream>
#include <string>

#include "doctest/doctest.h"
#include "lexer.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

TEST_SUITE("Lexer") {
  TEST_CASE("Tokenizes a valid BASIC line") {
    const std::string filename =
        createTempBas("lexer_valid.bas", "10 PRINT \"HELLO\"\n");

    Lexer lexer;
    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    REQUIRE(lexer.lines.size() == 1);

    LexerLine* line = lexer.lines[0];
    REQUIRE(line != nullptr);
    CHECK(line->getLexemeCount() >= 3);

    Lexeme* first = line->getFirstLexeme();
    REQUIRE(first != nullptr);
    CHECK(first->isLiteralNumeric());
    CHECK(first->value == "10");

    Lexeme* second = line->getNextLexeme();
    REQUIRE(second != nullptr);
    CHECK(second->isKeyword("PRINT"));

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when input file does not exist") {
    Lexer lexer;
    CHECK(lexer.load("tmp/does_not_exist.bas") == false);
    CHECK(lexer.errorMessage.find("File doesn't exist") != std::string::npos);
  }

  TEST_CASE("Fails for tokenized MSX BASIC input") {
    const std::string filename = "tmp/lexer_tokenized.bas";
    std::ofstream ofs(filename, std::ios::binary);
    unsigned char data[] = {0xFF, 0x00, 0x80};
    ofs.write(reinterpret_cast<const char*>(data), sizeof(data));
    ofs.close();

    Lexer lexer;
    CHECK(lexer.load(filename) == false);
    CHECK(lexer.errorMessage.find("Tokenized MSX BASIC") != std::string::npos);

    std::remove(filename.c_str());
  }
}

// NOLINTEND
