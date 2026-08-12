/***
 * @file test_compiler_control.cpp
 * @brief MSXBAS2ROM compiler control statement strategies unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "build_options.h"
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

static bool compileWithOpts(const std::string& filename,
                            shared_ptr<Compiler> compiler,
                            shared_ptr<BuildOptions> opts) {
  shared_ptr<Lexer> lexer = make_shared<Lexer>();
  shared_ptr<Parser> parser = make_shared<Parser>();

  opts->setInputFilename(filename);

  if (!lexer->load(opts)) return false;
  if (!lexer->evaluate()) return false;
  if (!parser->evaluate(lexer)) return false;
  return compiler->build(parser);
}

TEST_SUITE("CompilerControlStrategies") {
  TEST_CASE("RETURN statement variants compile") {
    SUBCASE("Plain RETURN") {
      std::string errors;
      bool ok = compileStatementProgram(
          "ret_plain.bas", "10 GOSUB 20\n20 RETURN\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("RETURN to line number") {
      std::string errors;
      bool ok = compileStatementProgram(
          "ret_line.bas", "10 GOSUB 20\n20 RETURN 10\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("RETURN with leading zero line number") {
      std::string errors;
      bool ok = compileStatementProgram(
          "ret_zero.bas", "10 GOSUB 20\n20 RETURN 010\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("RETURN to line in MegaROM mode") {
      const std::string path =
          createTempBas("ret_line_mega.bas", "10 GOSUB 20\n20 RETURN 10\n30 END\n");

      shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
      opts->compileMode = BuildOptions::CompileMode::Konami4;
      opts->megaROM = true;

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      CHECK(compileWithOpts(path, compiler, opts) == true);
      std::remove(path.c_str());
    }

    SUBCASE("Invalid RETURN parameters are rejected") {
      std::string errors;
      bool ok = compileStatementProgram(
          "ret_invalid.bas", "10 GOSUB 20\n20 RETURN CLEAR\n30 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("Invalid RETURN parameters") != std::string::npos);
    }
  }

  TEST_CASE("STOP statement variants compile") {
    SUBCASE("Plain STOP") {
      std::string errors;
      bool ok =
          compileStatementProgram("stop_plain.bas", "10 STOP\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("STOP ON") {
      std::string errors;
      bool ok = compileStatementProgram("stop_on.bas", "10 STOP ON\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("STOP OFF") {
      std::string errors;
      bool ok = compileStatementProgram("stop_off.bas", "10 STOP OFF\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("STOP STOP") {
      std::string errors;
      bool ok = compileStatementProgram("stop_stop.bas", "10 STOP STOP\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }
}

TEST_SUITE("CompilerMoreGraphics") {
  TEST_CASE("GET statement variants compile") {
    SUBCASE("GET TIME") {
      std::string errors;
      bool ok = compileStatementProgram(
          "get_time.bas", "10 DEFINT A-Z\n20 GET TIME H,M,S\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("GET DATE with three variables") {
      std::string errors;
      bool ok = compileStatementProgram(
          "get_date.bas", "10 DEFINT A-Z\n20 GET DATE Y,M,D\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("GET DATE with five variables") {
      std::string errors;
      bool ok = compileStatementProgram(
          "get_date5.bas",
          "10 DEFINT A-Z\n20 GET DATE Y,M,D,W,F\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("LINE statement variants compile") {
    SUBCASE("LINE with two points") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_basic.bas", "10 LINE (0,0)-(1,1)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("LINE with STEP and attribute") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_step.bas", "10 LINE STEP(1,1)-(2,2),15\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("LINE with box option") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_box.bas", "10 LINE (0,0)-(1,1),15,B\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("COLOR statement variants compile") {
    SUBCASE("COLOR foreground") {
      std::string errors;
      bool ok = compileStatementProgram("color_fg.bas", "10 COLOR 15\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("COLOR NEW") {
      std::string errors;
      bool ok = compileStatementProgram("color_new.bas", "10 COLOR NEW\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("COLOR RESTORE") {
      std::string errors;
      bool ok = compileStatementProgram(
          "color_restore.bas", "10 COLOR RESTORE\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("COLOR RGB with two colors") {
      std::string errors;
      bool ok = compileStatementProgram(
          "color_rgb2.bas", "10 COLOR RGB 1,2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("COLOR RGB with one color") {
      std::string errors;
      bool ok = compileStatementProgram(
          "color_rgb1.bas", "10 COLOR RGB 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("COLOR SPRITE assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "color_sprite.bas", "10 COLOR SPRITE(1)=33\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }
}

TEST_SUITE("CompilerIfStatement") {
  TEST_CASE("IF statement variants compile") {
    SUBCASE("IF THEN ELSE") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_else.bas", "10 IF A=1 THEN PRINT 1 ELSE PRINT 2\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF GOTO") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_goto.bas", "10 IF A=1 GOTO 20\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF GOSUB") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_gosub.bas",
          "10 IF A=1 GOSUB 30\n20 PRINT 1\n30 RETURN\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF with AND condition") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_and.bas", "10 IF A=1 AND B=2 THEN PRINT 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF with OR condition") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_or.bas", "10 IF A=1 OR B=2 THEN PRINT 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF with string condition") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_str.bas", "10 IF A$=\"X\" THEN PRINT 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF THEN line number") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_then_line.bas", "10 IF A=1 THEN 20\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF GOTO with line number") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_goto_num.bas", "10 IF A=1 GOTO 10\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("IF GOSUB with line number") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_gosub_num.bas",
          "10 IF A=1 GOSUB 20\n20 PRINT 1\n30 RETURN\n40 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Nested IF") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_nested.bas", "10 IF A=1 THEN IF B=2 THEN PRINT 1\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Nested IF with ELSE") {
      std::string errors;
      bool ok = compileStatementProgram(
          "if_nested_else.bas",
          "10 IF A=1 THEN IF B=2 THEN PRINT 1 ELSE PRINT 2\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }
}

TEST_SUITE("CompilerWaitStatement") {
  TEST_CASE("WAIT with two parameters") {
    std::string errors;
    bool ok = compileStatementProgram("wait_2.bas", "10 WAIT 1,2\n20 END\n",
                                      &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("WAIT with three parameters") {
    std::string errors;
    bool ok = compileStatementProgram("wait_3.bas", "10 WAIT 1,2,3\n20 END\n",
                                      &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("WAIT with invalid parameters is rejected") {
    std::string errors;
    bool ok = compileStatementProgram("wait_invalid.bas", "10 WAIT 1\n20 END\n",
                                      &errors);
    CHECK(ok == false);
    CHECK(errors.find("Invalid WAIT parameters") != std::string::npos);
  }
}

TEST_SUITE("CompilerFileStatements") {
  TEST_CASE("OPEN variants compile") {
    SUBCASE("OPEN FOR APPEND") {
      std::string errors;
      bool ok = compileStatementProgram(
          "open_append.bas", "10 OPEN \"A.TXT\" FOR APPEND AS #1\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("OPEN FOR INPUT with LEN") {
      std::string errors;
      bool ok = compileStatementProgram(
          "open_len.bas", "10 OPEN \"A.TXT\" FOR INPUT AS #1 LEN 128\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("CLOSE without number") {
      std::string errors;
      bool ok = compileStatementProgram("close_all.bas", "10 CLOSE\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("OPEN with compact AS# form") {
      std::string errors;
      bool ok = compileStatementProgram(
          "open_compact_hash.bas", "10 OPEN \"A.TXT\"AS#1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("OPEN with compact AS digit form") {
      std::string errors;
      bool ok = compileStatementProgram(
          "open_compact_digit.bas", "10 OPEN \"A.TXT\"AS1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("OPEN FOR INPUT with compact AS#") {
      std::string errors;
      bool ok = compileStatementProgram(
          "open_input_hash.bas",
          "10 OPEN \"A.TXT\" FOR INPUT AS#1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("OPEN FOR INPUT with compact AS digit") {
      std::string errors;
      bool ok = compileStatementProgram(
          "open_input_digit.bas",
          "10 OPEN \"A.TXT\" FOR INPUT AS1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("OPEN FOR INPUT with compact AS# and LEN") {
      std::string errors;
      bool ok = compileStatementProgram(
          "open_input_hash_len.bas",
          "10 OPEN \"A.TXT\" FOR INPUT AS#1 LEN 128\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("File I/O statements compile") {
    SUBCASE("PRINT# to file") {
      std::string errors;
      bool ok = compileStatementProgram(
          "file_print.bas",
          "10 OPEN \"A.TXT\" FOR OUTPUT AS #1\n20 PRINT #1,\"X\"\n"
          "30 CLOSE #1\n40 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("INPUT# from file") {
      std::string errors;
      bool ok = compileStatementProgram(
          "file_input.bas",
          "10 OPEN \"A.TXT\" FOR INPUT AS #1\n20 INPUT #1,A$\n"
          "30 CLOSE #1\n40 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("LINE INPUT# from file") {
      std::string errors;
      bool ok = compileStatementProgram(
          "file_lineinput.bas",
          "10 OPEN \"A.TXT\" FOR INPUT AS #1\n20 LINE INPUT #1,A$\n"
          "30 CLOSE #1\n40 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }
}

// NOLINTEND
