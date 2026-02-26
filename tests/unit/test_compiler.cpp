/***
 * @file test_compiler.cpp
 * @brief MSXBAS2ROM compiler unit testing
 */

// NOLINTBEGIN

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdio>
#include <fstream>
#include <string>

#include "compiler.h"
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

static bool compileProgram(const std::string& filename, Compiler& compiler) {
  Lexer lexer;
  Parser parser;

  if (!lexer.load(filename)) return false;
  if (!lexer.evaluate()) return false;
  if (!parser.evaluate(&lexer)) return false;
  return compiler.build(&parser);
}

TEST_SUITE("Compiler") {
  TEST_CASE("Compiles valid parser output") {
    const std::string filename =
        createTempBas("compiler_valid.bas", "10 PRINT \"HI\"\n20 END\n");

    Compiler compiler;
    CHECK(compileProgram(filename, compiler) == true);
    CHECK(compiler.compiled == true);
    CHECK(compiler.code_size > 0);
    CHECK(compiler.ram_size >= 0);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails on duplicated line numbers") {
    const std::string filename = createTempBas(
        "compiler_duplicated_line.bas", "10 PRINT \"A\"\n10 PRINT \"B\"\n");

    Compiler compiler;
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler.error_message.find("Line number already declared") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when FOR has no NEXT") {
    const std::string filename = createTempBas(
        "compiler_for_without_next.bas", "10 FOR I=1 TO 10\n20 PRINT I\n");

    Compiler compiler;
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler.error_message.find("FOR without a NEXT") !=
          std::string::npos);

    std::remove(filename.c_str());
  }
}

// NOLINTEND
