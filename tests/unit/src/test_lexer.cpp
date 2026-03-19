/***
 * @file test_lexer.cpp
 * @brief MSXBAS2ROM lexer unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "comment_state.h"
#include "doctest/doctest.h"
#include "identifier_state.h"
#include "keyword_state.h"
#include "lexeme.h"
#include "lexer.h"
#include "lexer_line_evaluator.h"
#include "lexer_line_state_factory.h"
#include "literal_state.h"
#include "logger.h"
#include "operator_state.h"
#include "separator_state.h"
#include "unknown_state.h"

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

static shared_ptr<Lexeme> findLexemeByValue(shared_ptr<LexerLineContext> line,
                                            const std::string& value) {
  if (!line) return nullptr;
  for (int i = 0; i < line->getLexemeCount(); i++) {
    shared_ptr<Lexeme> lexeme = line->getLexeme(i);
    if (lexeme && lexeme->value == value) return lexeme;
  }
  return nullptr;
}

static void seedLexeme(LexerLineStateContext& context, Lexeme::LexemeType type,
                       Lexeme::LexemeSubType subtype,
                       const std::string& value) {
  context.lexeme->type = type;
  context.lexeme->subtype = subtype;
  context.lexeme->value = value;
  context.lexeme->name = value;
}

TEST_SUITE("Lexer") {
  TEST_CASE("Tokenizes a valid BASIC line") {
    const std::string filename =
        createTempBas("lexer_valid.bas", "10 PRINT \"HELLO\"\n");

    Lexer lexer;
    REQUIRE(lexer.load(filename) == true);
    REQUIRE(lexer.evaluate() == true);
    REQUIRE(lexer.lines.size() == 1);

    shared_ptr<LexerLineEvaluator> line = lexer.lines[0];
    REQUIRE(line != nullptr);
    CHECK(line->getLexemeCount() >= 3);

    shared_ptr<Lexeme> first = line->getFirstLexeme();
    REQUIRE(first != nullptr);
    CHECK(first->isLiteralNumeric());
    CHECK(first->value == "10");

    shared_ptr<Lexeme> second = line->getNextLexeme();
    REQUIRE(second != nullptr);
    CHECK(second->isKeyword("PRINT"));

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when input file is empty") {
    const std::string filename = createTempBas("lexer_empty.bas", "");

    Lexer lexer;
    CHECK(lexer.load(filename) == false);
    CHECK(lexer.getLogger()->trace().toString().find("Empty file") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when input file does not exist") {
    Lexer lexer;
    CHECK(lexer.load("tmp/does_not_exist.bas") == false);
    CHECK(lexer.getLogger()->trace().toString().find("File doesn't exist") !=
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
    CHECK(lexer.getLogger()->trace().toString().find("Tokenized MSX BASIC") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails for non-MSXBASIC file header") {
    const std::string filename =
        createTempBin("lexer_invalid_header.bas", {0x01, 0x02, 0x03});

    Lexer lexer;
    CHECK(lexer.load(filename) == false);
    CHECK(lexer.getLogger()->trace().toString().find("not a MSX BASIC") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Reports line and message on lexical error") {
    const std::string filename =
        createTempBas("lexer_invalid_token.bas", "10 A=1..2\n");

    Lexer lexer;
    REQUIRE(lexer.load(filename) == true);
    CHECK(lexer.evaluate() == false);
    CHECK(lexer.getLogger()->containErrors());
    CHECK(lexer.getLogger()->trace().toString().find("1..2") !=
          std::string::npos);
    CHECK(lexer.getLogger()->trace().toString().find("10 A=1..2") !=
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

    shared_ptr<LexerLineEvaluator> line = lexer.lines[0];
    REQUIRE(line != nullptr);

    shared_ptr<Lexeme> num32767 = findLexemeByValue(line, "32767");
    shared_ptr<Lexeme> num32768 = findLexemeByValue(line, "32768");
    shared_ptr<Lexeme> num123456 = findLexemeByValue(line, "123456");
    shared_ptr<Lexeme> num1234567 = findLexemeByValue(line, "1234567");

    REQUIRE(num32767 != nullptr);
    CHECK(num32767->subtype == Lexeme::subtype_numeric);
    REQUIRE(num32768 != nullptr);
    CHECK(num32768->subtype == Lexeme::subtype_single_decimal);
    REQUIRE(num123456 != nullptr);
    CHECK(num123456->subtype == Lexeme::subtype_single_decimal);
    REQUIRE(num1234567 != nullptr);
    CHECK(num1234567->subtype == Lexeme::subtype_double_decimal);

    shared_ptr<Lexeme> sep = findLexemeByValue(line, ":");
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

    shared_ptr<LexerLineContext> line = lexer.lines[0];
    REQUIRE(line != nullptr);

    shared_ptr<Lexeme> hex = findLexemeByValue(line, "&HFF");
    REQUIRE(hex != nullptr);
    CHECK(hex->type == Lexeme::type_literal);

    shared_ptr<Lexeme> identA = findLexemeByValue(line, "A");
    REQUIRE(identA != nullptr);
    CHECK(identA->type == Lexeme::type_identifier);

    shared_ptr<Lexeme> opEq = findLexemeByValue(line, "=");
    REQUIRE(opEq != nullptr);
    CHECK(opEq->type == Lexeme::type_operator);

    shared_ptr<Lexeme> kwPrint = findLexemeByValue(line, "PRINT");
    REQUIRE(kwPrint != nullptr);
    CHECK(kwPrint->type == Lexeme::type_keyword);

    shared_ptr<Lexeme> kwTime = findLexemeByValue(line, "TIME");
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

      shared_ptr<LexerLineContext> line = lexer.lines[0];
      REQUIRE(line != nullptr);
      shared_ptr<Lexeme> rem = findLexemeByValue(line, "REM");
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

      shared_ptr<LexerLineContext> line = lexer.lines[0];
      REQUIRE(line != nullptr);

      shared_ptr<Lexeme> apostrophe = findLexemeByValue(line, "'");
      REQUIRE(apostrophe != nullptr);
      CHECK(apostrophe->type == Lexeme::type_operator);

      shared_ptr<Lexeme> comment = findLexemeByValue(line, "#COMMENT");
      REQUIRE(comment != nullptr);
      CHECK(comment->type == Lexeme::type_comment);

      std::remove(filename.c_str());
    }
  }

  TEST_CASE("Creates expected states from factory") {
    LexerLineStateFactory factory;

    ILexerLineState* unknown = factory.getState(Lexeme::type_unknown);
    ILexerLineState* literal = factory.getState(Lexeme::type_literal);
    ILexerLineState* identifier = factory.getState(Lexeme::type_identifier);
    ILexerLineState* keyword = factory.getState(Lexeme::type_keyword);
    ILexerLineState* op = factory.getState(Lexeme::type_operator);
    ILexerLineState* sep = factory.getState(Lexeme::type_separator);
    ILexerLineState* comment = factory.getState(Lexeme::type_comment);

    CHECK(dynamic_cast<UnknownState*>(unknown) != nullptr);
    CHECK(dynamic_cast<LiteralState*>(literal) != nullptr);
    CHECK(dynamic_cast<IdentifierState*>(identifier) != nullptr);
    CHECK(dynamic_cast<KeywordState*>(keyword) != nullptr);
    CHECK(dynamic_cast<OperatorState*>(op) != nullptr);
    CHECK(dynamic_cast<SeparatorState*>(sep) != nullptr);
    CHECK(dynamic_cast<CommentState*>(comment) != nullptr);

    ILexerLineState* fallback =
        factory.getState(static_cast<Lexeme::LexemeType>(999));
    CHECK(dynamic_cast<UnknownState*>(fallback) != nullptr);

    CHECK(factory.getState(Lexeme::type_keyword) == keyword);
  }

  TEST_CASE("UnknownState handles lexeme transitions") {
    UnknownState state;

    SUBCASE("Whitespace is ignored") {
      LexerLineEvaluator line;
      line.lineText = " ";
      LexerLineStateContext context(&line);
      context.current = ' ';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(line.getLexemeCount() == 0);
    }

    SUBCASE("Numeric literal starts") {
      LexerLineEvaluator line;
      line.lineText = "1";
      LexerLineStateContext context(&line);
      context.current = '1';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->type == Lexeme::type_literal);
      CHECK(context.lexeme->subtype == Lexeme::subtype_numeric);
      CHECK(context.lexeme->value == "1");
    }

    SUBCASE("Operator is pushed immediately") {
      LexerLineEvaluator line;
      line.lineText = "+";
      LexerLineStateContext context(&line);
      context.current = '+';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->type == Lexeme::type_operator);
      CHECK(lexeme->value == "+");
    }

    SUBCASE("Apostrophe comment creates operator and comment lexemes") {
      LexerLineEvaluator line;
      line.lineText = "'#COMMENT\n";
      LexerLineStateContext context(&line);
      context.current = '\'';

      CHECK(state.handle(context) == LexerLineProcessResult::Accept);
      REQUIRE(line.getLexemeCount() == 2);

      shared_ptr<Lexeme> op = line.getLexeme(0);
      shared_ptr<Lexeme> comment = line.getLexeme(1);
      REQUIRE(op != nullptr);
      REQUIRE(comment != nullptr);
      CHECK(op->type == Lexeme::type_operator);
      CHECK(op->value == "'");
      CHECK(comment->type == Lexeme::type_comment);
      CHECK(comment->value == "#COMMENT");
    }

    SUBCASE("Invalid character rejects") {
      LexerLineEvaluator line;
      line.lineText = "@";
      LexerLineStateContext context(&line);
      context.current = '@';

      CHECK(state.handle(context) == LexerLineProcessResult::Reject);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->type == Lexeme::type_unknown);
      CHECK(lexeme->value == "@");
    }
  }

  TEST_CASE("LiteralState processes numbers and strings") {
    LiteralState state;

    SUBCASE("String literal closes on quote") {
      LexerLineEvaluator line;
      line.lineText = "\"HELLO\"";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_literal, Lexeme::subtype_string,
                 "\"HELLO");
      context.current = '"';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->value == "\"HELLO\"");
    }

    SUBCASE("Hex literal accepts valid digit and rejects invalid") {
      LexerLineEvaluator line;
      line.lineText = "&HFG";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_literal, Lexeme::subtype_numeric, "&H");
      context.hexa = true;
      context.index = 2;
      context.current = 'F';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->value == "&HF");

      context.current = 'G';
      context.index = 3;
      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->value == "&HF");
      CHECK(context.index == 2);
    }

    SUBCASE("Double decimal rejects second dot") {
      LexerLineEvaluator line;
      line.lineText = "1..";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_literal, Lexeme::subtype_double_decimal,
                 "1.");
      context.current = '.';

      CHECK(state.handle(context) == LexerLineProcessResult::Reject);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->type == Lexeme::type_unknown);
      CHECK(lexeme->value == "1..");
    }

    SUBCASE("Numeric range promotes subtype") {
      LexerLineEvaluator line;
      line.lineText = "327678";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_literal, Lexeme::subtype_numeric,
                 "32767");
      context.current = '8';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->subtype == Lexeme::subtype_single_decimal);
    }

    SUBCASE("Single decimal grows to double decimal") {
      LexerLineEvaluator line;
      line.lineText = "1234567";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_literal, Lexeme::subtype_single_decimal,
                 "123456");
      context.current = '7';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->subtype == Lexeme::subtype_double_decimal);
    }

    SUBCASE("Numeric suffix adjusts subtype") {
      LexerLineEvaluator line;
      line.lineText = "10%";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_literal, Lexeme::subtype_numeric, "10");
      context.current = '%';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->subtype == Lexeme::subtype_numeric);
    }
  }

  TEST_CASE("IdentifierState classifies keywords and suffixes") {
    IdentifierState state;

    SUBCASE("Non-identifier terminates and normalizes keyword") {
      LexerLineEvaluator line;
      line.lineText = "PRINT ";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_identifier, Lexeme::subtype_any,
                 "PRINT");
      context.current = ' ';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->type == Lexeme::type_keyword);
      CHECK(lexeme->value == "PRINT");
    }

    SUBCASE("REM accepts and stops line") {
      LexerLineEvaluator line;
      line.lineText = "REM X";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_identifier, Lexeme::subtype_any, "RE");
      context.index = 2;
      context.current = 'M';

      CHECK(state.handle(context) == LexerLineProcessResult::Accept);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->value == "REM");
      CHECK(lexeme->type == Lexeme::type_keyword);
    }

    SUBCASE("Suffix sets identifier subtype") {
      LexerLineEvaluator line;
      line.lineText = "A%";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_identifier,
                 Lexeme::subtype_single_decimal, "A");
      context.current = '%';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->subtype == Lexeme::subtype_numeric);
    }
  }

  TEST_CASE("KeywordState accumulates keyword characters") {
    KeywordState state;

    SUBCASE("Identifier character continues") {
      LexerLineEvaluator line;
      line.lineText = "PRIN";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_keyword, Lexeme::subtype_any, "PRI");
      context.current = 'N';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->value == "PRIN");
    }

    SUBCASE("Whitespace pushes lexeme") {
      LexerLineEvaluator line;
      line.lineText = "PRINT ";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_keyword, Lexeme::subtype_any, "PRINT");
      context.current = ' ';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->value == "PRINT");
    }
  }

  TEST_CASE("OperatorState appends or pushes operators") {
    OperatorState state;

    SUBCASE("Operator character appends") {
      LexerLineEvaluator line;
      line.lineText = "+=";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_operator, Lexeme::subtype_any, "+");
      context.current = '=';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->value == "+=");
    }

    SUBCASE("Non-operator pushes lexeme") {
      LexerLineEvaluator line;
      line.lineText = "+A";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_operator, Lexeme::subtype_any, "+");
      context.current = 'A';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->value == "+");
    }
  }

  TEST_CASE("SeparatorState appends or pushes separators") {
    SeparatorState state;

    SUBCASE("Separator character appends") {
      LexerLineEvaluator line;
      line.lineText = "::";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_separator, Lexeme::subtype_any, ":");
      context.current = ':';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      CHECK(context.lexeme->value == "::");
    }

    SUBCASE("Non-separator pushes lexeme") {
      LexerLineEvaluator line;
      line.lineText = ":A";
      LexerLineStateContext context(&line);
      seedLexeme(context, Lexeme::type_separator, Lexeme::subtype_any, ":");
      context.current = 'A';

      CHECK(state.handle(context) == LexerLineProcessResult::Continue);
      REQUIRE(line.getLexemeCount() == 1);
      shared_ptr<Lexeme> lexeme = line.getLexeme(0);
      REQUIRE(lexeme != nullptr);
      CHECK(lexeme->value == ":");
    }
  }

  TEST_CASE("CommentState currently ends immediately") {
    CommentState state;

    LexerLineEvaluator line;
    line.lineText = "REM A";
    LexerLineStateContext context(&line);
    seedLexeme(context, Lexeme::type_comment, Lexeme::subtype_any, "REM");
    context.current = 'A';

    CHECK(state.handle(context) == LexerLineProcessResult::Continue);
    REQUIRE(line.getLexemeCount() == 1);
    shared_ptr<Lexeme> lexeme = line.getLexeme(0);
    REQUIRE(lexeme != nullptr);
    CHECK(lexeme->value == "REM");
  }

  TEST_CASE("Navigates lexemes in LexerLine") {
    LexerLineEvaluator line;
    line.lineText = "10 PRINT A\n";
    REQUIRE(line.evaluate() == true);
    REQUIRE(line.getLexemeCount() >= 3);

    shared_ptr<Lexeme> first = line.getFirstLexeme();
    REQUIRE(first != nullptr);
    CHECK(first->isLiteralNumeric());

    shared_ptr<Lexeme> second = line.getNextLexeme();
    REQUIRE(second != nullptr);
    CHECK(second->isKeyword("PRINT"));

    line.pushLexeme();
    shared_ptr<Lexeme> third = line.getNextLexeme();
    REQUIRE(third != nullptr);
    line.popLexeme();

    shared_ptr<Lexeme> afterPop = line.getCurrentLexeme();
    REQUIRE(afterPop != nullptr);
    CHECK(afterPop == second);
  }
}

// NOLINTEND
