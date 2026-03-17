/***
 * @file test_parser.cpp
 * @brief MSXBAS2ROM parser unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "action_node.h"
#include "call_statement_strategy.h"
#include "cmd_statement_strategy.h"
#include "color_statement_strategy.h"
#include "data_statement_strategy.h"
#include "def_statement_strategy.h"
#include "dim_statement_strategy.h"
#include "doctest/doctest.h"
#include "file_statement_strategy.h"
#include "for_statement_strategy.h"
#include "generic_statement_strategy.h"
#include "get_statement_strategy.h"
#include "graphics_statement_strategy.h"
#include "idata_statement_strategy.h"
#include "if_statement_strategy.h"
#include "input_statement_strategy.h"
#include "let_statement_strategy.h"
#include "lexeme.h"
#include "lexer.h"
#include "lexer_line_evaluator.h"
#include "logger.h"
#include "next_statement_strategy.h"
#include "noop_statement_strategy.h"
#include "on_statement_strategy.h"
#include "parser.h"
#include "parser_context.h"
#include "print_statement_strategy.h"
#include "put_statement_strategy.h"
#include "screen_statement_strategy.h"
#include "set_statement_strategy.h"
#include "sprite_statement_strategy.h"
#include "time_statement_strategy.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static Lexeme* findSymbolByValue(Parser& parser, const std::string& value) {
  for (auto& lexeme : parser.getSymbolList()) {
    if (lexeme && lexeme->value == value) return lexeme.get();
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
    CHECK(parser.getLogger()->trace().toString().find(
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
    CHECK(parser.getLogger()->trace().toString().find(
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
    CHECK(parser.getLogger()->trace().toString().find(
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

    Lexeme* last = parser.getDatas().back().get();
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
    CHECK(parser.getLogger()->trace().toString().find(
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
    CHECK(parser.getLogger()->trace().toString().find(
              "FOR/AS is missing in OPEN statement") != std::string::npos);

    REQUIRE(lexer.load(badClose) == true);
    REQUIRE(lexer.evaluate() == true);
    CHECK(parser.evaluate(&lexer) == false);
    CHECK(parser.getLogger()->trace().toString().find(
              "# is missing in CLOSE statement") != std::string::npos);

    std::remove(okFile.c_str());
    std::remove(badOpen.c_str());
    std::remove(badClose.c_str());
  }
}

TEST_SUITE("ParserStatementStrategyFactory") {
  TEST_CASE("All parser strategies are registered") {
    ParserStatementStrategyFactory factory;

    std::vector<std::string> keywords = {

        "REM",    "CLS",      "END",    "BEEP",    "RANDOMIZE", "'",

        "WIDTH",  "CLEAR",    "ERASE",  "LOCATE",  "DRAW",      "GOTO",
        "GOSUB",  "RETURN",   "SOUND",  "RESTORE", "RESUME",    "READ",
        "IREAD",  "IRESTORE", "POKE",   "IPOKE",   "VPOKE",     "OUT",
        "SWAP",   "WAIT",     "SEED",   "BLOAD",   "PLAY",

        "LET",    "DIM",      "REDIM",

        "PRINT",  "?",        "INPUT",

        "DATA",   "IDATA",

        "SCREEN", "SPRITE",   "BASE",   "VDP",

        "PUT",    "TIME",     "SET",    "GET",

        "ON",     "INTERVAL", "STOP",   "KEY",     "STRIG",

        "COLOR",  "CMD",      "OPEN",   "CLOSE",   "MAXFILES",

        "CALL",   "_",

        "DEF",    "DEFINT",   "DEFSTR", "DEFSNG",  "DEFDBL",

        "IF",     "FOR",      "NEXT",

        "PSET",   "PRESET",   "LINE",   "CIRCLE",  "PAINT",     "COPY"};

    for (const auto& kw : keywords) {
      CHECK(factory.getStrategyByKeyword(kw) != nullptr);
    }
  }

  TEST_CASE("Unknown keyword returns nullptr") {
    ParserStatementStrategyFactory factory;

    CHECK(factory.getStrategyByKeyword("FOOBAR") == nullptr);
    CHECK(factory.getStrategyByKeyword("") == nullptr);
    CHECK(factory.getStrategyByKeyword("INVALID") == nullptr);
  }

  TEST_CASE("PRINT aliases share same strategy") {
    ParserStatementStrategyFactory factory;

    auto* s1 = factory.getStrategyByKeyword("PRINT");
    auto* s2 = factory.getStrategyByKeyword("?");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("CALL aliases share same strategy") {
    ParserStatementStrategyFactory factory;

    auto* s1 = factory.getStrategyByKeyword("CALL");
    auto* s2 = factory.getStrategyByKeyword("_");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("ON family shares same strategy") {
    ParserStatementStrategyFactory factory;

    auto* base = factory.getStrategyByKeyword("ON");

    CHECK(base != nullptr);
    CHECK(base == factory.getStrategyByKeyword("INTERVAL"));
    CHECK(base == factory.getStrategyByKeyword("STOP"));
    CHECK(base == factory.getStrategyByKeyword("KEY"));
    CHECK(base == factory.getStrategyByKeyword("STRIG"));
  }

  TEST_CASE("DEF family shares same strategy") {
    ParserStatementStrategyFactory factory;

    auto* base = factory.getStrategyByKeyword("DEF");

    CHECK(base != nullptr);
    CHECK(base == factory.getStrategyByKeyword("DEFINT"));
    CHECK(base == factory.getStrategyByKeyword("DEFSTR"));
    CHECK(base == factory.getStrategyByKeyword("DEFSNG"));
    CHECK(base == factory.getStrategyByKeyword("DEFDBL"));
  }

  TEST_CASE("Graphics commands share same strategy") {
    ParserStatementStrategyFactory factory;

    auto* base = factory.getStrategyByKeyword("PSET");

    CHECK(base != nullptr);

    CHECK(base == factory.getStrategyByKeyword("PRESET"));
    CHECK(base == factory.getStrategyByKeyword("LINE"));
    CHECK(base == factory.getStrategyByKeyword("CIRCLE"));
    CHECK(base == factory.getStrategyByKeyword("PAINT"));
    CHECK(base == factory.getStrategyByKeyword("COPY"));
  }

  TEST_CASE("Repeated calls return same instance") {
    ParserStatementStrategyFactory factory;

    auto* s1 = factory.getStrategyByKeyword("PRINT");
    auto* s2 = factory.getStrategyByKeyword("PRINT");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("All statement strategies have unit tests") {
    ParserStatementStrategyFactory factory;

    CHECK(factory.size() == 71);
  }
}

static shared_ptr<Lexeme> kw(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, v);
}

static shared_ptr<Lexeme> sep(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_separator, Lexeme::subtype_any, v);
}

static shared_ptr<Lexeme> lit(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_basic_string,
                             v);
}

static shared_ptr<Lexeme> num(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_numeric, v);
}

static shared_ptr<Lexeme> id(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_identifier, Lexeme::subtype_any, v);
}

static shared_ptr<Lexeme> op(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_operator, Lexeme::subtype_any, v);
}

static unique_ptr<ParserContext> createContext() {
  unique_ptr<ParserContext> ctx;
  ctx.reset(new ParserContext());
  ctx->tag = new TagNode();
  ctx->actionRoot = new ActionNode();
  return ctx;
}

static void setActionRoot(ParserContext& ctx, const std::string& keyword) {
  ctx.actionRoot->lexeme =
      make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, keyword);
}

TEST_SUITE("PrintStatementStrategy") {
  TEST_CASE("PRINT simple literal") {
    unique_ptr<ParserContext> ctx = createContext();
    PrintStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(lit("\"HELLO\""));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }

  TEST_CASE("PRINT with comma separator") {
    unique_ptr<ParserContext> ctx = createContext();
    PrintStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(lit("\"A\""));
    line.addLexeme(sep(","));
    line.addLexeme(lit("\"B\""));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }

  TEST_CASE("PRINT with semicolon separator") {
    unique_ptr<ParserContext> ctx = createContext();
    PrintStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(lit("\"A\""));
    line.addLexeme(sep(";"));
    line.addLexeme(lit("\"B\""));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }

  TEST_CASE("PRINT to channel") {
    unique_ptr<ParserContext> ctx = createContext();
    PrintStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(sep("#"));
    line.addLexeme(lit("1"));
    line.addLexeme(sep(","));
    line.addLexeme(lit("\"HELLO\""));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }

  TEST_CASE("PRINT USING valid") {
    unique_ptr<ParserContext> ctx = createContext();
    PrintStatementStrategy strategy;

    LexerLineContext line;

    line.addLexeme(kw("USING"));
    line.addLexeme(lit("\"###\""));
    line.addLexeme(sep(";"));
    line.addLexeme(lit("123"));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }

  TEST_CASE("PRINT USING invalid syntax") {
    unique_ptr<ParserContext> ctx = createContext();
    PrintStatementStrategy strategy;

    LexerLineContext line;

    line.addLexeme(kw("USING"));
    line.addLexeme(sep(","));  // inválido

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == false);
  }

  TEST_CASE("Alias ? converts to PRINT") {
    unique_ptr<ParserContext> ctx = createContext();
    PrintStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(lit("\"HELLO\""));

    shared_ptr<Lexeme> lex =
        make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, "?");

    bool result = strategy.execute(*ctx, &line, lex);

    CHECK(result == true);
    CHECK(lex->value == "PRINT");
  }
}

TEST_SUITE("GenericStatementStrategy") {
  TEST_CASE("Parses a simple expression list") {
    unique_ptr<ParserContext> ctx = createContext();
    GenericStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(num("1"));
    line.addLexeme(sep(","));
    line.addLexeme(num("2"));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }
}

TEST_SUITE("LetStatementStrategy") {
  TEST_CASE("Parses a basic assignment") {
    unique_ptr<ParserContext> ctx = createContext();
    LetStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(id("A"));
    line.addLexeme(op("="));
    line.addLexeme(num("1"));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }
}

TEST_SUITE("InputStatementStrategy") {
  TEST_CASE("Parses INPUT prompt") {
    unique_ptr<ParserContext> ctx = createContext();
    InputStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(lit("\"A?\""));
    line.addLexeme(sep(";"));
    line.addLexeme(id("A"));

    line.setLexemeBOF();

    bool result = strategy.parseStatement(*ctx, &line);

    CHECK(result == true);
  }
}

TEST_SUITE("TimeStatementStrategy") {
  TEST_CASE("Parses TIME assignment and sets input flag") {
    unique_ptr<ParserContext> ctx = createContext();
    TimeStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(id("T"));
    line.addLexeme(op("="));
    line.addLexeme(num("1"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("TIME"));

    CHECK(result == true);
    CHECK(ctx->has_input == true);
  }
}

TEST_SUITE("NoopStatementStrategy") {
  TEST_CASE("Converts apostrophe to REM") {
    unique_ptr<ParserContext> ctx = createContext();
    NoopStatementStrategy strategy;

    LexerLineContext line;
    line.setLexemeBOF();

    shared_ptr<Lexeme> lex =
        make_shared<Lexeme>(Lexeme::type_operator, Lexeme::subtype_any, "'");

    bool result = strategy.execute(*ctx, &line, lex);

    CHECK(result == true);
    CHECK(lex->type == Lexeme::type_keyword);
    CHECK(lex->value == "REM");
  }
}

TEST_SUITE("NextStatementStrategy") {
  TEST_CASE("Parses NEXT with multiple variables") {
    unique_ptr<ParserContext> ctx = createContext();
    NextStatementStrategy strategy;

    setActionRoot(*ctx, "NEXT");

    LexerLineContext line;
    line.addLexeme(id("I"));
    line.addLexeme(sep(","));
    line.addLexeme(id("J"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("NEXT"));

    CHECK(result == true);
  }
}

TEST_SUITE("ForStatementStrategy") {
  TEST_CASE("Parses FOR with TO") {
    unique_ptr<ParserContext> ctx = createContext();
    ForStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(id("I"));
    line.addLexeme(op("="));
    line.addLexeme(num("1"));
    line.addLexeme(kw("TO"));
    line.addLexeme(num("10"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("FOR"));

    CHECK(result == true);
  }
}

TEST_SUITE("IfStatementStrategy") {
  TEST_CASE("Parses IF with THEN statement") {
    unique_ptr<ParserContext> ctx = createContext();
    IfStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(id("A"));
    line.addLexeme(op("="));
    line.addLexeme(num("1"));
    line.addLexeme(kw("THEN"));
    line.addLexeme(kw("PRINT"));
    line.addLexeme(lit("\"A\""));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("IF"));

    CHECK(result == true);
  }
}

TEST_SUITE("DefStatementStrategy") {
  TEST_CASE("Parses DEFINT and updates default type") {
    unique_ptr<ParserContext> ctx = createContext();
    DefStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(id("A"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("DEFINT"));

    CHECK(result == true);
    CHECK(ctx->deftbl[0] == 2);
  }

  TEST_CASE("Parses DEF USR assignment") {
    unique_ptr<ParserContext> ctx = createContext();
    DefStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(kw("USR"));
    line.addLexeme(op("="));
    line.addLexeme(num("1"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("DEF"));

    CHECK(result == true);
    CHECK(ctx->has_defusr == true);
  }
}

TEST_SUITE("DimStatementStrategy") {
  TEST_CASE("Parses DIM with size parameter") {
    unique_ptr<ParserContext> ctx = createContext();
    DimStatementStrategy strategy;

    setActionRoot(*ctx, "DIM");

    LexerLineContext line;
    line.addLexeme(id("A"));
    line.addLexeme(sep("("));
    line.addLexeme(num("1"));
    line.addLexeme(sep(")"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("DIM"));

    CHECK(result == true);
  }

  TEST_CASE("Rejects DIM without size parameter") {
    unique_ptr<ParserContext> ctx = createContext();
    DimStatementStrategy strategy;

    setActionRoot(*ctx, "DIM");

    LexerLineContext line;
    line.addLexeme(id("A"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("DIM"));

    CHECK(result == false);
  }
}

TEST_SUITE("DataStatementStrategy") {
  TEST_CASE("Parses DATA items") {
    unique_ptr<ParserContext> ctx = createContext();
    DataStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(num("1"));
    line.addLexeme(sep(","));
    line.addLexeme(num("2"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("DATA"));

    CHECK(result == true);
    CHECK(ctx->has_data == true);
    CHECK(ctx->datas.size() == 2);
  }
}

TEST_SUITE("IDataStatementStrategy") {
  TEST_CASE("Parses IDATA items") {
    unique_ptr<ParserContext> ctx = createContext();
    IDataStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(num("1"));
    line.addLexeme(sep(","));
    line.addLexeme(num("2"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("IDATA"));

    CHECK(result == true);
    CHECK(ctx->has_idata == true);
    CHECK(ctx->datas.size() == 2);
  }
}

TEST_SUITE("FileStatementStrategy") {
  TEST_CASE("Parses OPEN with FOR/AS/LEN") {
    unique_ptr<ParserContext> ctx = createContext();
    FileStatementStrategy strategy;

    setActionRoot(*ctx, "OPEN");

    LexerLineContext line;
    line.addLexeme(lit("\"A\""));
    line.addLexeme(kw("FOR"));
    line.addLexeme(kw("INPUT"));
    line.addLexeme(kw("AS"));
    line.addLexeme(sep("#"));
    line.addLexeme(num("1"));
    line.addLexeme(kw("LEN"));
    line.addLexeme(num("1"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("OPEN"));

    CHECK(result == true);
  }

  TEST_CASE("Parses CLOSE with channel") {
    unique_ptr<ParserContext> ctx = createContext();
    FileStatementStrategy strategy;

    setActionRoot(*ctx, "CLOSE");

    LexerLineContext line;
    line.addLexeme(sep("#"));
    line.addLexeme(num("1"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("CLOSE"));

    CHECK(result == true);
  }

  TEST_CASE("Parses MAXFILES assignment") {
    unique_ptr<ParserContext> ctx = createContext();
    FileStatementStrategy strategy;

    setActionRoot(*ctx, "MAXFILES");

    LexerLineContext line;
    line.addLexeme(kw("MAXFILES"));
    line.addLexeme(op("="));
    line.addLexeme(num("5"));

    line.getFirstLexeme();

    bool result = strategy.execute(*ctx, &line, kw("MAXFILES"));

    CHECK(result == true);
  }
}

TEST_SUITE("CmdStatementStrategy") {
  TEST_CASE("Sets flags for CMD WRTFNT") {
    unique_ptr<ParserContext> ctx = createContext();
    CmdStatementStrategy strategy;

    setActionRoot(*ctx, "CMD");

    LexerLineContext line;
    line.addLexeme(kw("WRTFNT"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("CMD"));

    CHECK(result == true);
    CHECK(ctx->has_font == true);
  }
}

TEST_SUITE("CallStatementStrategy") {
  TEST_CASE("Alias '_' converts to CALL") {
    unique_ptr<ParserContext> ctx = createContext();
    CallStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(id("USR"));

    line.setLexemeBOF();

    shared_ptr<Lexeme> lex =
        make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, "_");

    bool result = strategy.execute(*ctx, &line, lex);

    CHECK(result == true);
    CHECK(lex->value == "CALL");
  }
}

TEST_SUITE("ColorStatementStrategy") {
  TEST_CASE("Parses COLOR=RGB statement") {
    unique_ptr<ParserContext> ctx = createContext();
    ColorStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(op("="));
    line.addLexeme(sep("("));
    line.addLexeme(num("1"));
    line.addLexeme(sep(","));
    line.addLexeme(num("2"));
    line.addLexeme(sep(","));
    line.addLexeme(num("3"));
    line.addLexeme(sep(")"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("COLOR"));

    CHECK(result == true);
  }
}

TEST_SUITE("GetStatementStrategy") {
  TEST_CASE("Parses GET DATE with expression") {
    unique_ptr<ParserContext> ctx = createContext();
    GetStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(kw("DATE"));
    line.addLexeme(num("1"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("GET"));

    CHECK(result == true);
  }
}

TEST_SUITE("SetStatementStrategy") {
  TEST_CASE("Parses SET SCREEN with value") {
    unique_ptr<ParserContext> ctx = createContext();
    SetStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(kw("SCREEN"));
    line.addLexeme(num("0"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("SET"));

    CHECK(result == true);
  }
}

TEST_SUITE("ScreenStatementStrategy") {
  TEST_CASE("Parses SCREEN COPY") {
    unique_ptr<ParserContext> ctx = createContext();
    ScreenStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(kw("COPY"));
    line.addLexeme(kw("TO"));
    line.addLexeme(num("1"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("SCREEN"));

    CHECK(result == true);
  }
}

TEST_SUITE("SpriteStatementStrategy") {
  TEST_CASE("Parses SPRITE ON") {
    unique_ptr<ParserContext> ctx = createContext();
    SpriteStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(kw("ON"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("SPRITE"));

    CHECK(result == true);
  }
}

TEST_SUITE("PutStatementStrategy") {
  TEST_CASE("Parses PUT SPRITE with minimal args") {
    unique_ptr<ParserContext> ctx = createContext();
    PutStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(kw("SPRITE"));
    line.addLexeme(num("1"));
    line.addLexeme(sep(","));
    line.addLexeme(sep(","));
    line.addLexeme(num("2"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("PUT"));

    CHECK(result == true);
  }
}

TEST_SUITE("GraphicsStatementStrategy") {
  TEST_CASE("Parses PSET coordinates") {
    unique_ptr<ParserContext> ctx = createContext();
    GraphicsStatementStrategy strategy;

    setActionRoot(*ctx, "PSET");

    LexerLineContext line;
    line.addLexeme(sep("("));
    line.addLexeme(num("1"));
    line.addLexeme(sep(","));
    line.addLexeme(num("2"));
    line.addLexeme(sep(")"));

    line.setLexemeBOF();

    bool result = strategy.execute(*ctx, &line, kw("PSET"));

    CHECK(result == true);
  }
}

TEST_SUITE("OnStatementStrategy") {
  TEST_CASE("Parses ON ... GOTO list") {
    unique_ptr<ParserContext> ctx = createContext();
    OnStatementStrategy strategy;

    LexerLineContext line;
    line.addLexeme(kw("ON"));
    line.addLexeme(num("1"));
    line.addLexeme(kw("GOTO"));
    line.addLexeme(num("10"));

    line.getFirstLexeme();

    bool result = strategy.execute(*ctx, &line, kw("ON"));

    CHECK(result == true);
    CHECK(ctx->has_traps == false);
  }
}

// NOLINTEND
