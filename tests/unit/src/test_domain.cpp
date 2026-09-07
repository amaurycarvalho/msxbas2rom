/***
 * @file test_domain.cpp
 * @brief MSXBAS2ROM domain unit testing
 */

// NOLINTBEGIN

#include <string>

#include "action_node.h"
#include "build_options.h"
#include "doctest/doctest.h"
#include "lexeme.h"
#include "lexer_line_context.h"
#include "tag_node.h"

TEST_SUITE("Domain") {
  TEST_CASE("Lexeme keyword/function/boolean detection") {
    Lexeme kw(Lexeme::type_keyword, Lexeme::subtype_any, "PRINT");
    CHECK(kw.isKeyword() == true);

    Lexeme fn(Lexeme::type_keyword, Lexeme::subtype_function, "TIME");
    CHECK(fn.isFunction() == true);

    Lexeme op(Lexeme::type_keyword, Lexeme::subtype_any, "AND");
    CHECK(op.isBooleanOperator() == true);

    Lexeme other(Lexeme::type_keyword, Lexeme::subtype_any, "XYZ");
    CHECK(other.isKeyword() == false);
    CHECK(other.isFunction() == false);
    CHECK(other.isBooleanOperator() == false);
  }

  TEST_CASE("Lexeme literal numeric and clone") {
    Lexeme num(Lexeme::type_literal, Lexeme::subtype_numeric, "10", "10");
    CHECK(num.isLiteralNumeric() == true);

    Lexeme* cloned = num.clone().get();

    REQUIRE(cloned != nullptr);
    CHECK(cloned->type == num.type);
    CHECK(cloned->subtype == num.subtype);
    CHECK(cloned->name == num.name);
    CHECK(cloned->value == num.value);
  }

  TEST_CASE("Lexeme toString and type names") {
    Lexeme lex(Lexeme::type_identifier, Lexeme::subtype_string, "A$", "A$");
    std::string text = lex.toString();
    CHECK(text.find("Identifier") != std::string::npos);
    CHECK(text.find("String") != std::string::npos);
    CHECK(!lex.getTypeName().empty());
    CHECK(!lex.getSubTypeName().empty());
  }

  TEST_CASE("BuildOptions filename and compile mode naming") {
    BuildOptions opts;
    opts.setInputFilename("tmp/test.bas");
    CHECK(opts.outputFilename.find(".rom") != std::string::npos);
    CHECK(opts.baseFilename.find(".bas") == std::string::npos);

    opts.compileMode = BuildOptions::CompileMode::ASCII8;
    opts.setInputFilename("tmp/test2.bas");
    CHECK(opts.outputFilename.find("[ASCII8]") != std::string::npos);
    CHECK(opts.megaROM == true);

    opts.compileMode = BuildOptions::CompileMode::KonamiSCC;
    opts.setInputFilename("tmp/test3.bas");
    CHECK(opts.outputFilename.find("[KonamiSCC]") != std::string::npos);

    CHECK(opts.getCompileModeShortName().size() > 0);
    CHECK(opts.getCompileModeLongName().size() > 0);
  }

  TEST_CASE("ActionNode and TagNode toString") {
    shared_ptr<ActionNode> root = make_shared<ActionNode>("PRINT");
    shared_ptr<ActionNode> child = make_shared<ActionNode>("A");

    root->actions.push_back(child);

    std::string actionText = root->toString();
    CHECK(actionText.find("Action") != std::string::npos);

    TagNode tag;
    tag.name = "10";
    tag.value = "10";
    tag.actions.push_back(root);

    std::string tagText = tag.toString();

    CHECK(tagText.find("Tag 10") != std::string::npos);
  }

  TEST_CASE("LexerLineContext toString appends newline when missing") {
    LexerLineContext line;
    line.lineText = "10 PRINT";
    line.addLexeme(make_shared<Lexeme>(Lexeme::type_literal,
                                       Lexeme::subtype_numeric, "10"));
    std::string text = line.toString();
    CHECK(text.find("10 PRINT\n") != std::string::npos);
    CHECK(text.find("Literal") != std::string::npos);
  }

  TEST_CASE("LexerLineContext toString keeps existing line break") {
    LexerLineContext line;
    line.lineText = "10 PRINT\n";
    line.clearLexemes();
    std::string text = line.toString();
    CHECK(text.find("10 PRINT\n") != std::string::npos);

    LexerLineContext crLine;
    crLine.lineText = "10 PRINT\r";
    crLine.clearLexemes();
    std::string crText = crLine.toString();
    CHECK(crText.find("10 PRINT\r") != std::string::npos);
  }

  TEST_CASE("TagNode toString includes lexer line text") {
    LexerLineContext line;
    line.lineText = "10 PRINT 1";

    TagNode tag;
    tag.name = "10";
    tag.value = "10";
    tag.lexerLine = make_shared<LexerLineContext>(line);

    std::string tagText = tag.toString();
    CHECK(tagText.find("10 PRINT 1") != std::string::npos);
    CHECK(tagText.find("Tag 10") != std::string::npos);
  }

  TEST_CASE("Lexeme toString honours indent override") {
    Lexeme lex(Lexeme::type_literal, Lexeme::subtype_numeric, "10", "10");
    lex.indent = 5;
    std::string defaultText = lex.toString();
    CHECK(defaultText.find("     -->") != std::string::npos);

    std::string overridden = lex.toString(1);
    CHECK(overridden.find(" -->") != std::string::npos);

    std::string zero = lex.toString(0);
    CHECK(zero.find("-->") == 0);
  }

  TEST_CASE("TagNode toString appends newline only when missing") {
    TagNode tag;
    tag.name = "10";
    tag.value = "10";

    LexerLineContext line;
    line.lineText = "10 PRINT";
    tag.lexerLine = make_shared<LexerLineContext>(line);
    std::string t1 = tag.toString();
    CHECK(t1.find("10 PRINT\n") != std::string::npos);

    LexerLineContext line2;
    line2.lineText = "10 PRINT\n";
    tag.lexerLine = make_shared<LexerLineContext>(line2);
    std::string t2 = tag.toString();
    CHECK(t2.find("10 PRINT\n\n") == std::string::npos);
  }

  TEST_CASE("LexerLineContext getLastLexeme returns last lexeme") {
    LexerLineContext line;
    line.addLexeme(make_shared<Lexeme>(Lexeme::type_literal,
                                       Lexeme::subtype_numeric, "1"));
    line.addLexeme(make_shared<Lexeme>(Lexeme::type_literal,
                                       Lexeme::subtype_numeric, "2"));

    shared_ptr<Lexeme> last = line.getLastLexeme();
    REQUIRE(last.get() != nullptr);
    CHECK(last->value == "2");
  }

  TEST_CASE("Lexeme isKeyword/isSeparator/isOperator compare by type") {
    Lexeme kw(Lexeme::type_keyword, Lexeme::subtype_any, "PRINT");
    CHECK(kw.isKeyword("PRINT") == true);
    CHECK(kw.isKeyword("GOTO") == false);

    Lexeme sep(Lexeme::type_separator, Lexeme::subtype_any, ",");
    CHECK(sep.isSeparator(",") == true);
    CHECK(sep.isSeparator(";") == false);

    Lexeme op(Lexeme::type_operator, Lexeme::subtype_any, "+");
    CHECK(op.isOperator("+") == true);
    CHECK(op.isOperator("-") == false);
  }
}

// NOLINTEND
