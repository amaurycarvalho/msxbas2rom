/***
 * @file test_compiler_functions.cpp
 * @brief MSXBAS2ROM compiler string/math function strategies unit testing
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
#include "vscode_helper.h"
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

TEST_SUITE("CompilerStringFunctions") {
  TEST_CASE("MID$ function variants compile") {
    SUBCASE("MID$ with two arguments") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_mid2.bas", "10 A$=MID$(\"HELLO\",2)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("MID$ with three arguments") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_mid3.bas", "10 A$=MID$(\"HELLO\",2,3)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("MID$ with float start") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_mid_float.bas", "10 A$=MID$(\"HELLO\",2.5,3)\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("INSTR function variants compile") {
    SUBCASE("INSTR with two arguments") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_instr2.bas", "10 A=INSTR(\"HELLO\",\"LL\")\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("INSTR with three arguments") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_instr3.bas", "10 A=INSTR(1,\"HELLO\",\"LL\")\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("STRING$ function variants compile") {
    SUBCASE("STRING$ with integer character code") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_string_int.bas", "10 A$=STRING$(5,65)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("STRING$ with string argument") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_string_str.bas", "10 A$=STRING$(5,\"A\")\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("USR function variants compile") {
    SUBCASE("USR0") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_usr0.bas", "10 DEF USR0=1\n20 A=USR0(5)\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("USR1") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_usr1.bas", "10 DEF USR1=1\n20 A=USR1(5)\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("USR9") {
      std::string errors;
      bool ok = compileStatementProgram(
          "fn_usr9.bas", "10 DEF USR9=1\n20 A=USR9(5)\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("USING$ with complex formats compiles") {
    std::string errors;
    bool ok = compileStatementProgram(
        "fn_using_complex.bas",
        "10 PRINT USING$(\"+#,###.##^\", 12345.6)\n20 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }
}

TEST_SUITE("CompilerSmokeTests") {
  TEST_CASE("TIME statement strategy smoke test") {
    SUBCASE("TIME read assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "smoke_time_read.bas", "10 A=TIME\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("TIME write assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "smoke_time_write.bas", "10 TIME=1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("OPEN_GRP statement strategy smoke test") {
    SUBCASE("OPEN GRP for output") {
      std::string errors;
      bool ok = compileStatementProgram(
          "smoke_open_grp.bas",
          "10 OPEN \"GRP:\" FOR OUTPUT AS #1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("VSCodeHelper smoke test") {
    SUBCASE("Constructs and exposes app filenames") {
      VSCodeHelper helper("msxbas2rom");
      CHECK(helper.getCompilerAppFilename() == "msxbas2rom");
      CHECK(helper.getEmulatorAppFilename().empty() == false);
    }

    SUBCASE("Initializes .vscode files") {
      VSCodeHelper helper("msxbas2rom");
      CHECK(helper.initialize() == true);

      std::ifstream launch("tmp/../.vscode/launch.json");
      CHECK(launch.good());

      std::remove(".vscode/launch.json");
      std::remove(".vscode/tasks.json");
      std::remove(".vscode/debug.tcl");
      std::remove(".vscode");
    }
  }
}

// NOLINTEND
