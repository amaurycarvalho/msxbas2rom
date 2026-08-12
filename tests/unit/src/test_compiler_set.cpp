/***
 * @file test_compiler_set.cpp
 * @brief MSXBAS2ROM compiler SET statement unit testing
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

TEST_SUITE("CompilerSetStatementStrategy") {
  TEST_CASE("SET sub-commands compile successfully") {
    struct SetCase {
      const char* name;
      const char* program;
    };

    const SetCase cases[] = {
        {"SET_ADJUST", "10 SET ADJUST (1,2)\n20 END\n"},
        {"SET_BEEP", "10 SET BEEP 1,2\n20 END\n"},
        {"SET_DATE", "10 SET DATE 2025,1,1\n20 END\n"},
        {"SET_FONT", "10 SET FONT 1\n20 END\n"},
        {"SET_FONT_2", "10 SET FONT 1,2\n20 END\n"},
        {"SET_PAGE", "10 SET PAGE 1\n20 END\n"},
        {"SET_PAGE_2", "10 SET PAGE 1,2\n20 END\n"},
        {"SET_PROMPT", "10 SET PROMPT \"X\"\n20 END\n"},
        {"SET_SCREEN", "10 SET SCREEN\n20 END\n"},
        {"SET_SCROLL", "10 SET SCROLL 1,2,3\n20 END\n"},
        {"SET_TIME", "10 SET TIME 1,2,3\n20 END\n"},
        {"SET_TITLE", "10 SET TITLE \"GAME\",1\n20 END\n"},
        {"SET_VIDEO", "10 SET VIDEO 0,1\n20 END\n"},
        {"SET_VIDEO_7",
         "10 SET VIDEO 0,1,2,3,4,5,6\n20 END\n"},
        {"SET_TILE_ON", "10 SET TILE ON\n20 END\n"},
        {"SET_TILE_OFF", "10 SET TILE OFF\n20 END\n"},
        {"SET_TILE_FLIP", "10 SET TILE FLIP 0,1\n20 END\n"},
        {"SET_TILE_FLIP_BANK", "10 SET TILE FLIP 0,1,2\n20 END\n"},
        {"SET_TILE_ROTATE", "10 SET TILE ROTATE 0,1\n20 END\n"},
        {"SET_TILE_ROTATE_BANK", "10 SET TILE ROTATE 0,1,1\n20 END\n"},
        {"SET_SPRITE_FLIP", "10 SET SPRITE FLIP 0,1\n20 END\n"},
        {"SET_SPRITE_ROTATE", "10 SET SPRITE ROTATE 0,1\n20 END\n"},
    };

    for (const auto& test_case : cases) {
      SUBCASE(test_case.name) {
        std::string errors;
        bool ok = compileStatementProgram(
            std::string("set_") + test_case.name + ".bas", test_case.program,
            &errors);
        CHECK(ok == true);
        CHECK(errors.empty());
      }
    }
  }

  TEST_CASE("SET TILE COLOR variants compile successfully") {
    SUBCASE("FC only") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_color_fc.bas", "10 SET TILE COLOR 5, 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("FC and BC") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_color_fc_bc.bas", "10 SET TILE COLOR 5, 1, 15\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("FC array") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_color_fc_array.bas",
          "10 SET TILE COLOR 5, (1,2,3,4)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("FC and BC arrays") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_color_arrays.bas",
          "10 SET TILE COLOR 5, (1,2,3,4), (15,14,13,12)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("FC array with bank") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_color_fc_array_bank.bas",
          "10 SET TILE COLOR 5, (1,2,3,4), 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("FC and BC arrays with bank") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_color_arrays_bank.bas",
          "10 SET TILE COLOR 5, (1,2,3,4), (5,6,7,8), 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("FC scalar with BC array is rejected") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_color_scalar_bcarray.bas",
          "10 SET TILE COLOR 5, 1, (2,3,4)\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("Syntax not supported") != std::string::npos);
    }
  }

  TEST_CASE("SET SPRITE PATTERN and COLOR with arrays compile") {
    SUBCASE("SPRITE PATTERN with buffer") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_sprite_pattern.bas",
          "10 DIM PB%(3)\n20 SET SPRITE PATTERN 0, PB%\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SPRITE COLOR with buffer") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_sprite_color.bas",
          "10 DIM CB%(7)\n20 SET SPRITE COLOR 0, CB%\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("SET TILE PATTERN variants compile") {
    SUBCASE("Inline tuple") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_pattern_inline.bas",
          "10 SET TILE PATTERN 65, (255,129,129,129,129,129,129,255)\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Inline tuple with bank") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_pattern_bank.bas",
          "10 SET TILE PATTERN 65, (255,0,255,0,255,0,255,0), 1\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Array with parens") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_pattern_array.bas",
          "10 DIM PB%(3)\n20 SET TILE PATTERN 65, PB%()\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Array with bank") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_tile_pattern_array_bank.bas",
          "10 DIM PB%(3)\n20 SET TILE PATTERN 65, PB%(), 2\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("SET error paths are detected") {
    SUBCASE("Wrong SET SCREEN parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_screen.bas", "10 SET SCREEN,1\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET SCREEN") != std::string::npos);
    }

    SUBCASE("Wrong SET BEEP parameter count") {
      std::string errors;
      bool ok =
          compileStatementProgram("set_err_beep.bas", "10 SET BEEP\n20 END\n",
                                  &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET BEEP") != std::string::npos);
    }

    SUBCASE("Wrong SET ADJUST parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_adjust.bas", "10 SET ADJUST (1)\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET ADJUST") != std::string::npos);
    }

    SUBCASE("Wrong SET TITLE parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_title.bas", "10 SET TITLE \"A\"\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TITLE") != std::string::npos);
    }

    SUBCASE("Wrong SET VIDEO parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_video.bas", "10 SET VIDEO\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET VIDEO") != std::string::npos);
    }

    SUBCASE("Wrong SET PAGE parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_page.bas", "10 SET PAGE 1,2,3\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET PAGE") != std::string::npos);
    }

    SUBCASE("Wrong SET PROMPT parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_prompt.bas", "10 SET PROMPT\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET PROMPT") != std::string::npos);
    }

    SUBCASE("Wrong SET TIME parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_time.bas", "10 SET TIME 1,2\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TIME") != std::string::npos);
    }

    SUBCASE("Wrong SET SCROLL parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_scroll.bas", "10 SET SCROLL 1,2,3,4,5\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET SCROLL") != std::string::npos);
    }

    SUBCASE("Wrong SET FONT parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_font.bas", "10 SET FONT 1,2,3\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET FONT") != std::string::npos);
    }

    SUBCASE("Wrong SET SPRITE FLIP parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_sprite_flip.bas", "10 SET SPRITE FLIP 1\n20 END\n",
          &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET SPRITE FLIP") != std::string::npos);
    }

    SUBCASE("Wrong SET TILE FLIP parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_tile_flip.bas", "10 SET TILE FLIP 1\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TILE FLIP") != std::string::npos);
    }

    SUBCASE("Wrong SET TILE ROTATE parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_tile_rotate.bas", "10 SET TILE ROTATE 1\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TILE ROTATE") != std::string::npos);
    }

    SUBCASE("Wrong SET TILE COLOR parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_tile_color.bas", "10 SET TILE COLOR\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TILE COLOR") != std::string::npos);
    }

    SUBCASE("Wrong SET TILE PATTERN parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_tile_pattern.bas", "10 SET TILE PATTERN\n20 END\n",
          &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TILE PATTERN") != std::string::npos);
    }

    SUBCASE("Wrong SET TILE FLIP parameter count") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_tile_flip2.bas", "10 SET TILE FLIP\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TILE FLIP") != std::string::npos);
    }

    SUBCASE("SET TILE PATTERN with plain number is rejected") {
      std::string errors;
      bool ok = compileStatementProgram(
          "set_err_tile_pattern_num.bas",
          "10 SET TILE PATTERN 65, 1\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("SET TILE PATTERN") != std::string::npos);
    }
  }
}

// NOLINTEND
