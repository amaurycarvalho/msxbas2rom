/***
 * @file test_compiler_graphics.cpp
 * @brief MSXBAS2ROM compiler graphics statements unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>

#include "action_node.h"
#include "build_options.h"
#include "compiler.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "compiler_put_statement_strategy.h"
#include "compiler_screen_statement_strategy.h"
#include "compiler_statement_strategy_factory.h"
#include "cpu_workspace_context.h"
#include "doctest/doctest.h"
#include "lexeme.h"
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

static shared_ptr<CompilerContext> createGraphicsContext() {
  shared_ptr<CpuWorkspaceContext> workspace =
      make_shared<CpuWorkspaceContext>(COMPILE_CODE_SIZE, COMPILE_RAM_SIZE,
                                       def_RAM_BOTTOM);
  workspace->clear();
  shared_ptr<Z80OpcodeWriter> cpu = make_shared<Z80OpcodeWriter>();
  cpu->context = workspace;
  shared_ptr<CompilerContext> ctx = make_shared<CompilerContext>();
  ctx->setHelpers(ctx);
  ctx->cpu = cpu;
  ctx->opts = make_shared<BuildOptions>();
  ctx->parser = make_shared<Parser>();
  ctx->compiled = true;
  return ctx;
}

static shared_ptr<Lexeme> gfxNum(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_numeric, v);
}

static shared_ptr<Lexeme> gfxId(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_identifier, Lexeme::subtype_numeric,
                             v);
}

static shared_ptr<Lexeme> gfxKw(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_keyword, Lexeme::subtype_any, v);
}

static shared_ptr<ActionNode> gfxAction(const std::string& keyword) {
  return make_shared<ActionNode>(keyword);
}

TEST_SUITE("CompilerGraphicsStatements") {
  TEST_CASE("COPY statement variants compile") {
    struct CopyCase {
      const char* name;
      const char* program;
    };

    const CopyCase cases[] = {
        {"COPY_BASIC", "10 COPY (0,0)-(1,1) TO (2,2)\n20 END\n"},
        {"COPY_SCREEN_PARAM", "10 COPY SCREEN 1\n20 END\n"},
        {"COPY_ADDRESS",
         "10 DIM A%(3)\n20 COPY A%(0) TO (1,1)\n30 END\n"},
        {"COPY_TO_ADDRESS",
         "10 DIM A%(3)\n20 COPY (0,0)-(1,1) TO A%(0)\n30 END\n"},
        {"COPY_SCREEN_ARRAY",
         "10 DIM A%(3)\n20 COPY SCREEN A%(0)\n30 END\n"},
        {"COPY_ADDRESS_PAGE",
         "10 DIM A%(3)\n20 COPY A%(0) TO (1,1),2\n30 END\n"},
        {"COPY_ADDRESS_PAGE_OP",
         "10 DIM A%(3)\n20 COPY A%(0) TO (1,1),2,3\n30 END\n"},
        {"COPY_OPERATOR",
         "10 COPY (0,0)-(1,1) TO (2,2),5\n20 END\n"},
        {"COPY_ADDRESS_OPERATOR",
         "10 DIM A%(3)\n20 COPY A%(0) TO (1,1),5,6\n30 END\n"},
        {"COPY_TO_ADDR_MIXED",
         "10 DIM A%(3)\n20 COPY (1,1)-STEP(2,2) TO A%(0)\n30 END\n"},
        {"COPY_TO_ADDR_STEP",
         "10 DIM A%(3)\n20 COPY (0,0)-STEP(1,1) TO A%(0)\n30 END\n"},
        {"COPY_ARRAY_TO_ARRAY",
         "10 DIM A%(3),B%(3)\n20 COPY A%(0) TO B%(0)\n30 END\n"},
        {"COPY_MIXED_TO_ARRAY",
         "10 DIM A%(3),B%(3)\n20 COPY (0,0)-STEP(1,1) TO B%(0)\n30 END\n"},
        {"COPY_ARRAY_TO_COORD_OP",
         "10 DIM A%(3)\n20 COPY A%(0) TO (1,1),2,3\n30 END\n"},
    };

    for (const auto& test_case : cases) {
      SUBCASE(test_case.name) {
        std::string errors;
        bool ok = compileStatementProgram(
            std::string("copy_") + test_case.name + ".bas", test_case.program,
            &errors);
        CHECK(ok == true);
        CHECK(errors.empty());
      }
    }
  }

  TEST_CASE("COPY rejects unsupported parameters") {
    std::string errors;
    bool ok = compileStatementProgram(
        "copy_err.bas", "10 COPY (0,0)-(1,1) TO (2,2), 5, 6, 7, 8\n20 END\n",
        &errors);
    CHECK(ok == false);
    CHECK(errors.find("COPY parameters not supported") != std::string::npos);
  }

  TEST_CASE("COPY rejects invalid address type") {
    std::string errors;
    bool ok = compileStatementProgram(
        "copy_err_addr.bas", "10 DIM A%(3)\n20 COPY A%(0),5 TO (1,1)\n30 END\n",
        &errors);
    CHECK(ok == false);
    CHECK(errors.find("Invalid address in COPY") != std::string::npos);
  }

  TEST_CASE("PUT statement variants compile") {
    SUBCASE("PUT SPRITE with full attributes") {
      std::string errors;
      bool ok = compileStatementProgram(
          "put_sprite_full.bas",
          "10 PUT SPRITE 0,(10,10),15,0\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PUT SPRITE with STEP") {
      std::string errors;
      bool ok = compileStatementProgram(
          "put_sprite_step.bas",
          "10 PUT SPRITE 0,STEP(1,1),15,0\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PUT TILE") {
      std::string errors;
      bool ok = compileStatementProgram(
          "put_tile.bas", "10 PUT TILE 1,(1,1)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PUT SPRITE with empty attributes") {
      std::string errors;
      bool ok = compileStatementProgram(
          "put_sprite_empty.bas", "10 PUT SPRITE 0,(10,10),,\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PUT SPRITE with color only") {
      std::string errors;
      bool ok = compileStatementProgram(
          "put_sprite_color.bas", "10 PUT SPRITE 0,(1,2),3\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PUT TILE with STEP coordinates") {
      std::string errors;
      bool ok = compileStatementProgram(
          "put_tile_step.bas", "10 PUT TILE 1,STEP(1,2)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("SCREEN statement modes compile") {
    SUBCASE("SCREEN 0") {
      std::string errors;
      bool ok = compileStatementProgram("screen_0.bas", "10 SCREEN 0\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN 1 with page") {
      std::string errors;
      bool ok = compileStatementProgram("screen_1.bas", "10 SCREEN 1,2\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN 2 with pages") {
      std::string errors;
      bool ok = compileStatementProgram("screen_2.bas", "10 SCREEN 2,3,4\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN 3 with full switches") {
      std::string errors;
      bool ok = compileStatementProgram("screen_3.bas", "10 SCREEN 3,4,5,6\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN 4") {
      std::string errors;
      bool ok = compileStatementProgram("screen_4.bas", "10 SCREEN 4\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN 1 with all six parameters") {
      std::string errors;
      bool ok = compileStatementProgram(
          "screen_6param.bas", "10 SCREEN 1,2,3,4,5,6\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN 1 with five parameters") {
      std::string errors;
      bool ok = compileStatementProgram(
          "screen_5param.bas", "10 SCREEN 1,2,3,4,5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN with null parameter") {
      std::string errors;
      bool ok = compileStatementProgram(
          "screen_null.bas", "10 SCREEN 1,,3\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("SCREEN sub-commands compile") {
    SUBCASE("SCREEN PASTE FROM variable") {
      std::string errors;
      bool ok = compileStatementProgram(
          "screen_paste.bas", "10 A=1\n20 SCREEN PASTE FROM A\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN SCROLL") {
      std::string errors;
      bool ok = compileStatementProgram(
          "screen_scroll.bas", "10 SCREEN SCROLL 1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN LOAD") {
      std::string errors;
      bool ok = compileStatementProgram(
          "screen_load.bas", "10 A=1\n20 SCREEN LOAD A\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN ON") {
      std::string errors;
      bool ok = compileStatementProgram("screen_on.bas", "10 SCREEN ON\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SCREEN OFF") {
      std::string errors;
      bool ok = compileStatementProgram("screen_off.bas", "10 SCREEN OFF\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("CIRCLE statement variants compile") {
    SUBCASE("CIRCLE with radius only") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_radius.bas", "10 CIRCLE (10,10),5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("CIRCLE with color") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_color.bas", "10 CIRCLE (10,10),5,1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("CIRCLE with start angle") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_start.bas", "10 CIRCLE (10,10),5,1,2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("CIRCLE with end angle") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_end.bas", "10 CIRCLE (10,10),5,1,2,3\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("CIRCLE with aspect") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_aspect.bas", "10 CIRCLE (10,10),5,1,2,3,4\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("CIRCLE rejects too many parameters") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_too_many.bas", "10 CIRCLE (10,10),5,1,2,3,4,5\n20 END\n",
          &errors);
      CHECK(ok == false);
      CHECK(errors.find("Invalid CIRCLE parameters") != std::string::npos);
    }

    SUBCASE("CIRCLE with STEP coordinates") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_step.bas", "10 CIRCLE STEP(1,1),5\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("CIRCLE with empty attributes") {
      std::string errors;
      bool ok = compileStatementProgram(
          "circle_empty_attr.bas", "10 CIRCLE (10,10),5,,,2\n20 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("PAINT statement variants compile") {
    SUBCASE("PAINT with fill color only") {
      std::string errors;
      bool ok = compileStatementProgram(
          "paint_fill.bas", "10 PAINT (1,1),2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PAINT with fill and border") {
      std::string errors;
      bool ok = compileStatementProgram(
          "paint_border.bas", "10 PAINT (1,1),2,3\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PAINT with STEP coordinates") {
      std::string errors;
      bool ok = compileStatementProgram(
          "paint_step.bas", "10 PAINT STEP(1,1),2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PAINT without fill color") {
      std::string errors;
      bool ok = compileStatementProgram(
          "paint_nofill.bas", "10 PAINT (1,1)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PAINT with border only") {
      std::string errors;
      bool ok = compileStatementProgram(
          "paint_border_only.bas", "10 PAINT (1,1),,3\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PAINT rejects too many parameters") {
      std::string errors;
      bool ok = compileStatementProgram(
          "paint_too_many.bas", "10 PAINT (1,1),2,3,4\n20 END\n", &errors);
      CHECK(ok == false);
      CHECK(errors.find("PAINT parameters not supported") !=
            std::string::npos);
    }
  }

  TEST_CASE("PSET and PRESET statement variants compile") {
    SUBCASE("PSET with color") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_color.bas", "10 PSET (1,1),2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PSET with color and mode") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_mode.bas", "10 PSET (1,1),2,3\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PSET with STEP") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_step.bas", "10 PSET STEP(1,1),2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PSET with OR mode") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_or.bas", "10 PSET (1,1),2,OR\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PSET with XOR mode") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_xor.bas", "10 PSET (1,1),2,XOR\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PSET with AND mode") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_and.bas", "10 PSET (1,1),2,AND\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PSET with variables") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_var.bas", "10 X=1:Y=2\n20 PSET (X,Y),2\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PSET with empty attribute") {
      std::string errors;
      bool ok = compileStatementProgram(
          "pset_empty.bas", "10 PSET (1,1),\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("PRESET with color") {
      std::string errors;
      bool ok = compileStatementProgram(
          "preset_color.bas", "10 PRESET (1,1),2\n20 END\n", &errors);
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

    SUBCASE("LINE with variables") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_var.bas", "10 X=1:Y=2\n20 LINE (X,Y)-(1,1)\n30 END\n",
          &errors);
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

    SUBCASE("LINE with filled box option") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_bf.bas", "10 LINE (0,0)-(1,1),15,BF\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("LINE with OR mode") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_or.bas", "10 LINE (0,0)-(1,1),15,OR\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("LINE with TO STEP") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_tostep.bas", "10 LINE (0,0)-STEP(1,1)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("LINE from STEP") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_fromstep.bas", "10 LINE STEP(0,0)-(1,1)\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("LINE with B option") {
      std::string errors;
      bool ok = compileStatementProgram(
          "line_b_opt.bas", "10 LINE (0,0)-(1,1),15,B\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("COLOR statement variants compile") {
    SUBCASE("COLOR foreground only") {
      std::string errors;
      bool ok = compileStatementProgram(
          "color_fg.bas", "10 COLOR 15\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("COLOR foreground and background") {
      std::string errors;
      bool ok = compileStatementProgram(
          "color_bg.bas", "10 COLOR 15,1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("COLOR foreground, background and border") {
      std::string errors;
      bool ok = compileStatementProgram(
          "color_border.bas", "10 COLOR 15,1,1\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("KEY statement variants compile") {
    SUBCASE("KEY ON") {
      std::string errors;
      bool ok = compileStatementProgram("key_on.bas", "10 KEY ON\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("KEY OFF") {
      std::string errors;
      bool ok = compileStatementProgram("key_off.bas", "10 KEY OFF\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("KEY with keylist") {
      std::string errors;
      bool ok = compileStatementProgram(
          "key_list.bas", "10 KEY 1,\"ABC\"\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("KEY with ON trap") {
      std::string errors;
      bool ok = compileStatementProgram(
          "key_trap_on.bas", "10 KEY 1,ON\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("KEY with OFF trap") {
      std::string errors;
      bool ok = compileStatementProgram(
          "key_trap_off.bas", "10 KEY 1,OFF\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("KEY with STOP trap") {
      std::string errors;
      bool ok = compileStatementProgram(
          "key_trap_stop.bas", "10 KEY 1,STOP\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("DRAW statement compiles") {
    std::string errors;
    bool ok = compileStatementProgram(
        "draw_cmd.bas", "10 DRAW \"U1D1L1R1\"\n20 END\n", &errors);
    CHECK(ok == true);
    CHECK(errors.empty());
  }

  TEST_CASE("STRIG statement variants compile") {
    SUBCASE("STRIG ON") {
      std::string errors;
      bool ok = compileStatementProgram("strig_on.bas", "10 STRIG 1,ON\n20 END\n",
                                        &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("STRIG OFF") {
      std::string errors;
      bool ok = compileStatementProgram(
          "strig_off.bas", "10 STRIG 1,OFF\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("STRIG STOP") {
      std::string errors;
      bool ok = compileStatementProgram(
          "strig_stop.bas", "10 STRIG 1,STOP\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }
  }

  TEST_CASE("DRAW statement compiles in MegaROM mode") {
    const std::string path =
        createTempBas("draw_mega.bas", "10 DRAW \"U1D1L1R1\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    opts->setInputFilename(path);

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    shared_ptr<Lexer> lexer = make_shared<Lexer>();
    shared_ptr<Parser> parser = make_shared<Parser>();

    bool ok = false;
    if (lexer->load(opts) && lexer->evaluate() && parser->evaluate(lexer)) {
      ok = compiler->build(parser);
    }

    CHECK(ok == true);
    std::remove(path.c_str());
  }
}

TEST_SUITE("CompilerScreenDirect") {
  TEST_CASE("SCREEN COPY with empty parameters is rejected") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> copy = make_shared<ActionNode>(gfxKw("COPY"));
    action->actions.push_back(copy);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "SCREEN COPY with empty parameters") != std::string::npos);
  }

  TEST_CASE("SCREEN PASTE with empty parameters is rejected") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> paste = make_shared<ActionNode>(gfxKw("PASTE"));
    action->actions.push_back(paste);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "SCREEN PASTE with empty parameters") != std::string::npos);
  }

  TEST_CASE("SCREEN SCROLL with empty parameters is rejected") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> scroll = make_shared<ActionNode>(gfxKw("SCROLL"));
    action->actions.push_back(scroll);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "SCREEN SCROLL with empty parameters") != std::string::npos);
  }

  TEST_CASE("SCREEN LOAD with empty parameters is rejected") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> load = make_shared<ActionNode>(gfxKw("LOAD"));
    action->actions.push_back(load);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "SCREEN LOAD with empty parameters") != std::string::npos);
  }

  TEST_CASE("SCREEN COPY with identifier parameter") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> copy = make_shared<ActionNode>(gfxKw("COPY"));
    copy->actions.push_back(make_shared<ActionNode>(gfxId("A")));
    action->actions.push_back(copy);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->compiled == true);
  }

  TEST_CASE("SCREEN PASTE with identifier parameter") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> paste = make_shared<ActionNode>(gfxKw("PASTE"));
    paste->actions.push_back(make_shared<ActionNode>(gfxId("A")));
    action->actions.push_back(paste);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->compiled == true);
  }

  TEST_CASE("SCREEN SCROLL with parameter") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> scroll = make_shared<ActionNode>(gfxKw("SCROLL"));
    scroll->actions.push_back(make_shared<ActionNode>(gfxNum("1")));
    action->actions.push_back(scroll);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->compiled == true);
  }

  TEST_CASE("SCREEN LOAD with parameter") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> load = make_shared<ActionNode>(gfxKw("LOAD"));
    load->actions.push_back(make_shared<ActionNode>(gfxNum("1")));
    action->actions.push_back(load);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->compiled == true);
  }

  TEST_CASE("SCREEN COPY with excess parameters is rejected") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> copy = make_shared<ActionNode>(gfxKw("COPY"));
    copy->actions.push_back(make_shared<ActionNode>(gfxNum("1")));
    copy->actions.push_back(make_shared<ActionNode>(gfxNum("2")));
    copy->actions.push_back(make_shared<ActionNode>(gfxNum("3")));
    action->actions.push_back(copy);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "SCREEN COPY with excess of parameters") != std::string::npos);
  }

  TEST_CASE("SCREEN PASTE with excess parameters is rejected") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> paste = make_shared<ActionNode>(gfxKw("PASTE"));
    paste->actions.push_back(make_shared<ActionNode>(gfxNum("1")));
    paste->actions.push_back(make_shared<ActionNode>(gfxNum("2")));
    action->actions.push_back(paste);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "SCREEN PASTE with excess of parameters") != std::string::npos);
  }

  TEST_CASE("SCREEN COPY with two parameters") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> copy = make_shared<ActionNode>(gfxKw("COPY"));
    copy->actions.push_back(make_shared<ActionNode>(gfxId("A")));
    copy->actions.push_back(make_shared<ActionNode>(gfxNum("1")));
    action->actions.push_back(copy);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->compiled == true);
  }

  TEST_CASE("SCREEN SCROLL with excess parameters is rejected") {
    CompilerScreenStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("SCREEN");
    shared_ptr<ActionNode> scroll = make_shared<ActionNode>(gfxKw("SCROLL"));
    scroll->actions.push_back(make_shared<ActionNode>(gfxNum("1")));
    scroll->actions.push_back(make_shared<ActionNode>(gfxNum("2")));
    action->actions.push_back(scroll);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "SCREEN SCROLL with excess of parameters") != std::string::npos);
  }
}

TEST_SUITE("CompilerPutDirect") {
  TEST_CASE("PUT with empty parameters is rejected") {
    CompilerPutStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("PUT");
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find("Empty PUT statement") !=
          std::string::npos);
  }

  TEST_CASE("PUT with invalid sub-command is rejected") {
    CompilerPutStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("PUT");
    action->actions.push_back(make_shared<ActionNode>(gfxKw("FOOBAR")));
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find("Invalid PUT statement") !=
          std::string::npos);
  }

  TEST_CASE("PUT SPRITE with empty parameters is rejected") {
    CompilerPutStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("PUT");
    shared_ptr<ActionNode> sprite = make_shared<ActionNode>(gfxKw("SPRITE"));
    action->actions.push_back(sprite);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "PUT SPRITE with empty parameters") != std::string::npos);
  }

  TEST_CASE("PUT TILE with empty parameters is rejected") {
    CompilerPutStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("PUT");
    shared_ptr<ActionNode> tile = make_shared<ActionNode>(gfxKw("TILE"));
    action->actions.push_back(tile);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "PUT TILE with empty parameters") != std::string::npos);
  }

  TEST_CASE("PUT SPRITE with bad coordinates is rejected") {
    CompilerPutStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("PUT");
    shared_ptr<ActionNode> sprite = make_shared<ActionNode>(gfxKw("SPRITE"));
    shared_ptr<ActionNode> coord = make_shared<ActionNode>(gfxNum("0"));
    sprite->actions.push_back(make_shared<ActionNode>(gfxNum("0")));
    sprite->actions.push_back(coord);
    action->actions.push_back(sprite);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "Coordenates parameters error on PUT SPRITE") !=
          std::string::npos);
  }

  TEST_CASE("PUT SPRITE with excess parameters is rejected") {
    CompilerPutStatementStrategy strategy;
    shared_ptr<CompilerContext> ctx = createGraphicsContext();

    shared_ptr<ActionNode> action = gfxAction("PUT");
    shared_ptr<ActionNode> sprite = make_shared<ActionNode>(gfxKw("SPRITE"));
    shared_ptr<ActionNode> coord = make_shared<ActionNode>("COORD");
    coord->actions.push_back(make_shared<ActionNode>(gfxNum("1")));
    coord->actions.push_back(make_shared<ActionNode>(gfxNum("2")));
    sprite->actions.push_back(make_shared<ActionNode>(gfxNum("0")));
    sprite->actions.push_back(coord);
    sprite->actions.push_back(make_shared<ActionNode>(gfxNum("3")));
    sprite->actions.push_back(make_shared<ActionNode>(gfxNum("4")));
    sprite->actions.push_back(make_shared<ActionNode>(gfxNum("5")));
    action->actions.push_back(sprite);
    ctx->current_action = action;

    strategy.execute(ctx);
    CHECK(ctx->logger->errors().toString().find(
              "PUT SPRITE parameters not supported") != std::string::npos);
  }
}

// NOLINTEND
