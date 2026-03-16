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

    Lexeme* cloned = num.clone();
    REQUIRE(cloned != nullptr);
    CHECK(cloned->type == num.type);
    CHECK(cloned->subtype == num.subtype);
    CHECK(cloned->name == num.name);
    CHECK(cloned->value == num.value);
    delete cloned;
  }

  TEST_CASE("Lexeme toString and type names") {
    Lexeme lex(Lexeme::type_identifier, Lexeme::subtype_string, "A$", "A$");
    std::string text = lex.toString();
    CHECK(text.find("Identifier") != std::string::npos);
    CHECK(text.find("String") != std::string::npos);
    CHECK(lex.getTypeName() != nullptr);
    CHECK(lex.getSubTypeName() != nullptr);
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
    ActionNode root("PRINT");
    ActionNode* child = new ActionNode("A");
    root.actions.push_back(child);

    std::string actionText = root.toString();
    CHECK(actionText.find("Action") != std::string::npos);

    TagNode tag;
    tag.name = "10";
    tag.value = "10";
    tag.actions.push_back(&root);

    std::string tagText = tag.toString();

    CHECK(tagText.find("Tag 10") != std::string::npos);
  }
}

// NOLINTEND
