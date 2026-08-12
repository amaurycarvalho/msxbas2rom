/***
 * @file test_compiler_dim.cpp
 * @brief MSXBAS2ROM compiler DIM and DEF statement unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "compiler.h"
#include "compiler_context.h"
#include "compiler_statement_strategy_factory.h"
#include "cpu_workspace_context.h"
#include "doctest/doctest.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "z80.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static bool compileStatementProgram(const std::string& filename,
                                    const std::string& program,
                                    std::string* error_out = nullptr) {
  const std::string path = createTempBas(filename, program);

  shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter = make_shared<Z80OpcodeWriter>();
  shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
  shared_ptr<Lexer> lexer = make_shared<Lexer>();
  shared_ptr<Parser> parser = make_shared<Parser>();

  bool ok = false;
  if (lexer->load(path) && lexer->evaluate() && parser->evaluate(lexer)) {
    ok = compiler->build(parser);
  }

  if (!ok && error_out) {
    *error_out = compiler->getLogger()->errors().toString();
  }

  std::remove(path.c_str());

  return ok;
}

TEST_SUITE("CompilerDimStatement") {
  TEST_CASE("DIM statement variants compile") {
    SUBCASE("DIM one dimension") {
      std::string errors;
      bool ok = compileStatementProgram("dim_1d.bas", "10 DIM A(5)\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DIM two dimensions") {
      std::string errors;
      bool ok = compileStatementProgram("dim_2d.bas", "10 DIM A(3,3)\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DIM float array") {
      std::string errors;
      bool ok = compileStatementProgram("dim_float.bas",
                                        "10 DIM A#(5)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DIM double array") {
      std::string errors;
      bool ok = compileStatementProgram("dim_dbl.bas", "10 DIM A!(5)\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DIM integer array") {
      std::string errors;
      bool ok = compileStatementProgram("dim_int.bas", "10 DIM A%(5)\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("REDIM resizes array") {
      std::string errors;
      bool ok = compileStatementProgram(
          "redim.bas", "10 DIM A(5)\n20 REDIM A(10)\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DIM with three dimensions is rejected") {
      std::string errors;
      bool ok = compileStatementProgram(
          "dim_3d.bas", "10 DIM A(2,2,2)\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("more than 2 dimensions") != std::string::npos);
    }

    SUBCASE("DIM with non-constant index is rejected") {
      std::string errors;
      bool ok = compileStatementProgram(
          "dim_expr.bas", "10 N=5\n20 DIM A(N)\n30 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("must be a integer constant") != std::string::npos);
    }
  }
}

TEST_SUITE("CompilerDefStatement") {
  TEST_CASE("DEF statements compile") {
    SUBCASE("DEF USR") {
      std::string errors;
      bool ok = compileStatementProgram("def_usr.bas", "10 DEF USR=1\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DEF USR0") {
      std::string errors;
      bool ok = compileStatementProgram(
          "def_usr0.bas", "10 DEF USR0=1\n20 A=USR0(5)\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DEFINT range") {
      std::string errors;
      bool ok = compileStatementProgram(
          "defint.bas", "10 DEFINT A-Z\n20 A=1\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DEFSNG range") {
      std::string errors;
      bool ok = compileStatementProgram(
          "defsng.bas", "10 DEFSNG A-Z\n20 A=1.5\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DEFDBL range") {
      std::string errors;
      bool ok = compileStatementProgram(
          "defdbl.bas", "10 DEFDBL A-Z\n20 A=1\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("DEFSTR range") {
      std::string errors;
      bool ok = compileStatementProgram(
          "defstr.bas", "10 DEFSTR A-Z\n20 A$=\"X\"\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }
}

// NOLINTEND
