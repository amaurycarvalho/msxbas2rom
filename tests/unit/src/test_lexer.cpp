/***
 * @file test_lexer.cpp
 * @brief MSXBAS2ROM lexer unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "lexeme.h"
#include "lexer.h"
#include "lexer_line.h"
#include "logger.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static std::string createTempBin(const std::string& filename,
                                 const std::vector<unsigned char>& data) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path, std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
  ofs.close();
  return path;
}

static Lexeme* findLexemeByValue(LexerLine* line, const std::string& value) {
  if (!line) return nullptr;
  for (int i = 0; i < line->getLexemeCount(); i++) {
    Lexeme* lexeme = line->getLexeme(i);
    if (lexeme && lexeme->value == value) return lexeme;
  }
  return nullptr;
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

  TEST_CASE("Fails when input file is empty") {
    const std::string filename = createTempBas("lexer_empty.bas", "");

    Lexer lexer;
    CHECK(lexer.load(filename) == false);
    CHECK(lexer.logger->errors().toString().find("Empty file") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when input file does not exist") {
    Lexer lexer;
    CHECK(lexer.load("tmp/does_not_exist.bas") == false);
    CHECK(lexer.logger->errors().toString().find("File doesn't exist") !=
          std::string::npos);
  }

  TEST_CASE("Fails for tokenized MSX BASIC input") {
    const std::string filename = "tmp/lexer_tokenized.bas";
    std::ofstream ofs(filename, std::ios::binary);
    unsigned char data[] = {0xFF, 0x00, 0x80};
    ofs.write(reinterpret_cast<const char*>(data), sizeof(data));
    ofs.close();

    Lexer lexer;
    CHECK(lexer.load(filename) == false);
    CHECK(lexer.logger->errors().toString().find("Tokenized MSX BASIC") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails for non-MSXBASIC file header") {
    const std::string filename =
        createTempBin("lexer_invalid_header.bas", {0x01, 0x02, 0x03});

    Lexer lexer;
    CHECK(lexer.load(filename) == false);
    CHECK(lexer.logger->errors().toString().find("not a MSX BASIC") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Reports line and message on lexical error") {
    const std::string filename =
        createTempBas("lexer_invalid_token.bas", "10 A=1..2\n");

    Lexer lexer;
    REQUIRE(lexer.load(filename) == true);
    CHECK(lexer.evaluate() == false);
    CHECK(lexer.logger->containErrors());
    CHECK(lexer.logger->errors().toString().find("1..2") != std::string::npos);
    CHECK(lexer.logger->errors().toString().find("10 A=1..2") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Classifies numeric literal subtypes and separators") {
    const std::string filename = createTempBas(
        "lexer_numeric_ranges.bas", "10 A=32767:B=32768:C=123456:D=1234567\n");

    Lexer lexer;
    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    REQUIRE(lexer.lines.size() == 1);

    LexerLine* line = lexer.lines[0];
    REQUIRE(line != nullptr);

    Lexeme* num32767 = findLexemeByValue(line, "32767");
    Lexeme* num32768 = findLexemeByValue(line, "32768");
    Lexeme* num123456 = findLexemeByValue(line, "123456");
    Lexeme* num1234567 = findLexemeByValue(line, "1234567");

    REQUIRE(num32767 != nullptr);
    CHECK(num32767->subtype == Lexeme::subtype_numeric);
    REQUIRE(num32768 != nullptr);
    CHECK(num32768->subtype == Lexeme::subtype_single_decimal);
    REQUIRE(num123456 != nullptr);
    CHECK(num123456->subtype == Lexeme::subtype_single_decimal);
    REQUIRE(num1234567 != nullptr);
    CHECK(num1234567->subtype == Lexeme::subtype_double_decimal);

    Lexeme* sep = findLexemeByValue(line, ":");
    REQUIRE(sep != nullptr);
    CHECK(sep->type == Lexeme::type_separator);

    std::remove(filename.c_str());
  }

  TEST_CASE("Tokenizes hex literals, identifiers, operators and functions") {
    const std::string filename =
        createTempBas("lexer_hex_and_functions.bas", "10 A=&HFF:PRINT TIME\n");

    Lexer lexer;
    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    REQUIRE(lexer.lines.size() == 1);

    LexerLine* line = lexer.lines[0];
    REQUIRE(line != nullptr);

    Lexeme* hex = findLexemeByValue(line, "&HFF");
    REQUIRE(hex != nullptr);
    CHECK(hex->type == Lexeme::type_literal);

    Lexeme* identA = findLexemeByValue(line, "A");
    REQUIRE(identA != nullptr);
    CHECK(identA->type == Lexeme::type_identifier);

    Lexeme* opEq = findLexemeByValue(line, "=");
    REQUIRE(opEq != nullptr);
    CHECK(opEq->type == Lexeme::type_operator);

    Lexeme* kwPrint = findLexemeByValue(line, "PRINT");
    REQUIRE(kwPrint != nullptr);
    CHECK(kwPrint->type == Lexeme::type_keyword);

    Lexeme* kwTime = findLexemeByValue(line, "TIME");
    REQUIRE(kwTime != nullptr);
    CHECK(kwTime->subtype == Lexeme::subtype_function);

    std::remove(filename.c_str());
  }

  TEST_CASE("Handles REM and apostrophe comments") {
    SUBCASE("REM ends lexing for the line") {
      const std::string filename =
          createTempBas("lexer_rem.bas", "10 REM THIS IS A COMMENT\n");

      Lexer lexer;
      REQUIRE(lexer.load(filename) == true);
      REQUIRE(lexer.evaluate() == true);
      REQUIRE(lexer.lines.size() == 1);

      LexerLine* line = lexer.lines[0];
      REQUIRE(line != nullptr);
      Lexeme* rem = findLexemeByValue(line, "REM");
      REQUIRE(rem != nullptr);
      CHECK(rem->type == Lexeme::type_keyword);

      std::remove(filename.c_str());
    }

    SUBCASE("Apostrophe plus # creates comment lexeme") {
      const std::string filename =
          createTempBas("lexer_apostrophe.bas", "10 '#COMMENT\n");

      Lexer lexer;
      REQUIRE(lexer.load(filename) == true);
      REQUIRE(lexer.evaluate() == true);
      REQUIRE(lexer.lines.size() == 1);

      LexerLine* line = lexer.lines[0];
      REQUIRE(line != nullptr);

      Lexeme* apostrophe = findLexemeByValue(line, "'");
      REQUIRE(apostrophe != nullptr);
      CHECK(apostrophe->type == Lexeme::type_operator);

      Lexeme* comment = findLexemeByValue(line, "#COMMENT");
      REQUIRE(comment != nullptr);
      CHECK(comment->type == Lexeme::type_comment);

      std::remove(filename.c_str());
    }
  }

  TEST_CASE("Navigates lexemes in LexerLine") {
    LexerLine line;
    line.line = "10 PRINT A\n";
    REQUIRE(line.evaluate() == true);
    REQUIRE(line.getLexemeCount() >= 3);

    Lexeme* first = line.getFirstLexeme();
    REQUIRE(first != nullptr);
    CHECK(first->isLiteralNumeric());

    Lexeme* second = line.getNextLexeme();
    REQUIRE(second != nullptr);
    CHECK(second->isKeyword("PRINT"));

    line.pushLexeme();
    Lexeme* third = line.getNextLexeme();
    REQUIRE(third != nullptr);
    line.popLexeme();

    Lexeme* afterPop = line.getCurrentLexeme();
    REQUIRE(afterPop != nullptr);
    CHECK(afterPop == second);
  }
}

// NOLINTEND
