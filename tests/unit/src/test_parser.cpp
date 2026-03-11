/***
 * @file test_parser.cpp
 * @brief MSXBAS2ROM parser unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static Lexeme* findSymbolByValue(Parser& parser, const std::string& value) {
  for (auto* lexeme : parser.getSymbolList()) {
    if (lexeme && lexeme->value == value) return lexeme;
  }
  return nullptr;
}

static bool hasActionWithValue(ActionNode* action, const std::string& value) {
  if (!action || !action->lexeme) return false;
  if (action->lexeme->value == value) return true;
  for (auto* child : action->actions) {
    if (hasActionWithValue(child, value)) return true;
  }
  return false;
}

static bool tagHasAction(const TagNode* tag, const std::string& value) {
  if (!tag) return false;
  for (auto* action : tag->actions) {
    if (hasActionWithValue(action, value)) return true;
  }
  return false;
}

TEST_SUITE("Parser") {
  TEST_CASE("Builds tags from valid numbered statements") {
    const std::string filename =
        createTempBas("parser_valid.bas", "10 LET A=1\n20 PRINT A\n30 END\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);
    CHECK(parser.getTags().size() >= 3);

    std::remove(filename.c_str());
  }

  TEST_CASE("Rejects INCLUDE with non-string parameter") {
    const std::string filename =
        createTempBas("parser_bad_include.bas", "10 INCLUDE 123\n");

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
    CHECK(parser.getResourceCount() == 1);

    std::remove(filename.c_str());
  }

  TEST_CASE("Splits statements by ':' outside IF") {
    const std::string filename =
        createTempBas("parser_split.bas", "10 A=1:PRINT A\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);
    REQUIRE(parser.getTags().size() == 1);

    const TagNode* tag = parser.getTags().front();
    CHECK(tag->actions.size() >= 2);
    CHECK(tagHasAction(tag, "LET") == true);
    CHECK(tagHasAction(tag, "PRINT") == true);

    std::remove(filename.c_str());
  }

  TEST_CASE("Stops parsing phrase after apostrophe comment") {
    const std::string filename =
        createTempBas("parser_comment.bas", "10 PRINT 1 ' PRINT 2\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);
    REQUIRE(parser.getTags().size() == 1);

    const TagNode* tag = parser.getTags().front();
    CHECK(tagHasAction(tag, "PRINT") == true);

    std::remove(filename.c_str());
  }

  TEST_CASE("Loads INCLUDE directive with valid string parameter") {
    const std::string incFile =
        createTempBas("parser_include_child.bas", "10 PRINT \"X\"\n");
    const std::string mainFile =
        createTempBas("parser_include_main.bas",
                      "INCLUDE \"tmp/parser_include_child.bas\"\n"
                      "10 END\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(mainFile) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);
    CHECK(parser.getTags().size() >= 2);

    std::remove(incFile.c_str());
    std::remove(mainFile.c_str());
  }

  TEST_CASE("Rejects INCLUDE with non-string parameter (message)") {
    const std::string filename =
        createTempBas("parser_bad_include_msg.bas", "INCLUDE 123\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);
    CHECK(parser.getLogger()->errors().toString().find(
              "Invalid parameter in INCLUDE keyword") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Rejects invalid expression unary symbol") {
    const std::string filename =
        createTempBas("parser_expr_unary.bas", "10 A=*1\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);
    CHECK(parser.getLogger()->errors().toString().find(
              "Invalid expression unary symbol") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Handles LET implicit and explicit assignments") {
    const std::string filename =
        createTempBas("parser_let.bas", "10 LET A=1\n20 B=2\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);
    REQUIRE(parser.getTags().size() == 2);

    const TagNode* tag1 = parser.getTags()[0];
    const TagNode* tag2 = parser.getTags()[1];
    CHECK(tagHasAction(tag1, "LET") == true);
    CHECK(tagHasAction(tag2, "LET") == true);

    std::remove(filename.c_str());
  }

  TEST_CASE("Applies DEFINT to symbol subtype") {
    const std::string filename =
        createTempBas("parser_defint.bas", "10 DEFINT A\n20 A=1\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);

    Lexeme* sym = findSymbolByValue(parser, "A");
    REQUIRE(sym != nullptr);
    CHECK(sym->subtype == Lexeme::subtype_numeric);

    std::remove(filename.c_str());
  }

  TEST_CASE("Rejects DIM without size parameter") {
    const std::string filename =
        createTempBas("parser_dim_invalid.bas", "10 DIM A\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);
    CHECK(parser.getLogger()->errors().toString().find(
              "Invalid array declaration") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Parses DATA and IDATA statements") {
    const std::string filename =
        createTempBas("parser_data.bas", "10 DATA 1,2,,&H0F\n20 IDATA &B10\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);

    CHECK(parser.getHasData() == true);
    CHECK(parser.getHasIData() == true);
    CHECK(parser.getDatas().size() >= 2);

    Lexeme* last = parser.getDatas().back();
    REQUIRE(last != nullptr);
    CHECK(last->subtype == Lexeme::subtype_integer_data);

    std::remove(filename.c_str());
  }

  TEST_CASE("Parses IF with THEN statement") {
    const std::string filename =
        createTempBas("parser_if.bas", "10 IF A=1 THEN PRINT A\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);

    std::remove(filename.c_str());
  }

  TEST_CASE("Rejects IF with ELSE but no THEN/GOTO/GOSUB") {
    const std::string filename =
        createTempBas("parser_if_invalid.bas", "10 IF A=1 ELSE PRINT A\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);
    CHECK(parser.getLogger()->errors().toString().find(
              "ELSE without a THEN/GOTO/GOSUB") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Parses FOR/NEXT statements") {
    const std::string filename = createTempBas(
        "parser_for_next.bas", "10 FOR I=1 TO 10 STEP 2\n20 NEXT I,J\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);

    std::remove(filename.c_str());
  }

  TEST_CASE("Parses PRINT USING and INPUT") {
    const std::string filename =
        createTempBas("parser_print_input.bas",
                      "10 PRINT USING \"###\";A\n20 INPUT \"A?\";A\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);

    std::remove(filename.c_str());
  }

  TEST_CASE("Parses OPEN/CLOSE/MAXFILES and rejects invalid forms") {
    const std::string okFile = createTempBas(
        "parser_open_ok.bas",
        "10 OPEN \"A\" FOR INPUT AS #1 LEN 1\n20 CLOSE #1\n30 MAXFILES=5\n");
    const std::string badOpen =
        createTempBas("parser_open_bad.bas", "10 OPEN \"A\" INPUT\n");
    const std::string badClose =
        createTempBas("parser_close_bad.bas", "10 CLOSE 1\n");

    Lexer lexer;
    Parser parser;

    REQUIRE(lexer.load(okFile) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == true);

    REQUIRE(lexer.load(badOpen) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);
    CHECK(parser.getLogger()->errors().toString().find(
              "FOR/AS is missing in OPEN statement") != std::string::npos);

    REQUIRE(lexer.load(badClose) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);
    CHECK(parser.getLogger()->errors().toString().find(
              "# is missing in CLOSE statement") != std::string::npos);

    std::remove(okFile.c_str());
    std::remove(badOpen.c_str());
    std::remove(badClose.c_str());
  }
}

// NOLINTEND
