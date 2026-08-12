/***
 * @file test_compiler.cpp
 * @brief MSXBAS2ROM compiler unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "action_node.h"
#include "build_options.h"
#include "compiler.h"
#include "compiler_cmd_handler_factory.h"
#include "compiler_context.h"
#include "compiler_function_strategy_factory.h"
#include "compiler_hooks.h"
#include "compiler_statement_strategy_factory.h"
#include "cpu_workspace_context.h"
#include "doctest/doctest.h"
#include "lexeme.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "resources.h"
#include "z80.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static bool compileProgram(const std::string& filename,
                           shared_ptr<Compiler> compiler,
                           shared_ptr<Parser> parser) {
  shared_ptr<Lexer> lexer = make_shared<Lexer>();
  if (!lexer->load(filename)) return false;
  if (!lexer->evaluate()) return false;
  if (!parser->evaluate(lexer)) return false;
  return compiler->build(parser);
}

static bool compileProgram(const std::string& filename,
                           shared_ptr<Compiler> compiler) {
  shared_ptr<Lexer> lexer = make_shared<Lexer>();
  shared_ptr<Parser> parser = make_shared<Parser>();

  if (!lexer->load(filename)) return false;
  if (!lexer->evaluate()) return false;
  if (!parser->evaluate(lexer)) return false;
  return compiler->build(parser);
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

static shared_ptr<CompilerContext> createCmdContext(
    shared_ptr<Z80OpcodeWriter> cpu,
    shared_ptr<CpuWorkspaceContext> workspace) {
  workspace->clear();
  cpu->context = workspace;
  shared_ptr<CompilerContext> ctx = make_shared<CompilerContext>();
  ctx->setHelpers(ctx);
  ctx->cpu = cpu;
  ctx->opts = make_shared<BuildOptions>();
  ctx->compiled = true;
  return ctx;
}

static shared_ptr<Lexeme> numLex(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_numeric, v);
}

static shared_ptr<Lexeme> strLex(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_literal, Lexeme::subtype_string, v);
}

static shared_ptr<Lexeme> idLex(const std::string& v) {
  return make_shared<Lexeme>(Lexeme::type_identifier, Lexeme::subtype_numeric,
                             v);
}

static shared_ptr<ActionNode> makeCmdAction(
    const std::string& keyword, const std::vector<shared_ptr<Lexeme>>& params) {
  shared_ptr<ActionNode> action = make_shared<ActionNode>(keyword);
  for (const auto& lex : params) {
    action->actions.push_back(make_shared<ActionNode>(lex));
  }
  return action;
}

static bool compileStatementProgram(const std::string& filename,
                                    const std::string& program,
                                    std::string* error_out = nullptr) {
  const std::string path = createTempBas(filename, program);

  shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter = make_shared<Z80OpcodeWriter>();
  shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

  bool ok = compileProgram(path, compiler);
  if (!ok && error_out) {
    *error_out = compiler->getLogger()->errors().toString();
  }

  std::remove(path.c_str());

  return ok;
}

TEST_SUITE("Compiler") {
  TEST_CASE("Compiles valid parser output") {
    const std::string filename =
        createTempBas("compiler_valid.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    CHECK(compileProgram(filename, compiler) == true);
    CHECK(compiler->isCompiled() == true);
    CHECK(compiler->getCodeSize() > 0);
    CHECK(compiler->getRamSize() >= 0);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when parser has no tags") {
    const std::string filename = createTempBas("compiler_empty.bas", "\n\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler->isCompiled() == false);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails on duplicated line numbers") {
    const std::string filename = createTempBas(
        "compiler_duplicated_line.bas", "10 PRINT \"A\"\n10 PRINT \"B\"\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler->getLogger()->errors().toString().find(
              "Line number already declared") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when FOR has no NEXT") {
    const std::string filename = createTempBas(
        "compiler_for_without_next.bas", "10 FOR I=1 TO 10\n20 PRINT I\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler->getLogger()->errors().toString().find(
              "FOR without a NEXT") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when NEXT has no FOR") {
    const std::string filename =
        createTempBas("compiler_next_without_for.bas", "10 NEXT I\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler->getLogger()->errors().toString().find(
              "NEXT without a FOR") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails on unknown function or array in expression") {
    const std::string filename =
        createTempBas("compiler_unknown_func.bas", "10 A=FOO(1)\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);
    CHECK(compiler->getLogger()->errors().toString().find(
              "Undeclared array or unknown function") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Registers DATA and IDATA resources") {
    const std::string filename =
        createTempBas("compiler_data_idata.bas", "10 DATA 1,2\n20 IDATA 3\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    shared_ptr<Parser> parser = make_shared<Parser>();
    CHECK(compileProgram(filename, compiler, parser) == true);
    CHECK(compiler->getResourceManager() != nullptr);
    CHECK(compiler->getResourceManager()->resources.size() >= 2);

    std::remove(filename.c_str());
  }

  TEST_CASE("Writes compiled code to output buffer") {
    const std::string filename =
        createTempBas("compiler_write.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == true);

    std::vector<unsigned char> out(0x8000, 0);
    int written = compiler->write(out.data(), 0x8000);
    CHECK(written > 0);

    std::remove(filename.c_str());
  }

  TEST_CASE("Writes compiled code in MegaROM mode") {
    const std::string filename =
        createTempBas("compiler_write_megarom.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    std::vector<unsigned char> out(0x8000 * 4, 0);
    int written = compiler->write(out.data(), 0x8000);
    CHECK(written > 0);

    std::remove(filename.c_str());
  }

  TEST_CASE("Writes multi-segment MegaROM code crossing 16K boundaries") {
    std::string content;
    int line = 1;
    for (int i = 1; i < 200; i++) {
      content += std::to_string(line++) + " A=" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " B=A+" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " C=A*B\n";
      content += std::to_string(line++) + " PRINT C\n";
    }
    content += std::to_string(line) + " END\n";

    const std::string filename =
        createTempBas("compiler_write_multiseg.bas", content);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    CHECK(compiler->getCodeSize() > 0x4000);

    std::vector<unsigned char> out(0x20000, 0);
    int written = compiler->write(out.data(), 0x8000);
    CHECK(written > 0x4000);

    std::remove(filename.c_str());
  }

  TEST_CASE("Fails when compiled code exceeds maximum ROM limit") {
    const std::string content =
        "10 FOR I=1 TO 100\n20 A=I\n30 NEXT I\n40 FOR I=1 TO 100\n"
        "50 A=I\n60 NEXT I\n70 FOR I=1 TO 100\n80 A=I\n90 NEXT I\n"
        "100 FOR I=1 TO 100\n110 A=I\n120 NEXT I\n"
        "130 FOR I=1 TO 100\n140 A=I\n150 NEXT I\n"
        "160 FOR I=1 TO 100\n170 A=I\n180 NEXT I\n"
        "190 FOR I=1 TO 100\n200 A=I\n210 NEXT I\n"
        "220 FOR I=1 TO 100\n230 A=I\n240 NEXT I\n"
        "250 FOR I=1 TO 100\n260 A=I\n270 NEXT I\n"
        "280 FOR I=1 TO 100\n290 A=I\n300 NEXT I\n"
        "310 FOR I=1 TO 100\n320 A=I\n330 NEXT I\n"
        "340 FOR I=1 TO 100\n350 A=I\n360 NEXT I\n"
        "370 FOR I=1 TO 100\n380 A=I\n390 NEXT I\n"
        "400 FOR I=1 TO 100\n410 A=I\n420 NEXT I\n"
        "430 FOR I=1 TO 100\n440 A=I\n450 NEXT I\n"
        "460 FOR I=1 TO 100\n470 A=I\n480 NEXT I\n"
        "490 FOR I=1 TO 100\n500 A=I\n510 NEXT I\n"
        "520 FOR I=1 TO 100\n530 A=I\n540 NEXT I\n"
        "550 FOR I=1 TO 100\n560 A=I\n570 NEXT I\n"
        "580 FOR I=1 TO 100\n590 A=I\n600 NEXT I\n"
        "610 FOR I=1 TO 100\n620 A=I\n630 NEXT I\n"
        "640 FOR I=1 TO 100\n650 A=I\n660 NEXT I\n"
        "670 FOR I=1 TO 100\n680 A=I\n690 NEXT I\n"
        "700 FOR I=1 TO 100\n710 A=I\n720 NEXT I\n"
        "730 FOR I=1 TO 100\n740 A=I\n750 NEXT I\n"
        "760 FOR I=1 TO 100\n770 A=I\n780 NEXT I\n"
        "790 FOR I=1 TO 100\n800 A=I\n810 NEXT I\n"
        "820 FOR I=1 TO 100\n830 A=I\n840 NEXT I\n"
        "850 FOR I=1 TO 100\n860 A=I\n870 NEXT I\n"
        "880 FOR I=1 TO 100\n890 A=I\n900 NEXT I\n"
        "910 FOR I=1 TO 100\n920 A=I\n930 NEXT I\n"
        "940 FOR I=1 TO 100\n950 A=I\n960 NEXT I\n"
        "970 FOR I=1 TO 100\n980 A=I\n990 NEXT I\n";
    const std::string filename =
        createTempBas("compiler_too_large.bas", content);

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    bool ok = compileProgram(filename, compiler);

    if (ok) {
      std::vector<unsigned char> out(0x8000 * 4, 0);
      compiler->write(out.data(), 0x8000);
    }
    std::remove(filename.c_str());
    CHECK(true);
  }

  TEST_CASE("Error line number matches physical file line") {
    const std::string content =
        "\n"                       // line 1 (blank)
        "10 PRINT \"A\"\n"         // line 2
        "\n"                       // line 3 (blank)
        "20 PRINT \"B\"\n"         // line 4
        "\n"                       // line 5 (blank)
        "10 PRINT \"C\"\n";        // line 6 - duplicated line 10

    const std::string filename =
        createTempBas("compiler_phys_line.bas", content);

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    CHECK(compileProgram(filename, compiler) == false);

    auto errors = compiler->getLogger()->errors();
    auto& entries = errors.getAll();
    REQUIRE(entries.size() >= 1);
    CHECK(entries[0].line == 6);
    CHECK(entries[0].message.find("Line number already declared") !=
          std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Exposes accessor state after compilation") {
    const std::string filename =
        createTempBas("compiler_accessors.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    REQUIRE(compileProgram(filename, compiler) == true);

    CHECK(compiler->getRamMemoryPerc() >= 0.0f);
    CHECK(compiler->getCurrentTag() == nullptr);
    CHECK(compiler->getSymbolManager() != nullptr);
    const Compiler& constCompiler = *compiler;
    CHECK(constCompiler.getSymbolManager() != nullptr);
    CHECK(constCompiler.getResourceManager() != nullptr);
    CHECK(compiler->getOpts() != nullptr);

    std::remove(filename.c_str());
  }

  TEST_CASE("Flags PT3, AKM, font and tiny sprite state") {
    SUBCASE("PT3 flag exposed through getter") {
      const std::string filename = createTempBas(
          "compiler_flag_pt3.bas", "10 PRINT \"A\"\n20 END\n");

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      REQUIRE(compileProgram(filename, compiler) == true);
      CHECK(compiler->getPt3() == false);
      CHECK(compiler->getAkm() == false);
      CHECK(compiler->getFont() == false);

      std::remove(filename.c_str());
    }

    SUBCASE("AKM flag is set by CMD PLYLOAD") {
      const std::string filename = createTempBas(
          "compiler_flag_akm.bas", "10 CMD PLYLOAD 1\n20 END\n");

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      REQUIRE(compileProgram(filename, compiler) == true);
      CHECK(compiler->getPt3() == false);
      CHECK(compiler->getAkm() == true);

      std::remove(filename.c_str());
    }

    SUBCASE("Font flag is set by SET FONT") {
      const std::string filename = createTempBas(
          "compiler_flag_font.bas", "10 SET FONT 1\n20 END\n");

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      REQUIRE(compileProgram(filename, compiler) == true);
      CHECK(compiler->getFont() == true);

      std::remove(filename.c_str());
    }

    SUBCASE("Tiny sprite flag defaults to false") {
      const std::string filename = createTempBas(
          "compiler_flag_tiny.bas", "10 PRINT \"A\"\n20 END\n");

      shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
          make_shared<Z80OpcodeWriter>();
      shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
      REQUIRE(compileProgram(filename, compiler) == true);
      CHECK(compiler->getHasTinySprite() == false);

      std::remove(filename.c_str());
    }
  }

  TEST_CASE("Rejects SET SCREEN with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_screen_param.bas", "10 SET SCREEN,1\n20 END\n",
        &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong parameters count on SET SCREEN statement") !=
          std::string::npos);
  }

  TEST_CASE("Rejects SET BEEP with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_beep_param.bas", "10 SET BEEP\n20 END\n", &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong parameters count on SET BEEP statement") !=
          std::string::npos);
  }

  TEST_CASE("Rejects SET VIDEO with empty parameters") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_video_empty.bas", "10 SET VIDEO\n20 END\n", &errors);

    CHECK(ok == false);
    CHECK(errors.find("SET VIDEO with empty parameters") != std::string::npos);
  }

  TEST_CASE("Rejects SET ADJUST with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_adjust_count.bas", "10 SET ADJUST (1,2,3)\n20 END\n",
        &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong parameters count on SET ADJUST statement") !=
          std::string::npos);
  }

  TEST_CASE("Rejects SET TITLE with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_title_count.bas", "10 SET TITLE \"A\"\n20 END\n",
        &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong parameters count on SET TITLE statement") !=
          std::string::npos);
  }

  TEST_CASE("Rejects SET PAGE with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_page_count.bas",
        "10 SET PAGE 1,2,3\n20 END\n", &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong parameters count on SET PAGE statement") !=
          std::string::npos);
  }

  TEST_CASE("Rejects SET PROMPT with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_prompt_count.bas", "10 SET PROMPT\n20 END\n",
        &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong parameters count on SET PROMPT statement") !=
          std::string::npos);
  }

  TEST_CASE("Rejects SET TIME with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_time_count.bas", "10 SET TIME 1,2\n20 END\n",
        &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong SET TIME parameters count") != std::string::npos);
  }

  TEST_CASE("Rejects SET DATE with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_date_count.bas", "10 SET DATE 1\n20 END\n", &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong SET DATE parameters count") != std::string::npos);
  }

  TEST_CASE("Rejects SET SCROLL with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_scroll_count.bas", "10 SET SCROLL 1,2,3,4,5\n20 END\n",
        &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong parameters count on SET SCROLL statement") !=
          std::string::npos);
  }

  TEST_CASE("Rejects SET FONT with wrong parameter count") {
    std::string errors;
    bool ok = compileStatementProgram(
        "compiler_stmt_set_font_count.bas", "10 SET FONT 1,2,3\n20 END\n",
        &errors);

    CHECK(ok == false);
    CHECK(errors.find("Wrong number of parameters on SET FONT") !=
          std::string::npos);
  }
}

TEST_SUITE("CompilerCmdHandlerFactory") {
  TEST_CASE("All CMD handlers have unit tests") {
    CompilerCmdHandlerFactory factory;
    CHECK(factory.size() == 42);
  }

  TEST_CASE("All CMD handlers are registered") {
    CompilerCmdHandlerFactory factory;

    std::vector<std::string> keywords = {
        "RUNASM",    "RUNBAS",    "WRTVRAM",   "WRTFNT",    "WRTCHR",
        "WRTCLR",    "WRTSCR",    "WRTSPR",    "WRTSPRPAT", "WRTSPRCLR",
        "WRTSPRATR", "RAMTOVRAM", "VRAMTORAM", "RAMTORAM",  "RSCTORAM",
        "DISSCR",    "ENASCR",    "KEYCLKOFF", "MUTE",      "PLAY",
        "DRAW",      "CLIP",      "PT3LOAD",   "PT3PLAY",   "PT3MUTE",
        "PT3LOOP",   "PT3REPLAY", "PLYLOAD",   "PLYPLAY",   "PLYSONG",
        "PLYMUTE",   "PLYLOOP",   "PLYREPLAY", "PLYSOUND",  "MTF",
        "SETFNT",    "UPDFNTCLR", "CLRSCR",    "CLRKEY",    "PAGE",
        "RESTORE",   "TURBO"};

    for (const auto& kw : keywords) {
      CHECK(factory.getByKeyword(kw) != nullptr);
    }
  }

  TEST_CASE("Unknown keyword returns nullptr") {
    CompilerCmdHandlerFactory factory;

    CHECK(factory.getByKeyword("FOOBAR") == nullptr);
    CHECK(factory.getByKeyword("") == nullptr);
    CHECK(factory.getByKeyword("INVALIDCMD") == nullptr);
  }

  TEST_CASE("PT3 commands share same handler") {
    CompilerCmdHandlerFactory factory;

    auto* h1 = factory.getByKeyword("PT3LOAD");
    auto* h2 = factory.getByKeyword("PT3PLAY");
    auto* h3 = factory.getByKeyword("PT3MUTE");
    auto* h4 = factory.getByKeyword("PT3LOOP");
    auto* h5 = factory.getByKeyword("PT3REPLAY");

    CHECK(h1 != nullptr);
    CHECK(h1 == h2);
    CHECK(h1 == h3);
    CHECK(h1 == h4);
    CHECK(h1 == h5);
  }

  TEST_CASE("PLY commands have independent handlers") {
    CompilerCmdHandlerFactory factory;

    CHECK(factory.getByKeyword("PLYLOAD") != nullptr);
    CHECK(factory.getByKeyword("PLYPLAY") != nullptr);
    CHECK(factory.getByKeyword("PLYSONG") != nullptr);
    CHECK(factory.getByKeyword("PLYMUTE") != nullptr);
    CHECK(factory.getByKeyword("PLYLOOP") != nullptr);
    CHECK(factory.getByKeyword("PLYREPLAY") != nullptr);
    CHECK(factory.getByKeyword("PLYSOUND") != nullptr);
  }

  TEST_CASE("Repeated calls return same handler instance") {
    CompilerCmdHandlerFactory factory;

    auto* h1 = factory.getByKeyword("RUNASM");
    auto* h2 = factory.getByKeyword("RUNASM");

    CHECK(h1 != nullptr);
    CHECK(h1 == h2);
  }
}

TEST_SUITE("CompilerStatementStrategyFactory") {
  TEST_CASE("All strategies are registered") {
    CompilerStatementStrategyFactory factory;

    std::vector<std::string> keywords = {

        "REM",      "'",

        "FILE",     "TEXT",

        "CLEAR",    "DEF",       "DEFINT",   "DEFSNG", "DEFDBL", "DEFSTR",
        "DEFUSR",   "CLS",       "BEEP",     "LET",    "TIME",   "DIM",
        "REDIM",    "RANDOMIZE",

        "END",      "GOTO",      "GOSUB",    "RETURN", "IF",     "FOR",
        "NEXT",     "ON",        "INTERVAL", "STOP",

        "PRINT",    "INPUT",     "SOUND",    "OUT",    "POKE",   "VPOKE",
        "IPOKE",    "DATA",      "IDATA",    "READ",   "IREAD",  "RESTORE",
        "IRESTORE", "RESUME",    "WAIT",     "SWAP",   "CALL",   "CMD",
        "MAXFILES", "OPEN",      "OPEN_GRP", "CLOSE",  "BLOAD",

        "PLAY",     "DRAW",      "LOCATE",   "SCREEN", "WIDTH",  "COLOR",
        "PSET",     "PRESET",    "LINE",     "PAINT",  "CIRCLE", "COPY",
        "PUT",      "GET",       "SET",      "KEY",    "STRIG",  "SPRITE"};

    for (const auto& kw : keywords) {
      CHECK(factory.getByKeyword(kw) != nullptr);
    }
  }

  TEST_CASE("REM aliases share same strategy") {
    CompilerStatementStrategyFactory factory;

    auto* rem1 = factory.getByKeyword("REM");
    auto* rem2 = factory.getByKeyword("'");

    CHECK(rem1 != nullptr);
    CHECK(rem1 == rem2);
  }

  TEST_CASE("Unknown keyword returns nullptr") {
    CompilerStatementStrategyFactory factory;

    CHECK(factory.getByKeyword("FOOBAR") == nullptr);
    CHECK(factory.getByKeyword("") == nullptr);
    CHECK(factory.getByKeyword("INVALID") == nullptr);
  }

  TEST_CASE("Repeated calls return same instance") {
    CompilerStatementStrategyFactory factory;

    auto* s1 = factory.getByKeyword("PRINT");
    auto* s2 = factory.getByKeyword("PRINT");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("All statement strategies have unit tests") {
    CompilerStatementStrategyFactory factory;

    CHECK(factory.size() == 69);
  }
}

TEST_SUITE("CompilerFunctionStrategyFactory") {
  TEST_CASE("All function strategies are registered") {
    CompilerFunctionStrategyFactory factory;

    std::vector<std::string> keywords = {

        "NOOPERATION",

        "TIME",        "ERR",          "POS",    "LPOS",      "CSRLIN",
        "INKEY",
        "INKEY$",      "MAXFILES",     "FRE",    "HEAP",      "MSX",
        "NTSC",        "VDP",          "TURBO",  "COLLISION", "MAKER",
        "PLYSTATUS",

        "INT",         "FIX",          "RND",    "SIN",       "COS",
        "TAN",         "ATN",          "EXP",    "LOG",       "SQR",
        "SGN",         "ABS",          "VAL",    "PEEK",      "IPEEK",
        "VPEEK",       "INP",          "DSKF",   "EOF",       "LOC",
        "LOF",         "FPOS",         "PSG",    "PLAY",      "STICK",
        "STRIG",       "PAD",          "PDL",    "BASE",

        "ASC",         "LEN",          "CSNG",   "CDBL",      "CINT",

        "CHR$",        "SPACE$",       "SPC",    "TAB",       "STR$",
        "INPUT$",      "BIN$",         "OCT$",   "HEX$",

        "RESOURCE",    "RESOURCESIZE",

        "USR",         "USR0",         "USR1",   "USR2",      "USR3",
        "USR4",        "USR5",         "USR6",   "USR7",      "USR8",
        "USR9",

        "POINT",       "TILE",

        "STRING$",     "LEFT$",        "RIGHT$", "MID$",      "USING$",
        "INSTR",       "VARPTR"};

    for (const auto& kw : keywords) {
      CHECK(factory.getByKeyword(kw) != nullptr);
    }
  }

  TEST_CASE("Unknown keyword returns nullptr") {
    CompilerFunctionStrategyFactory factory;

    CHECK(factory.getByKeyword("FOOBAR") == nullptr);
    CHECK(factory.getByKeyword("") == nullptr);
    CHECK(factory.getByKeyword("INVALIDFUNC") == nullptr);
  }

  TEST_CASE("SPACE$ and SPC share same handler") {
    CompilerFunctionStrategyFactory factory;

    auto* s1 = factory.getByKeyword("SPACE$");
    auto* s2 = factory.getByKeyword("SPC");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("USR variants share same handler") {
    CompilerFunctionStrategyFactory factory;

    auto* base = factory.getByKeyword("USR");

    CHECK(base != nullptr);

    CHECK(base == factory.getByKeyword("USR0"));
    CHECK(base == factory.getByKeyword("USR1"));
    CHECK(base == factory.getByKeyword("USR2"));
    CHECK(base == factory.getByKeyword("USR3"));
    CHECK(base == factory.getByKeyword("USR4"));
    CHECK(base == factory.getByKeyword("USR5"));
    CHECK(base == factory.getByKeyword("USR6"));
    CHECK(base == factory.getByKeyword("USR7"));
    CHECK(base == factory.getByKeyword("USR8"));
    CHECK(base == factory.getByKeyword("USR9"));
  }

  TEST_CASE("Repeated calls return same instance") {
    CompilerFunctionStrategyFactory factory;

    auto* s1 = factory.getByKeyword("SIN");
    auto* s2 = factory.getByKeyword("SIN");

    CHECK(s1 != nullptr);
    CHECK(s1 == s2);
  }

  TEST_CASE("All function strategies have unit tests") {
    CompilerFunctionStrategyFactory factory;

    CHECK(factory.size() == 83);
  }
}

TEST_SUITE("CompilerStatementStrategies") {
  TEST_CASE("All statement strategies execute successfully") {
    struct StatementCase {
      const char* name;
      const char* program;
      bool expect_success;
    };

    const StatementCase cases[] = {
        {"REM", "10 REM TEST\n20 END\n", true},
        {"APOSTROPHE", "10 ' TEST\n20 END\n", true},
        {"FILE", "FILE \"asset.txt\"\n10 END\n", true},
        {"TEXT", "TEXT \"HELLO\"\n10 END\n", true},
        {"CLEAR", "10 CLEAR\n20 END\n", true},
        {"DEF", "10 DEF USR=1\n20 END\n", true},
        {"DEFINT", "10 DEFINT A-Z\n20 A=1\n30 END\n", true},
        {"DEFSNG", "10 DEFSNG A-Z\n20 A=1.5\n30 END\n", true},
        {"DEFDBL", "10 DEFDBL A-Z\n20 A=1\n30 END\n", true},
        {"DEFSTR", "10 DEFSTR A-Z\n20 A$=\"X\"\n30 END\n", true},
        {"DEFUSR", "10 DEF USR=1\n20 END\n", true},
        {"CLS", "10 CLS\n20 END\n", true},
        {"BEEP", "10 BEEP\n20 END\n", true},
        {"LET", "10 LET A=1\n20 END\n", true},
        {"TIME", "10 TIME=1\n20 END\n", true},
        {"DIM", "10 DIM A(1)\n20 END\n", true},
        {"REDIM", "10 REDIM A(2)\n20 END\n", true},
        {"RANDOMIZE", "10 RANDOMIZE 1\n20 END\n", true},
        {"END", "10 END\n", true},
        {"GOTO", "10 GOTO 20\n20 END\n", true},
        {"GOSUB", "10 GOSUB 20\n20 RETURN\n30 END\n", true},
        {"RETURN", "10 GOSUB 20\n20 RETURN\n30 END\n", true},
        {"IF", "10 IF 1 THEN PRINT \"A\"\n20 END\n", true},
        {"FOR", "10 FOR I=1 TO 2\n20 NEXT I\n30 END\n", true},
        {"NEXT", "10 FOR I=1 TO 2\n20 NEXT I\n30 END\n", true},
        {"ON", "10 ON 1 GOTO 20,30\n20 END\n30 END\n", true},
        {"INTERVAL", "10 INTERVAL ON\n20 END\n", true},
        {"STOP", "10 STOP\n20 END\n", true},
        {"PRINT", "10 PRINT \"HI\"\n20 END\n", true},
        {"INPUT", "10 INPUT A\n20 END\n", true},
        {"SOUND", "10 SOUND 1,2\n20 END\n", true},
        {"OUT", "10 OUT 1,2\n20 END\n", true},
        {"POKE", "10 POKE 1,2\n20 END\n", true},
        {"VPOKE", "10 VPOKE 1,2\n20 END\n", true},
        {"IPOKE", "10 IPOKE 1,2\n20 END\n", true},
        {"DATA", "10 DATA 1,2\n20 END\n", true},
        {"IDATA", "10 IDATA 1,2\n20 END\n", true},
        {"READ", "10 DATA 1\n20 READ A\n30 END\n", true},
        {"IREAD", "10 IDATA 1\n20 IREAD A\n30 END\n", true},
        {"RESTORE", "10 DATA 1\n20 RESTORE\n30 END\n", true},
        {"IRESTORE", "10 IDATA 1\n20 IRESTORE\n30 END\n", true},
        {"RESUME", "10 RESUME\n20 END\n", false},
        {"WAIT", "10 WAIT 1,2\n20 END\n", true},
        {"SWAP", "10 A=1\n20 B=2\n30 SWAP A,B\n40 END\n", true},
        {"CALL", "10 CALL TURBO\n20 END\n", true},
        {"CMD", "10 CMD KEYCLKOFF\n20 END\n", true},
        {"MAXFILES", "10 MAXFILES=5\n20 END\n", true},
        {"OPEN", "10 OPEN \"A\" FOR INPUT AS #1 LEN 1\n20 END\n", true},
        {"OPEN_GRP", "10 OPEN \"GRP:\" FOR OUTPUT AS #1\n20 END\n", true},
        {"CLOSE", "10 CLOSE #1\n20 END\n", true},
        {"BLOAD", "10 BLOAD \"ASSET.SCR\",S\n20 END\n", true},
        {"PLAY", "10 PLAY \"AB\"\n20 END\n", true},
        {"DRAW", "10 DRAW \"U1\"\n20 END\n", true},
        {"LOCATE", "10 LOCATE 1,1\n20 END\n", true},
        {"SCREEN", "10 SCREEN 0\n20 END\n", true},
        {"WIDTH", "10 WIDTH 40\n20 END\n", true},
        {"COLOR", "10 COLOR 15,1,1\n20 END\n", true},
        {"PSET", "10 PSET (1,1)\n20 END\n", true},
        {"PRESET", "10 PRESET (1,1)\n20 END\n", true},
        {"LINE", "10 LINE (0,0)-(1,1)\n20 END\n", true},
        {"PAINT", "10 PAINT (1,1),2\n20 END\n", true},
        {"CIRCLE", "10 CIRCLE (10,10),5\n20 END\n", true},
        {"COPY", "10 COPY (0,0)-(1,1) TO (2,2)\n20 END\n", true},
        {"PUT", "10 PUT SPRITE 0,(10,10)\n20 END\n", true},
        {"GET", "10 DEFINT A-Z\n20 GET TIME H,M,S\n30 END\n", true},
        {"SET", "10 SET SCREEN\n20 END\n", true},
        {"KEY", "10 KEY ON\n20 END\n", true},
        {"STRIG", "10 STRIG 1,ON\n20 END\n", true},
        {"SPRITE", "10 SPRITE ON\n20 END\n", true},
        {"SPRITE_LOAD", "10 SPRITE LOAD 0\n20 END\n", true},
        {"SPRITE_ASSIGN", "10 A$=STRING$(8,255)\n20 SPRITE$(0)=A$\n30 END\n",
         true},
        {"COLOR_SPRITE", "10 COLOR SPRITE(1)=33\n20 END\n", true},
        {"COLOR_SPRITE_STR", "10 COLOR SPRITE$(0)=CHR$(1)+CHR$(2)\n20 END\n",
         true},
        {"SET_SPRITE_PATTERN",
         "10 DIM PB%(3,3)\n20 SET SPRITE PATTERN 0, PB%\n30 END\n", true},
        {"GET_SPRITE_PATTERN",
         "10 DIM PB%(3,3)\n20 GET SPRITE PATTERN 0, PB%\n30 END\n", true},
        {"SET_SPRITE_COLOR",
         "10 DIM CB%(7)\n20 SET SPRITE COLOR 0, CB%\n30 END\n", true},
        {"GET_SPRITE_COLOR",
         "10 DIM CB%(7)\n20 GET SPRITE COLOR 0, CB%\n30 END\n", true},
        {"SET_TILE_PATTERN_INLINE",
         "10 SET TILE PATTERN 65, (255,129,129,129,129,129,129,255)\n20 END\n",
         true},
        {"SET_TILE_PATTERN_INLINE_BANK",
         "10 SET TILE PATTERN 65, (255,0,255,0,255,0,255,0), 1\n20 END\n",
         true},
        {"SET_TILE_PATTERN_ARRAY",
         "10 DIM PB%(3)\n20 PB%(0)=&H0100:PB%(1)=&H0302:PB%(2)=&H0504:PB%(3)=&H0706\n30 SET TILE PATTERN 65, PB%()\n40 END\n",
         true},
        {"SET_TILE_PATTERN_ARRAY_BANK",
         "10 DIM PB%(3)\n20 SET TILE PATTERN 65, PB%(), 2\n30 END\n",
         true},
        {"GET_TILE_PATTERN",
         "10 DIM PB%(3)\n20 GET TILE PATTERN 65, PB%()\n30 END\n",
         true},
         {"GET_TILE_PATTERN_BANK",
          "10 DIM PB%(3)\n20 GET TILE PATTERN 65, PB%(), 1\n30 END\n",
          true},
         {"SET_TILE_COLOR_FC_BC",
          "10 SET TILE COLOR 5, 1, 15\n20 END\n", true},
         {"SET_TILE_COLOR_FC_BC_BANK",
          "10 SET TILE COLOR 5, 1, 15, 1\n20 END\n", true},
         {"SET_TILE_COLOR_FC_ARRAY",
          "10 SET TILE COLOR 5, (1,2,3,4,5,6,7,8)\n20 END\n", true},
         {"SET_TILE_COLOR_FC_BC_ARRAYS",
          "10 SET TILE COLOR 5, (1,2,3,4,5,6,7,8), (15,14,13,12,11,10,9,8)\n20 END\n",
          true},
         {"SET_TILE_COLOR_FC_ARRAY_BANK",
          "10 SET TILE COLOR 5, (1,2,3,4,5,6,7,8), 1\n20 END\n", true},
         {"SET_TILE_COLOR_BUFFER",
          "10 DIM C%(3)\n20 SET TILE COLOR 5, C%()\n30 END\n", true},
         {"SET_TILE_COLOR_BUFFER_BANK",
          "10 DIM C%(3)\n20 SET TILE COLOR 5, C%(), 1\n30 END\n", true},
         {"GET_TILE_COLOR",
          "10 DIM C%(3)\n20 GET TILE COLOR 5, C%()\n30 END\n", true},
         {"GET_TILE_COLOR_BANK",
          "10 DIM C%(3)\n20 GET TILE COLOR 5, C%(), 2\n30 END\n", true},
        {"SET_TILE_FLIP",
         "10 SET TILE FLIP 0, 0\n20 END\n", true},
        {"SET_TILE_FLIP_BANK",
         "10 SET TILE FLIP 0, 1, 2\n20 END\n", true},
        {"SET_TILE_ROTATE",
         "10 SET TILE ROTATE 0, 0\n20 END\n", true},
        {"SET_TILE_ROTATE_BANK",
         "10 SET TILE ROTATE 0, 1, 1\n20 END\n", true},
        {"SET_TILE_COLOR_INVALID",
         "10 SET TILE COLOR 5\n20 END\n", false},
     };

    for (const auto& test_case : cases) {
      SUBCASE(test_case.name) {
        std::string errors;
        const std::string filename =
            std::string("compiler_stmt_") + test_case.name + ".bas";

        bool ok = compileStatementProgram(filename, test_case.program, &errors);

        CHECK(ok == test_case.expect_success);
        if (!test_case.expect_success) {
          CHECK(errors.size() > 0);
        }
      }
    }
  }

  TEST_CASE("LET statement compiles assignments of different types") {
    SUBCASE("MID$ assignment with two parameters") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_mid2.bas",
          "10 A$=\"HELLO WORLD\"\n20 MID$(A$,3)=\"X\"\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("MID$ assignment with three parameters") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_mid3.bas",
          "10 A$=\"HELLO WORLD\"\n20 MID$(A$,3,2)=\"XY\"\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("MID$ assignment with float start") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_mid_float.bas",
          "10 A$=\"HELLO WORLD\"\n20 MID$(A$,3.5)=\"X\"\n30 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("VDP register assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_vdp.bas", "10 VDP(1)=2\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("SPRITE$ assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_sprite.bas",
          "10 SPRITE$(0)=\"ABCD\"\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("String LET assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_string.bas", "10 A$=\"HELLO\"\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Float LET assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_float.bas", "10 A=3.14\n20 END\n", &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Array element LET assignment") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_array.bas", "10 DIM A(5)\n20 A(3)=42\n30 END\n",
          &errors);
      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Invalid MID$ assignment parameters") {
      std::string errors;
      bool ok = compileStatementProgram(
          "compiler_let_mid_invalid.bas", "10 MID$(A$)= \"X\"\n20 END\n",
          &errors);
      CHECK(ok == false);
      CHECK(errors.size() > 0);
    }
  }

  TEST_CASE(
      "PRINT# compiles with file output variants and rejects invalid forms") {
    SUBCASE("Valid PRINT# variants") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_print_file_valid.bas",
                                  "10 OPEN \"A:TEST.TXT\" FOR OUTPUT AS #1\n"
                                  "20 PRINT #1,\"A\",10,2.5\n"
                                  "30 PRINT #1,\"B\";\n"
                                  "40 PRINT #1,\"C\",\n"
                                  "50 PRINT #1\n"
                                  "60 CLOSE #1\n"
                                  "70 END\n",
                                  &errors);

      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Invalid PRINT# without file number expression") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_print_file_invalid_1.bas",
                                  "10 PRINT #,\"A\"\n20 END\n", &errors);

      CHECK(ok == false);
      CHECK(errors.size() > 0);
    }

    SUBCASE("Invalid PRINT# with duplicated separator") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_print_file_invalid_2.bas",
                                  "10 PRINT #1,,\"A\"\n20 END\n", &errors);

      CHECK(ok == false);
      CHECK(errors.size() > 0);
    }
  }

  TEST_CASE(
      "INPUT# and LINE INPUT# compile with file input variants and reject "
      "invalid forms") {
    SUBCASE("Valid INPUT# and LINE INPUT# variants") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_input_file_valid.bas",
                                  "10 OPEN \"A:TEST.TXT\" FOR INPUT AS #1\n"
                                  "20 INPUT #1,A$,B$\n"
                                  "30 LINE INPUT #1,L$\n"
                                  "40 CLOSE #1\n"
                                  "50 END\n",
                                  &errors);

      CHECK(ok == true);
      CHECK(errors.empty());
    }

    SUBCASE("Invalid INPUT# without file number expression") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_input_file_invalid_1.bas",
                                  "10 INPUT #,A$\n20 END\n", &errors);

      CHECK(ok == false);
      CHECK(errors.size() > 0);
    }

    SUBCASE("Invalid INPUT# with duplicated separator") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_input_file_invalid_2.bas",
                                  "10 INPUT #1,,A$\n20 END\n", &errors);

      CHECK(ok == false);
      CHECK(errors.size() > 0);
    }

    SUBCASE("Invalid INPUT# without destination variables") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_input_file_invalid_3.bas",
                                  "10 INPUT #1\n20 END\n", &errors);

      CHECK(ok == false);
      CHECK(errors.size() > 0);
    }

    SUBCASE("Invalid LINE INPUT# with non-identifier parameter") {
      std::string errors;
      bool ok =
          compileStatementProgram("compiler_stmt_input_file_invalid_4.bas",
                                  "10 LINE INPUT #1,\"A\"\n20 END\n", &errors);

      CHECK(ok == false);
      CHECK(errors.size() > 0);
    }
  }
}

TEST_SUITE("CompilerCmdHandlers") {
  TEST_CASE("All CMD instruction handlers execute successfully") {
    struct CmdCase {
      const char* name;
      const char* keyword;
      std::vector<shared_ptr<Lexeme>> params;
      bool expect_success;
      bool expect_pt3;
      bool expect_akm;
      bool expect_font;
    };

    const CmdCase cases[] = {
        {"RUNASM", "RUNASM", {numLex("1")}, true, false, false, false},
        {"RUNBAS", "RUNBAS", {}, false, false, false, false},
        {"WRTVRAM",
         "WRTVRAM",
         {numLex("1"), numLex("2")},
         true,
         false,
         false,
         false},
        {"WRTFNT", "WRTFNT", {numLex("1")}, true, false, false, false},
        {"WRTCHR", "WRTCHR", {numLex("1")}, true, false, false, false},
        {"WRTCLR", "WRTCLR", {numLex("1")}, true, false, false, false},
        {"WRTSCR", "WRTSCR", {numLex("1")}, true, false, false, false},
        {"WRTSPR", "WRTSPR", {numLex("1")}, true, false, false, false},
        {"WRTSPRPAT", "WRTSPRPAT", {numLex("1")}, true, false, false, false},
        {"WRTSPRCLR", "WRTSPRCLR", {numLex("1")}, true, false, false, false},
        {"WRTSPRATR", "WRTSPRATR", {numLex("1")}, true, false, false, false},
        {"RAMTOVRAM",
         "RAMTOVRAM",
         {numLex("1"), numLex("2"), numLex("3")},
         true,
         false,
         false,
         false},
        {"VRAMTORAM",
         "VRAMTORAM",
         {numLex("1"), numLex("2"), numLex("3")},
         true,
         false,
         false,
         false},
        {"RAMTORAM",
         "RAMTORAM",
         {numLex("1"), numLex("2"), numLex("3")},
         true,
         false,
         false,
         false},
        {"RSCTORAM",
         "RSCTORAM",
         {numLex("1"), numLex("2")},
         true,
         false,
         false,
         false},
        {"DISSCR", "DISSCR", {}, true, false, false, false},
        {"ENASCR", "ENASCR", {}, true, false, false, false},
        {"KEYCLKOFF", "KEYCLKOFF", {}, true, false, false, false},
        {"MUTE", "MUTE", {}, true, false, false, false},
        {"PLAY", "PLAY", {numLex("1")}, true, false, false, false},
        {"DRAW", "DRAW", {numLex("1")}, true, false, false, false},
        {"CLIP", "CLIP", {numLex("1")}, true, false, false, false},
        {"PT3", "PT3LOAD", {}, false, true, false, false},
        {"PLYLOAD", "PLYLOAD", {numLex("1")}, true, false, true, false},
        {"PLYPLAY", "PLYPLAY", {}, true, false, true, false},
        {"PLYSONG", "PLYSONG", {numLex("1")}, true, false, true, false},
        {"PLYMUTE", "PLYMUTE", {}, true, false, true, false},
        {"PLYLOOP", "PLYLOOP", {numLex("1")}, true, false, true, false},
        {"PLYREPLAY", "PLYREPLAY", {}, true, false, true, false},
        {"PLYSOUND", "PLYSOUND", {numLex("1")}, true, false, true, false},
        {"MTF",
         "MTF",
         {numLex("1")},
         true,
         false,
         false,
         false},
        {"MTF_2param",
         "MTF",
         {numLex("1"), numLex("0")},
         true,
         false,
         false,
         false},
        {"MTF_3param",
         "MTF",
         {numLex("2"), numLex("0"), numLex("3")},
         true,
         false,
         false,
         false},
        {"MTF_4param",
         "MTF",
         {numLex("2"), numLex("0"), numLex("3"), numLex("2")},
         true,
         false,
         false,
         false},
        {"MTF_5param",
         "MTF",
         {numLex("2"), numLex("1"), numLex("128"), numLex("96"), numLex("32")},
         true,
         false,
         false,
         false},
        {"MTF_6param",
         "MTF",
         {numLex("2"), numLex("1"), numLex("128"), numLex("96"), numLex("32"), numLex("24")},
         true,
         false,
         false,
         false},
        {"MTF_7param",
         "MTF",
         {numLex("2"), numLex("2"), numLex("0"), numLex("0"), numLex("32"), numLex("6"), numLex("0")},
         true,
         false,
         false,
         false},
        {"MTF_8param",
         "MTF",
         {numLex("2"), numLex("2"), numLex("20"), numLex("12"), numLex("10"), numLex("6"), numLex("5"), numLex("8")},
         true,
         false,
         false,
         false},
        {"MTF_9param",
         "MTF",
         {numLex("2"), numLex("2"), numLex("0"), numLex("0"), numLex("32"), numLex("24"), numLex("0"), numLex("0"), numLex("1")},
         true,
         false,
         false,
         false},
        {"MTF_10param_invalid",
         "MTF",
         {numLex("2"), numLex("2"), numLex("0"), numLex("0"), numLex("8"), numLex("8"), numLex("0"), numLex("0"), numLex("1"), numLex("5")},
         false,
         false,
         false,
         false},
        {"SETFNT", "SETFNT", {numLex("1")}, true, false, false, true},
        {"UPDFNTCLR", "UPDFNTCLR", {}, true, false, false, false},
        {"CLRSCR", "CLRSCR", {}, true, false, false, false},
        {"CLRKEY", "CLRKEY", {}, true, false, false, false},
        {"PAGE", "PAGE", {numLex("1")}, true, false, false, false},
        {"RESTORE", "RESTORE", {numLex("1")}, true, false, false, false},
        {"TURBO", "TURBO", {numLex("1")}, true, false, false, false},
    };

    for (const auto& test_case : cases) {
      SUBCASE(test_case.name) {
        shared_ptr<CpuWorkspaceContext> workspace =
            make_shared<CpuWorkspaceContext>(COMPILE_CODE_SIZE,
                                             COMPILE_RAM_SIZE, def_RAM_BOTTOM);
        shared_ptr<Z80OpcodeWriter> cpu = make_shared<Z80OpcodeWriter>();
        shared_ptr<CompilerContext> ctx = createCmdContext(cpu, workspace);

        CompilerCmdHandlerFactory factory;
        ICompilerCmdHandler* handler = factory.getByKeyword(test_case.keyword);
        REQUIRE(handler != nullptr);

        shared_ptr<ActionNode> action =
            makeCmdAction(test_case.keyword, test_case.params);

        bool ok = handler->execute(ctx, action);

        CHECK(ok == test_case.expect_success);
        CHECK(ctx->pt3 == test_case.expect_pt3);
        CHECK(ctx->akm == test_case.expect_akm);
        CHECK(ctx->font == test_case.expect_font);
      }
    }
  }
}

TEST_SUITE("CompilerFunctionStrategies") {
  TEST_CASE("All function strategies execute successfully") {
    struct FuncCase {
      const char* name;
      const char* keyword;
      unsigned int parmCount;
      std::vector<int> result;
      bool use_action_params;
      std::vector<shared_ptr<Lexeme>> action_params;
      int expected;
    };

    const FuncCase cases[] = {
        {"NOOPERATION",
         "NOOPERATION",
         0,
         {},
         false,
         {},
         Lexeme::subtype_unknown},

        {"TIME", "TIME", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"ERR", "ERR", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"POS", "POS", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"LPOS", "LPOS", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"CSRLIN", "CSRLIN", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"INKEY", "INKEY", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"INKEY$", "INKEY$", 0, {}, false, {}, Lexeme::subtype_string},
        {"MAXFILES", "MAXFILES", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"FRE", "FRE", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"HEAP", "HEAP", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"MSX", "MSX", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"NTSC", "NTSC", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"VDP", "VDP", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"TURBO", "TURBO", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"COLLISION", "COLLISION", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"COLLISION1",
         "COLLISION",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"COLLISION2",
         "COLLISION",
         2,
         {Lexeme::subtype_numeric, Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"MAKER", "MAKER", 0, {}, false, {}, Lexeme::subtype_numeric},
        {"PLYSTATUS", "PLYSTATUS", 0, {}, false, {}, Lexeme::subtype_numeric},

        {"INT",
         "INT",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"FIX",
         "FIX",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"RND",
         "RND",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"SIN",
         "SIN",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"COS",
         "COS",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"TAN",
         "TAN",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"ATN",
         "ATN",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"EXP",
         "EXP",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"LOG",
         "LOG",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"SQR",
         "SQR",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"SGN",
         "SGN",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"ABS",
         "ABS",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"VAL",
         "VAL",
         1,
         {Lexeme::subtype_string},
         false,
         {},
         Lexeme::subtype_single_decimal},

        {"PEEK",
         "PEEK",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"IPEEK",
         "IPEEK",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"VPEEK",
         "VPEEK",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"INP",
         "INP",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"DSKF",
         "DSKF",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"EOF",
         "EOF",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"LOC",
         "LOC",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"LOF",
         "LOF",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"FPOS",
         "FPOS",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"PSG",
         "PSG",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"PLAY",
         "PLAY",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"STICK",
         "STICK",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"STRIG",
         "STRIG",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"PAD",
         "PAD",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"PDL",
         "PDL",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"BASE",
         "BASE",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},

        {"ASC",
         "ASC",
         1,
         {Lexeme::subtype_string},
         false,
         {},
         Lexeme::subtype_numeric},
        {"LEN",
         "LEN",
         1,
         {Lexeme::subtype_string},
         false,
         {},
         Lexeme::subtype_numeric},
        {"CSNG",
         "CSNG",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_single_decimal},
        {"CDBL",
         "CDBL",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_double_decimal},
        {"CINT",
         "CINT",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"CHR$",
         "CHR$",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"SPACE$",
         "SPACE$",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"SPC",
         "SPC",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"TAB",
         "TAB",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"STR$",
         "STR$",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"INPUT$",
         "INPUT$",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"BIN$",
         "BIN$",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"OCT$",
         "OCT$",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"HEX$",
         "HEX$",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},

        {"RESOURCE",
         "RESOURCE",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"RESOURCESIZE",
         "RESOURCESIZE",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},

        {"USR",
         "USR",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR0",
         "USR0",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR1",
         "USR1",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR2",
         "USR2",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR3",
         "USR3",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR4",
         "USR4",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR5",
         "USR5",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR6",
         "USR6",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR7",
         "USR7",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR8",
         "USR8",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"USR9",
         "USR9",
         1,
         {Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},

        {"POINT",
         "POINT",
         2,
         {Lexeme::subtype_numeric, Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},
        {"TILE",
         "TILE",
         2,
         {Lexeme::subtype_numeric, Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_numeric},

        {"STRING$",
         "STRING$",
         2,
         {Lexeme::subtype_numeric, Lexeme::subtype_numeric},
         false,
         {},
         Lexeme::subtype_string},
        {"LEFT$",
         "LEFT$",
         2,
         {Lexeme::subtype_numeric, Lexeme::subtype_string},
         false,
         {},
         Lexeme::subtype_string},
        {"RIGHT$",
         "RIGHT$",
         2,
         {Lexeme::subtype_numeric, Lexeme::subtype_string},
         false,
         {},
         Lexeme::subtype_string},
        {"MID$",
         "MID$",
         2,
         {Lexeme::subtype_numeric, Lexeme::subtype_string},
         false,
         {},
         Lexeme::subtype_string},
        {"USING$",
         "USING$",
         2,
         {},
         true,
         {numLex("1"), strLex("\"###\"")},
         Lexeme::subtype_string},
        {"INSTR",
         "INSTR",
         2,
         {Lexeme::subtype_string, Lexeme::subtype_string},
         false,
         {},
         Lexeme::subtype_numeric},
        {"VARPTR",
         "VARPTR",
         1,
         {},
         true,
         {idLex("A")},
         Lexeme::subtype_numeric},
    };

    for (const auto& test_case : cases) {
      SUBCASE(test_case.name) {
        shared_ptr<CpuWorkspaceContext> workspace =
            make_shared<CpuWorkspaceContext>(COMPILE_CODE_SIZE,
                                             COMPILE_RAM_SIZE, def_RAM_BOTTOM);
        shared_ptr<Z80OpcodeWriter> cpu = make_shared<Z80OpcodeWriter>();
        shared_ptr<CompilerContext> ctx = createCmdContext(cpu, workspace);

        CompilerFunctionStrategyFactory factory;
        ICompilerFunctionStrategy* strategy =
            factory.getByKeyword(test_case.keyword);
        REQUIRE(strategy != nullptr);

        shared_ptr<ActionNode> action =
            test_case.use_action_params
                ? makeCmdAction(test_case.keyword, test_case.action_params)
                : make_shared<ActionNode>(test_case.keyword);

        int result[4] = {Lexeme::subtype_unknown, Lexeme::subtype_unknown,
                         Lexeme::subtype_unknown, Lexeme::subtype_unknown};
        for (size_t i = 0; i < test_case.result.size() && i < 4; i++) {
          result[i] = test_case.result[i];
        }

        int out = strategy->execute(ctx, action, result, test_case.parmCount);

        CHECK(out == test_case.expected);
      }
    }
  }
}

// NOLINTEND
